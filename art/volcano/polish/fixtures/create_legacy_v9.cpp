// Historical fixture, compiled against the pre-polish v9 source.
#include "../../../../src/save_game.hpp"
int main(int argc,char** argv){
 if(argc!=2||std::string(argv[1])!="--no-audio")return 3;
 static_assert(ember::saves::CurrentVersion==9,"Generate this fixture before the scenery revision change");
 ember::saves::Snapshot s;auto& g=s.game;g.activateField(ember::WorldLocation::Volcano);
 g.collectedFragments=2;g.desertBossDead=true;g.dawnQuest=ember::DawnQuest::AtTower;g.player={8,62};g.hp=63;g.mana=21;g.gold=137;
 g.chests[4].opened=true;g.chests[4].loot[0]={};g.volcanoFountainUsed[0]=true;
 auto& e=g.enemies[0];e.hp=0;e.alive=false;e.loot[0]=ember::item(ember::Kind::Gold,0,12);
 g.bag[12]=ember::item(ember::Kind::GuardianAxe,2);g.rng();g.combatRng();s.date="2026-10-29";
 ember::saves::atomicWrite("art/volcano/polish/fixtures/legacy-v9.efs",ember::saves::encode(s));return 0;
}
