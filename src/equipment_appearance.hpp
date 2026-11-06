#pragma once
#include "items.hpp"
#include "generated/equipment_paints.hpp"
#include "generated/additional_weapon_paints.hpp"
#include <algorithm>
namespace ember {
inline constexpr int FirstAdditionalWeapon=int(Kind::TrailFalchion);
static_assert(std::size(equipment_paint_data::paints)==FirstAdditionalWeapon
           &&std::size(equipment_paint_data::spans)==FirstAdditionalWeapon);
static_assert(std::size(additional_weapon_paint_data::paints)==ItemKindCount-FirstAdditionalWeapon
           &&std::size(additional_weapon_paint_data::spans)==ItemKindCount-FirstAdditionalWeapon,
              "Every new weapon needs an authored appearance entry");
struct GearInlay {
 const equipment_paint_data::Stroke* first;
 const equipment_paint_data::Stroke* last;
 auto begin()const{return first;}
 auto end()const{return last;}
};
inline GearInlay equipmentInlay(Kind kind){
 int index=int(kind);
 if(index<0||index>=ItemKindCount)return {nullptr,nullptr};
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
 auto paint=index>=FirstAdditionalWeapon?additional_weapon_paint_data::paints[index-FirstAdditionalWeapon]:equipment_paint_data::paints[index];
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
