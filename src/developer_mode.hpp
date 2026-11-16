#pragma once
#include "game.hpp"
#include <cctype>
#include <limits>
namespace ember {
struct DeveloperDestination {const char* id;const char* name;WorldLocation location;V spawn;int floor;};
inline constexpr auto DeveloperDestinations=[] {
 std::array<DeveloperDestination,std::size(FieldSites)+TowerFloorCount> result{};
 // Keep the old destination IDs and indices, including command-line shortcuts.
 size_t next=0;
 for(size_t i=0;i<8;i++){
  const auto& site=FieldSites[i];
  const V spawn=i==4?ForestRitualApproach:site.position;
  result[next++]={site.id,site.name,site.realm,spawn,0};
 }
 constexpr const char* towerIds[]={"tower-1","tower-2","tower-3","tower-4","tower-5"};
 for(int floor=0;floor<TowerFloorCount;floor++)result[next++]={towerIds[floor],TowerFloors[floor].name,WorldLocation::MageTower,{12,14},floor};
 for(size_t i=8;i<std::size(FieldSites);i++){
  const auto& site=FieldSites[i];result[next++]={site.id,site.name,site.realm,site.position,0};
 }
 return result;
}();
inline constexpr int DeveloperDestinationCount=int(DeveloperDestinations.size());
inline int developerDestination(std::string id){
 for(char& c:id)c=char(std::tolower(static_cast<unsigned char>(c)));
 if(id=="tower")id="tower-1";if(id=="boss")id="ruins";
 if(id=="kaldera"||id=="caldera")id="volcano";if(id=="varkhor")id="volcano-boss";
 for(int i=0;i<DeveloperDestinationCount;i++)if(id==DeveloperDestinations[i].id)return i;return -1;
}
struct DeveloperLaunch {bool enabled=false;int destination=-1;std::string error;};
inline bool parseDeveloperArgument(const std::string& arg,int& i,int argc,char** argv,DeveloperLaunch& launch){
 if(arg=="--dev"){launch.enabled=true;return true;}
 if(arg!="--start"&&arg.rfind("--start=",0)!=0)return false;
 launch.enabled=true;std::string id;
 if(arg=="--start"){if(i+1<argc&&std::string(argv[i+1]).rfind("--",0)!=0)id=argv[++i];}
 else id=arg.substr(8);
 int destination=developerDestination(id);
 if(destination<0){launch.destination=-1;launch.error=id.empty()?"Brakuje lokacji po --start. Wybierz ją z listy.":"Nieznana lokacja: "+id+". Wybierz ją z listy.";}
 else{launch.destination=destination;launch.error.clear();}return true;
}
inline constexpr int DeveloperLevel=100;
struct DeveloperSupply {Kind kind;int minimum;};
inline constexpr auto DeveloperSupplies=[]{
 std::array<DeveloperSupply,1+std::size(PotionTypes)> supplies{};
 supplies[0]={Kind::Arrow,100};
 for(size_t i=0;i<std::size(PotionTypes);i++)supplies[i+1]={PotionTypes[i].kind,50};
 return supplies;
}();
inline Item& developerBagSlot(Game& game){
 auto slot=std::find_if(game.bag.begin(),game.bag.end(),[](Item value){return value.empty();});
 if(slot!=game.bag.end())return *slot;
 const auto nextPage=game.bag.size();
 game.bag.resize(nextPage+BagSize);
 return game.bag[nextPage];
}
inline void replenishDeveloperItems(Game& game){
 std::array<bool,ItemKindCount> owned{};
 for(const auto& value:game.bag)if(!value.empty())owned[int(value.kind)]=true;
 for(const auto& value:game.equipment)if(!value.empty())owned[int(value.kind)]=true;
 for(int slot=0;slot<EquipSize;slot++)for(int id=1;id<ItemKindCount;id++){
  const Kind kind=Kind(id);
  if(!owned[id]&&gearSlot(kind)==slot)developerBagSlot(game)=item(kind);
 }
 for(int id=1;id<ItemKindCount;id++){
  const Kind kind=Kind(id);
  // Fragments must still be collected from their guardians to test the quest.
  if(!owned[id]&&gearSlot(kind)<0&&kind!=Kind::DawnShard&&kind!=Kind::Arrow&&potionSlotFor(kind)<0)
   developerBagSlot(game)=item(kind);
 }
 // Leave room to test picking up loot even when the catalogue fills a page.
 if(std::none_of(game.bag.begin(),game.bag.end(),[](const Item& value){return value.empty();}))
  game.bag.resize(game.bag.size()+BagSize);
}
inline void replenishDeveloperSupplies(Game& game){
 std::array<int,DeveloperSupplies.size()> missing{};
 for(std::size_t i=0;i<DeveloperSupplies.size();i++){
  auto [kind,minimum]=DeveloperSupplies[i];
  int available=game.bagCount(kind);
  // Older saves can contain oversized stacks. Keep them intact.
  if(available>=minimum&&std::any_of(game.bag.begin(),game.bag.end(),[kind](const Item& value){
    return value.kind==kind&&value.count>99;
  }))continue;
  int remaining=std::max(minimum,available);
  // Reuse existing stacks and consolidate small stacks before taking new slots.
  for(auto& stack:game.bag){
   if(stack.kind!=kind||stack.empty())continue;
   int count=std::min(remaining,99);
   if(count>0)stack.count=count;
   else stack={};
   remaining-=count;
  }
  missing[i]=remaining;
 }
 for(std::size_t i=0;i<DeveloperSupplies.size();i++){
  Kind kind=DeveloperSupplies[i].kind;
  int remaining=missing[i];
  while(remaining>0){
   int added=std::min(remaining,99);
   developerBagSlot(game)=item(kind,0,added);
   remaining-=added;
  }
 }
}
inline void refreshDeveloperLoadout(Game& game){
 for(int& rank:game.skillRanks)rank=std::max(rank,1);
 for(int i=1;i<int(Spell::Count);i++)game.learnedSpells|=spellBit(Spell(i));
 if(!game.knowsSpell(game.selectedSpell))game.chooseSpell(Spell::Fireball);
 replenishDeveloperSupplies(game);
 replenishDeveloperItems(game);
 for(const auto& profile:SkillProfiles){
  if(gearSlot(profile.weapon)!=0)continue;
  int freeSlot=-1;bool bound=false;
  for(int i=0;i<WeaponWheelSize;i++){
   const Slot source=game.weaponSource(i);
   if(source.place==Place::None&&freeSlot<0)freeSlot=i;
   if(source.place!=Place::None&&archetype(game.weaponWheel[i].kind)==profile.weapon)bound=true;
  }
  if(!bound&&freeSlot>=0){
   const auto weapon=std::find_if(game.bag.begin(),game.bag.end(),[&](const Item& value){
    return !value.empty()&&archetype(value.kind)==profile.weapon;
   });
   if(weapon!=game.bag.end())game.weaponWheel[freeSlot]={weapon->kind,weapon->tier};
   else if(archetype(game.equipment[0].kind)==profile.weapon)
    game.weaponWheel[freeSlot]={game.equipment[0].kind,game.equipment[0].tier};
  }
 }
}
inline bool developerSpawnSafe(const Game& g,V pos){
 if(!g.walkable(pos))return false;
 if(!g.inTower())for(const auto& enemy:g.enemies)if(enemy.alive&&distance(enemy.pos,pos)<enemy.radius()+.75f)return false;
 return true;
}
inline bool summonDeveloperEnemy(Game& game,int type){
 if(!game.developerSession||game.inTower()||game.dead()||game.storyBlocked()||type<0||type>=EnemyTypeCount)return false;
 const auto& info=enemyInfo(type);
 int id=FirstSummonedEnemyId;
 for(const auto& enemy:game.enemies)if(summonedEnemyId(enemy.id)){
  if(enemy.id==std::numeric_limits<int>::max())return false;
  id=std::max(id,enemy.id+1);
 }
 const V forward=normalized(length(game.facing)>.001f?game.facing:V{1,1});
 // Leave enough room for a charge or a melee approach after the menu closes.
 for(float range=std::max(3.f,info.radius+HeroBodyRadius+.5f);range<=6.f;range+=.5f)
  for(int step=0;step<24;step++){
   const int direction=step%2?(step+1)/2:-step/2;
   const V pos=game.player+rotateAbilityDirection(forward,direction*AnimationPi/12)*range;
   if(!game.enemyPositionClear(pos,info.radius)||!game.clearEnemySegment(game.player,pos,0))continue;
   if(std::any_of(game.chests.begin(),game.chests.end(),[&](const auto& chest){
     return chest.location==game.location&&distance(chest.pos,pos)<ChestRadius+info.radius;
    }))continue;
   if(std::any_of(game.enemies.begin(),game.enemies.end(),[&](const Enemy& enemy){
     return distance(enemy.pos,pos)<enemy.radius()+info.radius+.2f;
    }))continue;
   Enemy enemy{id,type,pos,pos,info.health,info.health};
   const V toward=game.player-pos;
   enemy.animation.face(toward.x,toward.y);
   enemy.behavior=EnemyBehavior::Chase;
   game.enemies.push_back(enemy);
   return true;
  }
 return false;
}
inline bool developerStart(Game& current,int destination,bool reset,unsigned seed=12593){
 if(destination<0||destination>=DeveloperDestinationCount)return false;
 Game candidate=reset?Game(seed):current;const auto& target=DeveloperDestinations[destination];
 if(target.location==WorldLocation::MageTower){candidate.enterMageTower();candidate.towerFloor=target.floor;candidate.dawnQuest=DawnQuest::AtTower;}
 else{candidate.activateField(target.location);candidate.towerFloor=0;if(target.location==WorldLocation::Desert||target.location==WorldLocation::Volcano){candidate.collectedFragments=std::max(target.location==WorldLocation::Volcano?2:1,candidate.collectedFragments);candidate.dawnQuest=DawnQuest::AtTower;if(target.location==WorldLocation::Volcano)candidate.desertBossDead=true;}if(candidate.storyBlocked())candidate.dawnQuest=DawnQuest::AtTower;}
 bool found=false;V spawn{};
 const FieldSite* site=target.location==WorldLocation::MageTower?nullptr:&fieldRegion(target.location,target.spawn);
 for(int ring=0;ring<=16&&!found;ring++)for(int angle=0;angle<(ring?24:1);angle++){
  float a=angle*6.2831853f/24;V position=target.spawn+V{std::cos(a),std::sin(a)}*(ring*.25f);
  if(developerSpawnSafe(candidate,position)&&(!site|| &fieldRegion(target.location,position)==site)){spawn=position;found=true;break;}
 }
 if(!found)return false;
 candidate.cancelBoulderAttacks();candidate.clearSandPrison();
 candidate.player=spawn;candidate.facing={0,-1};candidate.animation={};candidate.animation.face(0,-1);
 candidate.cancelBow();candidate.closeCorpse();candidate.openedChest=-1;candidate.towerReturnPending=false;
 candidate.rollRemaining=candidate.rollCd=candidate.swing=candidate.attackCd=candidate.spellCd=candidate.potionCd=candidate.strongCutCd=candidate.piercingArrowCd=candidate.healingCd=candidate.healingPulse=candidate.hurt=candidate.stepDistance=0;
 candidate.meleeDuration=SwingDuration;candidate.swingStrong=candidate.lastBowPiercing=false;candidate.manaRegenTime=0;
 candidate.weaponSkillCds={};
 candidate.arrows.clear();candidate.fireballs.clear();candidate.explosions.clear();candidate.effects.clear();candidate.combatImpacts.clear();candidate.pierceFlashes.clear();candidate.sounds.clear();
 candidate.developerSession=true;
 candidate.level=DeveloperLevel;
 candidate.hp=float(candidate.maxHp());candidate.mana=float(candidate.maxMana());
 candidate.xp=0;
 candidate.allocationRequired=false;
 candidate.skillAssigned.fill(true);
 refreshDeveloperLoadout(candidate);
 candidate.say(std::string("DEV — ")+target.name);
 current=std::move(candidate);return true;
}
}
