#pragma once
#include <algorithm>

namespace ember {
constexpr int EnemyTypeCount=8,PropTypeCount=7;
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
        {"Strażnik ruin","Konstrukt · boss",260,1.35f,25,8,6,DamageType::Magic,.84f,120,1.5f,126},
        {"Dzik","Bestia",88,2.25f,15,4,1,DamageType::Physical,.8f,42,1.25f,69},
        {"Leśny pająk","Jadowita bestia",48,2.8f,13,1,3,DamageType::Magic,.86f,38,1.1f,55},
        {"Leśny rozbójnik","Człowiek",96,2.2f,18,4,2,DamageType::Physical,.48f,55,1.1f,92},
        {"Ożywione drzewo","Duch kniei",170,1.45f,21,6,5,DamageType::Physical,.94f,65,1.6f,164}
    };
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
        {"Leśne grzyby","Grzyby"}
    };
    return entries[std::clamp(type,0,PropTypeCount-1)];
}
inline bool treeProp(int type){return type==0||type==4||type==5;}
inline float propRadius(int type){return treeProp(type)?.52f:type==6?.27f:.4f;}
}
