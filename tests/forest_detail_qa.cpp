#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include "forest_capture.hpp"

int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());
 fs::path root=argv[2];fs::create_directories(root/"frames");
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();
 InitWindow(W,H,"Emberfall forest creatures QA --no-audio");if(!IsWindowReady())return 4;
 makeArt();fallen_art::init();prepareCanvas(1);
 std::ofstream log(root/"checks.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const std::string& label){
  checks++;failures+=!ok;log<<(ok?"PASS ":"FAIL ")<<label<<'\n';
 };
 check(creature_paint::atlas.id&&!audioSystem.available,"actual atlas loaded and audio device closed");
 constexpr std::array<int,4> types{0,1,2,7};
 constexpr int width=400,height=430;
 RenderTexture2D target=LoadRenderTexture(width,height);
 auto capture=[&](int type,const AnimationState& state,float attack,const fs::path& file){
  float scale=type==7?1.95f:2.8f;
  actor_art::finishShadowFrame();BeginTextureMode(target);ClearBackground(BLANK);
  actor_art::draw(type+1,200,354,scale,state,attack);EndTextureMode();
  Image image=LoadImageFromTexture(target.texture);ImageFlipVertical(&image);
  Color* px=LoadImageColors(image);int body=0,border=0,sumX=0,sumY=0;
  for(int y=0;y<height;y++)for(int x=0;x<width;x++)if(px[y*width+x].a>238){
   body++;sumX+=x;sumY+=y;border+=x<2||x>=width-2||y<2||y>=height-2;
  }
  // Choose a stable interior pixel near the body center rather than a point
  // at a thin tip where raster coverage changes at subpixel boundaries.
  int cx=sumX/std::max(1,body),cy=sumY/std::max(1,body),distance=width*width+height*height;
  Vector2 interior{};
  for(int y=3;y<height-3;y++)for(int x=3;x<width-3;x++){
   int dx=x-cx,dy=y-cy,score=dx*dx+dy*dy;
   if(score>=distance||px[y*width+x].a<=238)continue;
   bool opaque=true;
   for(int v=-3;v<=3&&opaque;v++)for(int u=-3;u<=3&&opaque;u++)opaque=px[(y+v)*width+x+u].a>238;
   if(opaque){interior={x+.5f,y+.5f};distance=score;}
  }
  check(body>700&&border==0,"complete silhouette "+file.filename().string());
  check(interior.x&&actor_art::draw(type+1,200,354,scale,state,attack,false,false,Kind::Sword,false,BLANK,&interior),
        "visible body can be picked "+file.filename().string());
  check(ExportImage(image,file.string().c_str()),"export "+file.filename().string());
  UnloadImageColors(px);UnloadImage(image);
 };
 for(int type:types){
  for(int direction=0;direction<8;direction++)for(int action=0;action<3;action++){
   AnimationState state;state.direction=Direction(direction);state.phase=.18f;state.idleTime=.45f;
   state.moving=action==1;state.motionInitialized=true;state.moveBlend=action==1?1:0;
   capture(type,state,action==2?1.f:0.f,root/TextFormat("pose-%d-%d-%d.png",type,direction,action));
  }
  for(int frame=0;frame<36;frame++){
   AnimationState state;state.phase=frame/36.f;state.idleTime=state.phase;
   state.moving=true;state.motionInitialized=true;state.moveBlend=1;
   capture(type,state,0,root/"frames"/TextFormat("move-%d-%02d.png",type,frame));
   state.moving=false;state.moveBlend=0;
   float attack=frame<18?-frame/18.f:std::sin((frame-18)/18.f*PI);
   capture(type,state,attack,root/"frames"/TextFormat("attack-%d-%02d.png",type,frame));
  }
 }
 UnloadRenderTexture(target);
 bakeForestBodies(root,types,check);

 game=Game{};game.player={8,19};game.enemies.clear();game.messageTime=0;
 for(size_t i=0;i<types.size();i++){
  V p=game.player+V{float(i%2)*3-1.5f,float(i/2)*3-1.5f};
  int type=types[i];game.enemies.push_back({100+int(i),type,p,p,100,100});
 }
 welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;mouse={0,0};
 renderScene();
 check(world_shadows::ready,"actual forest scene used projected shadows");
 saveTexture(canvas,(root/"forest-field.png").string().c_str());
 check(fs::exists(root/"forest-field.png"),"actual forest view exported");
 fallen_art::unload();creature_paint::unload();actor_art::unloadCreatureBatch();
 sync_present::shutdown();CloseWindow();
 log<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';
 return failures?2:0;
}
