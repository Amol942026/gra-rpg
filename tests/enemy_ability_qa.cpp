#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include "enemy_ability_fixture.hpp"
#include <iostream>
int main(int argc,char** argv){
 if(argc!=4||std::string(argv[1])!="--no-audio"||std::string(argv[3])!="--background-test")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_ALWAYS_RUN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall abilities --no-audio");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1.2f);
 int checks=0,failures=0,captures=0;std::ofstream log(root/"checks.txt");
 auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;log<<(ok?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 auto resetUi=[](){intro={};towerStory={};clearTransientUi();welcome=help=paused=false;mouse={0,0};game.messageTime=0;};
 auto capture=[&](const std::string& name){
  renderScene();Image img=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&img);
  check(img.data&&img.width==int(W*renderScale)&&img.height==int(H*renderScale),"actual scene decoded / "+name);
  check(ExportImage(img,(root/(name+".png")).string().c_str()),"saved gameplay frame / "+name);UnloadImage(img);captures++;
  check(IsWindowHidden(),"test remains hidden / "+name);
 };
 const int capacity=actor_art::creatureCapacity;const unsigned allocations=shadow_geometry::allocations;
 for(int type:{9,10,16,17})for(int side=0;side<8;side++){
  game=enemyAbilityArena(type);resetUi();auto& enemy=game.enemies[0];const auto spec=enemyAbilityProfile(type);
  V dir{std::cos(side*PI/4),std::sin(side*PI/4)};game.player=enemy.pos+dir*3;
  std::array<bool,3> seen{};std::string prefix=std::to_string(type)+"-"+std::to_string(side);
  for(int frame=0;frame<190;frame++){
   game.update(1.f/60,{},false);
   int phase=!enemy.abilityBusy()?-1:enemy.abilityAge<spec.windup*.65f?-1:enemy.abilityAge<spec.windup?0:enemy.abilityAge<spec.windup+spec.active*.4f?-1:enemy.abilityAge<spec.windup+spec.active?1:2;
   if(phase>=0&&!seen[phase]){seen[phase]=true;static const char* labels[]={"warning","attack","recovery"};capture(prefix+"-"+labels[phase]);}
   audioSystem.update(1.f/60,game,false,false);
  }
  for(bool value:seen)check(value,"all three ability stages were rendered / "+prefix);
  check(enemy.abilityCasts==1,"rendered ability respects cooldown / "+prefix);
  check(game.hp<game.maxHp()&&game.hp>0,"stationary target receives finite damage / "+prefix);
  if(side==0){
   renderScene();Vector2 at=screen(enemy.pos);mouse={at.x,at.y-22};hovered=pickWorld(mouse);
   check(hovered.kind==HoverKind::Enemy&&hovered.index==enemy.id,"real body picking / "+prefix);
   auto info=inspectWorld(hovered);check(!info.secondary.empty()&&textWidth(info.secondary,14)<=301,"ability counter tip fits inspection HUD / "+prefix);capture(prefix+"-inspection");
  }
 }
 // Released venom must not become a false hero target in reverse draw picking.
 game=enemyAbilityArena(10,5);resetUi();for(int i=0;i<49;i++)game.update(1.f/60,{},false);renderScene();
 check(game.venomGlobs.size()==3,"actual rendered cobra releases three projectiles");
 if(!game.venomGlobs.empty()){mouse=screen(game.venomGlobs[1].pos);mouse.y-=35;hovered=pickWorld(mouse);auto shots=game.venomGlobs;game.venomGlobs.clear();auto baseline=pickWorld(mouse);game.venomGlobs=shots;check(hovered.kind==baseline.kind&&hovered.index==baseline.index,"projectiles do not change hover targets");capture("venom-flight");}
 // Preserve each realm's authored scenery and test an existing spawn's attack.
 for(int type:{9,10,16,17}){
  game=Game{};game.activateField(type<12?WorldLocation::Desert:WorldLocation::Volcano);resetUi();int id=-1;
  for(auto& enemy:game.enemies)if(enemy.type==type){
   for(int side=0;side<48;side++){V p=enemy.pos+V{std::cos(side*PI/24),std::sin(side*PI/24)}*3;
    if(game.enemyPositionClear(p,HeroBodyRadius)&&game.clearEnemySegment(enemy.pos,p,enemy.radius()+.01f)){game.player=p;enemy.abilityCd=0;id=enemy.id;break;}}
   if(id>=0)break;
  }
  check(id>=0,"authored realm has a reachable encounter / "+std::to_string(type));
  if(auto* enemy=game.findEnemy(id)){for(int frame=0;frame<100&&enemy->abilityCasts==0;frame++)game.update(1.f/60,{},false);
   check(enemy->abilityCasts>0,"existing spawn starts its new ability / "+std::to_string(type));
   for(int frame=0;frame<24;frame++)game.update(1.f/60,{},false);capture(std::to_string(type)+"-scenery");}
 }
 check(actor_art::creatureCapacity==capacity,"abilities reuse preloaded actor buffer");
 check(shadow_geometry::allocations==allocations,"abilities do not grow shadow storage");
 check(PlayerBuild&&!developerActive,"renderer uses player edition without developer mode");
 check(!audioSystem.available&&audioSystem.playedEffects==0,"no audio device opened or effects played");
 std::ofstream out(root/"summary.json");out<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"abilities\":4,\"directions\":8,\"captured_frames\":"<<captures<<",\"authored_encounters\":4,\"hidden\":true,\"audio_opened\":false,\"presentation_measured\":false}";
 std::cout<<"Elite ability integration: "<<checks<<" checks, "<<failures<<" failures\n";
 audioSystem.shutdown();CloseWindow();return failures?2:0;
}
