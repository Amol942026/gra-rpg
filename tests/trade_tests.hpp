#pragma once
void tradeTests(){
 auto trader=[](){Game g;g.enterMageTower();g.towerFloor=MerchantFloor;g.dawnQuest=DawnQuest::AtTower;g.player=MerchantPosition+V{-1,0};return g;};
 auto same=[](const auto& a,const auto& b){for(size_t i=0;i<a.size();i++)if(a[i].kind!=b[i].kind||a[i].tier!=b[i].tier||a[i].count!=b[i].count)return false;return true;};
 Game g=trader();
 check(g.merchantReachable()&&g.walkable(g.player),"merchant has a walkable approach on the alchemy floor");
 check(!g.walkable(MerchantPosition),"trader and stall have physical collision");
 int traderIndex=-1;for(int i=0;i<int(towerFurnishings(MerchantFloor).size());i++)if(towerFurnishings(MerchantFloor)[i].type==MerchantObjectType)traderIndex=i;
 check(traderIndex>=0&&g.towerObjectReachable(traderIndex),"merchant is an interactive tower object");
 check(Game{}.goldBalance()==0,"new character receives no unrequested starting gold");
 check(itemSellValue(item(Kind::Gold))==1&&itemSellValue(item(Kind::Arrow))==1,"currency and arrows have one-gold unit value");
 check(itemSellValue({})==0&&itemSellValue(item(Kind::DawnShard))==0&&!merchantSellable(item(Kind::DawnShard)),"story shards cannot be exchanged for money");
 for(int kind=1;kind<ItemKindCount;kind++){
  Item value=item(Kind(kind),0,1);
  check(itemSellValue(value)>0||value.kind==Kind::DawnShard,"every ordinary item has a positive merchant value");
  if(value.kind==Kind::DawnShard)continue;
  check(itemSellValue(item(value.kind,0,7))==itemSellValue(value)&&itemStackValue(item(value.kind,0,7))==7LL*itemSellValue(value),"unit value is independent of stack size");
  if(gearSlot(value.kind)>=0)check(itemSellValue(item(value.kind,1))>itemSellValue(value),"upgraded equipment is worth more than its ordinary form");
  g=trader();g.bag[10]=value;auto before=g.goldBalance();
  check(g.sellMerchantItem({Place::Bag,10})==TradeResult::Success&&g.bag[10].empty()&&g.goldBalance()==before+itemSellValue(value)-(value.kind==Kind::Gold?1:0),"merchant buys each equipment and monster material kind at its displayed value");
 }
 for(int index=0;index<int(MerchantOffers.size());index++){
  g=trader();g.bag={};g.gold=100000;Item offered=merchantOfferItem(index);auto money=g.goldBalance();
  check(g.buyMerchantItem(index)==TradeResult::Success&&g.bagCount(offered.kind)==1&&g.goldBalance()==money-itemBuyPrice(offered),"each stocked item can be bought for its quoted price");
  check(itemBuyPrice(offered)>itemSellValue(offered),"buying and immediately reselling cannot create gold");
  auto spent=g.goldBalance();check(g.sellMerchantItem({Place::Bag,0})==TradeResult::Success&&g.goldBalance()<money&&g.goldBalance()==spent+itemSellValue(offered),"round trip through the merchant conserves inventory and loses only the trading margin");
  if(offered.tier>0)check(power(offered)>power(item(offered.kind)),"stock upgrades improve equipment over the starting item");
 }
 g=trader();g.bag[10]=item(Kind::CobraScale,0,12);
 check(g.sellMerchantItem({Place::Bag,10},4)==TradeResult::Success&&g.bag[10].count==8&&g.gold==4*itemSellValue(item(Kind::CobraScale)),"partial cobra-scale sale preserves the remainder and pays exact unit value");
 check(g.sellMerchantItem({Place::Bag,10},8)==TradeResult::Success&&g.bag[10].empty(),"whole-stack sale clears only the sold slot");
 auto beforeGold=g.gold;check(g.sellMerchantItem({Place::Bag,10})==TradeResult::InvalidItem&&g.gold==beforeGold,"repeated sale of an empty slot cannot duplicate gold");
 g=trader();g.bag[10]=item(Kind::DawnShard,3);auto old=g.bag;
 check(g.sellMerchantItem({Place::Bag,10})==TradeResult::Protected&&same(g.bag,old)&&g.gold==0,"quest fragment remains protected after selection");
 g=trader();auto worn=g.equipment[0];g.weaponWheel[0]={worn.kind,worn.tier};
 check(g.sellMerchantItem({Place::Equip,0})==TradeResult::Success&&g.equipment[0].empty()&&g.gold==itemSellValue(worn),"worn equipment can be sold and its stats are removed");
 check(g.weaponSource(0).place==Place::None,"selling a bound weapon cannot leave a usable ghost wheel entry");
 g=trader();g.bag={};g.gold=10;g.bag[0]=item(Kind::Gold,0,30);
 check(g.goldBalance()==40&&g.buyMerchantItem(0)==TradeResult::Success&&g.goldBalance()==16&&g.gold==0&&g.bagCount(Kind::Gold)==16&&g.bagCount(Kind::Health)==1,"buying pays from wallet and then existing coin stacks");
 g=trader();g.bag={};for(auto& value:g.bag)value=item(Kind::Crystal,0,99);g.bag[0]=item(Kind::Gold,0,itemBuyPrice(merchantOfferItem(0)));
 check(g.buyMerchantItem(0)==TradeResult::Success&&g.bag[0].kind==Kind::Health,"spending an entire coin stack can free the slot needed by a purchased item");
 g=trader();g.bag={};g.gold=10000;g.bag[0]=item(Kind::Health,0,98);
 check(g.buyMerchantItem(0,3)==TradeResult::Success&&g.bag[0].count==99&&g.bag[1].kind==Kind::Health&&g.bag[1].count==2,"purchased potions fill the old stack and spill into a new slot without exceeding 99");
 g=trader();g.bag[10]=item(Kind::Gold,0,20);auto balance=g.goldBalance();
 check(g.sellMerchantItem({Place::Bag,10},20)==TradeResult::Success&&g.goldBalance()==balance&&g.bag[10].empty(),"exchanging bag coins for wallet gold does not create extra currency");
 g=trader();g.gold=0;old=g.bag;
 check(g.buyMerchantItem(0)==TradeResult::NotEnoughGold&&same(old,g.bag)&&g.gold==0,"insufficient gold leaves all inventory and currency unchanged");
 g=trader();for(auto& value:g.bag)value=item(Kind::Health,0,99);g.gold=10000;old=g.bag;beforeGold=g.gold;
 check(g.buyMerchantItem(0)==TradeResult::NoSpace&&same(old,g.bag)&&g.gold==beforeGold,"full bag and full matching stacks cannot consume gold");
 for(int quantity:{0,-1,100}){
  check(g.buyMerchantItem(0,quantity)==TradeResult::InvalidQuantity&&same(old,g.bag)&&g.gold==beforeGold,"invalid purchase quantity leaves state untouched");
  check(g.sellMerchantItem({Place::Bag,0},quantity)==TradeResult::InvalidQuantity&&same(old,g.bag)&&g.gold==beforeGold,"invalid sale quantity leaves state untouched");
 }
 check(g.buyMerchantItem(-1)==TradeResult::InvalidItem&&g.buyMerchantItem(999)==TradeResult::InvalidItem,"invalid stock indexes are rejected");
 check(g.buyMerchantItem(3,2)==TradeResult::InvalidQuantity,"nonstackable equipment cannot be bought as a stack");
 for(Place place:{Place::None,Place::Ground,Place::Corpse,Place::Forage})check(g.sellMerchantItem({place,0})==TradeResult::InvalidItem,"remote loot cannot be sold directly without collecting it");
 g=trader();g.gold=GoldLimit;g.bag[10]=item(Kind::WolfFang);old=g.bag;
 check(g.sellMerchantItem({Place::Bag,10})==TradeResult::GoldLimit&&same(old,g.bag)&&g.gold==GoldLimit,"wallet limit cannot destroy the item or overflow the gold balance");
 for(int state=0;state<6;state++){
  g=trader();g.gold=1000;old=g.bag;auto gear=g.equipment;
  if(state==0)g.location=WorldLocation::Forest;if(state==1)g.towerFloor=0;if(state==2)g.player=TowerCenter;
  if(state==3)g.hp=0;if(state==4)g.allocationRequired=true;if(state==5)g.dawnQuest=DawnQuest::Returning;
  check(g.buyMerchantItem(0)==TradeResult::Unavailable&&g.sellMerchantItem({Place::Bag,0})==TradeResult::Unavailable&&same(old,g.bag)&&same(gear,g.equipment)&&g.gold==1000,"wrong location, distance, death and locked story states cannot trade");
 }
 g=trader();g.gold=321;g.player=TowerStairDown;check(g.climbTower(-1)&&g.gold==321,"gold survives ordinary floor changes");
}
