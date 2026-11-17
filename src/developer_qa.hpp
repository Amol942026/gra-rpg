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
 for(const auto realm:{WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano}){
  const Rectangle map{0,0,180,155};
  const auto marks=LoadRenderTexture(180,155);
  BeginTextureMode(marks);ClearBackground(BLANK);drawFieldSiteMarkers(map,realm);EndTextureMode();
  Image pixels=LoadImageFromTexture(marks.texture);ImageFlipVertical(&pixels);
  unsigned quadrants=0;int sites=0,drawn=0;bool hoveredNames=true;
  const FieldSite* previewSite=nullptr;
  for(const auto& site:FieldSites)if(site.realm==realm){
   const Vector2 point=fieldMinimapPoint(map,site.position);
   quadrants|=1u<<((site.position.x>=MapSize/2?1:0)+(site.position.y>=MapSize/2?2:0));
   sites++;
   const Color sample=GetImageColor(pixels,int(point.x),int(point.y));
   drawn+=sample.r==231&&sample.g==219&&sample.b==186;
   hoveredNames&=hoveredFieldSite(realm,fieldMinimapPoint(minimapRect(),site.position))==&site;
   if(!previewSite&&site.position.x>70&&site.position.y>70)previewSite=&site;
  }
  check(sites>=14&&drawn==sites&&quadrants==15,"every named field site renders at its real minimap position and all four quarters are marked");
  check(hoveredNames&&!hoveredFieldSite(realm,{0,0})&&!hoveredFieldSite(realm,{minimapRect().x+90,25}),
        "each marker picks its real field site and blank minimap space clears the hover");
  UnloadImage(pixels);UnloadRenderTexture(marks);
  game=Game{};game.activateField(realm);game.messageTime=0;
  welcome=paused=help=false;renderScene();
  saveTexture(canvas,TextFormat("qa-v29/minimap-%d.png",int(realm)));
  if(previewSite){
   mouse=fieldMinimapPoint(minimapRect(),previewSite->position);
   check(!worldPointerAllowed(mouse),"a hovered minimap marker never targets the world");
   renderScene();saveTexture(canvas,TextFormat("qa-v29/minimap-hover-%d.png",int(realm)));
   mouse={0,0};
  }
 }
 {
  game=Game{};game.activateField(WorldLocation::Volcano);
  const auto target=LoadRenderTexture(160,80);
  for(int type:{14,17,VolcanoDecorationStart,VolcanoLandmarkStart,VolcanoLandmarkStart+3}){
   const auto prop=std::find_if(game.props.begin(),game.props.end(),[=](const Prop& p){return p.type==type;});
   check(prop!=game.props.end(),"Caldera contact shadow has a world prop");
   if(prop==game.props.end())continue;
   game.player=prop->pos;const Vector2 base=screen(prop->pos);
   BeginTextureMode(target);ClearBackground(WHITE);BeginMode2D({{80,40},base,0,1});
   volcano_art::drawPropContact(*prop);
   EndMode2D();EndTextureMode();
   Image pixels=LoadImageFromTexture(target.texture);ImageFlipVertical(&pixels);
   check(GetImageColor(pixels,80,42).r<115&&GetImageColor(pixels,80,8).r==255,
         "Caldera contact darkens the ground directly under the rock and fades away");
   UnloadImage(pixels);
  }
  UnloadRenderTexture(target);
 }
 const auto decorationStart=GetTime();
 for(const auto& group:{std::pair{WorldLocation::Forest,ForestDecorationStart},
                        std::pair{WorldLocation::Desert,DesertDecorationStart},
                        std::pair{WorldLocation::Volcano,VolcanoDecorationStart}}){
  game=Game{};game.activateField(group.first);game.messageTime=0;
  welcome=paused=help=false;
  const int limit=group.first==WorldLocation::Volcano?PropTypeCount:group.second+DecorationsPerRealm;
  for(int type=group.second;type<limit;type++){
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
 point(developerSummonTab());clickDeveloperMenu();
 check(!developerSummonMenu,"ordinary play cannot open the summon panel");
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
 point(developerSummonHudRect());
 check(developerSummonHudHit(mouse)&&!worldPointerAllowed(mouse),"visible DEV summon button intercepts world pointer");
 shot("summon-hud");
 leftClickGameplay({});
 check(developerSummonMenu&&developerMenuOpen&&!gameplayInputAllowed()&&!game.holdingMelee(),
       "a single gameplay HUD click opens the summon panel without starting a world attack");
 const double summonRender=GetTime();
 shot("summon-menu");
 report<<"Summon menu render and export: "<<GetTime()-summonRender<<" s\n";
 const auto initialEnemies=game.enemies.size();
 for(int n=0;n<EnemyTypeCount;n++){
  if(n&&n%DeveloperSummonPageSize==0){point(developerSummonPageButton(true));clickDeveloperMenu();}
  const int slot=n%DeveloperSummonPageSize,type=DeveloperSummonTypes[n];
  const auto card=developerSummonCard(slot);
  check(textWidth(enemyInfo(type).name,18)<card.width-(enemyBoss(type)?70:26),"full enemy name fits the summon card");
  if(game.enemies.size()>initialEnemies)game.enemies.pop_back();
  point(card);clickDeveloperMenu();
  check(developerSummonSucceeded&&game.enemies.size()==initialEnemies+1&&game.enemies.back().type==type,
        "clicking each named card creates the correct enemy in the world");
 }
 point(developerSummonReturn());clickDeveloperMenu();check(!developerMenuOpen&&gameplayInputAllowed(),"return button resumes combat after a summon");
 if(game.enemies.size()==initialEnemies+1){
  const auto id=game.enemies.back().id;
  game.enemies.erase(game.enemies.begin(),game.enemies.end()-1);
  game.combatImpacts.clear();
  const float hp=game.hp;
  for(int frame=0;frame<480&&game.combatImpacts.empty();frame++)game.update(1.f/60,{},false);
  check(game.enemies.front().id==id&&!game.combatImpacts.empty()&&!game.effects.empty()&&
        game.effects.back().text=="TRAFIENIE"&&game.hp==hp,
        "an enemy summoned through the HUD chases and hits the hero when play resumes");
  const double hitRender=GetTime();
  shot("summoned-enemy-hit");
  report<<"Summoned enemy hit render and export: "<<GetTime()-hitRender<<" s\n";
 }
 point(developerSummonHudRect());leftClickGameplay({});
 check(developerSummonMenu&&developerMenuOpen,"the DEV HUD button can reopen the panel after combat resumes");
 point(developerSummonReturn());clickDeveloperMenu();
 shot("summoned-enemy");
 openDeveloperMenu();chooseDestination(developerDestination("forest-deep"));
 point(developerNewButton());clickDeveloperMenu();
 game.props.clear();game.player={20,20};game.facing={1,0};
 const auto wolf=std::find_if(game.enemies.begin(),game.enemies.end(),[](const Enemy& e){return e.type==1&&e.alive;});
 check(wolf!=game.enemies.end(),"forest fixture contains an authored wolf");
 if(wolf!=game.enemies.end()){
  Enemy original=*wolf;
  original.pos=original.home=game.player+V{2.f,.075f};
  game.enemies={original};
  for(int i=0;i<120;i++)game.update(1.f/60,{},false);
  point(developerSummonHudRect());leftClickGameplay({});
  const auto wolfCard=std::find(DeveloperSummonTypes.begin(),DeveloperSummonTypes.end(),1);
  const int slot=int(wolfCard-DeveloperSummonTypes.begin());
  developerSummonPage=slot/DeveloperSummonPageSize;
  point(developerSummonCard(slot%DeveloperSummonPageSize));clickDeveloperMenu();
  point(developerSummonReturn());clickDeveloperMenu();
  check(game.enemies.size()==2&&game.enemies[0].id==original.id&&game.testSummon(game.enemies[1].id),
        "HUD summoning adds a wolf to the ongoing encounter");
  if(game.enemies.size()==2){
   std::array<bool,2> struck{};
   float largestTurn=0,largestStep=0;
   for(int frame=0;frame<360;frame++){
    const auto before=game.enemies;
    game.update(1.f/60,{},false);
    for(int i=0;i<2;i++){
     const auto& a=before[i];const auto& b=game.enemies[i];
     struck[i]|=a.windup>0&&b.strike>0;
     largestStep=std::max(largestStep,distance(a.pos,b.pos));
     if(a.plantedAttack())largestTurn=std::max(largestTurn,
       std::abs(std::remainder(b.animation.facingAngle-a.animation.facingAngle,2*AnimationPi)));
    }
   }
   check(struck[0]&&struck[1]&&largestTurn<.12f&&largestStep<.05f,
         "authored and summoned wolves keep stable movement and facing during attacks");
   report<<"Mixed fight max step: "<<largestStep<<" world units\n"
         <<"Mixed fight max attack turn: "<<largestTurn*180/AnimationPi<<" degrees per simulation step\n";
   const double renderStart=GetTime();
   shot("mixed-fight");
   report<<"Mixed fight render and export: "<<GetTime()-renderStart<<" s\n";
  }
 }
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
 welcome=false;point(developerSummonHudRect());
 check(!developerSummonHudHit(mouse)&&worldPointerAllowed(mouse),"the DEV summon button leaves the same area playable in ordinary mode");
 menuNewGame();check(!game.developerSession&&game.maxHp()==100&&intro.active(),"a new expedition from the title uses normal resources and rules");
 reset();
 check(startDeveloperSession(true),"start a silent scene for the copper scarab");
 game.activateField(WorldLocation::Desert);game.player={32,24};game.props.clear();game.enemies.clear();
 game.dawnQuest=DawnQuest::AtTower;
 const V scarabAt=game.player+V{2.5f,0};
 const auto& spec=enemyAbilityProfile(9);
 game.enemies.push_back({240,9,scarabAt,scarabAt,96,96});
 auto& scarab=game.enemies.front();
 scarab.animation.face(-1,0);scarab.abilityCd=0;
 check(creature_paint::available(9)&&scarab_wave_art::crest.id,"scarab material and wave crest are loaded by the real renderer");
 for(int direction=0;direction<fallen_art::Directions;direction++)for(int flank:{1,-1}){
  const auto& sprite=fallen_art::spriteFor(9,direction,flank);
  check(sprite.body.id&&sprite.ground.art.id&&sprite.footprint.width>35&&sprite.footprint.height>18,
        "each scarab corpse direction and flank has a detailed body and a ground shadow");
 }
 shot("scarab-live");
 const double scarabRender=GetTime();
 while(scarab.abilityAge<.55f&&game.time<2.4f)game.update(1.f/60,{},false);
 check(scarab.abilityBusy()&&scarab.abilityAge<spec.windup,"copper scarab coils up during the warning");
 shot("scarab-wave-warning");
 while(scarab.abilityAge<spec.windup+.48f&&game.time<2.4f)game.update(1.f/60,{},false);
 check(scarab.abilityBusy()&&scarab.abilityMask==1&&game.hp==game.maxHp(),"wave reaches the player once at the familiar radius");
 shot("scarab-wave-active");
 game.hit(scarab,10000,DamageType::Physical,false,{1,.2f});
 check(scarab.dying()&&scarab.deathSpecial>=0,"death begins in the current casting pose");
 game.effects.clear();
 const float contact=enemy_death::groundContactTime(9);
 for(int i=0;i<8;i++)game.advanceEnemyDeaths(contact/16);
 check(!scarab.corpseGrounded()&&scarab.deathProgress()>0,"the shell rolls and sheds weight before landing");
 shot("scarab-fall");
 for(int i=0;i<10;i++)game.advanceEnemyDeaths(contact/16);
 const Vector2 bodyPoint=corpseDrawPoint(scarab,screen(scarab.pos));
 const auto& body=fallen_art::spriteFor(scarab);
 bool bodyHit=false;
 for(float y=body.footprint.y;y<body.footprint.y+body.footprint.height&&!bodyHit;y+=2)
  for(float x=body.footprint.x;x<body.footprint.x+body.footprint.width&&!bodyHit;x+=2){
   const Vector2 probe{bodyPoint.x+x,bodyPoint.y+y};
   bodyHit=restingCorpseArt(scarab,bodyPoint.x,bodyPoint.y,BLANK,&probe);
  }
 check(scarab.corpseGrounded()&&bodyHit&&game.findCorpse(scarab.id),"settled copper scarab uses a pickable corpse with the new shell");
 shot("scarab-rest");
 report<<"Copper scarab render and export (four views): "<<GetTime()-scarabRender<<" s\n";
 reset();report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
