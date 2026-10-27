#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include <iostream>
extern "C" void __stdcall glFinish();
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall balcony --no-audio");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 int checks=0,failures=0;std::ofstream log(root/"checks.txt");
 auto check=[&](bool pass,const std::string& label){checks++;failures+=!pass;log<<(pass?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 auto reset=[](){game=Game{};intro={};towerStory={};clearTransientUi();welcome=paused=help=false;game.enterMageTower();game.dawnQuest=DawnQuest::AtTower;game.towerFloor=TowerBalconyFloor;game.player=TowerCenter;game.effects.clear();game.messageTime=0;mouse={0,0};};
 reset();
 check(PlayerBuild&&!audioSystem.available,"actual player renderer has no audio device");
 check(tower_balcony::valley.id&&tower_balcony::valley.width==1050&&tower_balcony::valley.height==900,"Krita panorama preloads at its native viewport dimensions");
 check(tower_balcony::back.id&&tower_balcony::front.id&&tower_balcony::back.width==480&&tower_balcony::front.height==320,"both depth layers preload before game rendering");
 for(int floor=0;floor<TowerFloorCount;floor++){
  game.towerFloor=floor;
  check(game.walkable(TowerArrival),"all original stair arrival positions remain safe");
  check(game.walkable(towerBalconyPoint(9))==(floor==TowerBalconyFloor),"only the observatory has a real exterior terrace");
 }
 reset();
 // Simulate walking with the production update, including its collision sliding.
 const V outward=normalized(towerBalconyPoint(1)-TowerCenter);
 for(int frame=0;frame<600;frame++){
  game.update(1.f/60,outward,false,frame>300);V q=towerBalconyLocal(game.player);
  check(game.walkable(game.player)&&q.x<11.121f&&std::abs(q.y)<=2.121f,"walk and sprint cannot cross the far balustrade");
 }
 check(towerBalconyLocal(game.player).x>10.9f,"normal movement reaches the outer viewpoint from inside the room");
 for(int side:{-1,1}){
  game.player=towerBalconyPoint(9);V across=normalized(towerBalconyPoint(9,float(side))-game.player);
  for(int frame=0;frame<180;frame++){game.update(1.f/60,across,false,true);V q=towerBalconyLocal(game.player);check(std::abs(q.y)<=2.121f&&q.x<11.121f,"both side rails stop real running movement");}
  check(std::abs(towerBalconyLocal(game.player).y)>1.9f,"each side of the balcony can be approached");
 }
 for(float u=6.5f;u<12;u+=.25f)for(float v=-3;v<=3;v+=.25f){
  V p=towerBalconyPoint(u,v);
  if(game.walkable(p))check(std::abs(v)<=2.121f&&u<=11.121f&&u+std::abs(v)<=12.601f,"every exterior walkable sample stays inside the painted railing polygon");
 }
 game.player=towerBalconyPoint(9.3f);V saved=game.player;
 try{auto recovered=saves::decode(saves::encode({game,{}, {},"2026-10-27"}));check(distance(saved,recovered.game.player)<.0001f&&recovered.game.towerFloor==TowerBalconyFloor&&recovered.game.walkable(recovered.game.player),"save format 7 restores a valid balcony position without migration");}
 catch(const std::exception& e){check(false,std::string("balcony save roundtrip: ")+e.what());}
 game.bag[10]=item(Kind::Health);check(game.drop({Place::Bag,10},game.player),"items can be dropped onto the real terrace");int loot=game.ground.back().id;
 check(game.get({Place::Ground,loot})&&game.transfer({Place::Ground,loot},{Place::Bag,10}),"terrace loot is recovered through normal inventory rules");
 game.player=towerBalconyPoint(10);
 for(int frame=0;frame<185;frame++)game.update(1.f/60,outward*-1,false,true);
 check(towerBalconyLocal(game.player).x<5.8f&&game.walkable(game.player),"the hero can return from the balcony to the observatory without a teleport");
 game.player=TowerStairDown+V{-1,0};check(game.climbTower(-1)&&game.towerFloor==3&&game.walkable(game.player),"normal downstairs remains usable after a terrace visit");
 int images=0;
 for(float factor:{.8f,1.f,1.2f}){
  prepareCanvas(factor);
  for(int scene=0;scene<4;scene++){
   reset();game.player=scene==0?TowerCenter:towerBalconyPoint(scene==1?6.7f:9.7f,scene==3?1.9f:0.f);game.time=2;
   auto fonts=typography.atlasCount();renderScene();check(fonts==typography.atlasCount(),"balcony rendering uses already prepared font atlases");
   auto name="balcony-"+std::to_string(scene)+"-"+std::to_string(int(factor*100));saveTexture(canvas,(root/(name+".png")).string());images++;
   Vector2 point=screen(game.player);check(std::abs(point.x-525)<=.5f/renderScale+.01f&&std::abs(point.y-428)<=.5f/renderScale+.01f,"real camera keeps the hero centered with physical-pixel quantization");
   check(game.walkable(game.player),"all captured room, threshold, lookout and front-rail positions are valid");
  }
 }
 prepareCanvas(1);reset();game.player=towerBalconyPoint(9.7f,1.9f);renderScene();Image full=LoadImageFromTexture(canvas.texture);auto foreground=tower_balcony::front;tower_balcony::front={};renderScene();Image without=LoadImageFromTexture(canvas.texture);tower_balcony::front=foreground;
 Color* a=LoadImageColors(full);Color* b=LoadImageColors(without);int different=0;for(int i=0;i<full.width*full.height;i++)different+=std::abs(a[i].r-b[i].r)+std::abs(a[i].g-b[i].g)+std::abs(a[i].b-b[i].b)>30;
 check(different>2000,"foreground stone rail and ivy are actually composited over the world");UnloadImageColors(a);UnloadImageColors(b);UnloadImage(full);UnloadImage(without);
 check(runTowerFloorsQA(),"existing five-floor interactions, stairs, crafting, telescope and loot isolation remain functional");
 prepareCanvas(1);std::ofstream timing(root/"render-timing.json");timing<<"{\"audio_opened\":false,\"presentation_measured\":false,\"scenes\":[";
 for(int scene=0;scene<2;scene++){
  reset();if(scene)game.player=towerBalconyPoint(9.7f);std::vector<double> samples;
  for(int frame=0;frame<150;frame++){double begin=GetTime();renderScene();rlDrawRenderBatchActive();glFinish();if(frame>=30)samples.push_back((GetTime()-begin)*1000);}
  std::sort(samples.begin(),samples.end());double p95=samples[size_t(samples.size()*.95)];if(scene)timing<<',';
  timing<<"{\"name\":\""<<(scene?"valley-lookout":"observatory")<<"\",\"frames\":120,\"p95_ms\":"<<p95<<",\"mean_ms\":"<<std::accumulate(samples.begin(),samples.end(),0.)/samples.size()<<",\"within_60hz_render_budget\":"<<(p95<1000./60?"true":"false")<<'}';
 }
 timing<<"]}";check(!audioSystem.available&&audioSystem.playedEffects==0,"all game diagnostics stayed silent");
 log<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';std::ofstream report(root/"summary.json");report<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"screenshots\":"<<images<<",\"audio_opened\":false}";
 std::cout<<"Balcony: "<<checks<<" checks, "<<failures<<" failures\n";CloseWindow();return failures?2:0;
}
