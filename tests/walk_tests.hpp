#pragma once
#include "hero_gait_sole.hpp"
void walkTests(){
 using namespace hero_motion;float worstFloor=100,highestSupport=0;
 auto initial=evaluate(0,1,0,0),last=evaluate(.999999f,1,0,0);float motionJump=0;
 for(int b=0;b<data::BoneCount;b++)motionJump=std::max(motionJump,length(initial.world[b].p-last.world[b].p));
 check(motionJump<.003f,"new native walk closes every joint at the loop seam");
 auto startAttack=evaluate(.25f,1,0,0,0),walking=evaluate(.25f,1,0,0);float jump=0;
 for(int b=0;b<data::BoneCount;b++)jump=std::max(jump,length(startAttack.world[b].p-walking.world[b].p));
 check(jump<.003f,"attacking from a walk starts from the same full-body pose without restoring the old wobble");
 for(int f=0;f<600;f++){
  float phase=f/600.f;auto p=evaluate(phase,1,0,0);float support=100;
  for(int side=0;side<2;side++){
   int thigh=side?data::ThighR:data::ThighL,shin=side?data::ShinR:data::ShinL,foot=side?data::FootR:data::FootL;
   auto h=p.world[thigh].p,k=p.world[shin].p,a=p.world[foot].p;
   check(std::abs(length(h-k)-14.5f)<.003f&&std::abs(length(k-a)-14.5f)<.003f,"walking never stretches either leg segment or locks the knee backwards");
   check((side?1:-1)*k.x>3.5f&&std::abs(k.x)<8.2f,"walking knees follow their own narrow foot lane without crossing or bowing out");
   float bottom=walk_checks::soleHeight(p,foot);worstFloor=std::min(worstFloor,bottom);support=std::min(support,bottom);
   float u=std::fmod(phase+side*.5f,1.f);
   if(u<.60f)check(std::abs(bottom)<.04f,"heel-to-toe support keeps the actual rounded boot surface on the floor");
  }
  highestSupport=std::max(highestSupport,support);
 }
 check(worstFloor>-.04f&&highestSupport<.04f,"at least one real boot supports the body throughout walking with no flight phase");
 for(int blend=1;blend<16;blend++)for(int frame=0;frame<80;frame++){
  const auto pose=evaluate(frame/80.f,1,blend/16.f,0);
  check(std::min(walk_checks::soleHeight(pose,data::FootL),walk_checks::soleHeight(pose,data::FootR))>-.04f,
        "the weight-bearing sole clears the floor between the walk and run clips");
 }
 auto base=forward(sample(walk_synfig::clip,.25f));
 check(std::abs(base.world[data::FootL].p.x+5.8f)<.002f&&std::abs(base.world[data::FootR].p.x-5.8f)<.002f,"foot lanes are narrower and parallel instead of spread to hip width");
 for(int side=0;side<2;side++){
  int foot=side?data::FootR:data::FootL;float offset=side?.5f:0;
  auto a=evaluate(std::fmod(.10f+offset,1.f),1,0,0),b=evaluate(std::fmod(.25f+offset,1.f),1,0,0),c=evaluate(std::fmod(.40f+offset,1.f),1,0,0);
  check(std::abs((b.world[foot].p.y-a.world[foot].p.y)-(c.world[foot].p.y-b.world[foot].p.y))<.03f,"support foot travels evenly under the hips without accelerating/slowing in place");
 }
 float low[2]={100,100},high[2]={-100,-100};
 for(int frame=0;frame<240;frame++){
  auto p=evaluate(frame/240.f,1,0,0);
  for(int side=0;side<2;side++){
   int upper=side?data::UpperArmR:data::UpperArmL;
   int fore=side?data::ForearmR:data::ForearmL,hand=side?data::HandR:data::HandL;
   const auto wrist=p.world[hand].p;
   low[side]=std::min(low[side],wrist.y);high[side]=std::max(high[side],wrist.y);
   check(std::abs(length(p.world[fore].p-p.world[upper].p)-13)<.003f&&
         std::abs(length(wrist-p.world[fore].p)-13)<.003f,
         "larger walking arm swing keeps both arm segments rigid");
  }
 }
 check(high[0]-low[0]>24&&high[1]-low[1]>20,
       "both walking wrists have a pronounced forward and backward swing");
 // In a run each push-off leaves a short flight before the other boot lands.
 for(int side=0;side<2;side++){
  const int foot=side?data::FootR:data::FootL;
  const float offset=side*.5f,sign=side?1.f:-1.f;
  for(int f=0;f<=40;f++){
   auto run=evaluate(offset+f/100.f,1,1,0);
   check(std::abs(walk_checks::soleHeight(run,foot))<.045f,
         "the supporting running boot rolls through contact without sinking");
  }
  auto loaded=evaluate(offset+.08f,1,1,0),extended=evaluate(offset+.32f,1,1,0);
  auto flying=evaluate(offset+.46f,1,1,0);
  check(extended.world[data::Pelvis].p.z-loaded.world[data::Pelvis].p.z>3 &&
        sign*extended.world[data::Pelvis].p.x>.9f,
        "run compresses after landing and rises over the supporting hip");
  check(std::min(walk_checks::soleHeight(flying,data::FootL),walk_checks::soleHeight(flying,data::FootR))>.35f,
        "both boots clear the ground between opposing running contacts");
 }
}
