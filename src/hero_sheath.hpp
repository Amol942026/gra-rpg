#pragma once
#include "sword_stow.hpp"
#include "hero_motion.hpp"
namespace ember::hero_sheath {
// This changes existing bone transforms only. Feet and locomotion stay intact.
inline sword_stow::Key apply(hero_motion::Pose& pose,float phase){
 using namespace hero_motion;auto key=sword_stow::sample(phase);
 if(phase>0&&phase<1){
  bend(pose,data::Spine,axisAngle({0,0,1},key.yaw));
  bend(pose,data::Spine,axisAngle({0,1,0},key.lean));
  bend(pose,data::Head,mul(axisAngle({0,1,0},-key.lean*.75f),axisAngle({0,0,1},-key.yaw*.7f)));
  pose.bind();
 }
 auto m=sword_stow::Mouth,a=sword_stow::Axis;Vec mouth=pose.point(data::Pelvis,{m.r,m.f,m.z});
 Vec axis=pose.vector(data::Pelvis,{a.r,a.f,a.z});axis=axis*(1/length(axis));
 // An explicit signed pitch travels via forward, never the shortest quaternion
 // arc from raised carry to downward scabbard (which crosses the head).
 Quat lateral=fromTo({0,a.f,a.z},{a.r,a.f,a.z});
 Quat authored=mul(mix(Quat{},lateral,key.alignment),axisAngle({1,0,0},key.pitch-75.5297f));
 Quat seated=mul(pose.skin[data::Pelvis].q,mul(lateral,axisAngle({1,0,0},std::atan2(a.z,a.f)*180/3.14159265358979323846f-75.5297f)));
 Quat carry=mul(pose.world[data::Weapon].q,conjugate(restRotation(data::Weapon)));
 Quat orientation=mix(carry,mul(pose.skin[data::Pelvis].q,authored),key.reach);
 Vec seatedGrip=mouth-axis*sword_stow::SeatedGrip;
 if(phase<1){
  Vec target=pose.point(data::Pelvis,{key.handR,key.handF,key.handZ});
  Vec natural=pose.world[data::HandR].p;
  if(key.insertion<1){
   // Rapid walk/run reversals can retain a running arm swing while the sword
   // is already drawn. Keep that carry hand outside the breastplate as well.
   Vec local=rotate(conjugate(pose.skin[data::Spine].q),natural-pose.world[data::Spine].p)+restHead(data::Spine);
   local.x=std::max(local.x,24.f);natural=pose.point(data::Spine,local);
  }
  target=natural*(1-key.reach)+target*key.reach;
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
