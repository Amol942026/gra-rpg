#pragma once
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace painted_turn_shape {
inline constexpr int Directions=16,Parts=6,Grid=16;
inline float mix(float a,float b,float t){return a+(b-a)*t;}

inline Rectangle interpolate(Rectangle a,Rectangle b,float t){
 return {mix(a.x,b.x,t),mix(a.y,b.y,t),mix(a.width,b.width,t),mix(a.height,b.height,t)};
}

struct Material {
 Texture2D ink{},flow{};
 std::vector<Color> pigment,displacements;

 bool ready()const{return ink.id&&flow.id;}
 void unload(){
  if(ink.id)UnloadTexture(ink);
  if(flow.id)UnloadTexture(flow);
  *this={};
 }
 void load(const std::string& prefix,int width,int height){
  if(ready())return;
  Image art=LoadImage((prefix+"-ink.png").c_str()),map=LoadImage((prefix+"-flow.png").c_str());
  if(art.data&&map.data&&art.width==width&&art.height==height&&
     map.width==Directions*Grid&&map.height==Parts*Grid){
   const auto read=[](Image& image,std::vector<Color>& data){
    ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    const auto* p=static_cast<const Color*>(image.data);
    data.assign(p,p+image.width*image.height);
    auto texture=LoadTextureFromImage(image);
    if(texture.id)SetTextureFilter(texture,TEXTURE_FILTER_BILINEAR);
    return texture;
   };
   ink=read(art,pigment);
   flow=read(map,displacements);
  }
  if(art.data)UnloadImage(art);
  if(map.data)UnloadImage(map);
  if(!ready())unload();
 }
 static float sample(const std::vector<Color>& data,Texture2D texture,float x,float y,unsigned char Color::*channel){
  x=std::clamp(x,0.f,float(texture.width-1));
  y=std::clamp(y,0.f,float(texture.height-1));
  const int i=int(x),j=int(y),r=std::min(i+1,texture.width-1),b=std::min(j+1,texture.height-1);
  auto at=[&](int u,int v){return float(data[v*texture.width+u].*channel);};
  return mix(mix(at(i,j),at(r,j),x-i),mix(at(i,b),at(r,b),x-i),y-j);
 }
 Vector2 motion(int direction,int part,Vector2 uv)const{
  const float x=direction*Grid+std::clamp(uv.x*Grid-.5f,0.f,float(Grid-1));
  const float y=part*Grid+std::clamp(uv.y*Grid-.5f,0.f,float(Grid-1));
  return {(sample(displacements,flow,x,y,&Color::r)-128)/384,
          (sample(displacements,flow,x,y,&Color::g)-128)/384};
 }
 float distance(Rectangle rect,Vector2 uv)const{
  const float x=rect.x+uv.x*rect.width,y=rect.y+uv.y*rect.height;
  const float u=std::clamp(x,rect.x-.5f,rect.x+rect.width+.5f);
  const float v=std::clamp(y,rect.y-.5f,rect.y+rect.height+.5f);
  return (sample(pigment,ink,u-.5f,v-.5f,&Color::a)-128)/10-std::hypot(x-u,y-v);
 }
 float coverage(Rectangle a,Rectangle b,int direction,int part,float t,Vector2 uv)const{
  const auto f=motion(direction,part,uv);
  const auto d=motion(direction,part,{uv.x+(.5f-t)*f.x,uv.y+(.5f-t)*f.y});
  const float left=distance(a,{uv.x-t*d.x,uv.y-t*d.y});
  const float right=distance(b,{uv.x+(1-t)*d.x,uv.y+(1-t)*d.y});
  return std::clamp(.5f+mix(left,right,t),0.f,1.f);
 }
};

// Shape follows one signed edge between the two registered outlines. The
// pigment, including small marks and highlights, comes from one painting.
inline constexpr const char* shaderCode=R"GLSL(
vec2 paintedDisplacement(sampler2D motionMap,int direction,int part,vec2 uv){
    vec2 cell=clamp(uv,vec2(.5/16.),vec2(1.-.5/16.));
    return (texture(motionMap,(vec2(direction,part)+cell)/vec2(16.,6.)).rg*255.-128.)/384.;
}
vec4 paintedTexel(sampler2D inkMap,vec4 rect,vec2 uv){
    vec2 px=rect.xy+uv*rect.zw;
    vec2 guard=clamp(px,rect.xy-.5,rect.xy+rect.zw+.5);
    vec4 paint=texture(inkMap,guard/vec2(textureSize(inkMap,0)));
    paint.a=(255.*paint.a-128.)/10.-length(guard-px);
    return paint;
}
vec4 paintedShape(sampler2D inkMap,sampler2D motionMap,int direction,int part,
                  vec4 rectA,vec4 rectB,float t,vec2 uv){
    vec2 d=paintedDisplacement(motionMap,direction,part,uv);
    d=paintedDisplacement(motionMap,direction,part,uv+(.5-t)*d);
    vec4 a=paintedTexel(inkMap,rectA,uv-t*d);
    vec4 b=paintedTexel(inkMap,rectB,uv+(1.-t)*d);
    float coverage=clamp(.5+mix(a.a,b.a,t),0.,1.);
    // Reveal ink from the other painting where the nearest view hides
    // this panel. A moving seam switches individual brush marks at depth.
    float seam=clamp(.50+.45*(uv.y-.5)+.26*(uv.x-.5),.10,.90);
    bool second=t>=seam;
    if(a.a<-.2&&b.a>.2)second=true;
    if(b.a<-.2&&a.a>.2)second=false;
    return vec4(second?b.rgb:a.rgb,coverage);
}
)GLSL";
}
