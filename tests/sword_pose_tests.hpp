#pragma once
#include "../src/sword_pose.hpp"
void swordPoseTests(){
 using namespace sword_pose;
 for(bool heavy:{false,true}){
  Pose previous=sample(0);
  for(int frame=0;frame<=160;frame++){
   float progress=frame/160.f;Pose pose=sample(progress,1.3f,heavy);Point shoulder{15,0,54.3f};
   check(std::abs(length(pose.elbow-shoulder)-UpperArm)<.001f&&std::abs(length(pose.hand-pose.elbow)-Forearm)<.001f,"sword animation preserves both anatomical arm lengths");
   check(std::abs(length(pose.blade)-BladeLength)<.001f,"sword never lengthens or shrinks while cutting");
   check(length(pose.hand-shoulder)<UpperArm+Forearm&&pose.hand.z>20,"sword hand remains within the arm's actual reach and above the ground");
   if(frame>0)check(length(pose.hand-previous.hand)<3&&length(pose.elbow-previous.elbow)<3,"sword joints move continuously without frame-to-frame popping");
   previous=pose;
  }
  Pose start=sample(0,0,heavy),finish=sample(1,0,heavy);
  check(length(start.hand-finish.hand)<.001f&&length(start.elbow-finish.elbow)<.001f&&length(start.blade-finish.blade)<.001f&&std::abs(finish.torsoYaw)<.0001f,"swing returns to the exact initial grip and body orientation");
  for(float boundary:{cutStart(heavy),cutEnd(heavy)}){
   Pose a=sample(boundary-.00001f,0,heavy),b=sample(boundary+.00001f,0,heavy);
   check(length(a.hand-b.hand)<.001f&&length(a.elbow-b.elbow)<.001f&&length(a.blade-b.blade)<.001f&&std::abs(a.torsoYaw-b.torsoYaw)<.001f,"windup, cut and recovery join without a discontinuity");
  }
  Pose contact=sample((cutStart(heavy)+cutEnd(heavy))*.5f,0,heavy);
  Point tip=turn(contact.hand+contact.blade,contact.torsoYaw);
  check(tip.f>50,"actual cutting edge reaches clearly in front of the hero");
  Pose ready=sample(cutStart(heavy),0,heavy),follow=sample(cutEnd(heavy),0,heavy);
  check(turn(ready.hand+ready.blade,ready.torsoYaw).r>0&&turn(follow.hand+follow.blade,follow.torsoYaw).r<0,"the sword crosses from the right windup to the left follow-through");
  for(float progress:{0.f,cutStart(heavy)*.5f,1.f})check(trailStrength(progress,heavy)==0,"the sword trail does not create phantom cuts in preparation or recovery");
  check(trailStrength((cutStart(heavy)+cutEnd(heavy))*.5f,heavy)>0,"the trail is visible during the actual forward cut");
 }
 check(cutStart(true)>cutStart(false)&&std::abs(sample(cutStart(true),0,true).torsoYaw)>std::abs(sample(cutStart(false)).torsoYaw),"strong cut has a distinct longer preparation and stronger body rotation");
 for(float running:{0.f,.25f,.5f,.75f,1.f})check(std::abs(length(idleBlade(running))-BladeLength)<.001f,"running carry preserves the same rigid sword length");
 for(Point target:{Point{15,0,53},Point{45,-3.6f,56.6f}}){Arm arm=solveArm({15,0,53},target);check(std::isfinite(arm.elbow.r)&&std::abs(length(arm.elbow-Point{15,0,53})-UpperArm)<.001f,"arm solver remains valid at a coincident target and an aligned bend pole");}
}
