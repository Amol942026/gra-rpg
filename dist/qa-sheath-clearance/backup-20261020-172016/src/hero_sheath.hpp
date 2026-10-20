#pragma once
#include "sword_stow.hpp"
#include "hero_motion.hpp"
namespace ember::hero_sheath {
// This changes existing bone transforms only. Feet and locomotion stay intact.
inline sword_stow::Key apply(hero_motion::Pose& pose,float phase){
 using namespace hero_motion;auto key=sword_stow::sample(phase);
 if(phase<=0)return key;
 if(phase<1){
  bend(pose,data::Spine,axisAngle({0,0,1},key.yaw));
  bend(pose,data::Spine,axisAngle({0,1,0},key.lean));
  bend(pose,data::Head,mul(axisAngle({0,1,0},-key.lean*.75f),axisAngle({0,0,1},-key.yaw*.7f)));
  pose.bind();
 }
 auto m=sword_stow::Mouth,a=sword_stow::Axis;Vec mouth=pose.point(data::Pelvis,{m.r,m.f,m.z});
 Vec axis=pose.vector(data::Pelvis,{a.r,a.f,a.z});axis=axis*(1/length(axis));
 auto resting=sword_pose::idleBlade(0);Vec restBlade{resting.r,resting.f,resting.z};
 Quat seated=mul(pose.skin[data::Pelvis].q,fromTo(restBlade,{a.r*37,a.f*37,a.z*37}));
 Quat carry=mul(pose.world[data::Weapon].q,conjugate(restRotation(data::Weapon)));
 Quat orientation=mix(carry,seated,key.alignment);
 Vec seatedGrip=mouth-axis*sword_stow::SeatedGrip;
 if(phase<1){
  Vec target=mouth-axis*(37-(37-sword_stow::SeatedGrip)*key.insertion);
  Vec natural=pose.world[data::HandR].p;target=natural*(1-key.reach)+target*key.reach;
  auto shoulder=pose.world[data::UpperArmR].p;
  auto solved=sword_pose::solveArm({shoulder.x,shoulder.y,shoulder.z},{target.x,target.y,target.z});
  Vec elbow{solved.elbow.r,solved.elbow.f,solved.elbow.z},hand{solved.hand.r,solved.hand.f,solved.hand.z};
  attach(pose,data::UpperArmR,shoulder,elbow,restHead(data::ForearmR));
  attach(pose,data::ForearmR,elbow,hand,restHead(data::HandR));
  pose.world[data::HandR]={hand,mix(pose.world[data::HandR].q,mul(orientation,restRotation(data::HandR)),key.reach)};
 }
 // After insertion, the weapon belongs to the pelvis while the hand lets go.
 bool latched=key.insertion>=.999999f;
 pose.world[data::Weapon]={latched?seatedGrip:pose.world[data::HandR].p,mul(latched?seated:orientation,restRotation(data::Weapon))};
 pose.bind();return key;
}
}
