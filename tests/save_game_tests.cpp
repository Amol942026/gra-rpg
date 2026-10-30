#include "../src/save_game.hpp"
#include <iostream>
using namespace ember;
namespace fs=std::filesystem;
int checks=0;
void check(bool ok,const char* label){checks++;if(!ok)throw std::runtime_error(label);}
template<class F>void rejects(F fn,const char* label){bool rejected=false;try{fn();}catch(...){rejected=true;}check(rejected,label);}
void roundtrip(const saves::Snapshot& snapshot){auto bytes=saves::encode(snapshot);auto restored=saves::decode(bytes);check(saves::encode(restored)==bytes,"all serialized fields roundtrip exactly");auto a=snapshot.game.rng,b=restored.game.rng,c=snapshot.game.combatRng,d=restored.game.combatRng;for(int i=0;i<40;i++)check(a()==b()&&c()==d(),"both random streams continue exactly");}
#include "boar_save_tests.hpp"
#include "volcano_save_tests.hpp"
int main(int argc,char** argv){
 if(argc<2||std::string(argv[1])!="--no-audio")return 3;
 try{
  fs::path root=argc>2?fs::path(argv[2]):fs::path("dist")/("qa-save-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
  check(!fs::exists(root),"fresh isolated test directory");fs::create_directories(root);
  boarSaveTests();
  volcanoSaveTests();
  saves::Snapshot s;roundtrip(s);
  for(int k=int(Kind::SlimeBlade);k<ItemKindCount;k++){
   auto extra=s;extra.game.bag[10]=item(Kind(k),2);extra.game.equipment[gearSlot(Kind(k))]=item(Kind(k),1);
   extra.game.enemies[0].alive=false;extra.game.enemies[0].hp=0;extra.game.enemies[0].loot[14]=item(Kind(k));roundtrip(extra);
   check(saves::decode(saves::encode(extra)).game.bag[10].kind==Kind(k),"every new item ID persists in bags, equipment and corpse loot");
  }
  auto legacy=saves::read("art/gameplay/guardian/fixtures/legacy-v1.efs");
  check(distance(legacy.game.player,{8,19})==0&&legacy.game.hp==57&&legacy.game.gold==123&&legacy.game.bag[8].kind==Kind::GuardianSword,"real pre-change v1 save preserves existing position and inventory");
  check(legacy.game.boulders.empty()&&legacy.game.enemies.back().boulderCd==BoulderFirstDelay,"v1 saves receive safe default boss timers");roundtrip(legacy);
  auto legacy2=saves::read("art/gameplay/guardian/fixtures/legacy-v2.efs");
  check(legacy2.game.enemies[0].boulderWindup>0&&!legacy2.game.knockedBack()&&length(legacy2.game.heroKnockback)==0,"real previous v2 player save retains its pending throw with safe recoil defaults");roundtrip(legacy2);
  auto legacy3=saves::read("art/gameplay/guardian/fixtures/legacy-v3.efs");
  check(legacy3.game.knockedBack()&&legacy3.game.heroKnockbackRemaining>0&&legacy3.game.knockdownProgress()<0,"real v3 mid-slide save loads with no invented fall progress");roundtrip(legacy3);
  auto legacy4=saves::read("art/gameplay/guardian/fixtures/legacy-v4.efs");
  check(legacy4.game.knockedBack()&&legacy4.game.heroKnockdownRemaining>HeroKnockdownDuration-.1f&&legacy4.game.heroKnockbackRemaining>.45f,"real v4 save migrates normalized fall and slide clocks into the current clip");roundtrip(legacy4);
  auto legacy5=saves::read("art/gameplay/guardian/fixtures/legacy-v5.efs");
  check(legacy5.game.knockedBack()&&legacy5.game.heroKnockdownRemaining<=HeroKnockdownDuration&&legacy5.game.fountainUsed==std::array<bool,FountainCount>{},"v5 fall migrates into faster recovery and unused fountains");roundtrip(legacy5);
  auto legacy6=saves::read("art/gameplay/guardian/fixtures/legacy-v6.efs");check(legacy6.game.fountainUsed[0]&&legacy6.game.fountainUsed[1]&&legacy6.game.merchantArrows==20,"real v6 save preserves exhausted fountains and adds finite shop stock");roundtrip(legacy6);
  auto throwing=s;throwing.game.props.clear();throwing.game.player=ForestGuardian+V{4,0};auto& guardian=throwing.game.enemies.back();guardian.boulderCd=0;
  for(auto& e:throwing.game.enemies)if(e.type!=3)e.alive=false;
  throwing.game.update(.05f,{},false);check(guardian.boulderWindup>0,"save fixture contains an actual pending boss throw");roundtrip(throwing);
  while(throwing.game.boulders.empty())throwing.game.update(.05f,{},false);
  throwing.game.update(.05f,{},false);roundtrip(throwing);
  auto continued=saves::decode(saves::encode(throwing));
  auto shattering=throwing;while(shattering.game.boulderImpacts.empty())shattering.game.update(.05f,{},false);
  check(shattering.game.knockedBack(),"actual throw creates a live impulse before saving");roundtrip(shattering);
  auto recoilRestored=saves::decode(saves::encode(shattering));auto fractureSnapshot=shattering;
  for(float elapsed:{.7f,1.4f}){
   auto fallen=shattering;for(int i=0;i<int(elapsed/.05f+.1f);i++)fallen.game.update(.05f,{},false);
   check(fallen.game.knockedBack()&&fallen.game.heroKnockbackRemaining==0,"save fixture is lying or standing up after the slide");roundtrip(fallen);
   auto restored=saves::decode(saves::encode(fallen));for(int i=0;i<30;i++){fallen.game.update(.05f,{},false);restored.game.update(.05f,{},false);}
   check(saves::encode(fallen)==saves::encode(restored),"mid-lying and mid-get-up saves continue identically through restored control");
  }
  for(int i=0;i<172;i++){shattering.game.update(1.f/60,{},false);recoilRestored.game.update(1.f/60,{},false);}
  check(saves::encode(shattering)==saves::encode(recoilRestored)&&!shattering.game.knockedBack(),"v6 resumes the backfall and quicker get-up, health and RNG");
  shattering=fractureSnapshot;for(int i=0;i<4;i++)shattering.game.update(.05f,{},false);
  check(shattering.game.boulderImpacts.size()==1&&shattering.game.boulderImpacts[0].progress()>0,"save fixture contains an actual mid-fracture animation");roundtrip(shattering);
  auto fragmentsRestored=saves::decode(saves::encode(shattering));
  for(int i=0;i<30;i++){shattering.game.update(.05f,{},false);fragmentsRestored.game.update(.05f,{},false);}
  check(saves::encode(shattering)==saves::encode(fragmentsRestored),"current save resumes identical fragment phase, expiry, health and RNG");
  for(int i=0;i<30;i++){throwing.game.update(.05f,{},false);continued.game.update(.05f,{},false);}
  check(saves::encode(throwing)==saves::encode(continued),"midair save resumes the identical trajectory, damage, knockback and cooldown");
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
  auto envelope=[](saves::Snapshot snapshot,int version){saves::Archive payload(false,{},std::min(version,saves::CurrentVersion));payload(snapshot);saves::Archive file(false);std::string magic="EMBERFALL-SAVE";size_t size=payload.bytes.size();uint64_t hash=saves::checksum(payload.bytes);file(magic,version,size,hash);return file.bytes+payload.bytes;};
  auto oldDesert=s;oldDesert.game=Game{};oldDesert.game.activateField(WorldLocation::Desert);oldDesert.game.player=DesertArrival;
  oldDesert.game.props.clear();oldDesert.game.props.push_back({DesertFountains[0],7,1});oldDesert.game.props.push_back({{35,35},8,1});
  auto migrated=saves::decode(envelope(oldDesert,5));
  check(migrated.game.fountainUsed==std::array<bool,FountainCount>{}&&migrated.game.props.size()==1&&distance(migrated.game.props[0].pos,{35,35})==0,"legacy desert save gains unused fountains and removes only overlapping scenery");roundtrip(migrated);
  auto fountains=migrated;fountains.game.player=DesertFountains[0]+V{1.6f,0};fountains.game.mana=7;check(fountains.game.useFountain(0),"save fixture uses actual mana fountain");roundtrip(fountains);
  fountains.game.update(.05f,{},false);roundtrip(fountains);
  auto badFountain=fountains;badFountain.game.fountainUsed[0]=false;rejects([&]{saves::encode(badFountain);},"draining unused fountain rejected");
  badFountain=fountains;badFountain.game.fountainDrain[0]=1.1f;rejects([&]{saves::encode(badFountain);},"oversized drain clock rejected");
  badFountain=fountains;badFountain.game.fountainDrain[0]=-.01f;rejects([&]{saves::encode(badFountain);},"negative drain clock rejected");
  auto oldProgress=s;oldProgress.game=Game{};oldProgress.game.level=5;oldProgress.game.xp=200;oldProgress.game.attributes[0]=12;
  auto oldProgressLoaded=saves::decode(envelope(oldProgress,6));
  check(oldProgressLoaded.game.level==5&&oldProgressLoaded.game.xp==experienceForLevel(5)/2&&oldProgressLoaded.game.attributes[0]==12,"legacy saves retain earned levels, allocated stats and proportional XP progress");roundtrip(oldProgressLoaded);
  auto scarce=s;scarce.game.merchantArrows=7;roundtrip(scarce);scarce.game.merchantArrows=0;scarce.game.desertBossDead=true;roundtrip(scarce);
  auto badStock=s;badStock.game.merchantArrows=-1;rejects([&]{saves::encode(badStock);},"negative shop stock rejected");badStock.game.merchantArrows=21;rejects([&]{saves::encode(badStock);},"oversized shop stock rejected");
  rejects([&]{saves::decode(envelope(s,saves::CurrentVersion+1));},"unknown future version refused");
  invalid=s;invalid.game.heroKnockback={100,0};rejects([&]{saves::encode(invalid);},"oversized knockback vector rejected");
  invalid=s;invalid.game.heroKnockbackRemaining=BoulderKnockbackDuration+.1f;rejects([&]{saves::encode(invalid);},"oversized knockback timer rejected");
  invalid=s;invalid.game.heroKnockdownRemaining=HeroKnockdownDuration+.1f;rejects([&]{saves::encode(invalid);},"oversized fall/get-up timer rejected");
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
