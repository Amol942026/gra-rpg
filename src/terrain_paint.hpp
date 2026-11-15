#pragma once
// Painted detail is baked into the existing terrain targets. Route coverage
// comes from the map's world coordinates, including its junctions and glades.
namespace terrain_paint {
inline constexpr const char* Directory="assets/media/terrain-paint-20261114/";
inline Texture2D load(const char* name,bool repeat=false){
 const std::string path=std::string(Directory)+name;
 if(!FileExists(path.c_str()))return {};
 auto texture=LoadTexture(path.c_str());
 if(texture.id){SetTextureFilter(texture,TEXTURE_FILTER_BILINEAR);SetTextureWrap(texture,repeat?TEXTURE_WRAP_REPEAT:TEXTURE_WRAP_CLAMP);}
 return texture;
}
inline Rectangle panoramaCrop(Texture2D image,float shift,float drift=0){
 // A little overscan gives the distant landscape a slower, bounded camera.
 const float height=std::min(image.height*.91f,image.width*.91f*H/WorldW);
 const float width=height*WorldW/H;
 return {(image.width-width)*(.5f+.38f*std::clamp(shift,-1.f,1.f)),
         (image.height-height)*(.5f+.28f*std::clamp(drift,-1.f,1.f)),width,height};
}
inline Rectangle panoramaCrop(Texture2D image){
 return panoramaCrop(image,(game.player.x-game.player.y)/MapSize,(game.player.x+game.player.y-MapSize)/MapSize);
}
inline void drawPanorama(Texture2D image){
 if(image.id)DrawTexturePro(image,panoramaCrop(image),{0,0,WorldW,H},{0,0},0,WHITE);
}
inline constexpr int Samples=6,MaskSize=MapSize*Samples;
inline constexpr const char* GroundMaterial=R"GLSL(
float groundVariation(vec2 p){
 return .5+.25*sin(p.x*.16+p.y*.09)+.25*sin(p.x*.19-p.y*.23);
}
vec3 groundColor(sampler2D material,vec2 p){
 vec3 fine=texture(material,p/4.7).rgb;
 vec3 broad=texture(material,p.yx/10.3+vec2(.17,.43)).rgb;
 return mix(fine,broad,.32)*(.91+.17*groundVariation(p));
}
float landInset(vec2 p,float high){
 vec2 edge=min(p-.5,vec2(high)-p);
 return min(edge.x,edge.y);
}
)GLSL";
inline std::array<bool,2> ready{};
inline float smooth(float t){t=std::clamp(t,0.f,1.f);return t*t*(3-2*t);}
inline void mark(Image& image,V a,V b,float margin){
 const auto pixel=[](float x){return std::clamp(int(std::floor(x*Samples)),0,MaskSize-1);};
 auto* data=static_cast<Color*>(image.data);
 for(int y=pixel(std::min(a.y,b.y)-margin);y<=pixel(std::max(a.y,b.y)+margin);y++)
  for(int x=pixel(std::min(a.x,b.x)-margin);x<=pixel(std::max(a.x,b.x)+margin);x++){
   const V p{(x+.5f)/Samples,(y+.5f)/Samples};
   const float d=desertSegmentDistance(p,a,b);
   const float edge=.1f*std::sin(p.x*3.2f+p.y*2.1f)+.06f*std::sin(p.y*5.2f-p.x*2.7f);
   const auto coverage=static_cast<unsigned char>(std::round(255*smooth((margin-d+edge)/1.05f)));
   auto& color=data[y*MaskSize+x];color.r=std::max(color.r,coverage);
  }
}
inline Image routeMask(bool forest){
 Image image=GenImageColor(MaskSize,MaskSize,BLACK);
 if(forest){
  for(const auto& route:ForestTrails)mark(image,route.a,route.b,1.85f);
  mark(image,{39,62},{66,64},1.85f);
  mark(image,{21,5},{21,5},3.6f);mark(image,ForestRitualCircle,ForestRitualCircle,3.6f);
 }else for(const auto& route:desertRoads())for(size_t i=1;i<route.points.size();i++)
  mark(image,route.points[i-1],route.points[i],1.9f*route.width);
 return image;
}
inline bool bake(bool forest){
 ready[forest]=false;
 const auto ground=load(forest?"forest-ground.png":"desert-ground.png",true);
 const auto path=load(forest?"forest-path.png":"desert-path.png",true);
 if(!ground.id||!path.id){if(ground.id)UnloadTexture(ground);if(path.id)UnloadTexture(path);return false;}
 Image mask=routeMask(forest);
 const auto coverage=LoadTextureFromImage(mask);UnloadImage(mask);
 SetTextureFilter(coverage,TEXTURE_FILTER_BILINEAR);
 const std::string fragment=std::string(R"GLSL(#version 330
in vec2 fragTexCoord;
uniform sampler2D texture0;
uniform sampler2D pathPaint;
uniform sampler2D coverage;
uniform float worldSize;
uniform vec2 walkBounds;
uniform int isForest;
out vec4 finalColor;
)GLSL")+GroundMaterial+forest_shore::Material+R"GLSL(
void main(){
 vec2 p=fragTexCoord;
 float variation=groundVariation(p);
 vec3 earth=groundColor(texture0,p);
 vec3 track=mix(texture(pathPaint,p/4.8).rgb,texture(pathPaint,p.yx/8.7).rgb,.25);
 float r=texture(coverage,p/worldSize).r;
 track*=.94+.10*variation;
 if(isForest==0)r*=.91;
 vec3 ground=mix(earth,track,r);
 if(isForest==0){
  float edge=landInset(p,worldSize-1.5);
  float ripple=.15*sin(p.x*.63+p.y*.72)+.09*sin(p.x*1.64-p.y*1.45);
  float alpha=smoothstep(0.,.98+ripple,edge);
  finalColor=vec4(ground*alpha,alpha);
  return;
 }
 float cliff=forestCliffInset(p,walkBounds);
 float grit=texture(texture0,p/1.3).g;
 float crown=1.-smoothstep(.10,.58+grit*.38,cliff);
 vec3 turf=earth*vec3(.90,1.10,.66)+vec3(.049,.059,.019);
 ground=mix(ground,turf,crown*(.26+.63*grit));
 finalColor=vec4(ground,smoothstep(-.065,.125,cliff));
}
)GLSL";
 const Shader material=LoadShaderFromMemory(nullptr,fragment.c_str());
 const bool baked=coverage.id&&IsShaderValid(material)&&material.id!=rlGetShaderIdDefault();
 if(baked){
  rlSetBlendFactors(RL_ONE,RL_ZERO,RL_FUNC_ADD);
  BeginBlendMode(BLEND_CUSTOM);
  BeginShaderMode(material);
  SetShaderValueTexture(material,GetShaderLocation(material,"pathPaint"),path);
  SetShaderValueTexture(material,GetShaderLocation(material,"coverage"),coverage);
  const int kind=forest?1:0;const float worldSize=MapSize;
  const Vector2 walkBounds{MapWalkMin,MapWalkMax};
  SetShaderValue(material,GetShaderLocation(material,"isForest"),&kind,SHADER_UNIFORM_INT);
  SetShaderValue(material,GetShaderLocation(material,"worldSize"),&worldSize,SHADER_UNIFORM_FLOAT);
  SetShaderValue(material,GetShaderLocation(material,"walkBounds"),&walkBounds,SHADER_UNIFORM_VEC2);
  const float low=forest?-1.2f:.5f,high=forest?MapSize+.2f:MapSize-1.5f;
  rlSetTexture(ground.id);rlBegin(RL_QUADS);rlColor4ub(255,255,255,255);
  for(V p:{V{low,low},V{low,high},V{high,high},V{high,low}}){
   rlTexCoord2f(p.x,p.y);rlVertex2f(TerrainOriginX+(p.x-p.y)*40,90+(p.x+p.y)*20);
  }
  rlEnd();rlSetTexture(0);
  EndShaderMode();
  EndBlendMode();
 }
 if(IsShaderValid(material)&&material.id!=rlGetShaderIdDefault())UnloadShader(material);
 if(coverage.id)UnloadTexture(coverage);
 UnloadTexture(ground);UnloadTexture(path);
 ready[forest]=baked;
 return baked;
}
}
