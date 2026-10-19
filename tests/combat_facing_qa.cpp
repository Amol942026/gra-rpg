// Exercise real player input, picking and rendered combat facing, always silent.
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main

int main(int argc,char** argv){
 if(argc!=2||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::create_directories("qa-combat-facing");
 std::ofstream report("qa-combat-facing/ui-tests.txt");int checks=0,failures=0;
 auto check=[&](bool result,const char* label){checks++;failures+=!result;report<<(result?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);
 sync_present::useOpenGL();InitWindow(W,H,"Emberfall combat facing QA --no-audio");
 if(!IsWindowReady())return 4;audioSystem.init(false,false);
 makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();
 makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 check(PlayerBuild&&!audioSystem.available,"real player code uses no audio device");
 auto reset=[](){
  game=Game{};game.props.clear();game.enemies.clear();game.forage.clear();game.player={20,20};game.messageTime=0;
  intro={};towerStory={};clearStoryMenus();welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=false;
  developerActive=developerMenuOpen=false;dragging={};mouse={0,0};
 };
 auto aimError=[](){return std::abs(std::remainder(game.animation.targetAngle-game.animation.facingAngle,2*AnimationPi));};
 auto pointAtEnemy=[](int id){
  Enemy* enemy=game.findEnemy(id);if(!enemy)return false;Vector2 base=screen(enemy->pos);
  for(int dy=-100;dy<10;dy+=3)for(int dx=-65;dx<65;dx+=3){
   Vector2 p={base.x+dx,base.y+dy};WorldHover hit=pickWorld(p);
   if(worldPointerAllowed(p)&&hit.kind==HoverKind::Enemy&&hit.index==id){mouse=p;return true;}
  }return false;
 };
 const V aims[]={{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1}};
 for(V aim:aims){
  reset();V target=game.player+normalized(aim)*2;game.enemies.push_back({42,1,target,target,500,500});
  check(pointAtEnemy(42),"enemy silhouette is selectable in every direction");
  check(distance(normalized(combatAimFromPointer()),normalized(aim))<.0001f,"cursor on enemy aims at its world position, independent of body height");
  leftClickGameplay(aim*-1);
  check(game.swing>0&&distance(game.facing,normalized(aim))<.0001f&&game.enemies[0].hp<500,"real sword click attacks and faces the selected enemy");
  game.enemies.clear();V before=game.player;
  for(int i=0;i<15;i++)game.update(1.f/60,aim*-1,false,true);
  check(game.animation.direction==directionFor(aim.x,aim.y,Direction::SE)&&aimError()<.02f&&distance(before,game.player)>1,"rendered sword facing survives opposite sprint input");
  reset();game.skillRanks[int(Skill::StrongCut)]=1;target=game.player+normalized(aim)*2;game.enemies.push_back({42,1,target,target,500,500});
  check(pointAtEnemy(42)&&activateGameplaySkill(Skill::StrongCut)&&distance(game.facing,normalized(aim))<.0001f,"strong-cut input also faces the selected target");
 }
 reset();Rectangle bag=bagRect(0);mouse={bag.x+20,bag.y+20};leftClickGameplay({1,0});check(game.swing==0,"HUD click does not become a directed sword attack");
 reset();game.selectWeapon(1);
 auto pointAtGround=[](V aim){Vector2 p=screen(game.player+normalized(aim)*4);mouse={p.x,p.y-35};};
 pointAtGround({-1,1});leftClickGameplay({1,-1});check(game.bowDrawing,"real world press starts an aimed bow draw");
 for(int frame=0;frame<36;frame++){
  V aim=frame<18?V{-1,1}:V{1,1};pointAtGround(aim);game.aimBow(combatAimFromPointer());game.update(1.f/60,{1,-1},false);
  if(frame==17||frame==35){
   check(game.animation.direction==directionFor(aim.x,aim.y,Direction::SE)&&aimError()<.01f,"moving bow visually tracks the current pointer instead of the movement vector");
   renderScene();saveTexture(canvas,"qa-combat-facing/bow-moving-"+std::to_string(frame)+".png");
  }
 }
 pointAtGround({1,1});V shotAim=normalized(combatAimFromPointer());
 check(releaseWorldBow()&&game.arrows.size()==1&&distance(normalized(game.arrows[0].velocity),shotAim)<.0001f,"actual mouse release fires along the visible aim");
 game.update(1.f/60,{1,-1},false);renderScene();saveTexture(canvas,"qa-combat-facing/bow-release.png");
 check(game.bowReleaseTime>0&&distance(game.facing,shotAim)<.0001f&&aimError()<.01f,"shot recovery retains target facing while moving");
 // A compact native-rendered sheet shows the same moving combat states in all directions.
 RenderTexture2D sheet=LoadRenderTexture(1920,630);
 BeginTextureMode(sheet);ClearBackground(Ink);EndTextureMode();
 for(int row=0;row<3;row++)for(int d=0;d<8;d++){
  reset();V aim=aims[d];game.facing=normalized(aim*-1);game.animation.face(-aim.x,-aim.y);
  if(row==0)game.attack(aim);else{game.selectWeapon(1);game.beginBow(aim);}
  for(int i=0;i<15;i++)game.update(1.f/60,aim*-1,false);
  if(row==2){game.releaseBow();game.update(1.f/60,aim*-1,false);}
  actor_art::finishShadowFrame();
  BeginTextureMode(sheet);float x=d*240+120.f,y=row*210+185.f;
  hero(x,y,1.65f,false);center(TextFormat("%s / %d",row==0?"MIECZ":row==1?"NACIAG":"STRZAL",d),x,y+8,14,Gold);EndTextureMode();
 }
 saveTexture(sheet,"qa-combat-facing/moving-combat-directions.png");UnloadRenderTexture(sheet);
 world_shadows::unload();actor_art::finishShadowFrame();fallen_art::unload();unloadHoverArt();
 unloadSkillArt();unloadTowerRoomArt();tower_stairs::unload();tower_furniture::unload();desert_art::unload();sand_prison::unload();
 for(auto& texture:propArt)UnloadRenderTexture(texture);
 UnloadRenderTexture(terrain);UnloadTexture(minimapTerrain);if(canvas.id)UnloadRenderTexture(canvas);if(worldCanvas.id)UnloadRenderTexture(worldCanvas);
 typography.clear();audioSystem.shutdown();CloseWindow();
 report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';std::printf("Combat facing UI: %d/%d checks passed\n",checks-failures,checks);
 return failures?2:0;
}
