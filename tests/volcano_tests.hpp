#pragma once
Game volcanoArena(int type=VolcanoBossType){Game g;g.activateField(WorldLocation::Volcano);g.props.clear();g.enemies.clear();g.player={16,50};float hp=enemyInfo(type).health;g.enemies.push_back({2099,type,{12,50},{12,50},hp,hp});return g;}
void volcanoTests(){
 for(int type:{SlagCrawlerType,FireImpType,AshWraithType,ObsidianSentinelType,EmberDrakeType}){
  Enemy patroller{2099,type,{12,50},{12,50},100,100};
  Game::advanceVolcanoAnimation(patroller,{.04f,.012f},1.f/60,true);
  const float precise=std::atan2((.04f+.012f)*.5f,.04f-.012f);
  check(std::abs(patroller.animation.facingAngle-precise)<.0001f,
        "a continuous-turn creature spawns facing its actual patrol path between the eight compass bearings");
  const float oldPhase=patroller.animation.phase;
  Game::faceEnemyTowards(patroller,{-1.f,-.6f});
  Enemy coarse=patroller;
  for(int i=0;i<42;++i){
   const float before=patroller.animation.facingAngle;
   Game::advanceVolcanoAnimation(patroller,{},1.f/60,false);
   check(std::abs(std::remainder(patroller.animation.facingAngle-before,2*AnimationPi))<=7.f/60+.00001f,
         "an abrupt reversal advances through intermediate poses at 60 Hz");
  }
  for(int i=0;i<21;++i)Game::advanceVolcanoAnimation(coarse,{},1.f/30,false);
  check(std::abs(patroller.animation.facingAngle-coarse.animation.facingAngle)<.0001f&&patroller.animation.phase==oldPhase,
        "the turn reaches the same angle at 30 and 60 Hz without moving the planted feet");
  patroller.animation.facingAngle=AnimationPi-.02f;patroller.animation.targetAngle=-AnimationPi+.02f;
  Game::advanceVolcanoAnimation(patroller,{},.05f,false);
  const float crossed=std::remainder(patroller.animation.facingAngle-(AnimationPi-.02f),2*AnimationPi);
  check(crossed>0&&crossed<.04f,"turning through west crosses the angular seam by the short arc");
  for(float side:{-1.f,1.f}){
   Enemy turning=patroller;
   turning.animation.facingAngle=0;turning.animation.targetAngle=side*2;turning.animation.turnLean=0;
   for(int i=0;i<10;++i)Game::advanceVolcanoAnimation(turning,{},1.f/60,false);
   check(turning.animation.turnLean*side>.65f,"the weight shifts into either turn");
   for(int i=0;i<90;++i)Game::advanceVolcanoAnimation(turning,{},1.f/60,false);
   check(std::abs(turning.animation.turnLean)<.001f,"bank settles to the neutral pose after turning");
  }
 }
 Game g;g.enterMageTower();g.player=MagePosition;check(!g.canTravelVolcano()&&!g.travelToVolcano(),"third realm is locked before second boss dies");
 g.desertBossDead=true;g.collectedFragments=1;check(g.canTravelVolcano()&&g.travelToVolcano()&&g.inVolcano(),"killing the second boss unlocks explicit mage travel, independent of moving its loot");
 check(distance(g.player,VolcanoArrival)==0&&g.walkable(g.player)&&distance(g.player,VolcanoBossHome)>45,"third arrival is safe and far from its boss");
 check(g.enemies.size()==121+RegionalEncountersPerRealm&&g.volcanoCreated&&g.forage.empty(),"third realm includes the original population and new regional enemies");
 std::array<int,VolcanoLandmarkCount> landmarks{};
 unsigned landmarkQuadrants=0;
 for(const auto& prop:g.props)if(volcanoLandmark(prop.type)){
  ++landmarks[prop.type-VolcanoLandmarkStart];
  landmarkQuadrants|=1u<<((prop.pos.x>=72?1:0)+(prop.pos.y>=72?2:0));
  const float radius=propRadius(prop.type);
  check(!g.walkable(prop.pos)&&!volcanoLava(prop.pos,radius+1.4f)&&
        volcano_roads::distanceTo(prop.pos)>=2.55f+radius&&!nearVolcanoFountain(prop.pos,3.4f),
        "new volcanic landmarks have collision and leave roads, lava banks and fountains clear");
 }
 check(std::all_of(landmarks.begin(),landmarks.end(),[](int count){return count==4;})&&landmarkQuadrants==15,
       "each of the four new landmarks is placed four times, spread over all map quadrants");
 {
  int portal=0,fractional=0;std::array<int,4> types{};
  for(const auto& p:g.props){if(p.type==13)portal++;if(!volcanoSceneryProp(p.type))continue;types[p.type-14]++;fractional+=std::abs(p.pos.x-std::round(p.pos.x))>.01f;
   check(!volcanoLava(p.pos,propRadius(p.type)+.8f)&&!nearVolcanoFountain(p.pos,3),"clustered scenery keeps river banks and shrines clear");
   const size_t roads=(p.pos.x>=PreviousMapSize-.5f||p.pos.y>=PreviousMapSize-.5f)?VolcanoRoads.size():9;
   for(size_t road=0;road<roads;road++)for(int i=1;i<6;i++)check(desertSegmentDistance(p.pos,VolcanoRoads[road][i-1],VolcanoRoads[road][i])>=1.7f+propRadius(p.type),"scenery preserves the clearance of roads in its region");
  }
  check(portal==1&&fractional>100&&std::count_if(g.props.begin(),g.props.end(),[](const Prop& p){return volcanoSceneryProp(p.type);})<=416,"irregular scenery covers the expanded realm and retains one return portal");
  for(int count:types)check(count>=8,"all four scenery families form visible groups");
  for(size_t i=0;i<g.props.size();i++)for(size_t j=i+1;j<g.props.size();j++)if(volcanoSceneryProp(g.props[i].type)&&volcanoSceneryProp(g.props[j].type))
   check(distance(g.props[i].pos,g.props[j].pos)>4.4f,"large scenery silhouettes are separated instead of forming crowded clumps");
 }
 for(const auto& road:VolcanoRoads)for(int section=1;section<6;section++)for(int sample=0;sample<=100;sample++)check(!volcanoLava(road[section-1]+(road[section]-road[section-1])*(sample/100.f)),"every marked road section crosses lava only at an actual safe bridge");
 std::array<int,7> species{};for(const auto& e:g.enemies){if(e.type<=VolcanoBossType)species[e.type-VolcanoFirstType]++;check(g.enemyPositionClear(e.home,e.radius())&&!volcanoLava(e.home,e.radius()),"every volcanic enemy spawns on safe ground with body clearance");}
 for(int i=0;i<6;i++)check(species[i]==20,"six volcanic species have twenty independent encounters each");check(species[6]==1,"one third boss, distinct from both earlier bosses");
 {
  auto wandering=g;for(int frame=0;frame<180;frame++){wandering.update(1.f/60,{},false);for(const auto& e:wandering.enemies)check(!volcanoLava(e.pos,e.radius()),"real volcanic patrols keep their complete bodies out of lava");}
  for(int fps:{30,60,120})for(int mode=0;mode<3;mode++){
   auto crossing=g;crossing.enemies.clear();crossing.props.clear();float edge=lavaCenter(0,40)-1.6f;crossing.player={edge-.1f,40};
   if(mode==2)check(crossing.startRoll({1,0}),"roll starts beside the lava bank");
   for(int i=0;i<fps;i++)crossing.update(1.f/fps,{1,0},false,mode==1);
   check(!volcanoLava(crossing.player)&&crossing.player.x<=edge+.001f,"walk, sprint and roll cannot cross lava at 30/60/120 Hz");
  }
  for(int i=0;i<4;i++){V p{lavaCenter(i/2,VolcanoBridges[i].y),VolcanoBridges[i].y};check(g.walkable(p)&&!volcanoLava(p,1.3f),"all four bridges have clearance for full-sized bodies");}
 }
 // Half-cell flood fill checks the actual map and obstacle layout, not just a drawn road.
 constexpr int N=MapSize*2;std::vector<unsigned char> visited(N*N);std::vector<int> queue;int start=int(g.player.y*2)*N+int(g.player.x*2);queue.push_back(start);visited[start]=1;
 for(size_t at=0;at<queue.size();at++){int cell=queue[at],x=cell%N,y=cell/N;for(V offset:{V{1,0},V{-1,0},V{0,1},V{0,-1}}){int a=x+int(offset.x),b=y+int(offset.y);if(a<0||b<0||a>=N||b>=N||visited[b*N+a])continue;V p{a*.5f,b*.5f};if(g.walkable(p)){visited[b*N+a]=1;queue.push_back(b*N+a);}}}
 auto reachable=[&](V target,float range){for(int cell:queue)if(distance({(cell%N)*.5f,(cell/N)*.5f},target)<=range)return true;return false;};
 check(queue.size()>9000,"large connected walkable third level");check(reachable(VolcanoBossHome,1.7f),"actual bridges lead from arrival to the boss arena");
 for(const auto& e:g.enemies)check(reachable(e.home,1.6f),"all encounters are reachable from the safe arrival");
 int chestCount=0;for(const auto& c:g.chests)if(c.location==WorldLocation::Volcano){chestCount++;check(reachable(c.pos,ChestUseRange),"both new chests have reachable approaches");for(auto i:c.loot)check(!i.empty()&&gearSlot(i.kind)>=0&&(itemRarity(i.kind)==Rarity::Common||itemRarity(i.kind)==Rarity::Rare),"volcanic chest yields ordinary or rare equipment");}
 check(chestCount==2,"exactly two third-realm treasure chests");
 for(int id=0;id<2;id++){
  check(reachable(VolcanoFountains[id],FountainUseRange),"both third-realm shrines are reachable");g.player=VolcanoFountains[id]+V{1.6f,0};g.hp=20;g.mana=9;
  check(g.useFountain(id)&&g.fountainSpent(id)&&(id==0?g.mana==g.maxMana():g.hp==g.maxHp()),"third-realm shrines fully restore their own resource");
  check(!g.useFountain(id),"third-realm shrine has exactly one use");
 }
 check(g.fountainUsed==std::array<bool,2>{},"using volcanic shrines does not exhaust desert shrines");
 g.player=g.chests[4].pos+V{1.7f,0};check(g.openChest(4),"third chest opens with ordinary container controls");
 check(g.quickTransfer({Place::Chest,0,4}),"double-click transfer works for third-realm chest");
 int corpseId=g.enemies[0].id;g.hit(g.enemies[0],10000);g.allocationRequired=false;g.dawnQuest=DawnQuest::AtTower;g.collectedFragments=2;
 g.player=VolcanoPortal;check(g.useVolcanoPortal()&&g.inTower(),"third portal returns to the existing tower");g.player=MagePosition;
 check(g.travelToVolcano()&&g.findCorpse(corpseId)&&g.chests[4].opened&&g.chests[4].loot[0].empty()&&g.volcanoFountainUsed[0],"revisiting preserves bodies, chest loot and used shrines without respawn");
 g=volcanoArena();auto& boss=g.enemies[0];boss.eruptionCd=0;g.update(.05f,{},false);
 check(boss.eruptionWindup>0&&g.fissures.size()==3&&g.hp==g.maxHp(),"third boss warns with three delayed ground eruptions");
 for(int i=0;i<25;i++)g.update(.05f,{},false);
 check(g.hp<g.maxHp()&&boss.eruptions==1,"eruption damages the target once and respects its cooldown");
 g=volcanoArena();g.enemies[0].eruptionCd=0;g.update(.05f,{},false);g.player={20,55};for(int i=0;i<25;i++)g.update(.05f,{},false);check(g.hp==g.maxHp(),"moving out of the marked eruption avoids damage");
 for(int type:{14,15}){
  g=volcanoArena(type);for(int i=0;i<68;i++)g.update(.02f,{},false);check(g.hp<g.maxHp(),"both volcanic casters fire real damaging projectiles");
  g=volcanoArena(type);g.launchCinder(g.enemies[0]);g.props.push_back({{14,50},14,1});for(int i=0;i<30;i++)g.updateVolcanoAttacks(.02f,g.player,false,0);check(g.hp==g.maxHp()&&g.cinders.empty(),"cover intercepts caster projectiles");
 }
 for(const int caster:{FireImpType,AshWraithType}){
  g=volcanoArena(caster);const auto origin=g.enemies[0].pos;
  g.launchCinder(g.enemies[0]);g.updateVolcanoAttacks(1.6f,g.player,false,0);
  check(g.hp<g.maxHp()&&g.cinders.empty()&&g.explosions.size()==size_t(caster==FireImpType),"only the imp fireball bursts on swept contact");
  if(!g.explosions.empty()){
   check(std::abs(distance(g.explosions.front().pos,g.player)-.42f)<.001f,"fireball burst appears at the near surface of the hero");
   const float after=g.hp;g.updateVolcanoAttacks(.1f,g.player,false,0);check(g.hp==after&&g.explosions.size()==1,"impact and visual fireburst happen exactly once");
  }
  g=volcanoArena(caster);g.player=origin+V{10,0};g.launchCinder(g.enemies[0]);g.updateVolcanoAttacks(2,g.player,false,0);
  check(g.hp==g.maxHp()&&g.explosions.empty()&&g.cinders.empty(),"large time step cannot carry a hostile fireball past maximum range");
 }
 g=volcanoArena(FireImpType);g.props.push_back({{14,50},14,1});g.launchCinder(g.enemies[0]);g.updateVolcanoAttacks(1.f,g.player,false,0);
 check(g.hp==g.maxHp()&&g.explosions.size()==1&&g.explosions[0].pos.x<14&&g.explosions[0].pos.x>12,"imp fireball bursts on the first physical obstruction");
 g=volcanoArena(FireImpType);g.player={13,50};g.launchCinder(g.enemies[0]);g.updateVolcanoAttacks(.2f,g.player,true,0);
 check(g.hp==g.maxHp()&&g.explosions.empty()&&g.cinders.empty(),"protected dodge avoids the imp fireball and a false impact");
 g=volcanoArena();g.collectedFragments=2;g.level=30;g.dawnQuest=DawnQuest::AtTower;g.hit(g.enemies[0],10000);g.allocationRequired=false;g.player=g.enemies[0].pos;
 check(g.volcanoBossDead&&g.enemies[0].loot[QuestLootSlot].kind==Kind::DawnShard&&g.enemies[0].loot[QuestLootSlot].tier==4,"third boss leaves guaranteed third story fragment");
 check(g.openCorpse(VolcanoBossId)&&g.quickTransfer({Place::Corpse,QuestLootSlot,VolcanoBossId})&&g.dawnFragments()==3&&g.takeTowerReturn(),"taking third fragment triggers exactly the third tower return");
 g.enterMageTower();g.finishTowerReturn();g.player=MagePosition;g.desertBossDead=true;
 check(g.travelToVolcano()&&g.findCorpse(VolcanoBossId)&&g.findCorpse(VolcanoBossId)->loot[QuestLootSlot].empty()&&!g.findEnemy(VolcanoBossId),"returning to the third realm never respawns its boss or story fragment");
 TowerSequence story;story.beginTalk(true,2,true);story.enter(TowerPhase::Choice);for(int i=0;i<100;i++)story.update(.05f);check(story.phase==TowerPhase::Choice&&!story.takeDesertArrival(),"third realm requires an explicit dialogue choice");
 check(story.chooseVolcano()&&story.travelVolcano&&!story.chooseVolcano(),"mage volcanic travel can be selected only once");int arrivals=0;for(int i=0;i<100;i++){story.update(.05f);arrivals+=story.takeDesertArrival();}check(arrivals==1&&!story.active(),"third teleport emits one arrival and finishes its reveal");
 story.beginReturn(3);check(std::string(story.current().text).find("Varkhor")!=std::string::npos,"third return names its own boss and fragment");
 for(int type=VolcanoFirstType;type<EnemyTypeCount;type++)for(auto rule:enemyLootTable(type))if(rule.kind!=Kind::None)check(rule.chance()==itemDropChance(rule.kind),"all new enemy loot follows the established rarity probabilities");
}
