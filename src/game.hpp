#pragma once
#include "animation.hpp"
#include "sound_events.hpp"
#include "entity_info.hpp"
#include "loot.hpp"
#include "commerce.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <string>
#include <vector>

namespace ember {
constexpr int MapSize=72, BagSize=25, EquipSize=6;
constexpr int MushroomCount=64,MushroomHealing=10;
constexpr int WeaponWheelSize=8;
constexpr float BowDrawDuration=1.4f,BowMinRange=2,BowMaxRange=12,BowCooldown=.38f;
constexpr float MapWalkMin=1.6f,MapWalkMax=MapSize-2.6f;

constexpr float WalkSpeed=3.8f, BootsSpeed=4.3f;
constexpr float RunMultiplier=1.5f;
constexpr float EnemySightRange=6.5f,PatrolRadius=2.5f,PatrolSpeedFactor=.6f;
constexpr float MeleeRange=2.1f, SwingDuration=.32f;
constexpr float RollDuration=.4f, RollSpeed=7.f, RollCooldown=1.2f, RollProtection=.28f;
constexpr float FireballCost=10,FireballCooldown=.8f,FireballSpeed=8,FireballRange=8,ManaRegenInterval=5;
enum class Attribute { Strength,MagicPower,Defense,MagicDefense,Vitality,Mana,Count };
constexpr int AttributeCount=int(Attribute::Count),PointsPerLevel=5;
using Attributes=std::array<int,AttributeCount>;
enum class Spell { None,Fireball };
enum class Skill { StrongCut,PiercingArrow,Healing,Count };
constexpr int SkillCount=int(Skill::Count),SkillPointsPerLevel=1;
using SkillRanks=std::array<int,SkillCount>;
constexpr float StrongCutCost=10,PiercingArrowCost=20,StrongCutDuration=.50f,StrongCutCooldown=1.2f,PiercingArrowCooldown=1.5f;
constexpr int CriticalChancePercent=5;
constexpr float CriticalMultiplier=2.5f,HealingCost=20,HealingAmount=30,HealingCooldown=1.f,HealingDuration=.85f;
inline const char* skillName(Skill skill){switch(skill){case Skill::StrongCut:return "Mocne cięcie";case Skill::PiercingArrow:return "Przeszywająca strzała";case Skill::Healing:return "Leczenie";default:return "";}}
struct V { float x=0,y=0; V operator+(V b)const{return{x+b.x,y+b.y};} V operator-(V b)const{return{x-b.x,y-b.y};} V operator*(float s)const{return{x*s,y*s};} };
inline float length(V v){return std::sqrt(v.x*v.x+v.y*v.y);}
inline float distance(V a,V b){return length(a-b);}
inline V normalized(V v){float n=length(v);return n>.001f?v*(1/n):V{};}
#include "forest_layout.hpp"
#include "tower_layout.hpp"
#include "desert_layout.hpp"
enum class WorldLocation {Forest,MageTower,Desert};
#include "treasure.hpp"
enum class DawnQuest {Seeking,Returning,AtTower};
struct Ground { int id; Item value; V pos; WorldLocation location=WorldLocation::Forest; int floor=0; };
struct Prop { V pos; int type; float size; };
enum class EnemyBehavior { Idle,Patrol,Chase,Return };
struct Enemy {
 int id,type; V pos,home; float hp,maxHp,attack=0,flash=0,windup=0; bool alive=true; AnimationState animation{}; float strike=0;
 V recoil{};
 std::array<Item,CorpseLootSize> loot{};
 EnemyBehavior behavior=EnemyBehavior::Idle;
 V patrolGoal{};float patrolWait=0,patrolBlocked=0;unsigned patrolSequence=0;bool patrolGoalActive=false;
 double sandPrisonCd=SandPrisonInterval;float sandCastPulse=0;int sandPrisonCasts=0;
 float attackPose()const{
  float regular=windup>0?-std::clamp(1-windup/.42f,0.f,1.f):strike>0?std::sin((1-strike/.22f)*3.14159265f):0;
  return sandCastPulse>0?std::sin((1-sandCastPulse/SandCastDuration)*3.14159265f):regular;
 }
 // Map units sized to the complete body footprint (including a wolf's muzzle
 // and tail), so separated centers also keep the rendered bodies apart.
 float radius()const{return enemyInfo(type).radius;}
 int damage()const{return enemyInfo(type).attack;}
 int defense()const{return enemyInfo(type).defense;}
 int magicDefense()const{return enemyInfo(type).magicDefense;}
 DamageType attackType()const{return enemyInfo(type).attackType;}
};
struct FieldState {std::vector<Enemy> enemies;std::vector<Prop> props;std::vector<Ground> forage;};
struct Effect { V pos; std::string text; float life; int color; bool damageNumber=false,critical=false;float height=75; };
struct Fireball { V pos,velocity;float remaining=FireballRange,damage=0;bool critical=false; };
struct WeaponBinding { Kind kind=Kind::None;int tier=0; };
struct Arrow {V pos,velocity;float remaining,damage;bool piercing=false;int maxTargets=1;std::vector<int> hitIds{};bool critical=false;};
struct PierceFlash {V pos;float life=.26f;};
struct Explosion { V pos;float life=.35f; };
inline float segmentCircle(V start,V delta,V center,float radius){
 V offset=start-center;float c=offset.x*offset.x+offset.y*offset.y-radius*radius;
 if(c<=0)return 0;
 float a=delta.x*delta.x+delta.y*delta.y;if(a<.0000001f)return 2;
 float b=offset.x*delta.x+offset.y*delta.y,discriminant=b*b-a*c;
 if(discriminant<0)return 2;
 float t=(-b-std::sqrt(discriminant))/a;return t>=0&&t<=1?t:2;
}
enum class Place { None,Bag,Equip,Ground,Corpse,Forage,Chest };
struct Slot { Place place=Place::None; int index=-1; int owner=-1; };
struct Game {
 std::array<TreasureChest,TreasureCount> chests{};int openedChest=-1;
 TreasureChest* findChest(int id){for(auto& c:chests)if(c.id==id&&c.location==location)return &c;return nullptr;}
 const TreasureChest* findChest(int id)const{for(const auto& c:chests)if(c.id==id&&c.location==location)return &c;return nullptr;}
 bool chestReachable(int id)const{auto* c=findChest(id);return c&&distance(player,c->pos)<=ChestUseRange;}
 bool openChest(int id){
  auto* chest=findChest(id);
  if(!chest||dead()||allocationRequired||storyBlocked()||rolling()||sandTrapped())return false;
  if(!chestReachable(id)){say("Podejdź do skrzyni (maks. 2,5 pola).");return false;}
  cancelBow();openedCorpse=-1;openedChest=id;chest->opened=true;sound(Cue::Ui,player);
  say("Skrzynia otwarta. Przeciągnij wyposażenie do plecaka.");return true;
 }
 bool openNearestChest(){
  int id=-1;float nearest=ChestUseRange;
  for(const auto& c:chests)if(c.location==location){float d=distance(player,c.pos);if(d<=nearest){nearest=d;id=c.id;}}
  return id>=0&&openChest(id);
 }
 std::mt19937 rng{12593},combatRng{12593};
 // Separate attack rolls from loot: one independent 5% roll per successful attack.
 bool rollCritical(){return std::uniform_int_distribution<int>(1,100)(combatRng)<=CriticalChancePercent;}
 V player{8,19},facing{0,-1};
 AnimationState animation{};
 std::array<Item,BagSize> bag{};std::array<Item,EquipSize> equipment{};
 std::vector<Ground> ground,forage;std::vector<Enemy> enemies;std::vector<Prop> props;std::vector<Effect> effects;
 std::array<WeaponBinding,WeaponWheelSize> weaponWheel{};
 std::vector<Arrow> arrows;bool bowDrawing=false;float bowDrawTime=0,bowReleaseTime=0,lastBowCharge=0;V bowAim{0,-1};
 std::vector<Fireball> fireballs;std::vector<Explosion> explosions;
 Attributes attributes{};int attributePoints=0;bool allocationRequired=false;
 SkillRanks skillRanks{};int skillPoints=0;
 float healingCd=0,healingPulse=0;
 float strongCutCd=0,piercingArrowCd=0,meleeDuration=SwingDuration;bool swingStrong=false,piercingArrowArmed=false,lastBowPiercing=false;
 std::vector<PierceFlash> pierceFlashes;
 Spell selectedSpell=Spell::Fireball;float manaRegenTime=0;
 std::vector<SoundEvent> sounds;float stepDistance=0;
 void sound(Cue cue,V position){if(sounds.size()<64)sounds.push_back({cue,position.x,position.y});}
 float hp=100,mana=100,attackCd=0,spellCd=0,potionCd=0,swing=0,hurt=0,time=0;
 float rollRemaining=0,rollCd=0;V rollDirection{};
 float sandPrisonRemaining=0;
 bool sandTrapped()const{return sandPrisonRemaining>0;}
 void clearSandPrison(){sandPrisonRemaining=0;}
 void imprisonInSand(Enemy& caster){
  if(dead()||!inDesert()||!caster.alive||caster.type!=DesertBossType)return;
  sandPrisonRemaining=SandPrisonDuration;cancelBow();rollRemaining=0;swing=0;swingStrong=false;
  animation.moving=false;animation.stopSprint();stepDistance=0;
  caster.sandCastPulse=SandCastDuration;caster.animation.face(player.x-caster.pos.x,player.y-caster.pos.y);
  caster.sandPrisonCasts++;sound(Cue::EnemyAttack,caster.pos);
  say("Piaskowe więzienie! Ruch i ataki zablokowane na 3 sekundy.");
 }
 int level=1,xp=0,kills=0,nextGround=1;bool bossDead=false;
 WorldLocation location=WorldLocation::Forest;DawnQuest dawnQuest=DawnQuest::Seeking;
 int towerFloor=0,towerRuneMask=0;std::int64_t gold=0;
 bool towerReturnPending=false,desertBossDead=false,desertCreated=false;int collectedFragments=0;
 WorldLocation fieldLocation=WorldLocation::Forest;FieldState forestState,desertState;
 bool inTower()const{return location==WorldLocation::MageTower;}
 bool storyBlocked()const{return dawnQuest==DawnQuest::Returning;}
 int dawnFragments()const{return std::max(collectedFragments,dawnQuest==DawnQuest::Seeking?0:1);}
 bool inDesert()const{return location==WorldLocation::Desert;}
 const char* currentRegion()const{return inDesert()?desertRegion(player):forestRegion(player);}
 #include "desert_system.hpp"
 bool mageReachable()const{return inTower()&&towerFloor==0&&!dead()&&!allocationRequired&&!storyBlocked()&&distance(player,MagePosition)<=MageTalkRange;}
 bool takeTowerReturn(){bool pending=towerReturnPending;towerReturnPending=false;return pending;}
 void enterMageTower(){
  clearSandPrison();location=WorldLocation::MageTower;towerFloor=0;player=TowerArrival;facing=normalized(MagePosition-player);animation={};animation.face(facing.x,facing.y);
  cancelBow();closeCorpse();rollRemaining=rollCd=swing=attackCd=spellCd=hurt=stepDistance=0;
  arrows.clear();fireballs.clear();explosions.clear();effects.clear();pierceFlashes.clear();sounds.clear();healingPulse=0;
 }
 void finishTowerReturn(){if(inTower()&&dawnQuest==DawnQuest::Returning){dawnQuest=DawnQuest::AtTower;say("Wieża Eldrina. Podejdź do maga i naciśnij E lub PPM, aby porozmawiać.");}}
 int openedCorpse=-1;
 std::string message="Ruiny czekają. PPM na ciele potwora otwiera jego łup.";float messageTime=8;
 explicit Game(unsigned seed=12593):rng(seed),combatRng(seed){
  chests=makeTreasure(seed);
  equipment[0]=item(Kind::Sword);equipment[3]=item(Kind::Armor);
  bag[0]=item(Kind::Health,0,5);bag[1]=item(Kind::Mana,0,3);bag[2]=item(Kind::Shield);bag[3]=item(Kind::Boots);bag[4]=item(Kind::Bow);bag[5]=item(Kind::Arrow,0,20);
  weaponWheel[0]={Kind::Sword,0};weaponWheel[1]={Kind::Bow,0};
  auto spawn=[&](int id,int type,V pos){float h=enemyInfo(type).health;enemies.push_back({id,type,pos,pos,h,h});enemies.back().patrolWait=.3f+(hash(id,17)%1000)*.0015f;};
  for(size_t i=0;i<ForestSpawns.size();i++)spawn(i<13?int(i):int(i)+1,ForestSpawns[i].type,ForestSpawns[i].pos);
  spawn(13,3,ForestGuardian);
  for(int x=2;x<MapSize-2;x++)for(int y=2;y<MapSize-2;y++){
   unsigned h=hash(x,y);V p{float(x),float(y)};ForestZone zone=forestZone(p);
   int density=zone==ForestZone::Ruins?17:zone==ForestZone::Wolves?14:zone==ForestZone::Heart?10:zone==ForestZone::Trail?16:12;
   if(h%density!=0||onPath(x,y)||forestClearing(p)||nearTreasure(p,WorldLocation::Forest))continue;
   bool near=false;for(auto&e:enemies)if(distance(p,e.home)<2.2f)near=true;
   const int mixed[]={0,4,5,0,4,5,1,2},birches[]={4,4,4,5,4,0,1,2},pines[]={0,0,0,0,5,0,1,2},oaks[]={5,5,0,5,5,4,1,2};
   const int* varieties=zone==ForestZone::Birches||zone==ForestZone::Grove?birches:zone==ForestZone::Wolves?pines:zone==ForestZone::Heart?oaks:mixed;
   if(!near)props.push_back({p,varieties[(h>>8)%8],.8f+(h%6)*.09f});
  }
  props.push_back({{19,4},3,1});props.push_back({{23,4},3,1});props.push_back({{19,7},3,1});props.push_back({{23,7},3,1});
  for(int i=0;i<8;i++){float angle=(i+.5f)*2*3.14159265f/8;props.push_back({V{59,57}+V{std::cos(angle),std::sin(angle)}*4.1f,3,.85f+i%3*.08f});}
  for(V p:{V{42,6},V{46,6},V{37,60},V{41,60}})props.push_back({p,3,.8f});
  plantMushrooms(seed);
 }
 void plantMushrooms(unsigned seed){
  // A separate generator keeps gathering independent from monster drop rolls.
  std::mt19937 plants(seed^0x6d757368u);std::uniform_real_distribution<float> coordinate(3.f,MapSize-4.f);
  for(int attempt=0;attempt<10000&&int(forage.size())<MushroomCount;attempt++){
   V pos{coordinate(plants),coordinate(plants)};
   if(!walkable(pos)||nearTreasure(pos,WorldLocation::Forest,1.5f)||onPath(int(std::round(pos.x)),int(std::round(pos.y)))||distance(pos,player)<3.5f)continue;
   bool clear=true;for(const auto& p:props)if(distance(pos,p.pos)<std::max(2.2f,propRadius(p.type)+.8f))clear=false;
   for(const auto& e:enemies)if(distance(pos,e.home)<1.4f)clear=false;
   for(const auto& mushroom:forage)if(distance(pos,mushroom.pos)<2.2f)clear=false;
   if(clear)forage.push_back({int(forage.size())+1,item(Kind::Mushroom),pos});
  }
 }
 static unsigned hash(int x,int y){unsigned n=unsigned(x)*374761393u+unsigned(y)*668265263u+8713u;n=(n^(n>>13))*1274126177u;return n^(n>>16);}
 static bool onPath(int x,int y){return forestPath({float(x),float(y)});}
 static const char* region(V pos){return forestRegion(pos);}
 int attribute(Attribute a)const{return attributes[int(a)];}
 int maxHp()const{return 100+attribute(Attribute::Vitality)*10;}
 int maxMana()const{return 100+attribute(Attribute::Mana)*5;}
 int damage()const{return 8+attribute(Attribute::Strength)*2+power(equipment[0])+power(equipment[5]);}
 int magicDamage()const{return 28+attribute(Attribute::MagicPower)*3;}
 int defense()const{int a=attribute(Attribute::Defense);for(int i=1;i<5;i++)a+=power(equipment[i]);return a;}
 int magicDefense()const{return attribute(Attribute::MagicDefense);}
 bool distribute(const Attributes& points){
  if(!allocationRequired||attributePoints<=0)return false;
  int total=0;for(int point:points){if(point<0||point>attributePoints)return false;total+=point;}
  if(total!=attributePoints)return false;
  int oldHp=maxHp(),oldMana=maxMana();
  for(int i=0;i<AttributeCount;i++)attributes[i]+=points[i];
  hp=std::min(float(maxHp()),hp+maxHp()-oldHp);mana=std::min(float(maxMana()),mana+maxMana()-oldMana);
  attributePoints=0;allocationRequired=skillPoints>0;say(allocationRequired?"Statystyki rozdane. Wybierz umiejętność.":"Punkty rozdane. Wyprawa trwa dalej.");return true;
 }
 bool chooseSpell(Spell choice){if(choice!=Spell::Fireball&&choice!=Spell::None)return false;selectedSpell=choice;return true;}
 bool dead()const{return hp<=0;}
 bool rolling()const{return rollRemaining>0;}
 bool combatFacingActive()const{return bowDrawing||bowReleaseTime>0||swing>0;}
 void aimMelee(V aim){
  if(length(aim)<.001f)return;
  facing=normalized(aim);
  animation.face(facing.x,facing.y);
 }
 float swingProgress()const{return swing>0?std::clamp(1-swing/meleeDuration,0.f,1.f):-1.f;}
 float rollProgress()const{return rolling()?1-rollRemaining/RollDuration:-1.f;}
 bool startRoll(V input){
  if(sandTrapped()||dead()||allocationRequired||storyBlocked()||rolling()||rollCd>0)return false;
  const V directions[]={{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1}};
  rollDirection=normalized(length(input)>.001f?input:directions[int(animation.direction)]);
  facing=rollDirection;animation.face(facing.x,facing.y);animation.moving=false;animation.stopSprint();
  cancelBow();rollRemaining=RollDuration;rollCd=RollCooldown;swing=0;sound(Cue::Roll,player);return true;
 }
 void say(std::string s){message=s;messageTime=4;}
 void addGround(Item i,V p){ground.push_back({nextGround++,i,p,location,towerFloor});}
 Enemy* findCorpse(int id){for(auto& e:enemies)if(e.id==id&&!e.alive)return &e;return nullptr;}
 const Enemy* findCorpse(int id)const{for(const auto& e:enemies)if(e.id==id&&!e.alive)return &e;return nullptr;}
 bool corpseReachable(int id)const{const Enemy* e=findCorpse(id);return !inTower()&&e&&distance(player,e->pos)<=2.5f;}
 bool openCorpse(int id){
  if(dead()||allocationRequired||storyBlocked()||inTower()||!findCorpse(id))return false;
  if(!corpseReachable(id)){say("Podejdź do ciała (maks. 2,5 pola).");return false;}
  openedChest=-1;openedCorpse=id;sound(Cue::Ui,player);return true;
 }
 void closeCorpse(){openedCorpse=-1;openedChest=-1;}
 Item* get(Slot s){
  if(s.place==Place::Chest){auto* c=findChest(s.owner);return c&&c->opened&&s.index>=0&&s.index<ChestLootSize?&c->loot[s.index]:nullptr;}
  if(s.place==Place::Bag&&s.index>=0&&s.index<BagSize)return &bag[s.index];if(s.place==Place::Equip&&s.index>=0&&s.index<EquipSize)return &equipment[s.index];if(s.place==Place::Forage&&!inTower())for(auto&g:forage)if(g.id==s.index)return &g.value;if(s.place==Place::Ground)for(auto&g:ground)if(g.id==s.index&&groundVisible(g))return &g.value;if(s.place==Place::Corpse&&!inTower()&&s.index>=0&&s.index<CorpseLootSize)if(Enemy* e=findCorpse(s.owner))return &e->loot[s.index];return nullptr;
 }
 bool reachable(Slot s)const{if(s.place==Place::Chest)return openedChest==s.owner&&chestReachable(s.owner);if(s.place==Place::Corpse)return corpseReachable(s.owner);if(s.place==Place::Forage){if(inTower())return false;for(const auto&g:forage)if(g.id==s.index)return distance(player,g.pos)<=2.5f;return false;}if(s.place!=Place::Ground)return true;for(const auto&g:ground)if(g.id==s.index&&groundVisible(g))return distance(player,g.pos)<=2.5f;return false;}
 static bool accepts(Slot s,Item i){if(!i.empty()&&i.kind==Kind::DawnShard&&s.place!=Place::Bag&&s.place!=Place::Corpse)return false;return s.place!=Place::Equip||i.empty()||gearSlot(i.kind)==s.index;}
 bool transfer(Slot from,Slot to){
  if(dead()||allocationRequired||storyBlocked()||(from.place==to.place&&from.index==to.index&&from.owner==to.owner)||to.place==Place::Corpse||to.place==Place::Forage||to.place==Place::Chest)return false;
  Item* a=get(from);Item* b=get(to);if(!a||!b||a->empty())return false;
  const Enemy* sourceBody=from.place==Place::Corpse?findCorpse(from.owner):nullptr;
  const bool collectingShard=a->kind==Kind::DawnShard&&to.place==Place::Bag&&sourceBody&&
   ((location==WorldLocation::Forest&&sourceBody->type==3&&dawnFragments()<1)||(inDesert()&&sourceBody->type==DesertBossType&&dawnFragments()<2));
  const bool equipLoot=(from.place==Place::Corpse||from.place==Place::Chest)&&to.place==Place::Equip;
  if(from.place==Place::Corpse||from.place==Place::Forage||from.place==Place::Chest){
   if(from.place==Place::Corpse&&openedCorpse!=from.owner)return false;
   if(!equipLoot&&(to.place!=Place::Bag||(!b->empty()&&(!stackable(a->kind)||a->kind!=b->kind||a->tier!=b->tier)))){say("Przeciągnij łup do plecaka lub na pasujący slot sprzętu.");return false;}
  }
  if(!reachable(from)||!reachable(to)){say("Podejdź bliżej przedmiotu (maks. 2,5 pola).");return false;}
  if(!accepts(to,*a)||!accepts(from,*b)){say("Ten przedmiot nie pasuje do wybranego miejsca.");return false;}
  if(equipLoot){
   int spare=-1;if(!b->empty()){for(int i=0;i<BagSize;i++)if(bag[i].empty()){spare=i;break;}
    if(spare<0){say("Brak miejsca w plecaku na zdejmowany sprzęt.");return false;}}
   if(spare>=0)bag[spare]=*b;*b=*a;*a={};
  }
  else if(stackable(a->kind)&&a->kind==b->kind&&a->tier==b->tier){int n=std::min(a->count,99-b->count);if(n<=0)return false;b->count+=n;a->count-=n;if(a->count==0)*a={};}
  else std::swap(*a,*b);
  if((to.place==Place::Equip&&to.index==0)||(from.place==Place::Equip&&from.index==0))cancelBow();
  sound(to.place==Place::Equip||from.place==Place::Equip?Cue::Equip:to.place==Place::Ground?Cue::Drop:Cue::Pickup,player);
  if(collectingShard){collectedFragments=inDesert()?2:1;dawnQuest=DawnQuest::Returning;towerReturnPending=true;cancelBow();say(inDesert()?"Druga część Serca Świtu odzyskana. Eldrin przywołuje cię do wieży...":"Pierwsza część Serca Świtu odzyskana. Eldrin przywołuje cię do wieży...");}
  else say(to.place==Place::Equip?"Ekwipunek założony. Statystyki zaktualizowane.":"Przedmiot przeniesiony.");return true;
 }
 bool quickTransfer(Slot source){
  if(dead()||allocationRequired||storyBlocked())return false;
  Item* value=get(source);if(!value||value->empty())return false;
  if(source.place==Place::Bag){int slot=gearSlot(value->kind);return slot>=0&&transfer(source,{Place::Equip,slot});}
  if(source.place!=Place::Corpse&&source.place!=Place::Chest)return false;
  if(source.place==Place::Corpse&&openedCorpse!=source.owner)return false;
  if(!reachable(source)){say("Podejdź bliżej, aby podnieść przedmiot.");return false;}
  bool moved=false;
  if(stackable(value->kind))for(int i=0;i<BagSize;i++){
   value=get(source);if(!value||value->empty())return moved;
   if(!bag[i].empty()&&bag[i].kind==value->kind&&bag[i].tier==value->tier&&bag[i].count<99)moved=transfer(source,{Place::Bag,i})||moved;
  }
  for(int i=0;i<BagSize;i++){
   value=get(source);if(!value||value->empty())return moved;
   if(bag[i].empty())moved=transfer(source,{Place::Bag,i})||moved;
  }
  if(!moved)say("Brak miejsca w plecaku.");return moved;
 }
 bool drop(Slot from,V p){Item* a=get(from);if(a&&a->kind==Kind::DawnShard){say("Serce Świtu jest zbyt cenne, by je porzucić.");return false;}if((from.place==Place::Corpse||from.place==Place::Forage||from.place==Place::Chest)||dead()||allocationRequired||storyBlocked()||!a||a->empty()||!reachable(from)||distance(player,p)>2.5f||!walkable(p))return false;if(from.place==Place::Equip&&from.index==0)cancelBow();Item copy=*a;*a={};addGround(copy,p);sound(Cue::Drop,p);say("Przedmiot odłożony na ziemię.");return true;}
 bool use(Slot s){if(s.place==Place::Forage){say("Najpierw zbierz grzyb do plecaka.");return false;}Item* a=get(s);if(s.place==Place::Corpse||s.place==Place::Chest||dead()||allocationRequired||storyBlocked()||potionCd>0||!a||a->empty()||!reachable(s))return false;
  bool mushroom=a->kind==Kind::Mushroom;
  if(mushroom){if(hp>=maxHp()){say("Zdrowie jest już pełne.");return false;}float restored=std::min(float(MushroomHealing),float(maxHp())-hp);hp+=restored;effects.push_back({player,"+"+std::to_string(int(restored))+" HP",1.3f,1});}
  else if(a->kind==Kind::Health){if(hp>=maxHp()){say("Zdrowie jest już pełne.");return false;}hp=std::min(float(maxHp()),hp+50);effects.push_back({player,"+50 HP",1.3f,1});}
  else if(a->kind==Kind::Mana){if(mana>=maxMana()){say("Mana jest już pełna.");return false;}mana=std::min(float(maxMana()),mana+35);effects.push_back({player,"+35 MP",1.3f,2});}else return false;
  if(--a->count<=0)*a={};potionCd=.65f;sound(Cue::Potion,player);say(mushroom?"Zjedzono borowika. Odzyskano zdrowie.":"Wypito miksturę.");return true;
 }
 #include "tower_system.hpp"
 #include "trade_system.hpp"
 #include "skill_system.hpp"
 #include "weapon_system.hpp"
 void potion(Kind k){for(int i=0;i<BagSize;i++)if(bag[i].kind==k&&!bag[i].empty()){use({Place::Bag,i});return;}say("Brak mikstury w plecaku.");}
 bool walkable(V p)const{if(inTower())return towerWalkable(p,towerFloor);if(inDesert()&&desertWater(p))return false;if(p.x<1.6f||p.y<1.6f||p.x>MapWalkMax||p.y>MapWalkMax)return false;for(const auto& c:chests)if(c.location==location&&distance(p,c.pos)<ChestRadius)return false;for(auto&q:props)if(distance(p,q.pos)<propRadius(q.type))return false;return true;}
 void move(V delta){if(sandTrapped())return;V p=player+delta;if(walkable(p))player=p;else{p={player.x+delta.x,player.y};if(walkable(p))player=p;p={player.x,player.y+delta.y};if(walkable(p))player=p;}if(length(delta)>.001f&&!combatFacingActive())facing=normalized(delta);}
 Enemy* findEnemy(int id){for(auto&e:enemies)if(e.id==id&&e.alive)return &e;return nullptr;}
 void hit(Enemy& e,float amount,DamageType type=DamageType::Physical,bool critical=false){
  if(!e.alive)return;
  // Multiply the armor-reduced normal hit, so a critical is exactly +150%.
  amount=damageAfterArmor(amount,type==DamageType::Magic?e.magicDefense():e.defense())*(critical?CriticalMultiplier:1.f);
  e.hp-=amount;e.flash=.18f;
  float lift=0;for(const auto& effect:effects)if(effect.damageNumber&&distance(effect.pos,e.pos)<.1f)lift=std::max(lift,effect.height-enemyInfo(e.type).visualHeight+24);
  effects.push_back({e.pos,std::to_string(int(std::round(amount))),critical?1.25f:.95f,0,true,critical,enemyInfo(e.type).visualHeight+18+std::min(lift,90.f)});
  const Cue materials[]={Cue::SlimeHit,Cue::WolfHit,Cue::BoneHit,Cue::StoneHit,Cue::WolfHit,Cue::SlimeHit,Cue::WolfHit,Cue::StoneHit};sound(materials[std::clamp(e.type,0,ForestEnemyTypeCount-1)],e.pos);
  if(e.hp>0)return;e.alive=false;e.hp=0;e.animation.moving=false;e.windup=e.strike=e.flash=0;e.recoil={};e.patrolGoalActive=false;e.behavior=EnemyBehavior::Idle;sound(enemyBoss(e.type)?Cue::Victory:Cue::EnemyFall,e.pos);kills++;xp+=enemyInfo(e.type).experience;
  e.loot=rollEnemyLoot(e.type,rng);
  if(e.type==3){bossDead=true;say("Kamienny Strażnik pokonany! PPM na ciele: łup i część Serca Świtu.");}
  else if(e.type==DesertBossType){desertBossDead=true;say("Żywa Wydma rozsypała się! PPM: łup i druga część Serca Świtu.");}
  else say("Przeciwnik pokonany. Otwórz jego ciało prawym przyciskiem myszy.");
  while(xp>=level*80){xp-=level*80;level++;attributePoints+=PointsPerLevel;skillPoints+=SkillPointsPerLevel;allocationRequired=true;cancelBow();hp=float(maxHp());mana=float(maxMana());manaRegenTime=0;sound(Cue::LevelUp,player);effects.push_back({player,"NOWY POZIOM",2,3});}
 }
 bool attack(V aim={}){
  if(sandTrapped()||dead()||allocationRequired||storyBlocked()||inTower()||rolling()||attackCd>0||archetype(equipment[0].kind)==Kind::Bow)return false;
  aimMelee(aim);
  animation.swordStow=0;attackCd=.46f;meleeDuration=SwingDuration;swingStrong=false;swing=SwingDuration;sound(Cue::Swing,player);
  // Capture once: a kill can level up the player during this same swing.
  const float amount=float(damage());const bool critical=rollCritical();
  for(auto&e:enemies)if(e.alive&&distance(player,e.pos)<=MeleeRange+.00001f)hit(e,amount,DamageType::Physical,critical);
  return true;
 }
 bool spell(V aim={}){
  if(sandTrapped()||dead()||allocationRequired||storyBlocked()||inTower()||rolling()||spellCd>0||bowDrawing)return false;
  if(selectedSpell==Spell::None){say("Wybierz czar w księdze (K lub kafelek czaru).");sound(Cue::Reject,player);return false;}
  if(mana<FireballCost){say("Kula ognia wymaga 10 many.");sound(Cue::Reject,player);return false;}
  const V directions[]={{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1}};
  V direction=normalized(length(aim)>.001f?aim:directions[int(animation.direction)]);
  mana-=FireballCost;spellCd=FireballCooldown;facing=direction;animation.face(direction.x,direction.y);
  fireballs.push_back({player,direction*FireballSpeed,FireballRange,float(magicDamage()),rollCritical()});
  sound(Cue::Spell,player);return true;
 }
 void explode(V position,float amount,bool critical=false){
  explosions.push_back({position});
  for(auto& e:enemies)if(e.alive&&distance(e.pos,position)<=1.05f+e.radius())hit(e,amount,DamageType::Magic,critical);
 }
 void updateFireballs(float dt){
  for(size_t i=0;i<fireballs.size();){
   auto& ball=fireballs[i];float travel=std::min(ball.remaining,FireballSpeed*dt);
   V delta=normalized(ball.velocity)*travel;float first=2;
   for(const auto& e:enemies)if(e.alive)first=std::min(first,segmentCircle(ball.pos,delta,e.pos,e.radius()+.18f));
   for(const auto& p:props)first=std::min(first,segmentCircle(ball.pos,delta,p.pos,propRadius(p.type)+.18f));
   V end=ball.pos+delta;
   for(int axis=0;axis<2;axis++){
    float start=axis==0?ball.pos.x:ball.pos.y,finish=axis==0?end.x:end.y;
    if(finish<1||finish>MapSize-2)first=std::min(first,std::clamp(((finish<1?1.f:float(MapSize-2))-start)/(finish-start),0.f,1.f));
   }
   bool impact=first<=1||travel>=ball.remaining-.00001f;
   ball.pos=ball.pos+delta*(first<=1?first:1);ball.remaining-=travel;
   if(impact){V position=ball.pos;float amount=ball.damage;bool critical=ball.critical;fireballs.erase(fireballs.begin()+i);explode(position,amount,critical);}
   else i++;
   if(allocationRequired)return;
  }
 }
 float incomingDamage(float amount,DamageType type)const{return damageAfterArmor(amount,type==DamageType::Magic?magicDefense():defense());}
 #include "enemy_ai.hpp"
 float constrainEnemy(Enemy& e){
  const float radius=e.radius();
  V before=e.pos;
  e.pos.x=std::clamp(e.pos.x,1.6f+radius,MapWalkMax-radius);
  e.pos.y=std::clamp(e.pos.y,1.6f+radius,MapWalkMax-radius);
  float correction=distance(before,e.pos);
  if(inDesert()&&desertWater(e.pos,radius)){V d=e.pos-DesertOasis;float n=length(d);float gap=2.45f+radius+.002f;correction=std::max(correction,gap-n);e.pos=DesertOasis+normalized(n>.0001f?d:V{1,0})*gap;}
  for(const auto& p:props){
   V delta=e.pos-p.pos;float d=length(delta),clearance=radius+propRadius(p.type)+.002f;
   if(d<clearance){correction=std::max(correction,clearance-d);V normal=d>.00001f?delta*(1/d):V{1,0};e.pos=p.pos+normal*clearance;}
  }
  return correction;
 }
 void moveEnemies(float dt,const std::vector<V>& desired){
  // Short steps stop fast enemies passing through one another between frames.
  const int steps=std::max(1,int(std::ceil(dt*120)));
  const float step=dt/steps;
  for(int sub=0;sub<steps;sub++){
   for(size_t i=0;i<enemies.size();i++){
    auto& e=enemies[i];if(!e.alive)continue;
    e.recoil=e.recoil*std::exp(-10.f*step);
    e.pos=e.pos+(desired[i]+e.recoil)*step;
    constrainEnemy(e);
   }
   // Resolve the whole crowd, including stationary/winding-up enemies. Recheck
   // scenery on each pass so another monster cannot push a body into a tree.
   for(int pass=0;pass<32;pass++){
    float overlap=0;
    for(size_t i=0;i<enemies.size();i++)for(size_t j=i+1;j<enemies.size();j++){
     auto& a=enemies[i];auto& b=enemies[j];if(!a.alive||!b.alive)continue;
     V delta=a.pos-b.pos;float d=length(delta),clearance=a.radius()+b.radius()+.002f;
     if(d>=clearance)continue;
     // Stable fallback also separates creatures spawned at identical positions.
     float angle=float(hash(std::min(a.id,b.id),std::max(a.id,b.id))%6283)*.001f;
     V normal=d>.00001f?delta*(1/d):V{std::cos(angle),std::sin(angle)};
     if(d<=.00001f&&a.id>b.id)normal=normal*-1;
     float invA=1/(a.radius()*a.radius()),invB=1/(b.radius()*b.radius());
     float shareA=invA/(invA+invB),shareB=1-shareA,depth=clearance-d;
     overlap=std::max(overlap,depth);
     a.pos=a.pos+normal*(depth*shareA);b.pos=b.pos-normal*(depth*shareB);
     V relative=desired[i]+a.recoil-desired[j]-b.recoil;
     float closing=relative.x*normal.x+relative.y*normal.y;
     if(closing<-.05f){
      const float impulse=-closing*1.18f;
      a.recoil=a.recoil+normal*(impulse*shareA);
      b.recoil=b.recoil-normal*(impulse*shareB);
      for(Enemy* e:{&a,&b}){float speed=length(e->recoil);if(speed>4)e->recoil=e->recoil*(4/speed);}
     }
    }
    for(auto& e:enemies)if(e.alive)overlap=std::max(overlap,constrainEnemy(e));
    if(overlap<.0001f)break;
   }
  }
 }
 // attackPressed is a single press event, never a held button.
 void update(float dt,V input,bool attackPressed,bool running=false){
  if(allocationRequired||storyBlocked())return;
  dt=std::clamp(dt,0.f,.05f);time+=dt;auto tick=[dt](float&v){v=std::max(0.f,v-dt);};tick(attackCd);tick(spellCd);tick(potionCd);tick(swing);tick(hurt);tick(messageTime);tick(rollCd);tick(bowReleaseTime);tick(strongCutCd);tick(piercingArrowCd);tick(healingCd);tick(healingPulse);
  for(auto& flash:pierceFlashes)flash.life-=dt;pierceFlashes.erase(std::remove_if(pierceFlashes.begin(),pierceFlashes.end(),[](const auto& flash){return flash.life<=0;}),pierceFlashes.end());
  for(auto&e:effects)e.life-=dt;effects.erase(std::remove_if(effects.begin(),effects.end(),[](auto&e){return e.life<=0;}),effects.end());
  for(auto& e:explosions)e.life-=dt;explosions.erase(std::remove_if(explosions.begin(),explosions.end(),[](auto& e){return e.life<=0;}),explosions.end());
  if(dead()){clearSandPrison();cancelBow();animation.moving=false;animation.stopSprint();rollRemaining=0;closeCorpse();return;}
  if(bowDrawing){if(archetype(equipment[0].kind)!=Kind::Bow||ammoCount()<=0)cancelBow();else bowDrawTime=std::min(BowDrawDuration,bowDrawTime+dt);}
  const float movementDt=std::max(0.f,dt-std::min(dt,sandPrisonRemaining));
  tick(sandPrisonRemaining);if(sandPrisonRemaining<.00001f)sandPrisonRemaining=0;
  const bool wasRolling=rolling();const float rollElapsed=RollDuration-rollRemaining;
  V before=player;float walkTime=movementDt;
  if(wasRolling){
   float stepTime=std::min(dt,rollRemaining);walkTime-=stepTime;
   // Small collision steps prevent a fast roll from skipping thin obstacles.
   int steps=std::max(1,int(std::ceil(stepTime*RollSpeed/.08f)));
   for(int i=0;i<steps;i++)move(rollDirection*(stepTime*RollSpeed/steps));
   rollRemaining=std::max(0.f,rollRemaining-stepTime);if(rollRemaining<.00001f)rollRemaining=0;
  }
  const float speed=(equipment[4].empty()?WalkSpeed:BootsSpeed)*(running?RunMultiplier:1.f)*(bowDrawing?.55f:1.f);
  move(normalized(input)*(walkTime*speed));
  if(openedCorpse>=0&&!corpseReachable(openedCorpse))closeCorpse();
  if(openedChest>=0&&!chestReachable(openedChest))openedChest=-1;
  if(wasRolling){animation.stopSprint();animation.face(rollDirection.x,rollDirection.y);animation.advance(0,0,dt);animation.swordStow=1;}
  else {
   // Resolve combat aim before smoothing; locomotion still advances the gait.
   const bool aiming=combatFacingActive();
   if(bowDrawing||bowReleaseTime>0)animation.face(bowAim.x,bowAim.y);
   animation.advance(player.x-before.x,player.y-before.y,dt,running&&!bowDrawing,!aiming,swing<=0);
  }
  if(!wasRolling){stepDistance+=distance(player,before);if(stepDistance>=.85f){stepDistance=std::fmod(stepDistance,.85f);sound(Cue::Step,player);}}
  if(mana<maxMana()){
   manaRegenTime+=dt;
   if(manaRegenTime+.00001f>=ManaRegenInterval){mana=std::min(float(maxMana()),mana+1);manaRegenTime=std::max(0.f,manaRegenTime-ManaRegenInterval);}
  }else manaRegenTime=0;
  if(inTower())return; // The forest remains intact but does not simulate in the tower.
  if(attackPressed&&!wasRolling)attack();
  if(allocationRequired)return;
  updateArrows(dt);if(allocationRequired)return;
  updateFireballs(dt);if(allocationRequired)return;
  std::vector<V> oldPositions,desired(enemies.size());oldPositions.reserve(enemies.size());
  for(size_t i=0;i<enemies.size();i++){auto& e=enemies[i];oldPositions.push_back(e.pos);tick(e.flash);tick(e.attack);tick(e.strike);if(!e.alive){e.animation.moving=false;continue;}float d=distance(player,e.pos);
   tick(e.sandCastPulse);
   const bool visible=canSeePlayer(e);
   // Combat time, including ordinary attack windups, advances this independent cooldown.
   if(inDesert()&&e.type==DesertBossType&&visible){
    e.sandPrisonCd-=dt;
    if(e.sandPrisonCd<=.00001f){e.sandPrisonCd+=SandPrisonInterval;imprisonInSand(e);}
   }
   if(e.windup>0){float impactTime=e.windup;e.windup-=dt;if(e.windup<=0){e.strike=.22f;if(d<(e.type==DesertBossType?2.15f:1.6f)&&visible){
    if(wasRolling&&!sandTrapped()&&rollElapsed+impactTime<RollProtection){sound(Cue::Dodge,player);effects.push_back({player,"UNIK",.65f,3});}
    else if(!dead()){float dmg=incomingDamage(float(e.damage()),e.attackType());hp=std::max(0.f,hp-dmg);hurt=.2f;sound(dead()?Cue::Death:Cue::Hurt,player);effects.push_back({player,"-"+std::to_string(int(dmg)),.8f,4});}
   }}continue;}
   if(visible){
    e.behavior=EnemyBehavior::Chase;e.patrolGoalActive=false;e.patrolBlocked=0;e.patrolWait=.7f;
    if(d<(e.type==DesertBossType?1.9f:1.25f)&&e.attack<=0){e.animation.face(player.x-e.pos.x,player.y-e.pos.y);e.attack=enemyInfo(e.type).cooldown;e.windup=.42f;sound(Cue::EnemyAttack,e.pos);}
    else if(e.sandCastPulse<=0&&d>(e.type==DesertBossType?1.7f:1.05f))desired[i]=normalized(player-e.pos)*enemySpeed(e.type);
   }else desired[i]=patrolVelocity(e,dt);
  }
  moveEnemies(dt,desired);
  for(size_t i=0;i<enemies.size();i++){
   auto& e=enemies[i];if(!e.alive)continue;
   V movement=e.pos-oldPositions[i];
   if(dt>0&&e.patrolGoalActive&&(e.behavior==EnemyBehavior::Patrol||e.behavior==EnemyBehavior::Return)){
    e.patrolBlocked=length(movement)<length(desired[i])*dt*.15f?e.patrolBlocked+dt:0;
    if(e.patrolBlocked>.65f){e.patrolGoalActive=false;e.patrolBlocked=0;}
   }
   e.animation.advance(movement.x,movement.y,dt);
   if(e.windup>0)e.animation.face(player.x-e.pos.x,player.y-e.pos.y);
  }
 }
};
}


