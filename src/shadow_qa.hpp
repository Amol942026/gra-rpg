#pragma once
bool runShadowQA(){
 fs::create_directories("qa-v40");std::ofstream report("qa-v40/shadow-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 Game saved=game;auto oldIntro=intro;auto oldStory=towerStory;bool oldWelcome=welcome;welcome=false;intro={};towerStory={};
 refreshDisplayLayout();world_shadows::init();check(!audioSystem.available,"shadow QA never opens the audio device");
 check(IsShaderValid(world_shadows::maskShader)&&IsShaderValid(world_shadows::softShader),"silhouette and soft-edge shaders are valid");
 constexpr int Width=640,Height=260;
 RenderTexture2D sample=LoadRenderTexture(Width,Height);
 auto begin=[&]{
  actor_art::beginShadowFrame();BeginTextureMode(sample);ClearBackground(BLANK);
  // Billboard projection inverts the winding. Use the same state as prepare().
  BeginShaderMode(world_shadows::maskShader);rlDisableBackfaceCulling();
 };
 auto end=[&]{
  rlDrawRenderBatchActive();rlEnableBackfaceCulling();EndShaderMode();EndTextureMode();actor_art::endShadowPreparation();
  Image image=LoadImageFromTexture(sample.texture);ImageFlipVertical(&image);return image;
 };
 auto valid=[&](Image im){
  Color* pixels=LoadImageColors(im);int solid=0,clipped=0;float cx=0,cy=0;bool opacity=true;
  for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++){const auto& c=pixels[y*im.width+x];if(c.a>200){solid++;cx+=x;cy+=y;clipped+=x<2||y<2||x>=im.width-2||y>=im.height-2;opacity&=c.a==255&&c.r==255&&c.g==255;}}
  check(solid>65&&clipped==0,"projected silhouette is present and is not clipped");
  check(opacity,"overlapping mesh faces produce a single opaque mask without dark seams");
  UnloadImageColors(pixels);return std::array<float,3>{float(solid),cx/std::max(1,solid),cy/std::max(1,solid)};
 };
 // Camera-hidden faces must still cast: check a box against an analytic ground projection.
 begin();actor_art::Rig box(100,90,1,Direction::E);box.box({0,0,45},{18,14,20},WHITE);box.drawShadow();Image prism=end();
 Color* pix=LoadImageColors(prism);auto expected=box.groundProject({0,0,45});
 check(pix[int(expected.y)*Width+int(expected.x)].a==255,"a physical box covers its analytically projected centre");UnloadImageColors(pix);UnloadImage(prism);
 for(int species=0;species<=EnemyTypeCount;species++)for(int d=0;d<8;d++){
  AnimationState state;state.direction=Direction(d);state.moving=true;state.phase=.17f;
  auto cast=[&]{
   auto draw=[&]{actor_art::draw(species,135,85,1,state,0,species==0,species==0);};
   if(regionalEnemy(species-1)||marshEnemy(species-1))world_shadows::billboard(85,draw);
   else draw();
  };
  begin();shadow_geometry::beginBatch();cast();shadow_geometry::flushBatch(world_shadows::maskShader);Image im=end();valid(im);
  actor_art::shadowCull=false;begin();cast();Image full=end();actor_art::shadowCull=true;
  Color* a=LoadImageColors(im),*b=LoadImageColors(full);int difference=0;
  for(int i=0;i<im.width*im.height;i++)difference+=(a[i].a>200)!=(b[i].a>200);
  check(difference<=5,"GPU batch and optimized light-facing geometry retain the complete physical shadow silhouette");
  UnloadImageColors(a);UnloadImageColors(b);UnloadImage(full);UnloadImage(im);
 }actor_art::finishShadowFrame();
 // Real attack and movement poses must alter the shadow, not only its position.
 auto actorImage=[&](Kind weapon,float phase,float strike,float roll,float charge){
  AnimationState state;state.direction=Direction::SE;state.moving=true;state.phase=phase;state.runBlend=1;
  begin();actor_art::draw(0,135,85,1,state,strike>=0?std::sin(strike*PI):0,true,true,weapon,false,BLANK,nullptr,roll,strike,charge,20);return end();
 };
 Image idle=actorImage(Kind::Sword,0,-1,-1,-1);
 auto differs=[&](Image a,Image b){Color* p=LoadImageColors(a),*q=LoadImageColors(b);int n=0;for(int i=0;i<a.width*a.height;i++)n+=(p[i].a>200)!=(q[i].a>200);UnloadImageColors(p);UnloadImageColors(q);return n;};
 for(auto mode:std::array<std::array<float,3>,3>{{{.25f,-1,-1},{.12f,.45f,-1},{.15f,-1,.34f}}}){
  Image frame=actorImage(Kind::Sword,mode[0],mode[1],mode[2],-1);check(differs(idle,frame)>35,"run, sword swing and roll change the projected physical silhouette");UnloadImage(frame);
 }
 Image bow=actorImage(Kind::Bow,.17f,-1,-1,.9f);check(differs(idle,bow)>70,"bow and quiver are included in the shadow");UnloadImage(bow);UnloadImage(idle);actor_art::finishShadowFrame();
 game=Game(47);game.props.clear();game.enemies.clear();game.forage.clear();game.ground.clear();game.player={25,25};
 for(int type=0;type<PropTypeCount;type++){
  Prop prop{toWorld({propArtScale(type)>1?185.f:120.f,85}),type,1};
  begin();world_shadows::prop(prop);Image im=end();valid(im);
  Image source{};
  if(type==13&&volcano_art::gateFrame().id)source=LoadImageFromTexture(volcano_art::gateFrame());
  else if(volcanoSceneryProp(type)){
   source=LoadImageFromTexture(volcano_art::propTexture(prop));
   ImageCrop(&source,volcano_art::source(prop));
  }else{
   source=LoadImageFromTexture(propArt[type].texture);ImageFlipVertical(&source);
  }
  Color* art=LoadImageColors(source),*shadow=LoadImageColors(im);int compared=0,matched=0;
  const float size=propArtSize(prop);
  for(int y=4;y<source.height-4;y+=11)for(int x=4;x<source.width-4;x+=11)if(art[y*source.width+x].a>240){
   float sy=(y+.5f)*230/source.height,sx=(x+.5f)*180/source.width;
   Vector2 base=screen(prop.pos);float bend=tree_wind::moves(type)?treeShift(prop,sy,game.time):0;
   Vector2 q=world_shadows::projected({base.x+(sx-90+bend)*size,base.y+(sy-208)*size},base.y);
   if(type>=MarshPropStart){
    auto point=marsh_footprints::castPoint(marsh_footprints::profiles[type-MarshPropStart],sx,sy);
    q={base.x+(point.x-90)*size,base.y+(point.y-208)*size};
   }
   bool found=false;
   if(q.x<2||q.x>=Width-2||q.y<2||q.y>=Height-2)continue;
   for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++)found|=shadow[(int(q.y)+dy)*Width+int(q.x)+dx].a>150;
   compared++;matched+=found;
  }
  check(compared>8&&matched>=compared*.98f,"object shadow follows the actual alpha silhouette and matching wind deformation");
  UnloadImageColors(art);UnloadImageColors(shadow);UnloadImage(source);UnloadImage(im);
 }
 {
  Image contact=LoadImageFromTexture(marsh_art::contacts);
  for(int id=0;id<int(std::size(marsh_footprints::profiles));id++){
   const auto& profile=marsh_footprints::profiles[id];bool meets=contact.data!=nullptr;
   for(int i=0;i<profile.count&&meets;i++){
    const auto& foot=profile.feet[i];const auto q=marsh_footprints::castPoint(profile,foot.x,foot.y);
    const int x=int((id%marsh_art::ContactColumns*marsh_art::ContactWidth+foot.x-marsh_art::ContactLeft)*marsh_art::ContactDensity);
    const int y=int((id/marsh_art::ContactColumns*marsh_art::ContactHeight+foot.y-marsh_art::ContactTop)*marsh_art::ContactDensity);
    meets=q.x==foot.x&&q.y==foot.y&&GetImageColor(contact,x,y).a>=170;
   }
   check(meets,"marsh contact and cast shadow meet at every painted bearing point");
  }
  if(contact.data)UnloadImage(contact);
 }
 for(int frame=0;frame<treasure_art::States;frame++){
  const int id=frame?marsh_footprints::OpenChest:marsh_footprints::ClosedChest;
  constexpr Vector2 base{170,130};
  begin();
  world_shadows::marshSprite(id,treasure_art::atlas,{float(frame*256),768,256,256},
                            {-38,25,256,256},base,treasure_art::Scale);
  Image im=end();valid(im);
  bool touches=true;const auto& p=marsh_footprints::profiles[id];
  for(int i=0;i<p.count;i++){
   const int x=int(base.x+(p.feet[i].x-90)*treasure_art::Scale);
   const int y=int(base.y+(p.feet[i].y-208)*treasure_art::Scale);
   bool ink=false;
   for(int dy=-3;dy<=2;dy++)for(int dx=-2;dx<=2;dx++)ink|=GetImageColor(im,x+dx,y+dy).a>200;
   touches&=ink;
  }
  check(touches,"real marsh chest shadow meets its base in the closed, open and emptied frames");
  UnloadImage(im);
 }
 Prop windTree{toWorld({120,85}),4,1};game.time=0;begin();world_shadows::prop(windTree);Image first=end();
 game.time=2.5f;begin();world_shadows::prop(windTree);Image second=end();
 check(differs(first,second)>35,"tree shadow sways with its canopy on the same wind field");UnloadImage(first);UnloadImage(second);
 for(const auto& sample:{std::pair{4,45.f},std::pair{2,150.f},std::pair{ForestDecorationStart+1,157.f},
                         std::pair{12,45.f},std::pair{DesertDecorationStart+2,154.f}}){
  Prop plant{{22.5f,19.25f},sample.first,1};
  float lo=0,hi=0;
  for(int frame=0;frame<=60;frame++){
   const float time=frame*.12f,shift=treeShift(plant,sample.second,time);
   lo=std::min(lo,shift);hi=std::max(hi,shift);
  }
  check(tree_wind::moves(plant.type)&&hi-lo>4&&treeShift(plant,208,2)==0,
        "trees and low plants have readable sway and a fixed root");
 }
 check(!tree_wind::moves(1)&&!tree_wind::moves(3)&&!tree_wind::moves(VolcanoDecorationStart+1),
       "wind leaves rocks, ruins and charred trunks fixed");
 Arrow flying{};flying.pos=toWorld({135,85});flying.velocity={6,0};
 begin();world_shadows::arrow(flying);Image arrowShadow=end();
 Color* arrowPixels=LoadImageColors(arrowShadow);
 auto impact=shadow_math::ground(135,85,35);
 Vector2 flightDirection{40,20};float flightLength=std::hypot(flightDirection.x,flightDirection.y);
 int sx=int(impact.x-flightDirection.x/flightLength*13),sy=int(impact.y-flightDirection.y/flightLength*13);
 check(arrowPixels[sy*Width+sx].a>200,"flying arrow casts its shaft, head and feathers from its physical height");
 UnloadImageColors(arrowPixels);UnloadImage(arrowShadow);
 // Static mesh shadows are already prepared; no mesh work is needed in combat.
 for(int type=0;type<tower_furniture::ObjectCount;type++){
  check(tower_furniture::groundShadows[type].art.id!=0,"each physical tower furnishing has a baked geometry shadow");
 }
 for(int direction:{1,-1})check(tower_stairs::groundShadows[tower_stairs::index(direction)].art.id!=0,"both stairs have geometry shadows for masonry above the floor");
 for(int type=0;type<LegacyEnemyTypeCount;type++)for(int d=0;d<8;d++)check(fallen_art::sprites[type][d].ground.art.id!=0,"each legacy fallen body has a cached physical shadow in all eight directions");
 // Test the real frame order, masks and reuse at different display scales.
 for(bool desert:{false,true}){
  game=Game(49);if(desert)game.activateField(WorldLocation::Desert);game.messageTime=0;game.hp=game.maxHp();
  world_shadows::prepare();check(world_shadows::ready&&world_shadows::visibleCasters>2,"a real forest or desert frame prepares visible shadow casters");
  check(world_shadows::preparedActors>0,"animated meshes are prepared once for both shadow and body passes");
  size_t count=actor_art::frameRigs.size();Vector2 p=screen(game.player);
  BeginTextureMode(sample);ClearBackground(BLANK);hero(p.x,p.y,1);EndTextureMode();
  check(actor_art::frameRigs.size()==count,"drawing the body reuses the existing mesh without preparing a second copy");actor_art::finishShadowFrame();
  prepareCanvas(1);renderScene();saveTexture(canvas,desert?"qa-v40/desert-shadows.png":"qa-v40/forest-shadows.png");
 }
 game.enterMageTower();game.dawnQuest=DawnQuest::AtTower;game.messageTime=0;
 for(int floor=0;floor<TowerFloorCount;floor++){
  game.towerFloor=floor;game.player=TowerCenter;prepareCanvas(1);renderScene();
  check(world_shadows::ready&&world_shadows::visibleCasters>4,"all five tower floors include furniture, stairs and player shadows");
  saveTexture(canvas,TextFormat("qa-v40/tower-%d-shadows.png",floor+1));
 }
 // A shadow must not continue into the void outside the tower's floor.
 world_shadows::prepare();BeginTextureMode(world_shadows::mask);ClearBackground(WHITE);EndTextureMode();world_shadows::soften();
 RenderTexture2D receiverTest=LoadRenderTexture(WorldW,H);BeginTextureMode(receiverTest);ClearBackground(BLANK);world_shadows::draw();EndTextureMode();
 Image receiver=LoadImageFromTexture(receiverTest.texture);ImageFlipVertical(&receiver);Color* receiverPixels=LoadImageColors(receiver);
 Vector2 c=screen(TowerCenter);int outside=std::min(WorldW-1,int(c.x+TowerRadius*56.5685f+25));
 check(receiverPixels[int(c.y)*WorldW+outside].a==0,"tower shadow cannot fall on empty space outside the floor");
 check(receiverPixels[int(c.y)*WorldW+int(c.x)].a>30&&receiverPixels[int(c.y)*WorldW+int(c.x)].a<100,"overlapping silhouettes are composed once with restrained opacity");
 UnloadImageColors(receiverPixels);UnloadImage(receiver);
 BeginTextureMode(receiverTest);ClearBackground(WHITE);world_shadows::draw();EndTextureMode();
 receiver=LoadImageFromTexture(receiverTest.texture);ImageFlipVertical(&receiver);receiverPixels=LoadImageColors(receiver);
 Color floorPixel=receiverPixels[int(c.y)*WorldW+int(c.x)];
 check(floorPixel.a==255&&floorPixel.r>170&&floorPixel.r<200,"shadow darkens an opaque receiver without changing its opacity");
 UnloadImageColors(receiverPixels);UnloadImage(receiver);UnloadRenderTexture(receiverTest);actor_art::finishShadowFrame();
 startIntro();intro.phase=IntroPhase::Dialogue;intro.time=1;prepareCanvas(1);renderScene();
 check(world_shadows::chamberFrame&&world_shadows::preparedActors==1,"story chamber renders the hero, mentor and pillars through the same shadow pass");
 saveTexture(canvas,"qa-v40/intro-shadows.png");
 intro.phase=IntroPhase::Charging;intro.age=IntroSequence::ChargeTime*.5f;world_shadows::prepare(true);
 Vector2 foot=actor_art::frameRigs.front().second.groundProject({0,0,0});
 check(std::abs(foot.x-(510+shadow_math::LightX*12))<.01f&&std::abs(foot.y-(532+shadow_math::LightY*12))<.01f,"levitating hero keeps the shadow on the floor while its physical height changes");
 actor_art::finishShadowFrame();
 game=saved;intro=oldIntro;towerStory=oldStory;welcome=oldWelcome;actor_art::finishShadowFrame();UnloadRenderTexture(sample);
 report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
