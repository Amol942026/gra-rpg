#pragma once
Game lessonTower(){Game g;g.bossDead=true;g.enterMageTower();g.dawnQuest=DawnQuest::AtTower;g.player=MagePosition;g.gold=10000;return g;}
void spellLearningTests(){
 Game fresh;check(fresh.selectedSpell==Spell::None&&fresh.learnedSpells==0,"fresh expedition starts without purchased magic");
 for(auto spell:{Spell::Fireball,Spell::IceShard,Spell::LightningOrb}){
  check(!fresh.knowsSpell(spell)&&!fresh.chooseSpell(spell),"every spell requires a lesson, including the old fireball");
  fresh.selectedSpell=spell;auto mana=fresh.mana;check(!fresh.spell({1,0})&&fresh.mana==mana&&fresh.fireballs.empty(),"manually selecting an unlearned spell cannot bypass purchase");
 }
 Game gate=lessonTower();gate.bossDead=false;auto balance=gate.goldBalance();
 check(gate.learnSpell(Spell::Fireball)==LearnSpellResult::Unavailable&&gate.goldBalance()==balance&&gate.learnedSpells==0,"Eldrin lessons stay locked until the first guardian dies");
 gate.bossDead=true;gate.towerFloor=1;check(gate.learnSpell(Spell::Fireball)==LearnSpellResult::Unavailable,"a different tower floor cannot buy a lesson");
 gate.towerFloor=0;gate.player={2,2};check(gate.learnSpell(Spell::Fireball)==LearnSpellResult::Unavailable,"learning requires speaking within Eldrin reach");
 gate.player=MagePosition;gate.gold=599;auto bag=gate.bag;check(gate.learnSpell(Spell::Fireball)==LearnSpellResult::NotEnoughGold&&gate.gold==599&&gate.bag[0].count==bag[0].count&&!gate.knowsSpell(Spell::Fireball),"insufficient payment preserves resources and ownership");
 gate.gold=580;gate.bag[8]=item(Kind::Gold,0,25);check(gate.learnSpell(Spell::Fireball)==LearnSpellResult::Success&&gate.goldBalance()==5&&gate.gold==0&&gate.bag[8].count==5,"a lesson atomically combines wallet and bag coins");
 auto rng=gate.rng,combatRng=gate.combatRng;balance=gate.goldBalance();
 check(gate.learnSpell(Spell::Fireball)==LearnSpellResult::AlreadyKnown&&gate.goldBalance()==balance&&gate.selectedSpell==Spell::Fireball&&gate.rng==rng&&gate.combatRng==combatRng,"duplicate lesson never charges twice or consumes combat randomness");
 check(gate.learnSpell(Spell::None)==LearnSpellResult::Invalid&&gate.goldBalance()==balance,"invalid lessons cannot charge money");
 Game all=lessonTower();for(auto spell:{Spell::Fireball,Spell::IceShard,Spell::LightningOrb})check(all.learnSpell(spell)==LearnSpellResult::Success,"all three elemental lessons can be bought independently");
 check(all.gold==4900&&all.learnedSpells==7,"expensive lessons charge exactly 600, 1500 and 3000 gold");
 check(all.spellDamage(Spell::Fireball)<all.spellDamage(Spell::IceShard)&&all.spellDamage(Spell::IceShard)<all.spellDamage(Spell::LightningOrb),"ice is stronger than fire and lightning is strongest");
 for(int i=1;i<3;i++)check(Spells[i].mana>Spells[i-1].mana&&Spells[i].cooldown>Spells[i-1].cooldown&&Spells[i].price>Spells[i-1].price,"spell power has strictly increasing mana, cooldown and price");
 for(auto spell:{Spell::Fireball,Spell::IceShard,Spell::LightningOrb})for(int fps:{30,60,120}){
  Game g=emptyArena();g.learnedSpells=7;g.chooseSpell(spell);const auto& definition=spellDefinition(spell);
  float mana=g.mana;check(g.spell({1,0})&&g.mana==mana-definition.mana&&g.fireballs.back().spell==spell&&g.spellCd==definition.cooldown,"cast snapshots its own element, exact mana and cooldown");
  auto shot=g.fireballs.front();g.chooseSpell(spell==Spell::Fireball?Spell::LightningOrb:Spell::Fireball);check(!g.spell({1,0})&&g.fireballs.size()==1,"changing selected magic cannot bypass the running cooldown");
  for(int n=0;n<fps/4;n++)g.updateFireballs(1.f/fps);
  check(g.fireballs.size()==1&&std::abs(g.fireballs.front().pos.x-(15+definition.speed*float(fps/4)/fps))<.003f&&g.fireballs.front().spell==spell,"each projectile moves at its own speed at 30, 60 and 120 Hz");
  g.enemies.push_back({710,0,{19,15},{19,15},1000,1000});
  for(int n=0;n<fps;n++)g.updateFireballs(1.f/fps);
  float expected=1000-shot.damage*(shot.critical?CriticalMultiplier:1.f);
  check(std::abs(g.enemies[0].hp-expected)<.01f&&g.fireballs.empty()&&!g.explosions.empty()&&g.explosions.back().spell==spell,"typed impact damages once and retains its own visual element after selection changes");
  g=emptyArena();g.learnedSpells=7;g.chooseSpell(spell);g.mana=definition.mana-1;
  check(!g.spell({1,0})&&g.fireballs.empty()&&g.mana==definition.mana-1&&g.spellCd==0,"each spell rejects insufficient mana without starting cooldown");
 }
 Game full=lessonTower();for(auto& item:full.bag)item=ember::item(Kind::Crystal,0,99);
 check(full.learnSpell(Spell::IceShard)==LearnSpellResult::Success&&full.knowsSpell(Spell::IceShard)&&full.bag[0].count==99,"knowledge can be learned with a full bag without discarding loot");
}
