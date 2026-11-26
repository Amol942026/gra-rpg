#pragma once
// Included inside Game. Each field keeps corpses, wandering positions and forage.
static bool desertGroveProp(int type){return type==7||type==8||type==12;}
static bool legacyDesertGroveProp(const Prop& p){
 if(p.type==12)return std::abs(distance(p.pos,DesertOasis)-4.9f)<.001f;
 if(!desertGroveProp(p.type))return false;
 const int x=int(p.pos.x),y=int(p.pos.y);
 if(p.pos.x!=x||p.pos.y!=y)return false;
 const unsigned h=hash(x+91,y+137);
 const int types[]={7,7,8,9,9,11};
 return h%13==0&&p.type==types[(h>>8)%6]&&std::abs(p.size-(.8f+(h%6)*.065f))<.001f;
}
void plantDesertGroves(std::vector<Prop>& scenery,const std::vector<Enemy>& inhabitants,const std::vector<Ground>& plants){
 const auto place=[&](V p,int type,float size){
  const bool palm=type==12,cactus=type==7||type==8;
  const float radius=propRadius(type);
  if(p.x<4||p.y<4||p.x>MapWalkMax-1.5f||p.y>MapWalkMax-1.5f||
     desertPath(p,radius+.32f)||nearDesertFountain(p,2.3f)||desertWater(p,radius+.32f)||
     distance(p,DesertArrival)<6||distance(p,DesertPortal)<4||distance(p,DesertBossHome)<6.8f||
     (!palm&&distance(p,DesertOasis)<8.5f)||nearFieldSite(WorldLocation::Desert,p,palm?2.5f:3.f))return false;
  for(const auto& chest:chests)if(chest.location==WorldLocation::Desert&&distance(p,chest.pos)<2.8f)return false;
  const float space=radius+(palm?1.05f:cactus?.8f:.55f);
  if(inDesert()&&distance(p,player)<space+HeroBodyRadius)return false;
  for(const auto& enemy:inhabitants){
   const float clearance=space+enemy.radius();
   if(distance(p,enemy.home)<clearance||distance(p,enemy.pos)<clearance||
      (enemy.patrolGoalActive&&distance(p,enemy.patrolGoal)<clearance))return false;
  }
  for(const auto& plant:plants)if(distance(p,plant.pos)<space+.7f)return false;
  for(const auto& drop:ground)if(drop.location==WorldLocation::Desert&&distance(p,drop.pos)<space+.65f)return false;
  for(const auto& old:scenery){
   const float gap=palm&&old.type==12?2.05f+.22f*(size+old.size):
       cactus&&desertGroveProp(old.type)?1.48f+.21f*(size+old.size):space+propRadius(old.type);
   if(!outsideCircleBounds(p-old.pos,gap)&&distance(p,old.pos)<gap)return false;
  }
  scenery.push_back({p,type,size});
  return true;
 };

 // Palms follow the bends of the oasis in irregular stands, with clear shore between them.
 static constexpr std::array<Prop,11> oasisPalms{{
  {{-3.52f,-.48f},12,1.07f},{{-4.45f,1.91f},12,1.16f},{{-2.17f,2.64f},12,.87f},
  {{-4.91f,4.61f},12,.93f},{{2.77f,1.84f},12,1.1f},{{1.42f,4.06f},12,1.13f},
  {{4.06f,4.78f},12,.89f},{{2.12f,6.9f},12,.92f},{{3.56f,-2.61f},12,1.14f},
  {{4.65f,-5.18f},12,.87f},{{2.1f,-4.9f},12,.95f}
 }};
 for(const auto& palm:oasisPalms)place(DesertOasis+palm.pos,palm.type,palm.size);

 // Tall cacti anchor each patch. Smaller opuntias and dry brush fill its edges.
 for(int x=7;x<MapSize-4;x+=9)for(int y=7;y<MapSize-4;y+=9){
  const unsigned h=hash(x+817,y+1291);
  if(h%100>=77)continue;
  const V center{float(x)+(int((h>>8)%49)-24)*.1f,float(y)+(int((h>>16)%49)-24)*.1f};
  const float bearing=float((h>>20)%360)*AnimationPi/180.f;
  const int wanted=2+int((h>>12)%3),dominant=(h>>15)%4==0?8:7;
  int planted=0;
  for(int i=0;i<8&&planted<wanted;i++){
   const unsigned pick=hash(x*13+i,y*17+317);
   const float angle=bearing+i*2.39996323f;
   const float spread=i==0?.36f:1.9f+float((pick>>8)%21)*.076f;
   const V p=center+V{std::cos(angle),std::sin(angle)}*spread;
   const int type=planted==0||pick%3!=0?dominant:8;
   const float size=planted==0?1.02f+float((pick>>17)%17)*.01f:.72f+float((pick>>17)%25)*.01f;
   if(place(p,type,size))++planted;
  }
  if(planted<2)continue;
  for(int i=0;i<4;i++){
   const float angle=bearing+.71f+i*2.39996323f;
   if(place(center+V{std::cos(angle),std::sin(angle)}*(2.6f+.42f*i),11,.72f+.06f*i))break;
  }
 }
}
void refreshDesertGroves(){
 if(!desertCreated)return;
 const bool active=fieldLocation==WorldLocation::Desert;
 auto& scenery=active?props:desertState.props;
 const auto& inhabitants=active?enemies:desertState.enemies;
 const auto& plants=active?forage:desertState.forage;
 if(std::none_of(inhabitants.begin(),inhabitants.end(),[](const Enemy& e){return e.id==DesertBossId&&e.type==DesertBossType;}))return;
 const auto retired=std::remove_if(scenery.begin(),scenery.end(),legacyDesertGroveProp);
 if(retired==scenery.end())return;
 scenery.erase(retired,scenery.end());
 plantDesertGroves(scenery,inhabitants,plants);
}

