#pragma once
#include "items.hpp"
#include "equipment_shapes.hpp"
#include "generated/equipment_paints.hpp"
#include "generated/additional_weapon_paints.hpp"
#include <algorithm>
namespace ember {
inline constexpr int FirstAdditionalWeapon=int(Kind::TrailFalchion);
static_assert(std::size(equipment_paint_data::paints)==FirstAdditionalWeapon
           &&std::size(equipment_paint_data::spans)==FirstAdditionalWeapon);
static_assert(std::size(additional_weapon_paint_data::paints)==FirstAdditionalArmor-FirstAdditionalWeapon
           &&std::size(additional_weapon_paint_data::spans)==FirstAdditionalArmor-FirstAdditionalWeapon,
              "Every new weapon needs an authored appearance entry");
inline constexpr std::array<equipment_paint_data::Paint,10> armorPaints{{
 {0x97805a,0x4e6043,0x67784b,0xd9c093},
 {0x486d6a,0x303f45,0x90648d,0xc6acc8},
 {0x74543a,0x394935,0x768548,0xe2ad5f},
 {0xbc935b,0xafa17f,0xd6bc78,0xf3d7a1},
 {0xb06e35,0x55372e,0xdd9f50,0xffc879},
 {0xc8af75,0xbab39b,0xe5ca83,0x72c2b3},
 {0x884849,0x42454d,0x8d9796,0xb3b9ad},
 {0x8a4b3d,0x4c3031,0xa27450,0xd8c59e},
 {0x454454,0x302f38,0xb37d52,0xeb9c60},
 {0x524065,0x392634,0x8b5777,0xd49dff}
}};
inline constexpr equipment_paint_data::Stroke armorInlays[][3]={
 {{4,6,2},{6,7,2},{8,8,2}},{{5,5,2},{8,7,2},{10,9,2}},
 {{8,5,1},{7,6,3},{8,7,1}},{{5,6,6},{5,8,6},{5,10,6}},
 {{6,6,4},{7,8,2},{7,10,2}},{{8,5,1},{6,7,5},{8,9,1}},
 {{5,5,1},{8,7,1},{11,9,1}},{{5,5,2},{6,6,2},{7,7,2}},
 {{8,4,1},{7,5,2},{6,6,2}},{{7,5,3},{6,6,5},{7,7,3}}
};
static_assert(armorPaints.size()==AdditionalArmorCount&&std::size(armorInlays)==AdditionalArmorCount);
struct RealmWeaponPaint {
 equipment_paint_data::Paint colors;
 equipment_paint_data::Stroke motif[3];
};
inline constexpr std::array<RealmWeaponPaint,RealmWeaponCount> realmWeaponPaints{{
 {{0xa5b3a3,0x405841,0xa79665,0xd2dcb3},{{8,4,1},{7,5,2},{6,6,2}}},
 {{0x6a527d,0x344f4b,0x8e829d,0xd8a5ed},{{7,5,1},{8,7,1},{9,9,1}}},
 {{0x687772,0x5f4634,0xa88c57,0xa4cf83},{{8,4,1},{6,6,5},{8,8,1}}},
 {{0xb89559,0x796b4d,0xd6c28c,0xf0deae},{{6,5,1},{8,7,1},{10,9,1}}},
 {{0xcbb77d,0x3a6261,0xddbe6e,0x9fe1cb},{{9,3,2},{8,4,2},{7,5,2}}},
 {{0xe4a551,0x453636,0xa7874f,0xffe3a2},{{9,4,1},{8,6,2},{7,8,1}}},
 {{0x666171,0x653d32,0xb57550,0xd89869},{{6,5,2},{8,6,1},{10,7,2}}},
 {{0x98584b,0x4d353b,0xa78967,0xe7c898},{{7,4,2},{8,7,2},{9,10,2}}},
 {{0x63596d,0x58413d,0xb98153,0xffa053},{{8,4,1},{8,6,1},{8,8,1}}},
 {{0x685278,0x4b3049,0x9c6683,0xe2b1fa},{{8,6,1},{7,7,3},{8,8,1}}}
}};
inline constexpr std::array<RealmWeaponPaint,SpearCount> spearPaints{{
 {{0xaabcb8,0x8f6544,0xb59d73,0xe2e7ce},{{8,4,1},{8,6,1},{8,8,1}}},
 {{0x628969,0x4f5a39,0x99845c,0xc7d98c},{{7,4,1},{6,5,3},{8,7,1}}},
 {{0xd6a754,0x749082,0xe1be79,0xffedb1},{{8,3,1},{7,5,3},{8,7,1}}},
 {{0x625a68,0x684736,0xb47958,0xf7a050},{{7,4,1},{8,6,1},{7,8,1}}},
 {{0xd6cfac,0x665275,0xddbd76,0x92dfcf},{{8,4,1},{7,5,3},{8,6,1}}}
}};
struct GearInlay {
 const equipment_paint_data::Stroke* first;
 const equipment_paint_data::Stroke* last;
 auto begin()const{return first;}
 auto end()const{return last;}
};
inline GearInlay equipmentInlay(Kind kind){
 int index=int(kind);
 if(index<0||index>=ItemKindCount)return {nullptr,nullptr};
 if(additionalArmor(kind)){
  const auto& strokes=armorInlays[index-FirstAdditionalArmor];
  return {std::begin(strokes),std::end(strokes)};
 }
 if(realmWeapon(kind)){
  const auto& strokes=realmWeaponPaints[index-FirstRealmWeapon].motif;
  return {std::begin(strokes),std::end(strokes)};
 }
 if(spearVariant(kind)){
  const auto& strokes=spearPaints[index-FirstSpear].motif;
  return {std::begin(strokes),std::end(strokes)};
 }
 const bool added=index>=FirstAdditionalWeapon;
 auto span=added?additional_weapon_paint_data::spans[index-FirstAdditionalWeapon]:equipment_paint_data::spans[index];
 auto first=(added?additional_weapon_paint_data::strokes:equipment_paint_data::strokes)+span.start;
 return {first,first+span.count};
}
struct GearAppearance {
 Item value{};Kind shape=Kind::None;
 equipment_paint_data::Paint paint{};
 bool present()const{return !value.empty();}
};
inline unsigned tierPaint(unsigned color,int tier){
 int lift=std::clamp(tier,0,4)*7;
 return (unsigned(std::min(255,int((color>>16)&255)+lift))<<16)|
        (unsigned(std::min(255,int((color>>8)&255)+lift))<<8)|unsigned(std::min(255,int(color&255)+lift));
}
inline GearAppearance gearAppearance(Item value){
 if(value.empty()||int(value.kind)<0||int(value.kind)>=ItemKindCount||gearSlot(value.kind)<0)return {};
 const int index=int(value.kind);
 auto paint=spearVariant(value.kind)?spearPaints[index-FirstSpear].colors:
            realmWeapon(value.kind)?realmWeaponPaints[index-FirstRealmWeapon].colors:
            additionalArmor(value.kind)?armorPaints[index-FirstAdditionalArmor]:
            index>=FirstAdditionalWeapon?additional_weapon_paint_data::paints[index-FirstAdditionalWeapon]:equipment_paint_data::paints[index];
 paint.trim=tierPaint(paint.trim,value.tier);
 return {value,archetype(value.kind),paint};
}
struct HeroAppearance {
 std::array<GearAppearance,6> slots{};bool explicitGear=false;
 const GearAppearance& operator[](int slot)const{return slots[slot];}
};
inline HeroAppearance heroAppearance(const std::array<Item,6>& equipment){
 HeroAppearance result;result.explicitGear=true;
 for(int i=0;i<6;i++){auto look=gearAppearance(equipment[i]);if(look.present()&&gearSlot(look.value.kind)==i)result.slots[i]=look;}
 return result;
}
inline HeroAppearance legacyHeroAppearance(bool helmet,bool shield,Kind weapon){
 auto result=heroAppearance({item(weapon),helmet?item(Kind::Helmet):Item{},shield?item(Kind::Shield):Item{},item(Kind::Armor),item(Kind::Boots),Item{}});
 result.explicitGear=false;return result;
}
}
