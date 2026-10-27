#pragma once
#include "items.hpp"
#include <cstdint>
#include <algorithm>
namespace ember {
inline constexpr std::int64_t GoldLimit=999999999;
inline constexpr int MerchantArrowCapacity=20;
inline bool merchantSellable(Item value){
 return !value.empty()&&value.kind>Kind::None&&value.kind<Kind::Count&&value.kind!=Kind::DawnShard;
}
// Values are per unit; stack size never changes a unit's price.
inline int itemSellValue(Item value){
 if(value.empty()||value.kind<=Kind::None||value.kind>=Kind::Count||value.kind==Kind::DawnShard)return 0;
 if(value.kind==Kind::Gold||value.kind==Kind::Arrow)return 1;
 int slot=gearSlot(value.kind),base=0;
 if(slot>=0){constexpr int gearBase[]={40,30,35,55,25,45};base=gearBase[slot]+std::max(0,power(value))*4;}
 else switch(value.kind){
  case Kind::Health:base=12;break;case Kind::Mana:base=14;break;case Kind::Mushroom:base=3;break;
  case Kind::Crystal:base=20;break;case Kind::WolfFang:base=8;break;case Kind::WolfFur:base=6;break;
  case Kind::BoarTusk:base=10;break;case Kind::BoarHide:base=9;break;case Kind::SpiderSilk:base=7;break;
  case Kind::VenomSac:base=12;break;case Kind::BoneShard:base=4;break;case Kind::SlimeGel:base=4;break;
  case Kind::AncientBark:base=10;break;case Kind::ForestEssence:base=16;break;case Kind::BanditBadge:base=12;break;
  case Kind::RuneStone:base=24;break;case Kind::ScorpionSting:base=12;break;case Kind::ScarabShell:base=14;break;
  case Kind::CobraScale:base=10;break;case Kind::SandEssence:base=30;break;default:base=1;break;
 }
 constexpr int multiplier[]={1,2,4,8};
 return base*multiplier[int(itemRarity(value))];
}
inline std::int64_t itemStackValue(Item value){return std::int64_t(itemSellValue(value))*std::max(0,value.count);}
struct MerchantOffer {Kind kind;int tier;};
inline constexpr std::array<MerchantOffer,12> MerchantOffers{{
 {Kind::Health,0},{Kind::Mana,0},{Kind::Arrow,0},
 {Kind::Sword,1},{Kind::Bow,1},{Kind::Armor,1},
 {Kind::Helmet,1},{Kind::Shield,1},{Kind::Boots,1},
 {Kind::WolfCloak,0},{Kind::GelAmulet,0},{Kind::BanditBow,0}
}};
inline Item merchantOfferItem(int index){return index>=0&&index<int(MerchantOffers.size())?item(MerchantOffers[index].kind,MerchantOffers[index].tier):Item{};}
inline int itemBuyPrice(Item value){return itemSellValue(value)*2;}
enum class TradeResult {Success,Unavailable,InvalidItem,InvalidQuantity,NotEnoughGold,NoSpace,GoldLimit,Protected,OutOfStock};
inline const char* tradeMessage(TradeResult result){switch(result){
 case TradeResult::Success:return "Transakcja zakończona.";
 case TradeResult::Unavailable:return "Podejdź do Mirona na 3. piętrze wieży.";
 case TradeResult::InvalidItem:return "Wybierz przedmiot z plecaka lub ekwipunku.";
 case TradeResult::InvalidQuantity:return "Wybierz prawidłową liczbę sztuk.";
 case TradeResult::NotEnoughGold:return "Nie masz wystarczająco dużo złota.";
 case TradeResult::NoSpace:return "Zwolnij miejsce w plecaku lub pasującym stosie.";
 case TradeResult::GoldLimit:return "Nie możesz pomieścić więcej złota.";
 case TradeResult::OutOfStock:return "Brak strzał w zapasie. Dostawa po pokonaniu Żywej Wydmy.";
 case TradeResult::Protected:return "Fragment Serca Świtu nie jest na sprzedaż.";
 }return "";
}
}