void buildDesert(){
 enemies.clear();props.clear();forage.clear();
 for(size_t i=0;i<DesertSpawns.size();i++){const auto& s=DesertSpawns[i];float hp=enemyInfo(s.type).health;enemies.push_back({1000+int(i)+(i>=99),s.type,s.pos,s.pos,hp,hp});enemies.back().patrolWait=.5f+(hash(int(i),99)%1000)*.001f;}
 float hp=enemyInfo(DesertBossType).health;enemies.push_back({DesertBossId,DesertBossType,DesertBossHome,DesertBossHome,hp,hp});
 for(V p:{V{40,13},V{46,13},V{40,17},V{46,17}})props.push_back({p,10,1});
 props.push_back({DesertPortal,13,1});
 plantDesertGroves(props,enemies,forage);
 for(int x=3;x<MapSize-3;x++)for(int y=3;y<MapSize-3;y++){
  V p{float(x),float(y)};unsigned h=hash(x+91,y+137);
  if(h%13||nearDesertFountain(p,2.3f)||desertPath(p)||nearTreasure(p,WorldLocation::Desert)||distance(p,DesertArrival)<6||distance(p,DesertOasis)<5.6f||distance(p,DesertBossHome)<6.8f||nearFieldSite(WorldLocation::Desert,p,2.4f))continue;
  bool clear=true;for(const auto& e:enemies)if(distance(p,e.home)<e.radius()+2.5f)clear=false;
  const int types[]={7,7,8,9,9,11};
  const int type=types[(h>>8)%6];
  if(!clear||desertGroveProp(type))continue;
  const float gap=propRadius(type)+.92f;
  if(std::none_of(props.begin(),props.end(),[&](const Prop& old){return !outsideCircleBounds(p-old.pos,gap)&&distance(p,old.pos)<gap;}))
   props.push_back({p,type,.8f+(h%6)*.065f});
 }
 const size_t guardianIndex=enemies.size()-1;
 addRegionalEnemies(WorldLocation::Desert,enemies,props,forage);
 addDesertReinforcements(enemies,props,forage);
 std::rotate(enemies.begin()+guardianIndex,enemies.begin()+guardianIndex+1,enemies.end());
 desertCreated=true;
 addRealmDecorations(WorldLocation::Desert);
}
bool activateField(WorldLocation next){
 if(next!=WorldLocation::Forest&&next!=WorldLocation::Desert&&next!=WorldLocation::Volcano&&next!=WorldLocation::Swamp)return false;
 if(location!=next){settleEnemyDeaths();cancelBoulderAttacks();}
 if(next!=fieldLocation){
  auto& saved=storedField(fieldLocation);
  saved.enemies=std::move(enemies);saved.props=std::move(props);saved.forage=std::move(forage);
  auto& restore=storedField(next);
  enemies=std::move(restore.enemies);props=std::move(restore.props);forage=std::move(restore.forage);
  fieldLocation=next;if(next==WorldLocation::Desert&&!desertCreated)buildDesert();if(next==WorldLocation::Volcano&&!volcanoCreated)buildVolcano();if(next==WorldLocation::Swamp&&!marshCreated)buildMarsh();
 }
 if(location!=next){clearSandPrison();duneDust.clear();closeCorpse();}location=next;towerFloor=0;refreshVolcanoScenery();return true;
}
bool canTravelDesert()const{return mageReachable()&&dawnFragments()>=1;}
bool travelToDesert(){
 if(!canTravelDesert())return false;
 activateField(WorldLocation::Desert);player=DesertArrival;facing={1,0};animation={};animation.face(1,0);
 cancelBow();closeCorpse();rollRemaining=rollCd=swing=attackCd=spellCd=hurt=stepDistance=0;
 arrows.clear();fireballs.clear();explosions.clear();effects.clear();combatImpacts.clear();pierceFlashes.clear();healingPulse=0;
 say(desertBossDead?"Morze Piasków. Brama Eldrina prowadzi z powrotem do wieży.":"Morze Piasków. Odnajdź Żywą Wydmę na południowym wschodzie.");sound(Cue::TeleportArrival,player);return true;
}
bool useDesertPortal(){
 if(sandTrapped()||!inDesert()||dead()||allocationRequired||storyBlocked()||rolling())return false;
 if(distance(player,DesertPortal)>2.8f){say("Podejdź do Bramy Eldrina (E / PPM).");return false;}
 enterMageTower();sound(Cue::TeleportArrival,player);dawnQuest=DawnQuest::AtTower;say("Powrót do wieży. Porozmawiaj z Eldrinem, aby wrócić na pustynię.");return true;
}
