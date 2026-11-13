#pragma once
// Included inside Game.
bool canHoldAttack()const{
 return !dead()&&!sandTrapped()&&!knockedBack()&&!allocationRequired&&!storyBlocked()&&!inTower()&&!rolling()&&attackCd<=0;
}
bool holdingMelee()const{
 return heldAttackWeapon.count>0&&equipment[0].kind==heldAttackWeapon.kind&&equipment[0].tier==heldAttackWeapon.tier;
}
void cancelMeleeHold(){heldAttackWeapon={};heldAttackTime=0;}
bool beginMeleeHold(V aim){
 if(!canHoldAttack()||holdingMelee()||archetype(equipment[0].kind)==Kind::Bow)return false;
 // Keep the unarmed short tap available with its existing fallback attack.
 heldAttackWeapon=equipment[0].empty()?item(Kind::None,0,1):equipment[0];
 heldAttackTime=0;aimMelee(aim);
 return true;
}
PowerStyle activePowerMelee()const{
 auto style=powerStyle(archetype(equipment[0].kind));
 return style!=PowerStyle::Count&&style!=PowerStyle::Bow&&swing>0&&meleeDuration==powerAttack(style).duration?style:PowerStyle::Count;
}
PowerStyle activePowerVisual()const{
 return lastBowPower&&bowReleaseTime>0&&archetype(equipment[0].kind)==Kind::Bow?PowerStyle::Bow:activePowerMelee();
}
float powerVisualAge()const{
 return activePowerVisual()==PowerStyle::Bow?1-bowReleaseTime/powerAttack(PowerStyle::Bow).duration:swingProgress();
}
float powerWindupProgress()const{
 if(!canHoldAttack())return -1;
 if(bowDrawing&&!piercingArrowArmed)return std::min(1.f,bowDrawTime/PowerHoldTime);
 if(holdingMelee()&&powerStyle(archetype(equipment[0].kind))!=PowerStyle::Count)return heldAttackTime/PowerHoldTime;
 return -1;
}
float powerHoldFraction()const{return std::max(0.f,powerWindupProgress());}
float visibleBowCharge()const{
 return bowDrawing?bowCharge():bowReleaseTime>0?lastBowCharge*bowReleaseTime/(lastBowPower?powerAttack(PowerStyle::Bow).duration:.18f):-1;
}
bool releaseMeleeHold(V aim){
 const bool held=holdingMelee();const float timeHeld=heldAttackTime;
 cancelMeleeHold();
 if(!held||!canHoldAttack())return false;
 const auto style=powerStyle(archetype(equipment[0].kind));
 if(timeHeld<PowerHoldTime||style==PowerStyle::Count)return attack(aim);
 const auto& attack=powerAttack(style);
 aimMelee(aim);faceThrust();animation.stopSprint();animation.swordStow=0;
 meleeDuration=swing=attack.duration;attackCd=attack.cooldown/powerAttackRate(style);swingStrong=false;
 sound(Cue(int(Cue::PowerSword)+int(style)),player);
 return true;
}
void updatePowerMelee(float previousSwing){
 const auto style=activePowerMelee();
 if(style==PowerStyle::Count||sandTrapped()||knockedBack()||rolling()||inTower())return;
 const auto& attack=powerAttack(style);
 auto crossed=[&](float at){const float clock=attack.duration-at+.00001f;return previousSwing>clock&&swing<=clock;};
 if(!crossed(attack.contact)&&!(attack.second>0&&crossed(attack.second)))return;
 const V axis=normalized(facing);
 const float amount=float(damage())*attack.multiplier;
 const bool critical=rollCritical();
 if(style==PowerStyle::Sword||style==PowerStyle::Axe){
  const float travel=weaponContactBlock(axis*1.42f,.10f);
  const V center=player+axis*(1.42f*std::min(1.f,travel));
  for(auto& enemy:enemies)if(enemy.alive){
   const V offset=enemy.pos-player;
   const float along=offset.x*axis.x+offset.y*axis.y;
   const bool area=style==PowerStyle::Sword?along>=distance(player,enemy.pos)*.26f&&length(offset)<=attack.reach+enemy.radius()*.25f
                     :distance(center,enemy.pos)<=1.18f+enemy.radius()*.35f;
   if(area&&weaponContactBlock(offset,.04f)>1)hit(enemy,amount,DamageType::Physical,critical);
  }
  return;
 }
 const V ray=axis*attack.reach;
 const float block=weaponContactBlock(ray,SpearTipRadius);
 Enemy* first=nullptr;Enemy* second=nullptr;float near=block,far=block;
 for(auto& enemy:enemies)if(enemy.alive){
  const float t=segmentCircle(player,ray,enemy.pos,enemy.radius()+SpearTipRadius);
  if(t<near){far=near;second=first;near=t;first=&enemy;}
  else if(t<far){far=t;second=&enemy;}
 }
 if(first&&near<=1)hit(*first,amount,DamageType::Physical,critical);
 if(style==PowerStyle::Spear&&second&&far<=1)hit(*second,amount*.62f,DamageType::Physical,critical);
}
