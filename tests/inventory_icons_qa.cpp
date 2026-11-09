// Real player HUD/icon/cinematic handlers, hidden window and closed audio.
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#include <iostream>
#include <set>
#undef main
int main(int argc,char** argv){
 if(argc!=4||std::string(argv[1])!="--no-audio"||std::string(argv[3])!="--background-test")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 std::ofstream report(root/"checks.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* text){checks++;failures+=!ok;report<<(ok?"PASS ":"FAIL ")<<text<<'\n';report.flush();};
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(1440,900,"Emberfall inventory icons silent QA");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 check(!IsAudioDeviceReady()&&!audioSystem.available,"actual player renderer keeps the audio device closed");
 check(IsWindowHidden(),"graphics window stays hidden");
 check(inventory_icons::atlas.id&&inventory_icons::atlas.width==1536&&inventory_icons::atlas.height==1024,"complete icon atlas is preloaded at startup");
 check(inventory_icons::lootAtlas.id&&inventory_icons::lootAtlas.width==1024&&inventory_icons::lootAtlas.height==384,"23 remaining item illustrations preload in their separate atlas");
 auto textureId=inventory_icons::atlas.id,lootId=inventory_icons::lootAtlas.id;inventory_icons::load();check(textureId==inventory_icons::atlas.id&&lootId==inventory_icons::lootAtlas.id,"reusing graphics never reloads or duplicates either icon texture");
 Image source=LoadImage("assets/sprites/equipment-icons.png");Color* pixels=LoadImageColors(source);std::vector<Kind> kinds;std::set<int> cells;
 for(int k=1;k<ItemKindCount;k++){
  int cell=inventory_icons::cell(Kind(k));if(gearSlot(Kind(k))<0){check(cell==-1,"new remaining items never change the equipment cell ordering");continue;}
  kinds.push_back(Kind(k));cells.insert(cell);int solid=0,edge=0;
  for(int y=0;y<128;y++)for(int x=0;x<128;x++){Color c=pixels[(cell/12*128+y)*source.width+cell%12*128+x];solid+=c.a>30;if(x<4||x>=124||y<4||y>=124)edge+=c.a>0;}
  check(solid>90&&edge==0,"every equipment cell has a visible padded transparent illustration");
 }
 check(kinds.size()==85&&cells.size()==85&&*cells.begin()==0&&*cells.rbegin()==84,"all 85 equipment kinds map to distinct stable cells");UnloadImageColors(pixels);UnloadImage(source);
 Image loot=LoadImage("assets/sprites/loot-icons.png");pixels=LoadImageColors(loot);std::set<int> lootCells;int otherItems=0;
 for(int k=1;k<ItemKindCount;k++)if(gearSlot(Kind(k))<0){
  int cell=inventory_icons::lootCell(Kind(k));lootCells.insert(cell);int solid=0,edge=0;
  for(int y=0;y<128;y++)for(int x=0;x<128;x++){Color c=pixels[(cell/8*128+y)*loot.width+cell%8*128+x];solid+=c.a>30;if(x<4||x>=124||y<4||y>=124)edge+=c.a>0;}
  check(solid>90&&edge==0,"every remaining item has a distinct visible padded illustration");kinds.push_back(Kind(k));otherItems++;
 }
 check(otherItems==23&&lootCells.size()==23&&*lootCells.begin()==0&&*lootCells.rbegin()==22,"all 23 remaining item kinds have independent stable cells");
 check(inventory_icons::lootCell(Kind::None)==-1&&inventory_icons::lootCell(Kind::Sword)==-1&&inventory_icons::lootCell(Kind(-1))==-1,"empty, equipment and invalid kinds cannot alias remaining-item cells");
 UnloadImageColors(pixels);UnloadImage(loot);
 RenderTexture2D preview=LoadRenderTexture(1440,1280);BeginTextureMode(preview);ClearBackground(Ink);
 for(size_t i=0;i<kinds.size();i++){
  float x=60+float(i%12)*120,y=60+float(i/12)*128;icon(item(kinds[i]),x,y,1.7f);txt(storyPrefix(itemName(item(kinds[i])),19),x-53,y+44,11,Text);icon(item(kinds[i]),x+36,y+35,.55f,.45f);
 }
 EndTextureMode();saveTexture(preview,(root/"icons-player-renderer.png").string().c_str());UnloadRenderTexture(preview);
 game=Game{};intro={};towerStory={};clearStoryMenus();game.enemies.clear();game.props.clear();game.messageTime=0;mouse={0,0};
 for(size_t page=0;page<(kinds.size()+BagSize-1)/BagSize;page++){
  game.bag={};for(int slot=0;slot<BagSize&&page*BagSize+slot<kinds.size();slot++){Kind kind=kinds[page*BagSize+slot];game.bag[slot]=item(kind,0,stackable(kind)?37:1);}
  renderScene();saveTexture(canvas,(root/("inventory-page-"+std::to_string(page)+".png")).string().c_str());
 }
 auto count=[](Cue cue){return std::count_if(game.sounds.begin(),game.sounds.end(),[&](auto event){return event.cue==cue;});};
 startIntro();intro.enter(IntroPhase::Arriving);intro.arrivalPending=true;game.sounds.clear();applyIntroArrival();applyIntroArrival();
 check(count(Cue::TeleportArrival)==1&&distance(game.player,ForestArrival)==0,"actual prologue arrival queues one destination burst, even if the handler is called twice");
 intro={};game.dawnQuest=DawnQuest::Returning;towerStory.beginReturn(1);towerStory.arrivalPending=true;game.sounds.clear();towerAction(false,false,.01f);towerAction(false,false,.01f);
 check(game.inTower()&&count(Cue::TeleportArrival)==1,"actual return cinematic queues one arrival after clearing the old scene");
 check(!IsAudioDeviceReady(),"scene transition checks never initialize hardware audio");
 inventory_icons::unload();check(!inventory_icons::atlas.id&&!inventory_icons::lootAtlas.id,"both item atlases are released at shutdown");
 audioSystem.shutdown();sync_present::shutdown();CloseWindow();
 std::ofstream summary(root/"summary.json");summary<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"equipment_icons\":85,\"other_item_icons\":23,\"window_hidden\":true,\"audio_device_opened\":false,\"audio_played\":false}";
 std::cout<<"Native inventory and arrival QA: "<<checks<<" checks, "<<failures<<" failures\n";return failures?2:0;
}
