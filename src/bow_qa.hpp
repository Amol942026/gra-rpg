#pragma once
bool runBowQA(){
 fs::create_directories("qa-v23");std::ofstream report("qa-v23/bow-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 auto reset=[](){game=Game{};game.props.clear();game.enemies.clear();game.forage.clear();game.player={15,15};game.messageTime=0;welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=false;dragging={};mouse={0,0};nearbyPage=0;};
 auto pointAt=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
 auto sector=[](int i){float a=(-90+i*45)*DEG2RAD;return Vector2{WheelCenter.x+std::cos(a)*145,WheelCenter.y+std::sin(a)*145};};
 reset();refreshDisplayLayout();check(!audioSystem.available,"bow diagnostic never opens audio device");
 openWeaponWheel();check(weaponWheelOpen&&!gameplayInputAllowed()&&!worldPointerAllowed({600,450}),"wheel pauses gameplay and blocks world attacks and inspection");
 mouse=sector(1);renderScene();saveTexture(canvas,"qa-v23/weapon-wheel.png");clickWeaponWheel();
 check(!weaponWheelOpen&&game.equipment[0].kind==Kind::Bow&&game.bag[4].kind==Kind::Sword&&game.ammoCount()==20,"clicking bow sector equips actual inventory bow and preserves sword plus ammo");
 mouse={250,125};leftClickGameplay({});check(!game.bowDrawing&&game.ammoCount()==20,"clicking ammo counter never starts world bow action");
 game.bag[10]=item(Kind::Axe,1);openWeaponWheel();pointAt(bagRect(10));clickWeaponWheel();mouse=sector(2);renderScene();saveTexture(canvas,"qa-v23/assign-inventory-weapon.png");finishDrag();
 check(game.weaponWheel[2].kind==Kind::Axe&&game.bag[10].kind==Kind::Axe&&dragging.place==Place::None,"dragging backpack weapon onto sector assigns it without consuming or duplicating inventory item");
 mouse=sector(2);clickWeaponWheel();check(game.equipment[0].kind==Kind::Axe&&game.bag[10].kind==Kind::Bow&&!weaponWheelOpen,"assigned weapon sector can switch actual equipment");
 openWeaponWheel();pointAt(bagRect(0));clickWeaponWheel();mouse=sector(3);finishDrag();
 check(game.weaponWheel[3].kind==Kind::None&&game.bag[0].count==5,"potion drag cannot create weapon shortcut");
 mouse=sector(1);clickWeaponWheel();game.messageTime=0;mouse={720,425};
 leftClickGameplay({});check(game.bowDrawing&&game.arrows.empty()&&game.ammoCount()==20&&game.swing==0,"LPM world press draws bow without sword attack or premature shot");
 for(int i=0;i<42;i++){game.aimBow(combatAimFromPointer());game.update(1.f/60,{},false);}
 check(std::abs(game.bowCharge()-.5f)<.002f&&game.arrows.empty(),"held LPM accumulates charge without automatic repeat shots");
 renderScene();saveTexture(canvas,"qa-v23/bow-half-draw.png");
 check(releaseWorldBow()&&!game.bowDrawing&&game.arrows.size()==1&&game.ammoCount()==19,"LPM release handler launches exactly one arrow and consumes one");
 game.updateArrows(.05f);renderScene();saveTexture(canvas,"qa-v23/arrow-in-flight.png");
 game.attackCd=0;mouse={720,425};leftClickGameplay({});pointAt(bagRect(0));
 check(!releaseWorldBow()&&!game.bowDrawing&&game.ammoCount()==19,"release over HUD cancels shot without ammunition cost");
 game.attackCd=0;mouse={720,425};leftClickGameplay({});openWeaponWheel();
 check(!game.bowDrawing&&game.ammoCount()==19,"opening wheel cancels drawn shot without ammo loss");closeWeaponWheel();
 pointAt(bagRect(5));leftClickGameplay({});check(dragging.place==Place::Bag&&!game.bowDrawing,"LPM on ammunition starts inventory drag instead of drawing bow");finishDrag();
 reset();game.selectWeapon(1);game.bag[5]={};mouse={720,425};leftClickGameplay({});
 check(!game.bowDrawing&&game.arrows.empty()&&game.ammoCount()==0,"empty quiver cannot fire through real LPM handler");
 renderScene();saveTexture(canvas,"qa-v23/empty-quiver.png");
 for(float factor:{.8f,1.f,1.2f}){reset();prepareCanvas(factor);openWeaponWheel();pointAt(bagRect(4));clickWeaponWheel();mouse=sector(3);finishDrag();check(game.weaponWheel[3].kind==Kind::Bow&&game.bag[4].kind==Kind::Bow,"weapon assignment works at every supported logical rendering scale");closeWeaponWheel();}
 RenderTexture2D sample=LoadRenderTexture(220,260);RenderTexture2D sheet=LoadRenderTexture(1300,700);
 BeginTextureMode(sheet);ClearBackground(Ink);txt("ŁUK I KOŁCZAN — OSIEM KIERUNKÓW",22,14,24,Gold);EndTextureMode();
 for(int row=0;row<3;row++)for(int d=0;d<8;d++){
  float charge=row==0?-1:row==1?.5f:1.f;AnimationState state;state.direction=static_cast<Direction>(d);state.idleTime=.7f;
  BeginTextureMode(sample);ClearBackground(BLANK);actor_art::draw(0,110,220,1,state,0,false,true,Kind::Bow,false,BLANK,nullptr,-1,-1,charge);EndTextureMode();
  Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);int opaque=0,clipped=0,mismatch=0,interior=0;
  for(int y=0;y<260;y++)for(int x=0;x<220;x++)if(pixels[y*220+x].a>230){opaque++;clipped+=x<2||x>=218||y<2||y>=258;}
  for(int y=8;y<252;y+=8)for(int x=8;x<212;x+=8){bool solid=true,empty=true;for(int dy:{-3,0,3})for(int dx:{-3,0,3}){int alpha=pixels[(y+dy)*220+x+dx].a;solid&=alpha>230;empty&=alpha<8;}
   if(!solid&&!empty)continue;Vector2 point{float(x)+.5f,float(y)+.5f};bool hit=actor_art::draw(0,110,220,1,state,0,false,true,Kind::Bow,false,BLANK,&point,-1,-1,charge);interior+=solid;mismatch+=solid?!hit:hit;
  }
  check(opaque>130&&clipped==0,"bow, quiver and drawn pose fit full silhouette in each direction");
  check(interior>2&&mismatch==0,"bow character GPU pixels match pointer geometry including back equipment");
  UnloadImageColors(pixels);UnloadImage(im);
  BeginTextureMode(sheet);float x=130+d*145.f,y=185+row*205.f;actor_art::draw(0,x,y,1,state,0,false,true,Kind::Bow,false,BLANK,nullptr,-1,-1,charge);center(TextFormat("%d  ·  %s",d,row==0?"spoczynek":row==1?"50%":"100%"),x,y+15,14,Muted);EndTextureMode();
 }
 saveTexture(sheet,"qa-v23/bow-8-directions.png");UnloadRenderTexture(sheet);UnloadRenderTexture(sample);
 reset();refreshDisplayLayout();report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
