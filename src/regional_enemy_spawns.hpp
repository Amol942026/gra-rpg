#pragma once
// Included inside Game. Stable IDs let existing saves gain these encounters
// without replacing their inhabitants, corpses or random streams.
void addRegionalEnemies(WorldLocation realm,std::vector<Enemy>& inhabitants,
                        const std::vector<Prop>& scenery,const std::vector<Ground>& plants,bool relocate=false){
 const int type=realm==WorldLocation::Forest?SporekinType:realm==WorldLocation::Desert?DuneEyeType:CinderhornType;
 const auto& info=enemyInfo(type);
 const V arrival=realm==WorldLocation::Forest?ForestArrival:realm==WorldLocation::Desert?DesertArrival:VolcanoArrival;
 const std::array<V,RegionalEncountersPerRealm> sites=realm==WorldLocation::Forest?
     std::array<V,RegionalEncountersPerRealm>{{{10,99},{26,99},{14,60},{69,24}}}:
     realm==WorldLocation::Desert?
     std::array<V,RegionalEncountersPerRealm>{{{23,15},{36,27},{61,37},{82,61}}}:
     std::array<V,RegionalEncountersPerRealm>{{{15,50},{40,87},{84,38},{116,106}}};
 auto clear=[&](V p,int ownId){
  const float pad=info.radius+.3f;
  const float spacing=realm==WorldLocation::Forest?8.f:5.5f;
  if(p.x<3||p.y<3||p.x>MapWalkMax-1||p.y>MapWalkMax-1||distance(p,arrival)<10||
     (location==realm&&distance(p,player)<EnemySightRange+2)||nearTreasure(p,realm,3))return false;
  if(realm==WorldLocation::Desert&&(desertWater(p,pad)||nearDesertFountain(p,pad)))return false;
  if(realm==WorldLocation::Volcano&&(volcanoLava(p,pad)||nearVolcanoFountain(p,pad)))return false;
  for(const auto& prop:scenery)if(distance(p,prop.pos)<pad+propRadius(prop.type))return false;
  for(const auto& e:inhabitants)if(e.id!=ownId&&(distance(p,e.home)<spacing||distance(p,e.pos)<pad+e.radius()+.7f))return false;
  for(const auto& plant:plants)if(distance(p,plant.pos)<pad+.45f)return false;
  return true;
 };
 for(int slot=0;slot<RegionalEncountersPerRealm;slot++){
  const int id=3000+(type-SporekinType)*RegionalEncountersPerRealm+slot;
  auto existing=std::find_if(inhabitants.begin(),inhabitants.end(),[&](const Enemy& e){return e.id==id;});
  if(existing!=inhabitants.end()&&(!relocate||!existing->alive))continue;
  V position=sites[slot];
  bool found=clear(position,id);
  for(int ring=1;ring<=24&&!found;ring++){
   for(int y=-ring;y<=ring&&!found;y++)for(int x=-ring;x<=ring&&!found;x++){
    if(std::max(std::abs(x),std::abs(y))!=ring)continue;
    V candidate=sites[slot]+V{float(x),float(y)};
    if(clear(candidate,id)){position=candidate;found=true;}
   }
  }
  if(!found)continue;
  if(existing!=inhabitants.end()){
   existing->home=position;
   existing->patrolGoalActive=false;
   const bool onScreen=location==realm&&distance(existing->pos,player)<EnemySightRange+PatrolRadius+6;
   if(!onScreen&&existing->windup<=0&&existing->strike<=0){
    existing->pos=position;
    existing->behavior=EnemyBehavior::Idle;
    existing->animation.moving=false;
   }
   continue;
  }
  inhabitants.push_back({id,type,position,position,info.health,info.health});
  inhabitants.back().patrolWait=.5f+.24f*slot;
 }
}
void restoreRegionalEnemies(WorldLocation realm,bool relocate=false){
 if((realm==WorldLocation::Desert&&!desertCreated)||(realm==WorldLocation::Volcano&&!volcanoCreated))return;
 auto& saved=storedField(realm);
 const bool active=fieldLocation==realm;
 auto& inhabitants=active?enemies:saved.enemies;
 const int guardian=realm==WorldLocation::Forest?13:realm==WorldLocation::Desert?DesertBossId:VolcanoBossId;
 // Small standalone test arenas don't contain the authored realm's guardian.
 if(std::none_of(inhabitants.begin(),inhabitants.end(),[&](const Enemy& e){return e.id==guardian&&enemyBoss(e.type);}))return;
 addRegionalEnemies(realm,inhabitants,active?props:saved.props,active?forage:saved.forage,relocate);
}

inline static constexpr int DesertReinforcementStart=3100;
void addDesertReinforcements(std::vector<Enemy>& inhabitants,const std::vector<Prop>& scenery,
                             const std::vector<Ground>& plants){
 const auto squared=[](V v){return v.x*v.x+v.y*v.y;};
 for(int sector=0;sector<6;sector++){
  const int left=sector%3*MapSize/3,top=sector/3*MapSize/2;
  const V center{left+MapSize/6.f,top+MapSize/4.f};
  for(int slot=0;slot<2;slot++){
   const int id=DesertReinforcementStart+2*sector+slot;
   const int type=(slot+sector)%2==0?DuneJackalType:GlassReaperType;
   const auto& info=enemyInfo(type);
   if(std::any_of(inhabitants.begin(),inhabitants.end(),[=](const Enemy& e){return e.id==id;}))continue;
   float best=-1e6f;
   V home{};
   for(int y=top+6;y<top+MapSize/2-5;y+=2)for(int x=left+6;x<left+MapSize/3-5;x+=2){
    const V p{float(x),float(y)};
    if(squared(p-DesertArrival)<144||squared(p-DesertOasis)<64||squared(p-DesertBossHome)<144||
       (location==WorldLocation::Desert&&squared(p-player)<100))continue;
    float nearest=1e6f;
    for(const auto& e:inhabitants){
     nearest=std::min({nearest,squared(p-e.home),squared(p-e.pos)});
     if(nearest<64)break;
    }
    if(nearest<64)continue;
    // Prefer the largest gap but keep the sector edges from becoming a second cluster.
    const float score=std::min(nearest,576.f)-.24f*squared(p-center);
    if(score<=best||desertWater(p,info.radius+.6f)||nearDesertFountain(p,3)||
       nearTreasure(p,WorldLocation::Desert,4)||nearFieldSite(WorldLocation::Desert,p,4))continue;
    if(std::any_of(scenery.begin(),scenery.end(),[&](const Prop& prop){
      const float radius=propRadius(prop.type)+info.radius+.65f;
      return squared(p-prop.pos)<radius*radius;
    })||std::any_of(plants.begin(),plants.end(),[&](const Ground& plant){return squared(p-plant.pos)<2.25f;}))continue;
    best=score;
    home=p;
   }
   if(best==-1e6f)continue;
   inhabitants.push_back({id,type,home,home,info.health,info.health});
   inhabitants.back().patrolWait=.55f+.17f*sector+.29f*slot;
  }
 }
}
void restoreDesertReinforcements(){
 if(!desertCreated)return;
 const bool active=fieldLocation==WorldLocation::Desert;
 auto& field=storedField(WorldLocation::Desert);
 auto& inhabitants=active?enemies:field.enemies;
 if(std::none_of(inhabitants.begin(),inhabitants.end(),[](const Enemy& e){
     return e.id==DesertBossId&&e.type==DesertBossType;
 }))return;
 addDesertReinforcements(inhabitants,active?props:field.props,active?forage:field.forage);
}
