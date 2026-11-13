#pragma once
#include "game.hpp"
#include <cctype>
namespace ember {
struct DeveloperDestination {const char* id;const char* name;const char* hint;WorldLocation location;V spawn;int floor;};
inline constexpr int DeveloperDestinationCount=19;
inline constexpr std::array<DeveloperDestination,DeveloperDestinationCount> DeveloperDestinations={{
 {"forest","Przesieka Wędrowca","Początek lasu, bez prologu",WorldLocation::Forest,ForestArrival,0},
 {"ruins","Boss — Ruiny Mchu","Start nieopodal Kamiennego Strażnika",WorldLocation::Forest,{21,8},0},
 {"birches","Brzozowa polana","Leśna polana i pobliskie spotkania",WorldLocation::Forest,{14,43},0},
 {"grove","Zaciszny zagajnik","Południowy zagajnik",WorldLocation::Forest,{14,61},0},
 {"heart","Serce Kniei","Zapomniany krąg w głębi lasu",WorldLocation::Forest,{59,57},0},
 {"trail","Stary trakt","Południowe skrzyżowanie",WorldLocation::Forest,{39,62},0},
 {"wolves","Wilcza knieja","Wschodnia droga przez las",WorldLocation::Forest,{55,21},0},
 {"spiders","Pajęcze ostępy","Ścieżka w krainie pająków",WorldLocation::Forest,{35,42},0},
 {"tower-1","1. Sala przybycia","Eldrin i źródło odnowy",WorldLocation::MageTower,{12,14},0},
 {"tower-2","2. Biblioteka","Regały i kroniki",WorldLocation::MageTower,{12,14},1},
 {"tower-3","3. Alchemia","Kocioł, materiały i mikstury",WorldLocation::MageTower,{12,14},2},
 {"tower-4","4. Sala pieczęci","Kamienie runiczne i artefakt",WorldLocation::MageTower,{12,14},3},
 {"tower-5","5. Obserwatorium","Luneta i mapy nieba",WorldLocation::MageTower,{12,14},4},
 {"desert","Pustynia — Brama Piasków","Początek drugiej krainy",WorldLocation::Desert,DesertArrival,0},
 {"desert-oasis","Pustynia — Oaza Bursztynu","Palmy i szlak wśród kaktusów",WorldLocation::Desert,{18,39},0},
 {"desert-boss","Boss — Żywa Wydma","Arena drugiego strażnika",WorldLocation::Desert,{57,49},0},
 {"volcano","Kaldera — Brama Popiołu","Początek trzeciej krainy",WorldLocation::Volcano,VolcanoArrival,0},
 {"volcano-forges","Obsydianowe Kuźnie","Wschodni szlak przez kalderę",WorldLocation::Volcano,{60,43},0},
 {"volcano-boss","Boss — Varkhor","Start przy trzecim bossie",WorldLocation::Volcano,{56,15},0}
}};
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
inline constexpr std::array<std::pair<Kind,int>,3> DeveloperSupplies{{
 {Kind::Arrow,100},{Kind::Health,50},{Kind::Mana,50}
}};
inline Item& developerBagSlot(Game& game){
 auto slot=std::find_if(game.bag.begin(),game.bag.end(),[](Item value){return value.empty();});
 if(slot!=game.bag.end())return *slot;
 const auto nextPage=game.bag.size();
 game.bag.resize(nextPage+BagSize);
 return game.bag[nextPage];
}
inline void replenishDeveloperEquipment(Game& game){
 std::array<bool,ItemKindCount> owned{};
 for(const auto& value:game.bag)if(!value.empty())owned[int(value.kind)]=true;
 for(const auto& value:game.equipment)if(!value.empty())owned[int(value.kind)]=true;
 for(int slot=0;slot<EquipSize;slot++)for(int id=1;id<ItemKindCount;id++){
  const Kind kind=Kind(id);
  if(!owned[id]&&gearSlot(kind)==slot)developerBagSlot(game)=item(kind);
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
  Kind kind=DeveloperSupplies[i].first;
  int remaining=missing[i];
  while(remaining>0){
   int added=std::min(remaining,99);
   developerBagSlot(game)=item(kind,0,added);
   remaining-=added;
  }
 }
}
inline bool developerSpawnSafe(const Game& g,V pos){
 if(!g.walkable(pos))return false;
 if(!g.inTower())for(const auto& enemy:g.enemies)if(enemy.alive&&distance(enemy.pos,pos)<enemy.radius()+.75f)return false;
 return true;
}
inline bool developerStart(Game& current,int destination,bool reset,unsigned seed=12593){
 if(destination<0||destination>=DeveloperDestinationCount)return false;
 Game candidate=reset?Game(seed):current;const auto& target=DeveloperDestinations[destination];
 if(target.location==WorldLocation::MageTower){candidate.enterMageTower();candidate.towerFloor=target.floor;candidate.dawnQuest=DawnQuest::AtTower;}
 else{candidate.activateField(target.location);candidate.towerFloor=0;if(target.location==WorldLocation::Desert||target.location==WorldLocation::Volcano){candidate.collectedFragments=std::max(target.location==WorldLocation::Volcano?2:1,candidate.collectedFragments);candidate.dawnQuest=DawnQuest::AtTower;if(target.location==WorldLocation::Volcano)candidate.desertBossDead=true;}if(candidate.storyBlocked())candidate.dawnQuest=DawnQuest::AtTower;}
 bool found=false;V spawn{};
 for(int ring=0;ring<=16&&!found;ring++)for(int angle=0;angle<(ring?24:1);angle++){
  float a=angle*6.2831853f/24;V position=target.spawn+V{std::cos(a),std::sin(a)}*(ring*.25f);
  if(developerSpawnSafe(candidate,position)){spawn=position;found=true;break;}
 }
 if(!found)return false;
 candidate.cancelBoulderAttacks();candidate.clearSandPrison();
 candidate.player=spawn;candidate.facing={0,-1};candidate.animation={};candidate.animation.face(0,-1);
 candidate.cancelBow();candidate.closeCorpse();candidate.openedChest=-1;candidate.towerReturnPending=false;
 candidate.rollRemaining=candidate.rollCd=candidate.swing=candidate.attackCd=candidate.spellCd=candidate.potionCd=candidate.strongCutCd=candidate.piercingArrowCd=candidate.healingCd=candidate.healingPulse=candidate.hurt=candidate.stepDistance=0;
 candidate.meleeDuration=SwingDuration;candidate.swingStrong=candidate.lastBowPiercing=false;candidate.manaRegenTime=0;
 candidate.arrows.clear();candidate.fireballs.clear();candidate.explosions.clear();candidate.effects.clear();candidate.pierceFlashes.clear();candidate.sounds.clear();
 candidate.developerSession=true;
 candidate.level=DeveloperLevel;
 candidate.hp=float(candidate.maxHp());candidate.mana=float(candidate.maxMana());
 candidate.xp=0;
 candidate.allocationRequired=false;
 for(int& rank:candidate.skillRanks)rank=std::max(rank,1);
 for(int i=1;i<int(Spell::Count);i++)candidate.learnedSpells|=spellBit(Spell(i));
 if(!candidate.knowsSpell(candidate.selectedSpell))candidate.chooseSpell(Spell::Fireball);
 replenishDeveloperSupplies(candidate);
 replenishDeveloperEquipment(candidate);
 candidate.say(std::string("DEV — ")+target.name);
 current=std::move(candidate);return true;
}
}
