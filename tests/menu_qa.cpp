#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#include <iostream>
#undef main
using MenuKeyCallback=void (*)(GLFWwindow*,int,int,int,int);
using MenuFocusCallback=void (*)(GLFWwindow*,int);
extern "C" MenuKeyCallback glfwSetKeyCallback(GLFWwindow*,MenuKeyCallback);
extern "C" MenuFocusCallback glfwSetWindowFocusCallback(GLFWwindow*,MenuFocusCallback);
#include "forest_menu_checks.hpp"
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path output=fs::absolute(argv[2]);if(fs::exists(output))return 4;fs::create_directories(output);
 menuSaveRoot=output/fs::u8path(u8"zapisy-żółć");
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_RESIZABLE);sync_present::useOpenGL();InitWindow(W,H,"Emberfall menu QA --no-audio");if(!IsWindowReady())return 5;SetExitKey(KEY_NULL);
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 int checks=0,failures=0;std::ofstream report(output/"checks.txt");auto check=[&](bool ok,const std::string& label){checks++;if(!ok)failures++;report<<(ok?"PASS ":"FAIL ")<<label<<'\n';report.flush();};
 auto shot=[&](const std::string& name){renderScene();Image im=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&im);check(ExportImage(im,(output/(name+".png")).string().c_str()),"screenshot "+name);Color* colors=LoadImageColors(im);int visible=0,transparent=0;for(int n=0;n<im.width*im.height;n++){visible+=colors[n].r>50||colors[n].g>50||colors[n].b>50;transparent+=colors[n].a!=255;}check(visible>im.width*im.height/500,"nonempty rendered menu "+name);check(transparent==0,"opaque final menu without double blending "+name);check(!native_world::drawingWorld,"menu restores render blending "+name);UnloadImageColors(colors);UnloadImage(im);};
 auto click=[&](int row){Rectangle r=menuRowRect(row);mouse={r.x+r.width*.5f,r.y+r.height*.5f};gameMenuEvent(false,0,false,true);};
 auto key=[&](int vertical=0,bool accept=false,bool escape=false,int horizontal=0){gameMenuEvent(escape,vertical,accept,false,horizontal);};
 auto start=[&]{game=Game{};intro={};towerStory={};clearTransientUi();welcome=true;paused=false;menuPageTo(MenuPage::Home);mouse={0,0};};
 check(!audioSystem.available,"no audio device opened");check(PlayerBuild,"uses actual player edition");start();
 // Inject through the existing raylib callbacks, then call production input().
 // This verifies event routing as well as the shared action dispatcher below.
 GLFWwindow* window=glfwGetCurrentContext();auto nativeKey=glfwSetKeyCallback(window,nullptr);glfwSetKeyCallback(window,nativeKey);auto nativeFocus=glfwSetWindowFocusCallback(window,nullptr);glfwSetWindowFocusCallback(window,nativeFocus);
 check(nativeKey&&nativeFocus,"native raylib keyboard and focus callbacks available");
 if(nativeKey&&nativeFocus){auto release=[&](int code){nativeKey(window,code,0,0,0);BeginDrawing();ClearBackground(BLACK);EndDrawing();nativeFocus(window,1);};
  nativeFocus(window,1);nativeKey(window,KEY_ENTER,0,1,0);input(.016f);check(intro.active()&&!welcome,"real input Enter starts New game");release(KEY_ENTER);
  nativeKey(window,KEY_ESCAPE,0,1,0);input(.016f);check(paused&&intro.active(),"real input Escape opens pause during prologue");release(KEY_ESCAPE);
  float frozen=intro.time;nativeKey(window,KEY_DOWN,0,1,0);input(.016f);check(paused&&menuSelection==1&&intro.time==frozen,"real Down navigates without advancing story");release(KEY_DOWN);
  nativeKey(window,KEY_ESCAPE,0,1,0);input(.016f);check(!paused&&intro.time==frozen,"real Escape resumes without advancing same frame");release(KEY_ESCAPE);
 }start();
 auto initial=saves::encode({game,intro,towerStory,{}});for(int i=0;i<20;i++)input(.016f);check(welcome&&!intro.active()&&saves::encode({game,intro,towerStory,{}})==initial,"startup menu freezes gameplay before prologue");
 for(float factor:{.8f,1.f,1.2f}){prepareCanvas(factor);mouse={0,0};shot("main-"+std::to_string(int(factor*100)));for(int row=0;row<5;row++){Rectangle r=menuRowRect(row);check(r.x>=0&&r.y>=0&&r.x+r.width<=W&&r.y+r.height<=H,"home hit target inside canvas");check(textWidth(std::array<const char*,5>{"Nowa gra","Wczytaj grę","Opcje","Twórcy","Wyjdź z gry"}[row],22)<r.width-50,"full Polish home label fits");}}
 prepareCanvas(1);key(-1);check(menuSelection==4,"keyboard wraps upward");key(1);check(menuSelection==0,"keyboard wraps downward");key(1);key(0,true);check(menuPage==MenuPage::Load&&welcome,"keyboard opens load at startup");check(!menuSaves[0].exists,"empty save slot metadata");click(0);check(welcome&&menuPage==MenuPage::Load&&!menuNotice.empty(),"empty load preserves startup and gives feedback");shot("load-empty");key(0,false,true);check(menuPage==MenuPage::Home,"Escape returns from submenu");key(0,false,true);check(welcome,"Escape cannot bypass startup menu");
 click(2);check(menuPage==MenuPage::Options,"mouse opens options");float m=audioSystem.musicVolume,e=audioSystem.effectsVolume;bool mute=audioSystem.muted,mm=audioSystem.musicMuted,em=audioSystem.effectsMuted;
 menuSelection=0;key(0,false,false,1);check(audioSystem.musicVolume>m,"keyboard volume adjustment");Rectangle slider=menuOption(0);mouse={800,slider.y+25};gameMenuEvent(false,0,false,true);check(std::abs(audioSystem.musicVolume-.5f)<.001f,"mouse music slider");mouse={410,menuOption(0).y+25};gameMenuEvent(false,0,false,true);check(audioSystem.musicMuted!=mm,"mouse toggles music separately");mouse={410,menuOption(1).y+25};gameMenuEvent(false,0,false,true);check(audioSystem.effectsMuted!=em,"mouse toggles effects separately");click(3);check(audioSystem.muted!=mute,"master mute works");bool fullscreen=fullScreen();click(2);check(fullScreen()!=fullscreen,"options switches actual screen mode");click(2);check(fullScreen()==fullscreen,"screen mode restores");
 audioSystem.musicVolume=m;audioSystem.effectsVolume=e;audioSystem.muted=mute;audioSystem.musicMuted=mm;audioSystem.effectsMuted=em;prepareCanvas(1);mouse={0,0};shot("options");key(0,false,true);click(3);check(menuPage==MenuPage::Credits,"credits opens");shot("credits");key(0,false,true);click(4);check(menuPage==MenuPage::Exit&&!exitRequested,"exit requires confirmation");shot("exit-startup");click(1);check(menuPage==MenuPage::Home&&!exitRequested,"cancel exit preserves menu");
 game.gold=123;click(0);check(intro.active()&&!welcome&&!paused&&game.gold==0,"new game resets state and starts prologue");intro.phase=IntroPhase::Dialogue;intro.line=4;intro.age=.5f;intro.time=9;
 key(0,false,true);check(paused&&intro.phase==IntroPhase::Dialogue&&intro.line==4,"Escape pauses prologue instead of skipping");auto age=intro.age;for(int i=0;i<20;i++)input(.05f);check(intro.age==age,"dialogue clock frozen under menu");shot("pause-prologue");click(0);check(menuPage==MenuPage::Save,"pause has functioning save action");click(0);check(menuSaves[0].valid&&menuPage==MenuPage::Save,"first slot written through mouse");shot("save-prologue");
 click(0);check(menuPage==MenuPage::Overwrite,"overwrite needs confirmation");click(1);check(menuPage==MenuPage::Save,"overwrite cancelled");click(0);game.gold=47;click(0);check(menuSaves[0].valid&&menuStore().load(0).game.gold==47,"confirmed overwrite saved state");
 key(0,false,true);click(1);click(0);check(menuPage==MenuPage::ReplaceSession,"in-game load confirms session replacement");click(1);check(game.gold==47&&menuPage==MenuPage::Load,"load cancel leaves active game untouched");intro.line=8;game.gold=999;click(0);click(0);check(paused&&!welcome&&intro.line==4&&game.gold==47,"load restores dialogue and game atomically");key(0,false,true);check(!paused&&intro.active(),"Escape resumes restored prologue");
 intro={};towerStory={};game=Game{};game.player={20,23};game.gold=878;game.chests[0].opened=true;game.chests[0].loot[1]={};game.hit(game.enemies[0],9999);openPauseMenu();click(0);click(1);check(menuSaves[1].valid,"forest saved in second slot");key(0,false,true);shot("pause-forest");
 game.activateField(WorldLocation::Desert);game.player=DesertArrival;game.chests[2].opened=true;game.collectedFragments=1;game.dawnQuest=DawnQuest::AtTower;game.gold=2121;openPauseMenu();click(0);click(2);check(menuSaves[2].valid&&menuStore().load(2).game.inDesert(),"desert saved in third slot");key(0,false,true);shot("pause-desert");
 welcome=true;paused=false;intro={};towerStory={};menuPageTo(MenuPage::Home);click(1);shot("load-populated");click(1);check(!welcome&&paused&&!game.inDesert()&&game.gold==878&&game.player.x==20&&!game.enemies[0].alive&&game.chests[0].loot[1].empty(),"startup load restores actual inventory, enemy, position, chest and gold");
 check(!gameplayInputAllowed(),"world input blocked by pause");V position=game.player;float clock=game.time;for(int i=0;i<40;i++)input(.016f);check(game.player.x==position.x&&game.player.y==position.y&&game.time==clock,"simulation frozen in pause");
 for(float factor:{.8f,1.2f}){prepareCanvas(factor);shot("pause-"+std::to_string(int(factor*100)));click(2);shot("options-"+std::to_string(int(factor*100)));key(0,false,true);}
 prepareCanvas(1);menuSelection=4;key(0,true);check(!paused,"keyboard resume action");
 game.allocationRequired=true;game.attributePoints=5;game.skillPoints=1;game.skillRanks={};openPauseMenu();click(0);menuSlot=1;menuSave();key(0,false,true);game.allocationRequired=false;game.attributePoints=0;click(1);click(1);click(0);check(game.allocationRequired&&game.attributePoints==5&&game.skillPoints==1&&game.skillRanks[0]==0,"unconfirmed level points preserved without unlocking skills");
 game.allocationRequired=false;game.enterMageTower();game.towerFloor=3;game.towerRuneMask=7;towerStory.beginTalk(true,1);towerStory.phase=TowerPhase::Choice;openPauseMenu();click(0);menuSlot=2;menuSave();key(0,false,true);towerStory={};click(1);click(2);click(0);check(game.inTower()&&game.towerFloor==3&&game.towerRuneMask==7&&towerStory.phase==TowerPhase::Choice,"tower floor, runes and travel dialogue roundtrip");shot("pause-tower");
 game.hp=0;openPauseMenu();shot("pause-death");key(0,false,true);check(!paused&&game.dead(),"resume preserves death rather than reviving");
 openPauseMenu();menuPageTo(MenuPage::Load);menuSlot=0;
 auto primary=menuStore().path(0),recovery=fs::path(primary.wstring()+L".bak");{std::ofstream out(primary,std::ios::binary|std::ios::trunc);out<<"broken";}{std::ofstream out(recovery,std::ios::binary|std::ios::trunc);out<<"also broken";}
 auto live=saves::encode({game,intro,towerStory,{}});menuLoad();check(saves::encode({game,intro,towerStory,{}})==live&&!menuNotice.empty(),"failed load leaves live session byte-for-byte unchanged");menuPageTo(MenuPage::Home);
 click(3);check(menuPage==MenuPage::Exit&&!exitRequested,"pause exit confirmation");shot("exit-ingame");click(0);check(exitRequested,"confirmed exit sets actual application loop condition");exitRequested=false;
 check(!audioSystem.available&&audioSystem.playedEffects==0,"no playback during entire diagnostic");check(!fs::exists(menuSaveRoot/"slot-4.efs"),"only three slots used");
 forestMenuChecks(output,check);
 menu_scene::unload();
 report<<"Checks: "<<checks<<"\nFailures: "<<failures<<"\nAudio device opened: no\nPlayer edition: yes\n";report.close();
 audioSystem.shutdown();world_shadows::unload();actor_art::finishShadowFrame();fallen_art::unload();unloadHoverArt();unloadSkillArt();unloadTowerRoomArt();tower_stairs::unload();tower_furniture::unload();desert_art::unload();sand_prison::unload();treasure_art::unload();actor_art::unloadCreatureBatch();hero_paint::unload();creature_paint::unload();for(auto& t:propArt)UnloadRenderTexture(t);UnloadRenderTexture(terrain);UnloadTexture(minimapTerrain);if(canvas.id)UnloadRenderTexture(canvas);if(worldCanvas.id)UnloadRenderTexture(worldCanvas);typography.clear();CloseWindow();std::cout<<"Menu QA: "<<checks<<" checks, "<<failures<<" failures\n";return failures?2:0;
}
