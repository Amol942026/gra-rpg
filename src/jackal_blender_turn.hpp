#pragma once

namespace turn {
// RG is displacement to the next heading and BA is displacement back.
// The grid follows the same Blender pose in both directions.
inline constexpr int GridW=26,GridH=30,MapW=Facings*GridW,MapH=Walk*GridH;
inline Texture2D fields{};
inline Shader shader{};
inline std::vector<Color> vectors;
struct Uniforms{int other,flow,tiles,grid,amount,outline;};
inline Uniforms uniforms{};

inline constexpr const char* Fragment=R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0, otherPage, flowMap;
uniform vec4 tiles;
uniform vec2 grid;
uniform float amount;
uniform int outlineOnly;
out vec4 finalColor;
const vec2 cell=vec2(104.,120.), samples=vec2(26.,30.);

vec4 displacements(vec2 p){
 vec2 q=clamp(p*samples/cell,vec2(.5),samples-.5);
 return (texture(flowMap,(grid+q)/vec2(textureSize(flowMap,0)))*255.-128.)*.125;
}
vec4 cel(sampler2D page,vec2 origin,vec2 p){
 if(any(lessThan(p,vec2(0.)))||any(greaterThanEqual(p,cell)))return vec4(0.);
 return texture(page,(origin+p)/vec2(textureSize(page,0)));
}
void main(){
 vec2 p=fragTexCoord*vec2(textureSize(texture0,0))-tiles.xy;
 vec4 d=displacements(p);
 vec2 left=p-amount*displacements(p-amount*d.xy).xy;
 vec2 right=p-(1.-amount)*displacements(p-(1.-amount)*d.zw).zw;
 vec4 a=cel(texture0,tiles.xy,left),b=cel(otherPage,tiles.zw,right);
 float wa=(1.-amount)*a.a,wb=amount*b.a,alpha=wa+wb;
 if(alpha<=0.)discard;
 vec3 rgb=outlineOnly==1?fragColor.rgb:fragColor.rgb*(a.rgb*wa+b.rgb*wb)/alpha;
 finalColor=vec4(rgb,alpha*fragColor.a);
}
)GLSL";

inline bool ready(){return shader.id&&fields.id;}
inline void unload(){
 if(shader.id)UnloadShader(shader);
 if(fields.id)UnloadTexture(fields);
 shader={};fields={};vectors.clear();
}
inline void load(){
 const char* path="assets/desert/jackal-blender/turn-flow.png";
 if(!FileExists(path))return;
 Image image=LoadImage(path);
 if(!image.data||image.width!=MapW||image.height!=MapH){if(image.data)UnloadImage(image);return;}
 ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
 const auto* colors=static_cast<const Color*>(image.data);
 vectors.assign(colors,colors+MapW*MapH);
 fields=LoadTextureFromImage(image);UnloadImage(image);
 if(!fields.id){unload();return;}
 SetTextureFilter(fields,TEXTURE_FILTER_BILINEAR);
 shader=LoadShaderFromMemory(nullptr,Fragment);
 if(!shader.id||shader.id==rlGetShaderIdDefault()){shader={};unload();return;}
 uniforms={GetShaderLocation(shader,"otherPage"),GetShaderLocation(shader,"flowMap"),
           GetShaderLocation(shader,"tiles"),GetShaderLocation(shader,"grid"),
           GetShaderLocation(shader,"amount"),GetShaderLocation(shader,"outlineOnly")};
}

struct Blend{Frame first,second;float amount;};
inline Blend between(const AnimationState& animation,int phase){
 const float angle=facingCoordinate(animation);
 const int sector=int(std::floor(angle)),first=(sector+Facings)%Facings;
 return {{first,phase},{(first+1)%Facings,phase},angle-sector};
}

inline Vector2 displacement(Frame tile,Vector2 p,bool backward){
 const float cx=std::clamp(p.x*GridW/Width-.5f,0.f,float(GridW-1));
 const float cy=std::clamp(p.y*GridH/Height-.5f,0.f,float(GridH-1));
 const int x=int(cx),y=int(cy),right=std::min(x+1,GridW-1),bottom=std::min(y+1,GridH-1);
 const int offset=tile.facing*GridW+tile.index*GridH*MapW;
 const auto component=[&](unsigned char Color::*channel){
  const auto at=[&](int i,int j){return float(vectors[offset+j*MapW+i].*channel);};
  const float a=at(x,y)+(at(right,y)-at(x,y))*(cx-x);
  const float b=at(x,bottom)+(at(right,bottom)-at(x,bottom))*(cx-x);
  return (a+(b-a)*(cy-y)-128.f)*.125f;
 };
 return {component(backward?&Color::b:&Color::r),component(backward?&Color::a:&Color::g)};
}

inline float cover(Blend blend,Vector2 p){
 if(p.x<0||p.x>=Width||p.y<0||p.y>=Height)return 0;
 const auto warp=[&](bool back,float t){
  auto d=displacement(blend.first,p,back);
  d=displacement(blend.first,{p.x-t*d.x,p.y-t*d.y},back);
  return Vector2{p.x-t*d.x,p.y-t*d.y};
 };
 const auto a=warp(false,blend.amount),b=warp(true,1-blend.amount);
 const float ca=coverage(pages[blend.first.facing/PerPage],tile(blend.first),a.x,a.y);
 const float cb=coverage(pages[blend.second.facing/PerPage],tile(blend.second),b.x,b.y);
 return ca+(cb-ca)*blend.amount;
}

inline void draw(Blend blend,Rectangle dest,Color outline,float opacity){
 const auto a=tile(blend.first),b=tile(blend.second);
 const auto& base=pages[blend.first.facing/PerPage];
 const auto& next=pages[blend.second.facing/PerPage];
 const Vector4 tiles{a.x,a.y,b.x,b.y};
 const Vector2 grid{float(blend.first.facing*GridW),float(blend.first.index*GridH)};
 BeginShaderMode(shader);
 SetShaderValue(shader,uniforms.tiles,&tiles,SHADER_UNIFORM_VEC4);
 SetShaderValue(shader,uniforms.grid,&grid,SHADER_UNIFORM_VEC2);
 SetShaderValue(shader,uniforms.amount,&blend.amount,SHADER_UNIFORM_FLOAT);
 const auto bindTextures=[&]{
  SetShaderValueTexture(shader,uniforms.flow,fields);
  SetShaderValueTexture(shader,uniforms.other,next.texture);
 };
 if(outline.a){
  const int mode=1;
  SetShaderValue(shader,uniforms.outline,&mode,SHADER_UNIFORM_INT);
  bindTextures();
  outline.a=static_cast<unsigned char>(outline.a*opacity);
  for(Vector2 shift:{Vector2{-1.3f,0},Vector2{1.3f,0},Vector2{0,-1.3f},Vector2{0,1.3f}})
   DrawTexturePro(base.texture,a,{dest.x+shift.x,dest.y+shift.y,dest.width,dest.height},{},0,outline);
  rlDrawRenderBatchActive();
 }
 const int mode=0;
 SetShaderValue(shader,uniforms.outline,&mode,SHADER_UNIFORM_INT);
 bindTextures();
 DrawTexturePro(base.texture,a,dest,{},0,Fade(WHITE,opacity));
 EndShaderMode();
}
}
