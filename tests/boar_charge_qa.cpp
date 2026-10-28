#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include <iostream>
int main(int argc,char** argv){
 if(argc!=4||std::string(argv[1])!="--no-audio"||std::string(argv[3])!="--background-test")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_ALWAYS_RUN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall boar charge --no-audio");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1.2f);
 int checks=0,failures=0;std::ofstream log(root/"checks.txt");
 auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;log<<(ok?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 auto setup=[](V direction){
  game=Game{};intro={};towerStory={};clearTransientUi();welcome=help=paused=false;mouse={0,0};game.messageTime=0;
  game.props.clear();game.enemies.clear();game.forage.clear();game.player={20,40};
  V at=game.player-normalized(direction)*4;
  game.enemies.push_back({91,BoarType,at,at,88,88});game.enemies[0].boarChargeCd=0;
 };
 auto capture=[&](const std::string& name){
  renderScene();Image image=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&image);
  check(image.data&&image.width==int(W*renderScale)&&image.height==int(H*renderScale),"real scene decoded / "+name);
  check(ExportImage(image,(root/(name+".png")).string().c_str()),"saved integrated frame / "+name);UnloadImage(image);
  check(IsWindowHidden(),"test window stays hidden / "+name);
 };
 int capacity=actor_art::creatureCapacity;unsigned shadowAllocations=shadow_geometry::allocations;
 std::array<bool,4> seen{};std::vector<double> frameTimes;
 auto finish=reinterpret_cast<void(*)()>(glfwGetProcAddress("glFinish"));
 for(int side=0;side<8;side++){
  V direction{std::cos(side*PI/4),std::sin(side*PI/4)};setup(direction);seen.fill(false);
  float expected=game.incomingDamage(float(game.enemies[0].damage()),DamageType::Physical)*2;
  for(int frame=0;frame<120;frame++){
   game.update(1.f/60,{},false);auto& boar=game.enemies[0];
   int phase=boar.boarWindup>.1f?0:boar.boarRush>0?(boar.boarHit?2:1):boar.boarRecovery>0?3:-1;
   if(phase>=0&&!seen[phase]&&(phase!=0||boar.boarWindup<.35f)){
    static const char* names[]={"warning","rush","hit","recovery"};seen[phase]=true;capture(std::string(names[phase])+"-"+std::to_string(side));
   }
   double start=GetTime();renderScene();rlDrawRenderBatchActive();if(finish)finish();
   if(frame>10)frameTimes.push_back((GetTime()-start)*1000);
   audioSystem.update(1.f/60,game,false,false);
  }
  for(bool phase:seen)check(phase,"actual rendered gameplay includes all four charge phases / "+std::to_string(side));
  check(std::abs(game.maxHp()-game.hp-expected)<.001f,"rendered charge deals one stronger hit / "+std::to_string(side));
  check(game.enemies[0].boarCharges==1,"rendered charge respects its cooldown / "+std::to_string(side));
 }
 setup({1,0});game.update(.05f,{},false);renderScene();mouse=screen(game.enemies[0].pos);mouse.y-=22;hovered=pickWorld(mouse);
 check(hovered.kind==HoverKind::Enemy&&hovered.index==91,"real body picking selects the boar for inspection");
 check(inspectWorld(hovered).secondary.find("Szarża")!=std::string::npos,"boar inspection explains its special ability");capture("inspection");
 check(hovered.kind==HoverKind::Enemy&&hovered.index==91,"rendering preserves the actual boar inspection target");
 check(textWidth(inspectWorld(hovered).secondary,14)<=301,"charge description fits existing HUD");
 // Verify the authored forest layout as well as the isolated contact arena.
 game=Game{};intro={};towerStory={};clearTransientUi();welcome=help=paused=false;mouse={0,0};game.messageTime=0;
 int forestBoar=-1;
 for(const auto& e:game.enemies)if(e.type==BoarType&&forestBoar<0){
  for(int angle=0;angle<40;angle++){
   V target=e.pos+V{std::cos(angle*PI/20),std::sin(angle*PI/20)}*4;
   if(game.enemyPositionClear(target,HeroBodyRadius)&&game.clearEnemySegment(e.pos,target,e.radius()+.01f)){
    forestBoar=e.id;game.player=target;break;
   }
  }
 }
 check(forestBoar>=0,"real forest has a valid charge encounter with its original trees and creatures");
 if(auto* boar=game.findEnemy(forestBoar)){
  boar->boarChargeCd=0;V start=boar->pos;
  for(int frame=0;frame<15;frame++)game.update(1.f/60,{},false);capture("forest-warning");
  for(int frame=0;frame<60&&boar->boarCharges==0;frame++)game.update(1.f/60,{},false);
  for(int frame=0;frame<4;frame++)game.update(1.f/60,{},false);
  check(boar->boarCharges==1&&distance(start,boar->pos)>.05f,"boar launches and moves in the unmodified forest encounter");capture("forest-rush");
 }
 check(actor_art::creatureCapacity==capacity,"charges reuse the preloaded creature buffer");
 check(shadow_geometry::allocations==shadowAllocations,"charge effects do not grow shadow storage");
 check(PlayerBuild&&!developerActive,"integrated QA uses player edition without developer mode");
 check(!audioSystem.available&&audioSystem.playedEffects==0,"all game and charge tests opened no audio and played no effects");
 std::sort(frameTimes.begin(),frameTimes.end());double mean=std::accumulate(frameTimes.begin(),frameTimes.end(),0.)/frameTimes.size(),p95=frameTimes[(frameTimes.size()-1)*95/100];
 std::ofstream out(root/"summary.json");out<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"directions\":8,\"captured_frames\":35,\"original_forest_encounter\":true,\"hidden\":true,\"audio_opened\":false,\"presentation_measured\":false,\"completed_render_mean_ms\":"<<mean<<",\"completed_render_p95_ms\":"<<p95<<"}";
 std::cout<<"Boar charge integration: "<<checks<<" checks, "<<failures<<" failures; hidden render mean "<<mean<<" ms\n";
 audioSystem.shutdown();CloseWindow();return failures?2:0;
}
