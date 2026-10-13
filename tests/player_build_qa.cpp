// Exercise the same compiled UI/input code as the distributed player EXE.
#ifndef EMBERFALL_PLAYER_BUILD
#error This integration test requires the player build.
#endif
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
using KeyCallback=void (*)(GLFWwindow*,int,int,int,int);
extern "C" KeyCallback glfwSetKeyCallback(GLFWwindow*,KeyCallback);

int main(int argc,char** argv){
 if(argc!=2||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());
 fs::create_directories("qa-v41");
 std::ofstream report("qa-v41/player-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';};
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);
 InitWindow(W,H,"Emberfall player integration QA");
 if(!IsWindowReady())return 4;
 audioSystem.init(false,false);
 check(PlayerBuild&&!audioSystem.available,"player edition opens no audio during diagnostics");
 game=Game{};startIntro();
 check(intro.active()&&!developerActive&&!developerMenuOpen,"normal start preserves the story prologue");
 check(game.level==1&&game.ammoCount()==20&&game.skillRanks==SkillRanks{}&&game.bagCount(Kind::Mushroom)==0,"normal character receives no developer kit or unlocked skills");

 GLFWwindow* window=glfwGetCurrentContext();
 KeyCallback key=glfwSetKeyCallback(window,nullptr);glfwSetKeyCallback(window,key);
 check(key!=nullptr,"actual raylib keyboard callback is available");
 if(key){
  key(window,KEY_F3,0,1,0);
  check(IsKeyPressed(KEY_F3),"F3 test reaches the actual raylib input state");
  check(!developerInput()&&!developerMenuOpen&&!developerActive,"F3 cannot open a developer menu during the prologue");
  key(window,KEY_F3,0,0,0);
 }

 intro={};game.dawnQuest=DawnQuest::AtTower;welcome=paused=help=false;
 mouse={107,170};
 check(worldPointerAllowed(mouse),"former DEV button area remains usable world space");
 check(!developerInput()&&!developerMenuOpen,"former DEV area does not open a hidden menu");
 check(!CheckCollisionPointRec(mouse,developerButton()),"no invisible developer button intercepts the mouse");
 if(key){
  key(window,KEY_F3,0,1,0);
  check(IsKeyPressed(KEY_F3)&&!developerInput()&&!developerMenuOpen&&!developerActive,"F3 cannot enable developer mode in the playable world");
  key(window,KEY_F3,0,0,0);
 }

 RenderTexture2D screen=LoadRenderTexture(W,H);
 BeginTextureMode(screen);ClearBackground(BLACK);drawDeveloperUi();EndTextureMode();
 Image picture=LoadImageFromTexture(screen.texture);
 Color* pixels=LoadImageColors(picture);int changed=0;
 for(int i=0;pixels&&i<picture.width*picture.height;i++)if(pixels[i].r||pixels[i].g||pixels[i].b)changed++;
 check(pixels&&changed==0,"player HUD renders no DEV button, label or menu");
 if(pixels)UnloadImageColors(pixels);UnloadImage(picture);UnloadRenderTexture(screen);

 game.props.clear();game.enemies.clear();game.forage.clear();game.player={15,15};game.messageTime=0;
 V pos=game.player+V{.8f,0};game.enemies.push_back({42,0,pos,pos,500,500});
 Vector2 target=::screen(pos);mouse={target.x,target.y-18};
 leftClickGameplay({});
 check(game.swing>0&&game.enemies[0].hp<500,"ordinary sword combat still works in the player build");
 check(castGameplaySpell()&&game.mana==90&&game.fireballs.size()==1,"ordinary spell casting still works in the player build");

 audioSystem.shutdown();CloseWindow();
 report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';report.close();
 std::printf("Player edition: %d/%d checks passed\n",checks-failures,checks);
 return failures?2:0;
}
