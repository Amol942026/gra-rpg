// GPU-complete offscreen render timing. Does not measure display frame pacing.
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#include <iostream>
#undef main
extern "C" void __stdcall glFinish();
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path output=argv[2];if(fs::exists(output))return 4;
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall offscreen resource QA --no-audio");
 if(!IsWindowReady())return 5;audioSystem.init(false,false);
 makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 std::ofstream report(output);report<<"{\"status\":\"MEASURED\",\"audio_opened\":false,\"presentation_measured\":false,\"samples\":[";
 for(int scene=0;scene<5;scene++){
  game=Game{};intro={};towerStory={};clearTransientUi();welcome=help=paused=false;mouse={0,0};
  if(scene<2){game.activateField(WorldLocation::Desert);game.dawnQuest=DawnQuest::AtTower;game.player=DesertFountains[scene]+V{1.6f,0};}
  else if(scene==2){game.player=ForestGuardian+V{4,0};game.enemies.back().boulderCd=0;}
  else if(scene==3){game.activateField(WorldLocation::Desert);game.player=DesertBossHome+V{4,1};game.hit(*game.findEnemy(DesertBossId),10000);Attributes points{};points[0]=game.attributePoints;game.distribute(points);SkillRanks skills{};skills[0]=game.skillPoints;game.trainSkills(skills);game.effects.clear();game.messageTime=0;}
  else openStats();
  std::vector<double> times;
  for(int frame=0;frame<240;frame++){
   double start=GetTime();if(!statsOpen&&!skillsOpen)game.update(1.f/60,{},false);game.hp=float(game.maxHp());renderScene();rlDrawRenderBatchActive();glFinish();
   double ms=(GetTime()-start)*1000;if(frame>=60)times.push_back(ms);
  }
  if(scene==3){Image image=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&image);ExportImage(image,"resource-dune-world.png");UnloadImage(image);}
  std::sort(times.begin(),times.end());double mean=std::accumulate(times.begin(),times.end(),0.)/times.size(),p95=times[size_t(times.size()*.95)];
  if(scene)report<<',';report<<"{\"scene\":\""<<(scene==0?"mana-fountain":scene==1?"health-fountain":scene==2?"guardian":scene==3?"fallen-dune":"stats-menu")<<"\",\"frames\":180,\"mean_ms\":"<<mean<<",\"p95_ms\":"<<p95<<",\"p99_ms\":"<<times[size_t(times.size()*.99)]<<",\"max_ms\":"<<times.back()<<",\"render_budget_60hz\":"<<(p95<=1000./60?"true":"false")<<"}";report.flush();
  std::cout<<scene<<": mean "<<mean<<" ms, p95 "<<p95<<" ms\n";
 }
 report<<"]}";report.close();fountain_art::unload();guardian_art::unload();CloseWindow();return 0;
}
