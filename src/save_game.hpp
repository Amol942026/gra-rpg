#pragma once
#include "game.hpp"
#include "intro_story.hpp"
#include "tower_story.hpp"
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <locale>
#include <ctime>
#include <type_traits>
#include <set>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>
#endif

namespace ember::saves {
namespace fs=std::filesystem;
constexpr size_t MaxBytes=4*1024*1024,MaxEntries=8192;
constexpr int CurrentVersion=10;
struct Error:std::runtime_error {using std::runtime_error::runtime_error;};
inline void require(bool ok){if(!ok)throw Error("Nieprawidłowy lub niezgodny zapis gry.");}
struct Snapshot {Game game;IntroSequence intro;TowerSequence tower;std::string date;};

// Explicit little-endian fields, never compiler structs, pointers or padding.
// The version and checksum protect against partial/incompatible files. Reads
// finish in a private candidate; the live session is only replaced on success.
struct Archive {
 bool reading;std::string bytes;size_t at=0;int version;
 explicit Archive(bool read,std::string data={},int format=CurrentVersion):reading(read),bytes(std::move(data)),version(format){}
 void raw(void* data,size_t n){if(reading){require(n<=bytes.size()-at);std::memcpy(data,bytes.data()+at,n);at+=n;}else{require(bytes.size()+n<=MaxBytes);bytes.append(static_cast<char*>(data),n);}}
 template<class T>void one(T& v){
  if constexpr(std::is_same_v<T,bool>){unsigned char n=v?1:0;raw(&n,1);require(n<=1);v=n!=0;}
  else if constexpr(std::is_enum_v<T>){int n=int(v);one(n);v=T(n);}
  else if constexpr(std::is_integral_v<T>){
   uint64_t n=uint64_t(v);unsigned char b[8]{};
   if(!reading)for(int i=0;i<8;i++)b[i]=static_cast<unsigned char>(n>>(i*8));raw(b,8);
   if(reading){n=0;for(int i=0;i<8;i++)n|=uint64_t(b[i])<<(i*8);v=T(n);require(uint64_t(v)==n);}
  }else if constexpr(std::is_floating_point_v<T>){
   unsigned char b[sizeof(T)]{};uint64_t n=0;
   if(!reading){std::memcpy(&n,&v,sizeof(T));for(size_t i=0;i<sizeof(T);i++)b[i]=static_cast<unsigned char>(n>>(i*8));}
   raw(b,sizeof(T));if(reading){for(size_t i=0;i<sizeof(T);i++)n|=uint64_t(b[i])<<(i*8);std::memcpy(&v,&n,sizeof(T));}
   require(std::isfinite(v)&&std::abs(v)<1.e10);
  }else visit(*this,v);
 }
 void one(std::string& s){size_t n=s.size();one(n);require(n<=16384);if(reading)s.resize(n);raw(s.data(),n);}
 template<class T,size_t N>void one(std::array<T,N>& v){for(auto& x:v)one(x);}
 template<class T>void one(std::vector<T>& v){size_t n=v.size();one(n);require(n<=MaxEntries);if(reading)v.resize(n);for(auto& x:v)one(x);}
 void one(std::mt19937& rng){std::string state;if(!reading){std::ostringstream o;o.imbue(std::locale::classic());o<<rng;state=o.str();}one(state);if(reading){std::istringstream i(state);i.imbue(std::locale::classic());require(bool(i>>rng));i>>std::ws;require(i.eof());}}
 template<class... T>void operator()(T&... v){(one(v),...);}
};
#define EF_SAVE(Type,...) inline void visit(Archive& a,Type& v){a(__VA_ARGS__);}
EF_SAVE(V,v.x,v.y)
EF_SAVE(Item,v.kind,v.tier,v.count) 
EF_SAVE(AnimationState,v.direction,v.phase,v.idleTime,v.moving,v.sprinting,v.runBlend,v.swordStow,v.moveBlend,v.facingAngle,v.targetAngle,v.motionInitialized,v.facingInitialized)
EF_SAVE(Ground,v.id,v.value,v.pos,v.location,v.floor)
EF_SAVE(Prop,v.pos,v.type,v.size)
EF_SAVE(TreasureChest,v.id,v.pos,v.location,v.opened,v.loot)
inline void visit(Archive& a,Enemy& v){
 a(v.id,v.type,v.pos,v.home,v.hp,v.maxHp,v.attack,v.flash,v.windup,v.alive,v.animation,v.strike,v.recoil,v.loot,v.behavior,v.patrolGoal,v.patrolWait,v.patrolBlocked,v.patrolSequence,v.patrolGoalActive,v.sandPrisonCd,v.sandCastPulse,v.sandPrisonCasts);
 if(a.version>=2)a(v.boulderCd,v.boulderWindup,v.boulderRecovery,v.boulderTarget,v.boulderThrows);
 if(a.version>=9)a(v.eruptionCd,v.eruptionWindup,v.eruptions);
 if(a.version>=8)a(v.boarChargeCd,v.boarWindup,v.boarRush,v.boarRecovery,v.boarAim,v.boarDirection,v.boarHit,v.boarCharges);
 else if(a.reading){v.boarChargeCd=BoarChargeFirstDelay;v.boarWindup=v.boarRush=v.boarRecovery=0;v.boarAim=v.boarDirection={};v.boarHit=false;v.boarCharges=0;}
 if(a.reading&&a.version>=2&&a.version<6){v.boulderCd*=BoulderCooldown/4.5f;v.boulderWindup*=BoulderWindup/2.2f;v.boulderRecovery*=BoulderRecovery/.6f;}
}
EF_SAVE(FieldState,v.enemies,v.props,v.forage)
EF_SAVE(Effect,v.pos,v.text,v.life,v.color,v.damageNumber,v.critical,v.height)
EF_SAVE(Fireball,v.pos,v.velocity,v.remaining,v.damage,v.critical)
EF_SAVE(WeaponBinding,v.kind,v.tier)
EF_SAVE(Arrow,v.pos,v.velocity,v.remaining,v.damage,v.piercing,v.maxTargets,v.hitIds,v.critical)
EF_SAVE(PierceFlash,v.pos,v.life)
EF_SAVE(Explosion,v.pos,v.life)
EF_SAVE(Boulder,v.origin,v.target,v.pos,v.age,v.duration,v.height,v.startHeight,v.damage,v.owner)
EF_SAVE(BoulderImpact,v.pos,v.life)
EF_SAVE(CinderBolt,v.pos,v.velocity,v.remaining,v.damage,v.owner)
EF_SAVE(LavaFissure,v.pos,v.age,v.delay,v.damage,v.owner,v.struck)
EF_SAVE(SoundEvent,v.cue,v.x,v.y)
EF_SAVE(IntroSequence,v.phase,v.line,v.age,v.time,v.revealed,v.arrivalPending)
inline void visit(Archive& a,TowerSequence& v){a(v.phase,v.line,v.age,v.time,v.returning,v.revealed,v.arrivalPending,v.completionPending,v.offerDesert,v.desertArrivalPending,v.fragments);if(a.version>=9)a(v.offerVolcano,v.travelVolcano);}
inline void visit(Archive& a,Game& v){
 if(a.version>=9)a(v.chests);else for(int i=0;i<4;i++)a(v.chests[i]);
 a(v.openedChest,v.rng,v.combatRng,v.player,v.facing,v.animation,v.bag,v.equipment,v.ground,v.forage,v.enemies,v.props,v.effects,v.weaponWheel,v.arrows,v.bowDrawing,v.bowDrawTime,v.bowReleaseTime,v.lastBowCharge,v.bowAim,v.fireballs,v.explosions,v.attributes,v.attributePoints,v.allocationRequired,v.skillRanks,v.skillPoints,v.healingCd,v.healingPulse,v.strongCutCd,v.piercingArrowCd,v.meleeDuration,v.swingStrong,v.piercingArrowArmed,v.lastBowPiercing,v.pierceFlashes,v.selectedSpell,v.manaRegenTime,v.sounds,v.stepDistance,v.hp,v.mana,v.attackCd,v.spellCd,v.potionCd,v.swing,v.hurt,v.time,v.rollRemaining,v.rollCd,v.rollDirection,v.sandPrisonRemaining,v.level,v.xp,v.kills,v.nextGround,v.bossDead,v.location,v.dawnQuest,v.towerFloor,v.towerRuneMask,v.gold,v.towerReturnPending,v.desertBossDead,v.desertCreated,v.collectedFragments,v.fieldLocation,v.forestState,v.desertState,v.openedCorpse,v.message,v.messageTime);
 if(a.version>=2)a(v.boulders,v.boulderImpacts);
 if(a.version>=3)a(v.heroKnockback,v.heroKnockbackRemaining);
 if(a.version>=4)a(v.heroKnockdownRemaining,v.heroKnockdownFacing);
 if(a.version>=6)a(v.fountainUsed,v.fountainDrain);
 if(a.version>=7)a(v.merchantArrows);
 if(a.version>=9)a(v.volcanoCreated,v.volcanoBossDead,v.volcanoState,v.volcanoFountainUsed,v.volcanoFountainDrain,v.cinders,v.fissures);
 if(a.version>=10)a(v.volcanoSceneryVersion);
 if(a.reading&&a.version<7){
  require(v.level>=1&&v.level<=100000&&v.xp>=0&&v.xp<std::int64_t(v.level)*80);
  v.xp=v.xp*experienceForLevel(v.level)/(std::int64_t(v.level)*80); // Keep legacy level and fractional progress.
 }
 if(a.reading&&a.version<6){
  if(a.version==5)v.heroKnockdownRemaining*=HeroKnockdownDuration/2.8f;
  v.clearFountainSites();
 }
 if(a.reading&&a.version<5){
  if(a.version>=3)v.heroKnockbackRemaining*=BoulderKnockbackDuration/.24f;
  if(a.version>=4)v.heroKnockdownRemaining*=HeroKnockdownDuration/1.2f;
 }
}
EF_SAVE(Snapshot,v.game,v.intro,v.tower,v.date)
#undef EF_SAVE

template<class E>inline bool range(E value,int count){return int(value)>=0&&int(value)<count;}
inline void validateItem(const Item& i){require(range(i.kind,int(Kind::Count))&&i.tier>=0&&i.tier<=100&&i.count>=0&&i.count<=1000000);}
inline void validateAnimation(const AnimationState& a){require(range(a.direction,8)&&a.phase>=0&&a.phase<=1&&a.runBlend>=0&&a.runBlend<=1&&a.swordStow>=0&&a.swordStow<=1&&a.moveBlend>=0&&a.moveBlend<=1);}
inline void validatePos(V p){require(p.x>=-10&&p.x<=MapSize+10&&p.y>=-10&&p.y<=MapSize+10);}
inline void validateGround(const std::vector<Ground>& list,bool unique=true){std::set<int> ids;for(auto& g:list){validateItem(g.value);validatePos(g.pos);require(range(g.location,4)&&g.floor>=0&&g.floor<5&&g.id>=0);if(unique)require(ids.insert(g.id).second);}}
inline void validateField(const std::vector<Enemy>& enemies,const std::vector<Prop>& props,const std::vector<Ground>& forage){
 std::set<int> ids;
 for(const auto& e:enemies){require(e.id>=0&&ids.insert(e.id).second&&range(e.behavior,4)&&e.type>=0&&e.type<EnemyTypeCount&&e.hp>=0&&e.maxHp>0&&e.hp<=e.maxHp&&e.sandPrisonCasts>=0);validatePos(e.pos);validatePos(e.home);validateAnimation(e.animation);for(auto& i:e.loot)validateItem(i);}
 for(const auto& e:enemies){require(e.boulderCd>=0&&e.boulderCd<=BoulderCooldown+.001f&&e.boulderWindup>=0&&e.boulderWindup<=BoulderWindup+.001f&&e.boulderRecovery>=0&&e.boulderRecovery<=BoulderRecovery+.001f&&e.boulderThrows>=0);validatePos(e.boulderTarget);}
 for(const auto& e:enemies){
  require(e.boarChargeCd>=0&&e.boarChargeCd<=BoarChargeCooldown+.001f&&e.boarWindup>=0&&e.boarWindup<=BoarChargeWindup+.001f&&e.boarRush>=0&&e.boarRush<=BoarChargeDuration+.001f&&e.boarRecovery>=0&&e.boarRecovery<=BoarChargeRecovery+.001f&&e.boarCharges>=0);
  int phases=(e.boarWindup>0)+(e.boarRush>0)+(e.boarRecovery>0);require(phases<=1&&(!phases||(e.type==BoarType&&e.alive)));
  validatePos(e.boarAim);require(length(e.boarDirection)<=1.001f&&(!e.boarRush||std::abs(length(e.boarDirection)-1)<.001f));
  require(!e.boarHit||(e.type==BoarType&&e.alive&&(e.boarRush>0||e.boarRecovery>0)));
 }
 for(const auto& e:enemies)require(e.eruptionCd>=0&&e.eruptionCd<=6.001f&&e.eruptionWindup>=0&&e.eruptionWindup<=.951f&&e.eruptions>=0&&(!e.eruptionWindup||(e.type==VolcanoBossType&&e.alive)));
 for(const auto& p:props){validatePos(p.pos);require(p.type>=0&&p.type<PropTypeCount&&p.size>0&&p.size<=10);}
 validateGround(forage);
}
inline void validate(const Snapshot& s){
 const auto& g=s.game;require(range(g.location,4)&&range(g.fieldLocation,4)&&g.fieldLocation!=WorldLocation::MageTower&&range(g.dawnQuest,3)&&range(g.selectedSpell,2));
 require(g.towerFloor>=0&&g.towerFloor<5&&g.towerRuneMask>=0&&g.towerRuneMask<16&&g.collectedFragments>=0&&g.collectedFragments<=5);
 require(g.level>=1&&g.level<=100000&&g.xp>=0&&g.xp<experienceForLevel(g.level)&&g.gold>=0&&g.gold<=1000000000000LL&&g.kills>=0&&g.nextGround>=1);
 require(g.merchantArrows>=0&&g.merchantArrows<=MerchantArrowCapacity);
 for(int n:g.attributes)require(n>=0&&n<=1000000);for(int n:g.skillRanks)require(n>=0&&n<=1000000);
 require(g.attributePoints>=0&&g.attributePoints<=1000000&&g.skillPoints>=0&&g.skillPoints<=1000000&&g.hp>=0&&g.hp<=g.maxHp()&&g.mana>=0&&g.mana<=g.maxMana());
 validatePos(g.player);validateAnimation(g.animation);for(auto& i:g.bag)validateItem(i);
 for(int n=0;n<EquipSize;n++){validateItem(g.equipment[n]);require(g.equipment[n].empty()||gearSlot(g.equipment[n].kind)==n);}
 for(auto& b:g.weaponWheel)require(range(b.kind,int(Kind::Count))&&b.tier>=0&&b.tier<=100);
 for(int n=0;n<TreasureCount;n++){const auto& c=g.chests[n];require(c.id==n&&c.location==TreasureLayout[n].location&&c.pos.x==TreasureLayout[n].pos.x&&c.pos.y==TreasureLayout[n].pos.y);for(auto& i:c.loot)validateItem(i);}
 validateGround(g.ground);for(auto& item:g.ground)require(item.id<g.nextGround);
 validateField(g.enemies,g.props,g.forage);validateField(g.forestState.enemies,g.forestState.props,g.forestState.forage);validateField(g.desertState.enemies,g.desertState.props,g.desertState.forage);
 validateField(g.volcanoState.enemies,g.volcanoState.props,g.volcanoState.forage);
 require(!g.inVolcano()||g.volcanoCreated);require(g.cinders.size()<=24&&g.fissures.size()<=12);
 require(g.volcanoSceneryVersion>=0&&g.volcanoSceneryVersion<=VolcanoSceneryRevision);
 for(const auto& b:g.cinders){validatePos(b.pos);require(g.inVolcano()&&std::abs(length(b.velocity)-7.5f)<.01f&&b.remaining>0&&b.remaining<=7&&b.damage>=0&&b.damage<=1000&&std::any_of(g.enemies.begin(),g.enemies.end(),[&](const Enemy& e){return e.id==b.owner&&e.alive&&Game::volcanoRanged(e.type);}));}
 for(const auto& f:g.fissures){validatePos(f.pos);require(g.inVolcano()&&f.age>=0&&f.delay>=.9f&&f.delay<=1.5f&&f.age<=f.delay+.651f&&f.damage>=0&&f.damage<=1000&&f.struck==(f.age>=f.delay)&&std::any_of(g.enemies.begin(),g.enemies.end(),[&](const Enemy& e){return e.id==f.owner&&e.alive&&e.type==VolcanoBossType;}));}
 for(int i=0;i<FountainCount;i++)require(g.volcanoFountainDrain[i]>=0&&g.volcanoFountainDrain[i]<=FountainDrainDuration&&(!g.volcanoFountainDrain[i]||g.volcanoFountainUsed[i]));
 for(auto& e:g.sounds)require(range(e.cue,int(Cue::Count)));for(auto& a:g.arrows)require(a.maxTargets>=1&&a.maxTargets<=1000);
 require(g.boulders.size()<=MaxBoulders&&g.boulderImpacts.size()<=MaxBoulderImpacts);
 for(const auto& b:g.boulders){
  require(g.location==WorldLocation::Forest&&b.age>=0&&b.duration>=BoulderMinFlight&&b.duration<=30&&b.age<=b.duration&&b.height>=0&&b.height<=250&&b.startHeight>=0&&b.startHeight<=200&&b.damage>=0&&b.damage<=1000000);
  validatePos(b.origin);validatePos(b.target);validatePos(b.pos);
  require(std::any_of(g.enemies.begin(),g.enemies.end(),[&](const Enemy& e){return e.id==b.owner&&e.type==3&&e.alive;}));
  require(distance(b.pos,b.origin+(b.target-b.origin)*(b.age/b.duration))<.001f);
 }
 for(const auto& b:g.boulderImpacts){validatePos(b.pos);require(b.life>=0&&b.life<=.401f);}
 for(int i=0;i<FountainCount;i++)require(g.fountainDrain[i]>=0&&g.fountainDrain[i]<=FountainDrainDuration&&(!g.fountainDrain[i]||g.fountainUsed[i]));
 require(g.heroKnockbackRemaining>=0&&g.heroKnockbackRemaining<=BoulderKnockbackDuration+.00001f&&length(g.heroKnockback)<=BoulderKnockbackDistance+.001f);
 require(!g.knockedBack()||g.location==WorldLocation::Forest);
 require(g.heroKnockdownRemaining>=0&&g.heroKnockdownRemaining<=HeroKnockdownDuration+.00001f&&std::abs(g.heroKnockdownFacing)<=7);
 require(range(s.intro.phase,6)&&s.intro.line<IntroDialogue.size()&&s.intro.age>=0&&s.intro.time>=0);
 require(range(s.tower.phase,9)&&s.tower.fragments>=0&&s.tower.fragments<=5&&s.tower.line<s.tower.lineCount()&&s.tower.age>=0&&s.tower.time>=0);
 require(!(s.intro.active()&&s.tower.active())&&s.date.size()<=32);
}
inline uint64_t checksum(const std::string& data){uint64_t h=14695981039346656037ULL;for(unsigned char c:data){h^=c;h*=1099511628211ULL;}return h;}
inline std::string encode(Snapshot snapshot){
 validate(snapshot);Archive payload(false);payload(snapshot);Archive out(false);std::string magic="EMBERFALL-SAVE";int version=CurrentVersion;size_t size=payload.bytes.size();uint64_t hash=checksum(payload.bytes);out(magic,version,size,hash);out.bytes+=payload.bytes;return out.bytes;
}
inline Snapshot decode(const std::string& bytes){
 require(bytes.size()<=MaxBytes);Archive in(true,bytes);std::string magic;int version=0;size_t size=0;uint64_t hash=0;in(magic,version,size,hash);require(magic=="EMBERFALL-SAVE"&&version>=1&&version<=CurrentVersion&&size==bytes.size()-in.at);std::string payload=bytes.substr(in.at);require(checksum(payload)==hash);Archive data(true,std::move(payload),version);Snapshot result;data(result);require(data.at==data.bytes.size());validate(result);result.game.refreshVolcanoScenery();validate(result);return result;
}
inline std::string now(){std::time_t t=std::time(nullptr);std::tm local{};
#ifdef _WIN32
 localtime_s(&local,&t);
#else
 localtime_r(&t,&local);
#endif
 std::ostringstream out;out<<std::put_time(&local,"%Y-%m-%d %H:%M");return out.str();}
inline fs::path defaultRoot(){
#ifdef _WIN32
 // Wide API preserves non-ASCII Windows account names.
 wchar_t path[32768]{};DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",path,32768);if(n>0&&n<32768)return fs::path(path)/L"Emberfall"/L"saves";
#endif
 throw Error("Nie można odnaleźć katalogu zapisów użytkownika.");
}
inline std::string readBytes(const fs::path& path){
 require(fs::is_regular_file(path)&&fs::file_size(path)<=MaxBytes);std::ifstream in(path,std::ios::binary);if(!in)throw Error("Nie można odczytać zapisu.");std::string bytes((std::istreambuf_iterator<char>(in)),{});if(in.bad())throw Error("Błąd odczytu zapisu.");return bytes;
}
inline Snapshot read(const fs::path& path){return decode(readBytes(path));}
inline void atomicWrite(const fs::path& path,const std::string& bytes){
 fs::path temp=path;temp+=L".tmp";
 try{
  {std::ofstream out(temp,std::ios::binary|std::ios::trunc);if(!out)throw Error("Nie można utworzyć pliku zapisu.");out.write(bytes.data(),std::streamsize(bytes.size()));out.flush();if(!out)throw Error("Brak miejsca lub błąd zapisu.");out.close();if(!out)throw Error("Błąd zamykania pliku zapisu.");}
  require(readBytes(temp)==bytes);
#ifdef _WIN32
  if(!MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw Error("Nie można zatwierdzić zapisu. Poprzedni plik pozostał na dysku.");
#else
  fs::rename(temp,path);
#endif
 }catch(...){std::error_code ignored;fs::remove(temp,ignored);throw;}
}
struct Entry {bool exists=false,valid=false,backup=false;std::string date,location,error;int level=0;};
struct Store {
 fs::path root;explicit Store(fs::path folder=defaultRoot()):root(std::move(folder)){}
 fs::path path(int slot)const{require(slot>=0&&slot<3);return root/("slot-"+std::to_string(slot+1)+".efs");}
 Snapshot load(int slot,bool* backup=nullptr)const{
  if(backup)*backup=false;try{return read(path(slot));}catch(...){fs::path b=path(slot);b+=L".bak";if(!fs::exists(b))throw;if(backup)*backup=true;return read(b);}
 }
 Entry inspect(int slot)const{
  Entry entry;try{entry.exists=fs::exists(path(slot))||fs::exists(fs::path(path(slot).wstring()+L".bak"));if(!entry.exists)return entry;auto s=load(slot,&entry.backup);entry.valid=true;entry.date=s.date;entry.level=s.game.level;entry.location=s.intro.active()?"Prolog":s.game.inTower()?"Wieża Eldrina · piętro "+std::to_string(s.game.towerFloor+1):s.game.inVolcano()?"Kaldera Popiołu":s.game.inDesert()?"Morze Piasków":"Las Mchu";}catch(...){entry.exists=true;entry.error="Nie można odczytać zapisu";}return entry;
 }
 void save(int slot,Snapshot snapshot)const{
  snapshot.date=now();std::string bytes=encode(std::move(snapshot));fs::create_directories(root);auto target=path(slot);
  // Only a validated previous save may replace the recovery copy.
  bool oldValid=false;if(fs::exists(target)){try{read(target);oldValid=true;}catch(const Error&){}}
  if(oldValid){auto backup=target;backup+=L".bak";atomicWrite(backup,readBytes(target));}
  atomicWrite(target,bytes);
 }
};
}
