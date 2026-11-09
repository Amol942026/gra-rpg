#pragma once
void spellLearningSaveTests(){
 auto bytes=saves::readBytes("art/gameplay/eldrin-spells-20261109/legacy-v14.efs");auto migrated=saves::decode(bytes);
 check(migrated.game.selectedSpell==Spell::None&&migrated.game.learnedSpells==0,"authentic v14 save applies the requested purchase gate without free legacy spells");
 check(migrated.game.hp==87&&migrated.game.mana==71&&migrated.game.gold==1420&&migrated.game.bag[9].count==100&&migrated.game.sounds.back().cue==Cue::BowDraw,"v14 migration preserves resources, bag money and old cue IDs");
 check(migrated.game.fireballs.size()==1&&migrated.game.fireballs[0].spell==Spell::Fireball&&migrated.game.fireballs[0].age==0&&migrated.game.explosions[0].spell==Spell::Fireball,"legacy in-flight fire and impact receive their original element safely");
 saves::Archive header(true,bytes);std::string magic;int version;size_t size;uint64_t hash;header(magic,version,size,hash);auto projected=migrated;projected.game.selectedSpell=Spell::Fireball;saves::Archive old(false,{},14);old(projected);
 check(version==14&&old.bytes==bytes.substr(header.at),"every v14 payload byte outside the explicitly changed selection policy remains intact");roundtrip(migrated);
 for(unsigned mask=0;mask<8;mask++){
  saves::Snapshot s;s.game.learnedSpells=mask;
  for(auto spell:{Spell::Fireball,Spell::IceShard,Spell::LightningOrb})if(mask&spellBit(spell)){
   s.game.selectedSpell=spell;s.game.spellCooldownTotal=spellDefinition(spell).cooldown;s.game.spellCd=s.game.spellCooldownTotal*.63f;
   s.game.fireballs.push_back({s.game.player,{1,2},4,float(s.game.spellDamage(spell)),true,spell,.27f});s.game.explosions.push_back({s.game.player,spellDefinition(spell).impactLife*.5f,spell});
  }
  s.game.gold=487;s.game.bag[9]=item(Kind::Gold,0,37);roundtrip(s);auto restored=saves::decode(saves::encode(s));
  check(restored.game.learnedSpells==mask&&restored.game.goldBalance()==524,"all eight independent learned-spell masks and payment balance survive reload");
  for(size_t i=0;i<s.game.fireballs.size();i++)check(restored.game.fireballs[i].spell==s.game.fireballs[i].spell&&restored.game.fireballs[i].age==.27f&&restored.game.explosions[i].spell==s.game.explosions[i].spell,"projectile element and Synfig phase persist independently from selected magic");
 }
 saves::Snapshot bad;bad.game.learnedSpells=8;rejects([&]{saves::encode(bad);},"unknown ownership bits are rejected");
 bad={};bad.game.selectedSpell=Spell::IceShard;rejects([&]{saves::encode(bad);},"unowned selection cannot be smuggled through a save");
 bad={};bad.game.fireballs.push_back({bad.game.player,{8,0},4,28,false,Spell(42)});rejects([&]{saves::encode(bad);},"invalid projectile elements are rejected");
}
