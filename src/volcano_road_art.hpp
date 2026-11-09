#pragma once
// Included within volcano_art. Bake one continuous union of curved trails.
// The small temporary mask is discarded after terrain preparation; all road
// detail and feathering then use the existing cached terrain render target.
inline bool roadBakeValid=false;
inline void bakeRoad(){
 constexpr int SamplesPerTile=8,MaskSize=MapSize*SamplesPerTile;
 Image image=GenImageColor(MaskSize,MaskSize,BLACK);auto* pixels=static_cast<Color*>(image.data);
 constexpr float reach=volcano_roads::FeatherLimit;
 auto pixel=[](float pos){return std::clamp(int(std::floor(pos*SamplesPerTile)),0,MaskSize-1);};
 for(const auto& segment:volcano_roads::segments()){
  int left=pixel(std::min(segment.a.x,segment.b.x)-reach),right=pixel(std::max(segment.a.x,segment.b.x)+reach);
  int top=pixel(std::min(segment.a.y,segment.b.y)-reach),bottom=pixel(std::max(segment.a.y,segment.b.y)+reach);
  for(int y=top;y<=bottom;y++)for(int x=left;x<=right;x++){
   V p{(x+.5f)/SamplesPerTile,(y+.5f)/SamplesPerTile};
   float coverage=volcano_roads::coverage(p,desertSegmentDistance(p,segment.a,segment.b));
   auto a=static_cast<unsigned char>(std::round(coverage*255));
   Color& sample=pixels[y*MaskSize+x];
   if(a>sample.r)sample={a,a,a,255};
  }
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
