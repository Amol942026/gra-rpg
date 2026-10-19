// Actual player inventory and renderer integration. All diagnostics are silent.
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
int main(int argc,char** argv){
 if(argc!=2||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::create_directories("qa-equipment");
 std::ofstream report("qa-equipment/ui-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall equipment QA --no-audio");
 if(!IsWindowReady())return 4;audioSystem.init(false,false);
 makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 check(PlayerBuild&&!audioSystem.available,"equipment QA runs real player code without an audio device");
 game=Game{};intro={};towerStory={};clearStoryMenus();welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=false;dragging={};
 game.props.clear();game.enemies.clear();game.forage.clear();game.player={20,20};game.messageTime=0;
 RenderTexture2D sample=LoadRenderTexture(400,300);
 auto capture=[&](bool portrait=false){
  actor_art::finishShadowFrame();BeginTextureMode(sample);ClearBackground(BLANK);hero(200,245,2.1f,portrait);EndTextureMode();
  Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);return im;
 };
 auto different=[](const Image& a,const Image& b){
  Color* x=LoadImageColors(a);Color* y=LoadImageColors(b);int n=0;
  for(int i=0;i<a.width*a.height;i++)if(std::abs(int(x[i].r)-y[i].r)+std::abs(int(x[i].g)-y[i].g)+std::abs(int(x[i].b)-y[i].b)+std::abs(int(x[i].a)-y[i].a)>25)n++;
  UnloadImageColors(x);UnloadImageColors(y);return n;
 };
 RenderTexture2D sheet=LoadRenderTexture(1500,2000);BeginTextureMode(sheet);ClearBackground(Ink);EndTextureMode();
 int index=0;
 for(int k=1;k<ItemKindCount;k++){
  Kind kind=Kind(k);int slot=gearSlot(kind);if(slot<0)continue;
  game.equipment={};game.equipment[3]=item(Kind::Armor);game.animation={};game.animation.direction=Direction::SE;game.swing=game.bowReleaseTime=0;game.bowDrawing=false;
  Kind baseline=archetype(kind);game.equipment[slot]=kind==baseline?Item{}:item(baseline);Image before=capture();Image portraitBefore=capture(true);
  game.equipment[slot]={};game.bag[10]=item(kind);game.bag[11]={};
  check(game.transfer({Place::Bag,10},{Place::Equip,slot}),"real equipment transfer accepts this item");
  Image after=capture(),portraitAfter=capture(true);int changed=different(before,after),portraitChanged=different(portraitBefore,portraitAfter);
  report<<"Item "<<itemName(item(kind))<<": world pixels="<<changed<<", portrait pixels="<<portraitChanged<<'\n';
  check(changed>3,"exact equipped item visibly changes the GPU-rendered world hero");
  check(portraitChanged>3,"exact equipped item also changes the portrait");
  Texture2D texture=LoadTextureFromImage(after);BeginTextureMode(sheet);
  float x=(index%5)*300.f,y=(index/5)*250.f;DrawTexturePro(texture,{0,0,400,300},{x,y,300,225},{0,0},0,WHITE);icon(item(kind),x+38,y+55,.65f);center(itemName(item(kind)),x+150,y+222,12,Gold);EndTextureMode();UnloadTexture(texture);index++;
  check(game.transfer({Place::Equip,slot},{Place::Bag,11}),"real removal returns the item to the backpack");
  Image removed=capture();check(different(after,removed)>3,"removing equipment visibly removes it from the hero");
  UnloadImage(before);UnloadImage(after);UnloadImage(removed);UnloadImage(portraitBefore);UnloadImage(portraitAfter);
 }
 saveTexture(sheet,"qa-equipment/all-39-items.png");UnloadRenderTexture(sheet);
 const std::array<Item,6> outfits[]={
  {item(Kind::GuardianSword),item(Kind::GuardianHelmet),item(Kind::RootShield),item(Kind::GuardianArmor),item(Kind::SilkBoots),item(Kind::EchoRing)},
  {item(Kind::DuneBlade),item(Kind::ScarabHelmet),item(Kind::DuneShield),item(Kind::ScarabArmor),item(Kind::CobraBoots),item(Kind::SunAmulet)},
  {item(Kind::MoonBow),item(Kind::WebCowl),item(Kind::RootShield),item(Kind::WolfCloak),item(Kind::BanditBoots),item(Kind::GroveAmulet)},
  {item(Kind::MirageBow),item(Kind::BoneHelmet),item(Kind::DuneShield),item(Kind::SandRobe),item(Kind::CobraBoots),item(Kind::GelAmulet)}
 };
 for(Kind bow:{Kind::Bow,Kind::MoonBow,Kind::BanditBow,Kind::MirageBow}){
  game.equipment={};game.equipment[0]=item(bow);game.animation={};game.animation.direction=Direction::NW;
  Image without=capture();game.equipment[2]=item(Kind::RootShield);Image with=capture();
  check(different(without,with)>20,"equipped shield remains visible on the back while a bow occupies both hands");UnloadImage(without);UnloadImage(with);
 }
 RenderTexture2D preview=LoadRenderTexture(920,280);BeginTextureMode(preview);ClearBackground(Ink);EndTextureMode();
 const char* labels[]={"STRAZNIK RUIN","SKARABEUSZ PUSTYNI","LESNY LOWCA","PIASKOWA BURZA"};
 for(int i=0;i<4;i++){
  game.equipment=outfits[i];game.animation={};game.animation.direction=Direction::SE;actor_art::finishShadowFrame();
  BeginTextureMode(preview);center(labels[i],115+i*230,16,14,Gold);hero(115+i*230,221,1.95f,false);
  for(int slot=0;slot<6;slot++)icon(game.equipment[slot],25+i*230+slot*35,253,.45f);EndTextureMode();
 }
 saveTexture(preview,"qa-equipment/outfits-preview.png");UnloadRenderTexture(preview);
 RenderTexture2D motion=LoadRenderTexture(1600,1600);BeginTextureMode(motion);ClearBackground(Ink);EndTextureMode();
 for(int outfit=0;outfit<4;outfit++)for(int mode=0;mode<5;mode++)for(int direction=0;direction<8;direction++){
  game.equipment=outfits[outfit];game.animation={};game.animation.direction=Direction(direction);game.animation.phase=.23f;game.animation.moving=mode==1||mode==2;game.animation.runBlend=mode==2?1:0;
  game.swing=mode==3&&archetype(game.equipment[0].kind)!=Kind::Bow?SwingDuration*.5f:0;game.meleeDuration=SwingDuration;
  game.bowDrawing=mode==3&&archetype(game.equipment[0].kind)==Kind::Bow;game.bowDrawTime=BowDrawDuration*.7f;game.bowReleaseTime=0;game.rollRemaining=mode==4?RollDuration*.6f:0;
  Image image=capture();Color* pixels=LoadImageColors(image);int solid=0,clipped=0;
  for(int y=0;y<300;y++)for(int x=0;x<400;x++)if(pixels[y*400+x].a>220){solid++;clipped+=x<2||x>397||y<2||y>297;}
  check(solid>1500&&clipped==0,"equipped hero remains complete in eight directions during idle, walk, sprint, attack/draw and roll");
  if(mode==0){
   Texture2D texture=LoadTextureFromImage(image);BeginTextureMode(motion);DrawTexturePro(texture,{0,0,400,300},{direction*200.f,outfit*400.f,200,150},{0,0},0,WHITE);EndTextureMode();UnloadTexture(texture);
  }
  if(mode==2&&direction==int(Direction::SE)){
   Texture2D texture=LoadTextureFromImage(image);BeginTextureMode(motion);DrawTexturePro(texture,{0,0,400,300},{0,outfit*400.f+170,200,150},{0,0},0,WHITE);EndTextureMode();UnloadTexture(texture);
  }
  UnloadImageColors(pixels);UnloadImage(image);
 }
 saveTexture(motion,"qa-equipment/outfits-and-directions.png");UnloadRenderTexture(motion);
 game.rollRemaining=game.swing=0;game.bowDrawing=false;game.animation={};game.equipment=outfits[1];game.messageTime=0;
 renderScene();check(world_shadows::ready,"equipped appearance participates in the real world shadow pass");saveTexture(canvas,"qa-equipment/outfit-forest.png");
 game.activateField(WorldLocation::Desert);game.player=DesertArrival;game.messageTime=0;renderScene();saveTexture(canvas,"qa-equipment/outfit-desert.png");
 check(game.equipment[0].kind==Kind::DuneBlade&&game.equipment[3].kind==Kind::ScarabArmor,"travel preserves exact visible equipment");
 for(bool prologue:{false,true}){
  intro={};towerStory={};if(prologue)intro.phase=IntroPhase::Dialogue;else towerStory.beginReturn(1);
  game.equipment=outfits[0];renderScene();Image first=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&first);ImageCrop(&first,{350,280,300,300});
  game.equipment=outfits[1];renderScene();Image second=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&second);ImageCrop(&second,{350,280,300,300});
  check(different(first,second)>300,prologue?"prologue close-up uses the exact equipped items":"mage tower story close-up retains the exact equipped items");
  UnloadImage(first);UnloadImage(second);
 }
 UnloadRenderTexture(sample);world_shadows::unload();actor_art::finishShadowFrame();fallen_art::unload();unloadHoverArt();unloadSkillArt();unloadTowerRoomArt();tower_stairs::unload();tower_furniture::unload();desert_art::unload();sand_prison::unload();treasure_art::unload();
 for(auto& texture:propArt)UnloadRenderTexture(texture);UnloadRenderTexture(terrain);UnloadTexture(minimapTerrain);if(canvas.id)UnloadRenderTexture(canvas);if(worldCanvas.id)UnloadRenderTexture(worldCanvas);typography.clear();audioSystem.shutdown();CloseWindow();
 report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';std::printf("Equipment UI: %d/%d checks passed\n",checks-failures,checks);return failures?2:0;
}
