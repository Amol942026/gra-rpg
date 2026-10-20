#pragma once
#include "../src/hero_motion.hpp"
void heroMotionTests(){
 using namespace hero_motion;
 const data::Clip* clips[]={&data::IdleClip,&data::WalkClip,&data::RunClip,&data::SlashClip,&data::HeavyClip,&data::RollClip};
 for(auto clip:clips){
  Pose previous=forward(sample(*clip,0));
  for(int frame=0;frame<=240;frame++){
   Pose p=forward(sample(*clip,frame/240.f));bool finite=true,unit=true,skin=true;
   for(int b=0;b<data::BoneCount;b++){
    auto v=p.world[b].p;auto q=p.world[b].q;
    finite&=std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z)&&length(v)<180;
    unit&=std::abs(q.w*q.w+q.x*q.x+q.y*q.y+q.z*q.z-1)<.0001f;
    skin&=length(p.point(b,restHead(b))-v)<.002f;
   }
   check(finite&&unit&&skin,"Blender pose interpolation yields finite rigid bone transforms and matching skin anchors");
   for(int side=0;side<2;side++){
    int upper=side?data::UpperArmR:data::UpperArmL,fore=side?data::ForearmR:data::ForearmL,hand=side?data::HandR:data::HandL;
    int thigh=side?data::ThighR:data::ThighL,shin=side?data::ShinR:data::ShinL,foot=side?data::FootR:data::FootL;
    check(std::abs(length(p.world[upper].p-p.world[fore].p)-13)<.003f&&std::abs(length(p.world[fore].p-p.world[hand].p)-13)<.003f,"skeletal arm lengths remain anatomical even between baked samples");
    check(std::abs(length(p.world[thigh].p-p.world[shin].p)-14.5f)<.003f&&std::abs(length(p.world[shin].p-p.world[foot].p)-14.5f)<.003f,"skeletal leg lengths do not stretch during walking running attacks or tucked rolls");
   }
   if(frame){bool continuous=true;for(int b=0;b<data::BoneCount;b++)continuous&=length(p.world[b].p-previous.world[b].p)<6;check(continuous,"the full articulated pose remains continuous at subframe samples");}
   previous=p;
  }
  if(clip->loop){
   auto unwrapped=*clip;unwrapped.loop=false;
   auto a=forward(sample(unwrapped,0)),b=forward(sample(unwrapped,1));bool closed=true;
   for(int bone=0;bone<data::BoneCount;bone++)closed&=length(a.world[bone].p-b.world[bone].p)<.02f;
   check(closed,"authored cyclic clips join at the same pose without restarting a displaced foot");
  }
 }
 for(float run:{0.f,.25f,.5f,.75f,1.f})for(float moving:{0.f,.3f,1.f}){
  auto p=evaluate(.17f,moving,run,1);bool valid=true;
  for(int b=0;b<data::BoneCount;b++)valid&=std::isfinite(p.world[b].p.z);
  check(valid,"idle walk and run blend continuously in the shared skeleton");
 }
 AnimationState state;for(int i=0;i<40;i++)state.advance(.02f,0,1.f/60);
 float phase=state.phase,weight=state.moveBlend;state.advance(0,0,1.f/60);
 check(state.phase==phase&&state.moveBlend<weight&&state.moveBlend>0,"stopping preserves gait phase and blends toward rest instead of snapping feet");
 state.face(-1,1);float angle=state.facingAngle;state.advance(0,0,1.f/60);
 check(std::abs(state.facingAngle-angle)<1&&std::abs(state.facingAngle-angle)>.001f,"facing turns through intermediate angles while gameplay keeps its directional state");
 // Synfig secondary tracks must never lift a planted foot or stretch an arm.
 for(float phase:{0.f,.13f,.25f,.5f,.73f,.99f})for(float running:{0.f,1.f}){
  Pose base=forward(sample(running?data::RunClip:data::WalkClip,phase)),p=base;
  secondary(p,hero_secondary::evaluate(phase,1,running,0));
  for(int b=data::ThighL;b<=data::FootR;b++)check(length(p.world[b].p-base.world[b].p)<.00001f,"Synfig torso motion preserves exact primary foot contact and leg pose");
  for(int side=0;side<2;side++){
   int upper=side?data::UpperArmR:data::UpperArmL,fore=side?data::ForearmR:data::ForearmL,hand=side?data::HandR:data::HandL;
   check(std::abs(length(p.world[upper].p-p.world[fore].p)-13)<.003f&&std::abs(length(p.world[fore].p-p.world[hand].p)-13)<.003f,"Synfig follow-through rotates rigid arm chains without stretching");
  }
 }
 for(auto clip:{&hero_secondary::idle,&hero_secondary::walk,&hero_secondary::run}){
  auto a=hero_secondary::sample(*clip,.99999f),b=hero_secondary::sample(*clip,0);
  check(std::abs(a.spineRoll-b.spineRoll)+std::abs(a.headRoll-b.headRoll)+std::abs(a.capeSway-b.capeSway)<.01f,"native Synfig cycles close smoothly across the virtual loop endpoint");
 }
}
