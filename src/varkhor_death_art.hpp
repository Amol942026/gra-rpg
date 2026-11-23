#pragma once

#include "generated/varkhor_ash_motion.hpp"

namespace varkhor_death_art {
constexpr int PileWidth=256,PileHeight=128,PuffSize=64;
constexpr int AirCloudSize=256;
constexpr unsigned TorsoParts=(1u<<0)|(1u<<3);
constexpr Rectangle PileArea{-112,-51,224,76};
inline Texture2D ash{},edge{},airborneAsh{};
inline Shader dissolve{};
inline int phaseLocation=-1;
inline std::vector<unsigned char> pileAlpha;

inline void init(){
 if(ash.id)return;
 Image source=LoadImage("assets/volcano/varkhor-ash.png");
 if(!source.data)return;
 if(source.width!=PileWidth||source.height!=PileHeight+PuffSize){UnloadImage(source);return;}
 ImageFormat(&source,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
 ash=LoadTextureFromImage(source);SetTextureFilter(ash,TEXTURE_FILTER_BILINEAR);
 const auto* original=static_cast<const Color*>(source.data);
 Image white=GenImageColor(PileWidth,PileHeight,BLANK);
 auto* mask=static_cast<Color*>(white.data);
 pileAlpha.resize(PileWidth*PileHeight);
 for(std::size_t i=0;i<pileAlpha.size();i++){
  const auto opacity=original[i].a;
  pileAlpha[i]=opacity;
  mask[i]={255,255,255,opacity};
 }
 edge=LoadTextureFromImage(white);SetTextureFilter(edge,TEXTURE_FILTER_BILINEAR);
 UnloadImage(white);UnloadImage(source);
 Image airborne=LoadImage("assets/volcano/varkhor-ash-cloud.png");
 if(airborne.data){
  if(airborne.width==AirCloudSize&&airborne.height==AirCloudSize){
   airborneAsh=LoadTextureFromImage(airborne);
   SetTextureFilter(airborneAsh,TEXTURE_FILTER_BILINEAR);
  }
  UnloadImage(airborne);
 }
 // Leave the live cutout intact and reveal fine holes progressively. The
 // sample is tied to UVs so individual grains move with the collapsing body.
 const char* fragment=R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform float phase[6];
out vec4 finalColor;
vec2 stoneSeed(vec2 p){
 vec3 v=fract(vec3(p.x,p.y,p.x)*vec3(.1031,.1030,.0973));
 v+=dot(v,v.yzx+33.33);
 return fract((v.xx+v.yz)*v.zy);
}
vec2 fractures(vec2 p){
 vec2 base=floor(p);
 vec2 local=fract(p);
 float closest=4.0,second=4.0,plate=0.0;
 for(int y=-1;y<=1;++y)for(int x=-1;x<=1;++x){
  vec2 offset=vec2(x,y);
  vec2 seed=stoneSeed(base+offset);
  vec2 delta=offset+0.14+seed*0.72-local;
  float d=dot(delta,delta);
  if(d<closest){
   second=closest;
   closest=d;
   plate=seed.x;
  }else second=min(second,d);
 }
 return vec2(plate,second-closest);
}
void main(){
 vec4 paint=texture(texture0,fragTexCoord)*fragColor;
 vec2 part=fragTexCoord*vec2(8.0,6.0);
 vec2 local=fract(part);
 int partIndex=clamp(int(part.y),0,5);
 float wear=phase[partIndex];
 vec2 warp=0.31*sin(local.yx*vec2(59.0,72.0)+sin(local*27.0));
 vec2 stone=fractures(mat2(.96,.27,-.27,.96)*(local*vec2(25.0,30.0)+warp)+floor(part)*vec2(5.0,11.0));
 float grain=stoneSeed(floor(part*139.0)).y;
 // Dissolve the lower plates first. Each angular cell keeps its shape as
 // the local fissures emerge and the material behind them gives way.
 float resistance=0.36*stone.x+0.09*grain+0.64*(1.0-local.y);
 bool torso=partIndex==0||partIndex==3;
 if(torso){
  // The chest caves in around the ember, then the outside plates crumble.
  float coreDistance=length((local-vec2(.50,partIndex==0?.67:.58))*vec2(1.25,1.0));
  resistance=.43*stone.x+.10*grain+.55*smoothstep(.06,.37,coreDistance);
 }
 float threshold=mix(-0.10,1.18,wear);
 float rim=resistance-threshold;
 float fractureFront=(1.0-smoothstep(.035,.16,abs(rim)))*smoothstep(0.0,.14,wear);
 float seam=(1.0-smoothstep(.018,.095,stone.y))*fractureFront;
 float alpha=smoothstep(-.035,.047,rim-seam*.038);
 float heat=clamp((paint.r-paint.b-.08)*2.2,0.0,1.0);
 float ashEdge=wear*(1.0-smoothstep(.025,.27,rim));
 vec3 charred=mix(paint.rgb,vec3(.37,.33,.31),ashEdge*.67);
 vec3 coated=charred*(1.0-.36*seam)+vec3(.31,.11,.026)*heat*seam;
 if(torso){
  float cool=smoothstep(.08,.43,wear)*heat;
  float luma=dot(coated,vec3(.299,.587,.114));
  coated=mix(coated,luma*vec3(.83,.78,.74),cool);
 }
 finalColor=vec4(coated,paint.a*alpha);
}
)GLSL";
 dissolve=LoadShaderFromMemory(nullptr,fragment);
 phaseLocation=GetShaderLocation(dissolve,"phase[0]");
}

inline void unload(){
 if(ash.id)UnloadTexture(ash);
 if(edge.id)UnloadTexture(edge);
 if(airborneAsh.id)UnloadTexture(airborneAsh);
 if(phaseLocation>=0)UnloadShader(dissolve);
 ash={};edge={};airborneAsh={};dissolve={};phaseLocation=-1;pileAlpha.clear();
}

inline varkhor_ash_motion::Key sample(float phase){
 using namespace varkhor_ash_motion;
 const float frame=std::clamp(phase,0.f,1.f)*Frames;
 const int i=std::min(Frames-1,int(frame));
 const float u=frame-i;
 const auto a=samples[i],b=samples[i+1];
 return {a.dissolve+(b.dissolve-a.dissolve)*u,a.sink+(b.sink-a.sink)*u,
         a.cloud+(b.cloud-a.cloud)*u,a.pile+(b.pile-a.pile)*u,a.torso+(b.torso-a.torso)*u};
}

inline float side(const Enemy& e){
 const int flank=e.deathScreenSide();
 return flank!=0?float(flank):e.animation.direction<=Direction::S?1.f:-1.f;
}
inline Rectangle footprint(const Enemy&){return {-58,-9,116,23};}

inline bool ashes(const Enemy& e,float x,float y,float fill,Color outline,const Vector2* probe,float opacity){
 if(fill<=0||!ash.id)return false;
 const bool mirror=side(e)<0;
 const float scale=.79f+.21f*fill;
 x+=(fill-1)*side(e)*8;
 const Rectangle area{x+PileArea.x*scale,y+PileArea.y*scale,PileArea.width*scale,PileArea.height*scale};
 const float coverage=fill*opacity;
 if(probe){
  int u=int(std::floor((probe->x-area.x)*PileWidth/area.width));
  int v=int(std::floor((probe->y-area.y)*PileHeight/area.height));
  if(u<0||u>=PileWidth||v<0||v>=PileHeight)return false;
  if(mirror)u=PileWidth-1-u;
  return pileAlpha[v*PileWidth+u]*coverage>=112;
 }
 const Rectangle source{0,0,mirror?-float(PileWidth):float(PileWidth),PileHeight};
 if(outline.a&&edge.id)for(Vector2 offset:{Vector2{-1.4f,0},Vector2{1.4f,0},Vector2{0,-1.4f},Vector2{0,1.4f}}){
  DrawTexturePro(edge,source,{area.x+offset.x,area.y+offset.y,area.width,area.height},{},0,Fade(outline,float(outline.a)/255*coverage));
 }
 DrawTexturePro(ash,source,area,{},0,Fade(WHITE,coverage));
 return false;
}

inline bool cutout(const Enemy& e,float x,float y,const varkhor_ash_motion::Key& motion,
                   Color outline,const Vector2* probe,bool shadow,float opacity,unsigned partMask=0x3fu){
 if(motion.dissolve>=1)return false;
 // The chest has its own crumble track. The rest of the armor uses staggered windows.
 constexpr std::array starts{0.f,.04f,.12f,0.f,.15f,.32f};
 constexpr std::array ends{1.f,.43f,.49f,1.f,.59f,.74f};
 constexpr std::array descent{32.f,13.f,15.f,33.f,23.f,25.f};
 constexpr std::array fan{0.f,-10.f,10.f,-2.f,-14.f,14.f};
 constexpr std::array rotation{9.f,-18.f,21.f,8.f,-23.f,26.f};
 const bool useShader=phaseLocation>=0&&motion.dissolve>0&&!shadow&&!probe;
 const float flank=side(e);
 std::array<float,6> wear{};
 volcano_actors::BrokenParts parts{};
 for(std::size_t p=0;p<parts.size();p++){
  const unsigned part=1u<<p;
  const bool torso=(TorsoParts&part)!=0;
  wear[p]=torso?motion.torso:enemy_death::window(motion.dissolve,starts[p],ends[p]);
  const float release=enemy_death::window(wear[p],.02f,.84f);
  const float releaseWeight=torso?.50f:.26f;
  const float drop=descent[p]*((1-releaseWeight)*motion.sink+releaseWeight*release);
  const float visibility=useShader?float(wear[p]<1.f):1.f-wear[p];
  parts[p]={{flank*4.f*motion.sink+fan[p]*release,drop},
            flank*5.f*motion.sink+rotation[p]*release,
            (partMask&part)!=0?visibility:0.f};
 }
 if(useShader){
  BeginShaderMode(dissolve);
  SetShaderValueV(dissolve,phaseLocation,wear.data(),SHADER_UNIFORM_FLOAT,int(wear.size()));
 }
 const bool picked=volcano_actors::draw(e.type,x,y,1,e.deathMotion,e.deathAction,outline,probe,shadow,false,
                                       false,e.deathSpecial,nullptr,0,opacity,0,&parts,volcano_actors::punchTarget(e));
 if(useShader)EndShaderMode();
 return picked;
}

inline void rollingAsh(const Enemy& e,float x,float y,float elapsed){
 using namespace varkhor_ash_motion;
 if(elapsed<0||elapsed>=enemy_death::endTime(e.type)||!ash.id)return;
 const float frame=std::min(float(AirFrames),elapsed*FramesPerSecond);
 const int i=std::min(AirFrames-1,int(frame));
 const float u=frame-i;
 const auto a=airSamples[i],b=airSamples[i+1];
 const float opacity=a.opacity+(b.opacity-a.opacity)*u;
 if(opacity<=0)return;
 const float spread=a.spread+(b.spread-a.spread)*u;
 const float settle=a.settle+(b.settle-a.settle)*u;
 const float altitude=1-settle;
 const float wind=side(e);
 const float floorBlend=enemy_death::window(settle,.78f,1.f);
 const float width=183*(.72f+.28f*spread)*(1-.20f*floorBlend);
 const float height=13+201*altitude;
 const Vector2 center{x+wind*6*spread,y-6-106*altitude};
 const Rectangle tile=airborneAsh.id
     ?Rectangle{0,0,wind<0?-float(AirCloudSize):float(AirCloudSize),AirCloudSize}
     :Rectangle{PuffSize,PileHeight,PuffSize,PuffSize};
 const Color tone=ColorLerp(rgb(252,245,238),rgb(223,210,199),settle);
 DrawTexturePro(airborneAsh.id?airborneAsh:ash,tile,{center.x,center.y,width,height},{width*.5f,height*.5f},
                wind*5*altitude,Fade(tone,opacity));
}

inline void sparks(const Enemy& e,float x,float y,float t,float strength){
 if(strength<=0||!ash.id)return;
 const float wind=side(e);
 const float burst=enemy_death::window(t,.17f,.40f)*(1-enemy_death::window(t,.50f,.72f));
 for(int i=0;i<36;i+=7){
  const float delay=.29f+.014f*(i%5);
  const float life=std::clamp((t-delay)/(.89f-delay),0.f,1.f);
  const float spread=enemy_death::ease(life);
  const float fade=enemy_death::window(life,0,.31f)*(1-enemy_death::window(life,.44f,1));
  if(fade<=0)continue;
  const float launch=std::sin(life*PI);
  const float drift=(float((i*59)%89)-44)*(.24f+spread)+wind*7*spread;
  const float height=84*(1-spread)+launch*18;
  const Vector2 p{x+drift+wind*launch*8,y-height+(i%7-3)*4.4f*spread};
  const Color tone=ColorLerp(rgb(251,169,88),rgb(185,79,49),spread);
  const float size=1.38f*(1-.48f*spread);
  const float visibility=fade*strength*(.88f+.24f*burst);
  const Color glow=rgb(227,103,34);
  DrawCircleGradient(int(std::lround(p.x)),int(std::lround(p.y)),size*3.1f,Fade(glow,visibility*.41f),Fade(glow,0));
  const Vector2 trail{p.x-(drift+wind*10)*.10f,p.y+2.0f-2*spread};
  DrawLineEx(trail,p,1.1f,Fade(tone,visibility*.67f));
  DrawCircleV(p,size,Fade(tone,visibility));
 }
}

inline bool fallingBasalt(const Enemy& e,float x,float y,float time,float opacity,const Vector2* probe=nullptr){
 const float wind=side(e);
 for(int i=0;i<14;i++){
  const float level=float((i*9)%14)/13;
  const float birth=.13f+.39f*level;
  const float life=std::clamp((time-birth)/(.84f-birth),0.f,1.f);
  const float first=enemy_death::window(life,0,.17f);
  const float grind=enemy_death::window(life,.49f,1);
  const float coverage=first*(1-grind)*opacity;
  if(coverage<=0||(probe&&coverage<.44f))continue;
  const float drift=life*(2-life);
  const float startSide=float((i*7)%15-7);
  const float height=(22+114*level)*(1-life*life)+std::sin(life*PI)*6.f;
  const Vector2 center{x+startSide*(4.1f+4.4f*drift)+wind*9*drift,
                       y-height+float(i%4-2)*4*drift};
  const float size=(2.5f+.58f*(i%5))*(1-.72f*grind);
  const float angle=float(i)*1.9f+wind*(2.1f+.57f*(i%4))*drift;
  const float c=std::cos(angle),s=std::sin(angle);
  const auto point=[&](float px,float py){return Vector2{center.x+size*(c*px-s*py),center.y+size*(s*px+c*py)};};
  const auto top=point(-.38f,-.92f),left=point(-1,.11f),bottom=point(.43f,.69f);
  const auto right=point(.81f,-.22f),ridge=point(.17f,-.29f);
  if(probe){
   if(CheckCollisionPointTriangle(*probe,top,left,bottom)||
      CheckCollisionPointTriangle(*probe,top,bottom,right))return true;
   continue;
  }
  const Color edge=i%5==0?rgb(183,108,61):rgb(112,104,104);
  DrawTriangle(top,left,ridge,Fade(edge,coverage));
  DrawTriangle(left,bottom,ridge,Fade(rgb(66,60,60),coverage));
  DrawTriangle(bottom,right,ridge,Fade(rgb(83,72,70),coverage));
  DrawTriangle(right,top,ridge,Fade(rgb(124,110,106),coverage));
 }
 return false;
}

inline bool draw(const Enemy& e,float x,float y,float normalizedTime,Color outline,
                 const Vector2* probe,bool shadow,float opacity){
 const auto motion=sample(normalizedTime);
 if(shadow)return cutout(e,x,y,motion,BLANK,nullptr,true,opacity);
 if(probe)return ashes(e,x,y,motion.pile,BLANK,probe,opacity)
             ||cutout(e,x,y,motion,BLANK,probe,false,opacity)
             ||fallingBasalt(e,x,y,normalizedTime,opacity,probe);
 cutout(e,x,y,motion,outline,nullptr,false,opacity);
 ashes(e,x,y,motion.pile,outline,nullptr,opacity);
 sparks(e,x,y,normalizedTime,motion.cloud*opacity);
 fallingBasalt(e,x,y,normalizedTime,opacity);
 return false;
}
}
