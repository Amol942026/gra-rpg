#pragma once
// Verify the old prefix before aligning the deliberate appended world data for
// a complete byte comparison of all remaining progression/combat fields.
void verifyAndAlignWorldExpansion(saves::Snapshot& original,const saves::Snapshot& expanded){
 auto field=[](const auto& value){saves::Archive out(false);auto copy=value;out(copy);return out.bytes;};
 auto align=[&](auto& a,auto& b,auto& plants,const auto& x,const auto& y,const auto& grown,WorldLocation realm){
  check(x.size()>=a.size(),"migration never removes an original enemy or corpse");
  for(size_t i=0;i<a.size();i++)check(field(a[i])==field(x[i]),"each original enemy keeps its exact position, health, combat clocks and corpse loot");
  check(field(plants)==field(grown),"migration preserves all original gathered and ungathered plants exactly");
  if(realm!=WorldLocation::Volcano){
   const auto retained=[&](const auto& props,bool legacy){
    std::vector<Prop> result;
    for(const auto& prop:props){
     if(realm==WorldLocation::Forest&&forestGroveProp(prop.type))continue;
     if(legacy&&realm==WorldLocation::Desert&&Game::legacyDesertGroveProp(prop))continue;
     result.push_back(prop);
    }
    return result;
   };
   const auto old=retained(b,true),current=retained(y,false);
   check(current.size()>=old.size(),"forest/desert migration retains scenery outside the grove redesigns");
   for(size_t i=0;i<std::min(old.size(),current.size());i++)check(field(old[i])==field(current[i]),"retained forest/desert scenery keeps its exact coordinates and materials");
   for(size_t i=old.size();i<current.size();i++){
    const auto& prop=current[i];
    const bool desertPlant=realm==WorldLocation::Desert&&(Game::desertGroveProp(prop.type)||
       (prop.type==11&&(prop.pos.x!=std::trunc(prop.pos.x)||prop.pos.y!=std::trunc(prop.pos.y))));
    if(prop.type<ForestDecorationStart&&!desertPlant)
     check(prop.pos.x>=71.5f||prop.pos.y>=71.5f,"map expansion appends unrevised forest/desert scenery only outside the old field");
   }
  }
  a=x;b=y;
 };
 auto& a=original.game;const auto& b=expanded.game;
 align(a.enemies,a.props,a.forage,b.enemies,b.props,b.forage,a.fieldLocation);
 align(a.forestState.enemies,a.forestState.props,a.forestState.forage,b.forestState.enemies,b.forestState.props,b.forestState.forage,WorldLocation::Forest);
 align(a.desertState.enemies,a.desertState.props,a.desertState.forage,b.desertState.enemies,b.desertState.props,b.desertState.forage,WorldLocation::Desert);
 align(a.volcanoState.enemies,a.volcanoState.props,a.volcanoState.forage,b.volcanoState.enemies,b.volcanoState.props,b.volcanoState.forage,WorldLocation::Volcano);
 a.worldExpansionVersion=b.worldExpansionVersion;
 // The later, explicitly requested v15 purchase policy clears old selection.
 check(b.learnedSpells==0&&b.selectedSpell==Spell::None,"legacy world migration also applies the explicit paid-spell selection policy");
 a.selectedSpell=b.selectedSpell;
 check(a.chests[ForestCircleChestId].id==b.chests[ForestCircleChestId].id&&
       distance(b.chests[ForestCircleChestId].pos,ForestRitualCircle)==0,
       "original forest circle chest keeps its ID at the ritual center");
 a.chests[ForestCircleChestId].pos=b.chests[ForestCircleChestId].pos;
}
void worldExpansionSaveTests(){
 check(saves::CurrentVersion>=12,"expanded realms retain version 12 fields and an explicit one-time migration revision");
 for(bool active:{true,false}){
  saves::Snapshot before;auto& forest=before.game;
  const V proposed=std::find_if(forest.props.begin(),forest.props.end(),[](const Prop& p){return forestCanopy(p.type);})->pos;
  if(active)forest.player=proposed;
  forest.enemies.front().pos=proposed;
  forest.props.erase(std::remove_if(forest.props.begin(),forest.props.end(),[](const Prop& p){return forestGroveProp(p.type);}),forest.props.end());
  forest.props.push_back({ForestRitualCircle+V{1,0},5,1});
  if(!active)forest.activateField(WorldLocation::Desert);
  saves::Archive payload(false,{},25),header(false);payload(before);
  std::string magic="EMBERFALL-SAVE";int version=25;size_t size=payload.bytes.size();uint64_t hash=saves::checksum(payload.bytes);
  header(magic,version,size,hash);
  auto migrated=saves::decode(header.bytes+payload.bytes);
  const auto& scenery=active?migrated.game.props:migrated.game.forestState.props;
  check(std::any_of(scenery.begin(),scenery.end(),[](const Prop& p){return forestCanopy(p.type);})&&
        std::none_of(scenery.begin(),scenery.end(),[&](const Prop& p){return forestCanopy(p.type)&&(forestClearing(p.pos)||distance(p.pos,proposed)<1.5f);}),
        "v25 forest replaces its common scenery and leaves the occupied position and clearings open");
  if(active)check(migrated.game.walkable(proposed),"active migration keeps the loaded hero clear of new trees");
  auto& originalProps=active?forest.props:forest.forestState.props;
  originalProps.pop_back();
  const auto same=[&](const Prop& p,const Prop& q){return p.type==q.type&&p.size==q.size&&distance(p.pos,q.pos)==0;};
  check(scenery.size()>=originalProps.size()&&std::equal(originalProps.begin(),originalProps.end(),scenery.begin(),same),
        "v25 forest keeps the authored columns and decorations in place");
  originalProps=scenery;
  check(saves::encode(before)==saves::encode(migrated),"forest layout migration leaves all non-scenery state byte-identical");
  roundtrip(migrated);
 }
 for(bool active:{true,false}){
  saves::Snapshot before;auto& game=before.game;
  game.activateField(WorldLocation::Desert);
  std::array<V,4> occupied{};size_t count=0;
  for(const auto& p:game.props)if(p.type==7&&count<occupied.size())occupied[count++]=p.pos;
  check(count==occupied.size(),"new desert supplies cactus positions for the v26 migration fixture");
  game.props.erase(std::remove_if(game.props.begin(),game.props.end(),[](const Prop& p){return Game::desertGroveProp(p.type);}),game.props.end());
  game.props.push_back({DesertOasis+V{4.9f,0},12,.8f});
  const Prop custom{{36.2f,34.8f},8,.93f};game.props.push_back(custom);
  game.player=occupied[0];game.enemies.front().pos=occupied[1];
  game.enemies.front().patrolGoal=occupied[2];game.enemies.front().patrolGoalActive=true;
  game.addGround(item(Kind::Health),occupied[3]);
  if(!active)game.activateField(WorldLocation::Forest);
  saves::Archive payload(false,{},26),header(false);payload(before);
  std::string magic="EMBERFALL-SAVE";int version=26;size_t size=payload.bytes.size();uint64_t hash=saves::checksum(payload.bytes);
  header(magic,version,size,hash);
  const auto migrated=saves::decode(header.bytes+payload.bytes);
  const auto& scenery=active?migrated.game.props:migrated.game.desertState.props;
  check(std::any_of(scenery.begin(),scenery.end(),[](const Prop& p){return p.type==12;})&&
        std::none_of(scenery.begin(),scenery.end(),Game::legacyDesertGroveProp),"v26 desert replaces its old palm ring with the oasis groves");
  check(std::any_of(scenery.begin(),scenery.end(),[&](const Prop& p){return p.type==custom.type&&p.size==custom.size&&distance(p.pos,custom.pos)==0;}),
        "v26 desert keeps a non-generated cactus in its original position");
  for(size_t i=active?0:1;i<occupied.size();i++)check(std::none_of(scenery.begin(),scenery.end(),[&](const Prop& p){
   return distance(p.pos,occupied[i])<propRadius(p.type);
  }),"v26 desert keeps saved hero, enemies, patrol destinations and dropped items clear");
  auto& originalProps=active?game.props:game.desertState.props;
  originalProps=scenery;
  check(saves::encode(before)==saves::encode(migrated),"desert scenery migration leaves all other state byte-identical");
  roundtrip(migrated);
 }
 for(const char* realm:{"forest","desert","volcano"}){
  auto bytes=saves::readBytes(std::string("art/world-expansion/fixtures/")+realm+"-v11.efs");saves::Archive envelope(true,bytes);std::string magic;int version;size_t size;uint64_t hash;envelope(magic,version,size,hash);
  saves::Archive payload(true,bytes.substr(envelope.at),version);saves::Snapshot original;payload(original);auto expanded=saves::decode(bytes);
  check(version==11&&original.game.worldExpansionVersion==0&&expanded.game.worldExpansionVersion==WorldExpansionRevision,"real previous-release 72x72 fixture gains the new territory");
  verifyAndAlignWorldExpansion(original,expanded);original.game.volcanoSceneryVersion=expanded.game.volcanoSceneryVersion;
  check(saves::encode(original)==saves::encode(expanded),"complete snapshot retains every other byte: player, items, chest contents, shrines, quests and both RNG streams");
  auto& g=expanded.game;check(g.enemies.size()==(g.inDesert()?109:g.inVolcano()?121:181)+RegionalEncountersPerRealm,"legacy active field receives the complete expanded population");
  check(g.player.x==(g.inDesert()?DesertArrival.x:g.inVolcano()?VolcanoArrival.x:1.6f)&&g.player.y==(g.inDesert()?DesertArrival.y:g.inVolcano()?VolcanoArrival.y:69.4f)&&g.walkable(g.player),"loaded hero stays at the exact old position with clear terrain");
  check(g.hp==67&&g.mana==43&&g.gold==127&&g.bag[12].kind==Kind::GuardianSword&&g.chests[4].loot[0].empty()&&g.fountainUsed[0]&&g.volcanoFountainUsed[1],"legacy resources, equipment, looted chest and consumed shrines are retained");
  auto once=saves::encode(expanded);check(saves::encode(saves::decode(once))==once,"expanded world saves exactly and does not append content a second time");roundtrip(expanded);
  auto dead=expanded;auto& world=dead.game;int id=world.inVolcano()?VolcanoBossId:world.inDesert()?DesertBossId:13;world.hit(*world.findEnemy(id),10000);world.allocationRequired=false;
  auto rng=world.rng,combat=world.combatRng;auto body=*world.findCorpse(id);size_t count=world.enemies.size();world.worldExpansionVersion=0;world.expandWorld();
  saves::Archive before(false),after(false);before(body);after(*world.findCorpse(id));
  check(!world.findEnemy(id)&&world.enemies.size()==count&&before.bytes==after.bytes&&world.rng==rng&&world.combatRng==combat,"migration never respawns a defeated guardian or rerolls its fragment and loot");
 }
 auto unvisited=saves::read("art/volcano/fixtures/legacy-v8.efs");check(!unvisited.game.volcanoCreated&&unvisited.game.volcanoState.enemies.empty(),"unvisited realms remain uncreated during load");
 unvisited.game.activateField(WorldLocation::Volcano);check(unvisited.game.enemies.size()==121+RegionalEncountersPerRealm,"first visit creates the expanded realm once");
 // Exercise the second migration independently with a v17-era active Caldera.
 saves::Snapshot previous;auto& caldera=previous.game;caldera.activateField(WorldLocation::Volcano);
 auto inExtension=[](V p,float limit){return p.x>=limit||p.y>=limit;};
 caldera.enemies.erase(std::remove_if(caldera.enemies.begin(),caldera.enemies.end(),[&](const Enemy& e){
  return !regionalEnemy(e.type)&&inExtension(e.home,PreviousMapSize);
 }),caldera.enemies.end());
 caldera.props.erase(std::remove_if(caldera.props.begin(),caldera.props.end(),[&](const Prop& p){return inExtension(p.pos,PreviousMapSize-.5f);}),caldera.props.end());
 const size_t safeProps=caldera.props.size();
 caldera.props.push_back({{30.58011f,50.88041f},17,.92f});
 caldera.props.push_back({{21.01995f,46.48085f},14,.74f});
 caldera.player=VolcanoArrival;caldera.worldExpansionVersion=1;caldera.volcanoSceneryVersion=5;
 caldera.addGround(item(Kind::Health),caldera.player);
 const auto twice=saves::decode(saves::encode(previous));
 check(twice.game.worldExpansionVersion==WorldExpansionRevision&&twice.game.volcanoSceneryVersion==VolcanoSceneryRevision,"previous expansion migrates once to the larger field and cleared bridges");
 check(twice.game.enemies.size()==121+RegionalEncountersPerRealm&&twice.game.props.size()>safeProps&&twice.game.ground.size()==1,"previous Caldera gains only extension encounters and scenery, preserving dropped items");
 for(size_t i=0;i<safeProps;i++)check(distance(twice.game.props[i].pos,caldera.props[i].pos)==0,"existing scenery outside the two removed props retains its exact location");
 check(std::none_of(twice.game.props.begin(),twice.game.props.end(),Game::retiredVolcanoBridgeProp),"the two bridge obstructions are removed from existing saves");
 roundtrip(twice);
 auto invalid=unvisited;invalid.game.worldExpansionVersion=WorldExpansionRevision+1;rejects([&]{saves::encode(invalid);},"unknown world expansion revision rejected");
}
