#pragma once
// Included within volcano_art. Bake one continuous union of curved trails.
// The small temporary mask is discarded after terrain preparation; all road
// detail and feathering then use the existing cached terrain render target.
inline bool roadBakeValid=false;
inline void bakeRoad(){
 constexpr int MaskSize=MapSize*8;Image image=GenImageColor(MaskSize,MaskSize,WHITE);auto* pixels=static_cast<Color*>(image.data);
 for(int y=0;y<MaskSize;y++)for(int x=0;x<MaskSize;x++){
  V p{(x+.5f)*MapSize/MaskSize,(y+.5f)*MapSize/MaskSize};unsigned char a=static_cast<unsigned char>(std::round(volcano_roads::coverage(p)*255));pixels[y*MaskSize+x]={a,a,a,255};
 }
 Texture2D mask=LoadTextureFromImage(image);UnloadImage(image);SetTextureFilter(mask,TEXTURE_FILTER_BILINEAR);
 Shader shader=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform sampler2D roadMask;
uniform float mapSize;
out vec4 finalColor;
void main(){
 vec2 world=fragTexCoord*6.0;
 float coverage=texture(roadMask,world/mapSize).r;
 if(coverage<.002)discard;
 vec4 material=texture(texture0,fragTexCoord);
 finalColor=vec4(material.rgb,material.a*coverage);
}
)GLSL");
 roadBakeValid=mask.id&&IsShaderValid(shader);
 if(roadBakeValid){
  BeginShaderMode(shader);SetShaderValueTexture(shader,GetShaderLocation(shader,"roadMask"),mask);float mapSize=float(MapSize);SetShaderValue(shader,GetShaderLocation(shader,"mapSize"),&mapSize,SHADER_UNIFORM_FLOAT);
  groundQuad(road,{{{BoundaryLow,BoundaryLow},{BoundaryLow,BoundaryHigh},{BoundaryHigh,BoundaryHigh},{BoundaryHigh,BoundaryLow}}},BoundaryLow/6,BoundaryLow/6,BoundaryHigh/6,BoundaryHigh/6);
  EndShaderMode();
 }
 UnloadShader(shader);UnloadTexture(mask);
}
