#pragma once
void desertTests(){
 AnimationState sideStep;
 sideStep.face(1,-1);
 const float strideCycle=2*desert_motion::ScarabStride/desert_motion::data::Support;
 desert_motion::advance(sideStep,.12f,.12f,.05f,false,strideCycle);
 const auto stepAxis=desert_motion::scarabStepAxis(sideStep);
 const auto footLift=desert_motion::sample(.80f);
 check(sideStep.moving&&std::abs(stepAxis.x)>.99f&&std::abs(stepAxis.y)<.01f&&
       std::abs(sideStep.phase-4.8f/(ActorGroundSquash*strideCycle))<.0001f&&footLift.lift>.95f,
       "scarab footfalls advance through lateral travel with full swing height");
 const float phase=sideStep.phase;
 desert_motion::advance(sideStep,0,0,.05f,false,strideCycle);
 check(!sideStep.moving&&sideStep.phase==phase,"the desert stride stops when collision stops actual travel");
 check(desert_motion::sample(.25f).sway>.99f&&desert_motion::sample(.75f).sway<-.99f&&
       desert_motion::sample(0).sway==desert_motion::sample(1).sway,
       "cobra side travel crosses both extremes and loops continuously");
 {
  Game sand=emptyArena();
  sand.activateField(WorldLocation::Desert);
  sand.enemies.clear();sand.props.clear();
  const V home{35,27};
  sand.player=home+V{5,-5};
  sand.enemies.push_back({DesertBossId,DesertBossType,home,home,560,560});
  auto& dune=sand.enemies.front();
  dune.attack=10;dune.sandPrisonCd=100;
  int footfalls=0;
  bool bounded=true,allSides=true;
  for(int i=0;i<80;i++){
   sand.update(.02f,{},false);
   const auto facing=actorFacing(dune.animation.facingAngle);
   int stepCount=0,rimCount=0,sides=0;
   for(const auto& dust:sand.duneDust){
    bounded&=distance(dust.pos,dune.pos)<3&&dust.life>0&&dust.life<=desert_motion::DuneDustDuration;
    if(dust.life<=desert_motion::DuneDustDuration-.02f)continue;
    if(dust.grains==desert_motion::DuneDustGrains){++stepCount;continue;}
    if(dust.grains==desert_motion::DuneRimGrains)++rimCount;
    const V delta=dust.pos-dune.pos;
    const float x=(delta.x-delta.y)*40,y=(delta.x+delta.y)*20/ActorGroundSquash;
    const float right=x*facing.y-y*facing.x,front=x*facing.x+y*facing.y;
    sides|=std::abs(right)>std::abs(front)?(right>0?1:2):(front>0?4:8);
   }
   footfalls+=stepCount;
   if(stepCount)allSides&=sides==15&&rimCount==int(std::size(desert_motion::DuneDustRim))*stepCount;
  }
  check(distance(dune.pos,home)>.5f&&footfalls>=2&&bounded,
        "the advancing dune releases bounded grains on successive foot contacts");
  check(footfalls>=2&&allSides,"each footfall releases sand along all four sides of the base together");
  dune.strike=2;
  for(int i=0;i<=int(desert_motion::DuneDustDuration/.02f);i++)sand.update(.02f,{},false);
  check(!dune.animation.moving&&sand.duneDust.empty(),"a stationary dune leaves no fresh dust and the last particles expire");
 }
 auto allocate=[](Game& g){if(g.attributePoints){Attributes points{};points[int(Attribute::Vitality)]=g.attributePoints;g.distribute(points);}if(g.skillPoints){SkillRanks ranks{};ranks[0]=g.skillPoints;g.trainSkills(ranks);}};
 Game g;g.enterMageTower();g.player={12,12};check(!g.travelToDesert()&&g.inTower(),"normal travel is locked before the first story fragment");
 g=Game{};int firstId=g.enemies.back().id;g.player=g.enemies.back().pos;g.hit(g.enemies.back(),1000);allocate(g);check(g.bossDead&&g.dawnFragments()==0&&!g.towerReturnPending,"first boss death preserves the existing loot-before-return flow");g.openCorpse(firstId);check(g.transfer({Place::Corpse,4,firstId},{Place::Bag,8})&&g.dawnFragments()==1&&g.takeTowerReturn(),"first boss fragment unlocks the next story step exactly once");
 g.enterMageTower();g.finishTowerReturn();g.player={12,12};auto bag=g.bag;auto equipment=g.equipment;int level=g.level;Attributes attrs=g.attributes;float hp=g.hp,mana=g.mana;
 check(g.travelToDesert()&&g.inDesert()&&distance(g.player,DesertArrival)==0&&g.walkable(g.player),"Eldrin travel arrives at a safe position in the second realm");
 bool same=true;for(int i=0;i<BagSize;i++)same&=bag[i].kind==g.bag[i].kind&&bag[i].count==g.bag[i].count&&bag[i].tier==g.bag[i].tier;for(int i=0;i<EquipSize;i++)same&=equipment[i].kind==g.equipment[i].kind&&equipment[i].tier==g.equipment[i].tier;
 check(same&&g.level==level&&g.attributes==attrs&&g.hp==hp&&g.mana==mana,"desert travel retains inventory, gear, progression and resources");
 check(g.enemies.size()==DesertSpawns.size()+1+RegionalEncountersPerRealm&&g.enemies.back().type==DesertBossType&&g.enemies.back().id==DesertBossId&&g.forage.empty()&&g.ground.empty(),"second realm has its own monsters and boss without forest mushrooms or starter loot");
 for(const auto& e:g.enemies){check(g.enemyPositionClear(e.home,e.radius()),"every desert monster spawns clear of cactus, rock and oasis water");check(distance(e.home,DesertArrival)>6,"arrival clearing is protected from immediate monster contact");}
 for(size_t i=0;i<g.enemies.size();i++)for(size_t j=i+1;j<g.enemies.size();j++)check(distance(g.enemies[i].home,g.enemies[j].home)>g.enemies[i].radius()+g.enemies[j].radius()+.5f,"initial desert bodies have distinct non-overlapping spaces");
 std::array<bool,PropTypeCount> present{};for(const auto& p:g.props){present[p.type]=true;check(p.type>=7&&!g.walkable(p.pos),"every desert obstacle uses its own type and a real collision");}for(int i=7;i<LegacyPropTypeCount;i++)check(present[i],"cacti, sandstone, obelisks, dry shrubs, palms and the gate all occur on the map");
 check(!g.walkable(DesertOasis)&&g.walkable(DesertOasis+V{0,-3.4f}),"oasis water is impassable while its shore remains accessible");
 int palms=0,pairedCacti=0;bool plantsClearRoads=true;
 for(const auto& p:g.props){
  if(!Game::desertGroveProp(p.type))continue;
  plantsClearRoads&=!desertPath(p.pos,propRadius(p.type));
  if(p.type==12){palms++;check(distance(p.pos,DesertOasis)<8.5f&&!desertWater(p.pos,propRadius(12)),"palms stand on the oasis banks");}
  else if(std::any_of(g.props.begin(),g.props.end(),[&](const Prop& q){return &p!=&q&&(q.type==7||q.type==8)&&distance(p.pos,q.pos)<5.f;}))pairedCacti++;
 }
 check(palms>=6&&pairedCacti>=100&&plantsClearRoads,"oasis palms and cactus patches leave caravan routes open");
 std::array<bool,MapSize*MapSize> visited{};std::vector<V> queue{DesertArrival};visited[int(DesertArrival.y)*MapSize+int(DesertArrival.x)]=true;
 for(size_t head=0;head<queue.size();head++)for(V step:{V{1,0},V{-1,0},V{0,1},V{0,-1}}){V p=queue[head]+step;if(p.x<2||p.y<2||p.x>MapSize-3||p.y>MapSize-3)continue;int cell=int(p.y)*MapSize+int(p.x);if(!visited[cell]&&g.walkable(p)){visited[cell]=true;queue.push_back(p);}}
 for(const auto& e:g.enemies)check(visited[int(e.home.y)*MapSize+int(e.home.x)],"walkable desert routes connect arrival to every encounter including the second boss");
 g.addGround(item(Kind::Health),g.player);int desertDrop=g.ground.back().id;
 int scorpion=g.enemies.front().id;g.player=g.enemies.front().pos;g.hit(g.enemies.front(),1000);allocate(g);g.enemies.front().loot[1]=item(Kind::ScorpionSting,0,3);g.openCorpse(scorpion);g.bag[9]=item(Kind::ScorpionSting,0,98);check(g.transfer({Place::Corpse,1,scorpion},{Place::Bag,9})&&g.bag[9].count==99&&g.enemies.front().loot[1].count==2,"desert trophies stack safely with remainder kept in the corpse");
 g.player=DesertArrival;check(g.useDesertPortal()&&g.inTower()&&!g.get({Place::Ground,desertDrop})&&!g.get({Place::Corpse,0,scorpion}),"return gate preserves the realm but isolates its loot and corpses from the tower");g.player={12,12};check(g.travelToDesert()&&g.findCorpse(scorpion)&&g.get({Place::Ground,desertDrop})&&g.forage.empty(),"revisiting restores dead monsters and dropped loot instead of generating a fresh desert");
 g.activateField(WorldLocation::Forest);check(g.bossDead&&g.findCorpse(firstId)&&g.findCorpse(firstId)->loot[4].empty()&&g.forage.size()==MushroomCount&&!g.get({Place::Ground,desertDrop}),"returning to forest restores its original guardian corpse, collected shard and forage");g.activateField(WorldLocation::Desert);
 Enemy* boss=g.findEnemy(DesertBossId);check(boss&&boss->maxHp==560&&boss->damage()>enemyInfo(3).attack,"second boss is a distinct tougher encounter with sand-based attacks");g.player=boss->pos;g.hit(*boss,3000);allocate(g);boss=g.findCorpse(DesertBossId);check(g.desertBossDead&&g.dawnFragments()==1&&!g.towerReturnPending&&boss&&boss->loot[4].kind==Kind::DawnShard&&boss->loot[4].tier==3,"dune death leaves the guaranteed second fragment in its sand corpse until collected");
 g.openCorpse(DesertBossId);g.bag[10]=item(Kind::Sword);check(!g.transfer({Place::Corpse,4,DesertBossId},{Place::Bag,10})&&!boss->loot[4].empty()&&g.dawnFragments()==1,"occupied backpack preserves the second fragment without starting a return");g.bag[10]={};check(g.transfer({Place::Corpse,4,DesertBossId},{Place::Bag,10})&&g.dawnFragments()==2&&g.storyBlocked()&&g.takeTowerReturn()&&!g.takeTowerReturn(),"second fragment records two of five parts and emits exactly one tower return");
 g.enterMageTower();g.finishTowerReturn();check(g.bag[8].kind==Kind::DawnShard&&g.bag[10].kind==Kind::DawnShard&&g.dawnFragments()==2&&!g.drop({Place::Bag,10},g.player),"both quest fragments remain protected in the pack after the second return");check(g.transfer({Place::Bag,10},{Place::Bag,11})&&g.dawnFragments()==2&&!g.towerReturnPending,"reordering the second fragment cannot duplicate progress or replay the return");g.player={12,12};g.travelToDesert();check(g.findCorpse(DesertBossId)&&g.findCorpse(DesertBossId)->loot[4].empty()&&!g.findEnemy(DesertBossId)&&g.desertBossDead,"revisiting never respawns the second boss or duplicates its fragment");
 for(Kind kind:{Kind::ScorpionSting,Kind::ScarabShell,Kind::CobraScale,Kind::SandEssence})check(material(kind)&&stackable(kind)&&gearSlot(kind)<0&&power(item(kind))==0&&!materialDescription(kind).empty(),"new trophies are named stackable materials rather than equipment");
 TowerSequence scene;scene.beginTalk(true,1);for(int i=0;i<4000&&scene.phase!=TowerPhase::Choice;i++)scene.update(.05f);check(scene.phase==TowerPhase::Choice&&scene.active()&&!scene.takeDesertArrival(),"mage talk ends in a paused choice instead of automatically teleporting");for(int i=0;i<200;i++)scene.update(.05f);check(scene.phase==TowerPhase::Choice&&!scene.takeDesertArrival(),"waiting on the dialogue choice never starts travel");check(scene.chooseDesert()&&!scene.chooseDesert(),"travel can only be chosen once from the actual dialogue choice");int arrivals=0;for(int i=0;i<100;i++){scene.update(.05f);arrivals+=scene.takeDesertArrival();}check(arrivals==1&&!scene.active(),"desert teleport fades out, emits one arrival and finishes its reveal");
 scene.beginTalk(true,2);scene.enter(TowerPhase::Choice);scene.skip();check(!scene.active()&&!scene.takeDesertArrival(),"staying or escaping from the choice cannot teleport the character");scene.beginReturn(2);check(scene.current().text==DesertReturnDialogue[0].text&&scene.lineCount()==DesertReturnDialogue.size(),"second return has its own correct boss and fragment dialogue");
}
