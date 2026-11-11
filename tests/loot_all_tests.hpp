#pragma once
inline void lootAllTests(){
 auto arena=[](){Game g;g.props.clear();g.enemies.clear();g.forage.clear();g.chests={};g.bag={};g.player={20,20};return g;};
 auto body=[](Game& g){Enemy e;e.id=9801;e.type=1;e.alive=false;e.pos=g.player;e.loot={};g.enemies.push_back(e);g.openedCorpse=e.id;};
 auto same=[](const auto& a,const auto& b){for(size_t i=0;i<a.size();i++)if(a[i].kind!=b[i].kind||a[i].tier!=b[i].tier||a[i].count!=b[i].count)return false;return true;};
 Game g=arena();body(g);auto& loot=g.enemies[0].loot;
 loot[0]=item(Kind::Gold,0,60);loot[5]=item(Kind::Health,0,4);loot[14]=item(Kind::Sword,2);
 g.bag[0]=item(Kind::Gold,0,80);g.sounds.clear();
 check(g.takeAllLoot(),"take all collects the full corpse across all loot pages in one operation");
 check(g.bag[0].count==99&&g.bag[1].kind==Kind::Gold&&g.bag[1].count==41&&g.bag[2].count==4&&g.bag[3].tier==2,"take all fills compatible stacks then free slots while preserving item tiers");
 check(std::all_of(loot.begin(),loot.end(),[](Item i){return i.empty();})&&g.ground.empty(),"successful collection empties the source without dropping anything on the ground");
 check(std::count_if(g.sounds.begin(),g.sounds.end(),[](auto e){return e.cue==Cue::Pickup;})==1,"take all emits one collection cue for the whole operation");
 check(!g.takeAllLoot(),"taking all from an empty body has no effect");
 g=arena();body(g);for(auto& slot:g.bag)slot=item(Kind::Sword);
 g.bag[0]=item(Kind::Gold,0,98);g.enemies[0].loot[0]=item(Kind::Gold,0,1);g.enemies[0].loot[1]=item(Kind::Mana,0,2);
 auto beforeBag=g.bag;auto beforeLoot=g.enemies[0].loot;
 check(!g.takeAllLoot()&&same(g.bag,beforeBag)&&same(g.enemies[0].loot,beforeLoot),"insufficient capacity rolls back the entire collection, including a stack that could partially fit");
 check(g.message.find("Brak miejsca")!=std::string::npos&&g.sounds.back().cue==Cue::Reject,"insufficient space reports a visible error and rejection cue");
 g.enemies[0].loot[1]={};check(g.takeAllLoot()&&g.bag[0].count==99,"a backpack without empty slots can still collect loot that fits an existing stack");
 g=arena();TreasureChest chest;chest.id=9802;chest.location=g.location;chest.pos=g.player;chest.opened=true;chest.loot={};chest.loot[0]=item(Kind::Arrow,0,120);chest.loot[1]=item(Kind::Helmet,2);g.chests[0]=chest;g.openedChest=chest.id;
 check(g.takeAllLoot()&&g.bag[0].count==99&&g.bag[1].count==21&&g.bag[2].kind==Kind::Helmet,"chests use the same atomic operation and split stacks larger than 99");
 g.chests[0].loot[0]=item(Kind::Mana);g.player={25,25};check(!g.takeAllLoot()&&!g.chests[0].loot[0].empty(),"a stale open chest panel cannot collect loot out of range");
 g.player=g.chests[0].pos;g.chests[0].opened=false;check(!g.takeAllLoot(),"an unopened chest cannot be collected through a stale ID");
 for(auto region:{WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano}){
  g=arena();g.location=region;g.fieldLocation=region;
  Enemy boss;boss.id=9803;boss.alive=false;boss.type=region==WorldLocation::Forest?3:region==WorldLocation::Desert?DesertBossType:VolcanoBossType;boss.pos=g.player;boss.loot={};boss.loot[0]=item(Kind::Sword,1);boss.loot[QuestLootSlot]=item(Kind::DawnShard);boss.loot[14]=item(Kind::Gold,0,20);g.enemies.push_back(boss);g.openedCorpse=boss.id;
  int previous=region==WorldLocation::Forest?0:region==WorldLocation::Desert?1:2;
  g.collectedFragments=previous;if(previous)g.bag[0]=item(Kind::DawnShard,0,previous);
  check(g.takeAllLoot()&&g.towerReturnPending&&g.collectedFragments==previous+1,"take all collects each boss fragment and preserves its return-to-Eldrin quest trigger");
  check(std::all_of(g.enemies[0].loot.begin(),g.enemies[0].loot.end(),[](Item i){return i.empty();}),"boss items after the quest fragment are collected before the story becomes blocked");
 }
 g=arena();body(g);g.enemies[0].type=3;g.enemies[0].loot[QuestLootSlot]=item(Kind::DawnShard);for(auto& slot:g.bag)slot=item(Kind::Sword);
 check(!g.takeAllLoot()&&!g.towerReturnPending&&g.collectedFragments==0&&!g.enemies[0].loot[QuestLootSlot].empty(),"a quest fragment cannot trigger a return when the complete loot does not fit");
 for(int gate=0;gate<3;gate++){
  g=arena();body(g);g.enemies[0].loot[0]=item(Kind::Mana);
  if(gate==0)g.hp=0;else if(gate==1)g.allocationRequired=true;else g.dawnQuest=DawnQuest::Returning;
  check(!g.takeAllLoot()&&!g.enemies[0].loot[0].empty(),"dead, paused progression and story-blocked states cannot collect loot");
 }
}
