#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#include <iostream>
#undef main
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];fs::create_directories(root/"frames");
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall walk --no-audio");if(!IsWindowReady())return 4;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 int failedWrites=0;auto save=[&](RenderTexture2D target,const fs::path& path){Image im=LoadImageFromTexture(target.texture);ImageFlipVertical(&im);if(!ExportImage(im,path.string().c_str()))failedWrites++;UnloadImage(im);};
 game=Game{};game.props.clear();game.enemies.clear();game.forage.clear();game.player={20,20};game.messageTime=0;
 game.equipment={item(Kind::Sword),{},item(Kind::Shield),item(Kind::WolfCloak),item(Kind::Boots),{}};
 RenderTexture2D sample=LoadRenderTexture(360,390),contact=LoadRenderTexture(1800,780);
 auto draw=[&](float phase,int direction,bool armed){
  AnimationState state;state.direction=Direction(direction);state.phase=phase;state.moving=true;state.moveBlend=1;state.motionInitialized=true;
  auto gear=game.equipment;if(!armed){gear[0]={};gear[2]={};}
  actor_art::finishShadowFrame();BeginTextureMode(sample);ClearBackground(BLANK);
  actor_art::draw(0,180,310,2.8f,state,0,false,armed,armed?Kind::Sword:Kind::None,false,BLANK,nullptr,-1,-1,-1,20,false,&gear);EndTextureMode();
 };
 for(int direction=0;direction<8;direction++){
  BeginTextureMode(contact);ClearBackground(Ink);EndTextureMode();
  for(int stage=0;stage<10;stage++){
   draw(stage/10.f,direction,true);BeginTextureMode(contact);DrawTexturePro(sample.texture,{0,0,360,-390},{float(stage%5)*360,float(stage/5)*390,360,390},{0,0},0,WHITE);EndTextureMode();
  }
  save(contact,root/("direction-"+std::to_string(direction)+".png"));
 }
 int frames=0,borders=0;for(int frame=0;frame<120;frame++){
  RenderTexture2D movie=LoadRenderTexture(1080,400);BeginTextureMode(movie);ClearBackground(Ink);EndTextureMode();
  for(int view=0;view<3;view++){
   draw(frame/60.f,view==0?0:view==1?2:1,view!=1);
   Image im=LoadImageFromTexture(sample.texture);Color* px=LoadImageColors(im);for(int y=0;y<390;y++)for(int x=0;x<360;x++)if(px[y*360+x].a>180&&(x<2||x>=358||y<2||y>=388))borders++;
   UnloadImageColors(px);UnloadImage(im);BeginTextureMode(movie);DrawTexturePro(sample.texture,{0,0,360,-390},{view*360.f,0,360,390},{0,0},0,WHITE);EndTextureMode();
  }
  save(movie,root/"frames"/TextFormat("frame-%03d.png",frame));UnloadRenderTexture(movie);frames++;
 }
 // Actual input includes starts, stops, direction changes and sprint transitions.
 int invalid=0;for(int f=0;f<240;f++){
  bool moving=f<80||f>=110;game.update(1.f/60,moving?V{f<150?1.f:-1.f,0}:V{},false,f>=170&&f<210);
  auto p=hero_motion::evaluate(game.animation.phase,game.animation.moveBlend,game.animation.runBlend,game.animation.idleTime);
  for(auto bone:p.world)invalid+=!std::isfinite(bone.p.x)||!std::isfinite(bone.p.z);
 }
 std::ofstream report(root/"render-verification.json");report<<"{\"status\":\""<<(borders||invalid||failedWrites?"FAIL":"PASS")<<"\",\"frames\":"<<frames<<",\"directions\":8,\"border_pixels\":"<<borders<<",\"invalid_bones\":"<<invalid<<",\"failed_writes\":"<<failedWrites<<",\"audio\":false}\n";
 UnloadRenderTexture(sample);UnloadRenderTexture(contact);hero_paint::unload();audioSystem.shutdown();sync_present::shutdown();CloseWindow();
 std::cout<<"Walk renderer: "<<(borders||invalid||failedWrites?"FAIL":"PASS")<<", 120 frames, eight directions --no-audio\n";return borders||invalid||failedWrites?2:0;
}
