#pragma once
unsigned attackSeed(bool critical){
 for(unsigned seed=0;seed<1000;seed++){std::mt19937 engine(seed);if((std::uniform_int_distribution<int>(1,100)(engine)<=CriticalChancePercent)==critical)return seed;}
 std::abort();
}
Game combatArena(bool critical){Game g=emptyArena();g.skillRanks={1,1,1};g.combatRng.seed(attackSeed(critical));return g;}
void criticalHealingTests(){
 for(bool critical:{false,true}){
  float multiplier=critical?CriticalMultiplier:1.f;
  for(int type=0;type<EnemyTypeCount;type++)for(DamageType damageType:{DamageType::Physical,DamageType::Magic}){
   Game g=combatArena(critical);Enemy enemy{800,type,g.player,g.player,5000,5000};float normal=damageAfterArmor(28,damageType==DamageType::Physical?enemy.defense():enemy.magicDefense());
   auto rng=g.combatRng;g.hit(enemy,28,damageType,critical);
   check(std::abs((5000-enemy.hp)-normal*multiplier)<.001f,"critical multiplier is exactly 250 percent after either armor type on each species");
   check(g.effects.size()==1&&g.effects[0].damageNumber&&g.effects[0].critical==critical&&g.effects[0].height>enemyInfo(type).visualHeight,"damage feedback retains critical flag and sits above complete enemy silhouette");
   check(g.combatRng==rng,"applying damage never rerolls the already determined attack outcome");
  }
  Game g=combatArena(critical);for(int i=0;i<3;i++){V p=g.player+V{float(i-1),.5f};g.enemies.push_back({800+i,i,p,p,5000,5000});}
  auto expected=g.combatRng;std::uniform_int_distribution<int>(1,100)(expected);float amount=g.damage();check(g.attack()&&g.combatRng==expected,"one successful area sword swing consumes exactly one attack roll");
  for(const auto& enemy:g.enemies)check(std::abs((5000-enemy.hp)-damageAfterArmor(amount,enemy.defense())*multiplier)<.001f,"all targets of a sword swing share its critical outcome");
  auto after=g.combatRng;check(!g.attack()&&g.combatRng==after,"blocked repeat swing cannot roll or deal another critical");
  g=combatArena(critical);for(int i=0;i<3;i++){V p=g.player+V{float(i-1),.5f};g.enemies.push_back({800+i,i,p,p,5000,5000});}amount=g.strongCutDamage();expected=g.combatRng;std::uniform_int_distribution<int>(1,100)(expected);
  check(g.strongCut()&&g.combatRng==expected&&g.mana==90,"strong cut rolls once and spends unchanged ten mana");
  for(const auto& enemy:g.enemies)check(std::abs((5000-enemy.hp)-damageAfterArmor(amount,enemy.defense())*multiplier)<.001f,"strong cut applies critical to its own stronger base damage for every target");
  for(bool piercing:{false,true}){
   g=combatArena(critical);g.selectWeapon(1);for(int i=0;i<3;i++){V p=g.player+V{3.f+i*2,0};g.enemies.push_back({800+i,1,p,p,5000,5000});}
   if(piercing)g.preparePiercingArrow();after=g.combatRng;check(g.beginBow({1,0})&&g.combatRng==after,"bow preparation and drawing never consume an attack roll");
   g.bowDrawTime=BowDrawDuration;amount=g.arrowDamage(1);expected=g.combatRng;std::uniform_int_distribution<int>(1,100)(expected);
   check(g.releaseBow()&&g.combatRng==expected&&g.arrows[0].critical==critical&&g.arrows[0].damage==amount&&g.ammoCount()==19,"bow release rolls once and stores critical outcome with unchanged charge damage and ammo cost");
   g.updateArrows(.1f);check(g.combatRng==expected,"arrow flight never rerolls a critical");g.updateArrows(1);
   for(int i=0;i<3;i++)check(std::abs((5000-g.enemies[i].hp)-(i==0||piercing?damageAfterArmor(amount,g.enemies[i].defense())*multiplier:0))<.001f,"normal arrow hits first target and piercing arrow shares critical across all penetrated targets");
   check(g.mana==(piercing?80:100)&&g.combatRng==expected,"piercing critical retains twenty-mana cost without extra hit rolls");
  }
  g=combatArena(critical);for(int i=0;i<2;i++){V p=g.player+V{3.f+i*.7f,0};g.enemies.push_back({800+i,i,p,p,5000,5000});}amount=g.magicDamage();expected=g.combatRng;std::uniform_int_distribution<int>(1,100)(expected);
  check(g.spell({1,0})&&g.fireballs[0].critical==critical&&g.combatRng==expected&&g.mana==90,"fireball cast stores one critical roll and costs normal ten mana");g.updateFireballs(.05f);check(g.combatRng==expected,"fireball in flight never rerolls");g.updateFireballs(1);
  for(const auto& enemy:g.enemies)check(std::abs((5000-enemy.hp)-damageAfterArmor(amount,enemy.magicDefense())*multiplier)<.001f,"critical fireball explosion applies magic defense and one shared critical outcome to all targets");
  check(g.combatRng==expected,"fireball impact does not consume another roll");
 }
 Game sample;sample.combatRng.seed(76543);int criticalCount=0;auto lootRng=sample.rng;
 for(int i=0;i<100000;i++)criticalCount+=sample.rollCritical();check(criticalCount>4600&&criticalCount<5400,"one hundred thousand independent attack rolls match a five percent critical probability");check(sample.rng==lootRng,"critical draws do not perturb the loot random generator");
 Game g=emptyArena();g.hp=20;check(!g.heal()&&g.hp==20&&g.mana==100,"unlearned healing cannot activate");earnLevel(g);check(!g.trainSkills({0,0,1}),"healing cannot be learned before mandatory attributes");g.distribute({5,0,0,0,0,0});check(g.trainSkills({0,0,1})&&g.skillRanks==SkillRanks{0,0,1}&&!g.allocationRequired,"one earned skill point unlocks healing after attributes");
 for(Kind weapon:{Kind::Sword,Kind::Bow,Kind::Axe,Kind::None}){
  g=combatArena(false);g.equipment[0]=item(weapon);g.hp=20;g.mana=20;auto rng=g.combatRng;
  check(g.skillAllowed(Skill::Healing)&&g.heal()&&g.hp==50&&g.mana==0&&g.healingPulse==HealingDuration,"healing restores thirty HP for exactly twenty mana with every weapon or empty hands");check(g.combatRng==rng,"healing is not an attack and never rolls a critical");check(!g.heal()&&g.hp==50&&g.mana==0,"duplicate healing is blocked without extra resource cost");
 }
 g=combatArena(false);g.hp=95;check(g.heal()&&g.hp==100&&g.mana==80&&g.effects.back().text=="+5 HP","healing caps health at maximum and reports actual restored amount");g.healingCd=0;auto rng=g.combatRng;check(!g.heal()&&g.mana==80&&g.combatRng==rng,"full health refuses healing without wasting mana");
 g=combatArena(false);g.hp=30;g.mana=19;check(!g.heal()&&g.hp==30&&g.mana==19&&g.healingCd==0&&g.healingPulse==0,"insufficient mana leaves healing entirely unchanged");
 g=combatArena(false);g.attributes[int(Attribute::Vitality)]=5;g.hp=125;check(g.heal()&&g.hp==150&&g.mana==80,"healing respects vitality-raised maximum health");
 g=combatArena(false);g.enterMageTower();g.dawnQuest=DawnQuest::AtTower;g.hp=20;check(g.heal()&&g.hp==50&&g.mana==80,"healing also works inside safe tower");for(int i=0;i<25;i++)g.update(.05f,{},false);check(g.healingCd==0&&g.healingPulse==0&&g.heal()&&g.hp==80,"healing cooldown and animation finish normally in tower");
 g=combatArena(false);g.hp=20;g.selectWeapon(1);g.preparePiercingArrow();g.beginBow({1,0});check(g.heal()&&!g.bowDrawing&&!g.piercingArrowArmed&&g.ammoCount()==20,"healing cancels drawn bow without firing or spending an arrow");
 for(int guard=0;guard<4;guard++){g=combatArena(false);g.hp=20;if(guard==0)g.hp=0;if(guard==1)g.allocationRequired=true;if(guard==2)g.dawnQuest=DawnQuest::Returning;if(guard==3)g.startRoll({1,0});float hp=g.hp;check(!g.heal()&&g.hp==hp&&g.mana==100,"death, advancement, story and active roll block healing without cost");}
 g=combatArena(false);g.hp=20;g.heal();g.allocationRequired=true;float pulse=g.healingPulse,cd=g.healingCd;g.update(.05f,{1,0},true);check(g.healingPulse==pulse&&g.healingCd==cd,"allocation pause freezes healing animation and cooldown");
 g=combatArena(false);g.xp=experienceForLevel(g.level)-1;Enemy fake{900,0,g.player,g.player,1,1};g.hit(fake,10);g.distribute({5,0,0,0,0,0});check(!g.trainSkills({0,0,1})&&g.skillPoints==1,"already learned single-rank healing cannot consume an upgrade point");check(g.trainSkills({1,0,0})&&g.skillRanks==SkillRanks{2,1,1},"remaining skills still accept points after healing is learned");
}
