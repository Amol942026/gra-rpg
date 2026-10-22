#include "../src/save_game.hpp"
#include <iostream>
using namespace ember;
namespace fs=std::filesystem;
int checks=0;
void check(bool ok,const char* label){checks++;if(!ok)throw std::runtime_error(label);}
template<class F>void rejects(F fn,const char* label){bool rejected=false;try{fn();}catch(...){rejected=true;}check(rejected,label);}
void roundtrip(const saves::Snapshot& snapshot){auto bytes=saves::encode(snapshot);auto restored=saves::decode(bytes);check(saves::encode(restored)==bytes,"all serialized fields roundtrip exactly");auto a=snapshot.game.rng,b=restored.game.rng,c=snapshot.game.combatRng,d=restored.game.combatRng;for(int i=0;i<40;i++)check(a()==b()&&c()==d(),"both random streams continue exactly");}
int main(int argc,char** argv){
 if(argc<2||std::string(argv[1])!="--no-audio")return 3;
 try{
  fs::path root=argc>2?fs::path(argv[2]):fs::path("dist")/("qa-save-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
  check(!fs::exists(root),"fresh isolated test directory");fs::create_directories(root);
  saves::Snapshot s;roundtrip(s);
  s.game.level=5;s.game.xp=47;s.game.gold=567;s.game.attributePoints=5;s.game.skillPoints=1;s.game.allocationRequired=true;
  s.game.bag[8]=item(Kind::GuardianSword,2);s.game.equipment[1]=item(Kind::GuardianHelmet);s.game.hp=37;s.game.mana=42;
  s.game.chests[0].opened=true;s.game.chests[0].loot[0]={};s.game.hit(s.game.enemies[1],10000);s.game.forage[0].value={};
  s.game.addGround(item(Kind::WolfFang,0,2),s.game.player);s.game.rng();s.game.combatRng();roundtrip(s);
  s.game.activateField(WorldLocation::Desert);s.game.collectedFragments=1;s.game.dawnQuest=DawnQuest::AtTower;s.game.player=DesertArrival;
  s.game.chests[2].opened=true;s.game.chests[2].loot[1]={};s.game.hit(s.game.enemies[0],10000);roundtrip(s);
  s.game.enterMageTower();s.game.towerFloor=4;s.game.towerRuneMask=7;roundtrip(s);
  s.game.activateField(WorldLocation::Forest);check(!s.game.enemies[1].alive&&s.game.forage[0].value.empty(),"forest state persisted across regions");roundtrip(s);
  s.game.activateField(WorldLocation::Desert);check(!s.game.enemies[0].alive,"desert corpse persisted across regions");roundtrip(s);
  s.game.enterMageTower();s.game.dawnQuest=DawnQuest::Returning;s.game.towerReturnPending=true;
  for(int phase=0;phase<9;phase++){s.tower={};s.tower.phase=TowerPhase(phase);s.tower.returning=true;s.tower.age=.17f;s.tower.time=4;s.tower.line=2;roundtrip(s);}
  s.tower={};for(int phase=0;phase<6;phase++){s.intro={};s.intro.phase=IntroPhase(phase);s.intro.line=7;s.intro.age=.28f;s.intro.time=9;s.intro.revealed=true;roundtrip(s);}s.intro={};
  auto combat=s;combat.game.arrows.push_back({{15,15},{2,3},7,24,true,3,{2,4},true});combat.game.fireballs.push_back({{16,16},{3,2},6,32,true});combat.game.explosions.push_back({{17,17},.12f});combat.game.pierceFlashes.push_back({{18,18},.17f});combat.game.healingCd=.3f;combat.game.rollCd=.9f;combat.game.sandPrisonRemaining=2;combat.game.weaponWheel[4]={Kind::MirageBow,2};roundtrip(combat);
  combat.game.hp=0;roundtrip(combat);
  auto bytes=saves::encode(s);
  for(size_t i=0;i<85;i++)rejects([&]{saves::decode(bytes.substr(0,i));},"truncated headers/payload rejected");
  for(size_t i:{bytes.size()/4,bytes.size()/2,bytes.size()-1})rejects([&]{saves::decode(bytes.substr(0,i));},"truncated world rejected");
  for(size_t i=0;i<bytes.size();i+=std::max(size_t(1),bytes.size()/64)){auto corrupt=bytes;corrupt[i]^=0x5a;rejects([&]{saves::decode(corrupt);},"modified save rejected");}
  rejects([&]{saves::decode(bytes+"extra");},"trailing bytes rejected");
  rejects([&]{saves::decode(std::string(saves::MaxBytes+1,'x'));},"oversized save rejected");
  auto invalid=s;invalid.game.bag[0].kind=Kind(999);rejects([&]{saves::encode(invalid);},"invalid item rejected");
  invalid=s;invalid.intro.phase=IntroPhase::Dialogue;invalid.intro.line=999;rejects([&]{saves::encode(invalid);},"invalid dialogue index rejected");
  invalid=s;invalid.game.player.x=std::numeric_limits<float>::quiet_NaN();rejects([&]{saves::encode(invalid);},"nonfinite position rejected");
  invalid=s;invalid.game.enemies.push_back(invalid.game.enemies[0]);rejects([&]{saves::encode(invalid);},"duplicate enemy ID rejected");
  invalid=s;invalid.game.equipment[0]=item(Kind::Boots);rejects([&]{saves::encode(invalid);},"wrong equipment slot rejected");
  auto envelope=[](saves::Snapshot snapshot,int version){saves::Archive payload(false);payload(snapshot);saves::Archive file(false);std::string magic="EMBERFALL-SAVE";size_t size=payload.bytes.size();uint64_t hash=saves::checksum(payload.bytes);file(magic,version,size,hash);return file.bytes+payload.bytes;};
  rejects([&]{saves::decode(envelope(s,2));},"unknown future version refused");
  invalid=s;invalid.intro.line=999;rejects([&]{saves::decode(envelope(invalid,1));},"valid checksum cannot bypass dialogue bounds");
  invalid=s;invalid.game.location=WorldLocation(999);rejects([&]{saves::decode(envelope(invalid,1));},"valid checksum cannot bypass location bounds");
  saves::Store store(root/fs::u8path(u8"Zapisy-żółć"));for(int i=0;i<3;i++){check(!store.inspect(i).exists,"empty slots reported");store.save(i,s);check(store.inspect(i).valid&&store.load(i).game.gold==567,"slot saved and loaded from Unicode directory");}
  auto previous=saves::readBytes(store.path(0));s.game.gold=999;store.save(0,s);auto current=saves::readBytes(store.path(0));check(previous!=current,"overwrite replaces state");check(saves::readBytes(fs::path(store.path(0).wstring()+L".bak"))==previous,"previous save retained exactly");
  {std::ofstream out(store.path(0),std::ios::binary|std::ios::trunc);out<<"broken";}
  bool backup=false;auto recovered=store.load(0,&backup);check(backup&&recovered.game.gold==567&&store.inspect(0).backup,"corrupted primary recovers valid backup");
  store.save(0,s);check(saves::readBytes(fs::path(store.path(0).wstring()+L".bak"))==previous,"corrupted primary cannot replace good backup");
  invalid=s;invalid.game.gold=-1;auto before=saves::readBytes(store.path(0));rejects([&]{store.save(0,invalid);},"invalid replacement refused");check(saves::readBytes(store.path(0))==before,"failed save preserves target");
  rejects([&]{store.path(-1);},"negative slot refused");rejects([&]{store.path(3);},"extra slot refused");
  fs::path blocked=root/"file-instead-of-directory";{std::ofstream out(blocked);out<<"keep";}saves::Store blockedStore(blocked);rejects([&]{blockedStore.save(0,s);},"filesystem failure surfaced");check(saves::readBytes(blocked)=="keep","unrelated file preserved");
  std::ofstream report(root/"report.txt");report<<"PASS: "<<checks<<" persistence checks\nAudio device opened: no\n";std::cout<<"PASS: "<<checks<<" persistence checks; "<<root.string()<<'\n';return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 2;}
}
