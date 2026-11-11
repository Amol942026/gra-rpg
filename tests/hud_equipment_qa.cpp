#include "raylib.h"
#include <string>
#include <vector>
struct UiText {std::string value;Rectangle box;};
std::vector<UiText> uiText;bool traceText=false;
void captureText(Font,const char*,Vector2,float,float,Color);
#define DrawTextEx captureText
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#undef DrawTextEx
#include <iostream>
void captureText(Font font,const char* value,Vector2 p,float size,float spacing,Color color){
 if(traceText&&*value){auto extent=MeasureTextEx(font,value,size,spacing);uiText.push_back({value,{p.x,p.y,extent.x,extent.y}});}
 DrawTextEx(font,value,p,size,spacing,color);
}
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=fs::absolute(argv[2]);fs::create_directories(root);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall HUD equipment --no-audio");if(!IsWindowReady())return 4;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 int checks=0,failures=0;std::ofstream log(root/"checks.txt"),texts(root/"visible-text.tsv");
 auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;log<<(ok?"PASS ":"FAIL ")<<label<<'\n';if(!ok)std::cerr<<"FAIL "<<label<<'\n';};
 auto reset=[&](){
  game=Game{};
  game.props.clear();game.enemies.clear();game.forage.clear();game.chests={};
  game.player={20,20};game.messageTime=0;
  intro={};towerStory={};
  clearTransientUi();invalidatePausedWorld();
  welcome=help=paused=developerMenuOpen=developerActive=false;
  developerError.clear();menuPageTo(MenuPage::Home);
  mouse={0,0};hovered={};
 };
 auto shot=[&](const char* name){uiText.clear();traceText=true;renderScene();traceText=false;saveTexture(canvas,(root/(std::string(name)+".png")).string().c_str());for(auto& t:uiText)texts<<name<<'\t'<<t.box.x<<'\t'<<t.box.y<<'\t'<<t.box.width<<'\t'<<t.box.height<<'\t'<<t.value<<'\n';};
 auto contains=[&](const std::string& value){return std::any_of(uiText.begin(),uiText.end(),[&](const UiText& t){return t.value.find(value)!=std::string::npos;});};
 auto exact=[&](const std::string& value){return std::any_of(uiText.begin(),uiText.end(),[&](const UiText& t){return t.value==value;});};
 reset();game.level=12;game.xp=experienceForLevel(game.level)/2;shot("01-hud");
 check(exact("PD")&&!contains("STRAŻNIK ŻARU")&&!exact("01")&&!exact("N"),"04/06/07: HUD omits region numbering, title and minimap N");
 check(exact("UMIEJĘTNOŚCI")&&exact("STATYSTYKI")&&!contains("UMIEJĘTNOŚCI ·")&&!contains("STATYSTYKI ·"),"11: HUD buttons have plain names");
 float xpY=0,attackY=0,armorY=0;for(auto& t:uiText){if(t.value=="PD")xpY=t.box.y;if(t.value.find("ATAK  ")==0)attackY=t.box.y;if(t.value.find("PANCERZ  ")==0)armorY=t.box.y;}
 check(xpY>0&&attackY>xpY+10&&armorY==attackY,"05: attack and armor are below the experience bar");
 Image hud=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&hud);int green=0;for(int y=310;y<319;y++)for(int x=1139;x<1225;x++){Color c=GetImageColor(hud,x,y);green+=std::abs(int(c.r)-38)<4&&std::abs(int(c.g)-91)<4&&std::abs(int(c.b)-56)<4;}UnloadImage(hud);
 check(green>300,"05: the GPU draws a dark green filled experience bar");
 check(!contains("POD KURSOREM")&&exact("Najedź na przeciwnika lub obiekt.")&&exact("Tutaj zobaczysz informacje o nim."),"09: empty inspection wording matches requested text");
 game.level=100000;game.xp=experienceForLevel(game.level)-1;shot("13-long-experience");
 const std::string longExperience=std::to_string(game.xp)+" / "+std::to_string(experienceForLevel(game.level));
 auto value=std::find_if(uiText.begin(),uiText.end(),[&](const UiText& t){return t.value==longExperience;});
 check(value!=uiText.end()&&value->box.x>1190&&value->box.x+value->box.width<=1401,"05: the largest supported experience count stays inside the HUD");
 reset();
 for(int k=1;k<ItemKindCount;k++){
  auto text=itemTooltipText(item(Kind(k)),{Place::Bag,10});auto box=tooltipBounds(text);
  check(text.instruction.empty()&&box.height<=148&&textWidth(text.description,14)<=box.width-34,"01: every compact item tooltip omits interaction hints and drop rates");
 }
 game.bag[10]=item(Kind::Shield);auto b=bagRect(10);mouse={b.x+25,b.y+20};shot("02-tooltip");
 check(!contains("Szansa na łup")&&!contains("LPM:"),"01: removed tooltip rows are absent from actual draw calls");
 reset();Enemy corpse;corpse.id=9911;corpse.type=1;corpse.alive=false;corpse.pos=game.player;corpse.loot={};corpse.loot[0]=item(Kind::Health,0,2);game.enemies.push_back(corpse);game.openedCorpse=corpse.id;shot("03-corpse");
 check(exact("WEŹ WSZYSTKO")&&!contains("2× LPM:"),"03: corpse loot keeps take-all without the single-item tip");
 reset();auto& chest=game.chests[0];chest.id=9912;chest.location=game.location;chest.pos=game.player;chest.opened=true;chest.loot[0]=item(Kind::Shield);game.openedChest=chest.id;shot("04-chest");
 check(exact("WEŹ WSZYSTKO")&&!contains("2× LPM:"),"03: chest loot keeps take-all without the single-item tip");
 reset();skillsOpen=true;shot("05-skills");check(exact("WRÓĆ DO GRY")&&!contains("U lub ESC")&&!contains("Z / X —"),"12: skills view omits both requested footer instructions");
 reset();statsOpen=true;shot("06-stats");check(exact("WRÓĆ DO GRY")&&!contains("ENTER / ESC / P"),"13: statistics view omits the shortcut footer");
 reset();paused=true;shot("07-pause");check(exact("CHWILA ODDECHU")&&!exact("Wielka Knieja"),"10: pause heading has no location subtitle");
 menuPageTo(MenuPage::Options);shot("08-options");check(!contains("Kliknij przełącznik"),"15: options omit the explanatory footer");
 bool before=audioSystem.musicEnabled();activateMenuRow(0);bool after=audioSystem.musicEnabled();activateMenuRow(0);check(before!=after&&audioSystem.musicEnabled()==before&&!audioSystem.available,"audio option still toggles without opening an audio device");
 menuPageTo(MenuPage::Controls);shot("09-controls");check(exact("STEROWANIE")&&!contains("Klawisze i mysz podczas wyprawy")&&exact("WSAD"),"16: controls keep bindings without their subtitle");
 reset();developerMenuOpen=true;shot("10-developer");check(exact("NOWA SESJA TESTOWA")&&exact("PRZENIEŚ OBECNĄ POSTAĆ")&&!contains("Poziom 100 ·")&&!contains("Nowa sesja resetuje"),"14: developer action buttons have no surrounding footnotes");
 reset();welcome=true;menuSelection=5;shot("11-main-menu");check(exact("Tryb deweloperski")&&!contains("Tryb deweloperski ·")&&!exact("06")&&!exact("01"),"04/17: main menu omits item numbering and the developer shortcut suffix");
 reset();mouse={skillsButton().x+20,skillsButton().y+10};check(clickProgression()&&skillsOpen,"plain skills button retains its real click action");skillsOpen=false;mouse={statsButton().x+20,statsButton().y+10};check(clickProgression()&&statsOpen,"plain stats button retains its real click action");
 reset();corpse.alive=true;corpse.hp=1;corpse.pos=game.player;game.enemies.push_back(corpse);
 game.hit(game.enemies.front(),100);shot("12-after-defeat");
 check(!game.enemies.front().alive&&game.messageTime==0&&!contains("Przeciwnik pokonany"),"08: a normal defeat leaves loot without an instruction banner");
 reset();RenderTexture2D sample=LoadRenderTexture(360,320),sheet=LoadRenderTexture(1200,1000);int index=0,page=0,rendered=0,clipped=0;
 BeginTextureMode(sheet);ClearBackground(Ink);EndTextureMode();
 std::ofstream catalog(root/"equipment-catalog.tsv");catalog<<"kind\tslot\tshape\tname\n";
 for(int k=1;k<ItemKindCount;k++){
  Kind kind=Kind(k);int slot=gearSlot(kind);if(slot<0)continue;
  game.equipment={};game.equipment[slot]=item(kind);game.swing=game.rollRemaining=game.bowReleaseTime=0;game.bowDrawing=false;
  for(int direction=0;direction<8;direction++)for(int mode=0;mode<3;mode++){
   game.animation={};game.animation.direction=Direction(direction);game.animation.phase=.23f;game.animation.motionInitialized=true;game.animation.moving=mode>0;game.animation.moveBlend=mode>0?1:0;game.animation.runBlend=mode==2?1:0;game.animation.swordStow=mode==2?1:0;
   actor_art::finishShadowFrame();BeginTextureMode(sample);ClearBackground(BLANK);hero(180,272,2.1f,false);EndTextureMode();
   Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);int solid=0,edge=0;
   for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++)if(pixels[y*im.width+x].a>220){solid++;edge+=x<2||x>im.width-3||y<2||y>im.height-3;}
   check(solid>1200&&edge==0,"02: equipped item remains visible and uncropped in idle, walk and run across eight directions");rendered++;clipped+=edge;
   Direction preview=archetype(kind)==Kind::Cloak?Direction::NW:Direction::SE;
   if(direction==int(preview)&&mode==0){
    int tile=index%20;float x=(tile%5)*240.f,y=(tile/5)*250.f;Texture2D texture=LoadTextureFromImage(im);
    BeginTextureMode(sheet);DrawTexturePro(texture,{0,0,360,320},{x,y,240,213.3f},{0,0},0,WHITE);icon(item(kind),x+34,y+45,.85f);center(itemName(item(kind)),x+120,y+227,11,Gold);EndTextureMode();UnloadTexture(texture);
   }
   UnloadImageColors(pixels);UnloadImage(im);
  }
  catalog<<k<<'\t'<<slot<<'\t'<<int(archetype(kind))<<'\t'<<itemName(item(kind))<<'\n';index++;
  if(index%20==0||index==EquipmentKindCount){saveTexture(sheet,(root/("equipment-page-"+std::to_string(++page)+".png")).string().c_str());BeginTextureMode(sheet);ClearBackground(Ink);EndTextureMode();}
 }
 check(index==EquipmentKindCount&&rendered==EquipmentKindCount*8*3&&clipped==0,"02: complete equipment catalogue covered; no clipped equipment");
 check(!audioSystem.available,"all diagnostics stay hidden and silent");
 UnloadRenderTexture(sample);UnloadRenderTexture(sheet);std::ofstream report(root/"report.json");report<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"equipment_items\":"<<index<<",\"equipment_renders\":"<<rendered<<",\"clipped_pixels\":"<<clipped<<",\"window_hidden\":true,\"audio_device_opened\":false}\n";
 audioSystem.shutdown();sync_present::shutdown();CloseWindow();std::cout<<"HUD/equipment: "<<checks-failures<<'/'<<checks<<" checks passed\n";return failures?2:0;
}
