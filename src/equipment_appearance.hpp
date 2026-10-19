#pragma once
#include "items.hpp"
#include "generated/equipment_paints.hpp"
#include <algorithm>
namespace ember {
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
 auto paint=equipment_paint_data::paints[int(value.kind)];
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
