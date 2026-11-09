#pragma once
// Verify the old prefix before aligning the deliberate appended world data for
// a complete byte comparison of all remaining progression/combat fields.
void verifyAndAlignWorldExpansion(saves::Snapshot& original,const saves::Snapshot& expanded){
 auto field=[](const auto& value){saves::Archive out(false);auto copy=value;out(copy);return out.bytes;};
 auto align=[&](auto& a,auto& b,auto& plants,const auto& x,const auto& y,const auto& grown,bool volcanic){
  check(x.size()>=a.size(),"migration never removes an original enemy or corpse");
  for(size_t i=0;i<a.size();i++)check(field(a[i])==field(x[i]),"each original enemy keeps its exact position, health, combat clocks and corpse loot");
  check(field(plants)==field(grown),"migration preserves all original gathered and ungathered plants exactly");
  if(!volcanic){check(y.size()>=b.size(),"forest/desert migration retains original scenery");for(size_t i=0;i<b.size();i++)check(field(b[i])==field(y[i]),"old forest/desert scenery keeps its exact coordinates and materials");
   for(size_t i=b.size();i<y.size();i++)check(y[i].pos.x>=71.5f||y[i].pos.y>=71.5f,"migration appends forest/desert scenery only outside the old field");}
  a=x;b=y;
 };
 auto& a=original.game;const auto& b=expanded.game;
 align(a.enemies,a.props,a.forage,b.enemies,b.props,b.forage,a.fieldLocation==WorldLocation::Volcano);
 align(a.forestState.enemies,a.forestState.props,a.forestState.forage,b.forestState.enemies,b.forestState.props,b.forestState.forage,false);
 align(a.desertState.enemies,a.desertState.props,a.desertState.forage,b.desertState.enemies,b.desertState.props,b.desertState.forage,false);
 align(a.volcanoState.enemies,a.volcanoState.props,a.volcanoState.forage,b.volcanoState.enemies,b.volcanoState.props,b.volcanoState.forage,true);
 a.worldExpansionVersion=b.worldExpansionVersion;
 // The later, explicitly requested v15 purchase policy clears old selection.
 check(b.learnedSpells==0&&b.selectedSpell==Spell::None,"legacy world migration also applies the explicit paid-spell selection policy");
 a.selectedSpell=b.selectedSpell;
}
void worldExpansionSaveTests(){
 check(saves::CurrentVersion>=12,"expanded realms retain version 12 fields and an explicit one-time migration revision");
 for(const char* realm:{"forest","desert","volcano"}){
  auto bytes=saves::readBytes(std::string("art/world-expansion/fixtures/")+realm+"-v11.efs");saves::Archive envelope(true,bytes);std::string magic;int version;size_t size;uint64_t hash;envelope(magic,version,size,hash);
  saves::Archive payload(true,bytes.substr(envelope.at),version);saves::Snapshot original;payload(original);auto expanded=saves::decode(bytes);
  check(version==11&&original.game.worldExpansionVersion==0&&expanded.game.worldExpansionVersion==WorldExpansionRevision,"real previous-release 72x72 fixture gains the new territory");
  verifyAndAlignWorldExpansion(original,expanded);original.game.volcanoSceneryVersion=expanded.game.volcanoSceneryVersion;
  check(saves::encode(original)==saves::encode(expanded),"complete snapshot retains every other byte: player, items, chest contents, shrines, quests and both RNG streams");
  auto& g=expanded.game;check(g.enemies.size()==(g.inDesert()?109:g.inVolcano()?121:181),"legacy active field receives the complete expanded population");
  check(g.player.x==(g.inDesert()?DesertArrival.x:g.inVolcano()?VolcanoArrival.x:1.6f)&&g.player.y==(g.inDesert()?DesertArrival.y:g.inVolcano()?VolcanoArrival.y:69.4f)&&g.walkable(g.player),"loaded hero stays at the exact old position with clear terrain");
  check(g.hp==67&&g.mana==43&&g.gold==127&&g.bag[12].kind==Kind::GuardianSword&&g.chests[4].loot[0].empty()&&g.fountainUsed[0]&&g.volcanoFountainUsed[1],"legacy resources, equipment, looted chest and consumed shrines are retained");
  auto once=saves::encode(expanded);check(saves::encode(saves::decode(once))==once,"expanded world saves exactly and does not append content a second time");roundtrip(expanded);
  auto dead=expanded;auto& world=dead.game;int id=world.inVolcano()?VolcanoBossId:world.inDesert()?DesertBossId:13;world.hit(*world.findEnemy(id),10000);world.allocationRequired=false;
  auto rng=world.rng,combat=world.combatRng;auto body=*world.findCorpse(id);size_t count=world.enemies.size();world.worldExpansionVersion=0;world.expandWorld();
  saves::Archive before(false),after(false);before(body);after(*world.findCorpse(id));
  check(!world.findEnemy(id)&&world.enemies.size()==count&&before.bytes==after.bytes&&world.rng==rng&&world.combatRng==combat,"migration never respawns a defeated guardian or rerolls its fragment and loot");
 }
 auto unvisited=saves::read("art/volcano/fixtures/legacy-v8.efs");check(!unvisited.game.volcanoCreated&&unvisited.game.volcanoState.enemies.empty(),"unvisited realms remain uncreated during load");
 unvisited.game.activateField(WorldLocation::Volcano);check(unvisited.game.enemies.size()==121,"first visit creates the expanded realm once");
 // Exercise the second migration independently with a v17-era active Caldera.
 saves::Snapshot previous;auto& caldera=previous.game;caldera.activateField(WorldLocation::Volcano);
 auto inExtension=[](V p,float limit){return p.x>=limit||p.y>=limit;};
 caldera.enemies.erase(std::remove_if(caldera.enemies.begin(),caldera.enemies.end(),[&](const Enemy& e){return inExtension(e.home,PreviousMapSize);}),caldera.enemies.end());
 caldera.props.erase(std::remove_if(caldera.props.begin(),caldera.props.end(),[&](const Prop& p){return inExtension(p.pos,PreviousMapSize-.5f);}),caldera.props.end());
 const size_t safeProps=caldera.props.size();
 caldera.props.push_back({{30.58011f,50.88041f},17,.92f});
 caldera.props.push_back({{21.01995f,46.48085f},14,.74f});
 caldera.player=VolcanoArrival;caldera.worldExpansionVersion=1;caldera.volcanoSceneryVersion=5;
 caldera.addGround(item(Kind::Health),caldera.player);
 const auto twice=saves::decode(saves::encode(previous));
 check(twice.game.worldExpansionVersion==WorldExpansionRevision&&twice.game.volcanoSceneryVersion==VolcanoSceneryRevision,"previous expansion migrates once to the larger field and cleared bridges");
 check(twice.game.enemies.size()==121&&twice.game.props.size()>safeProps&&twice.game.ground.size()==1,"previous Caldera gains only extension encounters and scenery, preserving dropped items");
 for(size_t i=0;i<safeProps;i++)check(distance(twice.game.props[i].pos,caldera.props[i].pos)==0,"existing scenery outside the two removed props retains its exact location");
 check(std::none_of(twice.game.props.begin(),twice.game.props.end(),Game::retiredVolcanoBridgeProp),"the two bridge obstructions are removed from existing saves");
 roundtrip(twice);
 auto invalid=unvisited;invalid.game.worldExpansionVersion=WorldExpansionRevision+1;rejects([&]{saves::encode(invalid);},"unknown world expansion revision rejected");
}
