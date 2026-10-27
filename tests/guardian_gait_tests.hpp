#pragma once
#include "../src/generated/creature_motion_data.hpp"
void guardianGaitTests(){
 using namespace ember::guardian_gait;
 check(Scale==GuardianBodyScale,"guardian gait and runtime silhouette share the same physical scale");
 for(int heading=0;heading<16;heading++)for(int fps:{30,60,120})for(float speed:{1.35f,.81f,.27f}){
  float angle=heading*AnimationPi/8,fx=std::cos(angle),fy=std::sin(angle)/.52f,n=std::hypot(fx,fy);fx/=n;fy/=n;
  V direction=normalized({(fx/40+fy*.52f/20)*.5f,(fy*.52f/20-fx/40)*.5f});
  V step=direction*(speed/fps),pos{};AnimationState state;state.phase=.08f;state.moveBlend=1;state.motionInitialized=true;
  float maxSlip=0;int contacts=0;
  for(int frame=0;frame<fps*4;frame++){
   auto before=state;V old=pos;pos=pos+step;advance(state,step.x,step.y,1.f/fps);
   for(float side:{0.f,.5f}){
    float p0=std::fmod(before.phase+side,1.f),p1=std::fmod(state.phase+side,1.f);
    if(p1>=p0&&p0<Support-.02f&&p1<Support-.02f){
     float f0=creature_motion::sample(2,p0).travel*Stride*Scale,f1=creature_motion::sample(2,p1).travel*Stride*Scale;
     float sx=(pos.x-pos.y-old.x+old.y)*40+(f1-f0)*fx,sy=(pos.x+pos.y-old.x-old.y)*20+(f1-f0)*fy*.52f;
     maxSlip=std::max(maxSlip,std::hypot(sx,sy));contacts++;
    }
   }
  }
  check(contacts>fps&&maxSlip<.004f,"guardian support feet remain planted at 16 headings, three speeds and three FPS");
  float expected=std::fmod(.08f+phaseDistance(step.x*fps*4,step.y*fps*4),1.f);
  check(std::abs(std::remainder(state.phase-expected,1.f))<.0001f,"guardian steps follow distance, independently of simulation FPS");
 }
 for(int frame=0;frame<=240;frame++){
  float phase=frame/240.f,hip=hipHeight(phase);bool extended=false;
  for(float offset:{0.f,.5f}){
   float p=std::fmod(phase+offset,1.f);auto k=creature_motion::sample(2,p);float foot=k.travel*Stride,z=footHeight(p,hip);auto bend=knee(foot,hip,z);
   check(std::abs(std::hypot(bend.forward,bend.height-hip)-LegLength)<.001f&&std::abs(std::hypot(foot-bend.forward,z-bend.height)-LegLength)<.001f,"guardian leg lengths stay fixed while pelvis and recovering foot adapt");
   if(p<=Support){float cross=std::abs(bend.forward*(z-hip)-(bend.height-hip)*foot);extended|=cross<.01f;check(k.lift<.01f,"straight support leg keeps its foot on the ground");}
  }
  check(extended,"at least one grounded guardian knee is fully straight at every phase of the walking cycle");
 }
 AnimationState blocked;blocked.phase=.23f;for(int frame=0;frame<120;frame++)advance(blocked,0,0,1.f/60);
 check(blocked.phase==.23f&&!blocked.moving,"guardian cannot step in place while stopped, blocked or throwing");
 for(bool chase:{false,true}){
  Game g;g.props.clear();g.enemies.clear();g.forage.clear();g.player=chase?V{18,15}:V{2,2};g.enemies.push_back({91,3,{14,15},{14,15},260,260});
  auto& e=g.enemies[0];e.boulderCd=1000;e.behavior=EnemyBehavior::Patrol;e.patrolGoal={16,15};e.patrolGoalActive=true;
  for(int frame=0;frame<45;frame++){
   V before=e.pos;float phase=e.animation.phase;g.update(1.f/60,{},false);V moved=e.pos-before;
   check(length(moved)>0&&std::abs(e.animation.phase-std::fmod(phase+phaseDistance(moved.x,moved.y),1.f))<.00001f,"actual boss patrol and pursuit use collision-resolved displacement for cadence");
  }
 }
}
