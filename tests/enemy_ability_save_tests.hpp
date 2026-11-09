#pragma once
#include "enemy_ability_fixture.hpp"
void enemyAbilitySaveTests(){
 check(saves::CurrentVersion>=11,"special-attack clocks and projectiles retain their version 11 fields with explicit legacy support");
 for(int type:{9,10,16,17}){
  const auto spec=enemyAbilityProfile(type);
  for(float elapsed:{.2f,spec.windup+.05f,spec.windup+spec.active+.12f}){
   saves::Snapshot original;original.game=enemyAbilityArena(type);for(int frame=0;frame<int(std::round(elapsed*100));frame++)original.game.update(.01f,{},false);
   check(original.game.enemies[0].abilityBusy(),"save fixture contains a real windup, active hit or recovery");roundtrip(original);auto restored=saves::decode(saves::encode(original));
   for(int frame=0;frame<240;frame++){original.game.update(1.f/120,{},false);restored.game.update(1.f/120,{},false);}
   check(saves::encode(original)==saves::encode(restored),"saving and resuming mid-special preserves exact phase, damage, projectiles and RNG");
  }
 }
 saves::Snapshot flying;flying.game=enemyAbilityArena(10);while(flying.game.venomGlobs.empty())flying.game.update(.01f,{},false);
 check(flying.game.venomGlobs.size()==3,"save fixture contains the actual three-glob venom fan");roundtrip(flying);
 auto bad=flying;bad.game.venomGlobs[0].velocity={90,0};rejects([&]{saves::encode(bad);},"overspeed venom projectile rejected");
 bad=flying;bad.game.venomGlobs[0].owner=987654;rejects([&]{saves::encode(bad);},"ownerless venom projectile rejected");
 bad=flying;bad.game.enemies[0].abilityAge=99;rejects([&]{saves::encode(bad);},"oversized ability clock rejected");
 bad=flying;bad.game.enemies[0].abilityMask=2;rejects([&]{saves::encode(bad);},"impossible single-hit event mask rejected");
 bad=flying;bad.game.enemies[0].alive=false;rejects([&]{saves::encode(bad);},"dead caster cannot retain an active saved ability");
 saves::Snapshot legacy;legacy.game=enemyAbilityArena(10);legacy.game.enemies[0].abilityCd=2;
 saves::Archive payload(false,{},10);payload(legacy);saves::Archive envelope(false);std::string magic="EMBERFALL-SAVE";int version=10;size_t size=payload.bytes.size();uint64_t hash=saves::checksum(payload.bytes);envelope(magic,version,size,hash);auto upgraded=saves::decode(envelope.bytes+payload.bytes);
 check(upgraded.game.enemies[0].abilityAge==-1&&upgraded.game.enemies[0].abilityCd==3&&upgraded.game.venomGlobs.empty(),"v10 format defaults new specials to a safe idle state without altering enemies or loot");roundtrip(upgraded);
 auto real=saves::read("art/volcano/layout/fixtures/revision-3-v10.efs");check(std::all_of(real.game.enemies.begin(),real.game.enemies.end(),[](const Enemy& e){return e.abilityAge<0&&e.abilityCasts==0;}),"real previous-release v10 save loads all four new special attacks without an invented active cast");
 saves::Snapshot dialogue;dialogue.game=Game{};dialogue.game.enterMageTower();dialogue.game.collectedFragments=3;dialogue.game.dawnQuest=DawnQuest::AtTower;dialogue.tower.beginTalk(true,3,true,true,false);dialogue.tower.line=1;roundtrip(dialogue);
 check(saves::decode(saves::encode(dialogue)).tower.missingDesertShard,"v11 also retains contextual dialogue flags while combat is paused");
}
