#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
extern "C" __declspec(dllimport) BOOL __stdcall GetClientRect(HWND,RECT*);
// Read actual hidden GL/DXGI presentation pixels. This is not a foreground FPS
// benchmark and never changes Windows display settings or restores a window.
int main(int argc,char** argv){
 if(argc!=4||std::string(argv[1])!="--no-audio"||std::string(argv[3])!="--background-test")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_ALWAYS_RUN|FLAG_WINDOW_RESIZABLE|FLAG_WINDOW_HIGHDPI|FLAG_VSYNC_HINT);
 InitWindow(1920,1080,"Emberfall image clarity --no-audio");if(!IsWindowReady())return 5;updateFramePacing(true);audioSystem.init(false,false);
 makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();
 game=Game{};game.activateField(WorldLocation::Volcano);game.player=VolcanoArrival;game.messageTime=0;welcome=help=paused=false;intro={};towerStory={};mouse={0,0};refreshDisplayLayout();game.time=1;
 renderScene();Image scene=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&scene);ExportImage(scene,(root/"scene-canvas.png").string().c_str());UnloadImage(scene);
 BeginTextureMode(canvas);BeginMode2D({{0,0},{0,0},0,renderScale});DrawRectangle(30,100,970,135,rgb(29,34,41));
 txt("Ostrość liter: Zażółć gęślą jaźń · 0123456789 · ŻYCIE / MANA",45,114,17,Text);
 txt("EKWIPUNEK  |  UMIEJĘTNOŚCI  |  KALDERA POPIOŁU",45,151,20,Gold,true);
 txt("Naturalne wygładzanie krawędzi, bez ponownego mieszania alfy.",45,193,15,WHITE);
 EndMode2D();EndTextureMode();Image expected=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&expected);ExportImage(expected,(root/"text-canvas.png").string().c_str());
 std::ofstream details(root/"checks.txt");int totalMismatch=0,maxError=0;bool hidden=true;int modes=0;
 for(float dpi:{1.f,1.25f,1.5f,2.f}){
  BeginDrawing();rlMatrixMode(RL_MODELVIEW);rlLoadIdentity();rlScalef(dpi,dpi,1);presentCanvas();Image actual=presentedImage();
  auto* a=static_cast<Color*>(expected.data);auto* b=static_cast<Color*>(actual.data);int mismatch=0,alphaEdges=0;
  for(int y=0;y<expected.height;y++)for(int x=0;x<expected.width;x++){
   Color s=a[y*expected.width+x],t=b[(y+int(presentationViewport.y))*actual.width+x+int(presentationViewport.x)];
   int error=std::max({std::abs(int(s.r)-t.r),std::abs(int(s.g)-t.g),std::abs(int(s.b)-t.b)});mismatch+=error>1;maxError=std::max(maxError,error);alphaEdges+=s.a<255;
  }
  details<<"DPI="<<dpi<<" mismatched RGB="<<mismatch<<" max error="<<maxError<<" antialiased pixels="<<alphaEdges<<'\n';totalMismatch+=mismatch;modes++;
  if(dpi==1)ExportImage(actual,(root/"text-presented.png").string().c_str());UnloadImage(actual);EndDrawing();finishFramePacing();hidden&=IsWindowHidden();
 }
 updateFramePacing(true);UnloadImage(expected);
 RECT client{};GetClientRect(static_cast<HWND>(GetWindowHandle()),&client);
 std::ofstream report(root/"summary.json");report<<"{\"backend\":\""<<pacingName()<<"\",\"width\":"<<GetRenderWidth()<<",\"height\":"<<GetRenderHeight()<<",\"physical_client_width\":"<<client.right<<",\"physical_client_height\":"<<client.bottom<<",\"window_dpi\":"<<GetDpiForWindow(static_cast<HWND>(GetWindowHandle()))<<",\"monitor_hz\":"<<refreshHz<<",\"swap_interval\":"<<swapInterval<<",\"actual_last_sync_interval\":"<<sync_present::lastSyncInterval()<<",\"canvas_width\":"<<canvas.texture.width<<",\"canvas_height\":"<<canvas.texture.height<<",\"dpi_modes\":"<<modes<<",\"rgb_mismatches\":"<<totalMismatch<<",\"max_rgb_error\":"<<maxError<<",\"hidden\":"<<(hidden?"true":"false")<<",\"audio_opened\":false,\"foreground_fps_measured\":false}";
 audioSystem.shutdown();CloseWindow();return hidden?0:6;
}
