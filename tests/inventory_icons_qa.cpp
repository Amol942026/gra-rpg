// Real player HUD/icon/cinematic handlers, hidden window and closed audio.
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#include <cstdint>
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
 check(inventory_icons::atlas.id&&inventory_icons::atlas.width==inventory_icons::Width&&inventory_icons::atlas.height==inventory_icons::Height,"complete icon atlas is preloaded at startup");
 check(inventory_icons::lootAtlas.id&&inventory_icons::lootAtlas.width==1024&&inventory_icons::lootAtlas.height==384,"23 remaining item illustrations preload in their separate atlas");
 check(inventory_icons::gearAtlas.id&&inventory_icons::gearAtlas.texture.width==inventory_icons::GearWidth
     &&inventory_icons::gearAtlas.texture.height==inventory_icons::Cell,"shields and both helmet styles are preloaded from their worn art");
 auto textureId=inventory_icons::atlas.id,lootId=inventory_icons::lootAtlas.id;inventory_icons::load();check(textureId==inventory_icons::atlas.id&&lootId==inventory_icons::lootAtlas.id,"reusing graphics never reloads or duplicates either icon texture");
 Image source=LoadImage("assets/sprites/equipment-icons.png");Color* pixels=LoadImageColors(source);std::vector<Kind> kinds;std::set<int> cells;
 Image protective=LoadImageFromTexture(inventory_icons::gearAtlas.texture);Color* protectivePixels=LoadImageColors(protective);
 if(!pixels||!protectivePixels)return 6;
 std::set<uint64_t> protectiveHashes;
 for(int k=1;k<ItemKindCount;k++){
  const Kind kind=Kind(k);
  const int cell=inventory_icons::cell(kind);
  if(gearSlot(kind)<0){check(cell==-1,"new remaining items never change the equipment cell ordering");continue;}
  kinds.push_back(kind);cells.insert(cell);
  const int procedural=inventory_icons::gearCell(kind);
  const int size=inventory_icons::Cell;
  const int originX=(procedural>=0?procedural:cell%inventory_icons::Columns)*size;
  const int originY=procedural>=0?0:cell/inventory_icons::Columns*size;
  const int width=procedural>=0?protective.width:source.width;
  auto art=procedural>=0?protectivePixels:pixels;
  int solid=0,edge=0,left=size,right=-1,top=size,bottom=-1;
  uint64_t fingerprint=14695981039346656037ull;
  for(int y=0;y<size;y++)for(int x=0;x<size;x++){
   Color c=art[(originY+y)*width+originX+x];
   if(c.a>30){solid++;left=std::min(left,x);right=std::max(right,x);top=std::min(top,y);bottom=std::max(bottom,y);}
   if(x<4||x>=size-4||y<4||y>=size-4)edge+=c.a>0;
   if(procedural>=0)for(unsigned char channel:{c.r,c.g,c.b,c.a})fingerprint=(fingerprint^channel)*1099511628211ull;
  }
  check(solid>90&&edge==0,"every equipment cell has a visible padded transparent illustration");
  if(procedural>=0){
   check(protectiveHashes.insert(fingerprint).second,"new shields and helmets have distinct icon artwork");
   if(archetype(kind)==Kind::Shield){
    const int boxWidth=right-left+1,boxHeight=bottom-top+1;
    const float fill=float(solid)/std::max(1,boxWidth*boxHeight);
    check(std::abs(boxWidth-boxHeight)<=8&&fill>.71f&&fill<.86f,"new shield icon has a round alpha silhouette");
   }
  }
 }
 check(protectiveHashes.size()==ProtectiveGearCount+OpenHelmetKinds.size(),"shared worn artwork is present for each new shield and each fitted helmet");
 check(kinds.size()==EquipmentKindCount&&cells.size()==EquipmentKindCount&&*cells.begin()==0&&*cells.rbegin()==EquipmentKindCount-1,"all equipment kinds map to distinct stable cells");UnloadImageColors(pixels);UnloadImage(source);
 UnloadImageColors(protectivePixels);UnloadImage(protective);
 Image loot=LoadImage("assets/sprites/loot-icons.png");pixels=LoadImageColors(loot);std::set<int> lootCells;int otherItems=0;
 for(int k=1;k<ItemKindCount;k++)if(gearSlot(Kind(k))<0){
  int cell=inventory_icons::lootCell(Kind(k));lootCells.insert(cell);int solid=0,edge=0;
  for(int y=0;y<128;y++)for(int x=0;x<128;x++){Color c=pixels[(cell/8*128+y)*loot.width+cell%8*128+x];solid+=c.a>30;if(x<4||x>=124||y<4||y>=124)edge+=c.a>0;}
  check(solid>90&&edge==0,"every remaining item has a distinct visible padded illustration");kinds.push_back(Kind(k));otherItems++;
 }
 check(otherItems==23&&lootCells.size()==23&&*lootCells.begin()==0&&*lootCells.rbegin()==22,"all 23 remaining item kinds have independent stable cells");
 check(inventory_icons::lootCell(Kind::None)==-1&&inventory_icons::lootCell(Kind::Sword)==-1&&inventory_icons::lootCell(Kind(-1))==-1,"empty, equipment and invalid kinds cannot alias remaining-item cells");
 UnloadImageColors(pixels);UnloadImage(loot);
 RenderTexture2D preview=LoadRenderTexture(1440,int((kinds.size()+11)/12)*128);BeginTextureMode(preview);ClearBackground(Ink);
 for(size_t i=0;i<kinds.size();i++){
  float x=60+float(i%12)*120,y=60+float(i/12)*128;icon(item(kinds[i]),x,y,1.7f);txt(storyPrefix(itemName(item(kinds[i])),19),x-53,y+44,11,Text);icon(item(kinds[i]),x+36,y+35,.55f,.45f);
 }
 EndTextureMode();saveTexture(preview,(root/"icons-player-renderer.png").string().c_str());UnloadRenderTexture(preview);
 game=Game{};intro={};towerStory={};clearStoryMenus();game.enemies.clear();game.props.clear();game.messageTime=0;mouse={0,0};
 for(size_t page=0;page<(kinds.size()+BagSize-1)/BagSize;page++){
  std::fill(game.bag.begin(),game.bag.end(),Item{});for(int slot=0;slot<BagSize&&page*BagSize+slot<kinds.size();slot++){Kind kind=kinds[page*BagSize+slot];game.bag[slot]=item(kind,0,stackable(kind)?37:1);}
  renderScene();saveTexture(canvas,(root/("inventory-page-"+std::to_string(page)+".png")).string().c_str());
 }
 auto count=[](Cue cue){return std::count_if(game.sounds.begin(),game.sounds.end(),[&](auto event){return event.cue==cue;});};
 startIntro();intro.enter(IntroPhase::Arriving);intro.arrivalPending=true;game.sounds.clear();applyIntroArrival();applyIntroArrival();
 check(count(Cue::TeleportArrival)==1&&distance(game.player,ForestArrival)==0,"actual prologue arrival queues one destination burst, even if the handler is called twice");
 intro={};game.dawnQuest=DawnQuest::Returning;towerStory.beginReturn(1);towerStory.arrivalPending=true;game.sounds.clear();towerAction(false,false,.01f);towerAction(false,false,.01f);
 check(game.inTower()&&count(Cue::TeleportArrival)==1,"actual return cinematic queues one arrival after clearing the old scene");
 check(!IsAudioDeviceReady(),"scene transition checks never initialize hardware audio");
 inventory_icons::unload();check(!inventory_icons::atlas.id&&!inventory_icons::lootAtlas.id&&!inventory_icons::gearAtlas.id,"all item atlases are released at shutdown");
 audioSystem.shutdown();sync_present::shutdown();CloseWindow();
 std::ofstream summary(root/"summary.json");summary<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"equipment_icons\":"<<EquipmentKindCount<<",\"other_item_icons\":23,\"window_hidden\":true,\"audio_device_opened\":false,\"audio_played\":false}";
 std::cout<<"Native inventory and arrival QA: "<<checks<<" checks, "<<failures<<" failures\n";return failures?2:0;
}
