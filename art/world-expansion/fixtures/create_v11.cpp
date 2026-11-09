// One-time previous-release fixture, compiled before expanding the 72x72 maps.
#include "../../../src/save_game.hpp"
#include <iostream>
int main(int argc,char** argv){
 if(argc!=2||std::string(argv[1])!="--no-audio")return 3;
 using namespace ember;static_assert(saves::CurrentVersion==11&&MapSize==72);
 for(auto target:{WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano}){
  saves::Snapshot s;
  for(auto realm:{WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano}){
   s.game.activateField(realm);auto& e=s.game.enemies[0];e.hp=0;e.alive=false;e.loot={};e.loot[0]=item(Kind::Gold,0,19+int(realm));
  }
  s.game.activateField(target);s.game.player=target==WorldLocation::Forest?ForestArrival:target==WorldLocation::Desert?DesertArrival:VolcanoArrival;
  s.game.level=5;s.game.xp=17;s.game.hp=67;s.game.mana=43;s.game.gold=127;s.game.bag[12]=item(Kind::GuardianSword,2);
  s.game.chests[4].opened=true;s.game.chests[4].loot[0]={};s.game.fountainUsed[0]=s.game.volcanoFountainUsed[1]=true;
  s.date="2026-11-08 16:12";std::string name=target==WorldLocation::Forest?"forest":target==WorldLocation::Desert?"desert":"volcano";
  std::ofstream out("art/world-expansion/fixtures/"+name+"-v11.efs",std::ios::binary);auto bytes=saves::encode(s);out.write(bytes.data(),bytes.size());
  std::cout<<name<<" v11 / "<<bytes.size()<<" bytes\n";
 }
}
