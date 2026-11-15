#pragma once
// Included inside Game. Use the named areas as a distribution grid, leaving
// the center of each area clear for ordinary travel and developer starts.
void addRealmDecorations(WorldLocation realm){
 if((realm==WorldLocation::Desert&&!desertCreated)||(realm==WorldLocation::Volcano&&!volcanoCreated))return;
 auto& field=storedField(realm);
 const bool active=fieldLocation==realm;
 auto& scenery=active?props:field.props;
 const auto& inhabitants=active?enemies:field.enemies;
 const auto& plants=active?forage:field.forage;
 const int guardian=realm==WorldLocation::Forest?13:realm==WorldLocation::Desert?DesertBossId:VolcanoBossId;
 const auto boss=std::find_if(inhabitants.begin(),inhabitants.end(),[&](const Enemy& e){return e.id==guardian&&enemyBoss(e.type);});
 if(boss==inhabitants.end())return;
 const int first=realm==WorldLocation::Forest?ForestDecorationStart:realm==WorldLocation::Desert?DesertDecorationStart:VolcanoDecorationStart;
 const V arrival=realm==WorldLocation::Forest?ForestArrival:realm==WorldLocation::Desert?DesertArrival:VolcanoArrival;
 std::vector<V> sites;
 for(const auto& site:FieldSites)if(site.realm==realm)sites.push_back(site.position);
 const auto clear=[&](V p,int type){
  const float radius=propRadius(type);
  if(p.x<3.5f||p.y<3.5f||p.x>MapWalkMax-2||p.y>MapWalkMax-2||
     distance(p,arrival)<2.2f||distance(p,boss->home)<5.5f||nearTreasure(p,realm,3)||
     (location==realm&&distance(p,player)<radius+1.2f))return false;
  if(realm==WorldLocation::Forest&&forestPath(p))return false;
  if(realm==WorldLocation::Desert&&(desertPath(p)||desertWater(p,radius+.65f)||nearDesertFountain(p,3)))return false;
  if(realm==WorldLocation::Volcano){
   if(volcanoLava(p,radius+1.4f)||nearVolcanoFountain(p,3)||volcano_roads::distanceTo(p)<2.65f+radius)return false;
   for(size_t i=0;i<VolcanoBridges.size();i++){
    const float y=VolcanoBridges[i].y;
    if(distance(p,{lavaCenter(VolcanoBridgeRivers[i],y),y})<6.5f)return false;
   }
  }
  for(V site:sites)if(distance(p,site)<2.25f)return false;
  for(const auto& old:scenery)if(distance(p,old.pos)<(old.type>=ForestDecorationStart?4.f:1.75f)+radius)return false;
  for(const auto& enemy:inhabitants)if(distance(p,enemy.home)<radius+enemy.radius()+.85f||
     distance(p,enemy.pos)<radius+enemy.radius()+.85f)return false;
  for(const auto& plant:plants)if(distance(p,plant.pos)<std::max(2.2f,radius+.8f))return false;
  for(const auto& drop:ground)if(drop.location==realm&&distance(p,drop.pos)<radius+1.2f)return false;
  return true;
 };
 for(size_t i=0;i<sites.size();i++)for(int group=0;group<2;group++){
  const int type=first+int((i*2+group)%DecorationsPerRealm);
  const float start=float(hash(int(i)*2+group,first)%360)*AnimationPi/180;
  for(int attempt=0;attempt<56;attempt++){
   const float angle=start+attempt*(2*AnimationPi/14);
   const float radius=3.4f+float(attempt/14)*1.2f;
   const V p=sites[i]+V{std::cos(angle),std::sin(angle)}*radius;
   if(!clear(p,type))continue;
   scenery.push_back({p,type,.86f+float(hash(int(i),type)%17)*.015f});
   break;
  }
 }
}
