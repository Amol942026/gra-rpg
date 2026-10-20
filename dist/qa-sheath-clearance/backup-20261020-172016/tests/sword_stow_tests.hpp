#pragma once
#include "../src/hero_sheath.hpp"
void swordStowTests(){
 for(int fps:{30,60,120}){
  AnimationState a;for(int f=0;f<fps;f++)a.advance(.03f,0,1.f/fps,true);
  check(a.swordStow==1,"sprinting completes the sheath timeline independently of FPS");
  for(int f=0;f<fps;f++)a.advance(.02f,0,1.f/fps,false);
  check(a.swordStow==0,"returning to walk completely draws the sword");
 }
 AnimationState a;a.advance(.1f,0,.2f,true);float before=a.swordStow;
 a.advance(.1f,0,.01f,false);check(a.swordStow<before&&before-a.swordStow<.02f,"releasing Shift reverses the current timeline without resetting its pose");
 auto previous=hero_motion::evaluate(.1f,1,0,0);
 for(int f=0;f<=240;f++){
  float phase=f/240.f;auto base=hero_motion::evaluate(.1f,1,phase,0),p=base;auto key=hero_sheath::apply(p,phase);
  for(int b=hero_motion::data::ThighL;b<=hero_motion::data::FootR;b++)check(hero_motion::length(p.world[b].p-base.world[b].p)<.00001f,"sheathing preserves locomotion and exact leg/foot transforms");
  using namespace hero_motion;auto shoulder=p.world[data::UpperArmR].p,elbow=p.world[data::ForearmR].p,hand=p.world[data::HandR].p;
  check(std::abs(length(shoulder-elbow)-13)<.003f&&std::abs(length(elbow-hand)-13)<.003f,"sheathing arm IK preserves both rigid anatomical lengths");
  auto rest=sword_pose::idleBlade(0);auto blade=p.vector(data::Weapon,{rest.r,rest.f,rest.z});check(std::abs(length(blade)-37)<.003f,"insertion clips the exposed blade without shrinking the physical sword");
  if(key.reach>.99999f&&key.alignment>.99999f){auto m=sword_stow::Mouth;auto mouth=p.point(data::Pelvis,{m.r,m.f,m.z});check(length(hand+blade*sword_stow::visibleFraction(key)-mouth)<.5f,"gripping hand and blade stay aligned with the scabbard throughout insertion");}
  if(key.insertion>=.999999f){auto m=sword_stow::Mouth;auto mouth=p.point(data::Pelvis,{m.r,m.f,m.z});Vec tip=p.world[data::Weapon].p+blade*sword_stow::visibleFraction(key);check(length(mouth-tip)<.003f,"seated weapon and exposed blade end at the actual scabbard mouth");}
  if(f)check(length(p.world[data::Weapon].p-previous.world[data::Weapon].p)<2.5f,"weapon attachment remains continuous through insertion and hand release");
  previous=p;
 }
 Game g;g.props.clear();g.enemies.clear();g.forage.clear();g.player={20,20};g.animation.swordStow=1;
 for(int gait=0;gait<16;gait++)for(int f=23;f<=45;f++){
  float phase=f/60.f;auto p=hero_motion::evaluate(gait/16.f,1,phase,0);auto key=hero_sheath::apply(p,phase);auto m=sword_stow::Mouth;auto blade=sword_pose::idleBlade(0);
  auto mouth=p.point(hero_motion::data::Pelvis,{m.r,m.f,m.z});auto tip=p.world[hero_motion::data::HandR].p+p.vector(hero_motion::data::Weapon,{blade.r,blade.f,blade.z})*sword_stow::visibleFraction(key);
  check(hero_motion::length(tip-mouth)<.65f,"gait variations preserve hand/blade alignment during physical insertion");
 }
 check(g.attack()&&g.animation.swordStow==0,"an attack interrupts sheathing immediately without delaying combat");
 g.swing=g.attackCd=0;g.animation.swordStow=.8f;g.allocationRequired=true;g.update(.1f,{1,0},false,true);
 check(g.animation.swordStow==.8f,"allocation pause freezes the weapon timeline");
}
