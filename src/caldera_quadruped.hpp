#pragma once

#include "generated/caldera_quadruped_curves.hpp"

namespace caldera_quadruped {

using namespace caldera_quadruped_curves;

inline bool handles(int type){return type==EmberDrakeType||type==SlagCrawlerType;}
inline float hoverAbove(int type){return type==EmberDrakeType?126.f:69.f;}
inline constexpr float HoverBelow=34.f;
inline Texture2D grain{},drakeGrain{};
inline void loadGrain(Texture2D& atlas,const char* path){
 if(atlas.id||!FileExists(path))return;
 atlas=LoadTexture(path);
 if(!atlas.id)return;
 GenTextureMipmaps(&atlas);
 SetTextureFilter(atlas,TEXTURE_FILTER_TRILINEAR);
}
inline void load(){
 loadGrain(grain,"assets/volcano/quadruped-anatomy.png");
 loadGrain(drakeGrain,"assets/volcano/ember-drake-hd.png");
 if(drakeGrain.id)SetTextureFilter(drakeGrain,TEXTURE_FILTER_ANISOTROPIC_4X);
}
inline void unload(){
 if(drakeGrain.id)UnloadTexture(drakeGrain);
 if(grain.id)UnloadTexture(grain);
 drakeGrain=grain={};
}
inline Texture2D& materialAtlas(bool drake){return drake&&drakeGrain.id?drakeGrain:grain;}

inline float mix(float a,float b,float t){return a+(b-a)*t;}
inline Vector2 add(Vector2 a,Vector2 b){return {a.x+b.x,a.y+b.y};}
inline Vector2 sub(Vector2 a,Vector2 b){return {a.x-b.x,a.y-b.y};}
inline Vector2 mul(Vector2 p,float s){return {p.x*s,p.y*s};}
inline Vector2 mix(Vector2 a,Vector2 b,float t){return add(a,mul(sub(b,a),t));}
inline float cross(Vector2 a,Vector2 b){return a.x*b.y-a.y*b.x;}
inline float length(Vector2 v){return std::hypot(v.x,v.y);}

inline Color light(Color color,float value){
 const auto shade=[value](unsigned char c){return static_cast<unsigned char>(std::clamp(float(c)*value,0.f,255.f));};
 return {shade(color.r),shade(color.g),shade(color.b),color.a};
}
template<size_t N> inline float sample(const Key (&curve)[N],float phase){
 const float t=std::clamp(phase,0.f,1.f);
 const auto* next=std::upper_bound(curve+1,curve+N-1,t,[](float at,const Key& key){return at<key.time;});
 const auto& a=next[-1];const auto& b=next[0];
 return mix(a.value,b.value,(t-a.time)/(b.time-a.time));
}
inline float loopPhase(float phase){return phase-std::floor(phase);}

// Rotate two ground-plane axes, then place the drawn joint at its screen height.
// Every stroke attached to a joint receives precisely the same coordinates.
struct Pose {
 bool dragon;
 ActorFacing axes;
 float gait,pace,bob,advance,jaw,fall,fold,flank;
 float tilt,pitch;
 Vector2 at(float forward,float side,float rise)const{
  const float lateral=side+flank*fall*(dragon?8.f:5.f);
  return {axes.x*forward-axes.y*lateral,
          ActorGroundSquash*(axes.y*forward+axes.x*lateral)-rise};
 }
 Vector2 mount(float forward,float side,float rise)const{
  const float compact=mix(1.f,dragon?.52f:.62f,fall);
  const float above=rise*compact+(bob+side*tilt+forward*pitch)*(1-fall);
  return at(forward+advance*(1-fall),side,above);
 }
 float lightOn(float side)const{return .86f+.14f*axes.x*side;}
 Vector2 hinge()const{return mount(dragon?34.f:27.f,0,dragon?53.f:20.f);}
 Vector2 nose()const{
  const float reach=dragon?45.f:36.f;
  return mount(reach+jaw*(dragon?1.5f:.7f),0,(dragon?51.f:18.f)+jaw*2.4f);
 }
 Vector2 lowerJaw()const{
  const float reach=dragon?44.f:35.f;
  return mount(reach-jaw*1.7f,0,(dragon?46.f:15.f)-jaw*(dragon?9.f:4.f));
 }
 Vector2 fireSocket()const{return mix(nose(),lowerJaw(),.38f);}
};

inline Pose pose(int type,const AnimationState& state,float attack,float breath,float dying=0,float side=0){
 constexpr float angles[]={0,.4636476f,PI*.5f,2.677945f,PI,3.6052403f,PI*1.5f,5.8195377f};
 const bool dragon=type==EmberDrakeType;
 const float angle=state.facingInitialized?state.facingAngle:angles[std::clamp(int(state.direction),0,7)];
 const auto axes=actorFacing(angle);
 const float breathClock=loopPhase(state.idleTime*.45f);
 const float walk=state.moveBlend;
 const float gait=loopPhase(state.phase);
 const float idle=dragon?sample(drake::idle,breathClock):sample(crawler::idle,breathClock);
 const float step=dragon?sample(drake::weight,gait):sample(crawler::weight,gait);
 const float fall=dragon?sample(drake::fall,dying):sample(crawler::fall,dying);
 const float fold=dragon?sample(drake::fold,dying):sample(crawler::fold,dying);
 float impulse=0,jaw=0;
 if(dragon&&breath>=0){impulse=sample(drake::breath,breath);jaw=sample(drake::breathJaw,breath);}
 else if(attack>=0){
  impulse=dragon?sample(drake::bite,attack):sample(crawler::bite,attack);
  jaw=dragon?sample(drake::biteJaw,attack):sample(crawler::biteJaw,attack);
 }
 const float lead=dragon?sample(drake::lead,gait)*walk:0.f;
 const float roll=dragon?sample(drake::roll,gait)*walk:0.f;
 const float pitch=dragon?sample(drake::pitch,gait)*walk:0.f;
 return {dragon,axes,gait,walk,mix(idle,step,walk)-std::max(0.f,-impulse)*2.1f,
         impulse*(dragon?9.f:7.f)+lead,jaw*(1-fall),fall,fold,side,state.turnLean*(dragon?.022f:.015f)+roll,pitch};
}

struct Leg {Vector2 socket,knee,ankle,toe;};
inline Leg leg(const Pose& rig,int axle,int flank){
 const bool fore=axle>0;
 const float timing=rig.dragon?rig.gait+(flank>0?0.f:.5f)+(fore?0.f:.23f)
                              :rig.gait+(axle==flank?0.f:.5f)+(fore?0.f:.08f);
 const float phase=loopPhase(timing);
 const float motion=rig.pace*(1-rig.fold);
 const float stride=(rig.dragon?(fore?sample(drake::foreReach,phase):sample(drake::hindReach,phase))
                              :sample(crawler::reach,phase))*motion;
 const float loft=(rig.dragon?(fore?sample(drake::foreLift,phase):sample(drake::hindLift,phase))
                             :sample(crawler::lift,phase))*motion;
 const float curl=rig.dragon?(fore?sample(drake::foreCurl,phase):sample(drake::hindCurl,phase))*motion:0.f;
 const float base=fore?(rig.dragon?17.f:12.f):(rig.dragon?-17.f:-12.5f);
 const float width=rig.dragon?13.f:10.7f,outward=width+(rig.dragon?4.f:2.5f)+rig.fold*(rig.dragon?9.f:5.f);
 const Vector2 socket=rig.mount(base,flank*(width-1),rig.dragon?29.f:16.f);
 const float bend=rig.dragon?loft*(fore?.38f:-.28f):0.f;
 const Vector2 knee=rig.mount(base+(fore?-4.f:5.f)+stride*(rig.dragon?.39f:.26f)+bend,flank*(width+3),
                              rig.dragon?17.f+loft*(fore?.51f:.62f):9.f);
 const float footWidth=outward+(rig.dragon?loft*.17f:0.f);
 const Vector2 ankle=rig.at(base+stride+(fore?1.f:-3.f),flank*footWidth,loft+(rig.dragon?3.f+.61f*curl:4.f));
 const Vector2 toe=rig.at(base+stride+(fore?(rig.dragon?6.f:3.f):1.f)-.52f*curl,flank*footWidth,
                          loft+(rig.dragon?1.f-.39f*curl:2.4f));
 return {socket,knee,ankle,toe};
}

// The indices follow the eight painted regions in the Krita atlas.
enum Material {DrakeScale,Membrane,DrakeFace,DrakeSkin,Plates,CrawlerFace,CrawlerSkin,Ivory,Paint};
inline Color textureLight(Color authored,Material material){
 constexpr float expectedLight[]={82.f,127.f,81.f,78.f,92.f,70.f,73.f,177.f};
 const float value=.24f*authored.r+.63f*authored.g+.13f*authored.b;
 const auto brightness=static_cast<unsigned char>(std::clamp(21.f+213.f*value/expectedLight[material],24.f,255.f));
 return {brightness,brightness,brightness,authored.a};
}

inline Vector2 surfaceUV(Vector2 uv,ActorFacing paintAxes){
 const Vector2 d{uv.x-.5f,uv.y-.5f};
 return { .5f+d.x*paintAxes.x+d.y*paintAxes.y,.5f-d.x*paintAxes.y+d.y*paintAxes.x};
}

struct Vertex {Vector2 point,uv;Color color;};
struct Triangle {Vertex a,b,c;Material material;};
struct Joint {Vector2 point;float radius;};
struct Mesh {
 std::vector<Triangle> triangles;
 void triangle(Vertex a,Vertex b,Vertex c,Material material){
  const float area=cross(sub(b.point,a.point),sub(c.point,a.point));
  if(std::abs(area)<.005f)return;
  if(area>0)std::swap(b,c);
  triangles.push_back({a,b,c,material});
 }
 void mark(Vector2 a,Vector2 b,Vector2 c,Color x,Color y,Color z,Material material=Paint){
  triangle({a,{.5f,0},x},{b,{0,1},y},{c,{1,1},z},material);
 }
 void ellipse(Vector2 center,Vector2 major,Vector2 minor,Color color,Material material,unsigned detail=28,ActorFacing paintAxes={1,0}){
  const float radius=std::max(length(major),length(minor));
  const Vertex interior{add(center,add(mul(major,-.12f),mul(minor,-.22f))),surfaceUV({.44f,.39f},paintAxes),light(color,1.27f)};
  const auto point=[&](unsigned i){
   const float angle=2*PI*float(i%detail)/detail;
   const float c=std::cos(angle),s=std::sin(angle);
   const float illumination=.79f-.22f*s-.12f*c;
   return Vertex{add(center,add(mul(major,c),mul(minor,s))),surfaceUV({.5f+.49f*c,.5f+.49f*s},paintAxes),light(color,illumination)};
  };
  auto a=point(0);
  for(unsigned i=1;i<=detail;i++){
   const auto b=point(i);
   triangle(interior,a,b,material);
   if(radius>=5){
    auto rim=[&](Vertex v){
     const auto radial=sub(v.point,center);
     v.point=add(v.point,mul(radial,.65f/std::max(1.f,length(radial))));
     v.color.a=0;
     return v;
    };
    auto aa=rim(a),bb=rim(b);
    if(bb.color.a!=b.color.a){triangle(a,aa,b,material);triangle(b,aa,bb,material);}
   }
   a=b;
  }
 }
 void oval(Vector2 center,float width,float height,Color color,Material material=Paint,unsigned detail=24){
  ellipse(center,{width,0},{0,height},color,material,detail);
 }
 template<class Project> void sculpt(const Project& project,Color color,Material material,ActorFacing paintAxes,int steps=36){
  const auto at=[&](Vector2 unit,float lightness){
   return Vertex{project(unit),surfaceUV({.5f+.478f*unit.x,.5f+.478f*unit.y},paintAxes),light(color,lightness)};
  };
  const auto middle=at({-.16f,-.28f},1.19f);
  const auto center=project({0,0});
  const auto contour=[&](int segment){
   const float a=2*PI*(segment%steps)/steps;
   const Vector2 local{std::cos(a),std::sin(a)};
   return at(local,.80f-.21f*local.y-.095f*local.x);
  };
  const auto feather=[&](Vertex v){
   const auto direction=sub(v.point,center);
   v.point=add(v.point,mul(direction,.62f/std::max(1.f,length(direction))));
   v.color.a=0;
   return v;
  };
  auto previous=contour(0);
  for(int i=1;i<=steps;i++){
   const auto next=contour(i);
   triangle(middle,previous,next,material);
   triangle(previous,feather(previous),next,material);
   triangle(next,feather(previous),feather(next),material);
   previous=next;
  }
 }

