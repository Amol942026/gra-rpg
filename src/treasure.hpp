#pragma once
// Included after V and WorldLocation. Stable IDs keep loot with its realm.
constexpr int TreasureCount=4,ChestLootSize=3;
constexpr float ChestUseRange=2.5f,ChestRadius=.65f;
struct TreasureChest {
 int id;V pos;WorldLocation location;bool opened=false;
 std::array<Item,ChestLootSize> loot{};
};
inline constexpr std::array<TreasureChest,TreasureCount> TreasureLayout{{
 {0,{12,41},WorldLocation::Forest},
 {1,{57,57},WorldLocation::Forest},
 {2,{23,43},WorldLocation::Desert},
 {3,{44,15},WorldLocation::Desert}
}};
inline bool nearTreasure(V p,WorldLocation location,float radius=2.4f){
 for(const auto& chest:TreasureLayout)if(chest.location==location&&distance(p,chest.pos)<radius)return true;
 return false;
}
inline std::array<TreasureChest,TreasureCount> makeTreasure(unsigned seed){
 auto result=TreasureLayout;
 // Separate stream: opening a chest never changes combat, enemy loot or plants.
 std::mt19937 random(seed^0x736b727au);
 constexpr Kind common[]={Kind::Sword,Kind::Bow,Kind::Armor,Kind::Shield,Kind::Boots};
 constexpr Kind rare[]={Kind::Axe,Kind::Helmet,Kind::Dagger,Kind::Cloak,Kind::Amulet,
  Kind::WolfKnife,Kind::WolfCloak,Kind::BoarArmor,Kind::SilkBoots,Kind::WebCowl,
  Kind::BoneHelmet,Kind::BanditBow,Kind::BanditBoots,Kind::RootShield,Kind::GelAmulet,
  Kind::GuardianHelmet,Kind::StingDagger,Kind::ScarabHelmet,Kind::CobraBoots};
 for(auto& chest:result)for(int slot=0;slot<ChestLootSize;slot++){
  bool uncommon=std::uniform_int_distribution<int>(0,99)(random)<25;
  const Kind* pool=uncommon?rare:common;int size=uncommon?int(std::size(rare)):int(std::size(common));
  int start=std::uniform_int_distribution<int>(0,size-1)(random);
  for(int step=0;step<size;step++){
   Kind kind=pool[(start+step)%size];bool duplicate=false;
   for(int previous=0;previous<slot;previous++)duplicate|=chest.loot[previous].kind==kind;
   if(!duplicate){chest.loot[slot]=item(kind);break;}
  }
 }
 return result;
}
