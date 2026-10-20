#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#include <iostream>
#undef main
int main(int argc,char** argv){
 if(argc!=2||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::create_directories("qa-sheath/frames");
 std::ofstream log("qa-sheath/tests.txt");int checks=0,failures=0;auto check=[&](bool ok,const char* label){checks++;failures+=!ok;log<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';log.flush();};
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall sword stow --no-audio");if(!IsWindowReady())return 4;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 check(PlayerBuild&&!audioSystem.available,"sword transition QA runs actual player code without audio");
 Image atlas=LoadImage("assets/sprites/hero-paints.png");Color* texels=LoadImageColors(atlas);int outside=0;
 for(int y=0;y<128;y++)for(int x=0;x<128;x++)if((x<30||x>98||y<38||y>100)&&texels[y*512+x].a>0)outside++;
 check(outside==0,"facial feature cell has no opaque skin patch, cheek outline or temple border");UnloadImageColors(texels);UnloadImage(atlas);
 game=Game{};intro={};towerStory={};clearStoryMenus();welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=false;
 game.props.clear();game.enemies.clear();game.forage.clear();game.player={20,20};game.messageTime=0;game.equipment[0]=item(Kind::Sword);game.equipment[2]={};
 RenderTexture2D sample=LoadRenderTexture(480,440),sheet=LoadRenderTexture(1920,880);
 BeginTextureMode(sheet);ClearBackground(Ink);EndTextureMode();
 for(int dir=0;dir<8;dir++){
  AnimationState state;state.direction=Direction(dir);actor_art::finishShadowFrame();BeginTextureMode(sample);ClearBackground(BLANK);actor_art::draw(0,240,340,3,state,0,false,false,Kind::Sword,false,BLANK,nullptr,-1,-1,-1,20,false,&game.equipment);EndTextureMode();
  BeginTextureMode(sheet);DrawTexturePro(sample.texture,{0,0,480,-440},{float(dir%4)*480,float(dir/4)*440,480,440},{0,0},0,WHITE);EndTextureMode();
 }
 saveTexture(sheet,"qa-sheath/face-directions.png");UnloadRenderTexture(sheet);
 for(int stage=0;stage<=10;stage++){
  RenderTexture2D directions=LoadRenderTexture(1920,880);BeginTextureMode(directions);ClearBackground(Ink);EndTextureMode();
  for(int dir=0;dir<8;dir++){
   AnimationState state;state.direction=Direction(dir);state.motionInitialized=true;state.moving=true;state.moveBlend=1;state.phase=.1875f;state.runBlend=1;state.swordStow=stage/10.f;
   actor_art::finishShadowFrame();BeginTextureMode(sample);ClearBackground(BLANK);actor_art::draw(0,240,340,3,state,0,false,false,Kind::Sword,false,BLANK,nullptr,-1,-1,-1,20,false,&game.equipment);EndTextureMode();
   BeginTextureMode(directions);DrawTexturePro(sample.texture,{0,0,480,-440},{float(dir%4)*480,float(dir/4)*440,480,440},{0,0},0,WHITE);EndTextureMode();
  }
  saveTexture(directions,TextFormat("qa-sheath/clearance-directions-%02d.png",stage));UnloadRenderTexture(directions);
 }
 float previous=0;int sheathing=0,drawing=0;
 for(int f=0;f<180;f++){
  bool run=f>=30&&f<90;game.update(1.f/60,{1,0},false,run);float p=game.animation.swordStow;
  if(p>previous+.00001f)sheathing++;if(p<previous-.00001f)drawing++;
  check(std::abs(p-previous)<=1.f/36+.00001f,"actual movement input advances/reverses the weapon gesture without snapping");previous=p;
  // Three facings show hand contact and the blade disappearing into the sheath.
  RenderTexture2D movie=LoadRenderTexture(1200,400);BeginTextureMode(movie);ClearBackground(Ink);EndTextureMode();
  for(int d=0;d<3;d++){
   AnimationState state=game.animation;state.direction=Direction(d+1);state.facingInitialized=false;
   actor_art::finishShadowFrame();BeginTextureMode(sample);ClearBackground(BLANK);actor_art::draw(0,240,340,3,state,0,false,false,Kind::Sword,false,BLANK,nullptr,-1,-1,-1,20,false,&game.equipment);EndTextureMode();
   Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);int solid=0,border=0;
   for(int y=0;y<440;y++)for(int x=0;x<480;x++)if(pixels[y*480+x].a>160){solid++;border+=x<2||x>=478||y<2||y>=438;}
   check(solid>1700&&border==0,"complete hero and rigid sword remain visible without clipped transition frames");UnloadImageColors(pixels);UnloadImage(im);
   BeginTextureMode(movie);DrawTexturePro(sample.texture,{0,0,480,-440},{d*400.f,0,400,366},{0,0},0,WHITE);center(run?"CHOWANIE / BIEG":"DOBYWANIE / CHOD",d*400+200,375,16,Gold);EndTextureMode();
  }
  saveTexture(movie,TextFormat("qa-sheath/frames/frame-%03d.png",f));UnloadRenderTexture(movie);
 }
 check(sheathing>=35&&drawing>=35&&game.animation.swordStow==0,"both physical gestures take multiple frames and finish at the correct carry state");
 game.animation.swordStow=.5f;game.attackCd=game.swing=0;check(game.attack()&&game.animation.swordStow==0,"clicking attack cancels a partial sheath gesture and keeps combat responsive");
 RenderTexture2D closeup=LoadRenderTexture(800,250);BeginTextureMode(closeup);ClearBackground(Ink);
 for(int d=0;d<4;d++){AnimationState state;state.direction=Direction(d);actor_art::draw(0,100+d*200,545,6,state,0,false,false,Kind::None,false,BLANK,nullptr,-1,-1,-1,0,false,nullptr);}EndTextureMode();saveTexture(closeup,"qa-sheath/face-closeups.png");UnloadRenderTexture(closeup);
 UnloadRenderTexture(sample);log<<checks-failures<<"/"<<checks<<" passed\n";std::cout<<"Sword stow QA: "<<checks-failures<<"/"<<checks<<" passed\n";hero_paint::unload();audioSystem.shutdown();sync_present::shutdown();CloseWindow();return failures?2:0;
}
