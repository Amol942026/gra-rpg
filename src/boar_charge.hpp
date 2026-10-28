// Included inside Game. The charge shares ordinary creature collision steps.
static void faceBoarCharge(Enemy& e,V direction){
 if(length(direction)<.001f)return;
 e.animation.face(direction.x,direction.y);
 e.animation.targetAngle=std::atan2((direction.x+direction.y)*.5f,direction.x-direction.y);
}
static void finishBoarCharge(Enemy& e){
 e.boarWindup=e.boarRush=0;e.boarRecovery=BoarChargeRecovery;
 e.attack=std::max(e.attack,BoarChargeRecovery);
}
void cancelBoarCharges(){
 for(auto& e:enemies){
  if(e.boarBusy())e.boarChargeCd=std::max(e.boarChargeCd,BoarChargeFirstDelay);
  e.boarWindup=e.boarRush=e.boarRecovery=0;e.boarDirection=e.boarAim={};e.boarHit=false;
 }
}
bool prepareBoarCharge(Enemy& e,float dt,bool visible,float range){
 if(location!=WorldLocation::Forest||e.type!=BoarType||!e.alive)return false;
 if(visible||e.boarBusy())e.boarChargeCd=std::max(0.f,e.boarChargeCd-dt);
 if(e.boarWindup>0){e.boarAim=player;faceBoarCharge(e,e.boarAim-e.pos);return true;}
 if(e.boarRush>0||e.boarRecovery>0)return true;
 if(dt>0&&visible&&range>=BoarChargeMinRange&&range<=EnemySightRange&&e.boarChargeCd<=.00001f&&e.attack<=0&&e.windup<=0&&e.strike<=0){
  e.boarAim=player;e.boarDirection=normalized(player-e.pos);e.boarHit=false;e.boarWindup=BoarChargeWindup;
  e.behavior=EnemyBehavior::Chase;e.patrolGoalActive=false;e.patrolBlocked=0;e.patrolWait=.7f;e.recoil={};
  faceBoarCharge(e,e.boarDirection);sound(Cue::EnemyAttack,e.pos);return true;
 }
 return false;
}
float boarObstacleTime(const Enemy& e,V delta)const{
 float first=2;const float radius=e.radius();
 auto contact=[&](V center,float gap){
  V offset=e.pos-center;
  if(std::abs(offset.x)>gap+std::abs(delta.x)||std::abs(offset.y)>gap+std::abs(delta.y))return 2.f;
  // Constraint resolution leaves a body exactly touching nearby scenery.
  // Contact behind or alongside the boar must allow it to move away.
  if(offset.x*offset.x+offset.y*offset.y<=gap*gap+.0002f&&offset.x*delta.x+offset.y*delta.y>=0)return 2.f;
  return segmentCircle(e.pos,delta,center,gap);
 };
 for(const auto& prop:props)first=std::min(first,contact(prop.pos,radius+propRadius(prop.type)+.002f));
 for(const auto& chest:chests)if(chest.location==location)first=std::min(first,contact(chest.pos,radius+ChestRadius+.002f));
 for(const auto& other:enemies)if(&other!=&e&&other.alive){
  first=std::min(first,contact(other.pos,radius+other.radius()+.002f));
 }
 const float low=MapWalkMin+radius,high=MapWalkMax-radius;
 if(delta.x>0)first=std::min(first,std::max(0.f,(high-e.pos.x)/delta.x));
 if(delta.x<0)first=std::min(first,std::max(0.f,(low-e.pos.x)/delta.x));
 if(delta.y>0)first=std::min(first,std::max(0.f,(high-e.pos.y)/delta.y));
 if(delta.y<0)first=std::min(first,std::max(0.f,(low-e.pos.y)/delta.y));
 return first;
}
void advanceBoarCharge(Enemy& e,float dt,V heroFrom,V heroTo,bool wasRolling,float rollAge){
 if(dt<=0)return;
 float remaining=dt,elapsed=0;
 if(e.boarWindup>0){
  float used=std::min(remaining,e.boarWindup);e.boarWindup=std::max(0.f,e.boarWindup-used);remaining-=used;elapsed+=used;
  if(e.boarWindup<.000001f){
   e.boarWindup=0;e.boarAim=heroFrom+(heroTo-heroFrom)*(elapsed/dt);e.boarDirection=normalized(e.boarAim-e.pos);
   e.boarChargeCd=BoarChargeCooldown;e.boarHit=false;
   if(length(e.boarDirection)<.5f||dead()){finishBoarCharge(e);}
   else{e.boarRush=BoarChargeDuration;e.boarCharges++;faceBoarCharge(e,e.boarDirection);e.animation.facingAngle=e.animation.targetAngle;sound(Cue::Swing,e.pos);}
  }
 }
 if(remaining>0&&e.boarRush>0){
  float used=std::min(remaining,e.boarRush);V delta=e.boarDirection*(BoarChargeSpeed*used);
  float obstacle=boarObstacleTime(e,delta);float travel=std::min(1.f,obstacle);
  V from=heroFrom+(heroTo-heroFrom)*(elapsed/dt),to=heroFrom+(heroTo-heroFrom)*((elapsed+used)/dt);
  float contact=segmentCircle(e.pos-from,delta-(to-from),{},e.radius()+HeroBodyRadius);
  if(!e.boarHit&&!dead()&&contact<=travel){
   e.boarHit=true;V heroAt=from+(to-from)*contact;
   if(wasRolling&&!sandTrapped()&&rollAge+elapsed+used*contact<RollProtection){sound(Cue::Dodge,heroAt);effects.push_back({heroAt,"UNIK",.65f,3});}
   else{float damage=incomingDamage(float(e.damage()),DamageType::Physical)*BoarChargeDamageMultiplier;hp=std::max(0.f,hp-damage);hurt=.25f;sound(dead()?Cue::Death:Cue::Hurt,heroAt);effects.push_back({heroAt,"-"+std::to_string(int(damage)),.8f,4});}
  }
  e.pos=e.pos+delta*travel;e.boarRush=std::max(0.f,e.boarRush-used);remaining-=used;elapsed+=used;
  if(obstacle<=1||e.boarRush<.000001f){if(obstacle<=1)sound(Cue::Step,e.pos);finishBoarCharge(e);}
 }
 if(remaining>0&&e.boarRecovery>0){
  e.boarRecovery=std::max(0.f,e.boarRecovery-remaining);
  if(e.boarRecovery<.000001f){e.boarRecovery=0;e.boarDirection=e.boarAim={};e.boarHit=false;}
 }
}
