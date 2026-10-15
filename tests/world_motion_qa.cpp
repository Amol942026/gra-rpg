#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include "../src/world_motion_qa.hpp"
int main(int argc,char** argv){
 if(argc!=2||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_VSYNC_HINT);
 InitWindow(W,H,"World motion QA");if(!IsWindowReady())return 4;audioSystem.init(false,false);
 makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();
 bool passed=runWorldMotionQA();
 world_shadows::unload();fallen_art::unload();unloadHoverArt();unloadSkillArt();unloadTowerRoomArt();tower_stairs::unload();tower_furniture::unload();desert_art::unload();sand_prison::unload();
 for(auto& texture:propArt)UnloadRenderTexture(texture);
 UnloadRenderTexture(terrain);UnloadTexture(minimapTerrain);if(canvas.id)UnloadRenderTexture(canvas);if(worldCanvas.id)UnloadRenderTexture(worldCanvas);
 typography.clear();audioSystem.shutdown();CloseWindow();return passed?0:2;
}
