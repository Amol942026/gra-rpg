#pragma once
void volcanoSaveTests(){
 {auto bytes=saves::readBytes("art/volcano/river-detail/fixtures/legacy-v10.efs");saves::Archive file(true,bytes);std::string magic;int version;size_t size;uint64_t hash;file(magic,version,size,hash);saves::Archive payload(true,bytes.substr(file.at),10);saves::Snapshot original;payload(original);auto upgraded=saves::decode(bytes);
  check(version==10&&original.game.volcanoSceneryVersion==2&&upgraded.game.volcanoSceneryVersion==VolcanoSceneryRevision&&upgraded.game.props.size()>original.game.props.size(),"real v10 scenery revision gains additional volcanic objects without respawning the realm");
  original.game.props=upgraded.game.props;original.game.volcanoSceneryVersion=upgraded.game.volcanoSceneryVersion;
  check(saves::encode(original)==saves::encode(upgraded),"v10 scenery upgrade preserves every other serialized field including loot, quests and RNG");
  auto once=saves::encode(upgraded);check(saves::encode(saves::decode(once))==once,"new scenery revision migrates only once");}
 {auto migrating=saves::read("art/volcano/river-detail/fixtures/legacy-v10.efs");V occupied=migrating.game.props.front().pos;migrating.game.enemies[1].pos=occupied;migrating.game.volcanoSceneryVersion=2;migrating.game.refreshVolcanoScenery();
  check(migrating.game.walkable(occupied)&&distance(migrating.game.enemies[1].pos,occupied)==0,"new decoration migration keeps a currently wandering enemy clear without moving it");}
 auto v9=saves::read("art/volcano/polish/fixtures/legacy-v9.efs");
 check(v9.game.volcanoSceneryVersion==VolcanoSceneryRevision&&v9.game.props.size()>50,"real v9 save receives the clustered scenery revision");
 check(v9.game.hp==63&&v9.game.mana==21&&v9.game.gold==137&&distance(v9.game.player,VolcanoArrival)==0,"scenery migration preserves player position, resources and money");
 check(v9.game.bag[12].kind==Kind::GuardianAxe&&v9.game.bag[12].tier==2&&v9.game.chests[4].opened&&v9.game.chests[4].loot[0].empty()&&v9.game.volcanoFountainUsed[0],"scenery migration preserves gear, chest contents and used shrine");
 check(!v9.game.enemies[0].alive&&v9.game.enemies[0].hp==0&&v9.game.enemies[0].loot[0].count==12&&v9.game.desertBossDead&&v9.game.collectedFragments==2,"scenery migration preserves corpse loot, boss state and quest progression");
 auto migrated=saves::encode(v9);check(saves::encode(saves::decode(migrated))==migrated,"scenery migration runs only once and saves exactly");
 {auto bytes=saves::readBytes("art/volcano/polish/fixtures/legacy-v9.efs");saves::Archive file(true,bytes);std::string magic;int version;size_t size;uint64_t hash;file(magic,version,size,hash);saves::Archive payload(true,bytes.substr(file.at),9);saves::Snapshot original;payload(original);original.game.props=v9.game.props;
  saves::Archive before(false,{},9),after(false,{},9);before(original);after(v9);check(version==9&&before.bytes==after.bytes,"real v9 migration changes only scenery and preserves every other legacy byte including both RNG states");}
 auto stored=v9;stored.game.activateField(WorldLocation::Desert);stored.game.volcanoSceneryVersion=0;stored.game.volcanoState.props={};auto random=stored.game.rng,combat=stored.game.combatRng;stored.game.refreshVolcanoScenery();
 check(stored.game.volcanoState.props.size()>50&&stored.game.rng==random&&stored.game.combatRng==combat&&!stored.game.volcanoState.enemies[0].alive,"stored field scenery refresh preserves random streams and dead enemies");
 auto occupied=v9;occupied.game.player=occupied.game.props.front().pos;V position=occupied.game.player;occupied.game.volcanoSceneryVersion=0;occupied.game.refreshVolcanoScenery();
 check(distance(occupied.game.player,position)==0&&occupied.game.walkable(position),"layout refresh leaves a loaded hero in place without trapping them inside a new obstacle");
 auto legacy=saves::read("art/volcano/fixtures/legacy-v8.efs");
 check(!legacy.game.volcanoCreated&&!legacy.game.volcanoBossDead&&legacy.game.chests.size()==6&&legacy.game.chests[4].location==WorldLocation::Volcano,"real v8 save gains an unexplored third realm and two fresh chests");roundtrip(legacy);
 saves::Archive oldPayload(false,{},8);oldPayload(legacy);auto oldBytes=saves::readBytes("art/volcano/fixtures/legacy-v8.efs");saves::Archive oldFile(true,oldBytes);std::string magic;int version;size_t size;uint64_t hash;oldFile(magic,version,size,hash);
 check(version==8&&oldPayload.bytes==oldBytes.substr(oldFile.at),"legacy v8 migration preserves every previously serialized byte");
 saves::Snapshot s;s.game.activateField(WorldLocation::Volcano);s.game.player=VolcanoArrival;s.game.collectedFragments=2;s.game.desertBossDead=true;s.game.dawnQuest=DawnQuest::AtTower;roundtrip(s);
 s.game.player=VolcanoFountains[0]+V{1.6f,0};s.game.mana=2;check(s.game.useFountain(0),"save fixture uses actual third-realm mana shrine");s.game.update(.05f,{},false);roundtrip(s);
 s.game.player=TreasureLayout[4].pos+V{1.6f,0};check(s.game.openChest(4)&&s.game.quickTransfer({Place::Chest,0,4}),"save fixture opens and loots actual third-realm chest");roundtrip(s);
 auto before=saves::encode(s);s.game.activateField(WorldLocation::Desert);roundtrip(s);s.game.activateField(WorldLocation::Volcano);
 check(s.game.chests[4].opened&&s.game.chests[4].loot[0].empty()&&s.game.volcanoFountainUsed[0],"stored third realm restores looted chest and consumed shrine after switching fields");
 auto encounter=s;encounter.game.enemies.clear();encounter.game.props.clear();encounter.game.player={16,50};float hp=enemyInfo(VolcanoBossType).health;
 encounter.game.enemies.push_back({VolcanoBossId,VolcanoBossType,{12,50},{12,50},hp,hp});encounter.game.enemies[0].eruptionCd=0;
 encounter.game.update(.05f,{},false);check(encounter.game.fissures.size()==3,"save fixture contains actual active eruption warnings");roundtrip(encounter);
 auto resumed=saves::decode(saves::encode(encounter));for(int i=0;i<95;i++){encounter.game.update(1.f/60,{},false);resumed.game.update(1.f/60,{},false);}
 check(saves::encode(encounter)==saves::encode(resumed),"v9 restores exact eruption damage, expiry, movement and RNG");
 auto caster=s;caster.game.props.clear();caster.game.enemies.clear();caster.game.player={16,50};caster.game.enemies.push_back({2000,14,{12,50},{12,50},104,104});
 for(int i=0;i<35&&caster.game.cinders.empty();i++)caster.game.update(.02f,{},false);
 check(!caster.game.cinders.empty(),"save fixture contains a real caster projectile");roundtrip(caster);resumed=saves::decode(saves::encode(caster));
 for(int i=0;i<60;i++){caster.game.update(1.f/60,{},false);resumed.game.update(1.f/60,{},false);}check(saves::encode(caster)==saves::encode(resumed),"v9 resumes exact projectile trajectory and damage");
 s.tower.beginTalk(true,2,true);s.tower.enter(TowerPhase::Choice);roundtrip(s);check(s.tower.chooseVolcano(),"save fixture chooses volcanic teleport");roundtrip(s);
 auto invalid=s;invalid.game.volcanoCreated=false;rejects([&]{saves::encode(invalid);},"uninitialized active third realm refused");
 invalid=s;invalid.game.volcanoFountainUsed[0]=false;rejects([&]{saves::encode(invalid);},"draining unused third-realm shrine refused");
 invalid=caster;invalid.game.cinders.push_back({{16,50},{2,0},4,20,2000});rejects([&]{saves::encode(invalid);},"invalid projectile speed refused");
 invalid=encounter;invalid.game.enemies[0].eruptionWindup=1.2f;rejects([&]{saves::encode(invalid);},"oversized eruption preparation refused");
}
