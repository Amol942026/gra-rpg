#pragma once
#include <algorithm>

namespace ember {
constexpr int ForestEnemyTypeCount=8,ForestPropTypeCount=7,LegacyEnemyTypeCount=12,LegacyPropTypeCount=14,EnemyTypeCount=19,PropTypeCount=18;
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
    float cooldown=1.2f,visualHeight=86;
};
inline const EnemyInfo& enemyInfo(int type) {
    static constexpr EnemyInfo entries[]={
        {"Szlam","Śluzowiec",36,1.85f,9,0,0,DamageType::Physical,.48f,25,1.2f,45},
        {"Wilk","Bestia",54,2.6f,12,1,0,DamageType::Physical,.8f,35,1.2f,76},
        {"Szkielet","Nieumarły",72,1.85f,15,3,0,DamageType::Physical,.46f,45,1.2f,86},
        {"Kamienny Strażnik","Konstrukt · boss",260,1.35f,25,8,6,DamageType::Magic,.84f,120,1.5f,126},
        {"Dzik","Bestia",88,2.25f,15,4,1,DamageType::Physical,.8f,42,1.25f,69},
        {"Leśny pająk","Jadowita bestia",48,2.8f,13,1,3,DamageType::Magic,.86f,38,1.1f,55},
        {"Leśny rozbójnik","Człowiek",96,2.2f,18,4,2,DamageType::Physical,.48f,55,1.1f,92},
        {"Ożywione drzewo","Duch kniei",170,1.45f,21,6,5,DamageType::Physical,.94f,65,1.6f,164},
        {"Skorpion bursztynowy","Pustynny pajęczak",82,2.35f,18,3,3,DamageType::Magic,.85f,48,1.35f,93},
        {"Skarabeusz miedziany","Pustynny owad",96,2.1f,17,6,2,DamageType::Physical,.72f,52,1.2f,67},
        {"Kobra piaskowa","Pustynny gad",88,2.7f,21,2,4,DamageType::Magic,.64f,56,1.35f,112},
        {"Żywa Wydma","Duch piasku · boss",560,1.45f,31,10,7,DamageType::Magic,1.45f,220,1.8f,190},
        {"Żużlowy pełzacz","Pancernik kaldery",138,1.9f,26,10,5,DamageType::Physical,.68f,72,1.4f,66},
        {"Salamandra lawowa","Ognisty gad",120,2.9f,28,5,8,DamageType::Magic,.68f,76,1.1f,70},
        {"Ognisty chochlik","Demon żaru",104,2.35f,27,3,10,DamageType::Magic,.46f,82,2.1f,97},
        {"Popielny upiór","Duch pogorzeliska",135,1.75f,30,4,13,DamageType::Magic,.5f,90,2.5f,103},
        {"Obsydianowy wojownik","Strażnik kuźni",205,1.65f,33,14,6,DamageType::Physical,.62f,100,1.55f,109},
        {"Żarowy drak","Skrzydlata bestia",155,2.65f,31,7,9,DamageType::Magic,.83f,96,1.2f,90},
        {"Varkhor, Serce Kaldery","Władca wulkanu · boss",940,1.3f,42,17,12,DamageType::Magic,1.3f,390,1.85f,205}
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
        {"Siarkowy wyziew","Wulkaniczny wyziew"},{"Obsydianowe formacje","Skała wulkaniczna"}
    };
    static_assert(sizeof(entries)/sizeof(entries[0])==PropTypeCount);
    return entries[std::clamp(type,0,PropTypeCount-1)];
}
inline bool treeProp(int type){return type==0||type==4||type==5||type==12;}
inline float propRadius(int type){return type==14?.82f:type==15?.45f:type==16?.75f:type==17?.62f:type==13?.78f:type==9?.65f:type==7||type==8?.58f:type==11?.28f:treeProp(type)?.52f:type==6?.27f:.4f;}
}

namespace ember { inline bool enemyBoss(int type){return type==3||type==11||type==18;} }
