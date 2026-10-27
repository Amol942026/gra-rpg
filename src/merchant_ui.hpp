#pragma once
bool merchantBuying=true;int merchantOffer=0,merchantPage=0,merchantQuantity=1;
Slot merchantSale{};Item merchantSaleSnapshot{};std::string merchantNotice;
constexpr int MerchantRows=6;
InventoryClicks merchantClicks;
Rectangle merchantCloseRect(){return {1273,128,42,36};}
Rectangle merchantOfferRect(int row){return {137,248.f+row*50,544,46};}
Rectangle merchantBagRect(int index){return {737.f+(index%5)*112,328.f+(index/5)*47,103,40};}
Rectangle merchantEquipRect(int index){return {737.f+index*93,250,85,54};}
Rectangle merchantPageRect(bool next){return {next?555.f:138.f,557,125,32};}
Rectangle merchantMinusRect(){return {776,641,34,34};}
Rectangle merchantPlusRect(){return {871,641,34,34};}
Rectangle merchantMaxRect(){return {920,641,70,34};}
Rectangle merchantTradeRect(){return {1009,622,282,64};}
Item merchantSelectedItem(){
 if(merchantBuying)return merchantOfferItem(merchantOffer);
 Item* value=game.get(merchantSale);return value?*value:Item{};
}
int merchantQuantityLimit(){
 Item value=merchantSelectedItem();if(value.empty()||!stackable(value.kind))return 1;
 if(!merchantBuying)return std::max(1,value.count);
 return int(std::clamp(game.goldBalance()/std::max(1,itemBuyPrice(value)),std::int64_t(1),std::int64_t(value.kind==Kind::Arrow?std::max(1,game.merchantArrows):99)));
}
void syncMerchantSelection(){
 if(!merchantBuying){Item value=merchantSelectedItem();if(value.kind!=merchantSaleSnapshot.kind||value.tier!=merchantSaleSnapshot.tier){merchantSale={};merchantSaleSnapshot={};merchantQuantity=1;}}
 merchantQuantity=std::clamp(merchantQuantity,1,merchantQuantityLimit());
}
void openMerchantSelection(){merchantClicks.clear();merchantBuying=true;merchantOffer=merchantPage=0;merchantQuantity=1;merchantSale={};merchantSaleSnapshot={};merchantNotice.clear();}
void selectMerchantSale(Slot source){
 Item* value=game.get(source);if(!value||value->empty())return;
 merchantBuying=false;merchantSale=source;merchantSaleSnapshot=*value;merchantQuantity=1;merchantNotice.clear();
}
TradeResult selectedMerchantStatus(){
 syncMerchantSelection();
 if(!merchantBuying)return game.merchantSaleStatus(merchantSale,merchantQuantity);
 std::array<Item,BagSize> result{};std::int64_t wallet=0;return game.merchantPurchasePlan(merchantOffer,merchantQuantity,result,wallet);
}
void executeMerchantTrade(int quantity){
 TradeResult status=merchantBuying?game.buyMerchantItem(merchantOffer,quantity):game.sellMerchantItem(merchantSale,quantity);
 merchantClicks.clear();merchantNotice=game.message;if(status==TradeResult::Success)syncMerchantSelection();
}
void clickMerchantSale(Slot source,double now){
 Item* value=game.get(source);if(!value||value->empty()){merchantClicks.clear();return;}
 bool quick=merchantClicks.press(source,*value,now,mouse.x,mouse.y);selectMerchantSale(source);if(quick)executeMerchantTrade(1);
}
bool clickMerchantPanel(double now){
 syncMerchantSelection();
 if(inside(merchantCloseRect())){merchantClicks.clear();closeTowerPanel();return true;}
 for(bool next:{false,true})if(inside(merchantPageRect(next))){merchantClicks.clear();merchantPage=std::clamp(merchantPage+(next?1:-1),0,int(MerchantOffers.size()-1)/MerchantRows);return true;}
 for(int row=0;row<MerchantRows;row++)if(inside(merchantOfferRect(row))){
  int index=merchantPage*MerchantRows+row;if(index<int(MerchantOffers.size())){
   bool quick=merchantClicks.press({Place::None,index,-2},merchantOfferItem(index),now,mouse.x,mouse.y);
   merchantBuying=true;merchantOffer=index;merchantQuantity=1;merchantNotice.clear();if(quick)executeMerchantTrade(1);
  }return true;
 }
 for(int i=0;i<EquipSize;i++)if(inside(merchantEquipRect(i))){clickMerchantSale({Place::Equip,i},now);return true;}
 for(int i=0;i<BagSize;i++)if(inside(merchantBagRect(i))){clickMerchantSale({Place::Bag,i},now);return true;}
 merchantClicks.clear();
 if(inside(merchantMinusRect())){merchantQuantity=std::max(1,merchantQuantity-1);return true;}
 if(inside(merchantPlusRect())){merchantQuantity=std::min(merchantQuantityLimit(),merchantQuantity+1);return true;}
 if(inside(merchantMaxRect())){merchantQuantity=merchantQuantityLimit();return true;}
 if(inside(merchantTradeRect())){
  executeMerchantTrade(merchantQuantity);return true;
 }
 return true;
}
void merchantTextFit(const std::string& text,float x,float y,float width,float size,Color color){
 while(size>14&&textWidth(text,size)>width)size--;txt(text,x,y,size,color);
}
void drawMerchantSlot(Slot source,Rectangle r,const char* emptyLabel=""){
 Item* value=game.get(source);bool selected=!merchantBuying&&merchantSale.place==source.place&&merchantSale.index==source.index;
 panel(r,selected?rgb(46,57,49):inside(r)?Card:Panel);DrawRectangleLinesEx(r,selected?2:1,selected?Gold:Line);
 if(value&&!value->empty()){
  icon(*value,r.x+r.width*.5f,r.y+r.height*.5f-2,.59f);
  if(value->count>1)txt(std::to_string(value->count),r.x+r.width-27,r.y+r.height-16,14,Text);
  rect(r.x+5,r.y+r.height-3,r.width-10,1,rarity(*value));
 }else if(*emptyLabel)center(emptyLabel,r.x+r.width/2,r.y+19,14,Muted);
}
void drawMerchantPanel(){
 syncMerchantSelection();
 rect(0,0,W,H,rgb(5,12,16,220));panel({98,110,1244,666},rgb(19,29,31));
 rect(99,111,5,664,Gold);line(130,198,1310,198,Line);
 txt("Miron",134,133,31,Text,true);txt("HANDLARZ WIEŻY · PRACOWNIA ALCHEMICZNA",135,173,14,Gold);
 icon(item(Kind::Gold),1032,156,.65f);merchantTextFit(TextFormat("ZŁOTO  %lld",static_cast<long long>(game.goldBalance())),1061,149,198,20,Gold);
 panel(merchantCloseRect(),inside(merchantCloseRect())?Card:Panel);center("X",1294,135,19,Gold);
 txt("KUP OD MIRONA",138,219,17,Gold);txt("SPRZEDAJ Z PLECAKA LUB EKWIPUNKU",737,219,17,Gold);
 line(713,217,713,590,Line);
 for(int row=0;row<MerchantRows;row++){
  int index=merchantPage*MerchantRows+row;if(index>=int(MerchantOffers.size()))break;
  Item value=merchantOfferItem(index);Rectangle r=merchantOfferRect(row);bool selected=merchantBuying&&merchantOffer==index;
  panel(r,selected?rgb(47,54,42):inside(r)?Card:Panel);DrawRectangleLinesEx(r,selected?2:1,selected?Gold:Line);
  icon(value,r.x+28,r.y+23,.62f);std::string name=itemName(value)+(value.tier>0?" +"+std::to_string(value.tier):"");
  merchantTextFit(name,r.x+55,r.y+5,342,18,rarity(value));
  std::string detail=gearSlot(value.kind)>=0?std::string(gearSlot(value.kind)==0||gearSlot(value.kind)==5?"Atak +":"Pancerz +")+std::to_string(power(value)):
   value.kind==Kind::Health?"+50 HP":value.kind==Kind::Mana?"+35 many":"Amunicja do łuku";
  if(value.kind==Kind::Arrow)detail+=" · Zapas: "+std::to_string(game.merchantArrows)+" / 20";
  txt(detail+" · "+rarityName(itemRarity(value)),r.x+55,r.y+27,14,Muted);
  txt(std::to_string(itemBuyPrice(value))+" zł",r.x+r.width-90,r.y+16,16,Gold);
 }
 for(bool next:{false,true}){Rectangle r=merchantPageRect(next);panel(r,inside(r)?Card:Panel);center(next?"DALEJ >":"< WSTECZ",r.x+r.width/2,r.y+8,14,Gold);}
 center(TextFormat("OFERTA %d / %d",merchantPage+1,int(MerchantOffers.size()+MerchantRows-1)/MerchantRows),413,566,14,Muted);
 const char* slots[]={"BROŃ","HEŁM","TARCZA","ZBROJA","BUTY","BIŻUT."};
 for(int i=0;i<EquipSize;i++)drawMerchantSlot({Place::Equip,i},merchantEquipRect(i),slots[i]);
 txt("PLECAK · 2× LPM: sprzedaj jedną sztukę",737,311,14,Muted);
 for(int i=0;i<BagSize;i++)drawMerchantSlot({Place::Bag,i},merchantBagRect(i));
 txt("Skup: sprzęt, mikstury, amunicja i trofea bestii.",737,572,14,Muted);
 panel({129,603,1180,109},rgb(13,23,26));
 Item selected=merchantSelectedItem();TradeResult status=selectedMerchantStatus();
 if(!selected.empty()){
  icon(selected,157,647,.64f);
  std::string name=itemName(selected)+(selected.tier>0?" +"+std::to_string(selected.tier):"");
  merchantTextFit(name,191,617,550,20,rarity(selected));
  ItemTooltipText details=itemTooltipText(selected,merchantBuying?Slot{}:merchantSale);
  merchantTextFit(std::string(rarityName(itemRarity(selected)))+" · "+details.stat,191,647,550,14,Muted);
  merchantTextFit(details.value,191,673,550,14,Gold);
 }else txt("Wybierz towar lub przedmiot do sprzedaży.",152,639,18,Muted);
 txt("LICZBA SZTUK",776,616,14,Muted);
 for(auto button:{merchantMinusRect(),merchantPlusRect(),merchantMaxRect()})panel(button,inside(button)?Card:Panel);
 center("-",793,647,18,Gold);center(std::to_string(merchantQuantity),840,649,17,Text);center("+",888,647,18,Gold);center("MAX",955,650,14,Gold);
 Rectangle action=merchantTradeRect();bool available=status==TradeResult::Success;panel(action,available?(inside(action)?rgb(67,82,52):rgb(43,61,45)):rgb(29,37,39));
 DrawRectangleLinesEx(action,1,available?Gold:Line);center(merchantBuying?"KUP":"SPRZEDAJ",action.x+action.width/2,action.y+10,18,available?Text:Muted);
 std::int64_t amount=std::int64_t(merchantBuying?itemBuyPrice(selected):itemSellValue(selected))*merchantQuantity;
 center(std::to_string(amount)+" złota",action.x+action.width/2,action.y+35,16,available?Gold:Muted);
 std::string notice=!merchantNotice.empty()?merchantNotice:status!=TradeResult::Success?tradeMessage(status):"2× LPM: kup / sprzedaj jedną sztukę. Przyciski ilości pozwalają handlować całym stosem.";
 merchantTextFit(notice,139,724,1170,15,status==TradeResult::Success?Green:Gold);
 center("Gra jest zatrzymana · ENTER / ESC — zamknij · Monety w plecaku też służą do płacenia",720,751,14,Muted);

 // The same value/rarity descriptions apply to stock, backpack and worn gear.
 for(int row=0;row<MerchantRows;row++)if(inside(merchantOfferRect(row))){
  Item value=merchantOfferItem(merchantPage*MerchantRows+row);
  if(!value.empty())drawItemTooltip(value,{},"Cena zakupu: "+std::to_string(itemBuyPrice(value))+" złota / szt.");return;
 }
 for(int i=0;i<BagSize;i++)if(inside(merchantBagRect(i))){Item* value=game.get({Place::Bag,i});if(value&&!value->empty())drawItemTooltip(*value,{Place::Bag,i},value->kind==Kind::DawnShard?"Artefakt fabularny · nie na sprzedaż":"2× LPM: sprzedaj jedną sztukę.");return;}
 for(int i=0;i<EquipSize;i++)if(inside(merchantEquipRect(i))){Item* value=game.get({Place::Equip,i});if(value&&!value->empty())drawItemTooltip(*value,{Place::Equip,i},"2× LPM: sprzedaj noszony sprzęt.");return;}
}
