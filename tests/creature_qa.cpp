#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#include <iostream>
#include <iomanip>
#undef main
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];fs::create_directories(root/"frames");fs::create_directories(root/"poses");
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall creatures --no-audio");if(!IsWindowReady())return 4;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 int checks=0,failures=0,writes=0;std::ofstream log(root/"checks.txt");auto check=[&](bool ok,const std::string& label){checks++;if(!ok)failures++;log<<(ok?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 auto save=[&](RenderTexture2D target,fs::path path){Image im=LoadImageFromTexture(target.texture);ImageFlipVertical(&im);if(!ExportImage(im,path.string().c_str()))writes++;UnloadImage(im);};
 check(!audioSystem.available,"audio device is closed");check(creature_paint::atlas.id!=0,"Krita atlas loaded");
 // Compare the shared renderer against actual hero PNGs retained before this
 // task; creature surface work must preserve the player's latest walk/sword.
 RenderTexture2D heroSample=LoadRenderTexture(400,420);const char* heroModes[]={"idle","walk","run","slash"};
 for(int mode=0;mode<4;mode++){
  game=Game{};game.equipment={item(Kind::Sword),{},item(Kind::Shield),item(Kind::WolfCloak),item(Kind::Boots),{}};
  float phase=17/60.f;game.animation={};game.animation.direction=Direction::SE;game.animation.phase=phase;game.animation.idleTime=phase*2;
  game.animation.moving=mode==1||mode==2;game.animation.runBlend=mode==2?1:0;
  game.swing=0;game.bowDrawing=false;game.bowReleaseTime=0;game.rollRemaining=0;game.swingStrong=false;
  if(mode==3){game.meleeDuration=SwingDuration;game.swing=(1-phase)*SwingDuration;}
  actor_art::finishShadowFrame();BeginTextureMode(heroSample);ClearBackground(BLANK);hero(200,320,2.7f,false);EndTextureMode();
  fs::path before=fs::path("qa-creatures/before/hero")/(std::string(heroModes[mode])+"-17.png");
  // Packaged QA receives these explicit fixtures alongside its executable.
  Image actual=LoadImageFromTexture(heroSample.texture);ImageFlipVertical(&actual);Image baseline=LoadImage(before.string().c_str());
  bool same=baseline.data&&baseline.width==actual.width&&baseline.height==actual.height;int difference=0;
  if(same){Color* a=LoadImageColors(actual),*b=LoadImageColors(baseline);for(int n=0;n<actual.width*actual.height;n++)difference+=std::abs(int(a[n].r)-b[n].r)+std::abs(int(a[n].g)-b[n].g)+std::abs(int(a[n].b)-b[n].b)+std::abs(int(a[n].a)-b[n].a)>4;UnloadImageColors(a);UnloadImageColors(b);}
  check(same&&difference==0,std::string("hero visual regression / ")+heroModes[mode]+" / changed pixels="+std::to_string(difference));
  UnloadImage(actual);if(baseline.data)UnloadImage(baseline);
 }
 UnloadRenderTexture(heroSample);
 for(int family=0;family<6;family++){
  auto a=creature_motion::sample(family,0),b=creature_motion::sample(family,1);check(std::abs(a.travel-b.travel)+std::abs(a.lift-b.lift)+std::abs(a.sway-b.sway)<.00001f,"native gait loop "+std::to_string(family));
  for(int f=0;f<60;f++){auto k=creature_motion::sample(family,f/60.f);check(std::isfinite(k.travel)&&k.lift>=0&&k.lift<=1.001f,"finite contact gait");}
 }
 RenderTexture2D sample=LoadRenderTexture(360,420),sheet=LoadRenderTexture(1440,1260);
 BeginTextureMode(sheet);ClearBackground(Ink);EndTextureMode();
 for(int type=0;type<12;type++)for(int direction=0;direction<8;direction++)for(int action=0;action<3;action++){
  AnimationState s;s.direction=Direction(direction);s.phase=.17f;s.idleTime=.7f;s.moving=action!=0;s.moveBlend=action!=0?1:0;s.motionInitialized=true;
  actor_art::finishShadowFrame();float hit=action==2?1:0;float scale=type==11?1.8f:type==7||type==8||type==10?2.1f:2.8f;
  BeginTextureMode(sample);ClearBackground(BLANK);actor_art::draw(type+1,180,342,scale,s,hit);EndTextureMode();
  Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);Color* px=LoadImageColors(im);int opaque=0,border=0;
  // Compare the efficient GPU buffer with the original immediate submission.
  actor_art::gpuCreatureDraw=false;BeginTextureMode(sample);ClearBackground(BLANK);actor_art::draw(type+1,180,342,scale,s,hit);EndTextureMode();actor_art::gpuCreatureDraw=true;
  Image legacy=LoadImageFromTexture(sample.texture);ImageFlipVertical(&legacy);Color* old=LoadImageColors(legacy);int backendDifference=0;
  for(int n=0;n<360*420;n++)backendDifference+=std::abs(int(px[n].r)-old[n].r)+std::abs(int(px[n].g)-old[n].g)+std::abs(int(px[n].b)-old[n].b)+std::abs(int(px[n].a)-old[n].a)>6;
  check(backendDifference<8,"GPU backend matches original material/depth/color / different pixels="+std::to_string(backendDifference));UnloadImageColors(old);UnloadImage(legacy);
  for(int y=0;y<420;y++)for(int x=0;x<360;x++)if(px[y*360+x].a>230){opaque++;border+=x<2||x>=358||y<2||y>=418;}
  check(opaque>250&&border==0,"complete silhouette type="+std::to_string(type)+" direction="+std::to_string(direction)+" action="+std::to_string(action));
  // Sparse GPU-versus-pointer checks, excluding anti-aliased edges.
  int mismatch=0,interior=0;for(int y=25;y<410;y+=29)for(int x=15;x<345;x+=31){
   bool solid=true,empty=true;for(int dy:{-2,0,2})for(int dx:{-2,0,2}){int a=px[(y+dy)*360+x+dx].a;solid&=a>230;empty&=a<8;}
   if(!solid&&!empty)continue;Vector2 point{float(x)+.5f,float(y)+.5f};bool picked=actor_art::draw(type+1,180,342,scale,s,hit,false,false,Kind::Sword,false,BLANK,&point);
   mismatch+=solid?!picked:picked;interior+=solid;
  }
  // A thin scorpion in profile can fall between sparse grid lines. Find an
  // actual opaque interior pixel rather than weakening its picking assertion.
  if(!interior)for(int y=4;y<416&&!interior;y++)for(int x=4;x<356&&!interior;x++){
   bool solid=true;for(int dy:{-2,0,2})for(int dx:{-2,0,2})solid&=px[(y+dy)*360+x+dx].a>230;
   if(solid){Vector2 point{float(x)+.5f,float(y)+.5f};interior++;mismatch+=!actor_art::draw(type+1,180,342,scale,s,hit,false,false,Kind::Sword,false,BLANK,&point);}
  }
  check(mismatch==0&&interior>0,"pointer matches visible body type="+std::to_string(type)+" direction="+std::to_string(direction)+" action="+std::to_string(action)+" interior="+std::to_string(interior)+" mismatch="+std::to_string(mismatch));UnloadImageColors(px);UnloadImage(im);
  save(sample,root/"poses"/TextFormat("type-%02d-dir-%d-action-%d.png",type,direction,action));
  if(direction==1&&action==1){int col=type%4,row=type/4;BeginTextureMode(sheet);DrawTexturePro(sample.texture,{0,0,360,-420},{float(col*360),float(row*420),360,420},{0,0},0,WHITE);txt(enemyInfo(type).name,col*360.f+25,row*420.f+382,18,Gold);EndTextureMode();}
 }
 save(sheet,root/"bestiary.png");UnloadRenderTexture(sample);UnloadRenderTexture(sheet);
 RenderTexture2D movie=LoadRenderTexture(1440,1080);
 for(int f=0;f<60;f++){
  BeginTextureMode(movie);ClearBackground(Ink);
  for(int type=0;type<12;type++){
   AnimationState s;s.direction=Direction::SE;s.phase=f/60.f;s.idleTime=f/60.f;s.moving=true;s.motionInitialized=true;s.moveBlend=1;
   float x=180+(type%4)*360.f,y=310+(type/4)*360.f;ellipse(x,y+3,48,15,rgb(17,27,25));
   actor_art::draw(type+1,x,y,type==11?1.65f:type==7?1.8f:2.4f,s,f>=30?std::sin((f-30)/29.f*PI):0);
  }
  EndTextureMode();save(movie,root/"frames"/TextFormat("frame-%02d.png",f));
 }
 UnloadRenderTexture(movie);
 // Both complete world render paths, with actual live enemies and corpses.
 for(int desert=0;desert<2;desert++){
  game=Game{};if(desert)game.activateField(WorldLocation::Desert);game.props.clear();game.enemies.clear();game.forage.clear();game.player={20,20};game.messageTime=0;
  welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;mouse={0,0};dragging={};refreshDisplayLayout();
  for(int i=0;i<(desert?4:8);i++){int type=desert?8+i:i;V pos=game.player+V{float(i%4-2)*2.4f,float(i/4-2)*2.4f};game.enemies.push_back({100+i,type,pos,pos,100,100});auto& e=game.enemies.back();e.animation.moving=true;e.animation.phase=.17f;}
  renderScene();saveTexture(canvas,(root/(desert?"desert.png":"forest.png")).string().c_str());
  Image layered=LoadImageFromTexture(canvas.texture);actor_art::allowSceneDepth=false;renderScene();Image previous=LoadImageFromTexture(canvas.texture);actor_art::allowSceneDepth=true;
  Color* a=LoadImageColors(layered),*b=LoadImageColors(previous);int changed=0;
  for(int n=0;n<layered.width*layered.height;n++)changed+=std::abs(int(a[n].r)-b[n].r)+std::abs(int(a[n].g)-b[n].g)+std::abs(int(a[n].b)-b[n].b)>12;
  check(changed<150,"scene depth layers preserve painter order / changed pixels="+std::to_string(changed));
  UnloadImageColors(a);UnloadImageColors(b);UnloadImage(layered);UnloadImage(previous);
 }
 auto workingDirectory=fs::current_path();
 fs::path legacy=fs::absolute(root/("legacy-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())));
 fs::create_directories(legacy);fs::current_path(legacy);
 // The scene resources are already preloaded. Give older diagnostics a fresh
 // owned directory so protected historical PNGs cannot silently reject export.
 verifyFallenArt(check);
 check(runCorpseQA(),"actual corpse opening, item transfer and looting");
 check(runShadowQA(),"physical ground shadows and shared body cache");
 fs::current_path(workingDirectory);
 check(writes==0,"every PNG exported");
 log<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';
 std::ofstream report(root/"verification.json");report<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"poses\":288,\"animation_frames\":60,\"species\":12,\"directions\":8,\"legacy_directory\":"<<std::quoted(legacy.generic_string())<<",\"audio_device_opened\":false}";
 actor_art::unloadCreatureBatch();creature_paint::unload();hero_paint::unload();audioSystem.shutdown();sync_present::shutdown();CloseWindow();
 std::cout<<"Creature QA: "<<checks<<" checks; "<<failures<<" failures --no-audio\n";return failures?2:0;
}
