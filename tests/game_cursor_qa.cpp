#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
extern "C" int glfwGetInputMode(GLFWwindow*,int);
int main(int argc,char** argv){
 if(argc!=5||std::string(argv[1])!="--no-audio"||std::string(argv[3])!="--background-test")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 if(std::string(argv[4])=="opengl")sync_present::useOpenGL();
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_ALWAYS_RUN|FLAG_WINDOW_RESIZABLE|FLAG_WINDOW_HIGHDPI);
 InitWindow(1440,900,"Emberfall native cursor --no-audio");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);game_cursor::initialize();int checks=0,failures=0,frames=0;
 std::ofstream log(root/"checks.txt");auto check=[&](bool b,const char* label){checks++;failures+=!b;log<<(b?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 GLFWwindow* window=glfwGetCurrentContext();auto callback=glfwSetCursorPosCallback(window,nullptr);glfwSetCursorPosCallback(window,callback);
 check(callback&&!IsCursorHidden()&&glfwGetInputMode(window,0x33001)==0x34001,"startup uses normal unhidden Windows cursor mode");
 for(Vector2 dimensions:{Vector2{1440,900},Vector2{1600,1000},Vector2{1152,900}}){
  SetWindowSize(int(dimensions.x),int(dimensions.y));PollInputEvents();refreshDisplayLayout();
  for(int point=0;point<3;point++){
   Vector2 raw{point==0?13.f:point==1?GetScreenWidth()*.5f:GetScreenWidth()*.85f,point==0?80.f:point==1?GetScreenHeight()*.45f:GetScreenHeight()*.72f};
   callback(window,raw.x,raw.y);refreshDisplayLayout();
   check(std::abs((presentationViewport.x+mouse.x*renderScale)-raw.x*GetRenderWidth()/GetScreenWidth())<.01f&&std::abs((presentationViewport.y+mouse.y*renderScale)-raw.y*GetRenderHeight()/GetScreenHeight())<.01f,"native click coordinates remain correct through resizing and margins");
   BeginTextureMode(canvas);ClearBackground(rgb(113,44,40));EndTextureMode();
   Image expected=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&expected);BeginDrawing();presentCanvas();Image actual=presentedImage();
   int differences=0;for(int y=0;y<expected.height;y++)for(int x=0;x<expected.width;x++){Color a=GetImageColor(expected,x,y),b=GetImageColor(actual,x+int(presentationViewport.x),y+int(presentationViewport.y));differences+=std::max({std::abs(int(a.r)-b.r),std::abs(int(a.g)-b.g),std::abs(int(a.b)-b.b)})>1;}
   check(differences==0,"final framebuffer contains only the game image and no drawn pointer");
   UnloadImage(actual);UnloadImage(expected);EndDrawing();frames++;
   check(IsWindowHidden()&&!IsCursorHidden()&&glfwGetInputMode(window,0x33001)==0x34001,"hidden renderer retains normal native cursor mode without focusing or capturing");
  }
 }
 game_cursor::shutdown();check(!IsCursorHidden()&&!audioSystem.available&&IsWindowHidden(),"shutdown remains hidden and silent with native cursor enabled");
 std::ofstream report(root/"summary.json");report<<"{\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"frames\":"<<frames<<",\"software_cursor\":false,\"hidden\":true,\"audio_opened\":false,\"physical_foreground_cursor_tested\":false}";
 audioSystem.shutdown();sync_present::shutdown();UnloadRenderTexture(canvas);typography.clear();CloseWindow();return failures?2:0;
}