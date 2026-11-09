#pragma once
// Included within Game. Migrate existing realms once, preserving their original
// enemy objects/corpses, loot, gathering, player state and both random streams.
void expandWorld(){
 if(worldExpansionVersion>=WorldExpansionRevision)return;
 const int previousSize=worldExpansionVersion==0?LegacyMapSize:PreviousMapSize;
 Game authored;
 for(auto realm:{WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano}){
  if((realm==WorldLocation::Desert&&!desertCreated)||(realm==WorldLocation::Volcano&&!volcanoCreated))continue;
  authored.activateField(realm);bool active=fieldLocation==realm;
  auto& inhabitants=active?enemies:storedField(realm).enemies;
  auto& scenery=active?props:storedField(realm).props;
  int guardian=realm==WorldLocation::Forest?13:realm==WorldLocation::Desert?DesertBossId:VolcanoBossId;
  int guardianType=realm==WorldLocation::Forest?3:realm==WorldLocation::Desert?DesertBossType:VolcanoBossType;
  // Custom diagnostic arenas are not authored realms and gain no strangers.
  if(std::none_of(inhabitants.begin(),inhabitants.end(),[&](const auto& e){return e.id==guardian&&e.type==guardianType;}))continue;
  for(const auto& e:authored.enemies){
   if(e.home.x<previousSize&&e.home.y<previousSize)continue;
   if(std::any_of(inhabitants.begin(),inhabitants.end(),[&](const auto& old){return old.id==e.id;}))continue;
   if(location==realm&&distance(e.home,player)<EnemySightRange+PatrolRadius+1)continue;
   inhabitants.push_back(e);
  }
  if(realm==WorldLocation::Volcano)continue; // Its independent scenery revision preserves the old volcanic placement.
  for(const auto& p:authored.props){
   if(p.pos.x<previousSize-.5f&&p.pos.y<previousSize-.5f)continue;
   if(std::any_of(scenery.begin(),scenery.end(),[&](const auto& old){return distance(old.pos,p.pos)<.01f;}))continue;
   float radius=propRadius(p.type);bool clear=true;
   if(location==realm&&distance(p.pos,player)<radius+1.2f)clear=false;
   for(const auto& e:inhabitants)if(distance(p.pos,e.pos)<radius+e.radius()+.6f)clear=false;
   for(const auto& drop:ground)if(drop.location==realm&&distance(p.pos,drop.pos)<radius+1.2f)clear=false;
   if(clear)scenery.push_back(p);
  }
 }
 worldExpansionVersion=WorldExpansionRevision;
}
