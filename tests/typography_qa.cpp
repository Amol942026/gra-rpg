// Actual player renderer: native glyph coverage, Polish text and UI at 3 sizes.
#include "raylib.h"
#include <string>
#include <vector>
struct TextProbe {std::string text;Rectangle box;};
std::vector<TextProbe> textProbes;
bool probeText=false;
void probeDrawTextEx(Font,const char*,Vector2,float,float,Color);
#define DrawTextEx probeDrawTextEx
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#undef DrawTextEx
#include <iostream>
void probeDrawTextEx(Font f,const char* text,Vector2 p,float size,float spacing,Color color){
 if(probeText&&*text){Vector2 extent=MeasureTextEx(f,text,size,spacing);textProbes.push_back({text,{p.x,p.y,extent.x,extent.y}});}
 DrawTextEx(f,text,p,size,spacing,color);
}
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall typography --no-audio");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 int checks=0,failures=0,scenes=0;std::ofstream log(root/"checks.txt"),bounds(root/"text-bounds.tsv"),coverage(root/"coverage.tsv");
 bounds<<"scene\tscale\tx\ty\twidth\theight\ttext\n";coverage<<"scale\trole\told_coverage\tnew_coverage\tratio\n";
 auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;log<<(ok?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 auto reset=[](){game=Game{};intro={};towerStory={};clearTransientUi();invalidatePausedWorld();welcome=help=paused=false;mouse={0,0};menuPageTo(MenuPage::Home);game.messageTime=0;menuClock=2;};
 auto shot=[&](const char* name){
  size_t atlases=typography.atlasCount();textProbes.clear();probeText=true;renderScene();probeText=false;scenes++;
  check(typography.atlasCount()==atlases,std::string(name)+": no font upload during rendering");
  std::string id=std::string(name)+"-"+std::to_string(int(std::round(renderScale*100)));
  saveTexture(canvas,(root/(id+".png")).string());
  for(const auto& text:textProbes){bounds<<name<<'\t'<<renderScale<<'\t'<<text.box.x<<'\t'<<text.box.y<<'\t'<<text.box.width<<'\t'<<text.box.height<<'\t'<<text.text<<'\n';}
  check(canvas.texture.width==int(std::round(W*renderScale)),id+": native framebuffer pixels");
 };
 auto point=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
 menuSaveRoot=fs::absolute(root/"saves");menuStore().save(0,{game,intro,towerStory,{}});
 check(PlayerBuild&&!audioSystem.available,"player code; audio device remains closed");
 // Compare actual GPU glyph coverage to the old faces. No synthetic outline.
 const char* phrase="Zażółć gęślą jaźń · Światło ruin 0123456789";
 for(float factor:{.8f,1.f,1.2f}){
  prepareCanvas(factor);
  RenderTexture2D board=LoadRenderTexture(canvas.texture.width,canvas.texture.height);
  BeginTextureMode(board);ClearBackground(Panel);EndTextureMode();
  for(int role=0;role<6;role++){
   bool title=role>=3;float size=role==0?14:role==1?18:role==2?21:role==3?15:role==4?28:39;
   int pixels=std::max(10,int(std::round(size*renderScale)));std::vector<int> codes;for(int c=32;c<384;c++)codes.push_back(c);
   Font old=LoadFontEx(title?"assets/Cinzel.ttf":size<=16?"assets/Lato-Bold.ttf":"assets/Lato-Regular.ttf",pixels,codes.data(),int(codes.size()));SetTextureFilter(old.texture,TEXTURE_FILTER_POINT);
   RenderTexture2D sample=LoadRenderTexture(canvas.texture.width,100);double totals[2]{};
   for(int variant=0;variant<2;variant++){
    BeginTextureMode(sample);ClearBackground(BLANK);BeginMode2D({{0,0},{0,0},0,renderScale});
    if(variant)txt(phrase,8,8,size,WHITE,title);else DrawTextEx(old,phrase,{8,8},old.baseSize/renderScale,title?1.f/renderScale:0,WHITE);
    EndMode2D();EndTextureMode();Image im=LoadImageFromTexture(sample.texture);Color* px=LoadImageColors(im);
    for(int i=0;i<im.width*im.height;i++)totals[variant]+=px[i].a/255.;UnloadImageColors(px);UnloadImage(im);
    BeginTextureMode(board);DrawTexturePro(sample.texture,{0,0,float(sample.texture.width),-float(sample.texture.height)},{0,float(role*145+variant*66)*renderScale,float(W),100.f},{0,0},0,WHITE);EndTextureMode();
   }
   coverage<<renderScale<<'\t'<<role<<'\t'<<totals[0]<<'\t'<<totals[1]<<'\t'<<totals[1]/totals[0]<<'\n';
   check(totals[0]>100&&totals[1]>totals[0]*1.08,"true heavier glyph coverage at size "+std::to_string(int(size))+" scale "+std::to_string(factor));
   UnloadRenderTexture(sample);UnloadFont(old);
  }
  saveTexture(board,(root/("weight-comparison-"+std::to_string(int(factor*100))+".png")).string());UnloadRenderTexture(board);
  // Complete conversations must still fit above the bottom row of controls.
  auto dialogue=[&](const auto& lines){for(const auto& line:lines){auto rows=storyRows(line.text,1140,21);bool fits=rows.size()<=3;for(const auto& row:rows)fits&=textWidth(row,21)<=1140;check(fits,"full dialogue turn fits its actual text region");}};
  dialogue(IntroDialogue);dialogue(TowerReturnDialogue);dialogue(TowerTalkDialogue);dialogue(DesertReturnDialogue);dialogue(DesertCompletedTalk);
  reset();welcome=true;shot("main-menu");menuPageTo(MenuPage::Options);shot("options");menuPageTo(MenuPage::Credits);shot("credits");menuPageTo(MenuPage::Load);refreshMenuSaves();shot("load-menu");
  reset();shot("forest-hud");openStats();shot("stats");reset();openSkills();shot("skills");reset();openSpellBook();shot("spell-book");
  reset();game.attributePoints=5;game.skillPoints=1;game.allocationRequired=true;openStats();shot("stats-allocation");game.attributePoints=0;statsOpen=false;openSkills();shot("skills-allocation");
  reset();game.bag[10]=item(Kind::MoonBow);point(bagRect(10));shot("item-tooltip");
  reset();help=true;shot("help");reset();openPauseMenu();shot("pause");reset();audioOpen=true;shot("audio");reset();game.hp=0;shot("death");
  reset();startIntro();intro.phase=IntroPhase::Dialogue;intro.line=6;intro.revealed=true;shot("dialogue");
  reset();game.enterMageTower();game.dawnQuest=DawnQuest::AtTower;game.towerFloor=MerchantFloor;game.player=MerchantPosition+V{-1,0};nearestTowerInteraction();shot("merchant");point(merchantOfferRect(0));shot("merchant-tooltip");
  reset();game.enterMageTower();game.towerFloor=0;game.player=MagePosition+V{2,0};shot("tower-labels");
 }
 probeText=false;
 check(runRarityQA(),"all item names, descriptions and values fit production tooltips");
 check(runTradeQA(),"actual merchant UI and every item value remain functional");
 check(runProgressionQA(),"stat allocation and spell UI regression");
 check(runSkillsQA(),"actual skill confirmation and input regression");
 check(runCriticalHealingQA(),"combat text uses preloaded atlases at every scale");
 check(!audioSystem.available&&!developerActive&&!developerMenuOpen,"no audio or developer features enabled by typography");
 log<<"Checks: "<<checks<<"\nFailures: "<<failures<<"\nScenes: "<<scenes<<'\n';
 std::ofstream summary(root/"summary.json");summary<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"scenes\":"<<scenes<<",\"audio_opened\":false}";
 std::cout<<"Typography: "<<checks<<" checks, "<<failures<<" failures; "<<scenes<<" scenes\n";
 CloseWindow();return failures?2:0;
}
