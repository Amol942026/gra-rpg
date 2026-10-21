#pragma once
namespace walk_checks {
using namespace ember::hero_motion;
inline float soleHeight(const Pose& p,int foot){
 float lowest=100;auto y=p.vector(foot,{0,1,0}),z=p.vector(foot,{0,0,1}),x=p.vector(foot,{1,0,0});
 for(auto shape:{std::array<float,5>{3,-1,5,8.5f,4},std::array<float,5>{1,4,4.5f,5,4.5f},std::array<float,5>{5,-3,5,7,1}}){
  Vec c=p.point(foot,restHead(foot)+Vec{0,shape[0],shape[1]});
  float radius=std::sqrt(std::pow(x.z*shape[2],2)+std::pow(y.z*shape[3],2)+std::pow(z.z*shape[4],2));
  lowest=std::min(lowest,c.z-radius);
 }
 return lowest;
}
}
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
 auto base=forward(sample(walk_synfig::clip,.25f));
 check(std::abs(base.world[data::FootL].p.x+5.8f)<.002f&&std::abs(base.world[data::FootR].p.x-5.8f)<.002f,"foot lanes are narrower and parallel instead of spread to hip width");
 for(int side=0;side<2;side++){
  int foot=side?data::FootR:data::FootL;float offset=side?.5f:0;
  auto a=evaluate(std::fmod(.10f+offset,1.f),1,0,0),b=evaluate(std::fmod(.25f+offset,1.f),1,0,0),c=evaluate(std::fmod(.40f+offset,1.f),1,0,0);
  check(std::abs((b.world[foot].p.y-a.world[foot].p.y)-(c.world[foot].p.y-b.world[foot].p.y))<.03f,"support foot travels evenly under the hips without accelerating/slowing in place");
 }
}
