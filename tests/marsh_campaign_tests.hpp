#pragma once
void marshCampaignTests(){
 saves::Snapshot session;
 auto& g=session.game;
 g.developerSession=true;g.dawnQuest=DawnQuest::AtTower;g.collectedFragments=2;g.desertBossDead=true;
 g.enterMageTower();g.player=MagePosition;
 check(!g.canTravelMarsh()&&g.travelToVolcano(),"the fourth realm is locked before Varkhor's shard");
 auto* v=g.findEnemy(VolcanoBossId);check(v!=nullptr,"Varkhor exists");
 g.player=v->pos;g.hit(*v,10000);g.enterMageTower();g.player=MagePosition;
 check(!g.canTravelMarsh()&&g.travelToVolcano(),"defeating Varkhor alone does not open the marsh gate");
 g.player=g.findCorpse(VolcanoBossId)->pos;
 check(g.openCorpse(VolcanoBossId)&&g.quickTransfer({Place::Corpse,QuestLootSlot,VolcanoBossId})&&g.takeTowerReturn(),"collecting the third shard calls Eldrin");
 g.enterMageTower();g.finishTowerReturn();g.player=MagePosition;
 check(g.canTravelMarsh(),"Eldrin offers the marsh after the pickup");
 session.tower.beginTalk(true,3,true,false,false,g.canTravelMarsh());
 while(session.tower.phase==TowerPhase::Dialogue)session.tower.nextLine();
 check(session.tower.chooseMarsh(),"the marsh choice starts a teleport");
 for(int i=0;i<12;i++)session.tower.update(.1f);
 check(session.tower.takeDesertArrival()&&g.travelToMarsh(),"the destination is reached at the fade");
 session.tower={};
 check(distance(g.player,marsh::Arrival)<.01f&&g.walkable(g.player),"dry and traversable arrival");
 std::vector<char> seen(MapSize*MapSize);std::vector<int> queue;
 auto push=[&](int x,int y){if(x<0||y<0||x>=MapSize||y>=MapSize)return;
  int k=x+y*MapSize;if(!seen[k]&&g.walkable({float(x),float(y)})){seen[k]=1;queue.push_back(k);}};
 push(int(g.player.x),int(g.player.y));
 for(size_t i=0;i<queue.size();i++){int x=queue[i]%MapSize,y=queue[i]/MapSize;push(x+1,y);push(x-1,y);push(x,y+1);push(x,y-1);}
 for(const auto& site:FieldSites)if(site.realm==WorldLocation::Swamp)
  check(seen[int(site.position.x)+MapSize*int(site.position.y)],"all eight marsh sites are reachable on foot");
 for(auto& chest:g.chests)if(chest.location==WorldLocation::Swamp){
  g.player=chest.pos+V{1.6f,0};check(g.walkable(g.player)&&g.openChest(chest.id),"marsh chest has an accessible approach");}
 check(!g.walkable(marsh::Pools[0].center),"deep water blocks foot travel");
 auto oldHutSave=session;
 const auto hut=std::find_if(g.props.begin(),g.props.end(),[](const Prop& p){return p.type==MarshFishingHut;});
 check(hut!=g.props.end(),"the fishing settlement has a hut");
 check(std::none_of(g.props.begin(),g.props.end(),[](const Prop& p){
  return p.type==MarshFishingHut&&distance(p.pos,marsh::Ferry+V{1,-5})<.01f;
 }),"larger cottages at the ferry leave a gap between their roofs");
 oldHutSave.game.player=hut->pos+V{1.4f,0};
 const auto restored=saves::decode(saves::encode(oldHutSave));
 check(restored.game.walkable(restored.game.player)&&distance(restored.game.player,oldHutSave.game.player)<4,
       "a hero saved at the old hut wall loads on nearby accessible ground");
 for(int type=MarshPropStart;type<PropTypeCount;type++)check(std::any_of(g.props.begin(),g.props.end(),[&](const Prop& p){return p.type==type;}),"every authored marsh scenery type is present");
 int counts[4]{};int boss=-1;
 for(const auto& e:g.enemies){check(marshEnemy(e.type)&&g.enemyPositionClear(e.pos,e.radius()),"marsh encounters start clear of water and props");counts[e.type-PeatToad]++;if(e.type==BellKeeper)boss=e.id;}
 check(counts[0]>=3&&counts[1]>=3&&counts[2]>=3&&counts[3]==1,"three roaming species and one guardian");
 auto loaded=saves::decode(saves::encode(session));
 check(loaded.game.inMarsh()&&loaded.game.enemies.size()==g.enemies.size()&&loaded.game.chests[6].opened,"marsh save preserves encounters and loot");
 g.player=marsh::Arrival;check(g.useMarshPortal(),"return through the arrival gate");g.player=MagePosition;
 check(g.travelToMarsh()&&g.chests[7].opened,"return trip preserves the field");
 auto* keeper=g.findEnemy(boss);check(keeper!=nullptr,"guardian survives the field swap");
 g.player=keeper->pos+V{2,0};keeper->abilityCd=0;
 check(g.prepareEnemyAbility(*keeper,.01f,true,2)&&keeper->abilityBusy(),"the bell keeper telegraphs an expanding wave");
 g.developerSession=false;
 const float hpBefore=g.hp;
 for(int i=0;i<121;i++)g.advanceEnemyAbility(*keeper,1.f/60,keeper->pos,g.player,false,0);
 check(g.hp<hpBefore,"the resonant wave hits at its expanding edge");
 g.developerSession=true;g.player=keeper->pos;g.hit(*keeper,10000);
 check(g.marshBossDead&&g.dawnFragments()==3&&g.openCorpse(boss)&&g.quickTransfer({Place::Corpse,QuestLootSlot,boss}),"only looting the keeper awards the fourth shard");
 check(g.dawnFragments()==4&&g.takeTowerReturn(),"fourth tower return");
 g.enterMageTower();g.finishTowerReturn();g.player=MagePosition;
 check(g.travelToMarsh()&&g.findCorpse(boss)&&g.findCorpse(boss)->loot[QuestLootSlot].empty(),"the keeper and its shard never respawn");
}
