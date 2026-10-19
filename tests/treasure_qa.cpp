// Real player UI integration, always silent; reuse existing art and HUD.
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main

int main(int argc,char** argv){
 if(argc!=2||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::create_directories("qa-treasure");
 std::ofstream report("qa-treasure/ui-tests.txt");int checks=0,failures=0;
 auto check=[&](bool result,const char* label){checks++;failures+=!result;report<<(result?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);
 sync_present::useOpenGL();InitWindow(W,H,"Emberfall treasure QA --no-audio");
 if(!IsWindowReady())return 4;audioSystem.init(false,false);
 makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();
 makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 check(PlayerBuild&&!audioSystem.available,"treasure QA uses player code and no audio device");
 check(treasure_art::atlas.id&&treasure_art::atlas.width==768&&treasure_art::atlas.height==256,"Krita chest atlas loads once at its expected dimensions");
 RenderTexture2D sample=LoadRenderTexture(150,125);
 for(int state=0;state<3;state++){
  auto chest=game.chests[0];chest.opened=state>0;if(state==2)chest.loot={};
  const Color backdrop={67,99,91,255};const Vector2 anchor={75,103};
  BeginTextureMode(sample);ClearBackground(backdrop);treasure_art::draw(chest,anchor);EndTextureMode();
  Image image=LoadImageFromTexture(sample.texture);ImageFlipVertical(&image);
  auto same=[&](Color color){return color.r==backdrop.r&&color.g==backdrop.g&&color.b==backdrop.b;};
  Rectangle r=treasure_art::bounds(anchor);
  check(same(GetImageColor(image,int(r.x)+1,int(r.y)+1))&&same(GetImageColor(image,int(r.x+r.width)-2,int(r.y)+1)),"GPU rendering preserves background through transparent chest corners, with no black panel");
  Color* pixels=LoadImageColors(image);int visible=0;
  for(int i=0;i<image.width*image.height;i++)visible+=!same(pixels[i]);
  check(visible>1800&&visible<7000,"each chest state renders a bounded, detailed visible silhouette");
  UnloadImageColors(pixels);ExportImage(image,("qa-treasure/sprite-state-"+std::to_string(state)+".png").c_str());UnloadImage(image);
 }
 UnloadRenderTexture(sample);
 auto reset=[](){game=Game(9876);intro={};towerStory={};clearStoryMenus();welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;developerActive=developerMenuOpen=false;dragging={};mouse={0,0};};
 auto pointAt=[&](const TreasureChest& chest){
  Rectangle r=treasureBounds(screen(chest.pos));
  for(float y=r.y+2;y<r.y+r.height-2;y+=3)for(float x=r.x+2;x<r.x+r.width-2;x+=3){
   WorldHover hit=pickWorld({x,y});if(hit.kind==HoverKind::Chest&&hit.index==chest.id){mouse={x,y};return true;}
  }return false;
 };
 for(int id=0;id<TreasureCount;id++){
  reset();const auto& layout=TreasureLayout[id];game.activateField(layout.location);
  game.player=layout.pos+V{1.5f,0};game.messageTime=0;
  renderScene();saveTexture(canvas,"qa-treasure/chest-"+std::to_string(id)+"-closed.png");
  check(treasure_art::frame(game.chests[id])==0,"untouched chest uses the closed artwork");
  Rectangle bounds=treasureBounds(screen(layout.pos));
  check(!treasureContains(game.chests[id],{bounds.x+1,bounds.y+1}),"transparent sprite corners do not intercept world clicks");
  check(pointAt(game.chests[id]),"each of the four world markers can be picked at its real location");
  check(inspectWorld(pickWorld(mouse)).source==HoverKind::Chest,"hover inspection identifies treasure");
  check(rightClickWorldOrInventory()&&game.openedChest==id&&game.swing==0,"right click opens treasure without attacking");
  check(treasure_art::frame(game.chests[id])==1,"opened treasure reveals the raised lid, coins and gem");
  game.messageTime=0;renderScene();saveTexture(canvas,"qa-treasure/chest-"+std::to_string(id)+"-loot.png");
  Item value=game.chests[id].loot[0];Rectangle r=lootRect(0);mouse={r.x+20,r.y+20};
  Slot source=hoverSlot();check(source.place==Place::Chest&&source.owner==id&&source.index==0,"HUD loot slot refers to the selected chest");
  check(itemTooltipText(value,source).chance=="Skrzynia: 3 przedmioty · pospolite / rzadkie","chest tooltip does not misrepresent monster drop rates");
  clickWorldOrInventory();check(dragging.place==Place::Chest,"left click starts dragging chest gear from the HUD");
  Rectangle target=bagRect(10);mouse={target.x+20,target.y+20};finishDrag();
  check(dragging.place==Place::None&&game.bag[10].kind==value.kind&&game.chests[id].loot[0].empty(),"actual drag handler transfers loot into the bag once");
  Rectangle close=corpseCloseRect();mouse={close.x+8,close.y+8};leftClickGameplay({});
  check(game.openedChest==-1,"loot close button closes the chest panel");
  check(pointAt(game.chests[id])&&clickWorldOrInventory()&&game.openedChest==id&&game.swing==0,"left click reopens the same chest without a weapon swing");
  game.bag[11]={};game.bag[12]={};game.transfer({Place::Chest,1,id},{Place::Bag,11});game.transfer({Place::Chest,2,id},{Place::Bag,12});
  game.messageTime=0;renderScene();saveTexture(canvas,"qa-treasure/chest-"+std::to_string(id)+"-empty.png");
  int count=0;for(auto loot:game.chests[id].loot)count+=!loot.empty();check(count==0,"all three pieces leave an empty persistent chest");
  check(treasure_art::frame(game.chests[id])==2,"fully looted chest uses the empty interior artwork");
 }
 // Stable tree occlusion uses the same sorted objects as actual rendering.
 reset();auto& chest=game.chests[0];game.player=chest.pos+V{1.5f,0};game.props.clear();game.enemies.clear();game.messageTime=0;
 check(pointAt(chest),"unoccluded treasure is selectable");Vector2 selected=mouse;
 game.props.push_back({chest.pos+V{1,1},0,1.2f});
 check(pickWorld(selected).kind==HoverKind::Prop,"a foreground tree blocks picking treasure through its canopy");
 report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';
 world_shadows::unload();actor_art::finishShadowFrame();fallen_art::unload();unloadHoverArt();
 unloadSkillArt();unloadTowerRoomArt();tower_stairs::unload();tower_furniture::unload();desert_art::unload();sand_prison::unload();treasure_art::unload();
 for(auto& texture:propArt)UnloadRenderTexture(texture);
 UnloadRenderTexture(terrain);UnloadTexture(minimapTerrain);if(canvas.id)UnloadRenderTexture(canvas);
 typography.clear();audioSystem.shutdown();CloseWindow();return failures?2:0;
}
