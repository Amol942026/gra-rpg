#pragma once

namespace volcano_corpse_material {
inline Shader shader{};
inline int guideLocation=-1,strengthLocation=-1,paintLocation=-1;

inline void init(){
 const char* fragment=R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform sampler2D reliefGuide;
uniform float strength;
uniform float authoredPaint;
out vec4 finalColor;
void main(){
 vec4 base=texture(texture0,fragTexCoord);
 vec4 paint=base*fragColor;
 vec4 material=texture(reliefGuide,fragTexCoord);
 float depth=mix(material.r,material.a,authoredPaint);
 vec2 gradient=vec2(dFdx(depth),dFdy(depth));
 vec3 normal=normalize(vec3(-gradient*19.0,1.0));
 vec3 light=normalize(vec3(-0.52,0.71,0.62));
 float diffuse=clamp(dot(normal,light),0.0,1.0);
 float crown=smoothstep(0.12,0.91,depth);
 float sheer=pow(max(dot(normal,normalize(vec3(-0.22,0.55,0.81))),0.0),5.0);
 float shade=mix(0.70+0.22*crown+0.37*diffuse,
                 0.80+0.12*crown+0.25*diffuse,authoredPaint);
 vec3 pigment=clamp(base.rgb+authoredPaint*(material.rgb-vec3(128.0/255.0)),0.0,1.0)*fragColor.rgb;
 vec3 rim=mix(vec3(0.076,0.055,0.030),vec3(0.023,0.025,0.028),authoredPaint);
 vec3 lit=pigment*shade+rim*crown*sheer;
 finalColor=vec4(mix(paint.rgb,lit,strength),paint.a);
}
)GLSL";
 shader=LoadShaderFromMemory(nullptr,fragment);
 guideLocation=GetShaderLocation(shader,"reliefGuide");
 strengthLocation=GetShaderLocation(shader,"strength");
 paintLocation=GetShaderLocation(shader,"authoredPaint");
}

inline Texture2D loadPainted(const char* species){
 const auto path=std::string("assets/volcano/corpses/")+species+".png";
 if(!FileExists(path.c_str()))return {};
 Image source=LoadImage(path.c_str());
 const int cell=source.width/8;
 const bool matches=source.data&&source.width%8==0&&cell>=96&&cell<=256&&source.height==6*cell;
 Texture2D result{};
 if(matches){
  ImageFormat(&source,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  result=LoadTextureFromImage(source);
  SetTextureFilter(result,TEXTURE_FILTER_BILINEAR);
 }
 if(source.data)UnloadImage(source);
 return result;
}

// Estimate a shallow convex surface inside each painted cutout. Every cell
// starts at zero on its own border so adjacent armor pieces cannot leak in.
inline Texture2D buildGuide(const std::vector<unsigned char>& alpha,int cell,int width,int height){
 std::vector<unsigned char> guide(alpha.size());
 std::vector<int> distance(cell*cell);
 for(int row=0;row<6;row++)for(int column=0;column<8;column++){
  const int base=row*cell*width+column*cell;
  for(int y=0;y<cell;y++)for(int x=0;x<cell;x++){
   const bool interior=x>0&&y>0&&x+1<cell&&y+1<cell;
   distance[y*cell+x]=interior&&alpha[base+y*width+x]>=110?3*cell:0;
  }
  for(int y=1;y<cell-1;y++)for(int x=1;x<cell-1;x++){
   int p=y*cell+x;
   distance[p]=std::min({distance[p],distance[p-1]+3,distance[p-cell]+3,distance[p-cell-1]+4,distance[p-cell+1]+4});
  }
  for(int y=cell-2;y>0;y--)for(int x=cell-2;x>0;x--){
   int p=y*cell+x;
   distance[p]=std::min({distance[p],distance[p+1]+3,distance[p+cell]+3,distance[p+cell-1]+4,distance[p+cell+1]+4});
  }
  const float radius=std::max(18.f,float(*std::max_element(distance.begin(),distance.end())));
  for(int y=0;y<cell;y++)for(int x=0;x<cell;x++){
   const float relative=distance[y*cell+x]/radius;
   guide[base+y*width+x]=static_cast<unsigned char>(255*std::sqrt(relative)+.5f);
  }
 }
 const Texture2D texture=LoadTextureFromImage({guide.data(),width,height,1,PIXELFORMAT_UNCOMPRESSED_GRAYSCALE});
 SetTextureFilter(texture,TEXTURE_FILTER_BILINEAR);
 return texture;
}

inline bool begin(Texture2D guide,float strength,bool authored){
 if(!guide.id||guideLocation<0||strengthLocation<0||paintLocation<0||strength<=0)return false;
 BeginShaderMode(shader);
 SetShaderValueTexture(shader,guideLocation,guide);
 SetShaderValue(shader,strengthLocation,&strength,SHADER_UNIFORM_FLOAT);
 const float paint=authored?1.f:0.f;
 SetShaderValue(shader,paintLocation,&paint,SHADER_UNIFORM_FLOAT);
 return true;
}

inline void unload(){
 if(IsShaderValid(shader))UnloadShader(shader);
 shader={};guideLocation=strengthLocation=paintLocation=-1;
}
}
