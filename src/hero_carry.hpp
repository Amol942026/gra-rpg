#pragma once
#include "hero_motion.hpp"
#include "sword_pose.hpp"
namespace ember::hero_carry {
// Equipment needs a carrying grip: a free wrist swing can put an upright
// blade through the elbow even when it clears the head and breastplate.
inline void placeGrip(hero_motion::Pose& pose,bool left,hero_motion::Vec target,hero_motion::Vec pole,
                      hero_motion::Quat orientation){
 using namespace hero_motion;
 int upper=left?data::UpperArmL:data::UpperArmR,fore=left?data::ForearmL:data::ForearmR,hand=left?data::HandL:data::HandR;
 Vec shoulder=pose.world[upper].p;
 auto arm=sword_pose::solveArm({shoulder.x,shoulder.y,shoulder.z},{target.x,target.y,target.z},{pole.x,pole.y,pole.z});
 Vec elbow{arm.elbow.r,arm.elbow.f,arm.elbow.z},wrist{arm.hand.r,arm.hand.f,arm.hand.z};
 attach(pose,upper,shoulder,elbow,restHead(fore));
 attach(pose,fore,elbow,wrist,restHead(hand));
 pose.world[hand]={wrist,mul(orientation,restRotation(hand))};
 if(!left)pose.world[data::Weapon]={wrist,mul(orientation,restRotation(data::Weapon))};
 pose.bind();
}
inline void grip(hero_motion::Pose& pose,bool left,float width,hero_motion::Quat orientation){
 using namespace hero_motion;
 int hand=left?data::HandL:data::HandR;
 float side=left?-1.f:1.f;
 Vec local=rotate(conjugate(pose.skin[data::Spine].q),pose.world[hand].p-pose.world[data::Spine].p)+restHead(data::Spine);
 // A bent elbow and a grip in front of the ribs allow a near-upright weapon.
 local.x=side*std::max(side*local.x,width);
 local.y=std::clamp(local.y*.8f+14,8.f,24.f);
 local.z=std::clamp(local.z+6,42.f,50.f);
 placeGrip(pose,left,pose.point(data::Spine,local),pose.vector(data::Spine,{side*.35f,-1,.12f}),orientation);
}
inline constexpr float ShieldOffset=7.f,ShieldRimDepth=3.2f;
inline void shield(hero_motion::Pose& pose){
 using namespace hero_motion;
 auto pelvis=pose.skin[data::Pelvis].q;
 Vec swing=rotate(conjugate(pelvis),pose.world[data::HandL].p-pose.world[data::Pelvis].p)+restHead(data::Pelvis);
 // The scabbard follows the pelvis. Use that frame for clearance and retain
 // a shortened forward/back swing in the wrist within the arm's reach.
 Vec guard{-19.f+std::clamp((swing.x+19)*.25f,-2.f,2.f),
           17.5f+std::clamp(swing.y*.27f,-2.5f,3.f),
           42.f+std::clamp((swing.z-37)*.45f,-3.f,4.f)};
 placeGrip(pose,true,pose.point(data::Pelvis,guard),pose.vector(data::Pelvis,{-.85f,-1,.1f}),pelvis);
}
inline void melee(hero_motion::Pose& pose,float weight=1){
 using namespace hero_motion;
 weight=std::clamp(weight,0.f,1.f);if(weight<=0)return;
 auto originalHand=pose.world[data::HandR],originalFore=pose.world[data::ForearmR],originalWeapon=pose.world[data::Weapon];
 auto v=sword_pose::idleBlade(0);
 constexpr float lateral=6*3.14159265358979323846f/180;
 Quat orientation=mul(pose.skin[data::Spine].q,axisAngle({0,1,0},6));
 grip(pose,false,24,orientation);
 // Clearance changes forward pitch, rather than pushing the blade farther
 // sideways. Keep the crossguard across the forearm's plane as well.
 Vec wrist=pose.world[data::HandR].p,fore=pose.world[data::ForearmR].p-wrist;
 fore=fore*(1/std::max(.00001f,length(fore)));
 Vec localFore=rotate(conjugate(pose.skin[data::Spine].q),fore);
 float up=localFore.x*std::sin(lateral)+localFore.z*std::cos(lateral);
 float pitch=75.5297f*3.14159265358979323846f/180;
 if(up>0&&localFore.y<0)pitch=std::min(pitch,std::atan2(-localFore.y,up));
 Vec axis=pose.vector(data::Spine,{std::sin(lateral)*std::sin(pitch),std::cos(pitch),std::cos(lateral)*std::sin(pitch)});
 auto dot=[](Vec a,Vec b){return a.x*b.x+a.y*b.y+a.z*b.z;};
 auto cross=[](Vec a,Vec b){return Vec{a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};};
 Vec guard=cross(fore,axis);guard=guard*(1/std::max(.00001f,length(guard)));
 orientation=fromTo({v.r/37,v.f/37,v.z/37},axis);
 Vec initial=rotate(orientation,{1,0,0});
 float roll=std::atan2(dot(axis,cross(initial,guard)),dot(initial,guard))*180/3.14159265358979323846f;
 orientation=mul(axisAngle(axis,roll),orientation);
 Quat handRotation=mul(orientation,restRotation(data::HandR)),weaponRotation=mul(orientation,restRotation(data::Weapon));
 if(weight<1){
  // Fade the carrying correction at attack entry/recovery while solving
  // the arm again, rather than stretching its segments by blending points.
  Vec shoulder=pose.world[data::UpperArmR].p,target=originalHand.p*(1-weight)+wrist*weight;
  Vec pole=(originalFore.p-shoulder)*(1-weight)+(pose.world[data::ForearmR].p-shoulder)*weight;
  auto arm=sword_pose::solveArm({shoulder.x,shoulder.y,shoulder.z},{target.x,target.y,target.z},{pole.x,pole.y,pole.z});
  Vec elbow{arm.elbow.r,arm.elbow.f,arm.elbow.z};wrist={arm.hand.r,arm.hand.f,arm.hand.z};
  attach(pose,data::UpperArmR,shoulder,elbow,restHead(data::ForearmR));attach(pose,data::ForearmR,elbow,wrist,restHead(data::HandR));
  handRotation=mix(originalHand.q,handRotation,weight);weaponRotation=mix(originalWeapon.q,weaponRotation,weight);
 }
 pose.world[data::HandR]={wrist,handRotation};pose.world[data::Weapon]={wrist,weaponRotation};pose.bind();
}
inline void bow(hero_motion::Pose& pose){
 using namespace hero_motion;
 // Both limbs of the longest bow must clear the shoulder and the legs.
 // Rotate around the upright bow axis so the slack-side chord clears the arm.
 Quat carry=mul(axisAngle({0,1,0},-6),axisAngle({0,0,1},-75));
 grip(pose,true,24,mul(pose.skin[data::Spine].q,carry));
}
}
