// Included inside Game. The charge shares ordinary creature collision steps.
static void faceCharge(Enemy& e,V direction){
 if(length(direction)<.001f)return;
 e.animation.face(direction.x,direction.y);
 e.animation.targetAngle=std::atan2((direction.x+direction.y)*.5f,direction.x-direction.y);
}
static void finishCharge(Enemy& e){
 const float recovery=chargeProfile(e.type).recovery;
 e.boarWindup=e.boarRush=0;e.boarRecovery=recovery;
 e.attack=std::max(e.attack,recovery);
}
void cancelCharges(){
 for(auto& e:enemies){
  if(e.chargeBusy())e.boarChargeCd=std::max(e.boarChargeCd,BoarChargeFirstDelay);
  e.boarWindup=e.boarRush=e.boarRecovery=0;e.boarDirection=e.boarAim={};e.boarHit=false;
 }
}
bool prepareCharge(Enemy& e,float dt,bool visible,float range){
 if(!e.alive||!((e.type==BoarType&&location==WorldLocation::Forest)||(e.type==CinderhornType&&inVolcano())))return false;
 const auto& charge=chargeProfile(e.type);
 if(visible||e.chargeBusy())e.boarChargeCd=std::max(0.f,e.boarChargeCd-dt);
 if(e.boarWindup>0){e.boarAim=player;faceCharge(e,e.boarAim-e.pos);return true;}
 if(e.boarRush>0||e.boarRecovery>0)return true;
 if(dt>0&&visible&&range>=charge.minRange&&range<=EnemySightRange&&e.boarChargeCd<=.00001f&&e.attack<=0&&e.windup<=0&&e.strike<=0){
  e.boarAim=player;e.boarDirection=normalized(player-e.pos);e.boarHit=false;e.boarWindup=charge.windup;
  e.behavior=EnemyBehavior::Chase;e.patrolGoalActive=false;e.patrolBlocked=0;e.patrolWait=.7f;e.recoil={};
  faceCharge(e,e.boarDirection);if(e.type==BoarType)sound(enemyAttackCue(e.type),e.pos);return true;
 }
 return false;
}
float chargeObstacleTime(const Enemy& e,V delta)const{
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
 if(inVolcano()){
  for(V fountain:VolcanoFountains)first=std::min(first,contact(fountain,radius+FountainRadius+.002f));
  // A motion substep is short, but refine the bank contact before applying
  // the sweep against the hero. A target across lava stays out of reach.
  const float limit=std::min(1.f,first);
  const int samples=std::max(4,int(std::ceil(length(delta)*limit/.15f)));
  for(int i=1;i<=samples;i++){
   float high=limit*float(i)/samples;
   if(!volcanoLava(e.pos+delta*high,radius+.002f))continue;
   float low=limit*float(i-1)/samples;
   for(int n=0;n<8;n++){
    const float mid=(low+high)*.5f;
    if(volcanoLava(e.pos+delta*mid,radius+.002f))high=mid;
    else low=mid;
   }
   first=std::min(first,low);break;
  }
 }
 return first;
}
void advanceCharge(Enemy& e,float dt,V heroFrom,V heroTo,bool wasRolling,float rollAge){
 if(dt<=0)return;
 const auto& charge=chargeProfile(e.type);
 float remaining=dt,elapsed=0;
 if(e.boarWindup>0){
  float used=std::min(remaining,e.boarWindup);e.boarWindup=std::max(0.f,e.boarWindup-used);remaining-=used;elapsed+=used;
  if(e.boarWindup<.000001f){
   e.boarWindup=0;e.boarAim=heroFrom+(heroTo-heroFrom)*(elapsed/dt);e.boarDirection=normalized(e.boarAim-e.pos);
   e.boarChargeCd=charge.cooldown;e.boarHit=false;
   if(length(e.boarDirection)<.5f||dead()){finishCharge(e);}
   else{
    e.boarRush=charge.duration;e.boarCharges++;faceCharge(e,e.boarDirection);e.animation.facingAngle=e.animation.targetAngle;
    if(e.type==CinderhornType)e.animation.phase=0;
    sound(e.type==CinderhornType?Cue::CinderhornCharge:Cue::BeastAttack,e.pos);
   }
  }
 }
 if(remaining>0&&e.boarRush>0){
  float used=std::min(remaining,e.boarRush);V delta=e.boarDirection*(charge.speed*used);
  float obstacle=chargeObstacleTime(e,delta);float travel=std::min(1.f,obstacle);
  V from=heroFrom+(heroTo-heroFrom)*(elapsed/dt),to=heroFrom+(heroTo-heroFrom)*((elapsed+used)/dt);
  float contact=segmentCircle(e.pos-from,delta-(to-from),{},e.radius()+HeroBodyRadius);
  bool stoppedOnHero=false;
  if(!e.boarHit&&!dead()&&contact<=travel){
   e.boarHit=true;V heroAt=from+(to-from)*contact;
   if(wasRolling&&!sandTrapped()&&rollAge+elapsed+used*contact<RollProtection){sound(Cue::Dodge,heroAt);effects.push_back({heroAt,"UNIK",.65f,3});}
   else{
    takeMonsterDamage(incomingDamage(float(e.damage()),DamageType::Physical)*charge.multiplier,heroAt,.25f);
    if(e.type==CinderhornType){sound(Cue::CinderhornAttack,e.pos+delta*contact);travel=contact;stoppedOnHero=true;}
   }
  }
  e.pos=e.pos+delta*travel;e.boarRush=std::max(0.f,e.boarRush-used);remaining-=used;elapsed+=used;
  if(obstacle<=1||e.boarRush<.000001f||stoppedOnHero){
   if(obstacle<=1)sound(Cue::Step,e.pos);
   finishCharge(e);
  }
 }
 if(remaining>0&&e.boarRecovery>0){
  e.boarRecovery=std::max(0.f,e.boarRecovery-remaining);
  if(e.boarRecovery<.000001f){e.boarRecovery=0;e.boarDirection=e.boarAim={};e.boarHit=false;}
 }
}