 // A single strip of skin runs through all of the existing joint centers.
 // UVs continue over the bend, keeping muscle and grain on the same limb.
 template<size_t N> void ribbon(const std::array<Joint,N>& joints,Color paint,Material material){
  const auto section=[&](size_t i){
   const auto& joint=joints[i];
   const auto tangent=sub(joints[std::min(i+1,N-1)].point,joints[i?i-1:0].point);
   const float norm=std::max(.08f,length(tangent));
   const Vector2 normal{-tangent.y/norm,tangent.x/norm};
   const float along=float(i)/(N-1);
   const float lightSide=.18f*(.45f*normal.x+.84f*normal.y);
   const auto gain=[&](float value){return light(paint,value*(1-.12f*along));};
   return std::array<Vertex,3>{{
    {add(joint.point,mul(normal,joint.radius)),{along,.03f},gain(.77f-lightSide)},
    {joint.point,{along,.48f},gain(1.15f)},
    {sub(joint.point,mul(normal,joint.radius)),{along,.97f},gain(.76f+lightSide)}
   }};
  };
  auto a=section(0);
  for(size_t i=1;i<N;i++){
   const auto b=section(i);
   triangle(a[0],a[1],b[0],material);triangle(b[0],a[1],b[1],material);
   triangle(a[1],a[2],b[2],material);triangle(a[1],b[2],b[1],material);
   a=b;
  }
 }

