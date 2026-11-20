// Export the 2D transforms used by Emberfall for the editable Synfig rig.
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../../src/main.cpp"
#undef main
#include <iomanip>

int main(int argc,char** argv){
 if(argc!=4||std::string_view(argv[1])!="--no-audio"||std::string_view(argv[2])!="--background-test")return 2;
 const fs::path output=argv[3];
 fs::create_directories(output);
 SetTraceLogLevel(LOG_ERROR);
 SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_ALWAYS_RUN);
 sync_present::useOpenGL();
 InitWindow(232,138,"Emberfall / Synfig");
 if(!IsWindowReady()||!IsWindowHidden()||IsAudioDeviceReady())return 3;
 using namespace caldera_puppets;
 load();
 if(!ready(Imp)||!impCore.texture.id)return 4;
 const auto target=render_targets::colorOnly(232,138);
 std::ofstream file(output/"pozy.json");
 file<<std::setprecision(8)<<'{';
 for(const std::string name:{"spoczynek","chod","obrot","rzut","upadek"}){
  if(name!="spoczynek")file<<',';
  file<<'\n'<<'"'<<name<<"\":[";
  const int frames=name=="obrot"?120:60;
  const auto directory=output/name;
  fs::create_directories(directory);
  for(int i=0;i<=frames;++i){
   AnimationState state{};
   state.facingInitialized=true;
   const float t=float(i)/frames;
   state.idleTime=t/.72f;
   state.facingAngle=PI/4;
   if(name=="chod"||name=="obrot"){
    state.moveBlend=1;
    state.phase=t;
   }
   if(name=="obrot"){
    state.facingAngle+=2*PI*t;
    state.moveBlend=.65f;
    state.phase=2*t;
    state.pivotPhase=4*t;
    state.turnLean=.65f;
   }
   state.direction=directionAtScreenAngle(state.facingAngle);
   if(name=="upadek")state.idleTime=.24f;
   const auto pose=name=="upadek"?fall(Imp,state,.35f,t,1):fireImp(state,name=="rzut"?t:-1.f);
   if(i)file<<',';
   file<<"\n[";
   auto parts=pose.pieces;
   const auto end=parts.begin()+pose.count;
   std::stable_sort(parts.begin(),end,[](const Piece& a,const Piece& b){return a.depth<b.depth;});
   for(auto piece=parts.begin();piece!=end;++piece){
    if(piece!=parts.begin())file<<',';
    const auto& p=*piece;
    file<<'['<<p.tile<<','<<int(p.mirror)<<','<<p.center.x<<','<<p.center.y<<','
        <<p.half.x<<','<<p.half.y<<','<<p.angle*RAD2DEG<<']';
   }
   file<<']';
   BeginTextureMode(target);
   ClearBackground(BLANK);
   draw(Imp,pose,116.f,119.f,1,BLANK,nullptr,false,false,1);
   EndTextureMode();
   auto screenshot=LoadImageFromTexture(target.texture);
   ImageFlipVertical(&screenshot);
   const bool saved=ExportImage(screenshot,(directory/(std::to_string(i)+".png")).string().c_str());
   UnloadImage(screenshot);
   if(!saved)return 5;
  }
  file<<']';
 }
 file<<"\n}\n";
 UnloadRenderTexture(target);
 unload();
 CloseWindow();
 return file?0:6;
}
