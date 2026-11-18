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
 for(int i=0;i<int(std::lround((BowNockDuration+.5f*BowDrawDuration)*60));i++){game.aimBow(combatAimFromPointer());game.update(1.f/60,{},false);}
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
 reset();game.selectWeapon(1);mouse={720,425};leftClickGameplay({});
 check(releaseWorldBow()&&game.bowReleaseQueued&&game.arrows.empty(),"quick world click queues the shot without an early projectile");
 const auto tapAim=game.bowAim;pointAt(bagRect(0));
 check(!releaseWorldBow()&&game.bowReleaseQueued,"moving the cursor over inventory after releasing cannot discard the queued shot");
 for(int frame=0;frame<60&&game.bowDrawing;frame++)game.update(1.f/60,{},false);
 check(game.arrows.size()==1&&game.ammoCount()==19&&!game.arrows[0].power&&distance(normalized(game.arrows[0].velocity),tapAim)<.0001f,
       "a released world click completes one weak shot along its original aim");
 reset();game.selectWeapon(1);mouse={720,425};leftClickGameplay({});releaseWorldBow();openWeaponWheel();
 check(!game.bowDrawing&&!game.bowReleaseQueued&&game.arrows.empty()&&game.ammoCount()==20,"opening the wheel still cancels a queued shot without cost");closeWeaponWheel();
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
 {
  namespace bone=hero_motion::data;
  auto pose=hero_motion::evaluate(.18f,0,0,.7f);
  const auto jaw=pose.point(bone::Spine,{-10.7f,5.7f,61.8f});
  auto held=hero_bow::sampleShot(1,-1);
  hero_bow::apply(pose,held);
  const auto wrist=pose.world[hero_quiver::DrawHand].p,elbow=pose.world[bone::ForearmL].p;
  check(hero_motion::length(wrist-jaw)<.2f&&elbow.x<wrist.x-3.f,"the fingers anchor outside the right cheek and the drawing elbow opens beside the ribs");
  check(std::abs(hero_motion::length(elbow-wrist)-sword_pose::Forearm)<.02f,"the draw retains full forearm length");
  for(float charge:{.045f,.38f,1.f}){
   auto before=hero_bow::sampleShot(charge,-1),loose=hero_bow::sampleShot(charge,0);
   check(hero_motion::length(loose.nock-before.nock)<.001f&&std::abs(loose.lift-before.lift)<.001f,"releasing a partial or full draw starts from its held hand position");
   auto recoil=hero_bow::sampleShot(charge*.75f,.25f);
   check(recoil.nock.y<before.nock.y&&recoil.stringContact==0,"the released string snaps away while the fingers follow through behind their anchor");
  }
 }
 {
  using namespace hero_quiver;
  const auto mouth=Nock+Shaft*MouthDepth;
  for(int ammo:{0,1,2,3,20}){
   auto before=stock(ammo,false,false),picked=stock(ammo,true,false);
   auto loose=stock(std::max(0,ammo-1),true,true);
   bool continuous=before.count==std::min(ammo,3)&&picked.inHand==(ammo>0)&&!loose.inHand&&
        picked.count+int(picked.inHand)==before.count&&loose.count==std::min(std::max(0,ammo-1),3);
   for(int i=0;i<picked.count;i++)
    continuous&=hero_motion::length(storedNock(i,true,-1)-storedNock(i,false,0))<.001f;
   for(int i=picked.count;i<loose.count;i++)
    continuous&=exposedLength(storedNock(i,false,0),mouth,Shaft)<.001f;
   check(continuous,"picking and releasing one arrow preserves every visible reserve shaft, including the last arrow");
  }
  for(Kind kind:{Kind::Bow,Kind::ReedBow,Kind::GroveBow,Kind::RiftBow}){
   const auto profile=weapon_profiles::bow(kind);
   const float brace=profile.curve-profile.recurve*.29f;
   for(float moving:{0.f,1.f}){
    auto poseAt=[&](float phase,float charge){
     auto pose=hero_motion::evaluate(.27f,moving,0,.31f);
     const auto shot=hero_bow::sampleShot(charge,-1,phase);
     if(shot.lift<1)hero_carry::bow(pose);
     hero_bow::apply(pose,shot,brace);
     return pose;
    };
    auto arrowAt=[&](const hero_motion::Pose& pose,float phase,float charge){
     const auto shot=hero_bow::sampleShot(charge,-1,phase);
     return carried(pose,shot.arrowAxis,shot.arrowGripOffset,phase);
    };
    namespace b=hero_motion::data;
    auto start=poseAt(Pickup,0);
    auto touched=arrowAt(start,Pickup,0);
    check(hero_motion::length(touched.nock-start.point(b::Spine,Nock))<.02f&&
          hero_motion::length(touched.axis-start.vector(b::Spine,Shaft))<.001f&&
          hero_motion::length(touched.across-start.vector(b::Spine,featherAcross(Shaft)))<.001f,
          "right-hand pickup preserves the exact position, shaft axis and feather angle of the selected quiver arrow");
    float previous=MouthDepth;
    bool smooth=true;
    for(int step=0;step<=60;step++){
     const float phase=Pickup+(1-Pickup)*step/60.f;
     auto pose=poseAt(phase,0);auto arrow=arrowAt(pose,phase,0);
     auto shot=hero_bow::sampleShot(0,-1,phase);
     smooth&=hero_motion::length(arrow.nock+arrow.axis*shot.arrowGripOffset-pose.world[DrawHand].p)<.01f&&
             std::abs(hero_motion::length(arrow.axis*ArrowLength)-ArrowLength)<.01f&&
             arrow.exposed>=previous-.01f;
     previous=arrow.exposed;
    }
    auto clear=poseAt(Clear,0);auto freed=arrowAt(clear,Clear,0);
    check(smooth&&freed.exposed>=ArrowLength-.01f,"one rigid arrow emerges tip-last through the mouth and stays in the right fingers across the transfer");
    auto ready=poseAt(1,0);auto seated=arrowAt(ready,1,0);
    auto prior=arrowAt(poseAt(.99999f,0),.99999f,0);
    const auto string=ready.world[BowHand].p-ready.vector(BowHand,{0,brace,0});
    check(hero_motion::length(seated.nock-string)<.02f&&hero_motion::length(seated.axis-ready.vector(BowHand,{0,1,0}))<.002f&&
          hero_motion::length(seated.nock-prior.nock)<.02f&&hero_motion::length(seated.axis-prior.axis)<.002f,
          "arrow seats on this bow's relaxed string and rest continuously at the boundary between preparation and draw");
    bool leavesFingers=true;
    for(float charge:{BowMinDrawTime/BowDrawDuration,.59f,1.f}){
     auto held=arrowAt(poseAt(1,charge),1,charge);
     for(int direction=0;direction<8;direction++){
      const float angle=direction*AnimationPi/4;
      actor_art::Rig hero(0,0,1,Direction(direction)),projectile(0,0,1,Direction::S);
      Arrow shot{};shot.velocity={12,5};shot.remaining=Game::bowRange(charge);
      shot.launch={charge,.27f,moving,.31f,angle,kind};
      auto flying=arrow_art::flight(shot);
      auto close=[&](hero_motion::Vec a,hero_motion::Vec b){
       auto x=hero.project({a.x,a.y,a.z}),y=projectile.project({b.x,b.y,b.z});
       return std::hypot(x.x-y.x,x.y-y.y)<.01f;
      };
      leavesFingers&=close(held.nock,flying.nock)&&close(held.nock+held.axis*ArrowLength,flying.nock+flying.axis*ArrowLength)&&
                      close(held.nock+held.across*2.5f,flying.nock+flying.across*2.5f);
     }
    }
    check(leavesFingers,"projectile starts with precisely the held nock, tip and feather orientation for all eight facings and draw strengths");
   }
  }
 }
 reset();refreshDisplayLayout();report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
