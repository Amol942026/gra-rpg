// Exercise the real player press/release handlers with deterministic click times.
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
int main(int argc,char** argv){
 if(argc!=2||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::create_directories("qa-loot-controls");
 std::ofstream report("qa-loot-controls/ui-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall loot controls QA --no-audio");
 if(!IsWindowReady())return 4;audioSystem.init(false,false);
 makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 check(PlayerBuild&&!audioSystem.available,"loot controls use player code with no audio device");
 auto reset=[](){
  game=Game(9876);game.props.clear();game.enemies.clear();game.forage.clear();game.bag={};game.player={15,15};game.messageTime=0;
  intro={};towerStory={};clearStoryMenus();welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=false;developerActive=developerMenuOpen=false;dragging={};inventoryClicks.clear();mouse={0,0};nearbyPage=corpsePage=0;corpsePageOwner=-1;
 };
 auto point=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
 auto release=[](){if(dragging.place!=Place::None)finishDrag();else releaseWorldBow();};
 auto click=[&](Rectangle r,double time){point(r);leftClickGameplay({},time);release();};
 auto doubleClick=[&](Rectangle r,double time){click(r,time);click(r,time+.16);};
 for(int id=0;id<TreasureCount;id++){
  reset();const auto layout=TreasureLayout[id];game.activateField(layout.location);game.props.clear();game.enemies.clear();game.forage.clear();game.player=layout.pos+V{1.5f,0};game.messageTime=0;
  Vector2 p=screen(layout.pos);Rectangle r=treasureBounds(p);
  auto at=[&](float x,float y){return Vector2{r.x+x*treasure_art::Scale,r.y+y*treasure_art::Scale};};
  Vector2 a=at(44,175),b=at(118,140),c=at(217,186),d=at(141,225);
  check(CheckCollisionPointTriangle(p,a,b,c)||CheckCollisionPointTriangle(p,a,c,d),"world collision center lies inside the chest's actual base footprint instead of below it");
  renderScene();saveTexture(canvas,"qa-loot-controls/chest-"+std::to_string(id)+"-grounded-closed.png");
  check(game.openChest(id),"chest opens at its real location");auto& chest=game.chests[id];chest.loot={};chest.loot[0]=item(Kind::Helmet,2);game.equipment[1]=item(Kind::BoneHelmet,1);
  point(lootRect(0));leftClickGameplay({},1);check(dragging.place==Place::Chest,"one left press still starts ordinary loot dragging");
  point(equipRect(1));release();
  check(chest.loot[0].empty()&&game.equipment[1].kind==Kind::Helmet&&game.equipment[1].tier==2,"real drag equips gear directly from each chest");
  check(game.bag[0].kind==Kind::BoneHelmet&&game.bag[0].tier==1,"direct chest equip keeps replaced gear in the backpack");
  chest.loot[0]=item(Kind::Dagger,2);click(lootRect(0),2);
  check(chest.loot[0].kind==Kind::Dagger&&game.bag[1].empty()&&dragging.place==Place::None,"first click-release leaves loot in place and does not duplicate it");
  click(lootRect(0),2.16);
  check(chest.loot[0].empty()&&game.bag[1].kind==Kind::Dagger&&game.equipment[0].kind==Kind::Sword,"second left click collects chest loot into the backpack without auto-equipping");
  doubleClick(bagRect(1),3);
  check(game.equipment[0].kind==Kind::Dagger&&game.equipment[0].tier==2&&game.bag[1].kind==Kind::Sword,"backpack double click equips to the correct slot and swaps old gear safely");
  check(game.swing==0&&!game.bowDrawing&&game.arrows.empty(),"double clicking HUD items never attacks or shoots into the world");
  game.messageTime=0;renderScene();saveTexture(canvas,"qa-loot-controls/chest-"+std::to_string(id)+"-grounded-open.png");
 }
 auto corpse=[&](int index=0){
  reset();V p=game.player+V{1,0};game.enemies.push_back({77,1,p,p,0,100});game.enemies[0].alive=false;game.enemies[0].loot={};game.enemies[0].loot[index]=item(Kind::ScarabArmor,2);game.openCorpse(77);
  if(index>=5){corpsePage=index/5;corpsePageOwner=77;}
 };
 for(int index:{0,5,10}){
  corpse(index);doubleClick(lootRect(index%5),10+index);
  check(game.enemies[0].loot[index].empty()&&game.bag[0].kind==Kind::ScarabArmor,"double click collects the correct real corpse slot on all loot pages");
  doubleClick(bagRect(0),30+index);
  check(game.equipment[3].kind==Kind::ScarabArmor&&game.bag[0].kind==Kind::Armor,"corpse loot can be equipped by a second double click in the backpack");
 }
 corpse();point(lootRect(0));leftClickGameplay({},50);point(equipRect(3));release();
 check(game.equipment[3].kind==Kind::ScarabArmor&&game.bag[0].kind==Kind::Armor&&game.enemies[0].loot[0].empty(),"direct drag from corpse to occupied equipment is also supported");
 corpse();for(auto& value:game.bag)value=item(Kind::Sword);doubleClick(lootRect(0),60);
 check(game.enemies[0].loot[0].kind==Kind::ScarabArmor&&game.equipment[3].kind==Kind::Armor,"full backpack rejects double-click collection without losing loot");
 point(lootRect(0));leftClickGameplay({},61);point(equipRect(3));release();
 check(game.enemies[0].loot[0].kind==Kind::ScarabArmor&&game.equipment[3].kind==Kind::Armor,"full backpack rejects replacing worn gear by direct dragging");
 game.equipment[3]={};point(lootRect(0));leftClickGameplay({},62);point(equipRect(3));release();
 check(game.equipment[3].kind==Kind::ScarabArmor&&game.enemies[0].loot[0].empty(),"direct dragging to an empty equipment slot works with a full backpack");
 corpse();game.enemies[0].loot[0]=item(Kind::Gold,0,8);for(auto& value:game.bag)value=item(Kind::Sword);game.bag[0]=item(Kind::Gold,0,96);doubleClick(lootRect(0),70);
 check(game.bag[0].count==99&&game.enemies[0].loot[0].count==5,"double click fills existing stacks and preserves excess loot in the corpse");
 reset();game.bag[10]=item(Kind::Helmet);click(bagRect(10),80);click(bagRect(10),80.6);
 check(game.equipment[1].empty()&&game.bag[10].kind==Kind::Helmet,"slow clicks do not equip an item");
 inventoryClicks.clear();click(bagRect(10),82);inventoryClicks.moved(mouse.x+20,mouse.y);click(bagRect(10),82.15);
 check(game.equipment[1].empty()&&game.bag[10].kind==Kind::Helmet,"cursor movement resets a pending double click");
 inventoryClicks.clear();click(bagRect(10),83);paused=true;point(bagRect(10));leftClickGameplay({},83.15);paused=false;click(bagRect(10),83.25);
 check(game.equipment[1].empty()&&game.bag[10].kind==Kind::Helmet,"opening a menu invalidates an old click pair");
 inventoryClicks.clear();game.bag[11]=item(Kind::Boots);click(bagRect(10),84);click(bagRect(11),84.15);
 check(game.equipment[1].empty()&&game.equipment[4].empty(),"clicks on different backpack items cannot form an equip action");
 inventoryClicks.clear();game.bag[12]=item(Kind::Health,0,3);game.hp=10;doubleClick(bagRect(12),85);
 check(game.bag[12].count==3&&game.hp==10,"non-equipment double clicks do not accidentally use potions");
 reset();game.bag[10]=item(Kind::Helmet);point(bagRect(10));leftClickGameplay({},90);point(bagRect(11));release();click(bagRect(11),90.15);
 check(game.bag[11].kind==Kind::Helmet&&game.equipment[1].empty(),"a drag/drop cannot become the first half of a double click at its destination");
 reset();game.bag[10]=item(Kind::Bow);game.bag[5]=item(Kind::Arrow,0,20);doubleClick(bagRect(10),92);
 check(game.equipment[0].kind==Kind::Bow&&!game.bowDrawing&&game.arrows.empty()&&game.ammoCount()==20,"equipping a bow by double click never draws or releases a shot");
 world_shadows::unload();actor_art::finishShadowFrame();fallen_art::unload();unloadHoverArt();unloadSkillArt();unloadTowerRoomArt();tower_stairs::unload();tower_furniture::unload();desert_art::unload();sand_prison::unload();treasure_art::unload();
 for(auto& texture:propArt)UnloadRenderTexture(texture);UnloadRenderTexture(terrain);UnloadTexture(minimapTerrain);if(canvas.id)UnloadRenderTexture(canvas);if(worldCanvas.id)UnloadRenderTexture(worldCanvas);typography.clear();audioSystem.shutdown();CloseWindow();
 report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';std::printf("Loot controls: %d/%d checks passed\n",checks-failures,checks);return failures?2:0;
}
