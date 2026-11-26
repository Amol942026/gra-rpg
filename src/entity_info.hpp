#pragma once
#include <algorithm>

namespace ember {
constexpr int ForestEnemyTypeCount=8,ForestPropTypeCount=7,LegacyEnemyTypeCount=12,LegacyPropTypeCount=14;
constexpr int StoneGuardianType=3;
constexpr int CopperScarabType=9,SandCobraType=10;
constexpr int VolcanoFirstType=12,VolcanoBossType=18;
inline constexpr float LargeBossMeleeReach=2.15f,VarkhorFissureReach=1.35f;
inline constexpr float VarkhorFirstFissure=.95f,VarkhorEruptionGesture=1.37f;
constexpr int SlagCrawlerType=VolcanoFirstType,LavaSalamanderType=13,FireImpType=14,EmberDrakeType=17;
constexpr int AshWraithType=15,ObsidianSentinelType=16;
constexpr int SporekinType=19,DuneEyeType=20,CinderhornType=21;
constexpr int DuneJackalType=22,GlassReaperType=23;
constexpr int PeatToad=24,MarshLeech=25,MarshLight=26,BellKeeper=27,EnemyTypeCount=28;
constexpr bool marshEnemy(int type){return type>=PeatToad&&type<=BellKeeper;}
constexpr int marshMotionFamily(int type){return type==PeatToad?0:type==MarshLeech?10:type==MarshLight?15:type==BellKeeper?7:type;};
constexpr bool newDesertEnemy(int type){return type==DuneJackalType||type==GlassReaperType;}
constexpr bool smoothEnemyTurn(int type){
 return type==SlagCrawlerType||type==LavaSalamanderType||type==EmberDrakeType||type==FireImpType||type==AshWraithType||
        type==ObsidianSentinelType||type==CinderhornType||newDesertEnemy(type)||marshEnemy(type);
}
constexpr int ForestDecorationStart=18,DesertDecorationStart=23,VolcanoDecorationStart=28;
constexpr int DecorationsPerRealm=5,VolcanoLandmarkCount=4;
constexpr int VolcanoLandmarkStart=VolcanoDecorationStart+DecorationsPerRealm;
constexpr int MarshPropStart=VolcanoLandmarkStart+VolcanoLandmarkCount,PropTypeCount=MarshPropStart+10;
constexpr int MarshFishingHut=MarshPropStart+2;
constexpr int MarshFishTrap=MarshPropStart+8,MarshPeatCart=MarshPropStart+9;
inline constexpr bool volcanoLandmark(int type){return type>=VolcanoLandmarkStart&&type<MarshPropStart;}
inline constexpr bool volcanoSceneryProp(int type){return type>=LegacyPropTypeCount&&type<ForestDecorationStart;}
constexpr int RegionalEnemyTypeCount=CinderhornType-SporekinType+1;
inline constexpr bool regionalEnemy(int type){return type>=SporekinType&&type<=GlassReaperType;}
constexpr float RegionalEnemyFallTime=.48f;
enum class DamageType { Physical, Magic };
struct EnemyInfo {
    const char* name;
    const char* kind;
    float health,speed;
    int attack,defense;
    int magicDefense=0;
    DamageType attackType=DamageType::Physical;
    float radius=.48f;
    int experience=25;
    float cooldown=1.2f,visualHeight=86,attackReach=1.25f;
};
inline const EnemyInfo& enemyInfo(int type) {
    static constexpr EnemyInfo entries[]={
        {"Szlam","Śluzowiec",36,1.85f,9,0,0,DamageType::Physical,.48f,25,1.2f,45},
        {"Wilk","Bestia",54,2.6f,12,1,0,DamageType::Physical,.8f,35,1.2f,76},
        {"Szkielet","Nieumarły",72,1.85f,15,3,0,DamageType::Physical,.46f,45,1.2f,86},
        {"Kamienny Strażnik","Konstrukt · boss",260,1.35f,25,8,6,DamageType::Magic,.84f,120,1.5f,126},
        {"Dzik","Bestia",88,2.25f,15,4,1,DamageType::Physical,.8f,42,1.25f,69},
        {"Leśny pająk","Jadowita bestia",48,2.8f,13,1,3,DamageType::Magic,.86f,38,1.1f,55},
        {"Leśny rozbójnik","Człowiek",96,1.85f,18,4,2,DamageType::Physical,.48f,55,1.1f,92},
        {"Ożywione drzewo","Duch kniei",170,1.45f,21,6,5,DamageType::Physical,.94f,65,1.6f,164},
        {"Skorpion bursztynowy","Pustynny pajęczak",82,2.35f,18,3,3,DamageType::Magic,.85f,48,1.35f,93},
        {"Skarabeusz miedziany","Pustynny owad",96,2.1f,17,6,2,DamageType::Physical,.72f,52,1.2f,67},
        {"Kobra piaskowa","Pustynny gad",88,2.7f,21,2,4,DamageType::Magic,.64f,56,1.35f,112},
        {"Żywa Wydma","Duch piasku · boss",560,1.45f,31,10,7,DamageType::Magic,1.45f,220,1.8f,190},
        {"Żużlowy pełzacz","Pancernik kaldery",138,1.9f,26,10,5,DamageType::Physical,.68f,72,1.4f,66},
        {"Salamandra lawowa","Ognisty gad",120,2.9f,28,5,8,DamageType::Magic,.68f,76,1.1f,70},
        {"Ognisty chochlik","Demon żaru",104,2.35f,27,3,10,DamageType::Magic,.46f,82,2.1f,119},
        {"Popielny upiór","Duch pogorzeliska",135,1.75f,30,4,13,DamageType::Magic,.5f,90,2.5f,129},
        {"Obsydianowy wojownik","Strażnik kuźni",205,1.65f,33,14,6,DamageType::Physical,.62f,100,1.55f,109},
        {"Żarowy drak","Skrzydlata bestia",155,2.65f,31,7,9,DamageType::Magic,.83f,96,1.2f,126},
        {"Varkhor, Serce Kaldery","Władca wulkanu · boss",940,1.3f,42,17,12,DamageType::Magic,1.3f,390,1.85f,205},
        {"Zarodnik kniei","Zarodnikowiec",65,2.2f,13,1,4,DamageType::Magic,.55f,39,1.45f,92,1.45f},
        {"Oko Wydm","Konstrukt piaskowca",115,1.65f,20,5,6,DamageType::Magic,.64f,62,1.7f,112,2.05f},
        {"Rogacz Żaru","Bestia wulkaniczna",174,1.95f,32,9,7,DamageType::Physical,.75f,95,1.65f,145,1.55f},
        {"Szakal wydmowy","Bestia pustyni",108,2.65f,21,4,2,DamageType::Physical,.64f,57,1.35f,82,1.60f},
        {"Szklany żniwiarz","Widmo mirażu",147,1.78f,28,8,6,DamageType::Physical,.54f,76,1.55f,115,1.95f},
        {"Ropucha torfowa","Płaz mokradeł",208,2.15f,34,9,7,DamageType::Physical,.76f,111,1.45f,66,1.5f},
        {"Pijawka denna","Pasożyt moczarów",176,2.85f,30,7,8,DamageType::Physical,.57f,104,1.02f,50,1.35f},
        {"Błędnik trzcinowy","Bagienny ognik",148,2.05f,36,5,17,DamageType::Magic,.49f,120,2.1f,97,3.6f},
        {"Dzwonnik Topieli","Strażnik dzwonnicy · boss",1210,1.38f,48,18,15,DamageType::Magic,1.2f,500,2.25f,175,2.65f}
    };
    static_assert(sizeof(entries)/sizeof(entries[0])==EnemyTypeCount);
    return entries[std::clamp(type,0,EnemyTypeCount-1)];
}
inline float enemySpeed(int type){return enemyInfo(type).speed;}
inline float damageAfterArmor(float amount,int defense){return std::max(2.f,amount-defense*.45f);}
struct PropInfo {const char* name;const char* kind;};
inline const PropInfo& propInfo(int type) {
    static constexpr PropInfo entries[]={
        {"Sosna","Drzewo iglaste"},
        {"Omszała skała","Skała"},
        {"Leśny krzew","Roślinność"},
        {"Kamienna kolumna","Ruina · budowla"},
        {"Brzoza","Drzewo liściaste"},
        {"Dąb","Drzewo liściaste"},
        {"Leśne grzyby","Grzyby"},
        {"Kaktus świecznikowy","Kaktus"},{"Opuncja kwitnąca","Kaktus"},
        {"Warstwowy piaskowiec","Skała pustynna"},{"Obelisk Słońca","Ruina pustynna"},
        {"Suchy krzew","Roślinność pustynna"},{"Palma oazy","Drzewo pustynne"},{"Brama Eldrina","Portal do wieży"},
        {"Bazaltowe formacje","Skała wulkaniczna"},{"Zwęglone pogorzelisko","Pogorzelisko"},
        {"Siarkowy wyziew","Wulkaniczny wyziew"},{"Obsydianowe formacje","Skała wulkaniczna"},
        {"Omszała kłoda","Martwe drewno"},{"Kępa paproci","Leśne runo"},
        {"Łuk opleciony bluszczem","Ruina · budowla"},{"Spróchniały pień","Martwe drewno"},{"Krzew jarzębiny","Roślinność"},
        {"Zagrzebane amfory","Ślad dawnej osady"},{"Żebra pustynnej bestii","Szczątki"},
        {"Rozeta agawy","Roślinność pustynna"},{"Płyta Słońca","Ruina pustynna"},{"Porzucony wóz","Ślad karawany"},
        {"Kryształy żaru","Skała wulkaniczna"},{"Zwęglone drzewko","Martwe drewno"},
        {"Skorupa siarki","Naciek wulkaniczny"},{"Kowadło obsydianowej kuźni","Ruina kuźni"},{"Pęknięta brama bazaltowa","Ruina · budowla"},
        {"Lawa poduszkowa","Skała wulkaniczna"},{"Zastygła kaskada","Zastygła lawa"},
        {"Skamieniały pancerz","Skamieniałość"},{"Latarnia górnicza","Ślad dawnej kopalni"},
        {"Wierzba rosochata","Drzewo mokradeł"},{"Pałka wodna","Roślinność szuwarowa"},
        {"Chata na palach","Zatopiona osada"},{"Niemy dzwon","Ruina dzwonnicy"},
        {"Łódź przewoźnika","Przystań"},{"Suszarnia sieci","Sprzęt rybacki"},
        {"Koło dawnego młyna","Ruina młyna"},{"Kapliczka wotywna","Miejsce pamięci"},
        {"Wiklinowy więcierz","Pułapka rybacka"},{"Wózek torfiarza","Wózek do przewozu torfu"}
    };
    static_assert(sizeof(entries)/sizeof(entries[0])==PropTypeCount);
    return entries[std::clamp(type,0,PropTypeCount-1)];
}
inline bool treeProp(int type){return type==0||type==4||type==5||type==12;}
inline float propRadius(int type){
    static constexpr float decorations[]={.65f,.32f,.74f,.49f,.35f,.42f,.60f,.32f,.54f,.75f,.56f,.49f,.46f,.59f,.73f,
                                          .78f,.91f,.69f,.34f,.68f,.25f,1.85f,1.2f,.67f,.54f,.79f,.45f,.47f,.83f};
    static_assert(sizeof(decorations)/sizeof(*decorations)==PropTypeCount-ForestDecorationStart);
    if(type>=ForestDecorationStart&&type<PropTypeCount)return decorations[type-ForestDecorationStart];
    return type==14?.82f:type==15?.45f:type==16?.75f:type==17?.62f:type==13?.78f:type==9?.65f:type==7||type==8?.58f:type==11?.28f:treeProp(type)?.52f:type==6?.27f:.4f;
}
}

namespace ember { inline bool enemyBoss(int type){return type==3||type==11||type==18||type==BellKeeper;} }
