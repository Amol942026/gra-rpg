#pragma once
// Included inside Game. Aimed, committed ballistic throws, independent of loot.
static BoulderSocket guardianSocket(const Enemy& e,float side,float forward,float up){
 const auto& s=e.animation;
 float angle=s.facingInitialized?s.facingAngle:int(s.direction)*3.14159265f/4;
 float fx=std::cos(angle),fy=std::sin(angle)/.52f,n=std::hypot(fx,fy);fx/=n;fy/=n;
 float dx=(side*fy+forward*fx)*GuardianBodyScale;
 float dy=(-side*fx+forward*fy)*.52f*GuardianBodyScale;
 float walk=s.motionInitialized?s.moveBlend:s.moving?1.f:0;
 float bob=(creature_motion::sample(2,s.phase).bob*walk+std::sin(s.idleTime*2.1f)*.25f*(1-walk))*.5f;
 return {e.pos+V{dx/80+dy/40,-dx/80+dy/40},(up+bob)*GuardianBodyScale};
}
static BoulderSocket boulderHand(const Enemy& e,float progress){
 // Central boulder socket shared by both grips, rendering, shadows and release.
 auto pose=guardian_motion::sample(progress);return guardianSocket(e,0,pose.rockForward,pose.rockUp);
}
void cancelBoulderAttacks(){
 cancelVolcanoAttacks();
 cancelBoarCharges();
 boulders.clear();boulderImpacts.clear();
 heroKnockback={};heroKnockbackRemaining=heroKnockdownRemaining=0;heroKnockdownFacing=0;
 for(auto& e:enemies){if(e.boulderWindup>0||e.boulderRecovery>0)e.boulderCd=std::max(e.boulderCd,BoulderFirstDelay);e.boulderWindup=e.boulderRecovery=0;}
}
float updateHeroKnockback(float dt){
 if(!knockedBack()||dt<=0)return 0;
 const float lockedTime=std::min(dt,std::max(heroKnockdownRemaining,heroKnockbackRemaining));
 heroKnockdownRemaining=std::max(0.f,heroKnockdownRemaining-dt);
 if(heroKnockdownRemaining<.00001f)heroKnockdownRemaining=0;
 const float step=std::min(dt,heroKnockbackRemaining);
 const float from=heroKnockbackRemaining/BoulderKnockbackDuration;
 heroKnockbackRemaining=std::max(0.f,heroKnockbackRemaining-step);
 const float to=heroKnockbackRemaining/BoulderKnockbackDuration;
 // Exact integral of a linearly slowing impulse; small collision steps also
 // respect chests and map edges without tunnelling through narrow scenery.
 V delta=heroKnockback*(from*from-to*to);
 int steps=std::max(1,int(std::ceil(length(delta)/.04f)));
 for(int i=0;i<steps;i++)move(delta*(1.f/steps),false);
 if(heroKnockbackRemaining<.00001f)heroKnockbackRemaining=0;
 if(!knockedBack()){heroKnockback={};heroKnockdownFacing=0;}
 return lockedTime;
}
bool updateGuardianThrow(Enemy& e,float dt,bool visible,float distanceToPlayer){
 if(location!=WorldLocation::Forest||e.type!=3||!e.alive)return false;
 if(visible)e.boulderCd=std::max(0.f,e.boulderCd-dt);
 if(e.boulderWindup>0){
  // Track the live hero through the lift, including displacement from a prior hit.
  // Commit only at release; the airborne projectile never homes.
  e.boulderTarget=player;e.animation.face(player.x-e.pos.x,player.y-e.pos.y);
  e.boulderWindup=std::max(0.f,e.boulderWindup-dt);
  if(e.boulderWindup<.00001f){
   e.boulderWindup=0;e.boulderRecovery=BoulderRecovery;e.boulderCd=BoulderCooldown;
   if(!dead()&&boulders.size()<MaxBoulders){
    auto socket=boulderHand(e,BoulderWindup/(BoulderWindup+BoulderRecovery));
    float flight=std::max(BoulderMinFlight,distance(socket.ground,e.boulderTarget)/BoulderSpeed);
    boulders.push_back({socket.ground,e.boulderTarget,socket.ground,0,flight,socket.height,socket.height,BoulderDamage,e.id});
    e.boulderThrows++;sound(Cue::Swing,e.pos);
   }
  }
  return true;
 }
 if(e.boulderRecovery>0){e.boulderRecovery=std::max(0.f,e.boulderRecovery-dt);return true;}
 if(dt>0&&visible&&distanceToPlayer>=BoulderMinRange&&distanceToPlayer<=BoulderMaxRange&&e.boulderCd<=.00001f&&e.windup<=0&&e.strike<=0
    &&boulders.size()<MaxBoulders&&clearEnemySegment(e.pos,player,BoulderRadius)){
  e.boulderTarget=player;e.boulderWindup=BoulderWindup;e.animation.face(player.x-e.pos.x,player.y-e.pos.y);
  e.behavior=EnemyBehavior::Chase;e.patrolGoalActive=false;e.patrolBlocked=0;e.patrolWait=.7f;
  sound(Cue::EnemyAttack,e.pos);return true;
 }
 return false;
}
void updateBoulders(float dt,V playerBefore,bool wasRolling=false,float rollElapsed=0){
 if(dt<=0||location!=WorldLocation::Forest)return;
 for(size_t i=0;i<boulders.size();){
  auto& b=boulders[i];float elapsed=std::min(dt,std::max(0.f,b.duration-b.age));
  float next=std::min(b.duration,b.age+elapsed),progress=std::clamp(next/b.duration,0.f,1.f);
  V end=b.origin+(b.target-b.origin)*progress,delta=end-b.pos;float block=2;
  for(const auto& p:props)block=std::min(block,segmentCircle(b.pos,delta,p.pos,propRadius(p.type)+BoulderRadius));
  if(block<=1||next>=b.duration-.00001f){
   const bool blocked=block<=1;V impact=blocked?b.pos+delta*block:b.target;float impactTime=elapsed*(blocked?block:1);
   float amount=b.damage;V incoming=end-b.origin;bool dodged=wasRolling&&rollElapsed+impactTime<RollProtection;
   boulders.erase(boulders.begin()+i);
   if(boulderImpacts.size()<MaxBoulderImpacts)boulderImpacts.push_back({impact});sound(Cue::StoneHit,impact);
   // Interpolate the actual hero movement at landing, independent of FPS.
   V heroAtImpact=playerBefore+(player-playerBefore)*std::clamp(impactTime/dt,0.f,1.f);
   float proximity=distance(heroAtImpact,impact);
   if(!blocked&&!dead()&&proximity<=BoulderKnockbackRadius&&clearEnemySegment(impact,heroAtImpact,0)){
    if(dodged){sound(Cue::Dodge,player);effects.push_back({player,"UNIK",.65f,3});}
    else{
     if(proximity<=BoulderImpactRadius){float damage=incomingDamage(amount,DamageType::Physical);hp=std::max(0.f,hp-damage);sound(dead()?Cue::Death:Cue::Hurt,player);effects.push_back({player,"-"+std::to_string(int(damage)),.8f,4});}
     if(!dead()){
      V away=heroAtImpact-impact;if(length(away)<.001f)away=incoming;if(length(away)<.001f)away={1,0};
      heroKnockback=normalized(away)*(BoulderKnockbackDistance*(1-.6f*std::clamp(proximity/BoulderKnockbackRadius,0.f,1.f)));
      heroKnockbackRemaining=BoulderKnockbackDuration;heroKnockdownRemaining=HeroKnockdownDuration;
      heroKnockdownFacing=animation.facingInitialized?animation.facingAngle:int(animation.direction)*3.14159265f/4;player=heroAtImpact;
      facing=normalized(away)*-1;animation.face(facing.x,facing.y);animation.targetAngle=std::atan2((facing.x+facing.y)*.5f,facing.x-facing.y);
      cancelBow();rollRemaining=0;swing=0;swingStrong=false;animation.moving=false;animation.stopSprint();stepDistance=0;
      hurt=BoulderKnockbackDuration;updateHeroKnockback(std::max(0.f,dt-impactTime));
     }
    }
   }
  }else{
   // The bottom of the visible stone reaches the ground at impact, rather
   // than its centre sinking through the terrain before the fracture starts.
   b.age=next;b.pos=end;b.height=guardian_break::RestHeight+(1-progress)*(b.startHeight-guardian_break::RestHeight)+std::sin(progress*3.14159265f)*40;i++;
  }
 }
}
