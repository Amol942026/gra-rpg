#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#include <iostream>
#undef main
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];fs::create_directories(root);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall loot all --no-audio");if(!IsWindowReady())return 4;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 int checks=0,failures=0;auto check=[&](bool ok,const char* label){checks++;if(!ok){failures++;std::cerr<<"FAIL: "<<label<<'\n';}};
 auto reset=[&](){
  game=Game{};game.props.clear();game.enemies.clear();game.forage.clear();game.chests={};game.bag={};game.player={20,20};game.messageTime=0;
  intro={};towerStory={};clearStoryMenus();welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=developerMenuOpen=false;
  towerPanelOpen=false;quickPickerSlot=-1;dragging={};inventoryClicks.clear();corpsePage=0;corpsePageOwner=-1;
 };
 auto shot=[&](const char* name){renderScene();saveTexture(canvas,(root/name).string().c_str());};
 auto click=[&](){auto r=takeAllLootRect();mouse={r.x+r.width*.5f,r.y+r.height*.5f};leftClickGameplay({},10);};
 reset();Enemy corpse;corpse.id=9901;corpse.type=1;corpse.alive=false;corpse.pos=game.player+V{1,0};corpse.loot={};corpse.loot[0]=item(Kind::Gold,0,20);corpse.loot[7]=item(Kind::Health,0,4);corpse.loot[14]=item(Kind::Sword,2);game.enemies.push_back(corpse);game.openedCorpse=corpse.id;
 syncCorpsePage();corpsePage=2;shot("corpse-before.png");click();
 check(std::all_of(game.enemies[0].loot.begin(),game.enemies[0].loot.end(),[](Item i){return i.empty();}),"one real button press collects all corpse pages");
 check(game.bag[0].kind==Kind::Gold&&game.bag[1].kind==Kind::Health&&game.bag[2].tier==2,"the button places every item in the backpack");
 check(game.swing==0&&!game.bowDrawing&&dragging.place==Place::None,"take all does not trigger combat or dragging");shot("corpse-after.png");
 reset();corpse.loot={};corpse.loot[0]=item(Kind::Gold,0,1);corpse.loot[1]=item(Kind::Mana,0,2);game.enemies.push_back(corpse);game.openedCorpse=corpse.id;
 for(auto& value:game.bag)value=item(Kind::Sword);game.bag[0]=item(Kind::Gold,0,98);click();
 check(game.bag[0].count==98&&game.enemies[0].loot[0].count==1&&game.enemies[0].loot[1].count==2,"failed take all leaves even the otherwise mergeable gold untouched");
 check(game.message.find("Brak miejsca")!=std::string::npos&&game.swing==0,"capacity error is visible and the click does not attack");shot("backpack-full.png");
 reset();auto& chest=game.chests[0];chest.id=9902;chest.location=game.location;chest.opened=true;chest.pos=game.player+V{1,0};chest.loot={};chest.loot[0]=item(Kind::Arrow,0,120);chest.loot[1]=item(Kind::Helmet,2);game.openedChest=chest.id;shot("chest-before.png");click();
 check(game.bag[0].count==99&&game.bag[1].count==21&&game.bag[2].kind==Kind::Helmet,"the chest button collects and splits all stacks");shot("chest-after.png");
 reset();RenderTexture2D master=LoadRenderTexture(320,360);AnimationState state;state.direction=Direction::SE;
 std::array<Item,6> gear{{{},{},{},item(Kind::Armor),item(Kind::Boots),{}}};
 actor_art::finishShadowFrame();BeginTextureMode(master);ClearBackground(BLANK);actor_art::draw(0,160,305,2.3f,state,0,false,false,Kind::None,false,BLANK,nullptr,-1,-1,-1,0,false,&gear);EndTextureMode();
 saveTexture(master,(root/"hero-master.png").string().c_str());UnloadRenderTexture(master);
 actor_art::Rig projection(160,305,2.3f,Direction::SE);auto pose=hero_motion::evaluate(0,0,0,0);
 const char* names[]={"root","pelvis","spine","neck","shoulder_l","elbow_l","hand_l","shoulder_r","elbow_r","hand_r","hip_l","knee_l","foot_l","hip_r","knee_r","foot_r","weapon"};
 std::ofstream joints(root/"hero-joints.json");joints<<"{\"width\":320,\"height\":360,\"points\":[";
 for(int b=0;b<hero_motion::data::BoneCount;b++){auto p=pose.world[b].p;auto q=projection.project({p.x,p.y,p.z});if(b)joints<<',';joints<<"{\"name\":\""<<names[b]<<"\",\"x\":"<<q.x<<",\"y\":"<<q.y<<'}';}
 auto crown=projection.project({0,0,82});joints<<",{\"name\":\"crown\",\"x\":"<<crown.x<<",\"y\":"<<crown.y<<"}]}";joints.close();
 check(!audioSystem.available,"UI and sprite diagnostic never opens an audio device");
 std::ofstream report(root/"ui-verification.json");report<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"audio\":false,\"window_hidden\":true}\n";
 hero_paint::unload();audioSystem.shutdown();sync_present::shutdown();CloseWindow();std::cout<<"Take all UI: "<<checks-failures<<'/'<<checks<<" passed\n";return failures?2:0;
}
