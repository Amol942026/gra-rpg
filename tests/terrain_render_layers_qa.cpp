#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio"||std::string(argv[2])!="--background-test")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::create_directories("layers");
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Hidden terrain layer QA");audioSystem.init(false,false);
 makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 game=Game{};game.activateField(WorldLocation::Volcano);game.player={16,54};intro={};towerStory={};clearTransientUi();welcome=paused=help=false;mouse={0,0};
 world_shadows::prepare();RenderTexture2D probe=LoadRenderTexture(WorldW,H);
 saveTexture(world_shadows::mask,"layers/raw-mask.png");saveTexture(world_shadows::softMask,"layers/soft-mask.png");
 for(int stage=0;stage<5;stage++){
  BeginTextureMode(probe);ClearBackground(Ink);native_world::beginOpaqueFrame();
  if(stage>=3){world_shadows::ready=stage==4;drawWorld();}
  else {volcano_art::backdrop();terrain_surface::draw(volcano_art::terrain.texture);if(stage>=1)volcano_art::flow();if(stage>=2)world_shadows::draw();}
  native_world::finishOpaqueBackground();EndTextureMode();saveTexture(probe,("layers/stage-"+std::to_string(stage)+".png").c_str());
 }
 actor_art::finishShadowFrame();UnloadRenderTexture(probe);audioSystem.shutdown();CloseWindow();return 0;
}
