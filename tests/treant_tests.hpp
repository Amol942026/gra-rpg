#pragma once
#include "../src/generated/creature_motion_data.hpp"
void treantTests(){
 using namespace ember::treant_gait;
 for(int heading=0;heading<8;heading++)for(int fps:{30,60,120})for(float speed:{1.45f,1.45f*PatrolSpeedFactor}){
  float angle=heading*AnimationPi/4,fx=std::cos(angle),fy=std::sin(angle)/.52f,n=std::hypot(fx,fy);fx/=n;fy/=n;
  V direction=normalized({(fx/40+fy*.52f/20)*.5f,(fy*.52f/20-fx/40)*.5f});
  V step=direction*(speed/fps),pos{};AnimationState state;state.phase=.08f;state.moveBlend=1;state.motionInitialized=true;
  float maxSlip=0;int contactSamples=0;
  for(int frame=0;frame<fps*3;frame++){
   auto before=state;V old=pos;pos=pos+step;advance(state,step.x,step.y,1.f/fps);
   for(float side:{0.f,.5f}){
    float p0=std::fmod(before.phase+side,1.f),p1=std::fmod(state.phase+side,1.f);
    if(p1>=p0&&p0<Support-.02f&&p1<Support-.02f){
     float f0=creature_motion::sample(5,p0).travel*Stride*Scale,f1=creature_motion::sample(5,p1).travel*Stride*Scale;
     float sx=(pos.x-pos.y-old.x+old.y)*40+(f1-f0)*fx;
     float sy=(pos.x+pos.y-old.x-old.y)*20+(f1-f0)*fy*.52f;
     maxSlip=std::max(maxSlip,std::hypot(sx,sy));contactSamples++;
    }
   }
  }
  check(contactSamples>fps&&maxSlip<.003f,"treant support foot remains planted in screen space at every direction, speed and FPS");
  float expected=std::fmod(.08f+phaseDistance(step.x*fps*3,step.y*fps*3),1.f);
  check(std::abs(std::remainder(state.phase-expected,1.f))<.00005f,"treant cadence depends on real ground distance, not elapsed time");
 }
 for(int frame=0;frame<=240;frame++){
  auto k=creature_motion::sample(5,frame/240.f);float foot=k.travel*Stride,z=4+k.lift*Lift,hip=40+k.bob;
  auto p=knee(foot,hip,z);
  check(std::abs(std::hypot(p.forward,p.height-hip)-LegLength)<.001f&&std::abs(std::hypot(foot-p.forward,z-p.height)-LegLength)<.001f,"treant leg lengths stay constant through full step");
 }
 AnimationState blocked;blocked.phase=.23f;for(int i=0;i<60;i++)advance(blocked,0,0,1.f/60);
 check(blocked.phase==.23f&&!blocked.moving,"blocked or attacking treant cannot walk in place");
 Game g;g.props.clear();g.enemies.clear();g.player={17,17};g.enemies.push_back({91,7,{14,14},{14,14},170,170});
 V before=g.enemies[0].pos;float phase=g.enemies[0].animation.phase;g.update(.05f,{},false);V moved=g.enemies[0].pos-before;
 check(length(moved)>0&&std::abs(g.enemies[0].animation.phase-std::fmod(phase+phaseDistance(moved.x,moved.y),1.f))<.00001f,"actual enemy simulation uses calibrated treant gait");
}
