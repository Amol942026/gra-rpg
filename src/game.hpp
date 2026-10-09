#pragma once
#include "animation.hpp"
#include "sound_events.hpp"
#include "entity_info.hpp"
#include "loot.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <string>
#include <vector>

namespace ember {
constexpr int MapSize=48, BagSize=25, EquipSize=6;
constexpr int MushroomCount=36,MushroomHealing=10;
constexpr float MapWalkMin=1.6f,MapWalkMax=MapSize-2.6f;

constexpr float WalkSpeed=3.8f, BootsSpeed=4.3f;
constexpr float RunMultiplier=1.5f;
constexpr float EnemySightRange=6.5f,PatrolRadius=2.5f,PatrolSpeedFactor=.6f;
constexpr float MeleeRange=2.1f, SwingDuration=.24f;
constexpr float RollDuration=.4f, RollSpeed=7.f, RollCooldown=1.2f, RollProtection=.28f;
constexpr float FireballCost=10,FireballCooldown=.8f,FireballSpeed=8,FireballRange=8,ManaRegenInterval=5;
enum class Attribute { Strength,MagicPower,Defense,MagicDefense,Vitality,Mana,Count };
constexpr int AttributeCount=int(Attribute::Count),PointsPerLevel=5;
using Attributes=std::array<int,AttributeCount>;
enum class Spell { None,Fireball };
struct V { float x=0,y=0; V operator+(V b)const{return{x+b.x,y+b.y};} V operator-(V b)const{return{x-b.x,y-b.y};} V operator*(float s)const{return{x*s,y*s};} };
inline float length(V v){return std::sqrt(v.x*v.x+v.y*v.y);}
inline float distance(V a,V b){return length(a-b);}
inline V normalized(V v){float n=length(v);return n>.001f?v*(1/n):V{};}
struct Ground { int id; Item value; V pos; };
struct Prop { V pos; int type; float size; };
enum class EnemyBehavior { Idle,Patrol,Chase,Return };
struct Enemy {
 int id,type; V pos,home; float hp,maxHp,attack=0,flash=0,windup=0; bool alive=true; AnimationState animation{}; float strike=0;
 V recoil{};
 std::array<Item,CorpseLootSize> loot{};
 EnemyBehavior behavior=EnemyBehavior::Idle;
 V patrolGoal{};float patrolWait=0,patrolBlocked=0;unsigned patrolSequence=0;bool patrolGoalActive=false;
 // Map units sized to the complete body footprint (including a wolf's muzzle
 // and tail), so separated centers also keep the rendered bodies apart.
 float radius()const{return enemyInfo(type).radius;}
 int damage()const{return enemyInfo(type).attack;}
 int defense()const{return enemyInfo(type).defense;}
 int magicDefense()const{return enemyInfo(type).magicDefense;}
 DamageType attackType()const{return enemyInfo(type).attackType;}
};
struct Effect { V pos; std::string text; float life; int color; };
struct Fireball { V pos,velocity;float remaining=FireballRange,damage=0; };
struct Explosion { V pos;float life=.35f; };
inline float segmentCircle(V start,V delta,V center,float radius){
 V offset=start-center;float c=offset.x*offset.x+offset.y*offset.y-radius*radius;
 if(c<=0)return 0;
 float a=delta.x*delta.x+delta.y*delta.y;if(a<.0000001f)return 2;
 float b=offset.x*delta.x+offset.y*delta.y,discriminant=b*b-a*c;
 if(discriminant<0)return 2;
 float t=(-b-std::sqrt(discriminant))/a;return t>=0&&t<=1?t:2;
}
enum class Place { None,Bag,Equip,Ground,Corpse,Forage };
struct Slot { Place place=Place::None; int index=-1; int owner=-1; };
struct Game {
 std::mt19937 rng{12593};
 V player{8,19},facing{0,-1};
 AnimationState animation{};
 std::array<Item,BagSize> bag{};std::array<Item,EquipSize> equipment{};
 std::vector<Ground> ground,forage;std::vector<Enemy> enemies;std::vector<Prop> props;std::vector<Effect> effects;
 std::vector<Fireball> fireballs;std::vector<Explosion> explosions;
 Attributes attributes{};int attributePoints=0;bool allocationRequired=false;
 Spell selectedSpell=Spell::Fireball;float manaRegenTime=0;
 std::vector<SoundEvent> sounds;float stepDistance=0;
 void sound(Cue cue,V position){if(sounds.size()<64)sounds.push_back({cue,position.x,position.y});}
 float hp=100,mana=100,attackCd=0,spellCd=0,potionCd=0,swing=0,hurt=0,time=0;
 float rollRemaining=0,rollCd=0;V rollDirection{};
 int level=1,xp=0,kills=0,nextGround=1;bool bossDead=false;
 int openedCorpse=-1;
 std::string message="Ruiny czekają. PPM na ciele potwora otwiera jego łup.";float messageTime=8;
 explicit Game(unsigned seed=12593):rng(seed){
  equipment[0]=item(Kind::Sword);equipment[3]=item(Kind::Armor);
  bag[0]=item(Kind::Health,0,5);bag[1]=item(Kind::Mana,0,3);bag[2]=item(Kind::Shield);bag[3]=item(Kind::Boots);
  const std::array<V,14> loc={V{10,14},V{13,16},V{6,12},V{16,17},V{17,12},V{12,8},V{19,20},V{7,6},V{22,14},V{22,9},V{16,6},V{24,20},V{4,22},V{21,5}};
  auto spawn=[&](int id,int type,V pos){float h=enemyInfo(type).health;enemies.push_back({id,type,pos,pos,h,h});enemies.back().patrolWait=.3f+(hash(id,17)%1000)*.0015f;};
  for(int i=0;i<13;i++)spawn(i,i%3,loc[i]);
  struct Spawn {int type;V pos;};
  const Spawn extended[]={
   {1,{30,18}},{1,{34,22}},{1,{37,18}},{1,{40,23}},
   {4,{9,28}},{4,{13,32}},{4,{7,37}},{4,{17,28}},{4,{16,39}},
   {5,{24,31}},{5,{26,37}},{5,{20,40}},{5,{25,43}},{5,{30,42}},{5,{19,35}},
   {6,{34,32}},{6,{38,34}},{6,{42,31}},{6,{39,38}},{6,{44,36}},
   {7,{35,42}},{7,{40,43}},{7,{43,40}},
   {0,{4,30}},{0,{11,42}},{0,{32,26}},
   {2,{28,28}},{2,{42,8}},{2,{43,13}},
   {1,{31,12}},{1,{37,9}},{1,{38,27}}
  };
  for(int i=0;i<int(std::size(extended));i++)spawn(14+i,extended[i].type,extended[i].pos);
  spawn(13,3,loc[13]);
  for(int x=2;x<MapSize-2;x++)for(int y=2;y<MapSize-2;y++){
   unsigned h=hash(x,y);int density=x<28&&y<28?17:9;
   if(h%density!=0||onPath(x,y))continue;V p{(float)x,(float)y};
   if(distance(p,player)<3.5f)continue;bool near=false;for(auto&e:enemies)if(distance(p,e.home)<1.7f)near=true;
   const int varieties[]={0,4,5,0,4,5,1,2,6};
   int type=varieties[(h>>8)%9];
   if(!near&&type!=6)props.push_back({p,type,.8f+(h%6)*.09f});
  }
  props.push_back({{19,4},3,1});props.push_back({{23,4},3,1});props.push_back({{19,7},3,1});props.push_back({{23,7},3,1});
  plantMushrooms(seed);
 }
 void plantMushrooms(unsigned seed){
  // A separate generator keeps gathering independent from monster drop rolls.
  std::mt19937 plants(seed^0x6d757368u);std::uniform_real_distribution<float> coordinate(3.f,MapSize-4.f);
  for(int attempt=0;attempt<10000&&int(forage.size())<MushroomCount;attempt++){
   V pos{coordinate(plants),coordinate(plants)};
   if(!walkable(pos)||onPath(int(std::round(pos.x)),int(std::round(pos.y)))||distance(pos,player)<3.5f)continue;
   bool clear=true;for(const auto& p:props)if(distance(pos,p.pos)<std::max(2.2f,propRadius(p.type)+.8f))clear=false;
   for(const auto& e:enemies)if(distance(pos,e.home)<1.4f)clear=false;
   for(const auto& mushroom:forage)if(distance(pos,mushroom.pos)<2.2f)clear=false;
   if(clear)forage.push_back({int(forage.size())+1,item(Kind::Mushroom),pos});
  }
 }
 static unsigned hash(int x,int y){unsigned n=unsigned(x)*374761393u+unsigned(y)*668265263u+8713u;n=(n^(n>>13))*1274126177u;return n^(n>>16);}
 static bool onPath(int x,int y){return std::abs(x+y-27)<2||(x>=19&&x<=23&&y>=3&&y<=8)||(x>=7&&x<=10&&y>=17&&y<=20)
  ||(x>=8&&std::abs(y-19)<2)||(y>=15&&std::abs(x-33)<2)||(x>=18&&y>=20&&std::abs(x+y-65)<2);}
 static const char* region(V pos){return pos.x>=30&&pos.y>=30?"Serce Kniei":pos.x<20&&pos.y>=26?"Brzozowa polana":pos.y>=30?"Pajęcze ostępy":pos.x>=28?"Wilcza knieja":"Ruiny Mchu";}
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
  attributePoints=0;allocationRequired=false;say("Punkty rozdane. Wyprawa trwa dalej.");return true;
 }
 bool chooseSpell(Spell choice){if(choice!=Spell::Fireball&&choice!=Spell::None)return false;selectedSpell=choice;return true;}
 bool dead()const{return hp<=0;}
 bool rolling()const{return rollRemaining>0;}
 float swingProgress()const{return swing>0?std::clamp(1-swing/SwingDuration,0.f,1.f):-1.f;}
 float rollProgress()const{return rolling()?1-rollRemaining/RollDuration:-1.f;}
 bool startRoll(V input){
  if(dead()||allocationRequired||rolling()||rollCd>0)return false;
  const V directions[]={{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1}};
  rollDirection=normalized(length(input)>.001f?input:directions[int(animation.direction)]);
  facing=rollDirection;animation.face(facing.x,facing.y);animation.moving=false;
  rollRemaining=RollDuration;rollCd=RollCooldown;swing=0;sound(Cue::Roll,player);return true;
 }
 void say(std::string s){message=s;messageTime=4;}
 void addGround(Item i,V p){ground.push_back({nextGround++,i,p});}
 Enemy* findCorpse(int id){for(auto& e:enemies)if(e.id==id&&!e.alive)return &e;return nullptr;}
 const Enemy* findCorpse(int id)const{for(const auto& e:enemies)if(e.id==id&&!e.alive)return &e;return nullptr;}
 bool corpseReachable(int id)const{const Enemy* e=findCorpse(id);return e&&distance(player,e->pos)<=2.5f;}
 bool openCorpse(int id){
  if(dead()||allocationRequired||!findCorpse(id))return false;
  if(!corpseReachable(id)){say("Podejdź do ciała (maks. 2,5 pola).");return false;}
  openedCorpse=id;sound(Cue::Ui,player);return true;
 }
 void closeCorpse(){openedCorpse=-1;}
 Item* get(Slot s){if(s.place==Place::Bag&&s.index>=0&&s.index<BagSize)return &bag[s.index];if(s.place==Place::Equip&&s.index>=0&&s.index<EquipSize)return &equipment[s.index];if(s.place==Place::Forage)for(auto&g:forage)if(g.id==s.index)return &g.value;if(s.place==Place::Ground)for(auto&g:ground)if(g.id==s.index)return &g.value;if(s.place==Place::Corpse&&s.index>=0&&s.index<CorpseLootSize)if(Enemy* e=findCorpse(s.owner))return &e->loot[s.index];return nullptr;}
 bool reachable(Slot s)const{if(s.place==Place::Corpse)return corpseReachable(s.owner);if(s.place==Place::Forage){for(const auto&g:forage)if(g.id==s.index)return distance(player,g.pos)<=2.5f;return false;}if(s.place!=Place::Ground)return true;for(const auto&g:ground)if(g.id==s.index)return distance(player,g.pos)<=2.5f;return false;}
 static bool accepts(Slot s,Item i){return s.place!=Place::Equip||i.empty()||gearSlot(i.kind)==s.index;}
 bool transfer(Slot from,Slot to){
  if(dead()||allocationRequired||(from.place==to.place&&from.index==to.index&&from.owner==to.owner)||to.place==Place::Corpse||to.place==Place::Forage)return false;
  Item* a=get(from);Item* b=get(to);if(!a||!b||a->empty())return false;
  if(from.place==Place::Corpse||from.place==Place::Forage){
   if(from.place==Place::Corpse&&openedCorpse!=from.owner)return false;
   if(to.place!=Place::Bag||(!b->empty()&&(!stackable(a->kind)||a->kind!=b->kind||a->tier!=b->tier))){say("Przeciągnij łup na wolne pole plecaka lub pasujący stos.");return false;}
  }
  if(!reachable(from)||!reachable(to)){say("Podejdź bliżej przedmiotu (maks. 2,5 pola).");return false;}
  if(!accepts(to,*a)||!accepts(from,*b)){say("Ten przedmiot nie pasuje do wybranego miejsca.");return false;}
  if(stackable(a->kind)&&a->kind==b->kind&&a->tier==b->tier){int n=std::min(a->count,99-b->count);if(n<=0)return false;b->count+=n;a->count-=n;if(a->count==0)*a={};}
  else std::swap(*a,*b);
  sound(to.place==Place::Equip||from.place==Place::Equip?Cue::Equip:to.place==Place::Ground?Cue::Drop:Cue::Pickup,player);
  say(to.place==Place::Equip?"Ekwipunek założony. Statystyki zaktualizowane.":"Przedmiot przeniesiony.");return true;
 }
 bool drop(Slot from,V p){Item* a=get(from);if((from.place==Place::Corpse||from.place==Place::Forage)||dead()||allocationRequired||!a||a->empty()||!reachable(from)||distance(player,p)>2.5f||!walkable(p))return false;Item copy=*a;*a={};addGround(copy,p);sound(Cue::Drop,p);say("Przedmiot odłożony na ziemię.");return true;}
 bool use(Slot s){if(s.place==Place::Forage){say("Najpierw zbierz grzyb do plecaka.");return false;}Item* a=get(s);if(s.place==Place::Corpse||dead()||allocationRequired||potionCd>0||!a||a->empty()||!reachable(s))return false;
  bool mushroom=a->kind==Kind::Mushroom;
  if(mushroom){if(hp>=maxHp()){say("Zdrowie jest już pełne.");return false;}float restored=std::min(float(MushroomHealing),float(maxHp())-hp);hp+=restored;effects.push_back({player,"+"+std::to_string(int(restored))+" HP",1.3f,1});}
  else if(a->kind==Kind::Health){if(hp>=maxHp()){say("Zdrowie jest już pełne.");return false;}hp=std::min(float(maxHp()),hp+50);effects.push_back({player,"+50 HP",1.3f,1});}
  else if(a->kind==Kind::Mana){if(mana>=maxMana()){say("Mana jest już pełna.");return false;}mana=std::min(float(maxMana()),mana+35);effects.push_back({player,"+35 MP",1.3f,2});}else return false;
  if(--a->count<=0)*a={};potionCd=.65f;sound(Cue::Potion,player);say(mushroom?"Zjedzono borowika. Odzyskano zdrowie.":"Wypito miksturę.");return true;
 }
 void potion(Kind k){for(int i=0;i<BagSize;i++)if(bag[i].kind==k&&!bag[i].empty()){use({Place::Bag,i});return;}say("Brak mikstury w plecaku.");}
 bool walkable(V p)const{if(p.x<1.6f||p.y<1.6f||p.x>MapWalkMax||p.y>MapWalkMax)return false;for(auto&q:props)if(distance(p,q.pos)<propRadius(q.type))return false;return true;}
 void move(V delta){V p=player+delta;if(walkable(p))player=p;else{p={player.x+delta.x,player.y};if(walkable(p))player=p;p={player.x,player.y+delta.y};if(walkable(p))player=p;}if(length(delta)>.001f)facing=normalized(delta);}
 Enemy* findEnemy(int id){for(auto&e:enemies)if(e.id==id&&e.alive)return &e;return nullptr;}
 void hit(Enemy& e,float amount,DamageType type=DamageType::Physical){if(!e.alive)return;amount=damageAfterArmor(amount,type==DamageType::Magic?e.magicDefense():e.defense());e.hp-=amount;e.flash=.18f;effects.push_back({e.pos,std::to_string(int(amount)),.85f,0});
  const Cue materials[]={Cue::SlimeHit,Cue::WolfHit,Cue::BoneHit,Cue::StoneHit,Cue::WolfHit,Cue::SlimeHit,Cue::WolfHit,Cue::StoneHit};sound(materials[std::clamp(e.type,0,EnemyTypeCount-1)],e.pos);
  if(e.hp>0)return;e.alive=false;e.hp=0;e.animation.moving=false;e.windup=e.strike=e.flash=0;e.recoil={};e.patrolGoalActive=false;e.behavior=EnemyBehavior::Idle;sound(e.type==3?Cue::Victory:Cue::EnemyFall,e.pos);kills++;xp+=enemyInfo(e.type).experience;
  e.loot=rollEnemyLoot(e.type,rng);
  if(e.type==3){bossDead=true;say("Strażnik pokonany! PPM na ciele otwiera legendarny łup.");}
  else say("Przeciwnik pokonany. Otwórz jego ciało prawym przyciskiem myszy.");
  while(xp>=level*80){xp-=level*80;level++;attributePoints+=PointsPerLevel;allocationRequired=true;hp=float(maxHp());mana=float(maxMana());manaRegenTime=0;sound(Cue::LevelUp,player);effects.push_back({player,"NOWY POZIOM",2,3});}
 }
 bool attack(){
  if(dead()||allocationRequired||rolling()||attackCd>0)return false;
  attackCd=.46f;swing=SwingDuration;sound(Cue::Swing,player);
  // Capture once: a kill can level up the player during this same swing.
  const float amount=float(damage());
  for(auto&e:enemies)if(e.alive&&distance(player,e.pos)<=MeleeRange+.00001f)hit(e,amount);
  return true;
 }
 bool spell(V aim={}){
  if(dead()||allocationRequired||rolling()||spellCd>0)return false;
  if(selectedSpell==Spell::None){say("Wybierz czar w księdze (K lub kafelek czaru).");sound(Cue::Reject,player);return false;}
  if(mana<FireballCost){say("Kula ognia wymaga 10 many.");sound(Cue::Reject,player);return false;}
  const V directions[]={{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1}};
  V direction=normalized(length(aim)>.001f?aim:directions[int(animation.direction)]);
  mana-=FireballCost;spellCd=FireballCooldown;facing=direction;animation.face(direction.x,direction.y);
  fireballs.push_back({player,direction*FireballSpeed,FireballRange,float(magicDamage())});
  sound(Cue::Spell,player);return true;
 }
 void explode(V position,float amount){
  explosions.push_back({position});
  for(auto& e:enemies)if(e.alive&&distance(e.pos,position)<=1.05f+e.radius())hit(e,amount,DamageType::Magic);
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
   if(impact){V position=ball.pos;float amount=ball.damage;fireballs.erase(fireballs.begin()+i);explode(position,amount);}
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
  if(allocationRequired)return;
  dt=std::clamp(dt,0.f,.05f);time+=dt;auto tick=[dt](float&v){v=std::max(0.f,v-dt);};tick(attackCd);tick(spellCd);tick(potionCd);tick(swing);tick(hurt);tick(messageTime);tick(rollCd);
  for(auto&e:effects)e.life-=dt;effects.erase(std::remove_if(effects.begin(),effects.end(),[](auto&e){return e.life<=0;}),effects.end());
  for(auto& e:explosions)e.life-=dt;explosions.erase(std::remove_if(explosions.begin(),explosions.end(),[](auto& e){return e.life<=0;}),explosions.end());
  if(dead()){animation.moving=false;rollRemaining=0;closeCorpse();return;}
  const bool wasRolling=rolling();const float rollElapsed=RollDuration-rollRemaining;
  V before=player;float walkTime=dt;
  if(wasRolling){
   float stepTime=std::min(dt,rollRemaining);walkTime-=stepTime;
   // Small collision steps prevent a fast roll from skipping thin obstacles.
   int steps=std::max(1,int(std::ceil(stepTime*RollSpeed/.08f)));
   for(int i=0;i<steps;i++)move(rollDirection*(stepTime*RollSpeed/steps));
   rollRemaining=std::max(0.f,rollRemaining-stepTime);if(rollRemaining<.00001f)rollRemaining=0;
  }
  const float speed=(equipment[4].empty()?WalkSpeed:BootsSpeed)*(running?RunMultiplier:1.f);
  move(normalized(input)*(walkTime*speed));
  if(openedCorpse>=0&&!corpseReachable(openedCorpse))closeCorpse();
  if(wasRolling){animation.advance(0,0,dt);animation.face(rollDirection.x,rollDirection.y);}
  else animation.advance(player.x-before.x,player.y-before.y,dt);
  if(!wasRolling){stepDistance+=distance(player,before);if(stepDistance>=.85f){stepDistance=std::fmod(stepDistance,.85f);sound(Cue::Step,player);}}
  if(mana<maxMana()){
   manaRegenTime+=dt;
   if(manaRegenTime+.00001f>=ManaRegenInterval){mana=std::min(float(maxMana()),mana+1);manaRegenTime=std::max(0.f,manaRegenTime-ManaRegenInterval);}
  }else manaRegenTime=0;
  if(attackPressed&&!wasRolling)attack();
  if(allocationRequired)return;
  updateFireballs(dt);if(allocationRequired)return;
  std::vector<V> oldPositions,desired(enemies.size());oldPositions.reserve(enemies.size());
  for(size_t i=0;i<enemies.size();i++){auto& e=enemies[i];oldPositions.push_back(e.pos);tick(e.flash);tick(e.attack);tick(e.strike);if(!e.alive){e.animation.moving=false;continue;}float d=distance(player,e.pos);
   const bool visible=canSeePlayer(e);
   if(e.windup>0){float impactTime=e.windup;e.windup-=dt;if(e.windup<=0){e.strike=.22f;if(d<1.6f&&visible){
    if(wasRolling&&rollElapsed+impactTime<RollProtection){sound(Cue::Dodge,player);effects.push_back({player,"UNIK",.65f,3});}
    else if(!dead()){float dmg=incomingDamage(float(e.damage()),e.attackType());hp=std::max(0.f,hp-dmg);hurt=.2f;sound(dead()?Cue::Death:Cue::Hurt,player);effects.push_back({player,"-"+std::to_string(int(dmg)),.8f,4});}
   }}continue;}
   if(visible){
    e.behavior=EnemyBehavior::Chase;e.patrolGoalActive=false;e.patrolBlocked=0;e.patrolWait=.7f;
    if(d<1.25f&&e.attack<=0){e.animation.face(player.x-e.pos.x,player.y-e.pos.y);e.attack=enemyInfo(e.type).cooldown;e.windup=.42f;sound(Cue::EnemyAttack,e.pos);}
    else if(d>1.05f)desired[i]=normalized(player-e.pos)*enemySpeed(e.type);
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


