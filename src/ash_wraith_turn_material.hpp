#pragma once
#include "generated/ash_wraith_cels.hpp"

namespace world_shadows { extern Shader maskShader; }

namespace wraith_death_art::rotation {
static_assert(ash_wraith_turn_atlas::PageCount==1,"Both registered cloth views must share one texture");
inline Shader shader{};
inline Texture2D flow{},edges{},weave{},robeCels{};
inline std::vector<Color> vectors;
inline std::vector<unsigned char> distance;
struct Uniforms {int rectA,rectB,motion,edge,relief,phase,celBounds,celMap,celOffset;};
inline Uniforms uniforms{};
inline bool ready(){return flow.id&&edges.id&&weave.id&&shader.id&&shader.id!=rlGetShaderIdDefault();}
inline void unload(){
 if(flow.id)UnloadTexture(flow);
 if(edges.id)UnloadTexture(edges);
 if(weave.id)UnloadTexture(weave);
 if(robeCels.id)UnloadTexture(robeCels);
 if(shader.id&&shader.id!=rlGetShaderIdDefault())UnloadShader(shader);
 flow={};edges={};weave={};robeCels={};shader={};vectors.clear();distance.clear();
}
inline void load(){
 Image f=LoadImage("assets/volcano/wraith-turn/registration.png");
 Image e=LoadImage("assets/volcano/wraith-turn/edges.png");
 if(f.data&&e.data&&f.width==32*32&&f.height==32*6){
  ImageFormat(&f,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  ImageFormat(&e,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  auto* fc=static_cast<Color*>(f.data);auto* ec=static_cast<Color*>(e.data);
  vectors.assign(fc,fc+f.width*f.height);distance.resize(e.width*e.height);
  for(size_t i=0;i<distance.size();++i)distance[i]=ec[i].r;
  flow=LoadTextureFromImage(f);edges=LoadTextureFromImage(e);
  SetTextureFilter(flow,TEXTURE_FILTER_BILINEAR);SetTextureFilter(edges,TEXTURE_FILTER_BILINEAR);
 }
 if(f.data)UnloadImage(f);
 if(e.data)UnloadImage(e);
 weave=LoadTexture("assets/volcano/wraith-turn/weave.png");
 if(!flow.id||!edges.id||!weave.id)return;
 SetTextureFilter(weave,TEXTURE_FILTER_BILINEAR);
 SetTextureWrap(weave,TEXTURE_WRAP_REPEAT);
 robeCels=LoadTexture("assets/volcano/wraith-turn/directional-robe.png");
 if(robeCels.id)SetTextureFilter(robeCels,TEXTURE_FILTER_BILINEAR);
 shader=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;
uniform sampler2D texture0, motionMap, edgeMap, clothRelief;
uniform vec4 rectA[6], rectB[6];
uniform vec4 phase; // geometry view, fraction, settled weight, cloth longitude in turns
uniform vec4 celBounds[6],celMap[6];
uniform float celOffset;
vec2 displacement(int p,vec2 uv){
 vec2 cell=clamp(uv,vec2(.5/32.),vec2(1.-.5/32.));
 return (texture(motionMap,(vec2(phase.x,p)+cell)/vec2(32.,6.)).rg*255.-128.)/384.;
}
vec2 pixel(vec4 rect,vec2 uv){return rect.xy+uv*rect.zw;}
float boundary(vec4 rect,vec2 uv){
 vec2 pos=pixel(rect,uv);
 vec2 guard=clamp(pos,rect.xy-vec2(1.5),rect.xy+rect.zw+vec2(1.5));
 return (texture(edgeMap,guard/vec2(textureSize(edgeMap,0))).r*255.-128.)/8.-length(pos-guard);
}
void main(){
 int mode=int(floor(fragTexCoord.x/16.));
 float tag=fragTexCoord.x-16.*float(mode);
 int part=int(floor(tag/2.));
 vec2 at=vec2(tag-float(part)*2.,fragTexCoord.y);
 vec4 a=rectA[part],b=rectB[part];
 float t=phase.y;
 vec2 d=displacement(part,at);
 d=displacement(part,at+(.5-t)*d);
 vec2 qa=at-t*d,qb=at+(1.-t)*d;
 // Interpolate the registered signed boundary. One zero crossing defines the
 // cloth edge even when the openings in two painted sleeves differ in size.
 float sa=boundary(a,qa),sb=boundary(b,qb);
 float coverage=clamp(.5+mix(sa,sb,t),0.,1.);
 if(mode==2){
  if(coverage<.62)discard;
  finalColor=vec4(fragColor.a);return;
 }
 if(coverage<.001)discard;
 if(mode==1){finalColor=vec4(fragColor.rgb,coverage*fragColor.a);return;}
 vec2 shape=vec2(textureSize(edgeMap,0));
 vec2 pa=pixel(a,clamp(qa,0.,1.))/shape,pb=pixel(b,clamp(qb,0.,1.))/shape;
 vec3 color;
 if(celBounds[part].z>0.){
  // The fold pigment is an already painted cel. Offset the one drawing by
  // the residual angle; the edge and the loose fabric still follow the rig.
  vec2 original=at+vec2(celOffset,0.);
  vec2 coords=celMap[part].xy+original*celMap[part].zw;
  vec4 limits=celBounds[part];
  color=texture(texture0,clamp(coords,limits.xy,limits.zw)).rgb;
 }else{
  vec2 chart=mix(texture(edgeMap,pixel(a,at)/shape).gb,texture(edgeMap,pixel(b,at)/shape).gb,t);
  float rows=float(textureSize(texture0,0).y)/6.;
  const float hem=16.;
  float v=(float(part)*rows+hem+.5+clamp(chart.y,0.,1.)*(rows-2.*hem-1.))/(6.*rows);
  vec2 dye=vec2(phase.w+.5*(chart.x-.5),v);
  vec2 dims=vec2(textureSize(texture0,0));
  vec2 dx=dFdx(dye)*dims,dy=dFdy(dye)*dims;
  vec2 along=dot(dx,dx)>dot(dy,dy)?dx:dy;
  float footprint=length(along);
  color=textureLod(texture0,dye,0.).rgb;
  if(footprint>4.){
   vec2 tap=along/max(footprint,.01)*min(.92,.12*footprint)/dims;
   vec3 side=.5*(textureLod(texture0,dye-tap,0.).rgb+textureLod(texture0,dye+tap,0.).rgb);
   color=mix(color,side,.25*smoothstep(4.,9.,footprint));
  }
 }
 if(phase.z>0.){
  vec4 material=mix(texture(clothRelief,pa),texture(clothRelief,pb),t);
  float depth=material.a;
  vec3 n=normalize(vec3(-19.*vec2(dFdx(depth),dFdy(depth)),1.));
  float crown=smoothstep(.12,.91,depth);
  float diffuse=max(0.,dot(n,normalize(vec3(-.52,.71,.62))));
  float sheen=pow(max(0.,dot(n,normalize(vec3(-.22,.55,.81)))),5.);
  vec3 lit=clamp(color+material.rgb-vec3(128./255.),0.,1.)*(.80+.12*crown+.25*diffuse)
           +vec3(.023,.025,.028)*crown*sheen;
  color=mix(color,lit,phase.z);
 }
 finalColor=vec4(color*fragColor.rgb,coverage*fragColor.a);
})GLSL");
 if(ready()){
  const auto loc=[](const char* s){return GetShaderLocation(shader,s);};
  uniforms={loc("rectA[0]"),loc("rectB[0]"),loc("motionMap"),loc("edgeMap"),loc("clothRelief"),loc("phase"),
            loc("celBounds[0]"),loc("celMap[0]"),loc("celOffset")};
 }
}
inline void begin(int left,int right,float t,float angle,float folds,Texture2D relief,bool useCels=true){
 Vector4 rects[2][6];
 for(int i=0;i<2;i++)for(int p=0;p<6;p++){
  const auto& tile=ash_wraith_turn_atlas::tiles[i?right:left][p];
  rects[i][p]={float(tile.x),float(tile.y),float(tile.w),float(tile.h)};
 }
 BeginShaderMode(shader);
 SetShaderValueV(shader,uniforms.rectA,rects[0],SHADER_UNIFORM_VEC4,6);
 SetShaderValueV(shader,uniforms.rectB,rects[1],SHADER_UNIFORM_VEC4,6);
 Vector4 phase{float(left),t,folds,angle/(2*PI)};
 SetShaderValue(shader,uniforms.phase,&phase,SHADER_UNIFORM_VEC4);
 Vector4 bounds[6]{},mapping[6]{};
 if(robeCels.id&&useCels){
  const float position=phase.w*ash_wraith_cels::Directions;
  const int pose=int(std::lround(position));
  const int key=pose%ash_wraith_cels::Directions;
  const float offset=(position-pose)*2/ash_wraith_cels::Directions;
  for(int p=0;p<6;p++){
   const auto& tile=ash_wraith_cels::tiles[key][p];
   const float w=float(robeCels.width),h=float(robeCels.height);
   bounds[p]={(tile.x+.5f)/w,(tile.y+.5f)/h,(tile.x+tile.w-.5f)/w,(tile.y+tile.h-.5f)/h};
   mapping[p]={(tile.x-tile.ox)/w,(tile.y-tile.oy)/h,tile.canvasWidth/w,tile.canvasHeight/h};
  }
  SetShaderValue(shader,uniforms.celOffset,&offset,SHADER_UNIFORM_FLOAT);
 }
 SetShaderValueV(shader,uniforms.celBounds,bounds,SHADER_UNIFORM_VEC4,6);
 SetShaderValueV(shader,uniforms.celMap,mapping,SHADER_UNIFORM_VEC4,6);
 SetShaderValueTexture(shader,uniforms.motion,flow);SetShaderValueTexture(shader,uniforms.edge,edges);
 if(folds>0&&relief.id)SetShaderValueTexture(shader,uniforms.relief,relief);
}
inline Texture2D paintTexture(){return robeCels.id?robeCels:weave;}
template<typename Sample>
inline float bilinear(float x,float y,Sample sample){
 const int ix=int(std::floor(x)),iy=int(std::floor(y));const float fx=x-ix,fy=y-iy;
 return (1-fy)*((1-fx)*sample(ix,iy)+fx*sample(ix+1,iy))+fy*((1-fx)*sample(ix,iy+1)+fx*sample(ix+1,iy+1));
}
inline Vector2 displacement(int k,int p,Vector2 at){
 const float x=k*32+std::clamp(at.x*32-.5f,0.f,31.f);
 const float y=p*32+std::clamp(at.y*32-.5f,0.f,31.f);
 auto component=[&](bool vertical){
  const float code=bilinear(x,y,[&](int col,int row){
   const auto& v=vectors[std::min(flow.height-1,row)*flow.width+std::min(flow.width-1,col)];
   return vertical?v.g:v.r;
  });
  return (code-128)/384.f;
 };
 return {component(false),component(true)};
}
inline float boundary(const ash_wraith_turn_atlas::Tile& tile,Vector2 uv){
 const float x=tile.x+uv.x*tile.w,y=tile.y+uv.y*tile.h;
 const float gx=std::clamp(x,float(tile.x)-1.5f,float(tile.x+tile.w)+1.5f);
 const float gy=std::clamp(y,float(tile.y)-1.5f,float(tile.y+tile.h)+1.5f);
 const float v=bilinear(gx-.5f,gy-.5f,[&](int col,int row){
  return distance[std::clamp(row,0,edges.height-1)*edges.width+std::clamp(col,0,edges.width-1)];
 });
 return (v-128)/8.f-std::hypot(x-gx,y-gy);
}
inline bool solid(int left,int right,float t,int part,Vector2 uv){
 const auto d=displacement(left,part,uv);
 const auto v=displacement(left,part,{uv.x+(.5f-t)*d.x,uv.y+(.5f-t)*d.y});
 const float a=boundary(ash_wraith_turn_atlas::tiles[left][part],{uv.x-t*v.x,uv.y-t*v.y});
 const float b=boundary(ash_wraith_turn_atlas::tiles[right][part],{uv.x+(1-t)*v.x,uv.y+(1-t)*v.y});
 return a+(b-a)*t>=-.1f;
}
}
