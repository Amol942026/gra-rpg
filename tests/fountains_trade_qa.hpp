#pragma once
// Runs against production player input, renderer, transaction and save code.
template<class Check,class Save>void fountainsTradeQA(const fs::path& root,Check check,Save save){
 auto point=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
 auto reset=[](){game=Game{};intro={};towerStory={};clearTransientUi();welcome=help=paused=false;mouse={0,0};};
 auto merchant=[&](){reset();game.enterMageTower();game.dawnQuest=DawnQuest::AtTower;game.towerFloor=MerchantFloor;game.player=MerchantPosition+V{-1,0};game.gold=5000;openMerchantSelection();};
 merchant();auto gold=game.goldBalance();int count=game.bagCount(Kind::Health);
 point(merchantOfferRect(0));clickMerchantPanel(10);
 check(game.goldBalance()==gold&&game.bagCount(Kind::Health)==count,"single stock click selects only");
 clickMerchantPanel(10.15);
 check(game.goldBalance()==gold-itemBuyPrice(item(Kind::Health))&&game.bagCount(Kind::Health)==count+1,"double LMB stock purchase is exactly one item and its exact gold price");
 gold=game.goldBalance();clickMerchantPanel(10.22);
 check(game.goldBalance()==gold,"third rapid stock click cannot repeat the completed pair");clickMerchantPanel(10.30);
 check(game.goldBalance()==gold-itemBuyPrice(item(Kind::Health)),"four clicks form exactly two purchases");
 merchant();gold=game.goldBalance();point(merchantOfferRect(0));clickMerchantPanel(20);point(merchantOfferRect(1));clickMerchantPanel(20.1);
 check(game.goldBalance()==gold,"clicks on different stock items cannot combine");
 point(merchantOfferRect(0));clickMerchantPanel(21);clickMerchantPanel(21.5);
 check(game.goldBalance()==gold,"slow clicks cannot buy");
 openMerchantSelection();point(merchantOfferRect(0));clickMerchantPanel(22);mouse.x+=20;clickMerchantPanel(22.1);
 check(game.goldBalance()==gold,"drag-like click movement cannot buy");
 openMerchantSelection();point(merchantOfferRect(0));clickMerchantPanel(23);openMerchantSelection();clickMerchantPanel(23.1);
 check(game.goldBalance()==gold,"reopening the merchant resets pending clicks");
 merchant();game.bag[10]=item(Kind::CobraScale,0,8);gold=game.goldBalance();point(merchantBagRect(10));clickMerchantPanel(30);
 check(game.bag[10].count==8&&game.goldBalance()==gold,"single backpack click only selects a sale");clickMerchantPanel(30.1);
 check(game.bag[10].count==7&&game.goldBalance()==gold+itemSellValue(item(Kind::CobraScale)),"double LMB player stack sells exactly one and pays once");
 gold=game.goldBalance();point(merchantEquipRect(0));Item gear=game.equipment[0];clickMerchantPanel(31);clickMerchantPanel(31.1);
 check(game.equipment[0].empty()&&game.goldBalance()==gold+itemSellValue(gear),"double LMB on worn weapon sells the correct equipped item");
 game.bag[11]=item(Kind::DawnShard);gold=game.goldBalance();point(merchantBagRect(11));clickMerchantPanel(32);clickMerchantPanel(32.1);
 check(game.bag[11].kind==Kind::DawnShard&&game.goldBalance()==gold,"quest artifact remains protected on double click");
 merchant();gold=game.goldBalance();point(merchantOfferRect(0));clickMerchantPanel(33);point(merchantBagRect(0));clickMerchantPanel(33.1);
 check(game.goldBalance()==gold,"buy and sell panes cannot form one click pair");
 merchant();game.gold=0;point(merchantOfferRect(0));count=game.bagCount(Kind::Health);clickMerchantPanel(34);clickMerchantPanel(34.1);
 check(game.goldBalance()==0&&game.bagCount(Kind::Health)==count&&!merchantNotice.empty(),"double-click insufficient funds reject atomically with feedback");
 merchant();for(auto& i:game.bag)i=item(Kind::Crystal,0,99);gold=game.goldBalance();point(merchantOfferRect(0));clickMerchantPanel(35);clickMerchantPanel(35.1);
 check(game.goldBalance()==gold&&game.bagCount(Kind::Crystal)==BagSize*99,"full backpack double-click purchase preserves gold and contents");
 merchant();game.bag[10]=item(Kind::CobraScale,0,8);point(merchantBagRect(10));clickMerchantPanel(36);point(merchantMaxRect());clickMerchantPanel(36.1);gold=game.goldBalance();point(merchantTradeRect());clickMerchantPanel(36.2);
 check(game.bag[10].empty()&&game.goldBalance()==gold+8*itemSellValue(item(Kind::CobraScale)),"existing MAX and sale button retain whole-stack trading");
 // Actual tower panel screenshot; transaction tests above use its production click handler.
 merchant();nearestTowerInteraction();mouse={0,0};save(root/"merchant-double-click.png");
 reset();game.activateField(WorldLocation::Desert);game.player=DesertArrival;
 auto order=worldDrawOrder();check(std::count_if(order.begin(),order.end(),[](DrawObject o){return o.type==12;})==2,"exactly two fountains participate in desert depth sorting");
 for(int i=0;i<FountainCount;i++){
  check(fountain_art::atlas[i].id&&fountain_art::atlas[i].width==1024&&fountain_art::atlas[i].height==960&&fountain_art::dry[i].id,"full, draining and dry native fountain art preloads");
  check(!game.walkable(DesertFountains[i])&&!game.enemyPositionClear(DesertFountains[i],.4f),"fountain stone footprint blocks player and creatures");
  check(game.walkable(DesertFountains[i]+V{1.6f,0}),"fountain has a walkable reachable use position");
  check(std::none_of(game.props.begin(),game.props.end(),[&](const Prop& p){return distance(p.pos,DesertFountains[i])<propRadius(p.type)+FountainRadius+1;}),"native generation keeps the complete fountain clearing free of scenery");
  V from=DesertFountains[i]-V{2,0},to=DesertFountains[i]+V{2,0};check(!game.clearEnemySegment(from,to,.2f),"enemy pathfinding respects the fountain footprint");
  game.player=DesertFountains[i]+V{1.6f,0};game.hp=game.maxHp();game.mana=game.maxMana();
  check(!game.useFountain(i)&&!game.fountainUsed[i],"full resource never consumes the single-use source");
  // Find an actually opaque pixel through the real back-to-front pointer picker.
  Vector2 base=screen(DesertFountains[i]);bool picked=false;
  for(int y=-137;y<0&&!picked;y+=4)for(int x=-48;x<48&&!picked;x+=4){mouse={base.x+x,base.y+y};auto hit=pickWorld(mouse);if(hit.kind==HoverKind::Fountain&&hit.index==i)picked=true;}
  check(picked,"opaque fountain art can be selected by production world hover");
  auto inspection=inspectWorld(pickWorld(mouse));check(inspection.source==HoverKind::Fountain&&inspection.kind.find("Jednorazowa")!=std::string::npos,"hover HUD identifies the source and its single-use rule");
  game.hp=37;game.mana=11;float hp=game.hp,mana=game.mana;
  fs::create_directories(root/("fountain-"+std::to_string(i)+"-frames"));
  for(int f=0;f<24;f++){game.time=f/24.f;save(root/("fountain-"+std::to_string(i)+"-frames")/(std::to_string(f)+".png"));}
  if(i==0)check(clickWorldOrInventory(50),"actual LMB on mana fountain activates it");else check(rightClickWorldOrInventory(),"actual PPM on health fountain activates it");
  check(game.fountainUsed[i]&&game.fountainDrain[i]==1,"use starts its one-second drain and records permanent exhaustion");
  check(i==0?game.mana==game.maxMana()&&game.hp==hp:game.hp==game.maxHp()&&game.mana==mana,"fountain fully restores exactly its own resource");
  check(!game.useFountain(i),"second use cannot restore another resource point");
  auto savedTime=game.time;openPauseMenu();float drain=game.fountainDrain[i];for(int n=0;n<4;n++)input(.05f);
  check(game.fountainDrain[i]==drain&&game.time==savedTime,"Escape menu pauses water and draining");paused=false;menuPageTo(MenuPage::Home);
  saves::Snapshot snap;snap.game=game;auto loaded=saves::decode(saves::encode(snap));
  check(loaded.game.fountainUsed[i]&&loaded.game.fountainDrain[i]==drain,"mid-drain save preserves usage and exact water animation phase");
  for(int f=0;f<24;f++){
   // No enemies are simulated for these captures; only the production drain clock.
   game.fountainDrain[i]=std::max(0.f,1-(f+1)/24.f);game.time+=1.f/24;
   save(root/("fountain-"+std::to_string(i)+"-frames")/(std::to_string(24+f)+".png"));
  }
  check(game.fountainDrain[i]==0,"empty state has no residual animated water");
  game.enterMageTower();game.activateField(WorldLocation::Desert);game.player=DesertFountains[i]+V{1.6f,0};
  check(game.fountainUsed[i]&&game.fountainDrain[i]==0&&!game.useFountain(i),"tower travel and return cannot recharge a dry source");
 }
 saves::Snapshot snap;snap.game=game;saves::Store store(fs::absolute(root/"fountain-saves"));store.save(0,snap);game=store.load(0).game;
 check(game.fountainUsed[0]&&game.fountainUsed[1]&&game.fountainDrain[0]==0&&game.fountainDrain[1]==0,"on-disk save/load preserves both fully exhausted shrines");
 reset();game.activateField(WorldLocation::Desert);game.player=DesertFountains[0]+V{1.6f,0};game.mana=1;
 check(game.useNearestFountain()&&game.mana==game.maxMana()&&game.fountainUsed[0],"E nearest-source action restores full mana once");
 game.fountainDrain[0]=1;game.update(.05f,{},false);check(std::abs(game.fountainDrain[0]-.95f)<.00001f,"real simulation advances drain using elapsed time");
 reset();game.activateField(WorldLocation::Desert);game.mana=1;game.player=DesertFountains[0]+V{4,0};
 check(!game.useFountain(0)&&!game.fountainUsed[0],"distant click cannot consume a source");game.player=DesertFountains[0]+V{1.6f,0};game.sandPrisonRemaining=2;
 check(!game.useFountain(0),"sand prison prevents remote source activation");game.sandPrisonRemaining=0;game.hp=0;check(!game.useFountain(0),"source cannot revive a dead hero");
 check(!game.useFountain(-1)&&!game.useFountain(FountainCount),"invalid fountain identifiers are rejected");
 reset();check(!game.useFountain(0),"desert sources cannot be consumed from the forest");
 order=worldDrawOrder();check(std::none_of(order.begin(),order.end(),[](DrawObject o){return o.type==12;}),"desert fountains are absent from forest rendering");
 check(!audioSystem.available,"fountain and double-click tests never open an audio device");
}
