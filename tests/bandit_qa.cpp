#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include "forest_capture.hpp"
#include <iostream>

int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());
 fs::path root=argv[2];fs::create_directories(root/"frames");
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();
 InitWindow(W,H,"Emberfall bandit QA --no-audio");
 if(!IsWindowReady())return 4;
 creature_paint::load();actor_art::prepareCreatureBatch();
 std::ofstream report(root/"checks.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const std::string& label){
  checks++;failures+=!ok;report<<(ok?"PASS ":"FAIL ")<<label<<'\n';
 };
 check(creature_paint::atlas.id!=0,"forest materials loaded");

 RenderTexture2D sample=LoadRenderTexture(360,420);
 auto capture=[&](const AnimationState& state,float attack,const fs::path& path){
  actor_art::finishShadowFrame();BeginTextureMode(sample);ClearBackground(BLANK);
  actor_art::draw(7,180,342,2.8f,state,attack);EndTextureMode();
  Image image=LoadImageFromTexture(sample.texture);ImageFlipVertical(&image);
  Color* pixels=LoadImageColors(image);int area=0,border=0;Vector2 inside{};
  for(int y=0;y<420;y++)for(int x=0;x<360;x++)if(pixels[y*360+x].a>240){
   area++;border+=x<2||x>=358||y<2||y>=418;
   if(x>165&&x<192&&y>156&&y<235)inside={x+.5f,y+.5f};
  }
  check(area>2000&&border==0,"complete body and blade "+path.filename().string());
  check(inside.x>0&&actor_art::draw(7,180,342,2.8f,state,attack,false,false,Kind::Sword,false,BLANK,&inside),
        "mouse picking reaches visible bandit "+path.filename().string());
  check(ExportImage(image,path.string().c_str()),"saved "+path.filename().string());
  UnloadImageColors(pixels);UnloadImage(image);
 };
 for(int dir=0;dir<8;dir++){
  AnimationState state;state.direction=Direction(dir);
  capture(state,0,root/TextFormat("standing-%d.png",dir));
 }
 for(int frame=0;frame<48;frame++){
  AnimationState state;state.phase=frame/48.f;state.idleTime=state.phase;
  state.moving=true;state.motionInitialized=true;state.moveBlend=1;
  capture(state,0,root/"frames"/TextFormat("walk-%02d.png",frame));
  state.moving=false;state.moveBlend=0;
  float attack=frame<24?-frame/24.f:std::sin((frame-24)/24.f*PI);
  capture(state,attack,root/"frames"/TextFormat("attack-%02d.png",frame));
 }
 UnloadRenderTexture(sample);

 bakeForestBodies(root,std::array<int,1>{6},check);
 creature_paint::unload();actor_art::unloadCreatureBatch();sync_present::shutdown();CloseWindow();
 report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';
 std::cout<<"Bandit QA: "<<checks<<" checks, "<<failures<<" failures\n";
 return failures?2:0;
}
