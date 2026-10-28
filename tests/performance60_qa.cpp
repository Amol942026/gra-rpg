#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include <iostream>
extern "C" void __stdcall glFinish();
struct PerfView {const char* name;V pos;int location;};
const PerfView PerfViews[]={{"desert-arrival",DesertArrival,1},{"desert-oasis",DesertOasis+V{0,-5},1},{"desert-boss",DesertBossHome+V{0,-4},1},{"desert-west",{2,35},1},{"desert-east",{69,35},1},{"desert-south",{35,69},1},{"desert-corner",{69,69},1},{"forest",{15,15},0},{"tower-fireplace",TowerArrival,2}};
void perfScene(const PerfView& view){
 game=Game{};intro={};towerStory={};clearTransientUi();welcome=paused=help=false;mouse={0,0};
 if(view.location==1){game.activateField(WorldLocation::Desert);game.dawnQuest=DawnQuest::AtTower;}
 if(view.location==2){game.enterMageTower();game.dawnQuest=DawnQuest::AtTower;}
 game.player=view.pos;game.messageTime=0;
}
double percentile(std::vector<double> values,double q){std::sort(values.begin(),values.end());return values[size_t((values.size()-1)*q)];}
int main(int argc,char** argv){
 if(argc<3||std::string(argv[1])!="--no-audio")return 3;
 bool present=false,dxgi=false,quick=false;for(int i=3;i<argc;i++){std::string a=argv[i];present|=a=="--present";dxgi|=a=="--dxgi";quick|=a=="--quick";nativeWorldRendering|=a=="--native-world";}
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 SetTraceLogLevel(LOG_WARNING);SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_WINDOW_HIGHDPI|(present?FLAG_VSYNC_HINT:FLAG_WINDOW_HIDDEN));if(!dxgi)sync_present::useOpenGL();InitWindow(W,H,"Emberfall 60 FPS / silent QA --no-audio");if(!IsWindowReady())return 5;SetExitKey(KEY_NULL);if(present)updateFramePacing(true);
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();
 auto genQueries=reinterpret_cast<void(*)(int,unsigned*)>(glfwGetProcAddress("glGenQueries"));
 auto beginQuery=reinterpret_cast<void(*)(unsigned,unsigned)>(glfwGetProcAddress("glBeginQuery"));auto endQuery=reinterpret_cast<void(*)(unsigned)>(glfwGetProcAddress("glEndQuery"));
 auto queryInt=reinterpret_cast<void(*)(unsigned,unsigned,int*)>(glfwGetProcAddress("glGetQueryObjectiv"));auto query64=reinterpret_cast<void(*)(unsigned,unsigned,unsigned long long*)>(glfwGetProcAddress("glGetQueryObjectui64v"));
 unsigned queries[16]{};bool gpuQueries=genQueries&&beginQuery&&endQuery&&queryInt&&query64;if(gpuQueries)genQueries(16,queries);
 std::ofstream report(root/"timing.json");report<<"{\"audio_opened\":false,\"presentation_measured\":"<<(present?"true":"false")<<",\"scenes\":[";bool comma=false;
 for(float factor:{1.f,1.2f}){
  if(quick&&factor==1)continue;
  if(present){if(factor>1){if(!fullScreen())toggleFullScreen();}else SetWindowSize(W,H);PollInputEvents();refreshDisplayLayout();}else prepareCanvas(factor);
  for(const auto& view:PerfViews){
   if(quick&&view.location!=1)continue;
   perfScene(view);std::vector<double> work,shown,gpu;performance={};performance.enabled=false;
   int count=present?240:100,focused=0;
   for(int frame=0;frame<count;frame++){
    if(WindowShouldClose()){CloseWindow();return 6;}
    double frameBegin=GetTime();double begin=GetTime();performance.begin();game.update(1.f/60,{},false);game.hp=game.maxHp();V previous=game.player;
    V p=view.pos+V{std::cos(frame*.012f),std::sin(frame*.012f)}*.3f;if(game.walkable(p))game.player=p;
    game.animation.advance(game.player.x-previous.x,game.player.y-previous.y,1.f/60,true);game.time=frame/60.f;
    if(gpuQueries)beginQuery(0x88BF,queries[frame%16]);renderScene();rlDrawRenderBatchActive();if(gpuQueries)endQuery(0x88BF);if(!present)glFinish();double ready=GetTime();
    if(gpuQueries&&frame>=16){int available=0;queryInt(queries[(frame-8)%16],0x8867,&available);if(available){unsigned long long ns=0;query64(queries[(frame-8)%16],0x8866,&ns);if(frame>=68)gpu.push_back(ns/1.e6);}}
    if(present){BeginDrawing();presentCanvas();EndDrawing();finishFramePacing();}
    double end=GetTime();if(frame>=60){work.push_back((ready-begin)*1000);shown.push_back((end-frameBegin)*1000);focused+=IsWindowFocused();}
   }
   auto name=std::string(view.name)+"-"+std::to_string(int(std::round(factor*100)));saveTexture(canvas,(root/(name+".png")).string());
   if(comma)report<<',';comma=true;
   report<<"{\"name\":\""<<view.name<<"\",\"width\":"<<canvas.texture.width<<",\"height\":"<<canvas.texture.height<<",\"frames\":"<<work.size()<<",\"focused_frames\":"<<focused<<",\"backend\":\""<<pacingName()<<"\",\"mean_work_ms\":"<<std::accumulate(work.begin(),work.end(),0.)/work.size()<<",\"p95_work_ms\":"<<percentile(work,.95)<<",\"p99_work_ms\":"<<percentile(work,.99)<<",\"p95_frame_ms\":"<<percentile(shown,.95)<<",\"mean_frame_ms\":"<<std::accumulate(shown.begin(),shown.end(),0.)/shown.size()<<",\"stages\":[";
   for(int i=0;i<int(RenderStage::Count);i++){if(i)report<<',';auto& values=performance.samples[i];report<<(values.empty()?0:percentile(values,.95));}
   report<<"],\"gpu_samples\":"<<gpu.size()<<",\"p95_gpu_ms\":"<<(gpu.empty()?0:percentile(gpu,.95))<<"}";report.flush();std::cout<<name<<": work p95 "<<percentile(work,.95)<<" ms, GPU "<<(gpu.empty()?0:percentile(gpu,.95))<<" ms, frame "<<percentile(shown,.95)<<" ms\n"<<std::flush;
  }
 }
 report<<"]}";auto deleteQueries=reinterpret_cast<void(*)(int,const unsigned*)>(glfwGetProcAddress("glDeleteQueries"));if(gpuQueries&&deleteQueries)deleteQueries(16,queries);sync_present::shutdown();CloseWindow();return 0;
}
