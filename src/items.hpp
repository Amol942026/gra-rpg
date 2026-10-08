#pragma once
#include <string>
namespace ember {
enum class Kind { None,Sword,Axe,Helmet,Armor,Shield,Boots,Ring,Health,Mana,Crystal,Gold,
 WolfFang,WolfFur,BoarTusk,BoarHide,SpiderSilk,VenomSac,BoneShard,SlimeGel,AncientBark,ForestEssence,BanditBadge,RuneStone,Dagger,Cloak,Amulet,Count };
constexpr int ItemKindCount=int(Kind::Count);
struct Item { Kind kind=Kind::None; int tier=0,count=0; bool empty()const{return kind==Kind::None||count<=0;} };
inline Item item(Kind k,int tier=0,int count=1){return{k,tier,count};}
inline bool material(Kind k){return k>=Kind::WolfFang&&k<=Kind::RuneStone;}
inline int gearSlot(Kind k){switch(k){case Kind::Sword:case Kind::Axe:case Kind::Dagger:return 0;case Kind::Helmet:return 1;case Kind::Shield:return 2;case Kind::Armor:case Kind::Cloak:return 3;case Kind::Boots:return 4;case Kind::Ring:case Kind::Amulet:return 5;default:return -1;}}
inline bool stackable(Kind k){return material(k)||k==Kind::Health||k==Kind::Mana||k==Kind::Crystal||k==Kind::Gold;}
inline std::string itemName(Item i){
 static constexpr const char* names[]={"","Miecz wędrowca","Topór strażnika","Żelazny hełm","Zbroja zwiadowcy","Tarcza dębowa","Buty podróżnika","Pierścień żaru","Mikstura zdrowia","Mikstura many","Odłamek echa","Złote monety",
 "Kieł wilka","Sierść wilka","Kieł dzika","Skóra dzika","Pajęcza nić","Woreczek jadu","Fragment kości","Śluz szlama","Prastara kora","Esencja kniei","Odznaka rozbójnika","Kamień runiczny","Sztylet łowcy","Płaszcz z wilczej skóry","Bursztynowy amulet"};
 int k=int(i.kind);return k>=0&&k<ItemKindCount?names[k]:"Nieznany przedmiot";
}
inline const char* itemShortName(Kind k){switch(k){case Kind::WolfFang:return "kieł";case Kind::WolfFur:return "sierść";case Kind::BoarTusk:return "kieł";case Kind::BoarHide:return "skóra";case Kind::SpiderSilk:return "nić";case Kind::VenomSac:return "jad";case Kind::BoneShard:return "kość";case Kind::SlimeGel:return "śluz";case Kind::AncientBark:return "kora";case Kind::ForestEssence:return "esencja";case Kind::BanditBadge:return "odznaka";case Kind::RuneStone:return "runa";case Kind::Crystal:return "odłamek";case Kind::Health:return "mikstura HP";case Kind::Mana:return "mikstura MP";case Kind::Dagger:return "sztylet";case Kind::Cloak:return "płaszcz";case Kind::Amulet:return "amulet";default:return "sprzęt";}}
inline int power(Item i){if(i.empty()||gearSlot(i.kind)<0)return 0;return (i.kind==Kind::Axe?8:i.kind==Kind::Sword?6:i.kind==Kind::Dagger?9:i.kind==Kind::Ring?2:i.kind==Kind::Amulet?4:i.kind==Kind::Cloak?5:3)+i.tier*3;}
inline std::string materialDescription(Kind k){switch(k){
 case Kind::WolfFang:return "Ostry kieł upolowanego wilka.";case Kind::WolfFur:return "Gęsta, srebrzysta sierść wilka.";
 case Kind::BoarTusk:return "Zakrzywiony kieł leśnego dzika.";case Kind::BoarHide:return "Wytrzymała skóra dzika.";
 case Kind::SpiderSilk:return "Mocna nić leśnego pająka.";case Kind::VenomSac:return "Zamknięty woreczek z jadem.";
 case Kind::BoneShard:return "Kość pokonanego nieumarłego.";case Kind::SlimeGel:return "Lepki materiał ze śluzowca.";
 case Kind::AncientBark:return "Kora ożywionego drzewa.";case Kind::ForestEssence:return "Magiczna esencja starego lasu.";
 case Kind::BanditBadge:return "Trofeum leśnych rozbójników.";case Kind::RuneStone:return "Kamień z dawną leśną runą.";
 default:return "Materiał znaleziony w lesie.";}}
}
