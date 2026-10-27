#pragma once
#include <string>
#include <array>
namespace ember {
enum class Kind { None,Sword,Axe,Helmet,Armor,Shield,Boots,Ring,Health,Mana,Crystal,Gold,
 WolfFang,WolfFur,BoarTusk,BoarHide,SpiderSilk,VenomSac,BoneShard,SlimeGel,AncientBark,ForestEssence,BanditBadge,RuneStone,Dagger,Cloak,Amulet,Mushroom,Bow,Arrow,DawnShard,ScorpionSting,ScarabShell,CobraScale,SandEssence,WolfKnife,WolfCloak,MoonBow,BoarArmor,TuskAxe,SilkBoots,VenomRing,WebCowl,BoneHelmet,CryptSword,BanditBow,BanditBoots,RootShield,GroveAmulet,GelAmulet,GuardianSword,GuardianArmor,GuardianHelmet,EchoRing,StingDagger,ScarabHelmet,ScarabArmor,CobraBoots,MirageBow,SunAmulet,DuneBlade,SandRobe,DuneShield,SlimeBlade,SlimeCowl,SlimeRing,WolfHood,WolfBoots,WolfHeartAmulet,BoneShield,BoneArmor,GraveBow,GuardianBoots,GuardianShield,GuardianAxe,BoarBoots,BoarShield,BoarHelm,SpiderDagger,WebRobe,NightSilkCloak,BanditSword,BanditHelm,OutlawRing,BarkHelm,RootBoots,GroveBow,ScorpionAxe,StingShield,AmberRing,ScarabBoots,ScarabAxe,ScarabRing,CobraCloak,ScaleShield,SerpentAmulet,DuneBoots,DuneHelmet,DawnBow,Count };
constexpr int ItemKindCount=int(Kind::Count);
enum class Rarity { Common, Rare, VeryRare, ExtremelyRare };
struct GearDefinition { const char* name; Kind shape; Rarity rarity; int basePower; const char* description; };
inline constexpr std::array<GearDefinition,64> BeastGear{{
 {"Sztylet wilczego kła",Kind::Dagger,Rarity::Rare,12,"Ostrze oprawione w srebrzysty wilczy kieł."},
 {"Płaszcz tropiciela",Kind::Cloak,Rarity::Rare,7,"Ciepły płaszcz z futra leśnych wilków."},
 {"Łuk księżycowego łowcy",Kind::Bow,Rarity::VeryRare,19,"Łuk z jasnego drewna i wilczej cięciwy."},
 {"Pancerz dziczej skóry",Kind::Armor,Rarity::Rare,8,"Skórzany pancerz wzmocniony dziczymi kłami."},
 {"Topór dziczego kła",Kind::Axe,Rarity::VeryRare,17,"Ciężkie ostrze z okuciem w kształcie kła."},
 {"Buty pajęczego szlaku",Kind::Boots,Rarity::Rare,5,"Lekkie buty obszyte mocną pajęczą nicią."},
 {"Pierścień jadowitego splotu",Kind::Ring,Rarity::VeryRare,7,"Srebrny pierścień z zielonym kamieniem."},
 {"Kaptur pajęczej nici",Kind::Helmet,Rarity::Rare,6,"Kaptur tkany z nici leśnych pająków."},
 {"Hełm kościanej straży",Kind::Helmet,Rarity::Rare,6,"Stalowy hełm z rzeźbionymi okuciami."},
 {"Miecz zapomnianej krypty",Kind::Sword,Rarity::VeryRare,17,"Stare ostrze pokryte błękitnymi runami."},
 {"Łuk leśnego rozbójnika",Kind::Bow,Rarity::Rare,17,"Krótki łuk ze skóry i ciemnego drewna."},
 {"Buty cichego kroku",Kind::Boots,Rarity::Rare,6,"Wysokie buty z miękkiej leśnej skóry."},
 {"Tarcza prastarych korzeni",Kind::Shield,Rarity::Rare,7,"Splecione korzenie chronią przed ciosami."},
 {"Amulet serca kniei",Kind::Amulet,Rarity::VeryRare,8,"Bursztyn zamknięty w żywych korzeniach."},
 {"Amulet szmaragdowej kropli",Kind::Amulet,Rarity::Rare,5,"Zastygła kropla esencji leśnego szlama."},
 {"Runiczne ostrze strażnika",Kind::Sword,Rarity::VeryRare,19,"Kamienny strażnik chronił runy tego miecza."},
 {"Zbroja kamiennego strażnika",Kind::Armor,Rarity::VeryRare,12,"Płyty pancerza z pradawnego kamienia."},
 {"Hełm mchu i kamienia",Kind::Helmet,Rarity::Rare,8,"Kamienne okucia oplecione zielonym mchem."},
 {"Pierścień pradawnego echa",Kind::Ring,Rarity::ExtremelyRare,11,"Fioletowy kryształ pamięta dawne ruiny."},
 {"Sztylet bursztynowego żądła",Kind::Dagger,Rarity::Rare,14,"Smukłe ostrze w kształcie żądła skorpiona."},
 {"Hełm miedzianego skarabeusza",Kind::Helmet,Rarity::Rare,8,"Miedziany hełm z ornamentem skarabeusza."},
 {"Zbroja pustynnego skarabeusza",Kind::Armor,Rarity::VeryRare,11,"Miedziane płytki zachodzą na siebie jak łuski."},
 {"Buty piaskowej kobry",Kind::Boots,Rarity::Rare,7,"Giętkie buty ze złocistych łusek kobry."},
 {"Łuk pustynnego mirażu",Kind::Bow,Rarity::VeryRare,22,"Złocisty łuk zdobiony pustynnymi runami."},
 {"Amulet palącego słońca",Kind::Amulet,Rarity::VeryRare,9,"Bursztynowe słońce w miedzianej oprawie."},
 {"Ostrze żywej wydmy",Kind::Sword,Rarity::ExtremelyRare,25,"Piasek krąży wokół klingi pustynnego bossa."},
 {"Płaszcz piaskowej burzy",Kind::Cloak,Rarity::ExtremelyRare,16,"Złote nici migoczą jak nawiewany piasek."},
 {"Tarcza pustynnego monolitu",Kind::Shield,Rarity::VeryRare,10,"Kamienna tarcza z ornamentem wydmy."},
 {"Ostrze zielonej kropli",Kind::Sword,Rarity::Common,7,"Proste ostrze pokryte zaschniętą esencją."},
 {"Kaptur mglistego runa",Kind::Helmet,Rarity::Rare,5,"Kaptur chroniący przed leśną wilgocią."},
 {"Pierścień bursztynowego żelu",Kind::Ring,Rarity::VeryRare,6,"Przejrzysta kropla w bursztynowej oprawie."},
 {"Kaptur szarego wilka",Kind::Helmet,Rarity::Common,3,"Szare futro chroni głowę wędrowca."},
 {"Buty srebrnego tropu",Kind::Boots,Rarity::Rare,5,"Srebrzysta sierść osłania skórzane cholewy."},
 {"Amulet wilczego serca",Kind::Amulet,Rarity::ExtremelyRare,10,"Księżycowy kamień w oprawie z kłów."},
 {"Tarcza kościanych odłamków",Kind::Shield,Rarity::Common,4,"Dębowa tarcza wzmocniona białą kością."},
 {"Pancerz strażnika grobów",Kind::Armor,Rarity::Rare,7,"Kościane płytki na ciemnej kolczudze."},
 {"Łuk milczącej krypty",Kind::Bow,Rarity::VeryRare,18,"Kościane ramiona zdobione zimnymi runami."},
 {"Nagolenniki omszałych ruin",Kind::Boots,Rarity::Rare,8,"Kamienne okucia oplecione młodym mchem."},
 {"Tarcza pieczęci strażnika",Kind::Shield,Rarity::VeryRare,10,"Runiczna płyta z dawnej bramy ruin."},
 {"Topór przebudzonej skały",Kind::Axe,Rarity::ExtremelyRare,24,"W kamiennej głowicy pulsuje pradawny żar."},
 {"Buty dziczej skóry",Kind::Boots,Rarity::Common,3,"Gruba skóra i mocne rzemienie."},
 {"Tarcza leśnego odyńca",Kind::Shield,Rarity::Rare,6,"Zaokrąglona tarcza okutą parą kłów."},
 {"Hełm odyńca",Kind::Helmet,Rarity::VeryRare,9,"Dwa jasne kły chronią skronie wojownika."},
 {"Sztylet leśnego pająka",Kind::Dagger,Rarity::Common,8,"Lekkie ostrze oplecione mocną nicią."},
 {"Szata tkacza pajęczyn",Kind::Armor,Rarity::Rare,8,"Gęsta pajęcza tkanina i srebrne przeszycia."},
 {"Płaszcz nocnego jedwabiu",Kind::Cloak,Rarity::ExtremelyRare,14,"Ciemne nici migoczą fioletowym blaskiem."},
 {"Szabla leśnego rabusia",Kind::Sword,Rarity::Common,9,"Wyszczerbiona klinga z ciemną rękojeścią."},
 {"Hełm z leśnej zasadzki",Kind::Helmet,Rarity::Rare,6,"Ciemna stal skryta pod skórzanym kapturem."},
 {"Sygnet króla bezdroży",Kind::Ring,Rarity::VeryRare,7,"Sygnet z bursztynem znalezionym w łupach."},
 {"Hełm starej kory",Kind::Helmet,Rarity::Common,4,"Twarda kora spięta miedzianymi obręczami."},
 {"Buty splecionych korzeni",Kind::Boots,Rarity::Rare,6,"Elastyczne korzenie osłaniają stopy."},
 {"Łuk przebudzonej kniei",Kind::Bow,Rarity::ExtremelyRare,23,"Żywe drewno przechowuje światło starego lasu."},
 {"Topór pustynnego żądła",Kind::Axe,Rarity::Common,11,"Bursztynowa głowica z twardego pancerza."},
 {"Tarcza bursztynowego żądła",Kind::Shield,Rarity::Rare,7,"Wygięte płyty osłaniają przed ciosami."},
 {"Pierścień pustynnego bursztynu",Kind::Ring,Rarity::VeryRare,8,"W złotym bursztynie zastygła kropla jadu."},
 {"Buty miedzianego szlaku",Kind::Boots,Rarity::Common,4,"Miedziane blaszki na piaskowej skórze."},
 {"Topór pancernej skorupy",Kind::Axe,Rarity::Rare,16,"Miedziany skarabeusz zdobi szerokie ostrze."},
 {"Sygnet słonecznego skarabeusza",Kind::Ring,Rarity::ExtremelyRare,10,"Złoty owad strzeże fioletowego kryształu."},
 {"Płaszcz piaskowych łusek",Kind::Cloak,Rarity::Common,6,"Piaskowa tkanina z warstwą drobnych łusek."},
 {"Tarcza złocistej kobry",Kind::Shield,Rarity::Rare,8,"Złociste łuski zachodzą na siebie jak płytki."},
 {"Amulet władcy węży",Kind::Amulet,Rarity::ExtremelyRare,11,"Oko kobry lśni w oprawie z łusek."},
 {"Buty wędrującej wydmy",Kind::Boots,Rarity::Rare,10,"Piaskowe okucia chronią przed gorącem."},
 {"Hełm serca pustyni",Kind::Helmet,Rarity::VeryRare,12,"Kamienna korona z rozżarzonym bursztynem."},
 {"Łuk ostatniej jutrzenki",Kind::Bow,Rarity::ExtremelyRare,27,"Złociste ramiona rozświetla chłodny żar."},
}};
inline constexpr bool beastGear(Kind k){return k>=Kind::WolfKnife&&k<Kind::Count;}
inline constexpr int EquipmentKindCount=11+int(BeastGear.size());
inline constexpr Kind archetype(Kind k){return beastGear(k)?BeastGear[int(k)-int(Kind::WolfKnife)].shape:k;}
inline constexpr Rarity itemRarity(Kind k){
 if(beastGear(k))return BeastGear[int(k)-int(Kind::WolfKnife)].rarity;
 switch(k){
 case Kind::DawnShard:return Rarity::ExtremelyRare;
 case Kind::Ring:case Kind::RuneStone:case Kind::SandEssence:return Rarity::VeryRare;
 case Kind::Axe:case Kind::Helmet:case Kind::Dagger:case Kind::Cloak:case Kind::Amulet:
 case Kind::Crystal:case Kind::VenomSac:case Kind::ForestEssence:case Kind::BanditBadge:return Rarity::Rare;
 default:return Rarity::Common;
 }
}
inline constexpr const char* rarityName(Rarity r){switch(r){case Rarity::Common:return "Pospolity";case Rarity::Rare:return "Rzadki";case Rarity::VeryRare:return "Bardzo rzadki";case Rarity::ExtremelyRare:return "Niezwykle rzadki";}return "";}
inline constexpr int rarityDropChance(Rarity r){switch(r){case Rarity::Common:return 60;case Rarity::Rare:return 15;case Rarity::VeryRare:return 4;case Rarity::ExtremelyRare:return 1;}return 0;}
inline constexpr int itemDropChance(Kind k){return k==Kind::None?0:k==Kind::DawnShard?100:rarityDropChance(itemRarity(k));}

struct Item { Kind kind=Kind::None; int tier=0,count=0; bool empty()const{return kind==Kind::None||count<=0;} };
inline Item item(Kind k,int tier=0,int count=1){return{k,tier,count};}
inline bool material(Kind k){return (k>=Kind::WolfFang&&k<=Kind::RuneStone)||(k>=Kind::ScorpionSting&&k<=Kind::SandEssence);}
inline int gearSlot(Kind k){switch(archetype(k)){case Kind::Sword:case Kind::Axe:case Kind::Dagger:case Kind::Bow:return 0;case Kind::Helmet:return 1;case Kind::Shield:return 2;case Kind::Armor:case Kind::Cloak:return 3;case Kind::Boots:return 4;case Kind::Ring:case Kind::Amulet:return 5;default:return -1;}}
inline bool stackable(Kind k){return k==Kind::Arrow||k==Kind::Mushroom||material(k)||k==Kind::Health||k==Kind::Mana||k==Kind::Crystal||k==Kind::Gold;}
inline std::string itemName(Item i){
 static constexpr const char* names[]={"","Miecz wędrowca","Topór strażnika","Żelazny hełm","Zbroja zwiadowcy","Tarcza dębowa","Buty podróżnika","Pierścień żaru","Mikstura zdrowia","Mikstura many","Odłamek echa","Złote monety",
 "Kieł wilka","Sierść wilka","Kieł dzika","Skóra dzika","Pajęcza nić","Woreczek jadu","Fragment kości","Śluz szlama","Prastara kora","Esencja kniei","Odznaka rozbójnika","Kamień runiczny","Sztylet łowcy","Płaszcz z wilczej skóry","Bursztynowy amulet","Borowik","Łuk podróżnika","Strzały","Część Serca Świtu","Żądło skorpiona","Pancerz skarabeusza","Łuska kobry","Esencja piasku"};
 if(beastGear(i.kind))return BeastGear[int(i.kind)-int(Kind::WolfKnife)].name;
 int k=int(i.kind);return k>=0&&k<ItemKindCount?names[k]:"Nieznany przedmiot";
}
inline const char* itemShortName(Kind k){switch(archetype(k)){case Kind::ScorpionSting:return "żądło";case Kind::ScarabShell:return "pancerz";case Kind::CobraScale:return "łuska";case Kind::SandEssence:return "piasek";case Kind::Bow:return "łuk";case Kind::Arrow:return "strzały";case Kind::Sword:return "miecz";case Kind::Axe:return "topór";case Kind::Mushroom:return "borowik";case Kind::WolfFang:return "kieł";case Kind::WolfFur:return "sierść";case Kind::BoarTusk:return "kieł";case Kind::BoarHide:return "skóra";case Kind::SpiderSilk:return "nić";case Kind::VenomSac:return "jad";case Kind::BoneShard:return "kość";case Kind::SlimeGel:return "śluz";case Kind::AncientBark:return "kora";case Kind::ForestEssence:return "esencja";case Kind::BanditBadge:return "odznaka";case Kind::RuneStone:return "runa";case Kind::Crystal:return "odłamek";case Kind::Health:return "mikstura HP";case Kind::Mana:return "mikstura MP";case Kind::Dagger:return "sztylet";case Kind::Cloak:return "płaszcz";case Kind::Amulet:return "amulet";default:return "sprzęt";}}
inline Rarity itemRarity(Item value){return itemRarity(value.kind);}
inline int itemDropChance(Item value){return itemDropChance(value.kind);}
inline int power(Item i){if(i.empty()||gearSlot(i.kind)<0)return 0;if(beastGear(i.kind))return BeastGear[int(i.kind)-int(Kind::WolfKnife)].basePower+i.tier*3;return (i.kind==Kind::Bow?14:i.kind==Kind::Axe?8:i.kind==Kind::Sword?6:i.kind==Kind::Dagger?9:i.kind==Kind::Ring?2:i.kind==Kind::Amulet?4:i.kind==Kind::Cloak?5:3)+i.tier*3;}
inline std::string gearDescription(Kind k){return beastGear(k)?BeastGear[int(k)-int(Kind::WolfKnife)].description:"";}
inline std::string materialDescription(Kind k){switch(k){
 case Kind::WolfFang:return "Ostry kieł upolowanego wilka.";case Kind::WolfFur:return "Gęsta, srebrzysta sierść wilka.";
 case Kind::BoarTusk:return "Zakrzywiony kieł leśnego dzika.";case Kind::BoarHide:return "Wytrzymała skóra dzika.";
 case Kind::SpiderSilk:return "Mocna nić leśnego pająka.";case Kind::VenomSac:return "Zamknięty woreczek z jadem.";
 case Kind::BoneShard:return "Kość pokonanego nieumarłego.";case Kind::SlimeGel:return "Lepki materiał ze śluzowca.";
 case Kind::AncientBark:return "Kora ożywionego drzewa.";case Kind::ForestEssence:return "Magiczna esencja starego lasu.";
 case Kind::BanditBadge:return "Trofeum leśnych rozbójników.";case Kind::RuneStone:return "Kamień z dawną leśną runą.";
 case Kind::ScorpionSting:return "Bursztynowe żądło pustynnego skorpiona.";case Kind::ScarabShell:return "Twardy pancerz miedzianego skarabeusza.";case Kind::CobraScale:return "Złocista łuska kobry piaskowej.";case Kind::SandEssence:return "Magiczny piasek z serca żywej wydmy.";default:return "Materiał znaleziony podczas wyprawy.";}}
}
