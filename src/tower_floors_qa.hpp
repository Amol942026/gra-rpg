#pragma once
bool runTowerFloorsQA(){
 fs::create_directories("qa-v28");std::ofstream report("qa-v28/tower-floors-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 auto reset=[&](){game=Game{};intro={};towerStory={};clearStoryMenus();game.enterMageTower();game.dawnQuest=DawnQuest::AtTower;game.messageTime=0;mouse={0,0};};
 auto image=[&](const char* name){std::string filename(name);renderScene();BeginDrawing();presentCanvas();Image im=presentedImage();check(im.data&&im.width==GetRenderWidth()&&im.height==GetRenderHeight(),"floor reaches the real display buffer at current window dimensions");if(im.data)ExportImage(im,(std::string("qa-v28/")+filename+".png").c_str());EndDrawing();finishFramePacing();return im;};
 auto locate=[&](HoverKind kind,int index,V pos){Vector2 p=screen(pos);for(int y=-154;y<=12;y+=3)for(int x=-58;x<=58;x+=3){Vector2 point{p.x+x,p.y+y};WorldHover hit=pickWorld(point);if(hit.kind==kind&&hit.index==index){mouse=point;return true;}}return false;};
 reset();refreshDisplayLayout();check(!audioSystem.available,"five-floor tests open no audio device");
 std::array<Image,TowerFloorCount> views{};
 for(int floor=0;floor<5;floor++){
  game.towerFloor=floor;game.player={12,15};game.time=2.7f;mouse={0,0};game.messageTime=0;
  for(const auto& object:worldDrawOrder())check(object.type!=6||floor==0,"Eldrin is drawn only on the arrival floor");
  views[floor]=image(TextFormat("floor-%d",floor+1));
  Color* pixels=LoadImageColors(views[floor]);int bright=0;for(int n=0;n<views[floor].width*views[floor].height;n++){auto c=pixels[n];bright+=int(c.r)+c.g+c.b>80;}check(bright>views[floor].width*views[floor].height*.22f,"themed floor has visible architecture and furnishings instead of a black scene");UnloadImageColors(pixels);
  check(towerFloorArt[floor].id&&towerFloorArt[floor].texture.width==1024,"static themed floor is cached on the GPU before play");
  if(floor>0){game.player=TowerStairDown+V{-1,0};check(locate(HoverKind::TowerStair,-1,TowerStairDown),"visible downstairs can be selected by its displayed shape");check(inspectWorld(pickWorld(mouse)).kind=="Przejście między piętrami","stairs expose destination in the inspection HUD");}
 }
 for(int floor=1;floor<5;floor++){Color* a=LoadImageColors(views[0]);Color* b=LoadImageColors(views[floor]);int changed=0;for(int y=170;y<views[0].height*3/4;y+=3)for(int x=40;x<views[0].width*2/3;x+=3){auto p=a[y*views[0].width+x],q=b[y*views[floor].width+x];changed+=std::abs(int(p.r)-q.r)+std::abs(int(p.g)-q.g)+std::abs(int(p.b)-q.b)>35;}check(changed>1500,"each themed upper floor differs materially from the arrival room pixels");UnloadImageColors(a);UnloadImageColors(b);}
 for(auto view:views)UnloadImage(view);
 reset();
 for(int floor=0;floor<4;floor++){
  game.player=TowerStairUp+V{1,0};check(locate(HoverKind::TowerStair,1,TowerStairUp),"upstairs can be reached through the shared pointer picker");
  check(rightClickWorldOrInventory()&&game.towerFloor==floor+1&&!towerPanelOpen,"right-click on actual stairs changes to exactly the next floor");
  check(game.walkable(game.player)&&game.towerFloor==floor+1&&!game.climbTower(1)&&game.towerFloor==floor+1,"safe arrival cannot immediately chain another upward climb");
 }
 for(int floor=4;floor>0;floor--){game.player=TowerStairDown+V{-1,0};check(locate(HoverKind::TowerStair,-1,TowerStairDown),"actual downstairs is selectable after return");check(clickWorldOrInventory()&&game.towerFloor==floor-1,"left-click returns to the previous floor without starting combat");}
 game.player=TowerStairUp+V{1,0};check(nearestTowerInteraction()&&game.towerFloor==1,"E interaction shares the nearest reachable stairs handler");
 game.player={12,12};check(locate(HoverKind::TowerObject,3,towerFurnishings(1)[3].pos),"library lectern is selectable in the actual world");check(rightClickWorldOrInventory()&&towerPanelOpen&&!gameplayInputAllowed(),"opening the book freezes and blocks game input through the real handler");
 Image book=image("library-book");UnloadImage(book);mouse={towerPageButton(true).x+20,680};clickTowerPanel();check(towerPage==1,"library next-page button changes the chapter");mouse={towerCloseButton().x+15,towerCloseButton().y+10};clickTowerPanel();check(!towerPanelOpen&&gameplayInputAllowed(),"close button restores control from a themed room menu");
 game.towerFloor=2;game.player={12,12};game.bag[8]=item(Kind::Mushroom,0,3);game.bag[9]=item(Kind::ForestEssence,0,3);
 check(locate(HoverKind::TowerObject,0,towerFurnishings(2)[0].pos)&&rightClickWorldOrInventory()&&towerPanelOpen,"right-click on cauldron opens the two actual alchemy recipes");
 Image cauldron=image("alchemy-recipes");UnloadImage(cauldron);int count=game.bag[0].count;mouse={towerRecipeButton(0).x+20,501};clickTowerPanel();check(game.bag[0].count==count+1&&game.bagCount(Kind::Mushroom)==0&&towerPanelOpen,"recipe button consumes ingredients and delivers exactly one health potion");
 count=game.bag[1].count;mouse={towerRecipeButton(1).x+20,501};clickTowerPanel();check(game.bag[1].count==count+1&&game.bagCount(Kind::ForestEssence)==0,"second recipe button produces a mana potion");closeTowerPanel();
 game.towerFloor=3;game.effects.clear();game.messageTime=0;
 for(int i=0;i<3;i++){game.player=towerFurnishings(3)[i].pos+V{1,0};check(locate(HoverKind::TowerObject,i,towerFurnishings(3)[i].pos)&&rightClickWorldOrInventory(),"each displayed rune is activated through the world pointer");}
 check(game.towerRuneMask==7,"all three activated rune stones light the seals floor");game.player={12,15};mouse={0,0};Image seals=image("seals-lit");UnloadImage(seals);
 game.towerFloor=4;game.effects.clear();game.messageTime=0;game.player={12,10.5f};check(locate(HoverKind::TowerObject,0,towerFurnishings(4)[0].pos)&&rightClickWorldOrInventory()&&towerPanelOpen,"luneta opens the star chart through its actual visible object");Image stars=image("observatory-stars");UnloadImage(stars);mouse={towerPageButton(true).x+20,680};clickTowerPanel();check(towerPage==1,"telescope can select a different constellation");closeTowerPanel();
 game.towerFloor=0;game.player=towerFurnishings(0)[7].pos+V{-1,0};game.hp=9;game.mana=1;check(nearestTowerInteraction()&&game.hp==game.maxHp()&&game.mana==game.maxMana(),"source of renewal is usable through the same E interaction");
 game.player=TowerStairUp;game.bag[8]=item(Kind::Health);game.drop({Place::Bag,8},game.player);int id=game.ground.back().id;game.climbTower(1);game.player=TowerStairUp;
 auto visibleLoot=nearby();check(std::find(visibleLoot.begin(),visibleLoot.end(),id)==visibleLoot.end(),"nearby HUD excludes loot dropped on another floor");
 bool picked=false;Vector2 p=screen(game.player);for(int y=-32;y<=2;y+=2)for(int x=-18;x<=18;x+=2){auto hit=pickWorld({p.x+x,p.y+y});picked|=hit.kind==HoverKind::Ground&&hit.index==id;}check(!picked,"floor-local loot cannot be picked through another floor");
 game.towerFloor=4;game.effects.clear();game.messageTime=0;game.player={12,15};bool startedFull=fullScreen();if(startedFull)toggleFullScreen();SetWindowSize(1152,720);PollInputEvents();refreshDisplayLayout();Image small=image("observatory-1152");UnloadImage(small);check(game.towerFloor==4&&game.walkable(game.player),"window resize preserves current floor and safe position");if(startedFull)toggleFullScreen();PollInputEvents();refreshDisplayLayout();
 reset();report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
