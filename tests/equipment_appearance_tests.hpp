#pragma once
#include "../src/equipment_appearance.hpp"
void equipmentAppearanceTests(){
 int kinds=0;bool slots[6]{};
 for(int k=1;k<ItemKindCount;k++){
  Kind kind=Kind(k);int slot=gearSlot(kind);if(slot<0)continue;kinds++;slots[slot]=true;
  Game g=emptyArena();g.bag[10]=item(kind,2);
  check(g.transfer({Place::Bag,10},{Place::Equip,slot}),"every gear kind can be equipped through the real inventory transfer");
  auto appearance=heroAppearance(g.equipment);
  check(appearance[slot].value.kind==kind&&appearance[slot].value.tier==2&&appearance[slot].shape==archetype(kind),"visual equipment retains exact kind and tier instead of only its archetype");
  auto look=gearAppearance(item(kind));auto upgraded=gearAppearance(item(kind,2));
  check(look.paint.primary&&look.paint.accent&&equipment_paint_data::spans[k].count>0,"every equippable item has authored material colors and a real Krita inlay");
  check(look.paint.trim!=upgraded.paint.trim,"upgraded equipment also has a visible trim change");
  check(g.transfer({Place::Equip,slot},{Place::Bag,11})&&!heroAppearance(g.equipment)[slot].present(),"removing gear immediately clears its visual slot");
 }
 check(kinds==39,"all thirty-nine current gear variants are covered");for(bool slot:slots)check(slot,"each of the six equipment slots drives appearance");
 std::array<Item,6> wrong{};wrong[0]=item(Kind::Armor);wrong[1]=item(Kind::Health);wrong[3]=item(Kind::Boots);
 auto rejected=heroAppearance(wrong);check(!rejected[0].present()&&!rejected[1].present()&&!rejected[3].present(),"invalid slot contents cannot create phantom visual equipment");
 check(!gearAppearance({Kind::Sword,0,0}).present()&&!gearAppearance(item(Kind::None)).present(),"empty slots never display a default sword");
}
