#pragma once
void powerAttackTests(){
 auto setup=[](Kind kind){
  Game g=emptyArena();g.props.clear();g.enemies.clear();g.equipment[0]=item(kind);
  g.attributes={};return g;
 };
 auto enemy=[](Game& g,int id,V at){
  const V p=g.player+at;
  g.enemies.push_back({id,2,p,p,500,500});
  g.enemies.back().windup=4;
 };
 auto advance=[](Game& g,int frames){for(int i=0;i<frames;i++)g.update(1.f/60,{},false);};
 for(Kind kind:{Kind::Sword,Kind::Axe,Kind::Dagger,Kind::Spear}){
  auto g=setup(kind);
  const auto style=powerStyle(kind);const auto& profile=powerAttack(style);
  enemy(g,1,{1.55f,0});enemy(g,2,{1.56f,1.25f});enemy(g,3,{-1.55f,0});
  check(g.beginMeleeHold({1,0})&&g.swing==0,"mouse press holds a melee attack without damage");
  advance(g,36);
  check(g.powerHoldFraction()==1&&g.enemies[0].hp==500,"holding saturates the meter without auto-attacking");
  check(g.releaseMeleeHold({1,0})&&!g.releaseMeleeHold({1,0})&&g.activePowerMelee()==style,
        "one release commits the matching weapon power attack");
  check(!g.sounds.empty()&&g.sounds.back().cue==Cue(int(Cue::PowerSword)+int(style)),"power attacks have separate weapon audio events");
  const float rate=powerAttackRate(style);
  advance(g,int(profile.contact*60/rate)-2);
  check(g.enemies[0].hp==500,"damage waits for the first visual contact");
  advance(g,4);const float hp=g.enemies[0].hp;
  check(hp<500&&g.enemies[2].hp==500,"forward contact damages its target without hitting behind the player");
  advance(g,20);
  check(style==PowerStyle::Dagger?g.enemies[0].hp<hp:g.enemies[0].hp==hp,
        "only the double dagger strike can hit on the second contact");
  if(kind==Kind::Sword||kind==Kind::Axe){
   const float expected=kind==Kind::Sword?.32f:.74f/1.4f;
   check(std::abs(profile.duration/rate-expected)<.001f&&g.swing==0&&g.attackCd<.06f,
         "the attack completes its strike and recovery on the weapon's release clock");
  }
 }
 auto g=setup(Kind::Spear);
 enemy(g,1,{2.5f,0});enemy(g,2,{4.0f,0});enemy(g,3,{5.2f,0});
 g.beginMeleeHold({1,0});advance(g,34);g.releaseMeleeHold({1,0});advance(g,18);
 check(g.enemies[0].hp<500&&g.enemies[1].hp<500&&g.enemies[2].hp==500,"charged spear reaches two lined-up enemies at extended range");
 g=setup(Kind::Sword);enemy(g,1,{1.5f,0});
 check(g.beginMeleeHold({1,0})&&g.releaseMeleeHold({1,0})&&g.activePowerMelee()==PowerStyle::Count&&g.enemies[0].hp<500,
       "a quick tap still uses the original sword attack");
 g=setup(Kind::Sword);
 enemy(g,1,{2.10f,0});enemy(g,2,{2.42f,0});
 g.beginMeleeHold({1,0});advance(g,34);g.releaseMeleeHold({1,0});advance(g,10);
 check(g.enemies[0].hp<500&&g.enemies[1].hp==500,"charged sword keeps its inner reach and misses at the old outer edge");
 g=setup(Kind::Axe);g.beginMeleeHold({1,0});advance(g,34);g.startRoll({0,1});
 check(!g.holdingMelee()&&!g.releaseMeleeHold({1,0}),"rolling clears pending charge");
 g=setup(Kind::Dagger);g.beginMeleeHold({1,0});advance(g,34);
 g.bag[10]=item(Kind::Sword);g.transfer({Place::Bag,10},{Place::Equip,0});
 check(!g.holdingMelee()&&!g.releaseMeleeHold({1,0}),"changing weapon clears pending charge");
 g=setup(Kind::Bow);g.bag[10]=item(Kind::Arrow,0,9);
 const int ammo=g.ammoCount();g.beginBow({1,0});advance(g,34);
 const float normal=g.arrowDamage(g.bowCharge());
 check(g.releaseBow(true)&&g.arrows.size()==1&&g.arrows[0].power&&g.arrows[0].damage>normal&&g.ammoCount()==ammo-1&&
       g.activePowerVisual()==PowerStyle::Bow&&g.sounds.back().cue==Cue::PowerBow,"charged bow fires one enhanced arrow and spends one");
}
