#pragma once
void audioSaveTests(){
 auto bytes=saves::readBytes("art/audio/physical-20261108/legacy-v12.efs");saves::Archive header(true,bytes);std::string magic;int version;size_t size;uint64_t hash;header(magic,version,size,hash);
 auto s=saves::decode(bytes);
 saves::Snapshot original;saves::Archive payload(true,bytes.substr(header.at),12);payload(original);
 verifyAndAlignWorldExpansion(original,s);original.game.volcanoSceneryVersion=s.game.volcanoSceneryVersion;
 check(version==12&&saves::encode(original)==saves::encode(s),"actual v12 payload preserves all data outside world expansion and the spell purchase gate");
 check(s.game.hp==71&&s.game.mana==23&&s.game.gold==312&&s.game.bag[8].kind==Kind::GuardianSword&&!s.game.enemies[0].alive,"audio revision preserves equipment, resources and corpses");
 check(saves::CurrentVersion>=13&&int(Cue::FireBreath)==32,"new attack IDs are appended and saved with an explicit format revision");
 for(auto cue:{Cue::BoneAttack,Cue::ChitinAttack,Cue::SerpentAttack,Cue::ReptileAttack,Cue::WoodAttack,Cue::ArmoredAttack,Cue::BladeAttack,Cue::HeavySwing})s.game.sound(cue,s.game.player);
 roundtrip(s);check(saves::decode(saves::encode(s)).game.sounds.size()==10,"new physical attack events persist without changing old cue numbers");
 for(auto cue:{Cue::FireballCast,Cue::FireballImpact,Cue::TeleportArrival,Cue::BowDraw})s.game.sound(cue,s.game.player);
 roundtrip(s);auto updated=saves::decode(saves::encode(s));
 check(saves::CurrentVersion>=14&&int(Cue::HeavySwing)==40&&updated.game.sounds.size()==14&&updated.game.sounds.back().cue==Cue::BowDraw,"new fire, portal and bow events retain all previous cue IDs");
}
