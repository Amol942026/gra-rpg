#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include <iostream>
extern "C" void __stdcall glFinish();
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall stair fit --no-audio");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 int checks=0,failures=0,cases=0,frames=0,images=0;std::ofstream log(root/"checks.txt"),points(root/"footprints.tsv");points<<"floor\tdirection\tx\ty\n";
 auto check=[&](bool pass,const std::string& name){checks++;failures+=!pass;log<<(pass?"PASS ":"FAIL ")<<name<<'\n';log.flush();};
 auto reset=[](int floor){game=Game{};intro={};towerStory={};clearTransientUi();welcome=help=paused=false;game.enterMageTower();game.dawnQuest=DawnQuest::AtTower;game.towerFloor=floor;game.effects.clear();game.messageTime=0;mouse={0,0};};
 auto shot=[&](const std::string& name){auto fonts=typography.atlasCount();renderScene();check(fonts==typography.atlasCount(),"stairs use preloaded text atlases");saveTexture(canvas,(root/(name+".png")).string());images++;};
 check(PlayerBuild&&!audioSystem.available,"real player renderer opens no audio device");
 double maxFootprint=0,maxSilhouette=0;
 for(int dir:{1,-1}){
  int index=tower_stairs::index(dir);V base=towerStairBase(dir);auto bounds=towerStairFootprint(dir);
  for(V point:tower_stairs::footprints[index]){
   check(point.x>=bounds.left-.001f&&point.x<=bounds.right+.001f&&std::abs(point.y)<=bounds.halfWidth+.001f,"all actual masonry vertices fit the physical stair bounds");
  }
  Image im=LoadImageFromTexture(tower_stairs::art[index].texture);ImageFlipVertical(&im);ExportImage(im,(root/(dir==1?"upstairs.png":"downstairs.png")).string().c_str());Color* px=LoadImageColors(im);int outside=0,solid=0;
  for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++)if(px[y*im.width+x].a>=210){
   float dx=(base.x-base.y)*40+(x+.5f)/tower_stairs::ArtScale-tower_stairs::OriginX;
   float dy=(base.x+base.y-24)*20+(y+.5f)/tower_stairs::ArtScale-tower_stairs::OriginY;
   double edge=std::pow(dx/(TowerRadius*56.56854249),2)+std::pow(dy/(TowerRadius*28.28427125),2);maxSilhouette=std::max(maxSilhouette,edge);outside+=edge>1;solid++;
  }
  check(solid>20000&&outside==0,"entire rendered staircase silhouette fits inside the floor ellipse");UnloadImageColors(px);UnloadImage(im);
 }
 for(int floor=0;floor<TowerFloorCount;floor++){
  reset(floor);check(game.walkable(TowerArrival),"original room arrival stays clear on every floor");
  for(int dir:{1,-1})if(dir==1?floor<TowerFloorCount-1:floor>0){
   V base=towerStairBase(dir);auto bounds=towerStairFootprint(dir);check(game.walkable(towerStairApproach(dir)),"each flight has a valid accessible threshold");
   for(V local:tower_stairs::footprints[tower_stairs::index(dir)]){
    V p=base+local;double radius=distance(p,TowerCenter);maxFootprint=std::max(maxFootprint,radius);points<<floor<<'\t'<<dir<<'\t'<<p.x<<'\t'<<p.y<<'\n';
    check(radius+TowerStairPlayerRadius<TowerRadius,"every stone vertex and player clearance remain inside the map outline");
    check(towerStairBlocked(p,dir)&&!game.walkable(p),"steps, post caps, railings and the shaft all have physical collisions");
   }
   const float middle=(bounds.left+bounds.right)*.5f;
   std::array<V,4> starts={towerStairApproach(dir),base+V{dir==1?bounds.left-.9f:bounds.right+.9f,0},base+V{middle,-bounds.halfWidth-.9f},base+V{middle,bounds.halfWidth+.9f}};
   V center=base+V{middle,0};
   for(int side=0;side<4;side++){
    if(!game.walkable(starts[side]))continue;cases++;
    for(int mode=0;mode<3;mode++){
     reset(floor);game.player=starts[side];V input=normalized(center-game.player);
     if(mode==2)check(game.startRoll(input),"a roll can start at the stair perimeter");
     for(int frame=0;frame<90;frame++){
      game.update(1.f/30,input,false,mode==1);frames++;
      check(game.walkable(game.player)&&!towerStairBlocked(game.player,dir),"walk, sprint and roll cannot penetrate masonry from any side");
     }
     if(floor==1&&mode==1){game.messageTime=0;shot("contact-"+std::to_string(dir)+"-"+std::to_string(side));}
    }
   }
   reset(floor);game.player=base+V{dir==1?1.05f:-1.05f,0};V before=game.player;game.move({dir==1?-8.f:8.f,0});
   check(game.walkable(game.player)&&distance(before,game.player)<3,"a large movement delta cannot tunnel through an entire flight");
   reset(floor);game.player=towerStairApproach(dir);check(nearestTowerInteraction()&&game.towerFloor==floor+dir&&game.walkable(game.player),"E uses the stairs from a safe approach and lands clear of the opposite flight");
  }
 }
 check(cases>=28,"perimeter motion tests cover all five floors and both flight types");
 for(float factor:{.8f,1.f,1.2f}){
  prepareCanvas(factor);for(int floor=0;floor<TowerFloorCount;floor++){reset(floor);game.player=TowerArrival;shot("floor-"+std::to_string(floor+1)+"-"+std::to_string(int(factor*100)));}
 }
 prepareCanvas(1);menuSaveRoot=fs::absolute(root/"owned-saves");
 for(int floor=0;floor<TowerFloorCount;floor++){
  reset(floor);int dir=floor==TowerFloorCount-1?-1:1;auto b=towerStairFootprint(dir);game.player=towerStairBase(dir)+V{(b.left+b.right)*.5f,0};
  auto bag=game.bag;int level=game.level;menuSlot=0;menuStore().save(0,{game,{}, {},{}});V old=game.player;menuLoad();
  check(game.inTower()&&game.towerFloor==floor&&game.walkable(game.player)&&distance(old,game.player)<=2.51f,"real menu loading repairs an old position trapped by relocated stairs");
  check(game.level==level&&game.bag[0].kind==bag[0].kind&&game.bag[0].count==bag[0].count,"safe load preserves character progression and inventory");
 }
 reset(4);game.player=towerBalconyPoint(9.5f);V balcony=game.player;game.ensureTowerPosition();check(distance(balcony,game.player)<.0001f,"already valid balcony saves keep their exact location");menuSaveRoot.clear();
 check(runTowerStairsQA(),"existing stone geometry, mouse picking, E and PPM checks pass");
 check(runTowerFloorsQA(),"all five themed floors, furnishings, recipes, telescope and floor-local loot still work");
 prepareCanvas(1);std::ofstream timing(root/"render-timing.json");timing<<"{\"audio_opened\":false,\"presentation_measured\":false,\"scenes\":[";
 for(int floor=0;floor<TowerFloorCount;floor++){
  reset(floor);game.player=TowerArrival;std::vector<double> times;
  for(int frame=0;frame<150;frame++){double start=GetTime();renderScene();rlDrawRenderBatchActive();glFinish();if(frame>=30)times.push_back((GetTime()-start)*1000);}
  std::sort(times.begin(),times.end());if(floor)timing<<',';timing<<"{\"floor\":"<<floor+1<<",\"frames\":120,\"mean_ms\":"<<std::accumulate(times.begin(),times.end(),0.)/times.size()<<",\"p95_ms\":"<<times[size_t(times.size()*.95)]<<'}';
 }
 timing<<"]}";check(!audioSystem.available&&audioSystem.playedEffects==0,"all tests remained silent");
 std::ofstream report(root/"summary.json");report<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"motion_cases\":"<<cases<<",\"motion_frames\":"<<frames<<",\"images\":"<<images<<",\"max_ground_radius\":"<<maxFootprint<<",\"max_silhouette_ellipse\":"<<maxSilhouette<<",\"audio_opened\":false}";
 std::cout<<"Stair fit: "<<checks<<" checks, "<<failures<<" failures\n";CloseWindow();return failures?2:0;
}
