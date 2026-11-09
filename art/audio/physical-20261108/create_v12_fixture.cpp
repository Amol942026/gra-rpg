// One-time golden fixture compiled with -Iart/pipeline/backups/audio-refinement-20261108/src -Isrc.
#include "save_game.hpp"
#include <iostream>
int main(int argc,char** argv){
 if(argc!=2||std::string(argv[1])!="--no-audio")return 3;
 using namespace ember;static_assert(saves::CurrentVersion==12&&int(Cue::Count)==33&&MapSize==102);
 saves::Snapshot s;s.game.activateField(WorldLocation::Desert);s.game.activateField(WorldLocation::Volcano);s.game.player=VolcanoArrival;
 auto& e=s.game.enemies[0];e.hp=0;e.alive=false;e.loot={};e.loot[0]=item(Kind::Gold,0,17);
 s.game.hp=71;s.game.mana=23;s.game.gold=312;s.game.level=4;s.game.xp=18;s.game.bag[8]=item(Kind::GuardianSword,2);s.game.rng();s.game.combatRng();
 s.game.sounds={{Cue::Swing,s.game.player.x,s.game.player.y},{Cue::FireBreath,s.game.player.x,s.game.player.y}};
 s.date="2026-11-08 21:00";auto bytes=saves::encode(s);std::ofstream out("art/audio/physical-20261108/legacy-v12.efs",std::ios::binary);out.write(bytes.data(),bytes.size());std::cout<<"previous-release v12: "<<bytes.size()<<" bytes\n";
}
