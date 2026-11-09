#pragma once
#include "../src/bandit_gait.hpp"
void treantTests(){
 using namespace ember::grounded_gait;
 for(int type:{7,6}){
  const auto& contact=type==7?ember::treant_gait::Contact:Bandit;
  for(int heading=0;heading<8;heading++)for(int fps:{30,60,120})for(float speed:{enemySpeed(type),enemySpeed(type)*PatrolSpeedFactor}){
   float angle=heading*AnimationPi/4;
   float fx=std::cos(angle),fy=std::sin(angle)/.52f,n=std::hypot(fx,fy);fx/=n;fy/=n;
   V direction=normalized({(std::cos(angle)/40+std::sin(angle)/20)*.5f,(std::sin(angle)/20-std::cos(angle)/40)*.5f});
   V step=direction*(speed/fps);
   AnimationState state;state.phase=.08f;state.moveBlend=1;state.motionInitialized=true;
   float maxSlip=0;int contactSamples=0;
   for(int frame=0;frame<fps*3;frame++){
    auto before=state;advance(state,step.x,step.y,1.f/fps,contact);
    const auto previous=pose(before,1,contact),current=pose(state,1,contact);
    for(int side=0;side<2;side++){
     const auto& a=previous.legs[side];const auto& b=current.legs[side];
     float p0=std::fmod(before.phase+side*.5f,1.f),p1=std::fmod(state.phase+side*.5f,1.f);
     if(p1>=p0&&p0<contact.support-.02f&&p1<contact.support-.02f){
      float travel=(b.forward-a.forward)*contact.scale;
      float sx=(step.x-step.y)*40+travel*fx,sy=(step.x+step.y)*20+travel*fy*.52f;
      maxSlip=std::max(maxSlip,std::hypot(sx,sy));contactSamples++;
     }
    }
   }
   check(contactSamples>fps&&maxSlip<.003f,"support foot stays planted at every direction, speed and FPS");
   float expected=std::fmod(.08f+phaseDistance(step.x*fps*3,step.y*fps*3,contact),1.f);
   check(std::abs(std::remainder(state.phase-expected,1.f))<.00005f,"cadence follows ground distance, not elapsed time");
  }
  AnimationState state;
  for(int frame=0;frame<=240;frame++)for(float blend:{0.f,.4f,1.f}){
   state.phase=frame/240.f;
   const auto gait=pose(state,blend,contact);
   for(const auto& leg:gait.legs){
    const auto k=leg.joint;
    check(std::abs(std::hypot(k.forward,k.height-gait.hip)-contact.legLength)<.001f&&
          std::abs(std::hypot(leg.forward-k.forward,leg.height-k.height)-contact.legLength)<.001f,
          "upper and lower leg lengths stay constant through walking and the idle transition");
    const float dz=leg.height-gait.hip,distance=std::hypot(leg.forward,dz);
    const float bend=(leg.forward*(k.height-gait.hip)-dz*k.forward)/distance;
    const float minBend=type==7?.12f:.04f;
    check(bend>=minBend*contact.legLength,"knees bend forward and never lock");
   }
  }
  for(int side=0;side<2;side++){
   state.phase=contact.support*.5f+side*.5f;
   const auto gait=pose(state,1,contact);
   const auto& planted=gait.legs[side];const auto& swing=gait.legs[1-side];
   const float extension=std::hypot(planted.forward,planted.height-gait.hip);
   const float minExtension=type==7?1.96f:1.99f,maxExtension=type==7?1.986f:1.999f;
   check(std::abs(planted.height-FootHeight)<.0001f&&extension>minExtension*contact.legLength&&extension<maxExtension*contact.legLength,
         "the weight-bearing leg reaches its species-specific extension without locking");
   check(swing.height>FootHeight+1&&swing.joint.forward>swing.forward*.5f+1,
         "the other knee flexes to clear the ground");
   if(type==6){
    state.phase=.21f+side*.5f;
    const auto transfer=pose(state,1,contact);
    const auto& support=transfer.legs[side];
    check(std::abs(support.height-FootHeight)<.0001f&&
          std::hypot(support.forward,support.height-transfer.hip)>1.98f*contact.legLength&&
          transfer.legs[1-side].height>FootHeight+1,
          "the bandit keeps an extended support leg as the trailing foot lifts");
   }
  }
  state.phase=.23f;
  for(int i=0;i<60;i++)advance(state,0,0,1.f/60,contact);
  check(state.phase==.23f&&!state.moving,"a blocked or attacking enemy cannot walk in place");
  Game g;g.props.clear();g.enemies.clear();g.player={17,17};
  g.enemies.push_back({91,type,{14,14},{14,14},170,170});
  auto& enemy=g.enemies[0];enemy.attack=1000;
  V before=enemy.pos;float phase=enemy.animation.phase;
  g.update(.05f,{},false);V moved=enemy.pos-before;
  check(length(moved)>0&&std::abs(enemy.animation.phase-std::fmod(phase+phaseDistance(moved.x,moved.y,contact),1.f))<.00001f,
        "actual enemy simulation uses the resolved ground distance");
 }
 AnimationState state;
 for(int frame=0;frame<=120;frame++){
  state.phase=frame/120.f;
  state.idleTime=frame/60.f;
  const auto standing=bandit_gait::pose(state,0);
  for(const auto& leg:standing.legs){
   const float halfAngle=std::acos((standing.hip-leg.height)/(2*Bandit.legLength));
   check(leg.height==FootHeight&&leg.forward==0&&halfAngle<AnimationPi/60,
         "an idle bandit stands with planted boots and less than six degrees of knee flexion");
  }
  for(float walk:{0.f,.01f,.25f,.5f,.75f,1.f}){
   for(float load:{0.f,-1.1f,-1.5f}){
    auto gait=bandit_gait::pose(state,walk,load);
    for(const auto& leg:gait.legs){
     check(leg.height>=FootHeight&&
           std::abs(std::hypot(leg.joint.forward,leg.joint.height-gait.hip)-Bandit.legLength)<.002f&&
           std::abs(std::hypot(leg.forward-leg.joint.forward,leg.height-leg.joint.height)-Bandit.legLength)<.002f,
           "bandit walking blend and sword action preserve contact and both bone lengths");
    }
   }
  }
 }
 auto ready=bandit_gait::sample(bandit_motion::windup,0.f);
 auto release=bandit_gait::sample(bandit_motion::windup,1.f);
 auto strike=bandit_gait::sample(bandit_motion::strike,0.f);
 check(ready.arm==0&&release.arm==strike.arm&&release.hip==strike.hip,
       "the bandit's windup releases into the swing without jumping joints");

 for(int type:{1,2}){
  const auto& contact=type==1?forest_gait::Wolf:forest_gait::Skeleton;
  for(int heading=0;heading<8;heading++){
   const float angle=heading*AnimationPi/4;
   V dir=normalized({(std::cos(angle)/40+std::sin(angle)/20)*.5f,
                     (std::sin(angle)/20-std::cos(angle)/40)*.5f});
   V step=dir*(enemySpeed(type)/60);
   AnimationState a;a.phase=.12f;a.moveBlend=1;a.motionInitialized=true;
   auto b=a;advance(b,step.x,step.y,1.f/60,contact);
   auto old=type==1?forest_gait::wolfLeg(a.phase,1,false,true,28):fromStanding(a,1,contact).legs[0];
   auto next=type==1?forest_gait::wolfLeg(b.phase,1,false,true,28):fromStanding(b,1,contact).legs[0];
   float projected=2*contact.stride*phaseDistance(step.x,step.y,contact)/contact.support;
   check(old.height==next.height&&std::abs(next.forward-old.forward+projected)<.0001f,
         "wolf and skeleton stance travel cancels actual ground displacement");
  }
 }
 for(int frame=0;frame<120;frame++)for(bool front:{false,true})for(bool right:{false,true}){
  const float hip=front?28.5f:25.5f;
  const auto leg=forest_gait::wolfLeg(frame/120.f,1,right,front,hip);
  const auto joint=leg.joint;
  check(leg.height>=forest_gait::PawHeight&&
        std::abs(std::hypot(joint.forward,joint.height-hip)-forest_gait::Wolf.legLength)<.001f&&
        std::abs(std::hypot(leg.forward-joint.forward,leg.height-joint.height)-forest_gait::Wolf.legLength)<.001f,
        "wolf four-paw cycle preserves anatomical segment lengths and ground clearance");
 }
}
