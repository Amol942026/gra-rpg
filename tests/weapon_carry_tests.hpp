#pragma once
#include "../src/items.hpp"
#include "../src/equipment_shapes.hpp"
#include "../src/hero_carry.hpp"
#include "../src/hero_weapon_motion.hpp"
#include "../src/spear_motion.hpp"
#include "sword_stow_clearance.hpp"
namespace carry_checks {
using namespace ember;
using namespace hero_motion;
inline float dot(Vec a,Vec b){return a.x*b.x+a.y*b.y+a.z*b.z;}
inline float pointSegment(Vec p,Vec a,Vec b){
 Vec d=b-a;return length(p-a-d*std::clamp(dot(p-a,d)/std::max(.000001f,dot(d,d)),0.f,1.f));
}
inline float segments(Vec a,Vec b,Vec c,Vec d){
 Vec u=b-a,v=d-c,w=a-c;
 float A=dot(u,u),B=dot(u,v),C=dot(v,v),D=dot(u,w),E=dot(v,w),den=A*C-B*B;
 float result=std::min({pointSegment(a,c,d),pointSegment(b,c,d),pointSegment(c,a,b),pointSegment(d,a,b)});
 if(den>.00001f){
  float s=(B*E-C*D)/den,t=(A*E-B*D)/den;
  if(s>=0&&s<=1&&t>=0&&t<=1)result=std::min(result,length(w+u*s-v*t));
 }
 return result;
}
struct Clearance {float arms=100,body=100;int armRegion=-1,feature=-1;};
inline Clearance measure(const Pose& pose,Kind kind){
 Clearance result;
 int feature=0;
 auto segment=[&](Vec a,Vec b,float radius){
  for(int side=0;side<2;side++){
   int upper=side?data::UpperArmR:data::UpperArmL,fore=side?data::ForearmR:data::ForearmL,hand=side?data::HandR:data::HandL;
   Vec shoulder=pose.world[upper].p,elbow=pose.world[fore].p,wrist=pose.world[hand].p;
   // The hand wraps the grip. Exclude only that hand/cuff region; the
   // shoulder, upper arm and proximal half of the forearm must stay clear.
   float regions[]={segments(a,b,shoulder,elbow)/(4.5f+radius),
       segments(a,b,elbow,(elbow+wrist)*.5f)/(4.f+radius),
       stow_clearance::segmentEllipsoid(pose,upper,a,b,restHead(upper),{7+radius,9+radius,7+radius})};
   for(int r=0;r<3;r++)if(regions[r]<result.arms){result.arms=regions[r];result.armRegion=side*3+r;result.feature=feature;}
  }
  result.body=std::min({result.body,
      stow_clearance::segmentEllipsoid(pose,data::Head,a,b,{0,0,72},{12+radius,12+radius,16+radius}),
      stow_clearance::segmentEllipsoid(pose,data::Spine,a,b,{0,0,46},{14+radius,10.5f+radius,14.5f+radius})});
  feature++;
 };
 Kind shape=archetype(kind);
 if(shape==Kind::Bow){
  float half=equipment_shape::BowHalfLength,curve=equipment_shape::BowCurve;
  Vec grip=pose.world[data::HandL].p,axis=pose.vector(data::HandL,{0,0,1}),forward=pose.vector(data::HandL,{0,1,0});
  Vec bottom=grip-axis*half-forward*curve,top=grip+axis*half-forward*curve,previous=bottom;
  for(int part=1;part<=16;part++){
   float t=part/16.f;Vec next=grip+axis*(-half+2*half*t)+forward*(curve*(std::sin(t*3.14159265358979323846f)-1));
   segment(previous,next,1.9f);previous=next;
  }
  segment(bottom,top,.4f);
 }else{
  auto blade=sword_pose::idleBlade(0);float scale=shape==Kind::Dagger?.65f:1;
  Vec hand=pose.world[data::Weapon].p,v=pose.vector(data::Weapon,{blade.r*scale,blade.f*scale,blade.z*scale});
  Vec across=pose.vector(data::Weapon,{1,0,0});
  auto outline=[&](const auto& points,Vec origin,Vec u,Vec h,float radius){
   auto point=[&](size_t i){return origin+u*points[i].x+h*points[i].y;};
   for(size_t i=0;i<points.size();i++)segment(point(i),point((i+1)%points.size()),radius);
  };
  if(shape==Kind::Axe){
   segment(hand,hand+v,1.3f);
   outline(equipment_shape::Axe,hand+v*.45f,across*(length(v)*.30f),v*.30f,1.2f);
  }else{
   bool dagger=shape==Kind::Dagger;
   Vec guard=hand+v*.1f,cross=across*(dagger?5.f:7.f);
   Vec left=guard-cross*.5f-v*.04f,right=guard+cross*.5f-v*.04f;
   segment(guard-cross,left,.85f);segment(left,right,.85f);segment(right,guard+cross,.85f);
   Vec base=guard,vertical=(hand+v-base)*(1/(dagger?1.9f:2.55f));
   Vec origin=base+vertical*.65f,width=across*length(vertical);
   if(dagger)outline(equipment_shape::Dagger,origin,width,vertical,.7f);
   else outline(equipment_shape::Sword,origin,width,vertical,.7f);
   segment(base,hand+v,.7f);
  }
 }
 return result;
}
inline Pose equipped(float phase,float motion,float run,Kind kind){
 auto pose=evaluate(phase,motion,run,0);
 if(archetype(kind)==Kind::Sword)hero_sheath::apply(pose,0);
 else if(archetype(kind)==Kind::Bow)hero_carry::bow(pose);
 else hero_weapon_motion::carry(pose,archetype(kind));
 return pose;
}
}
inline void weaponCarryTests(){
 using namespace ember;using namespace hero_motion;
 float shieldGap=100,shieldArmError=0;
 for(float motion:{0.f,.5f,1.f})for(float run:{0.f,.1f})for(int frame=0;frame<120;frame++){
  auto pose=evaluate(frame/120.f,motion,run,0);
  hero_sheath::apply(pose,0);
  hero_carry::shield(pose);
  // Both planes are measured along the pelvis forward axis. The scabbard
  // tapers back from its widest fitting at the mouth.
  Vec shieldRear=pose.point(data::HandL,restHead(data::HandL)+Vec{-2,hero_carry::ShieldOffset-hero_carry::ShieldRimDepth*.5f,1});
  Vec mouth=pose.point(data::Pelvis,{sword_stow::Mouth.r,sword_stow::Mouth.f,sword_stow::Mouth.z});
  shieldGap=std::min(shieldGap,carry_checks::dot(shieldRear-mouth,pose.vector(data::Pelvis,{0,1,0}))-3.3f);
  Vec elbow=pose.world[data::ForearmL].p;
  shieldArmError=std::max({shieldArmError,std::abs(length(elbow-pose.world[data::UpperArmL].p)-13),
                           std::abs(length(elbow-pose.world[data::HandL].p)-13)});
 }
 check(shieldGap>1,"held shield clears the scabbard throughout walking");
 check(shieldArmError<.003f,"shield carrying keeps the hand attached and preserves arm length");
 int weapons=0;
 for(int k=1;k<ItemKindCount;k++)if(gearSlot(Kind(k))==0){
  weapons++;
  if(spearVariant(Kind(k)))continue;
  for(float motion:{0.f,.5f,1.f})for(float run:{0.f,.5f,1.f})for(int f=0;f<120;f++){
   auto pose=carry_checks::equipped(f/120.f,motion,run,Kind(k));
   auto clearance=carry_checks::measure(pose,Kind(k));
   check(clearance.arms>=1&&clearance.body>=1,"all held weapon variants clear shoulders, upper arms, forearms, head and torso throughout locomotion");
   auto v=sword_pose::idleBlade(0);
   Vec axis=archetype(Kind(k))==Kind::Bow?pose.vector(data::HandL,{0,0,1}):pose.vector(data::Weapon,{v.r,v.f,v.z});
   axis=rotate(conjugate(pose.skin[data::Spine].q),axis);
   check(axis.z>0&&std::abs(std::atan2(axis.x,axis.z))*180/3.14159265358979323846f<6.1f,
         "held weapons remain upright without excessive lateral flare at any gait phase");
   if(archetype(Kind(k))==Kind::Axe)
    check(carry_checks::dot(pose.vector(data::Weapon,{-1,0,0}),pose.vector(data::Spine,{0,1,0}))>.9f,
          "axe cutting edge faces forward in every carry pose");
   for(int side=0;side<2;side++){
    int upper=side?data::UpperArmR:data::UpperArmL,fore=side?data::ForearmR:data::ForearmL,hand=side?data::HandR:data::HandL;
    check(std::abs(length(pose.world[fore].p-pose.world[upper].p)-13)<.003f&&std::abs(length(pose.world[hand].p-pose.world[fore].p)-13)<.003f,"weapon carrying preserves both anatomical arm lengths");
   }
  }
 }
 check(weapons==47,"clearance covers the complete current weapon catalogue");
 for(Kind kind:{Kind::Axe,Kind::Dagger}){
  const bool axe=kind==Kind::Axe;
  const int frames=axe?AxeAttackFrames:DaggerAttackFrames;
  const int impact=axe?AxeImpactFrame:DaggerImpactFrame;
  const auto shaft=sword_pose::idleBlade(0);
  for(float motion:{0.f,1.f})for(float gait:{0.f,.25f,.5f,.75f}){
   auto carried=carry_checks::equipped(gait,motion,0,kind);
   for(int frame=0;frame<=frames*2;frame++){
    auto pose=evaluate(gait,motion,0,0);
    hero_weapon_motion::apply(pose,kind,frame/float(frames*2));
    const auto wrist=pose.world[data::HandR].p,elbow=pose.world[data::ForearmR].p;
    const auto clearance=carry_checks::measure(pose,kind);
    check(clearance.arms>=1&&clearance.body>=1,"axe and dagger attack clear the arm and body");
    check(std::abs(length(elbow-pose.world[data::UpperArmR].p)-13)<.003f&&std::abs(length(wrist-elbow)-13)<.003f,
          "axe and dagger attack preserve both arm segments");
    if(frame==0||frame==frames*2)
     check(length(wrist-carried.world[data::HandR].p)<.001f&&
           length(elbow-carried.world[data::ForearmR].p)<.001f,"attack joins the same carrying pose at both ends");
    if(frame==impact*2)
     check(pose.vector(data::Weapon,{shaft.r,shaft.f,shaft.z}).y>33&&wrist.y>18,
           "the blade reaches forward at the real impact frame");
    if(axe&&frame==16)
     check(pose.vector(data::Weapon,{shaft.r,shaft.f,shaft.z}).y<-10,
           "axe winds up behind the hero before the forward chop");
   }
  }
 }
 for(int id=FirstSpear;id<ItemKindCount;id++)for(int frame=0;frame<=SpearFrames;frame++)for(float stride:{-1.f,0.f,1.f}){
  auto pose=evaluate(.25f,1,0,0);
  const float slide=spear_motion::apply(pose,Kind(id),frame/float(SpearFrames),1,stride);
  const Vec rear=pose.world[data::HandR].p,front=pose.world[data::HandL].p;
  check(length(front-rear)>8&&length(front-rear)<14,"both hands hold the same spear shaft through the thrust");
  if(frame==SpearImpactFrame){
   const auto axis=(front-rear)*(1/length(front-rear));
   check(front.y-rear.y>9&&std::abs(front.z-rear.z)<5&&rear.y+axis.y*(42.4f+slide)>62,
         "spear tip projects well ahead of the hero at contact");
  }
  for(int side=0;side<2;side++){
   int upper=side?data::UpperArmR:data::UpperArmL,fore=side?data::ForearmR:data::ForearmL,hand=side?data::HandR:data::HandL;
   check(std::abs(length(pose.world[fore].p-pose.world[upper].p)-13)<.003f&&
         std::abs(length(pose.world[hand].p-pose.world[fore].p)-13)<.003f,"spear IK keeps the anatomical arm lengths");
  }
 }
 for(bool heavy:{false,true})for(float gait:{0.f,.25f,.5f,.75f})for(float endpoint:{0.f,1.f}){
  auto carried=carry_checks::equipped(gait,1,0,Kind::Sword);
  auto attack=evaluate(gait,1,0,0,endpoint,heavy);hero_carry::melee(attack,1);
  check(length(carried.world[data::HandR].p-attack.world[data::HandR].p)<.001f&&
        length(carried.vector(data::Weapon,{0,0,37})-attack.vector(data::Weapon,{0,0,37}))<.001f,
        "attack entry and recovery join the corrected carrying grip without a snap");
 }
}
