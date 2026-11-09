#pragma once
#include "enemy_ability_fixture.hpp"
void enemyAbilityTests(){
 check(enemyAbility(9)==EnemyAbility::CopperWave&&enemyAbility(10)==EnemyAbility::VenomFan&&enemyAbility(16)==EnemyAbility::ObsidianCleave&&enemyAbility(17)==EnemyAbility::EmberBreath,"two strongest ordinary enemy types in each realm have distinct abilities");
 check(enemyAbility(11)==EnemyAbility::None&&enemyAbility(18)==EnemyAbility::None&&enemyAbility(4)==EnemyAbility::None,"existing bosses and boar retain their separate mechanics");
 check(expandingWaveContact({4,0},{-5,0},1,2,.44f)<1,"expanding wave catches a fast inward crossing even when both frame endpoints miss the band");
 check(expandingWaveContact({0,5},{1,0},1,2,.44f)>1,"a hero outside the entire annulus path remains safe");
 for(int type:{9,10,16,17}){
  auto game=enemyAbilityArena(type);auto rng=game.rng,combat=game.combatRng;game.update(.05f,{},false);auto& enemy=game.enemies[0];
  check(enemy.abilityBusy()&&enemy.abilityCasts==1&&game.hp==100,"special move starts with a visible anticipation and no instant damage");
  const auto spec=enemyAbilityProfile(type);V start=enemy.pos;
  while(enemy.abilityAge>=0&&enemy.abilityAge<spec.windup-.08f)game.update(.02f,{},false);
  check(game.hp==100&&game.venomGlobs.empty()&&distance(enemy.pos,start)<.001f,"every windup locks ordinary movement and permits a reaction before its first hit");
  float clock=enemy.abilityAge,cooldown=enemy.abilityCd;game.allocationRequired=true;game.update(.05f,{1,0},true);
  check(enemy.abilityAge==clock&&enemy.abilityCd==cooldown,"allocation pause freezes both ability phase and cooldown");game.allocationRequired=false;
  game.update(0,{},false);check(enemy.abilityAge==clock,"zero elapsed time cannot release an ability");
  for(int frame=0;frame<300&&enemy.abilityBusy();frame++)game.update(.01f,{},false);
  check(!enemy.abilityBusy()&&enemy.abilityCasts==1&&enemy.abilityCd>0,"a completed cast has a recovery window and a real cooldown");
  check(game.rng==rng&&game.combatRng==combat,"ability timing and damage do not consume either random stream");
  float damage=type==17?game.incomingDamage(enemy.damage()*spec.damageScale/3,spec.damageType)*3:game.incomingDamage(enemy.damage()*spec.damageScale,spec.damageType);
  check(std::abs((100-game.hp)-damage)<.01f,"stationary target receives the intended finite single-cast damage");
  for(int fps:{30,60,120}){
   auto timed=enemyAbilityArena(type);for(int frame=0;frame<fps*4;frame++){timed.update(1.f/fps,{},false);if(timed.enemies[0].abilityCasts==1&&!timed.enemies[0].abilityBusy())break;}
   check(std::abs(timed.hp-game.hp)<.01f&&timed.enemies[0].abilityCasts==1,"special hit count and damage agree at 30/60/120 Hz");
  }
  auto escaped=enemyAbilityArena(type);escaped.update(.05f,{},false);V direction=escaped.enemies[0].abilityDirection;
  for(int frame=0;frame<300&&escaped.enemies[0].abilityBusy();frame++)escaped.update(.01f,{0,1},false);
  check(escaped.hp==100,"ordinary movement out of the telegraphed area avoids all damage");
  auto covered=enemyAbilityArena(type);covered.update(.05f,{},false);covered.props.push_back({covered.enemies[0].pos+V{1.5f,0},1,1});
  for(int frame=0;frame<300&&covered.enemies[0].abilityBusy();frame++)covered.update(.01f,{},false);
  check(covered.hp==100,"real scenery added after windup blocks venom, cleave, wave and breath");
  auto turning=enemyAbilityArena(type);turning.update(.05f,{},false);turning.player=turning.enemies[0].pos+V{0,5};turning.update(.1f,{},false);
  check(distance(direction,turning.enemies[0].abilityDirection)<.00001f,"committed ability direction does not home onto a sidestepping hero");
  auto killed=enemyAbilityArena(type);killed.update(.05f,{},false);killed.hit(killed.enemies[0],10000);killed.allocationRequired=false;float health=killed.hp;
  for(int frame=0;frame<180;frame++)killed.update(1.f/60,{},false);
  check(killed.hp==health&&killed.enemies[0].abilityAge<0&&killed.venomGlobs.empty(),"killing a caster interrupts its special attack and owned projectiles");
  auto travel=enemyAbilityArena(type);travel.update(.05f,{},false);travel.enterMageTower();
  check(!travel.enemies[0].abilityBusy()&&travel.venomGlobs.empty(),"teleport cancels active enemy abilities before preserving the old field");
  auto hidden=enemyAbilityArena(type);hidden.props.push_back({hidden.enemies[0].pos+V{1.5f,0},1,1});hidden.update(.05f,{},false);
  check(!hidden.enemies[0].abilityBusy()&&hidden.enemies[0].abilityCasts==0,"a hidden hero cannot trigger the special through a sight-blocking obstacle");
 }
 for(int type:{9,10,16,17}){
  auto close=enemyAbilityArena(type,1.1f);close.enemies[0].abilityCd=3;
  for(int frame=0;frame<360&&close.enemies[0].abilityCasts==0;frame++){close.hp=100;close.update(1.f/60,{},false);}
  check(close.enemies[0].abilityCasts==1,"normal initial cooldown still allows a special after the enemy enters melee range");
 }
 auto fan=enemyAbilityArena(10,1.1f);fan.update(.01f,{},false);
 while(fan.enemies[0].abilityBusy())fan.update(.01f,{},false);
 check(std::abs(100-fan.hp-fan.incomingDamage(fan.enemies[0].damage()*enemyAbilityProfile(10).damageScale,DamageType::Magic))<.01f,"overlapping venom bolts cannot triple-hit at melee range");
 auto dodge=enemyAbilityArena(16);dodge.enemyAbilityDamage(40,DamageType::Physical,dodge.player,true,.1f,0);
 check(dodge.hp==100,"new special attacks respect the protected portion of a real dodge");dodge.enemyAbilityDamage(40,DamageType::Physical,dodge.player,true,.35f,0);
 check(dodge.hp<100,"late dodge recovery does not grant permanent special-attack immunity");
 for(int type:{9,10,16,17}){Game wrong;wrong.props.clear();wrong.enemies.clear();wrong.player={18,15};wrong.enemies.push_back({8000,type,{15,15},{15,15},100,100});wrong.enemies[0].abilityCd=0;wrong.update(.05f,{},false);check(wrong.enemies[0].abilityCasts==0,"specials activate only in their own realm");}
}
