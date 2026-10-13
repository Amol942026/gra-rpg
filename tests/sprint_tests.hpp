#pragma once
void sprintTests(){
 for(int frame=0;frame<120;frame++)for(float side:{-1.f,1.f}){
  float phase=frame/120.f,hip=32.5f+.9f*std::cos(phase*4*AnimationPi);RunningLeg leg=runningLeg(phase,side,hip);
  check(std::abs(std::hypot(leg.kneeForward,leg.kneeHeight-hip)-17)<.001f&&std::abs(std::hypot(leg.kneeForward-leg.footForward,leg.kneeHeight-leg.footHeight)-17)<.001f,"running keeps both leg segments at their anatomical length throughout the cycle");
  check(leg.footHeight>=4&&leg.footHeight<=16.01f&&(!leg.planted||std::abs(leg.footHeight-4)<.001f),"running support foot stays on the ground and the recovery foot lifts without sinking");
 }
 for(float boundary:{0.f,.42f,.5f,.92f,1.f}){
  auto a=runningLeg(boundary-.00001f,1,32.5f),b=runningLeg(boundary+.00001f,1,32.5f);
  check(std::hypot(a.footForward-b.footForward,a.footHeight-b.footHeight)<.01f,"running foot trajectory stays continuous at contact and cycle boundaries");
 }
 auto arena=[](){Game g;g.props.clear();g.enemies.clear();g.forage.clear();g.player={20,20};return g;};
 float referencePhase=-1;
 for(int fps:{30,60,120}){
  Game g=arena();
  for(int i=0;i<fps;i++)g.update(1.f/fps,{1,0},false,true);
  check(g.animation.sprinting&&g.animation.runBlend>.99f&&g.animation.moving,"held Shift activates a distinct running gait during actual movement");
  if(referencePhase<0)referencePhase=g.animation.phase;
  else check(std::abs(g.animation.phase-referencePhase)<.001f,"running cycle and transition stay independent of update frequency");
  V before=g.player;g.update(1.f/fps,{1,0},false,false);
  check(!g.animation.sprinting&&g.animation.runBlend>0&&g.animation.runBlend<1&&distance(g.player,before)>0,"releasing Shift smoothly returns to walking while movement remains active");
  for(int i=0;i<fps;i++)g.update(1.f/fps,{1,0},false,false);
  check(g.animation.runBlend<.001f,"ordinary walking fully removes the running pose");
 }
 Game g=arena();g.update(.05f,{},false,true);
 check(!g.animation.sprinting&&g.animation.runBlend==0,"holding Shift while stationary never starts the running animation");
 g.props.push_back({g.player+V{.65f,0},0,1});
 for(int i=0;i<30;i++)g.update(.05f,{1,0},false,true);
 check(!g.animation.sprinting&&g.animation.runBlend<.001f,"a blocked movement stops the running gait instead of cycling against a wall");
 g=arena();for(int i=0;i<20;i++)g.update(.05f,{1,0},false,true);
 check(g.startRoll({1,0})&&!g.animation.sprinting&&g.animation.runBlend==0,"dodge immediately overrides running posture");
 for(int i=0;i<8;i++)g.update(.05f,{},false,true);
 check(!g.animation.sprinting&&g.animation.runBlend==0,"a completed dodge never leaves stale sprint posture while idle");
 g=arena();g.selectWeapon(1);g.beginBow({1,0});g.update(.05f,{1,0},false,true);
 check(g.animation.moving&&!g.animation.sprinting,"nocking a bow retains its aiming posture rather than the free running gait");
 g=arena();for(int i=0;i<20;i++)g.update(.05f,{1,0},false,true);g.hp=0;g.update(.05f,{1,0},false,true);
 check(!g.animation.sprinting&&g.animation.runBlend==0,"death clears sprint posture");
 g=arena();for(int i=0;i<20;i++)g.update(.05f,{1,0},false,true);g.allocationRequired=true;
 float blend=g.animation.runBlend,phase=g.animation.phase;g.update(.05f,{1,0},false,false);
 check(g.animation.runBlend==blend&&g.animation.phase==phase,"allocation pause freezes gait phase and transition along with gameplay");
}
