// Real player input, new arrival, actual boss simulation and native renderer.
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#include <iostream>
#undef main
#include "fountains_trade_qa.hpp"
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root/"frames");
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_RESIZABLE);sync_present::useOpenGL();InitWindow(W,H,"Emberfall Guardian --no-audio");if(!IsWindowReady())return 5;SetExitKey(KEY_NULL);
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 std::ofstream log(root/"checks.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;log<<(ok?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 auto reset=[&]{game=Game{};intro={};towerStory={};clearTransientUi();welcome=help=paused=false;mouse={0,0};};
 auto save=[&](const fs::path& path){renderScene();Image image=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&image);check(ExportImage(image,path.string().c_str()),"render written: "+path.filename().string());UnloadImage(image);};
 check(PlayerBuild&&!audioSystem.available&&guardian_art::stone.id,"player code preloads native Krita stone without audio");
 check(guardian_art::fragments.id&&guardian_art::fragments.width==288&&guardian_art::fragments.height==192,"six native Krita cutouts are preloaded once in the player renderer");
 // Full equipped hero, not a rotated flattened screenshot: all facing views,
 // representative fall/get-up poses and every wearable item are rendered.
 fs::create_directories(root/"knockdown/angles");fs::create_directories(root/"knockdown/frames");fs::create_directories(root/"knockdown/equipment");
 reset();game.animation={};game.animation.motionInitialized=game.animation.facingInitialized=true;
 RenderTexture2D sample=LoadRenderTexture(640,520);
 auto fallFrame=[&](int direction,int frame,const fs::path& path){
  game.animation.direction=Direction(direction);game.animation.facingAngle=game.animation.targetAngle=direction*PI/4;
  game.heroKnockdownFacing=direction*PI/4;game.heroKnockdownRemaining=HeroKnockdownDuration*(1-frame/float(hero_motion::knockdown_synfig::Frames));
  float angle=direction*PI/4;game.heroKnockback=normalized(V{-std::cos(angle)/80-std::sin(angle)/40,std::cos(angle)/80-std::sin(angle)/40});
  actor_art::finishShadowFrame();BeginTextureMode(sample);ClearBackground(BLANK);hero(320,350,2.6f,false);EndTextureMode();
  Image image=LoadImageFromTexture(sample.texture);ImageFlipVertical(&image);Color* pixels=LoadImageColors(image);int solid=0,border=0;
  for(int y=0;y<image.height;y++)for(int x=0;x<image.width;x++)if(pixels[y*image.width+x].a>32){solid++;if(x<2||x>=image.width-2||y<2||y>=image.height-2)border++;}
  check(solid>1200&&border==0,"equipped falling/getting-up hero remains complete inside the frame");
  check(ExportImage(image,path.string().c_str()),"knockdown render written: "+path.filename().string());UnloadImageColors(pixels);UnloadImage(image);
 };
 const auto defaultGear=game.equipment;
 for(int direction=0;direction<8;direction++)for(int frame:{0,14,27,42,56,64,75,96,111})fallFrame(direction,frame,root/"knockdown/angles"/(std::to_string(direction)+"-"+std::to_string(frame)+".png"));
 for(int frame=0;frame<=112;frame+=2)fallFrame(1,std::min(frame,111),root/"knockdown/frames"/(std::to_string(std::min(frame,111))+".png"));
 int outfits=0;for(int k=1;k<int(Kind::Count);k++)if(gearSlot(Kind(k))>=0){game.equipment=defaultGear;game.equipment[gearSlot(Kind(k))]=item(Kind(k));fallFrame(1,42,root/"knockdown/equipment"/(std::to_string(k)+".png"));outfits++;}
 check(outfits==EquipmentKindCount,"all equipment items stay visible and supported by the same fall renderer");UnloadRenderTexture(sample);actor_art::finishShadowFrame();
 reset();check(distance(game.player,ForestArrival)==0&&game.walkable(game.player),"actual New Game factory uses western arrival");save(root/"forest-start.png");
 welcome=true;menuNewGame();check(intro.active()&&distance(game.player,ForestArrival)==0,"production New Game menu action starts the prologue at the new spawn");
 intro.phase=IntroPhase::Arriving;intro.arrivalPending=true;game.player={8,19};applyIntroArrival();
 check(distance(game.player,ForestArrival)==0&&!intro.arrivalPending,"actual prologue handoff teleports to the new western arrival");intro={};
 V start=game.player;for(int frame=0;frame<600;frame++)game.update(1.f/60,{},false);
 check(game.hp==100&&distance(game.player,start)==0&&std::none_of(game.enemies.begin(),game.enemies.end(),[&](const Enemy& e){return game.canSeePlayer(e);}),"new start stays safe through ten seconds of real patrols");
 // Every frame is a real tick: prepare, release, flight, impact, recovery.
 reset();game.player=ForestGuardian+V{4,0};game.props.clear();game.forage.clear();game.enemies={};game.enemies.push_back({13,3,ForestGuardian,ForestGuardian,260,260});game.enemies[0].attack=1000;game.enemies[0].boulderCd=0;game.messageTime=0;
 int airborne=0,telegraphed=0,throws=0,recoilFrame=-1,recoilTicks=0;float hp=game.hp;V locked=game.player;bool recoilPauseChecked=false;
 fs::create_directories(root/"backfall-frames");
 for(int frame=0;frame<288;frame++){
  game.update(1.f/60,{},false);auto& boss=game.enemies[0];
  if(boss.boulderWindup>0){telegraphed++;check(distance(boss.boulderTarget,locked)<.0001f,"telegraph target stays committed");}
  if(!game.boulders.empty()){
   airborne++;auto& rock=game.boulders[0];check(rock.height>=0&&distance(rock.target,locked)<.0001f,"actual elevated stone flies to the marker");
   auto order=worldDrawOrder();check(std::count_if(order.begin(),order.end(),[](const DrawObject& o){return o.type==10;})==1,"stone participates once in world depth sorting");
   if(airborne==1){auto hand=Game::boulderHand(boss,BoulderWindup/(BoulderWindup+BoulderRecovery));check(distance(hand.ground,rock.origin)<.001f&&std::abs(hand.height-rock.startHeight)<.1f,"projectile leaves the actual animated hand socket");}
  }
  if(game.knockedBack()){
   recoilTicks++;if(recoilFrame<0)recoilFrame=frame;
   if(!recoilPauseChecked){
    V at=game.player;float remaining=game.heroKnockbackRemaining,down=game.heroKnockdownRemaining;openPauseMenu();for(int i=0;i<5;i++)input(.05f);
    check(distance(at,game.player)==0&&game.heroKnockbackRemaining==remaining&&game.heroKnockdownRemaining==down,"actual pause menu freezes the hero mid-fall");paused=false;menuPageTo(MenuPage::Home);
    saves::Snapshot snapshot;snapshot.game=game;saves::Store store(fs::absolute(root/"knockback-saves"));store.save(0,snapshot);auto loaded=store.load(0);
    saves::Archive original(false),restored(false);original(snapshot.game);restored(loaded.game);
    check(original.bytes==restored.bytes&&loaded.game.knockedBack(),"real on-disk save restores the exact mid-knockback game state");recoilPauseChecked=true;
   }
  }
  if(recoilFrame>=0&&frame-recoilFrame==8)save(root/"guardian-knockback.png");
  if(recoilFrame>=0){int age=frame-recoilFrame;if(age==42)save(root/"guardian-knockdown-ground.png");if(age==64)save(root/"guardian-knockdown-brace.png");if(age==90)save(root/"guardian-knockdown-rise.png");if(age==113)save(root/"guardian-knockdown-recovered.png");if(age<=124&&age%2==0)save(root/"backfall-frames"/(std::to_string(age)+".png"));}
  if(frame<240&&frame%2==0)save(root/"frames"/(std::to_string(frame/2)+".png"));
  if(frame==16)save(root/"guardian-uproot.png");if(frame==53)save(root/"guardian-lift.png");if(frame==74)save(root/"guardian-windup.png");if(frame==94)save(root/"guardian-flight.png");if(frame==119)save(root/"guardian-impact.png");
  throws=boss.boulderThrows;
 }
 check(telegraphed>=89&&airborne>10&&throws==1,"one complete crouch, tear, overhead lift and visible flight really occurred");
 check(std::abs(game.hp-(hp-game.incomingDamage(BoulderDamage,DamageType::Physical)))<.001f,"actual encounter inflicts exactly one armor-reduced stone hit");
 check(recoilPauseChecked&&recoilTicks>=110&&recoilTicks<=112&&!game.knockedBack(),"one real ground hit knocks the hero down, keeps control locked and returns it after getting up");
 check(distance(game.player,locked)>1.3f&&game.player.x>locked.x&&std::abs(game.player.y-locked.y)<.3f,"hero visibly travels away from the actual falling rock");
 std::ofstream(root/"knockback-report.json")<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"onset_frame\":"<<recoilFrame<<",\"active_frames\":"<<recoilTicks<<",\"distance\":"<<distance(game.player,locked)<<",\"audio_played\":false}";
 // A real committed throw and real roll leave the impact unobstructed for the
 // full fracture clip. Damage still belongs only to the original projectile.
 reset();game.player=ForestGuardian+V{4,0};game.props.clear();game.forage.clear();game.enemies={{13,3,ForestGuardian,ForestGuardian,260,260}};game.enemies[0].attack=1000;game.enemies[0].boulderCd=0;
 for(int i=0;i<180&&game.boulders.empty();i++)game.update(1.f/60,{},false);
 check(game.boulders.size()==1&&game.boulderImpacts.empty(),"no fracture appears before the thrown boulder lands");game.startRoll({1,-1});fs::create_directories(root/"shatter-frames");
 int impactFrame=-1,fractureTicks=0;bool pauseChecked=false;
 for(int frame=0;frame<120;frame++){
  game.update(1.f/60,{},false);auto order=worldDrawOrder();int pieces=std::count_if(order.begin(),order.end(),[](const DrawObject& o){return o.type==11;});
  if(!game.boulderImpacts.empty()){
   if(impactFrame<0)impactFrame=frame;fractureTicks++;check(game.boulders.empty()&&(pieces==guardian_break::Pieces||game.boulderImpacts[0].progress()>.999f),"six separately depth-sorted pieces replace the intact stone at impact");
   if(!pauseChecked&&game.boulderImpacts[0].progress()>.12f){float phase=game.boulderImpacts[0].life;openPauseMenu();for(int i=0;i<5;i++)input(.05f);check(game.boulderImpacts[0].life==phase,"production pause freezes the fracture animation");paused=false;menuPageTo(MenuPage::Home);pauseChecked=true;}
   int age=frame-impactFrame;if(age==0)save(root/"guardian-break-contact.png");if(age==12)save(root/"guardian-break-burst.png");if(age==34)save(root/"guardian-break-bounce.png");if(age==62)save(root/"guardian-break-settled.png");
  }else check(pieces==0,"no fragments are drawn before impact or after expiry");
  if(frame%2==0)save(root/"shatter-frames"/(std::to_string(frame/2)+".png"));
 }
 check(impactFrame>=0&&fractureTicks>=71&&fractureTicks<=73&&game.boulderImpacts.empty(),"actual ground fracture persists through its full 72-frame Synfig clip and expires");
 check(game.hp==100&&pauseChecked,"rolling away avoids the hit and cosmetic fragments cause no extra damage");
 game.player=ForestGuardian+V{4,0};game.enemies[0].pos=ForestGuardian;game.enemies[0].animation.moveBlend=0;game.enemies[0].animation.moving=false;
 fs::create_directories(root/"angles");game.boulders.clear();game.boulderImpacts.clear();
 for(int direction=0;direction<8;direction++)for(int poseFrame:{16,29,53,74,89,98}){
  auto& boss=game.enemies[0];boss.hp=220;boss.animation.direction=Direction(direction);boss.animation.facingAngle=boss.animation.targetAngle=direction*PI/4;boss.animation.facingInitialized=true;
  boss.boulderWindup=std::max(0.f,BoulderWindup-poseFrame/60.f);boss.boulderRecovery=poseFrame<guardian_motion::ReleaseFrame?0.f:BoulderRecovery-(poseFrame/60.f-BoulderWindup);
  auto pose=guardian_motion::sample(boss.boulderProgress());Vector2 origin=screen(boss.pos);actor_art::Rig rig(origin.x,origin.y,GuardianBodyScale,boss.animation.direction);rig.setFacing(boss.animation.facingAngle);
  float bob=std::sin(boss.animation.idleTime*2.1f)*.125f;
  for(float side:{-1.f,1.f}){
   auto p=rig.project({side*pose.handSide,pose.handForward,pose.handUp+bob});auto socket=Game::guardianSocket(boss,side*pose.handSide,pose.handForward,pose.handUp);auto expected=screen(socket.ground);expected.y-=socket.height;
   check(std::hypot(p.x-expected.x,p.y-expected.y)<1.5f,"both physical grip projections match shared sockets in every facing");
  }
  save(root/"angles"/(std::to_string(direction)+"-"+std::to_string(poseFrame)+".png"));
 }
 // Pausing through the production menu must freeze both marker and flight.
 reset();game.player=ForestGuardian+V{4,0};game.props.clear();game.enemies={};game.enemies.push_back({13,3,ForestGuardian,ForestGuardian,260,260});game.enemies[0].attack=1000;game.enemies[0].boulderCd=0;game.update(.05f,{},false);
 openPauseMenu();float time=game.time,windup=game.enemies[0].boulderWindup;
 for(int i=0;i<10;i++)input(.05f);
 check(paused&&game.time==time&&game.enemies[0].boulderWindup==windup,"Escape pause freezes the pending boss ability through real input routing");paused=false;menuPageTo(MenuPage::Home);
 // Save/load with an isolated store, without touching the player's real slots.
 menuSaveRoot=fs::absolute(root/"saves");saves::Snapshot snapshot;snapshot.game=game;saves::Store(menuSaveRoot).save(0,snapshot);game=Game{};menuSlot=0;menuLoad();
 check(game.enemies[0].boulderWindup==windup&&distance(game.player,ForestGuardian+V{4,0})<.001f,"menu load restores a saved in-progress windup and exact position");
 fountainsTradeQA(root,check,save);
 check(!audioSystem.available,"all diagnostics and UI actions stayed silent");log<<"Checks: "<<checks<<"\nFailures: "<<failures<<'\n';std::cout<<"Guardian QA: "<<checks<<" checks, "<<failures<<" failures\n";
 guardian_art::unload();CloseWindow();return failures?2:0;
}
