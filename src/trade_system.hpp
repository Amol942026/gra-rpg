#pragma once
// Included inside Game. Validate a complete transaction before changing state.
bool merchantReachable()const{return towerUseAllowed()&&towerFloor==MerchantFloor&&distance(player,MerchantPosition)<=TowerUseRange;}
std::int64_t goldBalance()const{
 std::int64_t balance=gold;for(const auto& value:bag)if(!value.empty()&&value.kind==Kind::Gold)balance+=value.count;return balance;
}
TradeResult merchantPurchasePlan(int index,int quantity,std::array<Item,BagSize>& result,std::int64_t& wallet)const{
 if(!merchantReachable())return TradeResult::Unavailable;
 Item offer=merchantOfferItem(index);if(offer.empty())return TradeResult::InvalidItem;
 if(quantity<1||quantity>99||(!stackable(offer.kind)&&quantity!=1))return TradeResult::InvalidQuantity;
 std::int64_t cost=std::int64_t(itemBuyPrice(offer))*quantity;
 if(cost>goldBalance())return TradeResult::NotEnoughGold;
 result=bag;std::int64_t fromWallet=std::min(gold,cost);wallet=gold-fromWallet;cost-=fromWallet;
 for(auto& value:result)if(value.kind==Kind::Gold&&!value.empty()&&cost>0){
  int used=int(std::min(cost,std::int64_t(value.count)));value.count-=used;cost-=used;if(value.empty())value={};
 }
 int remaining=quantity;
 if(stackable(offer.kind))for(auto& value:result)if(value.kind==offer.kind&&value.tier==offer.tier&&!value.empty()&&value.count<99){
  int added=std::min(remaining,99-value.count);value.count+=added;remaining-=added;
 }
 for(auto& value:result)if(value.empty()&&remaining>0){int added=stackable(offer.kind)?std::min(remaining,99):1;value=item(offer.kind,offer.tier,added);remaining-=added;}
 return remaining>0?TradeResult::NoSpace:TradeResult::Success;
}
TradeResult buyMerchantItem(int index,int quantity=1){
 std::array<Item,BagSize> result{};std::int64_t wallet=0;TradeResult status=merchantPurchasePlan(index,quantity,result,wallet);
 if(status!=TradeResult::Success){say(tradeMessage(status));sound(Cue::Reject,player);return status;}
 Item bought=merchantOfferItem(index);bag=result;gold=wallet;cancelBow();sound(Cue::Pickup,player);
 say("Kupiono: "+itemName(bought)+" × "+std::to_string(quantity)+".");return status;
}
TradeResult merchantSaleStatus(Slot source,int quantity)const{
 if(!merchantReachable())return TradeResult::Unavailable;
 const Item* value=source.place==Place::Bag&&source.index>=0&&source.index<BagSize?&bag[source.index]:
  source.place==Place::Equip&&source.index>=0&&source.index<EquipSize?&equipment[source.index]:nullptr;
 if(!value||value->empty())return TradeResult::InvalidItem;
 if(value->kind==Kind::DawnShard)return TradeResult::Protected;
 if(!merchantSellable(*value))return TradeResult::InvalidItem;
 if(quantity<1||quantity>value->count||(!stackable(value->kind)&&quantity!=1))return TradeResult::InvalidQuantity;
 std::int64_t gain=std::int64_t(itemSellValue(*value))*quantity;
 if(goldBalance()+gain-(value->kind==Kind::Gold?quantity:0)>GoldLimit)return TradeResult::GoldLimit;
 return TradeResult::Success;
}
TradeResult sellMerchantItem(Slot source,int quantity=1){
 TradeResult status=merchantSaleStatus(source,quantity);
 if(status!=TradeResult::Success){say(tradeMessage(status));sound(Cue::Reject,player);return status;}
 Item* value=get(source);std::string name=itemName(*value);
 gold+=std::int64_t(itemSellValue(*value))*quantity;
 value->count-=quantity;if(value->empty())*value={};cancelBow();sound(Cue::Pickup,player);
 say("Sprzedano: "+name+" × "+std::to_string(quantity)+".");return status;
}
