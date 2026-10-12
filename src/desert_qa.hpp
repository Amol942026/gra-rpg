#pragma once
bool runDesertQA(){
 fs::create_directories("qa-v33");std::ofstream report("qa-v33/desert-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 auto reset=[](){game=Game{};intro={};towerStory={};clearStoryMenus();developerActive=developerMenuOpen=false;game.messageTime=0;mouse={0,0};};
 auto shot=[&](const std::string& name){renderScene();saveTexture(canvas,"qa-v33/"+name+".png");};
 auto allocate=[](){if(game.attributePoints){Attributes p{};p[0]=game.attributePoints;game.distribute(p);}if(game.skillPoints){SkillRanks ranks{};ranks[0]=game.skillPoints;game.trainSkills(ranks);}};
 auto finishStory=[](){for(int i=0;i<260&&towerStory.active();i++)towerAction(false,true,1.f/60);};
 auto choice=[](){for(int i=0;i<12&&towerStory.phase==TowerPhase::Dialogue;i++){towerStory.revealed=true;towerStory.age=.2f;towerAction(true,false,0);}};
 auto locate=[](HoverKind kind,int index,V pos){Vector2 p=screen(pos);for(int y=-165;y<30;y+=3)for(int x=-120;x<120;x+=3){auto hit=pickWorld({p.x+x,p.y+y});if(hit.kind==kind&&hit.index==index){mouse={p.x+x,p.y+y};return true;}}return false;};
 reset();refreshDisplayLayout();check(!audioSystem.available,"desert tests open no audio device");check(desert_art::terrain.id&&desert_art::minimap.id,"second realm terrain and minimap are cached before gameplay");
 // Verify new 2x obstacle masks against every GPU pixel and transparent margins.
 for(int type=ForestPropTypeCount;type<PropTypeCount;type++){
  Image im=LoadImageFromTexture(propArt[type].texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);int solid=0,clipCount=0,mismatch=0;std::vector<unsigned> colors;
  for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++){const auto& c=pixels[y*im.width+x];bool p=c.a>=160;solid+=p;clipCount+=p&&(x<3||y<3||x>=im.width-3||y>=im.height-3);mismatch+=(propMasks[type][y*im.width+x]!=0)!=p;if(p&&x%3==0&&y%3==0)colors.push_back((unsigned(c.r)<<16)|(unsigned(c.g)<<8)|c.b);}
  std::sort(colors.begin(),colors.end());int count=std::unique(colors.begin(),colors.end())-colors.begin();
  check(im.width==360&&im.height==460&&solid>500&&clipCount==0,"desert prop "+std::to_string(type)+": detailed 2x model has no cut-off edges");check(count>=(type==11?4:5)&&mismatch==0,"desert prop "+std::to_string(type)+": rich material shades and physical mouse mask agree with GPU pixels");UnloadImageColors(pixels);UnloadImage(im);
 }
 // Same physical rigs draw, turn, animate and receive mouse inspection in all directions.
 for(int type=8;type<EnemyTypeCount;type++){
  RenderTexture2D sheet=LoadRenderTexture(1120,570);BeginTextureMode(sheet);ClearBackground(Ink);txt(enemyInfo(type).name,22,13,25,Gold);
  for(int direction=0;direction<8;direction++){float x=direction%4*280.f+140,y=direction/4*260.f+246;AnimationState state;state.direction=Direction(direction);state.moving=true;state.phase=.17f;state.idleTime=2.7f;actor_art::draw(type+1,x,y,1,state);center(TextFormat("KIERUNEK %d",direction+1),x,y+18,14,Muted);}
  EndTextureMode();saveTexture(sheet,"qa-v33/enemy-"+std::to_string(type)+"-directions.png");UnloadRenderTexture(sheet);
  Image before{};int changed=0;
  for(int pose=0;pose<3;pose++){
   RenderTexture2D rt=LoadRenderTexture(320,250);AnimationState state;state.direction=Direction::SE;state.idleTime=2.7f;state.moving=pose!=0;state.phase=pose==1?.15f:.45f;float attack=pose==2?.85f:0;
   BeginTextureMode(rt);ClearBackground(BLANK);actor_art::draw(type+1,160,212,1,state,attack);EndTextureMode();Image im=LoadImageFromTexture(rt.texture);ImageFlipVertical(&im);Color* p=LoadImageColors(im);
   if(pose==0)before=ImageCopy(im);else{Color* a=LoadImageColors(before);for(int i=0;i<im.width*im.height;i++)changed+=std::abs(int(a[i].r)-p[i].r)+std::abs(int(a[i].g)-p[i].g)+std::abs(int(a[i].a)-p[i].a)>50;UnloadImageColors(a);}
   int drawn=0,cut=0;for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++){bool opaque=p[y*im.width+x].a>=240;drawn+=opaque;cut+=opaque&&(x<3||y<3||x>=im.width-3||y>=im.height-3);}check(drawn>400&&cut==0,"enemy "+std::to_string(type)+": idle/walk/attack silhouette renders fully inside its frame");
   if(pose==1){int probes=0,bad=0;actor_art::Rig rig(160,212,1,state.direction);actor_art::buildDesert(rig,type,state,attack,false);
    for(int y=5;y<im.height-5;y+=6)for(int x=5;x<im.width-5;x+=6){bool opaque=true,empty=true;for(int dy:{-2,0,2})for(int dx:{-2,0,2}){auto a=p[(y+dy)*im.width+x+dx].a;opaque&=a>=240;empty&=a<4;}if(!opaque&&!empty)continue;bool picked=rig.contains({x+.5f,y+.5f});probes++;bad+=opaque?!picked:picked;}
    check(probes>1000&&bad==0,"enemy "+std::to_string(type)+": actual physical rig inspection agrees with GPU body pixels");
   }UnloadImageColors(p);UnloadImage(im);UnloadRenderTexture(rt);
  }check(changed>200,"enemy "+std::to_string(type)+": movement and attack visibly change the pose");UnloadImage(before);
 }
 RenderTexture2D models=LoadRenderTexture(1200,370);BeginTextureMode(models);ClearBackground(Ink);txt("MORZE PIASKÓW — PRZECIWNICY",23,13,24,Gold);
 for(int i=0;i<4;i++){float x=i*300.f;rect(x+7,55,286,300,Panel);Enemy e{100+i,8+i,{},{},100,100};e.animation.idleTime=2.7f;actor_art::draw(e.type+1,x+111,287,1,e.animation);hero(x+237,287,1);center(enemyInfo(e.type).name,x+150,310,19,Text);center(TextFormat("HP %.0f · ATAK %d",enemyInfo(e.type).health,enemyInfo(e.type).attack),x+150,338,14,Gold);}EndTextureMode();saveTexture(models,"qa-v33/desert-enemies.png");UnloadRenderTexture(models);
 models=LoadRenderTexture(1200,650);BeginTextureMode(models);ClearBackground(Ink);txt("MORZE PIASKÓW — ROŚLINY I RUINY",23,13,24,Gold);
 for(int i=0;i<7;i++){float x=i%4*300.f,y=54+i/4*286.f;rect(x+7,y+4,286,278,Panel);auto& tex=propArt[7+i];DrawTexturePro(tex.texture,{0,0,float(tex.texture.width),-float(tex.texture.height)},{x+24,y+15,180,230},{0,0},0,WHITE);hero(x+238,y+223,1);center(propInfo(7+i).name,x+150,y+251,17,Text);}EndTextureMode();saveTexture(models,"qa-v33/desert-props.png");UnloadRenderTexture(models);
 // Real first boss loot -> return -> mage conversation -> explicit mouse choice.
 reset();game.player=game.enemies.back().pos;int first=game.enemies.back().id;game.hit(game.enemies.back(),1000);allocate();game.openCorpse(first);game.transfer({Place::Corpse,4,first},{Place::Bag,8});startTowerReturn();finishStory();game.player={12,12};mouse={0,0};game.messageTime=0;
 check(game.inTower()&&game.dawnFragments()==1&&talkToMage(),"first fragment returns to Eldrin and opens the actual repeat conversation");choice();check(towerStory.phase==TowerPhase::Choice&&game.inTower()&&!gameplayInputAllowed(),"real dialogue ends in a paused travel choice without teleporting");
 for(int i=0;i<360;i++)towerAction(false,false,1.f/60);check(towerStory.phase==TowerPhase::Choice&&game.inTower(),"waiting or dialogue advancement never substitutes for selecting travel");
 for(float s:{.8f,1.f,1.2f}){prepareCanvas(s);shot("mage-choice-"+std::to_string(int(s*100)));}refreshDisplayLayout();
 mouse={desertStayButton().x+20,desertStayButton().y+20};check(clickMageTravelChoice()&&!towerStory.active()&&game.inTower(),"staying option closes dialogue and retains the tower");talkToMage();choice();auto bag=game.bag;int level=game.level;mouse={desertTravelButton().x+20,desertTravelButton().y+20};check(clickMageTravelChoice()&&towerStory.phase==TowerPhase::TravelOut&&game.inTower(),"travel mouse button begins departure fade before changing region");float age=towerStory.age;towerAction(true,true,.1f,false);check(towerStory.age==age&&game.inTower(),"lost focus freezes teleport and ignores all controls");
 for(int i=0;i<150&&towerStory.phase!=TowerPhase::TravelIn;i++)towerAction(false,false,1.f/60);check(game.inDesert()&&towerStory.phase==TowerPhase::TravelIn&&game.walkable(game.player)&&game.level==level&&game.bag[8].kind==bag[8].kind,"fade changes to the real desert and keeps the hero and first fragment");shot("desert-teleport-arrival");for(int i=0;i<120&&towerStory.active();i++)towerAction(false,false,1.f/60);check(!towerStory.active()&&gameplayInputAllowed()&&game.inDesert(),"arrival reveal finishes and restores movement and combat");
 // The portal uses normal world picking and PPM, without resetting the desert.
 game.messageTime=0;check(locate(HoverKind::Prop,int(game.props.size())-1,DesertPortal)&&inspectWorld(pickWorld(mouse)).name=="Brama Eldrina","desert gate has accurate picking and an inspection HUD");shot("gate-hover");check(rightClickWorldOrInventory()&&game.inTower()&&!towerStory.active(),"PPM on the gate returns through the same game handler");game.player={12,12};talkToMage();choice();mouse={desertTravelButton().x+20,desertTravelButton().y+20};clickMageTravelChoice();for(int i=0;i<150;i++)towerAction(false,false,1.f/60);
 game.player=DesertBossHome+V{-1.3f,0};game.messageTime=0;check(locate(HoverKind::Enemy,DesertBossId,DesertBossHome)&&inspectWorld(pickWorld(mouse)).maxHp==560,"second boss is selectable and exposes its own HP, kind and damage");shot("dune-boss-hover");
 Enemy* boss=game.findEnemy(DesertBossId);game.hit(*boss,3000);allocate();game.player=DesertBossHome+V{-1.3f,0};game.messageTime=0;check(locate(HoverKind::Corpse,DesertBossId,DesertBossHome)&&rightClickWorldOrInventory()&&game.openedCorpse==DesertBossId,"PPM opens the persistent sand corpse loot in the HUD");shot("dune-boss-loot");
 auto rect=lootRect(4);mouse={rect.x+20,rect.y+20};clickWorldOrInventory();check(dragging.place==Place::Corpse&&dragging.owner==DesertBossId&&dragging.index==4,"LPM on the actual corpse HUD starts dragging the second fragment");rect=bagRect(10);mouse={rect.x+20,rect.y+20};finishDrag();startTowerReturn();check(game.dawnFragments()==2&&game.bag[10].kind==Kind::DawnShard&&towerStory.returning&&towerStory.fragments==2,"dropping the second fragment in the backpack starts its own return scene");finishStory();check(game.inTower()&&game.dawnFragments()==2&&gameplayInputAllowed()&&game.desertBossDead,"second return finishes in the tower with both protected fragments");
 // Actual room views at native and scaled resolutions, plus the real fullscreen buffer.
 const std::array<V,5> locations={{DesertArrival,{21,26},DesertOasis+V{0,-4.2f},{43,15},DesertBossHome+V{0,-3}}};
 for(int view=0;view<5;view++){
  reset();game.activateField(WorldLocation::Desert);game.dawnQuest=DawnQuest::AtTower;game.collectedFragments=1;game.player=locations[view];game.time=2.7f;game.messageTime=0;
  for(float s:{.8f,1.f,1.2f}){prepareCanvas(s);mouse={0,0};shot("desert-view-"+std::to_string(view+1)+"-"+std::to_string(int(s*100)));}
  refreshDisplayLayout();mouse={0,0};renderScene();BeginDrawing();presentCanvas();Image im=presentedImage();check(im.data&&im.width==GetRenderWidth()&&im.height==GetRenderHeight(),"desert view "+std::to_string(view+1)+": actual fullscreen framebuffer has correct dimensions");if(im.data){ExportImage(im,("qa-v33/desert-view-"+std::to_string(view+1)+"-display.png").c_str());UnloadImage(im);}EndDrawing();finishFramePacing();
 }
 reset();refreshDisplayLayout();report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}