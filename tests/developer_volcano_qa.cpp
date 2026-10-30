#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include <iostream>
int main(int argc,char** argv){
 if(argc!=4||std::string(argv[1])!="--no-audio"||std::string(argv[3])!="--background-test")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_ALWAYS_RUN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall DEV volcano --no-audio");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 int checks=0,failures=0;std::ofstream log(root/"checks.txt");auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;log<<(ok?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 auto point=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
 auto reset=[](){game=Game{};intro={};towerStory={};clearTransientUi();welcome=help=paused=false;developerActive=developerMenuOpen=false;developerKit=false;developerSelection=0;mouse={0,0};game.messageTime=0;};
 auto capture=[&](const std::string& name){renderScene();Image im=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&im);check(ExportImage(im,(root/(name+".png")).string().c_str()),"real developer UI render / "+name);UnloadImage(im);check(IsWindowHidden(),"test window remains hidden / "+name);};
 reset();openDeveloperMenu();check(!PlayerBuild&&DeveloperDestinationCount==19,"developer edition exposes all nineteen destinations");
 for(int id=0;id<DeveloperDestinationCount;id++){
  Rectangle r=developerCard(id);check(r.x>=140&&r.y>=220&&r.x+r.width<=1300&&r.y+r.height<developerKitButton().y,"location card stays inside picker above controls");
  check(textWidth(DeveloperDestinations[id].name,18)<=r.width-26&&textWidth(DeveloperDestinations[id].hint,14)<=r.width-26,"location name and hint fit the actual bold font");
  for(int other=0;other<id;other++)check(!CheckCollisionRecs(r,developerCard(other)),"destination cards never overlap");
  point(r);clickDeveloperMenu();check(developerSelection==id&&developerMenuOpen,"actual card click selects the corresponding destination");
 }
 for(float scale:{.8f,1.f,1.2f}){prepareCanvas(scale);capture("picker-"+std::to_string(int(scale*100)));}
 prepareCanvas(1.2f);
 for(int id=16;id<19;id++){
  reset();openDeveloperMenu();point(developerCard(id));clickDeveloperMenu();point(developerNewButton());check(clickDeveloperMenu(),"actual New Session button accepts each volcanic destination");
  check(game.inVolcano()&&developerActive&&!developerMenuOpen&&gameplayInputAllowed()&&developerSpawnSafe(game,game.player),"new volcanic session arrives safely with working controls");
  check(game.level==1&&game.skillRanks==SkillRanks{}&&game.findEnemy(VolcanoBossId)&&!game.volcanoBossDead,"fresh volcanic test keeps skills locked and boss alive");
  capture(std::string(DeveloperDestinations[id].id)+"-new");
 }
 game.level=8;game.xp=31;game.bag[12]=item(Kind::GuardianAxe,2);auto oldBag=game.bag;auto oldGear=game.equipment;
 game.fissures.push_back({game.player,0,.95f,48,VolcanoBossId,false});game.cinders.push_back({game.player,{7.5f,0},7,27,2002});game.heroKnockdownRemaining=.5f;
 openDeveloperMenu();point(developerCard(17));clickDeveloperMenu();point(developerMoveButton());check(clickDeveloperMenu(),"actual Move Character button reaches the volcanic forges");
 bool same=true;for(int i=0;i<BagSize;i++)same&=game.bag[i].kind==oldBag[i].kind&&game.bag[i].count==oldBag[i].count&&game.bag[i].tier==oldBag[i].tier;
 for(int i=0;i<EquipSize;i++)same&=game.equipment[i].kind==oldGear[i].kind&&game.equipment[i].tier==oldGear[i].tier;
 check(game.level==8&&game.xp==31&&same&&game.cinders.empty()&&game.fissures.empty()&&!game.knockedBack(),"move preserves real character and clears pending attacks");capture("forges-current-character");
 openDeveloperMenu();point(developerCard(18));clickDeveloperMenu();capture("picker-varkhor-selected");
 check(!audioSystem.available&&audioSystem.playedEffects==0&&IsWindowHidden(),"entire developer QA remained silent and hidden");
 std::ofstream report(root/"summary.json");report<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"destinations\":19,\"volcanic_destinations\":3,\"hidden\":true,\"audio_opened\":false}";
 std::cout<<"Developer volcano UI: "<<checks<<" checks, "<<failures<<" failures\n";audioSystem.shutdown();CloseWindow();return failures?2:0;
}
