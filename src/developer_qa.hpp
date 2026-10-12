#pragma once
bool runDeveloperQA(){
 fs::create_directories("qa-v29");std::ofstream report("qa-v29/developer-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 auto point=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
 auto reset=[&](){game=Game{};intro={};towerStory={};clearStoryMenus();developerActive=developerMenuOpen=false;developerError.clear();developerKit=true;developerSelection=0;mouse={0,0};};
 auto shot=[&](const char* name){std::string path=std::string("qa-v29/")+name+".png";renderScene();BeginDrawing();presentCanvas();Image im=presentedImage();check(im.data&&im.width==GetRenderWidth()&&im.height==GetRenderHeight(),"developer screen reaches real presentation at current window size");if(im.data)ExportImage(im,path.c_str());UnloadImage(im);EndDrawing();finishFramePacing();};
 reset();refreshDisplayLayout();check(!audioSystem.available,"developer tests open no audio device");startIntro();float age=intro.age;openDeveloperMenu();
 check(developerMenuOpen&&intro.active()&&!developerActive&&!gameplayInputAllowed()&&!worldPointerAllowed({525,428}),"opening developer picker during prologue blocks world input without prematurely replacing normal play");
 check(!castGameplaySpell()&&!clickWorldOrInventory()&&!rightClickWorldOrInventory(),"developer picker blocks spells, combat and inventory handlers");
 for(float s:{.8f,1.f,1.2f}){prepareCanvas(s);renderScene();saveTexture(canvas,TextFormat("qa-v29/menu-%d.png",int(s*100)));check(canvas.texture.width==int(W*s),"developer picker renders at each supported interface scale");}
 refreshDisplayLayout();shot("menu-prologue");point(developerCloseButton());clickDeveloperMenu();check(!developerMenuOpen&&intro.active()&&intro.age==age&&!developerActive,"cancel returns to same paused prologue without modifying the expedition");
 openDeveloperMenu();for(int id=0;id<DeveloperDestinationCount;id++){point(developerCard(id));clickDeveloperMenu();check(developerSelection==id&&developerMenuOpen,"each location card selects its actual destination without starting accidentally");check(textWidth(DeveloperDestinations[id].name,18)<developerCard(id).width-26&&textWidth(DeveloperDestinations[id].hint,14)<developerCard(id).width-26,"destination name and hint fit card without clipping");}
 point(developerNewButton());check(clickDeveloperMenu()&&developerActive&&!developerMenuOpen&&!intro.active()&&!towerStory.active()&&game.inTower()&&game.towerFloor==4&&gameplayInputAllowed(),"start button skips prologue and return scene and makes the selected observatory playable");
 check(game.walkable(game.player)&&game.skillRanks==SkillRanks{1,1,1}&&game.ammoCount()==100&&game.bagCount(Kind::Mushroom)==15,"new test session receives safe position and optional kit");shot("observatory-start");
 point(developerButton());check(!worldPointerAllowed(mouse),"visible DEV HUD button cannot leak its click into a world attack");
 game.level=6;game.xp=17;game.bag[12]=item(Kind::DawnShard);game.bag[13]=item(Kind::Sword,2);game.skillRanks={3,2};game.attributes={4,3,2,1,3,4};auto old=game.bag;
 openDeveloperMenu();point(developerCard(10));clickDeveloperMenu();point(developerMoveButton());clickDeveloperMenu();
 bool same=true;for(int i=0;i<BagSize;i++)same&=game.bag[i].kind==old[i].kind&&game.bag[i].tier==old[i].tier&&game.bag[i].count==old[i].count;
 check(game.towerFloor==2&&game.level==6&&game.xp==17&&game.skillRanks==SkillRanks{3,2,1}&&same,"move button preserves character and possessions while switching straight to alchemy");
 game.player={12,12};check(nearestTowerInteraction()&&towerPanelOpen&&!developerMenuOpen,"developer start can immediately use ordinary cauldron interaction");int potions=game.bagCount(Kind::Health);check(game.brewTowerPotion(towerPanelObject,Kind::Health)&&game.bagCount(Kind::Health)==potions+1,"kit materials work with the real alchemy mechanic");shot("alchemy-start");
 openDeveloperMenu();point(developerCard(1));clickDeveloperMenu();point(developerKitButton());clickDeveloperMenu();point(developerNewButton());clickDeveloperMenu();
 check(!developerKit&&!game.inTower()&&game.level==1&&game.ammoCount()==20&&game.skillRanks==SkillRanks{}&&!game.bossDead&&game.enemies.back().alive&&gameplayInputAllowed(),"fresh boss session without kit resets character and creates living guardian immediately");
 check(distance(game.player,ForestGuardian)<5&&developerSpawnSafe(game,game.player),"boss start is nearby without spawning inside his body or a pillar");shot("boss-start");
 game.hp=0;check(startDeveloperSession(true)&&!game.dead()&&developerActive&&!intro.active()&&developerSelection==1,"death restart reuses same developer destination and skips prologue");
 openDeveloperMenu();developerSelection=-1;check(!startDeveloperSession(true)&&developerMenuOpen&&!developerError.empty(),"invalid developer selection does not close picker or replace the game");developerSelection=0;developerError="Nieznana lokacja: tower-6. Wybierz ją z listy.";shot("invalid-location");
 point(developerNormalButton());clickDeveloperMenu();check(!developerActive&&!developerMenuOpen&&intro.active()&&game.level==1&&game.ammoCount()==20&&game.skillRanks==SkillRanks{}&&game.bagCount(Kind::Mushroom)==0,"normal-game button removes test state and restarts untouched normal prologue");
 intro={};towerStory.beginReturn();towerStory.enter(TowerPhase::Dialogue);openDeveloperMenu();point(developerCard(11));clickDeveloperMenu();point(developerNewButton());clickDeveloperMenu();
 check(game.towerFloor==3&&!towerStory.active()&&!intro.active()&&gameplayInputAllowed()&&!game.storyBlocked(),"developer start interrupts an active return cutscene without leaving ghost dialogue or blocked movement");
 bool startedFull=fullScreen();if(startedFull)toggleFullScreen();SetWindowSize(1152,720);PollInputEvents();refreshDisplayLayout();openDeveloperMenu();shot("menu-1152");
 Rectangle card=developerCard(12);Vector2 logical{card.x+20,card.y+20},raw{viewport.x+logical.x*viewport.width/W,viewport.y+logical.y*viewport.height/H};mouse=pointerToGame(raw);clickDeveloperMenu();check(developerSelection==12&&inside(card),"small-window pointer correctly chooses the requested tower floor");closeDeveloperMenu();if(startedFull)toggleFullScreen();PollInputEvents();refreshDisplayLayout();
 reset();report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
