#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include <iostream>
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root/"hero");
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall sheath and attributes --no-audio");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 int checks=0,failures=0;std::ofstream log(root/"checks.txt"),tips(root/"tooltips.tsv");tips<<"scale\tattribute\tx\ty\twidth\theight\trows\n";
 auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;log<<(ok?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 auto reset=[](){game=Game{};intro={};towerStory={};clearTransientUi();welcome=help=paused=false;mouse={0,0};game.messageTime=0;};
 auto point=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
 auto allocation=[&](int points=5){reset();game.level=2;game.attributePoints=points;game.skillPoints=1;game.allocationRequired=true;openStats();};
 auto shot=[&](const std::string& name){size_t atlases=typography.atlasCount();renderScene();check(typography.atlasCount()==atlases,"UI uses preloaded fonts: "+name);saveTexture(canvas,(root/(name+".png")).string());};
 check(PlayerBuild&&!audioSystem.available,"real player renderer keeps audio closed");
 for(int i=0;i<AttributeCount;i++){
  allocation();Attributes before=game.attributes;int base=attributePreviewValue(i);point(statAddFive(i));clickProgression();
  check(statDraft[i]==5&&draftRemaining()==0&&game.attributes==before&&game.attributePoints==5,"DODAJ 5 previews exactly five points without committing: "+std::to_string(i));
  point(statAddFive((i+1)%AttributeCount));clickProgression();check(statDraft[i]==5&&statDraft[(i+1)%AttributeCount]==0,"bulk cannot overspend the earned pool");
  point(statAdjust(i,false));clickProgression();check(statDraft[i]==4&&draftRemaining()==1,"COFNIJ refunds one point from a bulk preview");
  point(statAddFive(i));clickProgression();check(statDraft[i]==4&&draftRemaining()==1,"bulk cannot spend five when only one remains");
  point(statAdjust(i,true));clickProgression();int expected=base+5*(i==0?2:i==1?3:i==4?10:i==5?5:1);
  check(attributePreviewValue(i)==expected,"hover preview matches the precise attribute formula");
  point(statsConfirm());clickProgression();check(game.attributes[i]==5&&game.attributePoints==0&&skillsOpen&&game.skillRanks==SkillRanks{},"confirmation applies bulk and keeps skills unlearned until their own confirmation");
  point(skillCard(0));clickSkills();point(skillsConfirm());clickSkills();
  check(!game.allocationRequired&&game.skillRank(Skill::StrongCut)==1,"normal skill confirmation resumes gameplay after bulk attributes");
 }
 allocation(10);point(statAddFive(0));clickProgression();point(statAddFive(0));clickProgression();check(statDraft[0]==10&&draftRemaining()==0,"two earned levels allow two separate batches of five");
 allocation(4);point(statAddFive(0));clickProgression();check(statDraft==Attributes{}&&draftRemaining()==4,"incomplete pool cannot make an unauthorized five-point batch");
 check(!adjustStat(0,true,0)&&!adjustStat(0,true,-5)&&!adjustStat(-1,true,5)&&!adjustStat(AttributeCount,true,5),"invalid bulk requests cannot change the draft");
 reset();openStats();point(statAddFive(0));clickProgression();check(statDraft==Attributes{}&&game.attributes==Attributes{},"read-only character sheet cannot allocate unearned points");
 for(float factor:{.8f,1.f,1.2f}){
  prepareCanvas(factor);allocation();mouse={0,0};shot("allocation-"+std::to_string(int(factor*100)));
  for(int i=0;i<AttributeCount;i++){
   for(Rectangle button:{statAdjust(i,false),statAdjust(i,true),statAddFive(i)})check(button.x>=878&&button.x+button.width<=1112&&button.height<=36,"all controls fit inside their attribute row");
   check(!CheckCollisionRecs(statAdjust(i,false),statAdjust(i,true))&&!CheckCollisionRecs(statAdjust(i,true),statAddFive(i)),"bulk, single and undo hit targets do not overlap");
   point(statRow(i));check(hoveredAttribute()==i,"hover identifies the real attribute row");Rectangle box=attributeTooltipBounds(i);auto& lines=attributeDetailRows(i);bool fits=textWidth(AttributeHints[i].effect,15)<=box.width-32&&textWidth(AttributeHints[i].five,15)<=box.width-32;
   for(const auto& row:lines)fits&=textWidth(row,16)<=box.width-32;
   check(fits&&box.x>=8&&box.y>=8&&box.x+box.width<=W-12&&box.y+box.height<=H-12,"whole attribute explanation fits its cursor tooltip and screen");
   check(!CheckCollisionPointRec(mouse,box),"attribute tooltip stays away from its cursor");
   tips<<factor<<'\t'<<i<<'\t'<<box.x<<'\t'<<box.y<<'\t'<<box.width<<'\t'<<box.height<<'\t'<<lines.size()<<'\n';
   shot("tooltip-"+std::to_string(i)+"-"+std::to_string(int(factor*100)));
   Image im=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&im);Color stripe=GetImageColor(im,int((box.x+1)*renderScale),int((box.y+box.height-8)*renderScale));check(stripe.r==Gold.r&&stripe.g==Gold.g&&stripe.b==Gold.b,"cursor tooltip is drawn on top of the HUD, including its bottom edge");UnloadImage(im);
  }
  for(int i=0;i<AttributeCount;i++){point(statAddFive(i));Rectangle box=attributeTooltipBounds(i);check(box.x>=8&&box.x+box.width<=W-12,"hover near right-side batch button keeps tooltip on screen");}
  reset();openStats();mouse={0,0};shot("readonly-"+std::to_string(int(factor*100)));
 }
 prepareCanvas(1);allocation();point(statAddFive(0));clickProgression();point(statRow(0));shot("five-strength-preview");
 check(runProgressionQA(),"existing single-point, undo, spell and advancement UI remains functional");
 reset();game.equipment[0]=item(Kind::Sword);game.equipment[2]={};RenderTexture2D sample=LoadRenderTexture(480,440);
 int heroFrames=0;
 for(int direction=0;direction<8;direction++)for(int pose=0;pose<4;pose++){
  AnimationState state;state.direction=Direction(direction);state.phase=.1875f;state.motionInitialized=true;state.moving=pose>0;state.moveBlend=pose>0?1.f:0.f;state.runBlend=pose>=2?1.f:0.f;state.swordStow=pose==2?1.f:pose==3?.65f:0;
  actor_art::finishShadowFrame();BeginTextureMode(sample);ClearBackground(BLANK);actor_art::draw(0,240,340,3,state,0,false,false,Kind::Sword,false,BLANK,nullptr,-1,-1,-1,20,false,&game.equipment);EndTextureMode();
  Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);Color* px=LoadImageColors(im);int solid=0,border=0;for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++)if(px[y*im.width+x].a>32){solid++;border+=x<2||y<2||x>=im.width-2||y>=im.height-2;}
  check(solid>1700&&border==0,"complete closer scabbard and sword visible in each facing and gait");check(ExportImage(im,(root/"hero"/(std::to_string(direction)+"-"+std::to_string(pose)+".png")).string().c_str()),"hero close-up exported");UnloadImageColors(px);UnloadImage(im);heroFrames++;
 }
 check(sword_stow::Mouth.r==-16&&sword_stow::Mouth.f==13&&sword_stow::Mouth.z==36,"actual renderer and grip use the closer Synfig scabbard socket");
 check(!audioSystem.available&&audioSystem.playedEffects==0,"all diagnostics remained silent");
 log<<"Checks: "<<checks<<"\nFailures: "<<failures<<'\n';std::ofstream out(root/"summary.json");out<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"hero_views\":"<<heroFrames<<",\"attribute_tooltips\":18,\"audio_opened\":false}";
 std::cout<<"Sheath/attributes: "<<checks<<" checks, "<<failures<<" failures\n";CloseWindow();return failures?2:0;
}
