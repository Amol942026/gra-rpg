#pragma once
bool runTradeQA(){
 fs::create_directories("qa-v42");std::ofstream report("qa-v42/trade-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 auto point=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
 auto reset=[](){game=Game{};intro={};towerStory={};clearStoryMenus();developerActive=developerMenuOpen=false;game.enterMageTower();game.towerFloor=MerchantFloor;game.dawnQuest=DawnQuest::AtTower;game.player=MerchantPosition+V{-1,0};game.gold=1000;game.messageTime=0;mouse={0,0};};
 auto shot=[](const char* name){renderScene();saveTexture(canvas,std::string("qa-v42/")+name+".png");};
 reset();refreshDisplayLayout();check(!audioSystem.available,"trading graphics QA opens no audio device");
 int index=-1;const auto& objects=towerFurnishings(MerchantFloor);for(int i=0;i<int(objects.size());i++)if(objects[i].type==MerchantObjectType)index=i;
 check(index>=0&&game.merchantReachable(),"trader exists and can be reached on the alchemy floor");
 Vector2 base=screen(MerchantPosition);bool picked=false;
 for(int y=-100;y<15&&!picked;y+=3)for(int x=-50;x<50&&!picked;x+=3){mouse={base.x+x,base.y+y};WorldHover hit=pickWorld(mouse);if(hit.kind==HoverKind::TowerObject&&hit.index==index)picked=true;}
 check(picked,"actual opaque merchant silhouette can be selected by mouse");
 WorldHover hit=pickWorld(mouse);Inspection info=inspectWorld(hit);
 check(info.name.find("Miron")!=std::string::npos&&info.kind.find("Handlarz")!=std::string::npos,"hover HUD identifies the friendly trader");
 mouse={0,0};shot("merchant-in-tower");
 check(nearestTowerInteraction()&&towerPanelOpen&&towerPanelObject==index,"E interaction opens the actual merchant HUD");
 check(!gameplayInputAllowed()&&!worldPointerAllowed(base),"merchant HUD pauses gameplay and blocks world clicks");
 check(!castGameplaySpell()&&!clickWorldOrInventory()&&!rightClickWorldOrInventory(),"trading cannot leak spells, sword attacks or looting into the world");

 auto& sprite=tower_furniture::sprites[tower_furniture::MerchantSprite];
 Image im=LoadImageFromTexture(sprite.art.texture);Color* pixels=LoadImageColors(im);int solid=0,clipped=0;
 for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++)if(pixels[y*im.width+x].a>210){solid++;clipped+=x<3||y<3||x>=im.width-3||y>=im.height-3;}
 check(solid>3000&&clipped==0,"merchant, counter and wares have a complete detailed sprite without cut edges");
 UnloadImageColors(pixels);ImageFlipVertical(&im);ExportImage(im,"qa-v42/merchant-sprite.png");UnloadImage(im);
 check(tower_furniture::groundShadows[tower_furniture::MerchantSprite].art.id!=0,"merchant's physical silhouette produces a ground shadow");

 for(int kind=1;kind<ItemKindCount;kind++){
  Item value=item(Kind(kind),0,3);auto text=itemTooltipText(value,{Place::Bag,10});Rectangle box=tooltipBounds(text);
  check(text.value.find("Wartość: ")==0&&text.value.find(std::to_string(itemSellValue(value)))!=std::string::npos&&text.value.find(std::to_string(itemStackValue(value)))!=std::string::npos,"every item tooltip shows unit and stack gold values");
  check(textWidth(text.value,14)<=box.width-34&&textWidth(text.chance,14)<=box.width-34,"value and rarity text fit without clipping");
 }
 mouse={0,0};shot("merchant-shop");
 game.bag[10]=item(Kind::CobraScale,0,8);point(merchantBagRect(10));clickTowerPanel();
 check(!merchantBuying&&merchantSale.place==Place::Bag&&merchantSale.index==10,"clicking a backpack item selects a monster material for sale");
 point(merchantMaxRect());clickTowerPanel();check(merchantQuantity==8,"MAX selects the whole owned stack");
 auto money=game.goldBalance();point(merchantTradeRect());clickTowerPanel();
 check(game.bag[10].empty()&&game.goldBalance()==money+8*itemSellValue(item(Kind::CobraScale)),"sell button removes only the selected stack and pays its full value");
 point(merchantTradeRect());clickTowerPanel();check(game.goldBalance()==money+8*itemSellValue(item(Kind::CobraScale)),"repeated sell-button click cannot duplicate currency");
 point(merchantOfferRect(0));clickTowerPanel();point(merchantPlusRect());clickTowerPanel();
 check(merchantBuying&&merchantQuantity==2,"offer and quantity controls select a two-potion purchase");
 money=game.goldBalance();int health=game.bagCount(Kind::Health);point(merchantTradeRect());clickTowerPanel();
 check(game.goldBalance()==money-2*itemBuyPrice(item(Kind::Health))&&game.bagCount(Kind::Health)==health+2,"buy button charges the displayed cost and adds the selected quantity");
 point(merchantPageRect(true));clickTowerPanel();check(merchantPage==1,"stock page buttons reveal remaining equipment");
 point(merchantOfferRect(5));clickTowerPanel();check(merchantOffer==11&&merchantSelectedItem().kind==Kind::BanditBow,"second page selects its actual better bow");
 mouse={0,0};shot("merchant-equipment");

 game.bag[10]=item(Kind::DawnShard,3);point(merchantBagRect(10));clickTowerPanel();money=game.goldBalance();point(merchantTradeRect());clickTowerPanel();
 check(game.bag[10].kind==Kind::DawnShard&&game.goldBalance()==money&&selectedMerchantStatus()==TradeResult::Protected,"quest protection applies through the real sell HUD");
 mouse={0,0};shot("merchant-protected-fragment");
 game.bag[10]=item(Kind::CobraScale,0,7);point(merchantBagRect(10));clickTowerPanel();point(merchantMaxRect());clickTowerPanel();mouse={0,0};shot("merchant-selling");
 point(merchantBagRect(10));shot("merchant-tooltip");

 point(merchantCloseRect());clickTowerPanel();check(!towerPanelOpen&&gameplayInputAllowed(),"close button resumes normal gameplay");
 game.player=MerchantPosition+V{-1,0};check(nearestTowerInteraction()&&towerPanelOpen,"trader can be reopened normally");
 game.player=TowerCenter;check(towerPanelInput()&&!towerPanelOpen,"merchant dialog closes if its context becomes unreachable");
 reset();game.gold=0;nearestTowerInteraction();point(merchantTradeRect());clickTowerPanel();
 check(game.goldBalance()==0&&game.bagCount(Kind::Health)==5&&selectedMerchantStatus()==TradeResult::NotEnoughGold,"insufficient funds show an error without removing any item");
 mouse={0,0};shot("merchant-no-gold");
 reset();nearestTowerInteraction();for(auto& value:game.bag)value=item(Kind::Crystal,0,99);money=game.goldBalance();point(merchantTradeRect());clickTowerPanel();
 check(game.goldBalance()==money&&selectedMerchantStatus()==TradeResult::NoSpace,"full bag rejects UI purchase without consuming currency");
 mouse={0,0};shot("merchant-full-bag");
 reset();report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
