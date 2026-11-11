#pragma once
// Included inside Game. Stable IDs let existing saves gain these encounters
// without replacing their inhabitants, corpses or random streams.
void addRegionalEnemies(WorldLocation realm,std::vector<Enemy>& inhabitants,
                        const std::vector<Prop>& scenery,const std::vector<Ground>& plants){
 const int type=realm==WorldLocation::Forest?SporekinType:realm==WorldLocation::Desert?DuneEyeType:CinderhornType;
 const auto& info=enemyInfo(type);
 const V arrival=realm==WorldLocation::Forest?ForestArrival:realm==WorldLocation::Desert?DesertArrival:VolcanoArrival;
 const std::array<V,RegionalEncountersPerRealm> sites=realm==WorldLocation::Forest?
     std::array<V,RegionalEncountersPerRealm>{{{10,99},{26,99},{14,60},{69,24}}}:
     realm==WorldLocation::Desert?
     std::array<V,RegionalEncountersPerRealm>{{{23,15},{36,27},{61,37},{82,61}}}:
     std::array<V,RegionalEncountersPerRealm>{{{15,50},{35,45},{51,27},{75,63}}};
 auto clear=[&](V p){
  const float pad=info.radius+.3f;
  const float spacing=realm==WorldLocation::Forest?8.f:5.5f;
  if(p.x<3||p.y<3||p.x>MapWalkMax-1||p.y>MapWalkMax-1||distance(p,arrival)<10||
     (location==realm&&distance(p,player)<EnemySightRange+2)||nearTreasure(p,realm,3))return false;
  if(realm==WorldLocation::Desert&&(desertWater(p,pad)||nearDesertFountain(p,pad)))return false;
  if(realm==WorldLocation::Volcano&&(volcanoLava(p,pad)||nearVolcanoFountain(p,pad)))return false;
  for(const auto& prop:scenery)if(distance(p,prop.pos)<pad+propRadius(prop.type))return false;
  for(const auto& e:inhabitants)if(distance(p,e.home)<spacing||distance(p,e.pos)<pad+e.radius()+.7f)return false;
  for(const auto& plant:plants)if(distance(p,plant.pos)<pad+.45f)return false;
  return true;
 };
 for(int slot=0;slot<RegionalEncountersPerRealm;slot++){
  const int id=3000+(type-SporekinType)*RegionalEncountersPerRealm+slot;
  if(std::any_of(inhabitants.begin(),inhabitants.end(),[&](const Enemy& e){return e.id==id;}))continue;
  V position=sites[slot];
  bool found=clear(position);
  for(int ring=1;ring<=24&&!found;ring++){
   for(int y=-ring;y<=ring&&!found;y++)for(int x=-ring;x<=ring&&!found;x++){
    if(std::max(std::abs(x),std::abs(y))!=ring)continue;
    V candidate=sites[slot]+V{float(x),float(y)};
    if(clear(candidate)){position=candidate;found=true;}
   }
  }
  if(!found)continue;
  inhabitants.push_back({id,type,position,position,info.health,info.health});
  inhabitants.back().patrolWait=.5f+.24f*slot;
 }
}
void restoreRegionalEnemies(){
 for(auto realm:{WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano}){
  if((realm==WorldLocation::Desert&&!desertCreated)||(realm==WorldLocation::Volcano&&!volcanoCreated))continue;
  auto& saved=storedField(realm);
  const bool active=fieldLocation==realm;
  auto& inhabitants=active?enemies:saved.enemies;
  const int guardian=realm==WorldLocation::Forest?13:realm==WorldLocation::Desert?DesertBossId:VolcanoBossId;
  // Small standalone test arenas don't contain the authored realm's guardian.
  if(std::none_of(inhabitants.begin(),inhabitants.end(),[&](const Enemy& e){return e.id==guardian&&enemyBoss(e.type);}))continue;
  addRegionalEnemies(realm,inhabitants,active?props:saved.props,active?forage:saved.forage);
 }
}
