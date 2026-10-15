#pragma once
#include "game.hpp"
#include <cctype>
namespace ember {
struct DeveloperDestination {const char* id;const char* name;const char* hint;WorldLocation location;V spawn;int floor;};
inline constexpr int DeveloperDestinationCount=16;
inline constexpr std::array<DeveloperDestination,DeveloperDestinationCount> DeveloperDestinations={{{"forest","Przesieka Wędrowca","Początek lasu, bez prologu",WorldLocation::Forest,{8,19},0},{"ruins","Boss — Ruiny Mchu","Start nieopodal Kamiennego Strażnika",WorldLocation::Forest,{21,8},0},{"birches","Brzozowa polana","Leśna polana i pobliskie spotkania",WorldLocation::Forest,{14,43},0},{"grove","Zaciszny zagajnik","Południowy zagajnik",WorldLocation::Forest,{14,61},0},{"heart","Serce Kniei","Zapomniany krąg w głębi lasu",WorldLocation::Forest,{59,57},0},{"trail","Stary trakt","Południowe skrzyżowanie",WorldLocation::Forest,{39,62},0},{"wolves","Wilcza knieja","Wschodnia droga przez las",WorldLocation::Forest,{55,21},0},{"spiders","Pajęcze ostępy","Ścieżka w krainie pająków",WorldLocation::Forest,{35,42},0},{"tower-1","1. Sala przybycia","Eldrin i źródło odnowy",WorldLocation::MageTower,{12,14},0},{"tower-2","2. Biblioteka","Regały i kroniki",WorldLocation::MageTower,{12,14},1},{"tower-3","3. Alchemia","Kocioł, materiały i mikstury",WorldLocation::MageTower,{12,14},2},{"tower-4","4. Sala pieczęci","Kamienie runiczne i artefakt",WorldLocation::MageTower,{12,14},3},{"tower-5","5. Obserwatorium","Luneta i mapy nieba",WorldLocation::MageTower,{12,14},4},{"desert","Pustynia — Brama Piasków","Początek drugiej krainy",WorldLocation::Desert,DesertArrival,0},{"desert-oasis","Pustynia — Oaza Bursztynu","Palmy i szlak wśród kaktusów",WorldLocation::Desert,{18,39},0},{"desert-boss","Boss — Żywa Wydma","Arena drugiego strażnika",WorldLocation::Desert,{57,49},0}}};
inline int developerDestination(std::string id){
 for(char& c:id)c=char(std::tolower(static_cast<unsigned char>(c)));
 if(id=="tower")id="tower-1";if(id=="boss")id="ruins";
 for(int i=0;i<DeveloperDestinationCount;i++)if(id==DeveloperDestinations[i].id)return i;return -1;
}
struct DeveloperLaunch {bool enabled=false,kit=true;int destination=-1;std::string error;};
inline bool parseDeveloperArgument(const std::string& arg,int& i,int argc,char** argv,DeveloperLaunch& launch){
 if(arg=="--dev"){launch.enabled=true;return true;}
 if(arg=="--no-dev-kit"){launch.kit=false;return true;}
 if(arg!="--start"&&arg.rfind("--start=",0)!=0)return false;
 launch.enabled=true;std::string id;
 if(arg=="--start"){if(i+1<argc&&std::string(argv[i+1]).rfind("--",0)!=0)id=argv[++i];}
 else id=arg.substr(8);
 int destination=developerDestination(id);
 if(destination<0){launch.destination=-1;launch.error=id.empty()?"Brakuje lokacji po --start. Wybierz ją z listy.":"Nieznana lokacja: "+id+". Wybierz ją z listy.";}
 else{launch.destination=destination;launch.error.clear();}return true;
}
inline bool developerAddItem(Game& g,Kind kind,int minimum){
 int remaining=std::max(0,minimum-g.bagCount(kind));
 for(auto& value:g.bag)if(value.kind==kind&&value.tier==0&&!value.empty()){int count=std::min(remaining,99-value.count);if(count>0){value.count+=count;remaining-=count;}}
 for(auto& value:g.bag)if(value.empty()&&remaining>0){int count=std::min(99,remaining);value=item(kind,0,count);remaining-=count;}
 return remaining==0;
}
inline bool developerSpawnSafe(const Game& g,V pos){
 if(!g.walkable(pos))return false;
 if(!g.inTower())for(const auto& enemy:g.enemies)if(enemy.alive&&distance(enemy.pos,pos)<enemy.radius()+.75f)return false;
 return true;
}
inline bool developerStart(Game& current,int destination,bool reset,bool kit,unsigned seed=12593){
 if(destination<0||destination>=DeveloperDestinationCount)return false;
 Game candidate=reset?Game(seed):current;const auto& target=DeveloperDestinations[destination];
 if(target.location==WorldLocation::MageTower){candidate.enterMageTower();candidate.towerFloor=target.floor;candidate.dawnQuest=DawnQuest::AtTower;}
 else{candidate.activateField(target.location);candidate.towerFloor=0;if(target.location==WorldLocation::Desert){candidate.collectedFragments=std::max(1,candidate.collectedFragments);candidate.dawnQuest=DawnQuest::AtTower;}if(candidate.storyBlocked())candidate.dawnQuest=DawnQuest::AtTower;}
 bool found=false;V spawn{};
 for(int ring=0;ring<=16&&!found;ring++)for(int angle=0;angle<(ring?24:1);angle++){
  float a=angle*6.2831853f/24;V position=target.spawn+V{std::cos(a),std::sin(a)}*(ring*.25f);
  if(developerSpawnSafe(candidate,position)){spawn=position;found=true;break;}
 }
 if(!found)return false;
 candidate.player=spawn;candidate.facing={0,-1};candidate.animation={};candidate.animation.face(0,-1);
 candidate.cancelBow();candidate.closeCorpse();candidate.towerReturnPending=false;
 candidate.rollRemaining=candidate.rollCd=candidate.swing=candidate.attackCd=candidate.spellCd=candidate.potionCd=candidate.strongCutCd=candidate.piercingArrowCd=candidate.healingCd=candidate.healingPulse=candidate.hurt=candidate.stepDistance=0;
 candidate.meleeDuration=SwingDuration;candidate.swingStrong=candidate.lastBowPiercing=false;candidate.manaRegenTime=0;
 candidate.hp=float(candidate.maxHp());candidate.mana=float(candidate.maxMana());
 candidate.arrows.clear();candidate.fireballs.clear();candidate.explosions.clear();candidate.effects.clear();candidate.pierceFlashes.clear();candidate.sounds.clear();
 bool complete=true;
 if(kit){
  // Supplies do not grant skills: ranks come only from allocated level points.
  for(auto recipe:std::array<std::pair<Kind,int>,5>{{{Kind::Arrow,100},{Kind::Mushroom,15},{Kind::ForestEssence,15},{Kind::Health,10},{Kind::Mana,10}}})complete=developerAddItem(candidate,recipe.first,recipe.second)&&complete;
 }
 candidate.say(complete?std::string("DEV — ")+target.name:"DEV — lokacja gotowa; pełny plecak ograniczył zestaw testowy.");
 current=std::move(candidate);return true;
}
}
