#pragma once
// Included inside Game. Each field keeps corpses, wandering positions and forage.
void buildDesert(){
 enemies.clear();props.clear();forage.clear();
 for(size_t i=0;i<DesertSpawns.size();i++){const auto& s=DesertSpawns[i];float hp=enemyInfo(s.type).health;enemies.push_back({1000+int(i),s.type,s.pos,s.pos,hp,hp});enemies.back().patrolWait=.5f+(hash(int(i),99)%1000)*.001f;}
 float hp=enemyInfo(DesertBossType).health;enemies.push_back({DesertBossId,DesertBossType,DesertBossHome,DesertBossHome,hp,hp});
 for(int x=3;x<MapSize-3;x++)for(int y=3;y<MapSize-3;y++){
  V p{float(x),float(y)};unsigned h=hash(x+91,y+137);
  if(h%13||desertPath(p)||distance(p,DesertArrival)<6||distance(p,DesertOasis)<5.6f||distance(p,DesertBossHome)<6.8f)continue;
  bool clear=true;for(const auto& e:enemies)if(distance(p,e.home)<e.radius()+2.5f)clear=false;
  if(clear){const int types[]={7,7,8,9,9,11};props.push_back({p,types[(h>>8)%6],.8f+(h%6)*.065f});}
 }
 for(int i=0;i<7;i++){float a=i*6.2831853f/7;props.push_back({DesertOasis+V{std::cos(a),std::sin(a)}*4.9f,12,.8f+(i%3)*.08f});}
 for(V p:{V{40,13},V{46,13},V{40,17},V{46,17}})props.push_back({p,10,1});
 props.push_back({DesertPortal,13,1});desertCreated=true;
}
bool activateField(WorldLocation next){
 if(next!=WorldLocation::Forest&&next!=WorldLocation::Desert)return false;
 if(next!=fieldLocation){
  auto& saved=fieldLocation==WorldLocation::Forest?forestState:desertState;
  saved.enemies=std::move(enemies);saved.props=std::move(props);saved.forage=std::move(forage);
  auto& restore=next==WorldLocation::Forest?forestState:desertState;
  enemies=std::move(restore.enemies);props=std::move(restore.props);forage=std::move(restore.forage);
  fieldLocation=next;if(next==WorldLocation::Desert&&!desertCreated)buildDesert();
 }
 location=next;towerFloor=0;return true;
}
bool canTravelDesert()const{return mageReachable()&&dawnFragments()>=1;}
bool travelToDesert(){
 if(!canTravelDesert())return false;
 activateField(WorldLocation::Desert);player=DesertArrival;facing={1,0};animation={};animation.face(1,0);
 cancelBow();closeCorpse();rollRemaining=rollCd=swing=attackCd=spellCd=hurt=stepDistance=0;
 arrows.clear();fireballs.clear();explosions.clear();effects.clear();pierceFlashes.clear();healingPulse=0;
 say(desertBossDead?"Morze Piasków. Brama Eldrina prowadzi z powrotem do wieży.":"Morze Piasków. Odnajdź Żywą Wydmę na południowym wschodzie.");sound(Cue::Spell,player);return true;
}
bool useDesertPortal(){
 if(!inDesert()||dead()||allocationRequired||storyBlocked()||rolling())return false;
 if(distance(player,DesertPortal)>2.8f){say("Podejdź do Bramy Eldrina (E / PPM).");return false;}
 enterMageTower();dawnQuest=DawnQuest::AtTower;say("Powrót do wieży. Porozmawiaj z Eldrinem, aby wrócić na pustynię.");return true;
}