 void bone(Vector2 a,Vector2 b,float first,float last,Color pigment,Material material=Paint){
  const auto direction=sub(b,a);
  const float span=length(direction);
  if(span<.05f)return;
  const Vector2 tangent=mul(direction,1/span);
  const Vector2 width{-tangent.y,tangent.x};
  const Color shade=light(pigment,.66f),glint=light(pigment,1.16f);
  const Vertex ap{add(a,mul(width,first)),{0,0},shade},an{sub(a,mul(width,first)),{0,1},glint};
  const Vertex bp{add(b,mul(width,last)),{1,0},shade},bn{sub(b,mul(width,last)),{1,1},pigment};
  const Vertex centerA{a,{0,.52f},pigment},centerB{b,{1,.52f},pigment};
  triangle(ap,centerA,bp,material);triangle(bp,centerA,centerB,material);
  triangle(centerA,an,bn,material);triangle(centerA,bn,centerB,material);
 }
 bool includes(Vector2 q)const{
  for(const auto& t:triangles){
   if(t.a.color.a<250||t.b.color.a<250||t.c.color.a<250)continue;
   if(cross(sub(t.b.point,t.a.point),sub(q,t.a.point))<=0 &&
      cross(sub(t.c.point,t.b.point),sub(q,t.b.point))<=0 &&
      cross(sub(t.a.point,t.c.point),sub(q,t.c.point))<=0)return true;
  }
  return false;
 }
 void paint(Texture2D atlas,Color tint,float opacity,bool solid,Vector2 offset={})const{
  const bool textured=atlas.id&&!solid;
  const unsigned texture=textured?atlas.id:rlGetTextureIdDefault();
  rlCheckRenderBatchLimit(static_cast<int>(triangles.size()*4)+4);
  // Submit each triangle as a quad with a degenerate second triangle. This
  // keeps the actor, its flash and the following effect in the sprite batch.
  rlBegin(RL_QUADS);rlSetTexture(texture);
  const auto emit=[&](Vertex v,Material material){
   const Color c=solid?tint:textured&&material!=Paint?textureLight(v.color,material):v.color;
   const float coverage=solid?v.color.a/255.f:1.f;
   rlColor4ub(c.r,c.g,c.b,static_cast<unsigned char>(c.a*opacity*coverage+.5f));
   if(textured){
    if(material==Paint){
     const bool hd=atlas.id==drakeGrain.id;
     rlTexCoord2f(hd?.375f:4.f/1024,hd?.75f:4.f/512);
    }
    else rlTexCoord2f((material%4+.035f+.93f*v.uv.x)/4,(material/4+.035f+.93f*v.uv.y)/2);
   }else rlTexCoord2f(0,0);
   rlVertex2f(v.point.x+offset.x,v.point.y+offset.y);
  };
  for(const auto& t:triangles){emit(t.a,t.material);emit(t.b,t.material);emit(t.c,t.material);emit(t.c,t.material);}
  rlEnd();
 }
};

inline constexpr Color iron{79,94,111,255},ironEdge{137,151,154,255},ash{78,70,65,255};
inline constexpr Color dragonScales{82,83,99,255},gold{199,119,68,255};
inline constexpr Color talon{192,159,111,255},boneColor{219,200,159,255},burn{244,133,40,255};

inline void drawLeg(Mesh& mesh,const Pose& rig,int axle,int flank){
 const Leg paw=leg(rig,axle,flank);
 const float lighting=rig.lightOn(float(flank));
 const float thick=rig.dragon?1.08f:1.f;
 const Color skin=light(rig.dragon?dragonScales:ash,lighting);
 const auto hide=rig.dragon?DrakeSkin:CrawlerSkin;
 const float upper=rig.dragon?(axle<0?8.4f:7.1f):6.3f;
 const std::array<Joint,7> limb{{
  {paw.socket,upper*thick*.76f}, {mix(paw.socket,paw.knee,.32f),upper*thick},
  {mix(paw.socket,paw.knee,.71f),5.4f*thick}, {paw.knee,4.2f*thick},
  {mix(paw.knee,paw.ankle,.52f),3.55f*thick}, {paw.ankle,3.1f*thick}, {paw.toe,4.0f*thick}
 }};
 mesh.ribbon(limb,skin,hide);
 mesh.oval(paw.knee,4.5f*thick,4.0f*thick,light(skin,1.08f),hide,14);
 mesh.oval(paw.toe,5.0f*thick,2.9f*thick,skin,hide,18);
 const Vector2 forward{rig.axes.x,rig.axes.y*ActorGroundSquash};
 const Vector2 across{-rig.axes.y,rig.axes.x*ActorGroundSquash};
 for(float digit:{-1.f,0.f,1.f}){
  const auto root=add(paw.toe,mul(across,digit*3.15f*thick));
  const float reach=(rig.dragon?3.1f:2.7f)+.6f*(1-std::abs(digit));
  const auto bend=add(root,add(mul(forward,reach*.67f),{0,-.52f}));
  const auto tip=add(root,add(mul(forward,reach),{0,.56f}));
  mesh.ribbon(std::array<Joint,3>{{{root,1.31f},{bend,.94f},{tip,.08f}}},light(talon,lighting*.94f),Ivory);
 }
}

inline void drawTail(Mesh& mesh,const Pose& rig){
 const float swing=rig.dragon?0.f:std::sin(2*PI*rig.gait-.8f)*rig.pace;
 const float longTail=rig.dragon?37.f:25.f;
 std::array<Joint,10> tail;
 tail[0]={rig.mount(-21,0,rig.dragon?29.f:18.f),rig.dragon?9.f:7.3f};
 for(int link=1;link<=9;link++){
  const float t=link/9.f;
  const float lagged=rig.dragon?sample(drake::tailSwing,loopPhase(rig.gait-.27f*t))*rig.pace:0.f;
  const float tailMotion=rig.dragon?7.5f*lagged*t*t:4.f*swing*t;
  const float arc=(rig.dragon?18.f:9.f)*t*t+tailMotion+rig.fold*rig.flank*10*t;
  const float lift=(rig.dragon?28.f:17.f)*std::pow(1-t,1.7f)+2*t;
  const auto p=rig.mount(-21-t*longTail,arc,lift);
  const float width=mix(rig.dragon?9.f:7.3f,.58f,t*t*.37f+t*.63f);
  tail[link]={p,width};
 }
 const auto skin=rig.dragon?DrakeSkin:CrawlerSkin;
 const auto scales=light(rig.dragon?dragonScales:iron,.92f);
 mesh.ribbon(tail,scales,skin);
 for(int i=1;i<9;i++){
  const auto& joint=tail[i];
  const float y=joint.radius*.25f;
  const auto crease=add(mix(tail[i-1].point,joint.point,.82f),{0,-y});
  mesh.bone(crease,add(mix(joint.point,tail[i+1].point,.31f),{0,-y*.64f}),
            joint.radius*.47f,joint.radius*.13f,light(scales,1.16f),skin);
 }
}

inline void drawWing(Mesh& mesh,const Pose& rig,int side){
 const float fold=rig.fold;
 const float flex=rig.pace*sample(drake::wingYield,loopPhase(rig.gait+side*.12f));
 const auto socket=rig.mount(11,side*9.f,42);
 const auto at=[&](float span,float chord){
  const float x=11-21*span-45*chord-fold*4*chord+flex*.44f*span*span;
  const float y=side*(9+mix(37.f,45.f,fold)*span+18*chord);
  const float z=42+mix(40.f,13.f,fold)*span-31*chord+17*span*chord+flex*span;
  return rig.mount(x,y,z);
 };
 const float shade=rig.lightOn(float(side));
 const Color leather=light({189,100,64,255},shade);
 const Color sinew=light({160,105,69,255},shade);
 const Vector2 wristUV{.49f,.055f};
 const auto skin=[&](Vector2 p,float value){
  return Vertex{at(p.x,p.y),{std::clamp(p.x*.94f,0.f,1.f),std::clamp((p.y+.13f)/1.17f,0.f,1.f)},light(leather,value)};
 };
 const Vertex wrist=skin(wristUV,.73f);
 constexpr std::array<Vector2,5> fingertip{{{1.07f,.158f},{.88f,.54f},{.61f,.855f},{.318f,.985f},{.067f,.685f}}};
 const auto curve=[&](Vector2 start,Vector2 guide,Vector2 end,int steps,bool leading=false){
  auto a=skin(start,.99f-.15f*start.y);
  for(int i=1;i<=steps;i++){
   const float t=float(i)/steps;
   const auto p=mix(mix(start,guide,t),mix(guide,end,t),t);
   const auto b=skin(p,1.02f-.18f*p.y);
   mesh.triangle(wrist,a,b,Membrane);
   if(leading)mesh.bone(a.point,b.point,.71f,.71f,light(sinew,.88f),Membrane);
   a=b;
  }
 };
 curve({0,0},{.58f,-.34f},fingertip[0],10,true);
 for(size_t i=1;i<fingertip.size();i++){
  const auto left=fingertip[i-1],right=fingertip[i];
  const auto hollow=mix(mix(left,right,.53f),wristUV,.37f+.035f*float(i));
  curve(left,hollow,right,7);
 }
 curve(fingertip.back(),{.05f,.26f},{0,0},5);
 for(size_t i=0;i<fingertip.size();i++){
  const auto tip=fingertip[i];
  const auto joint=mix(wristUV,tip,.60f);
  const float ridge=.019f*float(i+1);
  const auto bend=at(joint.x+ridge,joint.y-.033f);
  mesh.ribbon(std::array<Joint,3>{{{wrist.point,1.76f-.14f*i},{bend,.98f-.073f*i},{at(tip.x,tip.y),.10f}}},sinew,Membrane);
 }
 const auto elbow=at(.22f,-.045f);
 mesh.ribbon(std::array<Joint,3>{{{socket,4.9f},{elbow,3.8f},{wrist.point,2.4f}}},light(dragonScales,shade),DrakeSkin);
 mesh.ribbon(std::array<Joint,3>{{{wrist.point,1.5f},{at(.575f,-.102f),.91f},{at(.608f,-.117f),.08f}}},light(boneColor,.72f*shade),Ivory);
}

struct TorsoSection { float forward, halfWidth, back, flank, belly, sweep=0; };

// Longitudinal silhouettes, from the tail root to the neck. The drake's
// abdomen tucks between the chest and hips. The crawler has a low crown and
// a wide lower rim, with the soft underside tucked inside the shell.
inline constexpr std::array<TorsoSection,15> drakeTorso{{
 {-33,0,30,30,30}, {-28,8,39,33,25}, {-22,12.8f,43,34,24}, {-15,12.5f,44,35,26},
 {-8,10.4f,44.5f,37,30}, {-1,12.0f,47,36,27}, {7,16.7f,51,35,22},
 {16,18.8f,55,34,19}, {21,17.2f,55,36,21,1.4f}, {25.5f,14.3f,56,39,24,2.7f},
 {29.4f,11.2f,57.4f,44,29.8f,3.5f}, {31.7f,8.7f,60.1f,49.3f,36.5f,4.0f},
 {33.3f,6.4f,62,53.6f,44.8f,3.5f}, {35.3f,4.2f,60.2f,54.3f,48.6f,2.2f},
 {38,0,53,53,53}
}};
inline constexpr std::array<TorsoSection,12> crawlerTorso{{
 {-37,0,19,19,19}, {-34,6,25,19,16}, {-30,11,30,20,14}, {-24,15.5f,32,20,13.5f},
 {-16,17.1f,34,20.5f,13}, {-8,17.5f,34.5f,20.5f,12.5f}, {1,17.6f,34.1f,20,12},
 {10,17.3f,33.2f,19.5f,11.8f}, {18,16.3f,31.7f,18.8f,12},
 {24,13.7f,29,18,13}, {29,9,24,17.5f,14.5f}, {32,0,19,19,19}
}};

template<size_t N> void drawTorso(Mesh& mesh,const Pose& rig,const std::array<TorsoSection,N>& profile){
 struct Point { Vertex vertex; float order; };
 struct Panel { Triangle triangle; float order; };
 constexpr std::array<float,8> drakeAcross{0,.65f,1,.73f,0,-.73f,-1,-.65f};
 constexpr std::array<float,8> shellAcross{0,.73f,1,.80f,0,-.80f,-1,-.73f};
 constexpr std::array<float,8> drakeLight{1.13f,1.23f,.90f,.71f,.60f,.66f,.85f,1.03f};
 constexpr std::array<float,8> shellLight{1.04f,1.15f,.77f,.66f,.53f,.62f,.76f,.99f};
 const auto& across=rig.dragon?drakeAcross:shellAcross;
 const auto& lighting=rig.dragon?drakeLight:shellLight;
 const size_t bands=across.size();
 const float roll=rig.dragon?rig.fold:0.f;
 std::array<Panel,(N-1)*48> panels;
 size_t count=0;

 const auto ring=[&](const TorsoSection& s){
  const float shoulder=mix(s.flank,s.back,rig.dragon?.69f:.79f);
  const float lower=mix(s.belly,s.flank,rig.dragon?.28f:.29f);
  const std::array<float,8> rises{s.back,shoulder,s.flank,lower,s.belly,lower,s.flank,shoulder};
  std::array<Point,8> points;
  for(size_t i=0;i<bands;i++){
   const float side=across[i]*s.halfWidth;
   const float arc=float(std::min(i,bands-i));
   // The throat curves forward from the chest as the nape rises towards the
   // skull. Its scales still use the same material and coordinates as the torso.
   const float tilt=s.sweep*(.5f*arc-1.f);
   const float f=s.forward+tilt-(rig.dragon?1.45f:3.35f)*side*side/(18.f*18.f);
   // On the drake's flank, the rib cage spreads under the folded wing while
   // the underside stays at the same height. Its volume survives the fall.
   const float spread=side*(1+.29f*roll);
   const float rise=rises[i]+(rises[i]-s.belly)*.30f*roll;
   const float depth=f*rig.axes.y+spread*rig.axes.x;
   const float wrap=arc*.215f+.073f;
   const float materialForward=rig.dragon&&s.forward>16.f?mix(16.f,34.f,(s.forward-16.f)/22.f):s.forward;
   const float u=.07f+.86f*(materialForward+37.f)/71.f;
   points[i]={{rig.mount(f,spread,rise),{u,wrap},WHITE},
              depth+ActorGroundSquash*rise};
  }
  return points;
 };
 const auto panel=[&](Point a,Point b,Point c,Material skin){
  // The strips are wound in the same order at every heading. Only the visible
  // side of each strip contributes to the color and picking silhouette.
  if(cross(sub(b.vertex.point,a.vertex.point),sub(c.vertex.point,a.vertex.point))<=.005f)return;
  panels[count++]={{a.vertex,b.vertex,c.vertex,skin},(a.order+b.order+c.order)/3.f};
 };
 const auto between=[](Point a,Point b,float t){
  return Point{{mix(a.vertex.point,b.vertex.point,t),mix(a.vertex.uv,b.vertex.uv,t),WHITE},mix(a.order,b.order,t)};
 };
 if(rig.dragon){
  for(size_t i=1;i<=7;i++){
   const auto& s=profile[i];
   const float h=(2.9f+.41f*i)*(1-.36f*rig.fold);
   const auto start=rig.mount(s.forward+1.4f,0,s.back-1.5f);
   const auto end=rig.mount(s.forward-2.6f,0,s.back-1.5f);
   const auto tip=rig.mount(s.forward-2.0f,0,s.back+h);
   mesh.mark(start,tip,end,light(dragonScales,1.03f),light(gold,.76f),light(dragonScales,.69f),DrakeFace);
  }
 }
 auto previous=ring(profile.front());
 for(size_t section=1;section<N;section++){
  const auto next=ring(profile[section]);
  for(size_t row=0;row<bands;row++){
   const size_t other=(row+1)%bands;
   const bool belly=row>=2&&row<6;
   const auto material=rig.dragon?(belly?DrakeSkin:DrakeScale):(belly?CrawlerSkin:Plates);
   const auto pigment=rig.dragon?dragonScales:(belly?ash:iron);
   const auto a=previous[row],b=next[row],c=next[other],d=previous[other];
   const auto paint=[&](Point p,size_t index,float shade){
    p.vertex.color=light(pigment,lighting[index]*shade);
    return p;
   };
   const auto strip=[&](Point startA,Point endA,Point endB,Point startB,float start,float end){
    startA=paint(startA,row,start);endA=paint(endA,row,end);
    endB=paint(endB,other,end);startB=paint(startB,other,start);
    panel(startA,endA,endB,material);
    panel(startA,endB,startB,material);
   };
   if(!rig.dragon&&!belly&&section>=3&&section<=9){
    // Overlapping shell plates cast a narrow seam. The groove bends back at
    // the lower rim and turns with the carapace, as does the scale pattern.
    const auto seam=[&](size_t t){
     constexpr std::array<float,7> starts{.78f,.70f,.82f,.74f,.76f,.72f,.80f};
     return starts[section-3]-.057f*float(std::min(t,bands-t));
    };
    const float u=seam(row),v=seam(other);
    const auto lipA=between(a,b,u),lipB=between(d,c,v);
    const auto gapA=between(a,b,u+.11f),gapB=between(d,c,v+.11f);
    strip(a,lipA,lipB,d,.90f,1.13f);
    strip(lipA,gapA,gapB,lipB,.76f,.58f);
    strip(gapA,b,c,gapB,.67f,.91f);
   }else{
    const auto bodyValue=[&](size_t i){
     if(!rig.dragon)return .94f;
     const float at=profile[i].forward;
     if(belly&&at<4&&at>-15)return .84f;
     if(!belly&&at>=7&&at<=23)return 1.04f;
     return .97f;
    };
    strip(a,b,c,d,bodyValue(section-1),bodyValue(section));
   }
  }
  previous=next;
 }
 std::sort(panels.begin(),panels.begin()+count,[](const Panel& a,const Panel& b){return a.order<b.order;});
 for(size_t i=0;i<count;i++){
  const auto& t=panels[i].triangle;
  mesh.triangle(t.a,t.b,t.c,t.material);
 }
}

inline void drawBody(Mesh& mesh,const Pose& rig){
 if(rig.dragon)drawTorso(mesh,rig,drakeTorso);
 else drawTorso(mesh,rig,crawlerTorso);
}

inline void drawHead(Mesh& mesh,const Pose& rig){
 const bool dragon=rig.dragon;
 const auto crown=rig.mount(dragon?32.f:26.f,0,dragon?58.f:23.f);
 const auto nose=rig.nose(),jaw=rig.lowerJaw(),hinge=rig.hinge();
 const Color outer=dragon?dragonScales:ash;
 const auto face=dragon?DrakeFace:CrawlerFace;
 if(!dragon){
  mesh.ribbon(std::array<Joint,4>{{{rig.mount(17,0,21),8.6f},{rig.mount(20,0,22),7.8f},
                                   {rig.mount(24,0,22),6.35f},{crown,6.0f}}},outer,face);
 }
 const float width=(dragon?9.7f:8.6f)+.55f*std::abs(rig.axes.y);
 const auto skull=[&](Vector2 p){
  const float front=p.x*rig.axes.x+.48f*p.y*rig.axes.y;
  const float y=p.y*(.87f-.16f*front)-(p.y<0?.22f:0.f)*front*.9f;
  return add(crown,{width*p.x*(1-.08f*front),y*(dragon?8.1f:6.9f)});
 };
 mesh.sculpt(skull,light(outer,1.06f),face,rig.axes,28);
 if(rig.jaw>.001f){
  mesh.mark(hinge,nose,jaw,{32,18,21,255},{81,34,28,255},{133,64,45,255});
  const auto root=mix(hinge,jaw,.53f);
  mesh.ribbon(std::array<Joint,3>{{{hinge,dragon?4.8f:4.f},{root,dragon?4.1f:3.25f},{jaw,dragon?3.1f:2.65f}}},light(outer,.83f),face);
 }
 const auto bridge=add(mix(hinge,nose,.53f),{0,dragon?-1.3f:-.55f});
 mesh.ribbon(std::array<Joint,3>{{{hinge,dragon?7.2f:5.9f},{bridge,dragon?5.15f:5.05f},{nose,dragon?3.65f:3.3f}}},outer,face);
 mesh.oval(nose,dragon?4.2f:4.5f,dragon?2.8f:2.7f,light(outer,.76f),face,22);
 const auto ridge=rig.mount(dragon?36.f:30.f,0,dragon?59.6f:23.5f);
 mesh.ribbon(std::array<Joint,3>{{{ridge,dragon?3.4f:2.75f},{add(bridge,{0,-1.65f}),dragon?2.5f:2.15f},
                                  {{nose.x,nose.y-1.9f},dragon?1.75f:1.6f}}},light(outer,1.25f),face);
 if(dragon&&rig.jaw>.09f){
  const float show=std::clamp((rig.jaw-.09f)*1.7f,0.f,1.f);
  for(float t:{.43f,.70f,.88f}){
   const auto tooth=add(mix(hinge,nose,t),{0,2.05f});
   mesh.mark({tooth.x-1.f,tooth.y},{tooth.x+1.1f,tooth.y},
             {tooth.x+.28f*rig.axes.x,tooth.y+(2.45f-.71f*t)*show},
             light(boneColor,.80f),light(boneColor,1.09f),light(boneColor,.71f),Ivory);
  }
 }
 for(int side:{-1,1}){
  if(dragon){
   const auto root=rig.mount(30.5f,side*6.3f,61.3f);
   const auto shoulder=rig.mount(27.f,side*8.1f,67.2f);
   const auto crook=rig.mount(22.4f,side*10.6f,71.1f);
   const auto tip=rig.mount(17.3f,side*13.1f,72.6f);
   mesh.ribbon(std::array<Joint,4>{{{root,3.15f},{shoulder,2.27f},{crook,1.26f},{tip,.08f}}},light(boneColor,.89f),Ivory);
   const auto cheek=rig.mount(29.5f,side*6.8f,54.9f);
   const auto cheekTip=rig.mount(25.1f,side*10.5f,56.6f);
   mesh.bone(cheek,cheekTip,1.95f,.22f,light(outer,1.15f),face);
  }else{
   const auto root=rig.mount(31,side*4.8f,16),bend=rig.mount(35,side*6.2f,12.3f);
   const auto tip=rig.mount(39.2f,side*7.0f,15.75f);
   mesh.ribbon(std::array<Joint,3>{{{root,2.35f},{bend,1.17f},{tip,.13f}}},light(boneColor,.86f),Ivory);
  }
  const float sideVisibility=rig.axes.x*side;
  const float visible=std::clamp((sideVisibility+.60f)/.23f,0.f,1.f)*std::clamp((rig.axes.y+.76f)/.22f,0.f,1.f);
  if(visible<.01f)continue;
  const auto eye=rig.mount(dragon?35.f:29.f,side*(dragon?7.0f:5.65f),dragon?57.5f:22.0f);
  const float eyeWidth=1.17f+std::max(0.f,sideVisibility)*.79f;
  const Vector2 eyeAxis{(eyeWidth+.83f)*visible,-.40f*rig.axes.x*visible};
  if(dragon){
   const auto brow=add(eye,{-.16f*rig.axes.x,-1.3f*visible});
   mesh.ellipse(brow,mul(eyeAxis,.92f),{0,.45f*visible},{151,111,77,255},Paint,16);
   const auto nostril=add(nose,{-side*1.75f*rig.axes.y,-1.02f});
   mesh.ellipse(nostril,mul(eyeAxis,.45f),{0,.54f*visible},{51,32,26,255},Paint,12);
  }
  mesh.ellipse(eye,eyeAxis,{0,1.25f*visible},{25,19,23,255},Paint,16);
  mesh.ellipse(eye,mul(eyeAxis,.65f),{0,.88f*visible},dragon?burn:gold,Paint,14);
  mesh.oval({eye.x+.19f*visible,eye.y-.08f*visible},.30f*visible,.71f*visible,{49,27,18,255},Paint,12);
  mesh.oval({eye.x+.29f*visible,eye.y-.39f*visible},.30f*visible,.24f*visible,{245,215,143,255},Paint,8);
 }
}

inline void build(Mesh& mesh,const Pose& rig){
 const int near=rig.axes.x>=0?1:-1;
 drawTail(mesh,rig);
 if(rig.dragon){drawWing(mesh,rig,-near);drawWing(mesh,rig,near);}
 for(int flank:{-near,near}){
  drawLeg(mesh,rig,-1,flank);
  drawLeg(mesh,rig,1,flank);
 }
 if(!rig.dragon)drawHead(mesh,rig);
 drawBody(mesh,rig);
 if(rig.dragon)drawHead(mesh,rig);
}

inline Rectangle footprint(int type,const AnimationState& state,int side){
 const auto rig=pose(type,state,-1,-1,1,float(side));
 const auto anchor=rig.mount(0,0,0);
 const float width=std::hypot(rig.axes.x*(rig.dragon?32.f:25.f),rig.axes.y*(rig.dragon?22.f:20.f));
 return {anchor.x-width,anchor.y-7,width*2,13};
}

inline bool draw(int type,float x,float y,float size,const AnimationState& state,float attack,float breath,float fall,int side,
                 Color outline,const Vector2* probe,bool shadow,bool flash,float opacity){
 thread_local Mesh mesh;
 thread_local std::array<float,13> previous{};
 const auto current=pose(type,state,attack,breath,fall,float(side));
 const std::array<float,13> key{float(type),current.axes.x,current.axes.y,current.gait,current.pace,current.bob,
                                current.advance,current.jaw,current.fall,current.fold,current.flank,current.tilt,current.pitch};
 if(key!=previous){
  mesh.triangles.clear();
  if(mesh.triangles.capacity()<1400)mesh.triangles.reserve(1400);
  build(mesh,current);
  previous=key;
 }
 if(probe)return mesh.includes({(probe->x-x)/size,(probe->y-y)/size});
 rlPushMatrix();
 if(shadow){
  const float ground[]={1,0,0,0,-shadow_math::LightX,-shadow_math::LightY,0,0,0,0,1,0,
                        shadow_math::LightX*y,(1+shadow_math::LightY)*y,0,1};
  rlMultMatrixf(ground);
 }
 rlTranslatef(x,y,0);
 rlScalef(size,size,1);
 const auto atlas=materialAtlas(current.dragon);
 if(!shadow&&outline.a)for(Vector2 offset:{Vector2{-1.45f,0},Vector2{1.45f,0},Vector2{0,-1.45f},Vector2{0,1.45f}})
  mesh.paint(atlas,outline,opacity,true,mul(offset,1/size));
 mesh.paint(atlas,WHITE,opacity,shadow);
 if(flash&&!shadow)mesh.paint(atlas,Fade(WHITE,.44f),opacity,true);
 rlSetTexture(rlGetTextureIdDefault());
 rlPopMatrix();
 return false;
}
}
