#pragma once
#include "enemy_death_settle.hpp"
namespace actor_art {
namespace fallen_quadrupeds {
struct RestLeg {P3 hip,joint,hock,paw;};
inline constexpr std::array<RestLeg,4> WolfLegs{{
 {{-3.5f,-16.5f,17},{-5,-9.1f,11},{-14.8f,-13.6f,5.8f},{-21.8f,-15.2f,3.2f}},
 {{-1.9f,-18.1f,4.1f},{-8,-12.8f,3.6f},{-19.2f,-25.7f,3.1f},{-25.8f,-27.4f,2.6f}},
 {{-1.3f,12.8f,17.2f},{-3.2f,9.1f,10.9f},{-12.2f,19.3f,4.9f},{-18.2f,22.5f,3.3f}},
 {{-.1f,13.8f,4.9f},{-7.1f,12.1f,4.4f},{-19.8f,14.9f,3.1f},{-26.1f,16.7f,2.6f}},
}};
inline constexpr std::array<RestLeg,4> BoarLegs{{
  {{-3.2f,-14.2f,21.4f},{-3.9f,-11.4f,14.6f},{-11.6f,-14.5f,6.9f},{-16.6f,-15.0f,3.1f}},
  {{-2.5f,-15.6f,6.2f},{-8.1f,-10.6f,4.9f},{-14.8f,-16.5f,3.9f},{-21.4f,-17.0f,2.45f}},
  {{-2.5f,13.0f,21.2f},{-3.4f,10.1f,14.7f},{-12.2f,17.1f,5.6f},{-16.4f,20.3f,3.2f}},
  {{-2.9f,13.8f,6.9f},{-8.6f,11.0f,5.5f},{-17.6f,13.2f,3.5f},{-22.2f,14.8f,2.45f}},
}};

inline float interp(float a,float b,float t){return a*(1-t)+b*t;}
inline P3 interp(P3 a,P3 b,float t){return a*(1-t)+b*t;}
inline hero_motion::Vec asVector(P3 p){return {p.r,p.f,p.z};}
inline P3 asPoint(hero_motion::Vec p){return {p.x,p.y,p.z};}
inline P3 onFlank(P3 point,int side){return {point.r*side,point.f,point.z};}
inline RestLeg onFlank(const RestLeg& leg,int side){
 return {onFlank(leg.hip,side),onFlank(leg.joint,side),onFlank(leg.hock,side),onFlank(leg.paw,side)};
}

struct Fall {
 hero_motion::Transform start,rest,body;
 float collapse;
 P3 carried(P3 standing,P3 prone)const{
  using namespace hero_motion;
  const auto a=rotate(conjugate(start.q),asVector(standing)-start.p);
  const auto b=rotate(conjugate(rest.q),asVector(prone)-rest.p);
  return asPoint(body.p+rotate(body.q,a*(1-collapse)+b*collapse));
 }
 hero_motion::Transform head(const hero_motion::Transform& live,hero_motion::Vec neck,float yaw,float dip)const{
  using namespace hero_motion;
  const auto a=mul(conjugate(start.q),live.q);
  const auto b=axisAngle({0,0,1},yaw);
  const auto p=carried(asPoint(live.p),asPoint(rest.p+rotate(rest.q,neck)));
  return {asVector(p),mul(mul(body.q,mix(a,b,collapse)),axisAngle({1,0,0},dip))};
 }
 RestLeg leg(const RestLeg& live,const RestLeg& target,float release,float lift)const{
  // Keep the paw down until that leg bends. After lift-off let its
  // folded joints and paw slide independently toward their rest contacts.
  auto paw=interp(live.paw,target.paw,release);
  paw.z+=lift;
  auto hock=interp(carried(live.hock,target.hock),interp(live.hock,target.hock,release),.72f);
  hock.z+=lift*.73f;
  return {carried(live.hip,target.hip),carried(live.joint,target.joint),hock,paw};
 }
};

inline Fall fall(hero_motion::Transform start,float height,float roll,float t,float pitch,float slide,float bounce,float rollNudge){
 using namespace hero_motion;
 const hero_motion::Transform rest{{0,0,height},axisAngle({0,1,0},roll)};
 const auto q=mul(mul(mix(start.q,rest.q,t),axisAngle({0,1,0},rollNudge)),axisAngle({1,0,0},pitch));
 const auto p=start.p*(1-t)+rest.p*t+Vec{0,slide,bounce};
 return {start,rest,{p,q},t};
}

inline void wolf(Rig& rig,const AnimationState& state,float action,enemy_death::Key key,int side){
 using namespace forest_detail::wolf_art;
 using namespace hero_motion;
 const float t=key.collapse,walk=creatureWalk(state);
 if(t==0&&key.recoil==0){draw(rig,state,walk,action,false);return;}
 const auto live=stance(state,walk,action);
 const float mid=(live.hip+live.chest)*.5f;
 const auto pose=fall({{0,0,mid},{}},11.4f,79.f*side,t,-6.f*key.recoil,-1.5f*key.recoil,
                      1.1f*key.settle-2.4f*key.recoil,4.5f*side*key.recoil);
 const auto head=pose.head({asVector(live.head),{}},{4.55f*side,25.6f,6.2f},-7.f*side,-9.f*key.recoil);
 rig.setGroundBounded();
 rig.setAttachment(&pose.body);
 coat(rig,interp(live.hip-mid,-.6f,t),interp(live.chest-mid,.7f,t),live.weight*(1-t),live.sway*(1-t),false);
 rig.setAttachment(&head);
 face(rig,{},interp(live.pitch,-4.5f,t),live.jaw*(1-t),interp(live.ears,-7.f,t),false,t>.34f);
 rig.setAttachment(nullptr);
 for(std::size_t i=0;i<WolfLegs.size();i++){
  const bool front=i>=2;
  const auto [root,joint,wrist,paw,pitch]=legPose(state,walk,live.hip,live.chest,live.weight,live.action,front,(i%2)!=0);
  // The tucked lower leg and the relaxed upper leg trade roles across the flanks.
  const std::size_t restIndex=side<0?(i^1u):i;
  const float release=ease((t-(front?.04f:.22f))/(front?.83f:.75f));
  const float lift=std::sin(release*AnimationPi)*(restIndex%2?1.5f:3.7f);
  const auto leg=pose.leg({root,joint,wrist,paw},onFlank(WolfLegs[restIndex],side),release,lift);
  const Color shadeFur=shade(Coat,interp(.90f,restIndex%2?.92f:.82f,t));
  rig.tapered(leg.hip,leg.joint,front?7.7f:10.7f,front?8.5f:11.8f,shadeFur,front?.66f:.52f,1);
  rig.rounded(leg.joint,interp({front?4.9f:5.45f,5.3f,5.6f},{5.f,5.2f,5.f},t),shadeFur,101,false,-1,true);
  rig.tapered(leg.joint,leg.hock,front?4.8f:5.1f,interp(front?5.2f:5.5f,5.2f,t),shade(shadeFur,interp(.88f,.89f,t)),interp(.69f,.71f,t),1);
  if(t<1)rig.rounded(leg.hock,P3{3.45f,3.7f,3.5f}*(1-t),shade(shadeFur,.85f));
  rig.tapered(leg.hock,leg.paw,interp(3.4f,3.55f,t),3.8f,shade(shadeFur,.85f),interp(1.06f,1.05f,t),1);
  const hero_motion::Transform toes{asVector(leg.paw),hero_motion::mix(axisAngle({1,0,0},pitch),
      axisAngle({0,0,1},side*(restIndex%2?72.f:105.f)),release)};
  rig.setAttachment(&toes);
  rig.rounded(interp({0,1.15f,-.50f},{0,.8f,-.35f},t),interp({5.85f,7.65f,4.15f},{5.55f,7.0f,3.95f},t),shade(Charcoal,interp(.96f,.94f,t)));
  for(float toe:{-1.f,0.f,1.f}){
   rig.rounded(interp({1.48f*toe,3.65f-.30f*std::abs(toe),-1.1f},{1.4f*toe,2.95f,-.9f},t),
       interp({1.95f,2.9f,2.f},{1.85f,2.61f,1.78f},t),ColorLerp(shade(shade(Charcoal,.96f),1.18f),Charcoal,t));
  }
  rig.setAttachment(nullptr);
 }
 const P3 root=pose.carried(live.root,{2.8f*side,-23.5f,10.4f}),curve=pose.carried(live.curve,{6.2f*side,-32.8f,5.7f});
 const P3 brush=pose.carried(live.brush,{13.9f*side,-40.2f,3.8f}),tip=pose.carried(live.end,{22.2f*side,-42.9f,2.6f});
 const P3 tailEnd=pose.carried(live.tailTip,{25.3f*side,-44.25f,1.95f});
 rig.tapered(root,curve,6.65f,7.55f,shade(Charcoal,1.18f),1.11f,1);
 rig.tapered(curve,brush,7.38f,8.4f,shade(Charcoal,1.12f),.88f,1);
 rig.rounded(brush,{6.5f,7.8f,6.4f},shade(Charcoal,1.06f),101,false,-1,true);
 rig.tapered(brush,tip,6.5f,7.35f,Charcoal,.29f,1);
 rig.spike(tip,tailEnd,2.5f,2.5f,Charcoal);
}

inline void wolf(Rig& rig,int side=1){wolf(rig,{},-1,{1,0,0},side);}

inline void boar(Rig& rig,const AnimationState& state,float attack,float action,enemy_death::Key key,int side){
 using namespace forest_boar_art;
 using namespace hero_motion;
 const float t=key.collapse,walk=creatureWalk(state);
 if(t==0&&key.recoil==0){draw(rig,state,walk,attack,action,false);return;}
 const auto live=stance(state,walk,attack,action);
 const auto pose=fall(live.body,15.2f,82.f*side,t,-15.f*key.recoil,5.5f*key.recoil,
                      2.5f*key.settle-3.7f*key.recoil,6.5f*side*key.recoil);
 const auto headPose=pose.head(live.head,{6.65f*side,20.f,4.55f},5.f*side,-13.f*key.recoil);
 rig.setGroundBounded();
 rig.setAttachment(&pose.body);
 trunk(rig,live.cycle*(1-t),live.rush*(1-t),interp(live.wag,2.25f*side,t),interp(live.flick,-3.f,t),false);
 rig.setAttachment(&headPose);
 head(rig,interp(live.earLag,-1.4f,t),false,t>.47f);
 rig.setAttachment(nullptr);
 for(std::size_t i=0;i<BoarLegs.size();i++){
  const auto& step=live.limbs.legs[i];
  const auto point=[&](forest_boar_motion::Joint p){return P3{step.side,p.forward,p.height};};
  const std::size_t restIndex=side<0?(i^1u):i;
  const float release=ease((t-(step.front?.11f:.30f))/(step.front?.72f:.63f));
  const float kick=std::sin(release*AnimationPi)*(step.front?2.65f:5.60f);
  const auto leg=pose.leg({point(step.hip),point(step.knee),point(step.hock),point(step.foot)},onFlank(BoarLegs[restIndex],side),release,kick);
  const float width=i<2?9.65f:7.4f;
  const Color shin=shade(Bristle,1.3f);
  const Color upper=shade(Flank,interp(step.front?.92f:1.f,restIndex%2?.92f:.84f,t));
  rig.tapered(leg.hip,leg.joint,width,width*1.06f,upper,.66f,4);
  rig.rounded(leg.joint,{width*.63f,width*.69f,width*.64f},ColorLerp(upper,shade(Flank,.91f),t),t<.12f?-1:104,false,-1,t>=.12f);
  rig.tapered(leg.joint,leg.hock,interp(step.front?4.6f:5.25f,4.95f,t),interp(step.front?4.7f:5.2f,5.05f,t),shin,.71f);
  rig.rounded(leg.hock,{3.65f,4.05f,3.85f},shin);
  rig.tapered(leg.hock,leg.paw,3.65f,4.05f,shade(Bristle,1.15f),.97f);
  const hero_motion::Transform toes{asVector(leg.paw),hero_motion::mix(axisAngle({1,0,0},step.rock),
      axisAngle({0,0,1},side*(restIndex%2?79.f:99.f)),release)};
  rig.setAttachment(&toes);
  if(t<1)rig.rounded({0,.45f,.6f},P3{4.8f,4.1f,3.3f}*(1-t),shade(Bristle,1.15f));
  for(float split:{-1.f,1.f}){
   const P3 toe=interp({split*1.2f,1.18f,-.49f},{split*1.19f,1.25f,-.49f},t);
   rig.rounded(toe,{2.3f,5.55f,3.9f},HoofColor);
   rig.tapered(toe+interp({split*.60f,1,1.f},{split*.58f,1.1f,1.f},t),
       toe+interp({split*.60f,3.1f,.37f},{split*.58f,2.85f,.39f},t),interp(.59f,.57f,t),interp(.48f,.45f,t),shade(HoofColor,1.36f));
  }
  rig.setAttachment(nullptr);
 }
}

inline void boar(Rig& rig,int side=1){boar(rig,{},0,-1,{1,0,0},side);}

inline void death(Rig& rig,const Enemy& e,float progress){
 rig.setCreatureSurfaces();
 rig.reserve(2400);
 const float finish=int(e.animation.direction)*PI/4;
 const float start=e.deathMotion.facingInitialized?e.deathMotion.facingAngle:int(e.deathMotion.direction)*PI/4;
 const float turn=std::remainder(finish-start,2*PI);
 const float contact=enemy_death::groundContact(e.type);
 rig.setFacing(start+turn*enemy_death::window(progress,.12f,contact));
 const int side=e.deathSide();
 if(progress>=contact){
  if(e.type==BoarType)boar(rig,side);
  else wolf(rig,side);
  return;
 }
 const auto key=enemy_death::sample(e.type,progress);
 if(e.type==BoarType)boar(rig,e.deathMotion,e.deathAttack,e.deathAction,key,side);
 else wolf(rig,e.deathMotion,heavyMeleePreview(e.deathAction,e.deathAttack,meleeWindup(1)/(meleeWindup(1)+meleeRecovery(1))),key,side);
}
}

inline float guardianSpread(float progress){
 const float contact=enemy_death::groundContact(3);
 return std::clamp((progress-(contact-.24f))/.20f,0.f,1.f);
}
inline void buildFallenGuardian(Rig& rig,int flank,float scatter=1){
 rig.setCreatureSurfaces();rig.reserve(1000);rig.setGroundBounded();
 const Color stone=rgb(97,124,111),moss=rgb(78,104,63),crack=rgb(34,49,45),mica=rgb(142,151,111);
 const float spread=enemy_death::ease(scatter),hop=std::sin(scatter*PI);
 auto fragment=[&](P3 p,P3 size,Color tint,int seed,auto detail){
  p.r*=flank;
  const float advance=1-spread;
  const float lift=hop*(8+3*(seed%5))+advance*(9+2*(seed%3));
  const auto yaw=hero_motion::axisAngle({0,0,1},float((seed%5-2)*12)*advance);
  const auto tumble=hero_motion::axisAngle({1,0,0},float((seed%4-1)*22)*advance);
  const hero_motion::Transform pose{{p.r*(.48f+.52f*spread),
                                    p.f*(.70f+.30f*spread),p.z+lift},hero_motion::mul(yaw,tumble)};
  rig.setAttachment(&pose);
  const float x=size.r*.5f,y=size.f*.5f,z=size.z*.5f;
  const float chipped=.52f+.067f*(seed%4);
  const std::array<P3,8> edge{{{-x*chipped,-y,0},{x*.55f,-y*.94f,0},{x,-y*.55f,0},{x*.91f,y*.52f,0},
                              {x*.51f,y,0},{-x*.65f,y*.85f,0},{-x,y*.45f,0},{-x*.91f,-y*.51f,0}}};
  const P3 crown{-.14f*x,.16f*y,z};
  for(int i=0;i<8;i++){
   const auto a=edge[i],b=edge[(i+1)%8];
   const auto lowA=a*.80f-P3{0,0,z},lowB=b*.80f-P3{0,0,z};
   auto highA=crown+a*.74f,highB=crown+b*.74f;
   highA.z-=z*(.05f+.083f*((seed+i*3)%4));highB.z-=z*(.05f+.083f*((seed+((i+1)%8)*3)%4));
   const float light=.75f+std::cos(i*PI/4+.52f)*.14f;
   const float grain=.08f+.16f*((seed+i)%3);
   rig.face(lowA,lowB,b,a,tint,light);
   rig.face(a,b,highB,highA,tint,light+.24f);
   rig.materialQuad(a,b,highB,highA,3,grain,.18f,grain+.53f,.66f,Fade(shade(tint,1.35f),.72f));
   rig.face(crown,highA,highB,highB,tint,1.05f+.10f*std::cos(seed*.6f+i*.7f));
   rig.materialQuad(crown,highA,highB,highB,3,grain,.17f,grain+.45f,.73f,Fade(shade(tint,1.52f),.75f));
  }
  if(size.r>=12){
   // Mineral seams and lichen sit on the broken top face.
   const P3 mark{-.20f*x,.15f*y,z};
   rig.tube({mark+P3{-.37f*x,-.48f*y,-.13f*z},mark+P3{.1f*x,-.24f*y,.1f},
             mark+P3{.35f*x,.02f*y,.04f},mark+P3{.51f*x,.32f*y,-.08f*z}},.78f,.67f,crack);
   for(int i=0;i<3;i++){
    const P3 fleck{(.08f*i-.35f)*x,(.18f*i-.06f)*y,z+.05f};
    rig.rounded(fleck,{1.8f+(seed+i)%3,2.7f,1.1f},shade(seed%2?mica:moss,.78f+.10f*i),112,false,-1,true);
   }
  }
  detail(z);
  rig.setAttachment(nullptr);
 };
 const auto none=[](float){};
 fragment({-13,2,15},{26,35,29},shade(stone,1.04f),2,[&](float z){
  rig.tube({{-5,-7,z+.3f},{0,0,z+.32f},{-4,8,z+.26f},{-8,2,z+.28f},{-5,-7,z+.3f}},1.5f,1.25f,rgb(176,153,91));
 });
 fragment({15,7,12},{24,28,23},shade(stone,.87f),4,none);
 fragment({-1,-21,8},{25,14,16},shade(stone,.83f),7,none);
 fragment({7,45,11},{23,22,22},shade(stone,.96f),8,[&](float z){
  for(float side:{-1.f,1.f}){
   const P3 eye{side*4.6f,2.8f,z-.35f};
   rig.tapered(eye+P3{-2.6f,0,0},eye+P3{2.6f,0,0},2.3f,1.7f,crack);
   rig.tapered(eye+P3{-1.2f,0,.68f},eye+P3{1.8f,.32f,.68f},1.04f,1.03f,rgb(115,116,64));
  }
 });
 fragment({17,59,3},{11,8,6},shade(stone,.78f),9,none);
 for(int flankIndex=0;flankIndex<2;flankIndex++){
  const float s=flankIndex==0?-1.f:1.f;
  const float lead=s*7;
  fragment({s*37,17+lead,9},{19,21,18},shade(stone,1.07f),10+flankIndex,[&](float z){
   for(int i=0;i<4;i++)
    rig.rounded({-5+i*2.7f,1.8f+(i%2)*2,z+.25f},{4.6f,4.1f,1.9f},shade(moss,.85f+.10f*i),112,false,-1,true);
  });
  fragment({s*48,-6+lead,6},{14,18,13},shade(stone,.88f),12+flankIndex,none);
  fragment({s*53,-26+lead,6},{17,14,13},stone,14+flankIndex,[&](float z){
   for(int i=0;i<3;i++)rig.rounded({s*(3+i*1.5f),-5+i*3.5f,z-.8f},{4.7f,4.4f,3.8f},shade(stone,.84f+i*.11f));
  });
  fragment({s*19,-36+lead*.34f,7},{18,16,14},shade(stone,.95f),16+flankIndex,none);
  fragment({s*24,-53+lead*.55f,5},{16,13,11},shade(stone,.89f),18+flankIndex,none);
  fragment({s*28,-69+lead*.60f,4},{16,14,9},shade(stone,.81f),20+flankIndex,none);
 }
 for(int i=0;i<10;i++){
  const float angle=i*2.39996f+.64f,span=23+float(i%6)*7;
  const P3 fragmentCenter{std::cos(angle)*span,std::sin(angle)*span*1.12f,1.7f+(i%2)*.5f};
  fragment(fragmentCenter,{4+(i%3)*1.8f,4.4f,3.8f},shade(stone,.71f+.12f*(i%3)),22+i,none);
 }
}

// Fallen figures keep their anatomy, palette and equipment, with relaxed,
// asymmetric limbs that rest on the terrain.
inline void buildFallenForest(Rig& rig,int type,int side=1){
 rig.setCreatureSurfaces();rig.reserve(2400);
 Color ivory=rgb(227,217,181);
 if(type==0){
  Color gel=shade(forest_detail::Gel,.88f);forest_detail::surface(rig,{0,0,4.5f},{53,42,9},gel,0,false);
  rig.rounded({-5,-4,7},{34,27,10},shade(gel,1.12f));
  for(int i=0;i<7;i++)rig.rounded({-19+i*6.f,std::sin(i*1.7f)*15,2},{12,12,4},shade(gel,.93f));
  for(float side:{-1.f,1.f})rig.tapered({side*7-2,14,8},{side*7+2,14,8},1,1,rgb(41,65,42));
  rig.rounded({-10,-5,11},{12,6,1.8f},rgb(165,199,137));rig.rounded({8,3,9},{4,3,1.5f},rgb(153,192,126));
 }else if(type==1){
  fallen_quadrupeds::wolf(rig,side);
 }else if(type==2){
  buildFallenSkeleton(rig,side);
 }else if(type==3){
  buildFallenGuardian(rig,side);
 }else if(type==BoarType){
  fallen_quadrupeds::boar(rig,side);
 }else if(type==5){
  Color shell=rgb(62,78,67),venom=rgb(109,135,71);
  rig.rounded({0,-11,9},{27,31,18},shell,105);rig.rounded({1,14,5},{21,21,11},shade(shell,1.08f),105);
  for(int i=0;i<4;i++)for(float side:{-1.f,1.f}){
   float f=-19+i*10.f;rig.tube({{side*10,f,8},{side*24,f-6,5},{side*20,f+3,3},{side*12,f+6,2}},3,3,shade(shell,.78f));
   rig.rounded({side*24,f-6,5},{4,4,4},shade(shell,1.15f));
  }
  rig.rounded({0,-12,17},{8,19,2.8f},venom);
  for(int n=0;n<3;n++)rig.tapered({-8,-20+n*7.f,15},{7,-20+n*7.f,15},.8f,.8f,shade(shell,1.27f));
  for(float side:{-1.f,1.f})rig.spike({side*5,24,5},{side*8,28,2},2.3f,2.3f,ivory);
 }else if(type==6){
  buildFallenBandit(rig,side);
 }else{
  legacy_death::treantRest(rig,side);
 }
}
}
