#pragma once
#include "generated/shield_stow_data.hpp"
#include "hero_carry.hpp"
#include "items.hpp"

namespace ember::shield_stow {
using namespace hero_motion;
namespace bone=hero_motion::data;

inline data::Key sample(float phase){
 constexpr int last=sizeof(data::keys)/sizeof(data::keys[0])-1;
 const float position=std::clamp(phase,0.f,1.f)*last;
 const int index=std::min(int(position),last-1);
 const float t=position-index;
 const auto a=data::keys[index],b=data::keys[index+1];
 const auto blend=[t](float x,float y){return x+(y-x)*t;};
 return {blend(a.right,b.right),blend(a.forward,b.forward),blend(a.height,b.height),
         blend(a.yaw,b.yaw),blend(a.depart,b.depart),blend(a.reach,b.reach)};
}
inline constexpr Vec HeldOffset{-2,hero_carry::ShieldOffset,1};
inline Vec socket(Kind weapon,float backDepth){
 const float side=weapon==Kind::Bow?7.f:weapon==Kind::Spear?-8.f:weapon==Kind::Axe?8.f:0.f;
 const float depth=weapon==Kind::Axe||weapon==Kind::Spear?std::max(16.f,backDepth+3.4f):15.f;
 return {side,-depth,weapon==Kind::Bow?42.f:45.f};
}
inline Transform apply(Pose& pose,Kind weapon,float phase,float backDepth,bool moveArm,const Pose* untwisted=nullptr){
 const Vec rest=socket(weapon,backDepth);
 const Quat spine=pose.skin[bone::Spine].q;
 const auto turn=[](float yaw){return axisAngle({0,0,1},yaw);};
 const Transform stored{pose.point(bone::Spine,rest),mul(spine,turn(180))};
 if(phase>=1)return stored;
 Pose guard=pose;
 if(moveArm)hero_carry::shield(guard);
 const Transform held{guard.point(bone::HandL,restHead(bone::HandL)+HeldOffset),guard.skin[bone::HandL].q};
 if(phase<=0){
  if(moveArm)pose=guard;
  return held;
 }

 const auto key=sample(phase);
 const float nearBack=ease((phase-.43f)/.32f);
 const Vec fixedArc{key.right,key.forward,key.height};
 const Vec local=fixedArc+(rest-Vec{0,-15,45})*nearBack;
 Vec arc=pose.point(bone::Spine,local);
 Quat carrier=spine;
 if(untwisted){
  // The sword's waist-level draw adds a fast torso turn. Ease the shield
  // into that moving frame as it approaches the back instead of inheriting
  // the full twist at the start of its own transfer.
  const float follow=ease((phase-.18f)/.54f);
  arc=untwisted->point(bone::Spine,local)*(1-follow)+arc*follow;
  carrier=mix(untwisted->skin[bone::Spine].q,spine,follow);
 }
 const Vec target=held.p*(1-key.depart)+arc*key.depart;
 const Quat orientation=mul(mix(held.q,carrier,key.depart),turn(key.yaw));
 const Vec offset=rotate(orientation,HeldOffset);
 Vec center=target;
 if(moveArm){
  const Vec shoulder=pose.world[bone::UpperArmL].p;
  const auto freeHand=pose.world[bone::HandL];
  const Vec freePole=pose.world[bone::ForearmL].p-shoulder;
  const Vec guardPole=(guard.world[bone::ForearmL].p-shoulder)*(1.f/13);
  const Vec sidePole=pose.vector(bone::Spine,{-1,-.43f,-.17f});
  const Vec pole=(guardPole*(1-key.depart)+sidePole*key.depart)*key.reach+freePole*((1-key.reach)/13);
  hero_carry::placeGrip(pose,true,freeHand.p*(1-key.reach)+(target-offset)*key.reach,
                       pole,mix(pose.skin[bone::HandL].q,orientation,key.reach));
  // The wrist stays on the handle during the transfer, then the back mount
  // takes the weight so the hand can resume the natural running swing.
  const float latch=ease((phase-.625f)/.125f);
  center=(pose.world[bone::HandL].p+offset)*(1-latch)+target*latch;
 }
 return {center,orientation};
}
}
