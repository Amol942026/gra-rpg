// Actual player simulation and renderer over a fixed ground grid.
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include <iostream>
#ifdef EMBERFALL_EXPANDED_LOOT_QA
#include "expanded_loot_qa.hpp"
#endif
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall guardian gait --no-audio");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 int checks=0,failures=0;std::ofstream log(root/"checks.txt"),tracks(root/"feet.csv");
 tracks<<"sequence,frame,side,phase,foot_x,foot_y,body_x,body_y,hip_z,foot_forward,foot_z,knee_forward,knee_z\n";
 auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;log<<(ok?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 auto reset=[](){game=Game{};intro={};towerStory={};clearTransientUi();welcome=help=paused=false;mouse={0,0};game.props.clear();game.enemies.clear();game.forage.clear();game.messageTime=0;};
 check(PlayerBuild&&!audioSystem.available,"actual player renderer opens no audio device");
 RenderTexture2D target=LoadRenderTexture(900,600);
 auto save=[&](const fs::path& path){Image im=LoadImageFromTexture(target.texture);ImageFlipVertical(&im);Color* px=LoadImageColors(im);int solid=0,border=0;
  for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++)if(px[y*im.width+x].a>32){solid++;border+=x<2||x>=im.width-2||y<2||y>=im.height-2;}
  check(solid>1000&&border==0,"complete guardian silhouette: "+path.filename().string());check(ExportImage(im,path.string().c_str()),"native frame exported");UnloadImageColors(px);UnloadImage(im);
 };
 // View all step phases, including both support/swing transitions, at 8 angles.
 fs::create_directories(root/"poses");
 AnimationState pose;pose.motionInitialized=pose.facingInitialized=true;pose.moving=true;pose.moveBlend=1;
 for(int direction=0;direction<8;direction++)for(int frame=0;frame<24;frame++){
  pose.direction=Direction(direction);pose.facingAngle=pose.targetAngle=direction*PI/4;pose.phase=frame/24.f;
  actor_art::finishShadowFrame();BeginTextureMode(target);ClearBackground(BLANK);actor_art::draw(4,450,480,2.4f,pose);EndTextureMode();save(root/"poses"/(std::to_string(direction)+"-"+std::to_string(frame)+".png"));
 }
 // Straight chase, actual patrol and smooth stop/start, all from real ticks.
 int contacts=0;float maxSlip=0;V begin{};constexpr float zoom=2.2f;
 for(int sequence=0;sequence<3;sequence++){
  reset();game.player={2,2};game.enemies.push_back({91,3,{20,20},{20,20},260,260});auto& e=game.enemies[0];e.boulderCd=1000;e.attack=1000;
  e.patrolGoalActive=true;e.patrolGoal={22.4f,17.6f};e.behavior=EnemyBehavior::Patrol;
  std::string name=sequence==0?"chase":sequence==1?"patrol":"stop-start";fs::create_directories(root/name);
  // Warm the gait blending before measuring foot contacts; captures include
  // transitions in the separate stop/start sequence below.
  for(int f=0;f<36&&sequence!=2;f++){if(sequence==0)game.player=e.pos+normalized(V{1,-1})*5;game.update(1.f/60,{},false);}
  begin=e.pos;std::array<Vector2,2> previous{};std::array<float,2> previousPhase{};
  V first=e.pos;float initialPhase=e.animation.phase;
  for(int frame=0;frame<120;frame++){
   if(sequence==0||sequence==2)game.player=e.pos+normalized(V{1,-1})*5;
   if(sequence==2)e.windup=frame>=40&&frame<80?1.f:0;
   float beforePhase=e.animation.phase;V beforePos=e.pos;game.update(1.f/60,{},false);
   if(sequence==2&&frame>=40&&frame<80)check(length(e.pos-beforePos)==0&&e.animation.phase==beforePhase&&!e.animation.moving,"actual melee pause cannot march or advance the walking clock");
   float x=220+((e.pos.x-e.pos.y)-(begin.x-begin.y))*40*zoom,y=460+((e.pos.x+e.pos.y)-(begin.x+begin.y))*20*zoom;
   actor_art::Rig rig(x,y,zoom*GuardianBodyScale,e.animation.direction);rig.setFacing(e.animation.facingAngle);
   for(int side=0;side<2;side++){
    float phase=std::fmod(e.animation.phase+side*.5f,1.f);auto k=creature_motion::sample(2,phase);float walk=actor_art::creatureWalk(e.animation);
    float hip=guardian_gait::hipHeight(e.animation.phase),forward=k.travel*guardian_gait::Stride,z=guardian_gait::footHeight(phase,hip);auto knee=guardian_gait::knee(forward,hip,z);
    Vector2 foot=rig.project({side?10.f:-10.f,forward*walk,4+(z-4)*walk});
    tracks<<name<<','<<frame<<','<<side<<','<<phase<<','<<foot.x<<','<<foot.y<<','<<x<<','<<y<<','<<hip<<','<<forward<<','<<z<<','<<knee.forward<<','<<knee.height<<'\n';
    if(sequence<2&&frame>0&&phase>=previousPhase[side]&&phase<guardian_gait::Support-.02f&&previousPhase[side]<guardian_gait::Support-.02f){
     float slip=std::hypot(foot.x-previous[side].x,foot.y-previous[side].y);maxSlip=std::max(maxSlip,slip);contacts++;check(slip<.015f,"real renderer support foot stays fixed over the static ground");
    }
    previous[side]=foot;previousPhase[side]=phase;
   }
   actor_art::finishShadowFrame();BeginTextureMode(target);ClearBackground(BLANK);actor_art::draw(4,x,y,zoom,e.animation,e.attackPose());EndTextureMode();save(root/name/(std::to_string(frame)+".png"));
  }
  check(distance(first,e.pos)>.75f&&e.animation.phase!=initialPhase,"real AI moves and animates: "+name);
 }
 check(contacts>200&&maxSlip<.015f,"synchronized planted feet validated from native projected rig points");
 reset();game.player={24,20};game.enemies.push_back({91,3,{20,20},{20,20},260,260});auto& boss=game.enemies[0];boss.boulderCd=0;boss.attack=1000;boss.animation.phase=.31f;
 int throwFrames=0;
 for(int frame=0;frame<130;frame++){
  V before=boss.pos;float phase=boss.animation.phase;game.update(1.f/60,{},false);
  if(boss.boulderWindup>0||boss.boulderRecovery>0){throwFrames++;check(distance(before,boss.pos)==0&&boss.animation.phase==phase&&!boss.animation.moving,"boulder animation keeps both feet at rest");}
 }
 check(throwFrames>80&&boss.boulderThrows>=1,"unchanged real two-handed boulder attack still releases successfully");
 reset();game.player=ForestGuardian+V{4,0};game.enemies.push_back({13,3,ForestGuardian,ForestGuardian,260,260});game.enemies.back().boulderCd=1000;game.enemies.back().attack=1000;
 for(int i=0;i<30;i++)game.update(1.f/60,{},false);renderScene();saveTexture(canvas,(root/"in-game.png").string());
 check(game.findEnemy(13)&&game.findEnemy(13)->alive,"world capture contains the real live guardian");
#ifdef EMBERFALL_EXPANDED_LOOT_QA
 expandedLootChecks(root,check);
#endif
 check(!audioSystem.available&&audioSystem.playedEffects==0,"all movement and throw diagnostics remained silent");
 log<<"Checks: "<<checks<<"\nFailures: "<<failures<<"\nContact samples: "<<contacts<<"\nMax slip px: "<<maxSlip<<'\n';
 std::ofstream summary(root/"summary.json");summary<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"contact_samples\":"<<contacts<<",\"max_slip_px\":"<<maxSlip<<",\"pose_frames\":192,\"movement_frames\":360,\"audio_opened\":false}";
 std::cout<<"Guardian gait: "<<checks<<" checks, "<<failures<<" failures, maximum support slip "<<maxSlip<<" px\n";CloseWindow();return failures?2:0;
}
