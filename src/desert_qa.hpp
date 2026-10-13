#pragma once
bool runDesertQA(){
 fs::create_directories("qa-v39");std::ofstream report("qa-v39/desert-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 auto reset=[](){game=Game{};intro={};towerStory={};clearStoryMenus();developerActive=developerMenuOpen=false;game.messageTime=0;mouse={0,0};};
 auto shot=[&](const std::string& name){renderScene();saveTexture(canvas,"qa-v39/"+name+".png");};
 auto allocate=[](){if(game.attributePoints){Attributes p{};p[0]=game.attributePoints;game.distribute(p);}if(game.skillPoints){SkillRanks ranks{};ranks[0]=game.skillPoints;game.trainSkills(ranks);}};
 auto finishStory=[](){for(int i=0;i<260&&towerStory.active();i++)towerAction(false,true,1.f/60);};
 auto choice=[](){for(int i=0;i<12&&towerStory.phase==TowerPhase::Dialogue;i++){towerStory.revealed=true;towerStory.age=.2f;towerAction(true,false,0);}};
 auto locate=[](HoverKind kind,int index,V pos){Vector2 p=screen(pos);for(int y=-165;y<30;y+=3)for(int x=-120;x<120;x+=3){auto hit=pickWorld({p.x+x,p.y+y});if(hit.kind==kind&&hit.index==index){mouse={p.x+x,p.y+y};return true;}}return false;};
 reset();refreshDisplayLayout();check(!audioSystem.available,"desert tests open no audio device");check(desert_art::terrain.id&&desert_art::minimap.id,"second realm terrain and minimap are cached before gameplay");
 // New scenery is GPU cached, and wind keeps its physical world position.
 for(int variant=0;variant<4;variant++){
  auto& rt=desert_art::mountainArt[variant];Image im=LoadImageFromTexture(rt.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);int solid=0,clipped=0;std::vector<unsigned> colors;
  for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++){auto c=pixels[y*im.width+x];solid+=c.a>=240;clipped+=c.a>=160&&(x<2||y<2||x>=im.width-2||y>=im.height-2);if(c.a>=240&&x%7==0&&y%7==0)colors.push_back((unsigned(c.r)<<16)|(unsigned(c.g)<<8)|c.b);}
  std::sort(colors.begin(),colors.end());check(rt.id&&im.width==900&&im.height==660&&solid>10000&&clipped==0&&std::unique(colors.begin(),colors.end())-colors.begin()>100,"mountain variant "+std::to_string(variant)+": complete cached silhouette and rich shaded terrain");saveTexture(rt,"qa-v39/mountain-"+std::to_string(variant)+".png");UnloadImageColors(pixels);UnloadImage(im);
 }
 {
  RenderTexture2D rt=LoadRenderTexture(WorldW,H);auto wind=[&](V pos,float time){game.player=pos;game.time=time;BeginTextureMode(rt);ClearBackground(BLANK);desert_art::drawSandWind();EndTextureMode();Image im=LoadImageFromTexture(rt.texture);ImageFlipVertical(&im);return im;};
  Image first=wind({30,30},3.1f),shifted=wind({31,30},3.1f),later=wind({30,30},3.8f);Color* a=LoadImageColors(first),*b=LoadImageColors(shifted),*c=LoadImageColors(later);int samples=0,bad=0,moving=0,visible=0,soft=0;
  for(int y=110;y<H-110;y+=3)for(int x=90;x<WorldW-130;x+=3){auto p=a[(y+20)*WorldW+x+40],q=b[y*WorldW+x],old=a[y*WorldW+x],next=c[y*WorldW+x];int error=std::abs(int(p.r)-q.r)+std::abs(int(p.g)-q.g)+std::abs(int(p.b)-q.b)+std::abs(int(p.a)-q.a);bad+=error>12;samples++;visible+=old.a>3;soft+=old.a>3&&old.a<130;moving+=std::abs(int(old.a)-next.a)>3;}
  report<<"Wind: visible="<<visible<<"; translucent="<<soft<<"; moving="<<moving<<"; camera mismatches="<<bad<<" / "<<samples<<'\n';
  check(visible>500&&soft>visible*.98f,"wind: visible low-opacity dust veils and small grains remain translucent");check(samples>10000&&bad<samples*.005f,"wind: frozen-time GPU pixels follow a one-tile camera move by exactly 40x20 pixels");check(moving>500,"wind: actual GPU dust coverage evolves naturally over time");saveTexture(rt,"qa-v39/wind-layer.png");UnloadImageColors(a);UnloadImageColors(b);UnloadImageColors(c);UnloadImage(first);UnloadImage(shifted);UnloadImage(later);UnloadRenderTexture(rt);
 }
 reset();
 // The cheaper distant layer must preserve opaque coverage when composited.
 reset();game.player={2,2};desert_art::renderBackdrop(1.2f);
 {Image im=LoadImageFromTexture(desert_art::mountainBackdrop.texture);Color* p=LoadImageColors(im);int holes=0;for(int i=0;i<im.width*im.height;i++)holes+=p[i].a!=255;check(im.width>=840&&im.height>=720&&holes==0,"optimized mountain layer: every GPU pixel is opaque without compositing holes");UnloadImageColors(p);UnloadImage(im);}
 reset();
 // Verify new 2x obstacle masks against every GPU pixel and transparent margins.
 for(int type=ForestPropTypeCount;type<PropTypeCount;type++){
  Image im=LoadImageFromTexture(propArt[type].texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);int solid=0,clipCount=0,mismatch=0;std::vector<unsigned> colors;
  for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++){const auto& c=pixels[y*im.width+x];bool p=c.a>=160;solid+=p;clipCount+=p&&(x<3||y<3||x>=im.width-3||y>=im.height-3);mismatch+=(propMasks[type][y*im.width+x]!=0)!=p;if(p&&x%3==0&&y%3==0)colors.push_back((unsigned(c.r)<<16)|(unsigned(c.g)<<8)|c.b);}
  std::sort(colors.begin(),colors.end());int count=std::unique(colors.begin(),colors.end())-colors.begin();
  check(im.width==360&&im.height==460&&solid>500&&clipCount==0,"desert prop "+std::to_string(type)+": detailed 2x model has no cut-off edges");check(count>=(type==11?4:5)&&mismatch==0,"desert prop "+std::to_string(type)+": rich material shades and physical mouse mask agree with GPU pixels");UnloadImageColors(pixels);UnloadImage(im);
 }
 // GPU checks of smooth lighting and opaque joins, independent of rig internals.
 {
  RenderTexture2D rt=LoadRenderTexture(256,220);BeginTextureMode(rt);ClearBackground(BLANK);
  actor_art::Rig sphere(128,174,1,Direction::S);sphere.setSmoothSurfaces();sphere.rounded({0,0,40},{110,80,80},rgb(210,170,105));sphere.draw();EndTextureMode();
  Image im=LoadImageFromTexture(rt.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);std::vector<unsigned> colors;int jump=0;
  for(int x=88;x<168;x++){auto a=pixels[133*256+x],b=pixels[133*256+x+1];colors.push_back((unsigned(a.r)<<16)|(unsigned(a.g)<<8)|a.b);jump=std::max(jump,std::max({std::abs(int(a.r)-b.r),std::abs(int(a.g)-b.g),std::abs(int(a.b)-b.b)}));}
  std::sort(colors.begin(),colors.end());check(std::unique(colors.begin(),colors.end())-colors.begin()>35&&jump<9,"rounded sand surface: light changes continuously across GPU pixels without flat facet bands");UnloadImageColors(pixels);UnloadImage(im);
  BeginTextureMode(rt);ClearBackground(BLANK);actor_art::Rig overlap(128,128,1,Direction::S);overlap.setSmoothSurfaces();overlap.box({0,20,10},{60,20,20},BLUE);overlap.box({0,0,10},{60,20,20},RED);overlap.draw();EndTextureMode();
  im=LoadImageFromTexture(rt.texture);ImageFlipVertical(&im);Color pixel=GetImageColor(im,128,126);check(pixel.b>100&&pixel.r<30,"intersecting surfaces: the nearer blue surface hides the red rear surface at the actual pixel");UnloadImage(im);UnloadRenderTexture(rt);
 }
 // Fallen bodies retain their physical picking silhouettes after rounding.
 for(int type=8;type<EnemyTypeCount;type++){
  RenderTexture2D rt=LoadRenderTexture(300,150);BeginTextureMode(rt);ClearBackground(BLANK);Enemy e{100+type,type,{},{},0,100};e.animation.direction=Direction::SE;corpseArt(e,150,98);EndTextureMode();Image im=LoadImageFromTexture(rt.texture);ImageFlipVertical(&im);Color* p=LoadImageColors(im);int drawn=0,cut=0,bad=0,probes=0;

  for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++){bool solid=p[y*im.width+x].a>=240;drawn+=solid;cut+=solid&&(x<3||y<3||x>=im.width-3||y>=im.height-3);}
  for(int y=3;y<im.height-3;y+=4)for(int x=3;x<im.width-3;x+=4){bool opaque=true,empty=true;for(int dy:{-2,0,2})for(int dx:{-2,0,2}){auto a=p[(y+dy)*im.width+x+dx].a;opaque&=a>=240;empty&=a<4;}if(!opaque&&!empty)continue;Vector2 point{x+.5f,y+.5f};bool picked=corpseArt(e,150,98,BLANK,&point);probes++;bad+=opaque?!picked:picked;}
  check(drawn>200&&cut==0&&probes>1000&&bad==0,"rounded corpse "+std::to_string(type)+": full GPU silhouette and physical picking agree");saveTexture(rt,"qa-v39/corpse-"+std::to_string(type)+".png");UnloadImageColors(p);UnloadImage(im);UnloadRenderTexture(rt);
 }
 // Same physical rigs draw, turn, animate and receive mouse inspection in all directions.
 for(int type=8;type<EnemyTypeCount;type++){
  RenderTexture2D sheet=LoadRenderTexture(1120,570);BeginTextureMode(sheet);ClearBackground(Ink);txt(enemyInfo(type).name,22,13,25,Gold);
  for(int direction=0;direction<8;direction++){float x=direction%4*280.f+140,y=direction/4*260.f+246;AnimationState state;state.direction=Direction(direction);state.moving=true;state.phase=.17f;state.idleTime=2.7f;actor_art::draw(type+1,x,y,1,state);center(TextFormat("KIERUNEK %d",direction+1),x,y+18,14,Muted);}
  EndTextureMode();saveTexture(sheet,"qa-v39/enemy-"+std::to_string(type)+"-directions.png");UnloadRenderTexture(sheet);
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
 for(int i=0;i<4;i++){float x=i*300.f;rect(x+7,55,286,300,Panel);Enemy e{100+i,8+i,{},{},100,100};e.animation.idleTime=2.7f;actor_art::draw(e.type+1,x+111,287,1,e.animation);hero(x+237,287,1);center(enemyInfo(e.type).name,x+150,310,19,Text);center(TextFormat("HP %.0f · ATAK %d",enemyInfo(e.type).health,enemyInfo(e.type).attack),x+150,338,14,Gold);}EndTextureMode();saveTexture(models,"qa-v39/desert-enemies.png");UnloadRenderTexture(models);
 models=LoadRenderTexture(1200,650);BeginTextureMode(models);ClearBackground(Ink);txt("MORZE PIASKÓW — ROŚLINY I RUINY",23,13,24,Gold);
 for(int i=0;i<7;i++){float x=i%4*300.f,y=54+i/4*286.f;rect(x+7,y+4,286,278,Panel);auto& tex=propArt[7+i];DrawTexturePro(tex.texture,{0,0,float(tex.texture.width),-float(tex.texture.height)},{x+24,y+15,180,230},{0,0},0,WHITE);hero(x+238,y+223,1);center(propInfo(7+i).name,x+150,y+251,17,Text);}EndTextureMode();saveTexture(models,"qa-v39/desert-props.png");UnloadRenderTexture(models);
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
  refreshDisplayLayout();mouse={0,0};renderScene();BeginDrawing();presentCanvas();Image im=presentedImage();check(im.data&&im.width==GetRenderWidth()&&im.height==GetRenderHeight(),"desert view "+std::to_string(view+1)+": actual fullscreen framebuffer has correct dimensions");if(im.data){ExportImage(im,("qa-v39/desert-view-"+std::to_string(view+1)+"-display.png").c_str());UnloadImage(im);}EndDrawing();finishFramePacing();
 }
 // All four sides and four corners must contain mountain relief outside land.
 const std::array<V,8> borders={{{2,2},{2,35},{2,69},{35,69},{69,69},{69,35},{69,2},{35,2}}};
 for(int view=0;view<int(borders.size());view++){
  reset();game.activateField(WorldLocation::Desert);game.player=borders[view];game.time=3.1f;
  RenderTexture2D rt=LoadRenderTexture(WorldW,H);BeginTextureMode(rt);ClearBackground(BLANK);desert_art::drawBackdrop(false);EndTextureMode();Image plain=LoadImageFromTexture(rt.texture);ImageFlipVertical(&plain);Color* a=LoadImageColors(plain);
  BeginTextureMode(rt);ClearBackground(BLANK);desert_art::drawBackdrop();desert_art::drawFoothills();EndTextureMode();Image mountains=LoadImageFromTexture(rt.texture);ImageFlipVertical(&mountains);Color* b=LoadImageColors(mountains);int outside=0,covered=0;
  for(int y=10;y<H-10;y+=5)for(int x=10;x<WorldW-10;x+=5){V p=toWorld({float(x),float(y)});if(p.x>=.5f&&p.y>=.5f&&p.x<=MapSize-1.5f&&p.y<=MapSize-1.5f)continue;outside++;auto c=a[y*WorldW+x],d=b[y*WorldW+x];covered+=std::abs(int(c.r)-d.r)+std::abs(int(c.g)-d.g)+std::abs(int(c.b)-d.b)>9;}
  report<<"Border "<<view<<": outside samples="<<outside<<"; mountain pixels="<<covered<<'\n';check(outside>1000&&covered>outside*.8f,"map border "+std::to_string(view)+": mountains fill the exterior on GPU");UnloadImageColors(a);UnloadImageColors(b);UnloadImage(plain);UnloadImage(mountains);UnloadRenderTexture(rt);
  prepareCanvas(1);mouse={0,0};shot("desert-border-"+std::to_string(view+1)+"-100");if(view==0)saveTexture(worldCanvas,"qa-v39/desert-mountains-preview.png");if(view==0||view==4)for(float size:{.8f,1.2f}){prepareCanvas(size);shot("desert-border-"+std::to_string(view+1)+"-"+std::to_string(int(size*100)));}
 }
 reset();game.activateField(WorldLocation::Desert);game.player={30,30};game.time=2.5f;RenderTexture2D rt=LoadRenderTexture(WorldW,H);
 auto land=[&](bool backdrop){BeginTextureMode(rt);ClearBackground(BLANK);if(backdrop)desert_art::drawBackdrop();Vector2 origin=screen({0,0});DrawTextureRec(desert_art::terrain.texture,{0,0,float(TerrainWidth),-float(TerrainHeight)},{origin.x-TerrainOriginX,origin.y-90},WHITE);EndTextureMode();Image im=LoadImageFromTexture(rt.texture);ImageFlipVertical(&im);return im;};
 Image before=land(false),after=land(true);Color* a=LoadImageColors(before),*b=LoadImageColors(after);int samples=0,bad=0;for(int y=5;y<H-5;y+=7)for(int x=5;x<WorldW-5;x+=7){int index=y*WorldW+x;if(a[index].a<255)continue;samples++;bad+=a[index].r!=b[index].r||a[index].g!=b[index].g||a[index].b!=b[index].b;}check(samples>10000&&bad==0,"mountain background remains behind every opaque pixel of playable desert land");UnloadImageColors(a);UnloadImageColors(b);UnloadImage(before);UnloadImage(after);UnloadRenderTexture(rt);

 // Baked roads remain opaque and distinguishable from surrounding sand.
 {
  Image im=LoadImageFromTexture(desert_art::terrain.texture);ImageFlipVertical(&im);int opaque=0,samples=0;double roadGreen=0,sandGreen=0;
  for(const auto& road:desertRoads())for(size_t i=10;i+10<road.points.size();i+=4){
   V p=road.points[i];if(distance(p,DesertOasis)<7||distance(p,DesertBossHome)<7)continue;
   V tangent=normalized(road.points[i+1]-road.points[i-1]),normal{-tangent.y,tangent.x};
   auto pixel=[&](V v){return GetImageColor(im,int(TerrainOriginX+(v.x-v.y)*40),int(90+(v.x+v.y)*20));};
   Color center=pixel(p),side=pixel(p+normal*2.8f);opaque+=center.a==255;samples++;roadGreen+=center.g;sandGreen+=side.g;
  }
  report<<"Road GPU samples: "<<samples<<"; center green: "<<roadGreen/samples<<"; nearby sand green: "<<sandGreen/samples<<'\n';
  check(samples>35&&opaque==samples,"caravan roads: actual baked path pixels remain fully opaque");
  check(roadGreen/samples+7<sandGreen/samples,"caravan roads: curved worn sand remains visibly distinct from surrounding terrain");
  UnloadImage(im);
 }
 // Actual captured hero, mouse/keyboard handlers and all visible trap stages.
 reset();game.activateField(WorldLocation::Desert);game.props.clear();
 game.enemies.erase(std::remove_if(game.enemies.begin(),game.enemies.end(),[](const Enemy& e){return e.id!=DesertBossId;}),game.enemies.end());
 game.player=DesertBossHome+V{3.7f,0};game.skillRanks={1,1,1};game.enemies[0].attack=100;game.enemies[0].sandPrisonCd=.05f;
 game.update(.05f,{},false);
 check(game.sandTrapped()&&gameplayInputAllowed(),"sand prison: simulation and inspection remain active while offensive model actions are blocked");
 Vector2 pointer=screen(game.player+V{0,3});mouse={pointer.x,pointer.y-35};auto random=game.combatRng;float mana=game.mana;int ammo=game.ammoCount();
 check(!clickWorldOrInventory()&&!castGameplaySpell()&&!activateGameplaySkill(Skill::StrongCut),"sand prison: real mouse sword, C spell and skill handlers cannot attack");
 activateMouseAction(4,{1,0});check(!game.rolling()&&game.mana==mana&&game.combatRng==random,"sand prison: real dodge handler and rejected attacks preserve resources");
 game.selectWeapon(1);check(!clickWorldOrInventory()&&!releaseWorldBow()&&!activateGameplaySkill(Skill::PiercingArrow)&&game.ammoCount()==ammo,"sand prison: mouse draw/release and piercing skill handlers cannot shoot");
 game.selectWeapon(0);game.messageTime=0;mouse={0,0};
 for(int layer=0;layer<2;layer++){
  auto& rt=sand_prison::mound[layer];Image im=LoadImageFromTexture(rt.texture);Color* p=LoadImageColors(im);int drawn=0,clipped=0;std::vector<unsigned> colors;
  for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++){auto c=p[y*im.width+x];drawn+=c.a>240;clipped+=c.a>20&&(x<2||y<2||x>=im.width-2||y>=im.height-2);if(c.a>240&&x%3==0&&y%3==0)colors.push_back((unsigned(c.r)<<16)|(unsigned(c.g)<<8)|c.b);}
  std::sort(colors.begin(),colors.end());check(rt.id&&drawn>1000&&clipped==0&&std::unique(colors.begin(),colors.end())-colors.begin()>60,"sand prison: cached mound layer "+std::to_string(layer)+" has detailed soft shading and complete margins");saveTexture(rt,"qa-v39/sand-mound-"+std::to_string(layer)+".png");UnloadImageColors(p);UnloadImage(im);
 }
 game.time=12.3f;
 for(int stage=0;stage<3;stage++){
  game.sandPrisonRemaining=stage==0?2.95f:stage==1?2.4f:.15f;game.enemies[0].sandCastPulse=stage==1?.38f:0;
  for(float scale:{.8f,1.f,1.2f}){prepareCanvas(scale);shot("sand-prison-"+std::to_string(stage)+"-"+std::to_string(int(scale*100)));}
 }refreshDisplayLayout();
 game.sandPrisonRemaining=2.4f;game.enemies[0].sandCastPulse=.38f;game.messageTime=0;shot("sand-prison-preview");
 saveTexture(worldCanvas,"qa-v39/sand-prison-world.png");
 paused=true;float frozen=game.sandPrisonRemaining;input(.05f);
 check(game.sandPrisonRemaining==frozen&&!gameplayInputAllowed(),"sand prison: actual pause menu freezes the trap rather than letting it expire");
 paused=false;
 check(locate(HoverKind::Enemy,DesertBossId,game.enemies[0].pos),"sand prison: boss casting pose uses the same physical mouse picking geometry");
 V held=game.player;for(int i=0;i<48;i++)game.update(.05f,{1,0},false,true);
 check(!game.sandTrapped()&&distance(game.player,held)<.001f,"sand prison: actual game update releases without moving before the three-second boundary");
 game.update(.05f,{1,0},false,true);check(distance(game.player,held)>.1f&&clickWorldOrInventory(),"sand prison: movement and actual sword click are restored on release");
 shot("sand-prison-released");
 reset();game.activateField(WorldLocation::Desert);game.player={24,23};game.time=3.1f;game.messageTime=0;mouse={0,0};
 shot("desert-roads-preview");saveTexture(worldCanvas,"qa-v39/desert-roads-world.png");
 reset();
 // Terrain-height apron and uncut foreground ridges at all near edges.
 check(desert_art::outsideGroundShader.id!=rlGetShaderIdDefault()&&desert_art::groundOriginLocation>=0,"outside sand shader loads successfully with world-position uniforms");
 for(V view:{V{69,35},V{35,69},V{69,69}}){
  reset();game.activateField(WorldLocation::Desert);game.player=view;
  RenderTexture2D rt=LoadRenderTexture(WorldW,H);BeginTextureMode(rt);ClearBackground(BLANK);desert_art::drawFoothills();EndTextureMode();
  Image im=LoadImageFromTexture(rt.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);int rising=0;
  for(int y=20;y<H-20;y+=3)for(int x=20;x<WorldW-20;x+=3){V p=toWorld({float(x),float(y)});
   if(p.x>=.5f&&p.y>=.5f&&p.x<=MapSize-1.5f&&p.y<=MapSize-1.5f)rising+=pixels[y*WorldW+x].a>220;
  }
  check(rising>500,"foreground slopes rise above the projected map plane without being sliced off by terrain");
  UnloadImageColors(pixels);UnloadImage(im);UnloadRenderTexture(rt);prepareCanvas(1);mouse={0,0};shot("mountain-join-"+std::to_string(int(view.x))+"-"+std::to_string(int(view.y)));
  if(view.x==69&&view.y==35)saveTexture(worldCanvas,"qa-v39/mountain-join-world.png");
 }
 reset();game.activateField(WorldLocation::Desert);game.player={66,43};game.time=3.1f;game.messageTime=0;
 for(float resolution:{.8f,1.f,1.2f}){prepareCanvas(resolution);shot("reported-mountain-border-"+std::to_string(int(resolution*100)));}
 reset();refreshDisplayLayout();report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}