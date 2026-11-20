#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallGameMain
#include "../../src/main.cpp"
#undef main
#include <chrono>
#include <iomanip>

int main(int argc,char** argv){
 if(argc!=4||std::string(argv[1])!="--background-test"||std::string(argv[2])!="--no-audio")return 2;
 const fs::path out=argv[3];fs::create_directories(out);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_ALWAYS_RUN);
 sync_present::useOpenGL();InitWindow(256,256,"Wraith cel export");
 if(!IsWindowReady()||!IsWindowHidden()||IsAudioDeviceReady())return 3;
 volcano_actors::load();
 if(!wraith_death_art::rotation::ready())wraith_death_art::rotation::load();
 if(!wraith_death_art::rotation::ready())return 4;
 auto target=render_targets::colorOnly(256,256);
 constexpr int Directions=96;
 constexpr int Raster=216;
 const auto begin=std::chrono::steady_clock::now();
 for(int tick=0;tick<Directions;++tick){
  AnimationState state;state.facingAngle=(2*PI*tick)/Directions;state.facingInitialized=true;
  const auto view=wraith_death_art::at(state);
  for(int part=0;part<6;++part){
   BeginTextureMode(target);
   // Blending is disabled throughout the single layer export, so the PNG
   // retains the original straight RGB along the semitransparent contour.
   rlDisableColorBlend();ClearBackground(BLANK);
   auto& w=wraith_death_art::rotation::weave;
   wraith_death_art::rotation::begin(view.left,view.right,view.fraction,view.angle,0,{},false);
   rlSetTexture(w.id);rlBegin(RL_QUADS);
   const float x=float(part)*2;
   rlColor4ub(255,255,255,255);
   rlTexCoord2f(x,0);rlVertex2f(0,0);
   rlTexCoord2f(x,1);rlVertex2f(0,Raster);
   rlTexCoord2f(x+1,1);rlVertex2f(Raster,Raster);
   rlTexCoord2f(x+1,0);rlVertex2f(Raster,0);
   rlEnd();rlSetTexture(0);
   EndShaderMode();rlEnableColorBlend();
   EndTextureMode();
   Image im=LoadImageFromTexture(target.texture);ImageFlipVertical(&im);
   ImageCrop(&im,{0,0,Raster,Raster});
   const auto name=out/(std::to_string(tick)+"-"+std::to_string(part)+".png");
   if(!ExportImage(im,name.string().c_str()))return 5;
   UnloadImage(im);
  }
 }
 const double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count();
 std::ofstream(out/"seconds.txt")<<std::setprecision(8)<<seconds;
 UnloadRenderTexture(target);volcano_actors::unload();CloseWindow();return 0;
}

