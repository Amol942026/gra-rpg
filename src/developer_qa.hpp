#pragma once
bool runDeveloperQA(){
 fs::create_directories("qa-v29");std::ofstream report("qa-v29/developer-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 auto point=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
 auto chooseTab=[&](WorldLocation realm){
  for(int i=0;i<int(std::size(DeveloperTabs));i++)if(DeveloperTabs[i]==realm){point(developerTabRect(i));clickDeveloperMenu();break;}
 };
 auto chooseDestination=[&](int index){chooseTab(DeveloperDestinations[index].location);point(developerCard(index));clickDeveloperMenu();};
 auto reset=[&](){game=Game{};intro={};towerStory={};clearStoryMenus();developerActive=developerMenuOpen=false;developerError.clear();developerSelection=0;mouse={0,0};};
 auto standardStart=[](){return game.level==100&&game.skillRanks==SkillRanks{1,1,1,1,1,1}&&game.learnedSpells==7&&game.ammoCount()==100&&game.bagCount(Kind::Health)==50&&game.bagCount(Kind::Mana)==50;};
 auto shot=[&](const char* name){std::string path=std::string("qa-v29/")+name+".png";renderScene();BeginDrawing();presentCanvas();Image im=presentedImage();check(im.data&&im.width==GetRenderWidth()&&im.height==GetRenderHeight(),"developer screen reaches real presentation at current window size");if(im.data)ExportImage(im,path.c_str());UnloadImage(im);EndDrawing();finishFramePacing();};
 refreshDisplayLayout();
 const auto decorationStart=GetTime();
 for(const auto& group:{std::pair{WorldLocation::Forest,ForestDecorationStart},
                        std::pair{WorldLocation::Desert,DesertDecorationStart},
                        std::pair{WorldLocation::Volcano,VolcanoDecorationStart}}){
  game=Game{};game.activateField(group.first);game.messageTime=0;
  welcome=paused=help=false;
  for(int type=group.second;type<group.second+DecorationsPerRealm;type++){
   const auto found=std::find_if(game.props.begin(),game.props.end(),[&](const Prop& prop){return prop.type==type;});
   check(found!=game.props.end(),"new decoration is present in its native realm");
   if(found==game.props.end())continue;
   check(std::count(propMasks[type].begin(),propMasks[type].end(),255)>500,"new decoration has a real GPU silhouette and mouse mask");
   game.player=found->pos+V{1.5f,1.5f};mouse={0,0};
   renderScene();saveTexture(canvas,TextFormat("qa-v29/decoration-%d.png",type));
  }
 }
 report<<"Decoration render and export: "<<GetTime()-decorationStart<<" s\n";
 auto impactShot=[&](int type,const char* name){
  game=Game{};game.player={20,20};game.props.clear();game.enemies.clear();game.forage.clear();game.messageTime=0;
  welcome=paused=help=false;
  V p=game.player+V{1.05f,0};
  game.enemies.push_back({100,type,p,p,500,500});
  game.hit(game.enemies[0],20);
  game.update(.05f,{},false);game.update(.02f,{},false);
  renderScene();saveTexture(canvas,std::string("qa-v29/")+name+".png");
 };
 impactShot(0,"impact-slime");impactShot(3,"impact-stone");impactShot(7,"impact-bark");
 reset();refreshDisplayLayout();check(!audioSystem.available,"developer tests open no audio device");startIntro();float age=intro.age;openDeveloperMenu();
 check(developerMenuOpen&&intro.active()&&!developerActive&&!gameplayInputAllowed()&&!worldPointerAllowed({525,428}),"opening developer picker during prologue blocks world input without prematurely replacing normal play");
 check(!castGameplaySpell()&&!clickWorldOrInventory()&&!rightClickWorldOrInventory(),"developer picker blocks spells, combat and inventory handlers");
 for(float s:{.8f,1.f,1.2f}){prepareCanvas(s);renderScene();saveTexture(canvas,TextFormat("qa-v29/menu-%d.png",int(s*100)));check(canvas.texture.width==int(W*s),"developer picker renders at each supported interface scale");}
 refreshDisplayLayout();shot("menu-prologue");point(developerCloseButton());clickDeveloperMenu();check(!developerMenuOpen&&intro.active()&&intro.age==age&&!developerActive,"cancel returns to same paused prologue without modifying the expedition");
 openDeveloperMenu();for(int id=0;id<DeveloperDestinationCount;id++){chooseDestination(id);check(developerSelection==id&&developerMenuOpen,"each location card selects its actual destination without starting accidentally");check(textWidth(DeveloperDestinations[id].name,18)<developerCard(id).width-26&&developerCard(id).y+developerCard(id).height<developerNewButton().y,"destination fits without clipping");}
 chooseTab(WorldLocation::Desert);shot("destinations-desert");
 chooseTab(WorldLocation::Volcano);shot("destinations-caldera");
 chooseDestination(12);point(developerNewButton());check(clickDeveloperMenu()&&developerActive&&!developerMenuOpen&&!intro.active()&&!towerStory.active()&&game.inTower()&&game.towerFloor==4&&gameplayInputAllowed(),"start button skips prologue and return scene and makes the selected observatory playable");
 check(game.walkable(game.player)&&standardStart(),"new test session receives safe position, level 100, all unlocks and supplies");shot("observatory-start");
 int gearCount=0;
 for(int id=1;id<ItemKindCount;id++){
  const Kind kind=Kind(id);const int slot=gearSlot(kind);if(slot<0)continue;
  ++gearCount;check(game.equipment[slot].kind==kind||game.bagCount(kind)>0,"all equipment kinds are available in DEV");
 }
 check(gearCount==EquipmentKindCount&&bagPages()>2,"catalogue uses as many 25-slot pages as needed");
 for(int page=1;page<bagPages();page++){
  point(bagPageRect(true));leftClickGameplay({});
  point(bagRect(0));check(bagPage==page&&hoverSlot().index==page*BagSize,"next arrow maps each visible cell to the next page");
 }
 point(bagPageRect(true));leftClickGameplay({});
 check(bagPage==bagPages()-1,"next arrow stops at the last page");
 shot("last-inventory-page");
 point(bagPageRect(false));leftClickGameplay({});
 check(bagPage==bagPages()-2,"previous arrow returns a page");
 const int index=bagIndex(0);
 const Item gear=game.bag[index];
 const int gearIndex=gearSlot(gear.kind);
 check(gearIndex>=0,"a later-page cell contains gear");
 if(gearIndex>=0){
  point(bagRect(0));leftClickGameplay({},1);finishDrag();leftClickGameplay({},1.1);
  check(game.equipment[gearIndex].kind==gear.kind,"double click equips the item on the visible later page");
 }
 openWeaponWheel();point(bagPageRect(true));clickWeaponWheel();
 check(bagPage==bagPages()-1,"weapon wheel also allows backpack paging");
 closeWeaponWheel();bagPage=0;
 mouse={107,170};check(worldPointerAllowed(mouse),"former DEV HUD area is usable world space");
 game.level=6;game.xp=17;developerBagSlot(game)=item(Kind::DawnShard);developerBagSlot(game)=item(Kind::Sword,2);game.skillRanks={3,2};game.attributes={4,3,2,1,3,4};auto old=game.bag;
 openDeveloperMenu();chooseDestination(10);point(developerMoveButton());clickDeveloperMenu();
 bool same=true;for(int i=0;i<BagSize;i++)same&=game.bag[i].kind==old[i].kind&&game.bag[i].tier==old[i].tier&&game.bag[i].count==old[i].count;
 check(game.towerFloor==2&&game.level==100&&game.xp==0&&game.skillRanks==SkillRanks{3,2,1,1,1,1}&&same,"move button preserves gear and higher ranks while enabling the developer loadout");
 developerBagSlot(game)=item(Kind::Mushroom,0,3);game.player={11,10.7f};check(nearestTowerInteraction()&&towerPanelOpen&&!developerMenuOpen,"developer start can immediately use ordinary cauldron interaction");int potions=game.bagCount(Kind::Health);check(game.brewTowerPotion(towerPanelObject,Kind::Health)&&game.bagCount(Kind::Health)==potions+1,"ordinary alchemy works with materials obtained in a developer session");shot("alchemy-start");
 closeTowerPanel();game.player=MerchantPosition;
 check(nearestTowerInteraction()&&towerPanelOpen&&towerFurnishings(game.towerFloor)[towerPanelObject].type==MerchantObjectType,"merchant interaction opens the actual trade panel");
 point(merchantBagPageRect(true));clickMerchantPanel();check(bagPage==1,"merchant opens the next DEV backpack page");
 point(merchantBagRect(0));clickMerchantPanel(2);check(merchantSale.index==BagSize,"merchant selects the item on that page");
 const auto wallet=game.goldBalance();clickMerchantPanel(2.1);
 check(game.bag[BagSize].empty()&&game.goldBalance()>wallet,"double click sells the item on the visible later page");
 openDeveloperMenu();chooseDestination(1);point(developerNewButton());clickDeveloperMenu();
 check(!game.inTower()&&standardStart()&&!game.bossDead&&game.enemies.back().alive&&gameplayInputAllowed(),"fresh boss session grants the standard test character and a living guardian");
 for(Skill skill:{Skill::Cleave,Skill::DoubleThrust,Skill::Skewer}){
  const auto binding=std::find_if(game.weaponWheel.begin(),game.weaponWheel.end(),[&](auto value){return archetype(value.kind)==skillProfile(skill).weapon;});
  game.attackCd=game.swing=0;
  const bool equipped=binding!=game.weaponWheel.end()&&game.selectWeapon(int(binding-game.weaponWheel.begin()));
  check(equipped&&game.skillForSlot(0)==skill&&activateSkillSlot(Skill::StrongCut),
        "developer weapon wheel and real Z handler activate the new skill");
 }
 const auto originalRoot=menuSaveRoot;
 menuSaveRoot=fs::absolute("qa-v29/dev-loadout-saves");menuSlot=0;
 game.skillRanks={4,0,0};game.learnedSpells=0;game.selectedSpell=Spell::None;
 game.bag[0]={};game.weaponWheel={};
 for(auto& value:game.bag)if(protectiveGear(value.kind))value={};
 const V savedPosition=game.player;
 menuSave();menuLoad();
 check(game.skillRanks==SkillRanks{4,1,1,1,1,1}&&game.learnedSpells==7&&game.bagCount(Kind::Health)==50&&
       distance(game.player,savedPosition)<.001f&&paused,
       "real menu load refreshes developer unlocks and supplies while keeping saved position and higher ranks");
 bool protectiveGearAvailable=true;
 for(int id=FirstProtectiveGear;id<ItemKindCount;id++)protectiveGearAvailable&=game.bagCount(Kind(id))==1;
 check(protectiveGearAvailable,"loading a DEV save created before the addition grants every new shield and helmet once");
 menuSaveRoot=originalRoot;resumeFromMenu();
 check(distance(game.player,ForestGuardian)<5&&developerSpawnSafe(game,game.player),"boss start is nearby without spawning inside his body or a pillar");shot("boss-start");
 const int farSite=developerDestination("volcano-obsidian-waste");
 openDeveloperMenu();chooseDestination(farSite);point(developerMoveButton());clickDeveloperMenu();
 check(game.inVolcano()&&std::string(game.currentRegion())==DeveloperDestinations[farSite].name,"far caldera warp and world HUD use the same site name");
 const int desertSite=developerDestination("desert-buried");
 openDeveloperMenu();chooseDestination(desertSite);point(developerMoveButton());clickDeveloperMenu();
 menuSaveRoot=fs::absolute("qa-v29/field-saves");
 auto& attacker=game.enemies.front();
 attacker.pos=game.player+V{1,0};attacker.behavior=EnemyBehavior::Chase;attacker.windup=.27f;
 const V combatPosition=game.player;
 openSaveMenu();check(paused&&menuPage==MenuPage::Save&&menuSaveSlotCount()==3,"direct save menu opens during an enemy windup");
 activateMenuRow(0);if(menuPage==MenuPage::Overwrite)activateMenuRow(0);
 check(menuStore().inspect(0).valid&&!menuStore().inspect(LegacySaveSlot).exists,"battle save writes an ordinary slot, without creating a checkpoint");
 shot("save-during-battle");
 menuPageTo(MenuPage::Load);refreshMenuSaves();activateMenuRow(0);
 check(menuPage==MenuPage::ReplaceSession,"field load goes through confirmation");
 activateMenuRow(0);check(game.inDesert()&&distance(game.player,combatPosition)<.001f&&game.enemies.front().windup==.27f&&
  std::string(game.currentRegion())==DeveloperDestinations[desertSite].name,"battle save restores the exact position, enemy windup and area");
 menuSaveRoot=originalRoot;resumeFromMenu();
 developerSelection=1;
 const float devHp=game.hp;
 game.enemyAbilityDamage(5000,DamageType::Magic,game.player,false,0,0);
 check(game.developerSession&&game.hp==devHp&&!game.dead(),"a lethal attack during a real developer session leaves its health unchanged");
 game.hp=0;check(startDeveloperSession(true)&&!game.dead()&&developerActive&&!intro.active()&&developerSelection==1&&standardStart(),"death restart restores the developer character at the same destination");
 openDeveloperMenu();developerSelection=-1;check(!startDeveloperSession(true)&&developerMenuOpen&&!developerError.empty(),"invalid developer selection does not close picker or replace the game");developerSelection=0;developerError="Nieznana lokacja: tower-6. Wybierz ją z listy.";shot("invalid-location");
 mouse={720,760};clickDeveloperMenu();check(developerActive&&developerMenuOpen&&!intro.active(),"former normal-game button area has no action");
 intro={};towerStory.beginReturn();towerStory.enter(TowerPhase::Dialogue);openDeveloperMenu();chooseDestination(11);point(developerNewButton());clickDeveloperMenu();
 check(game.towerFloor==3&&!towerStory.active()&&!intro.active()&&gameplayInputAllowed()&&!game.storyBlocked(),"developer start interrupts an active return cutscene without leaving ghost dialogue or blocked movement");
 bool startedFull=fullScreen();if(startedFull)toggleFullScreen();SetWindowSize(1152,720);PollInputEvents();refreshDisplayLayout();openDeveloperMenu();shot("menu-1152");
 Rectangle card=developerCard(12);Vector2 logical{card.x+20,card.y+20},raw{viewport.x+logical.x*viewport.width/W,viewport.y+logical.y*viewport.height/H};mouse=pointerToGame(raw);clickDeveloperMenu();check(developerSelection==12&&inside(card),"small-window pointer correctly chooses the requested tower floor");closeDeveloperMenu();if(startedFull)toggleFullScreen();PollInputEvents();refreshDisplayLayout();
 openPauseMenu();menuHomeAction(3);activateMenuRow(0);
 check(welcome&&!paused&&!developerActive&&!developerMenuOpen&&!game.developerSession,"return to main menu clears the developer session and its protection");
 check(bagPages()==1&&!clickBagPage(),"normal play has no backpack paging controls");
 menuNewGame();check(!game.developerSession&&game.maxHp()==100&&intro.active(),"a new expedition from the title uses normal resources and rules");
 reset();report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
