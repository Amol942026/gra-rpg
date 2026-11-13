#pragma once
#include "items.hpp"

namespace ember {
enum class PowerStyle { Sword,Axe,Dagger,Spear,Bow,Count };
struct PowerAttack {Kind weapon;float duration,contact,second,cooldown,reach,multiplier;};
inline constexpr float PowerHoldTime=.52f;
inline constexpr PowerAttack PowerAttacks[]={
 {Kind::Sword,.64f,.23f,0,.76f,2.15f,1.60f},
 {Kind::Axe,.74f,.31f,0,.89f,2.60f,1.85f},
 {Kind::Dagger,.60f,.18f,.38f,.72f,2.35f,.97f},
 {Kind::Spear,.68f,.26f,0,.81f,4.25f,1.60f},
 {Kind::Bow,.30f,0,0,.62f,1.15f,1.45f}
};
static_assert(std::size(PowerAttacks)==int(PowerStyle::Count));
inline PowerStyle powerStyle(Kind weapon){
 for(int i=0;i<int(PowerStyle::Count);i++)if(PowerAttacks[i].weapon==weapon)return PowerStyle(i);
 return PowerStyle::Count;
}
inline const PowerAttack& powerAttack(PowerStyle style){return PowerAttacks[int(style)];}
// Swing clocks use authored clip time, including in existing saves.
inline constexpr float powerAttackRate(PowerStyle style){
 switch(style){
 case PowerStyle::Sword:return 2.f;
 case PowerStyle::Axe:return 1.4f;
 default:return 1.f;
 }
}
}
