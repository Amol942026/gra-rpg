#pragma once

// Included inside Game. The existing saved swing clock carries the windup
// across saves. Crossing the impact time resolves the thrust exactly once.
bool spearActive()const{return meleeDuration==SpearDuration&&swing>0&&
                               archetype(equipment[0].kind)==Kind::Spear;}
void cancelTimedWeaponAttack(){
 if(spearActive()||meleeDuration==AxeAttackDuration||meleeDuration==DaggerAttackDuration||activePowerMelee()!=PowerStyle::Count)swing=0;
}
Cue spearSound(bool impact)const{
 return Cue(int(Cue::SpearThrust)+2*(int(equipment[0].kind)-FirstSpear)+int(impact));
}
void faceThrust(){
 if(length(facing)<.001f)facing={0,-1};
 animation.face(facing.x,facing.y);
 animation.targetAngle=std::atan2((facing.x+facing.y)*.5f,facing.x-facing.y);
}
bool startSpear(){
 faceThrust();
 animation.stopSprint();animation.swordStow=0;
 attackCd=SpearCooldown;meleeDuration=swing=SpearDuration;swingStrong=false;
 sound(spearSound(false),player);
 return true;
}
bool startAxeOrDagger(Kind weapon){
 const bool axe=weapon==Kind::Axe;
 if(!axe)faceThrust();
 animation.swordStow=0;
 attackCd=.46f;
 meleeDuration=swing=axe?AxeAttackDuration:DaggerAttackDuration;
 swingStrong=false;
 sound(Cue::Swing,player);
 return true;
}
SpearContact spearContact(bool charged)const{
 const auto kind=equipment[0].kind;
 if(!spearVariant(kind))return {{},0};
 const auto tip=spear_motion::impactTip(animation,kind,charged);
 const float angle=animation.facingInitialized?animation.facingAngle:int(animation.direction)*AnimationPi/4;
 const auto forward=actorFacing(angle);
 // The flat screen delta uses the same facing and depth squash as Rig.
 // Invert the isometric basis to test the tip against world colliders.
 const float x=tip.x*forward.y+tip.y*forward.x;
 const float y=(tip.y*forward.y-tip.x*forward.x)*ActorGroundSquash;
 return {{x/80+y/40,-x/80+y/40},tip.z};
}
float weaponContactBlock(V delta,float padding)const{
 float block=2.f;
 auto stopAt=[&](V center,float radius){block=std::min(block,segmentCircle(player,delta,center,radius+padding));};
 for(const auto& prop:props)stopAt(prop.pos,propRadius(prop.type));
 for(const auto& chest:chests)if(chest.location==location)stopAt(chest.pos,ChestRadius);
 if(hasFountains())for(V center:fountains())stopAt(center,FountainRadius);
 const V end=player+delta;
 if(end.x<MapWalkMin||end.x>MapWalkMax)block=std::min(block,(std::clamp(end.x,MapWalkMin,MapWalkMax)-player.x)/delta.x);
 if(end.y<MapWalkMin||end.y>MapWalkMax)block=std::min(block,(std::clamp(end.y,MapWalkMin,MapWalkMax)-player.y)/delta.y);
 return block;
}
void updateTimedWeaponAttack(float previousSwing){
 if(previousSwing<=0||sandTrapped()||knockedBack()||rolling()||inTower())return;
 const Kind weapon=archetype(equipment[0].kind);
 const bool spear=weapon==Kind::Spear&&meleeDuration==SpearDuration;
 const bool axe=weapon==Kind::Axe&&meleeDuration==AxeAttackDuration;
 const bool dagger=weapon==Kind::Dagger&&meleeDuration==DaggerAttackDuration;
 if(!spear&&!axe&&!dagger)return;
 const int impactFrame=spear?SpearImpactFrame:axe?AxeImpactFrame:DaggerImpactFrame;
 const float impactRemaining=meleeDuration-impactFrame/60.f+.00001f;
 const bool impact=previousSwing>impactRemaining&&swing<=impactRemaining;
 const float secondRemaining=meleeDuration-SkillDaggerSecond+.00001f;
 const bool second=dagger&&swingStrong&&previousSwing>secondRemaining&&swing<=secondRemaining;
 if(!impact&&!second)return;
 if(swingStrong){
  const Skill skill=skillForSlot(0);
  const float rankBonus=.1f*std::max(0,skillRank(skill)-1);
  const V axis=normalized(facing);
  const bool critical=rollCritical();
  const float amount=float(damage())*((axe?1.65f:spear?1.40f:.90f)+rankBonus);
  if(axe){
   for(auto& enemy:enemies)if(enemy.alive){
    const V offset=enemy.pos-player;
    const float reach=length(offset);
    if(reach<=SkillAxeReach&&offset.x*axis.x+offset.y*axis.y>=reach*.32f&&weaponContactBlock(offset,.04f)>1)
     hit(enemy,amount,DamageType::Physical,critical);
   }
   return;
  }
  const V ray=spear?spearContact(false).ray:axis*SkillDaggerReach;
  const float padding=spear?0.f:SpearTipRadius;
  const float block=weaponContactBlock(ray,padding);
  Enemy* first=nullptr;float closest=block;
  for(auto& enemy:enemies)if(enemy.alive){
   const float t=segmentCircle(player,ray,enemy.pos,enemy.radius()+padding);
   if(t>1||t>=block)continue;
   if(spear)hit(enemy,amount,DamageType::Physical,critical,ray);
   else if(t<closest){closest=t;first=&enemy;}
  }
  if(first)hit(*first,amount,DamageType::Physical,critical,ray);
  if(spear)sound(spearSound(true),player+ray*std::min(1.f,block));
  if(second)sound(Cue::Swing,player);
  return;
 }
 if(axe){hitMeleeArea(float(damage()));return;}
 const V delta=spear?spearContact(false).ray:normalized(facing)*MeleeRange;
 const float padding=spear?0.f:SpearTipRadius;
 const float block=weaponContactBlock(delta,padding);
 Enemy* first=nullptr;float contact=block;
 for(auto& enemy:enemies)if(enemy.alive){
  const float t=segmentCircle(player,delta,enemy.pos,enemy.radius()+padding);
  if(t<contact){contact=t;first=&enemy;}
 }
 if(contact>1.f)return;
 if(spear)sound(spearSound(true),player+delta*std::clamp(contact,0.f,1.f));
 if(first)hit(*first,float(damage()),DamageType::Physical,rollCritical(),delta);
}
