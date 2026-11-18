#pragma once
void bowTests(){
 Game g;int before=total(g);check(g.ammoCount()==20&&g.equipment[0].kind==Kind::Sword&&g.weaponSource(1).place==Place::Bag,"new expedition owns sword, bow and exactly twenty arrows");
 check(!g.bindWeapon({Place::Bag,0},2)&&!g.bindWeapon({Place::Ground,1},2)&&!g.bindWeapon({Place::Bag,4},99),"wheel rejects food, world references and invalid sectors");
 g.bag[10]=item(Kind::Axe,2);before=total(g);check(g.bindWeapon({Place::Bag,10},2)&&total(g)==before&&g.bag[10].kind==Kind::Axe,"assigning inventory weapon creates only a shortcut");
 check(g.selectWeapon(2)&&g.equipment[0].kind==Kind::Axe&&g.equipment[0].tier==2&&g.bag[10].kind==Kind::Sword&&total(g)==before,"wheel selection swaps real equipment without duplicating either weapon");
 check(g.transfer({Place::Bag,10},{Place::Bag,11})&&g.selectWeapon(0)&&g.equipment[0].kind==Kind::Sword,"shortcut follows the weapon after backpack reordering");
 check(g.bindWeapon({Place::Bag,4},4)&&g.weaponWheel[1].kind==Kind::None&&g.weaponSource(4).place==Place::Bag,"reassigning same weapon moves its shortcut without item duplication");
 check(g.clearWeaponBinding(4)&&g.bag[4].kind==Kind::Bow,"removing shortcut keeps its weapon in backpack");
 g.drop({Place::Equip,0},g.player);check(!g.selectWeapon(0)&&g.equipment[0].empty(),"lost weapon cannot be materialized from a shortcut");
 g.transfer({Place::Ground,g.ground.back().id},{Place::Bag,12});check(g.selectWeapon(0)&&g.equipment[0].kind==Kind::Sword,"shortcut works again after recovering its actual weapon");
 Game full;for(auto& value:full.bag)value=item(Kind::Armor);full.bag[10]=item(Kind::Axe);full.bindWeapon({Place::Bag,10},2);
 check(full.selectWeapon(2)&&full.equipment[0].kind==Kind::Axe&&full.bag[10].kind==Kind::Sword,"switching works with full backpack using an exact swap");
 auto arena=[](){Game a;a.props.clear();a.enemies.clear();a.forage.clear();a.player={15,15};a.selectWeapon(1);return a;};
 auto finishTap=[](Game& game){for(int i=0;i<60&&game.bowReleaseQueued;i++)game.update(1.f/60,{},false);};
 g=arena();int swordDamage=Game{}.damage();
 check(g.arrowDamage(0)>swordDamage&&g.arrowDamage(1)>g.arrowDamage(0),"even shortest arrow shot is stronger than initial sword and full draw increases damage");
 check(!g.attack()&&g.swing==0,"equipped bow never performs sword melee attack");
 check(g.beginBow({1,0})&&g.ammoCount()==20&&g.arrows.empty(),"LPM press starts drawing without firing or consuming ammo");
 for(int i=0;i<240;i++)g.update(1.f/60,{},false);
 check(g.bowCharge()==1&&g.ammoCount()==20&&g.arrows.empty(),"holding indefinitely caps charge without automatic firing");
 check(g.releaseBow()&&g.ammoCount()==19&&g.arrows.size()==1&&std::abs(g.arrows[0].remaining-BowMaxRange)<.001f,"release fires one maximum-range arrow and consumes exactly one");
 check(!g.releaseBow()&&!g.beginBow({1,0})&&g.ammoCount()==19,"duplicate release and cooldown cannot fire extra arrows");
 for(float duration:{0.f,.14f,.61f,.7f,1.4f,2.8f}){
  Game a=arena();a.beginBow({1,0});int steps=int(std::ceil(duration*60));for(int i=0;i<steps;i++)a.update(duration/steps,{},false);
  const float charge=std::min(1.f,std::max(BowMinDrawTime,duration-BowNockDuration)/BowDrawDuration);
  a.releaseBow();finishTap(a);check(a.arrows.size()==1&&std::abs(a.arrows[0].remaining-Game::bowRange(charge))<.002f,"arrow range scales only with actual string tension");
 }
 g=arena();g.beginBow({0,1});g.aimBow({1,0});const auto startingRng=g.combatRng;
 check(g.bowNockProgress()==0&&g.releaseBow(true)&&!g.releaseBow(true)&&!g.beginBow({0,-1}),"one early release is remembered for the current arrow");
 for(int i=1;i<=36;i++){
  g.aimBow({0,-1});g.update(1.f/60,{0,1},false);
  check(std::abs(g.bowNockProgress()-i/36.f)<.0001f&&g.bowCharge()<.00001f&&g.arrows.empty()&&g.ammoCount()==20&&g.combatRng==startingRng,
        "nocking advances with no tension, projectile, ammo expense or critical roll");
 }
 g.update(.05f,{},false);
 check(g.bowReleaseQueued&&g.bowCharge()>0&&g.arrows.empty()&&g.bowNockProgress()==1,"a tap includes a visible pull after nocking");
 finishTap(g);
 check(g.arrows.size()==1&&g.arrows[0].velocity.x>0&&g.arrows[0].velocity.y==0&&distance(g.player,g.arrows[0].pos)<.00001f&&
       !g.arrows[0].power&&g.bowNockProgress()==1&&g.ammoCount()==19,"tap fires once from the current position along the released aim without gaining power");
 g=arena();g.beginBow({1,0});g.releaseBow(true);g.startRoll({1,0});finishTap(g);
 check(g.arrows.empty()&&g.ammoCount()==20&&g.bowNockProgress()<0,"rolling cancels a queued tap without spending its arrow");
 for(int frames:{3,38}){
  g=arena();V body=g.player+V{4,0};g.enemies.push_back({81,1,body,body,0,100});g.enemies.back().alive=false;
  g.beginBow({1,0});g.releaseBow(true);for(int i=0;i<frames;i++)g.update(1.f/60,{},false);
  check(!g.openCorpse(81)&&g.bowReleaseQueued,"an unreachable corpse leaves the current bow action alone");
  g.enemies.back().pos=g.player+V{1,0};check(g.openCorpse(81),"opening a nearby corpse succeeds during a queued tap");
  for(int i=0;i<60;i++)g.update(1.f/60,{},false);
  check(g.openedCorpse==81&&!g.bowDrawing&&!g.bowReleaseQueued&&g.arrows.empty()&&g.ammoCount()==20,
        "opening a corpse from the nearby panel cancels a queued tap both before and after the arrow reaches the string");
 }
 for(bool loseMana:{false,true}){
  g=arena();g.skillRanks[int(Skill::PiercingArrow)]=1;g.mana=PiercingArrowCost;
  g.preparePiercingArrow();g.beginBow({1,0});g.releaseBow(true);
  check(g.bowReleaseQueued&&g.piercingArrowCd==0&&g.mana==PiercingArrowCost&&g.ammoCount()==20,
        "queuing a piercing arrow spends neither mana nor ammunition");
  if(loseMana)g.mana-=1;
  finishTap(g);
  if(loseMana)check(g.arrows.empty()&&g.ammoCount()==20&&g.piercingArrowCd==0,"losing required mana before release cancels the queued piercing shot");
  else check(g.arrows.size()==1&&g.arrows[0].piercing&&!g.arrows[0].power&&g.ammoCount()==19&&g.mana==0&&g.piercingArrowCd>0,
             "quick piercing shot consumes mana and ammunition once when released");
 }
 g=arena();g.beginBow({1,0});g.update(.05f,{},false);g.cancelBow();check(!g.releaseBow()&&g.ammoCount()==20,"cancelled drawing does not consume ammunition");
 g.beginBow({1,0});g.startRoll({1,0});check(!g.bowDrawing&&!g.releaseBow()&&g.ammoCount()==20,"roll cancels drawn shot without consuming arrow");
 g=arena();g.beginBow({1,0});g.selectWeapon(0);check(!g.bowDrawing&&!g.releaseBow()&&g.ammoCount()==20,"weapon switching cancels drawn arrow");
 g=arena();g.bag[5].count=1;g.beginBow({1,0});g.releaseBow();finishTap(g);g.attackCd=0;check(g.ammoCount()==0&&g.bag[5].empty()&&!g.beginBow({1,0}),"last arrow clears stack and an empty quiver cannot shoot");
 g=arena();g.bag[5].count=1;g.bag[10]=item(Kind::Arrow,0,2);g.beginBow({1,0});g.releaseBow();finishTap(g);check(g.ammoCount()==2&&g.bag[5].empty()&&g.bag[10].count==2,"ammunition can be split across inventory stacks");
 for(int steps:{15,30,60}){g=arena();g.beginBow({1,0});g.bowDrawTime=BowDrawDuration;g.releaseBow();for(int i=0;i<steps;i++)g.updateArrows(.25f/steps);check(g.arrows.size()==1&&std::abs(g.arrows[0].pos.x-20)<.001f,"arrow travel is independent of update frequency");}
 g=arena();g.enemies.push_back({81,1,{18,15},{18,15},500,500});g.enemies.push_back({82,1,{20,15},{20,15},500,500});
 g.beginBow({1,0});g.bowDrawTime=BowDrawDuration;float expected=damageAfterArmor(g.arrowDamage(1),g.enemies[0].defense());g.releaseBow();g.updateArrows(.7f);
 check(g.arrows.empty()&&std::abs(g.enemies[0].hp-(500-expected))<.001f&&g.enemies[1].hp==500,"swept collision hits only nearest enemy once with physical armor applied");
 g.updateArrows(1);check(g.enemies[1].hp==500,"removed projectile cannot damage another enemy later");
 g=arena();g.props.push_back({{16.4f,15},0,1});g.enemies.push_back({81,1,{18,15},{18,15},500,500});g.beginBow({1,0});g.bowDrawTime=BowDrawDuration;g.releaseBow();g.updateArrows(.7f);
 check(g.arrows.empty()&&g.enemies[0].hp==500,"tree stops fast arrow before enemy behind it");
 g=arena();g.beginBow({1,0});g.releaseBow();finishTap(g);g.enemies.push_back({81,1,{19,15},{19,15},500,500});g.updateArrows(1);
 check(g.arrows.empty()&&g.enemies[0].hp==500,"short draw cannot hit beyond its maximum travel distance");
 g=arena();g.player={45,15};g.beginBow({1,0});g.bowDrawTime=BowDrawDuration;g.releaseBow();g.updateArrows(1);check(g.arrows.empty(),"map edge stops arrow without leaving immortal projectiles");
 g=arena();g.xp=experienceForLevel(1)-10;g.enemies.push_back({81,0,{17,15},{17,15},1,100});g.beginBow({1,0});g.bowDrawTime=BowDrawDuration;g.releaseBow();g.updateArrows(.4f);
 check(g.kills==1&&g.allocationRequired&&g.level==2&&g.ground.empty()&&g.findCorpse(81),"arrow kill uses existing corpse loot and mandatory advancement pause");
 check(!g.beginBow({1,0})&&!g.selectWeapon(0),"mandatory level allocation blocks bow and wheel equipment actions");
}
