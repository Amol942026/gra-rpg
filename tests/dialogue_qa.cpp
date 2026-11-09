#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include <iostream>
int main(int argc,char** argv){
 if(argc!=4||std::string(argv[1])!="--no-audio"||std::string(argv[3])!="--background-test")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_ALWAYS_RUN);sync_present::useOpenGL();InitWindow(W,H,"Hidden Emberfall dialogue QA");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 int checks=0,failures=0,totalLines=0;std::ofstream report(root/"checks.txt"),transcript(root/"dialogues.txt");
 auto check=[&](bool ok,const std::string& name){checks++;failures+=!ok;report<<(ok?"PASS ":"FAIL ")<<name<<'\n';report.flush();};
 auto shot=[&](const std::string& name){renderScene();saveTexture(canvas,(root/(name+".png")).string().c_str());};
 auto reset=[&](int parts,bool missingDesert=false,bool missingVolcano=false){
  game=Game{};game.collectedFragments=parts;game.dawnQuest=DawnQuest::AtTower;game.enterMageTower();game.player=MagePosition+V{1,1};game.desertBossDead=parts>=2||missingDesert;game.volcanoBossDead=parts>=3||missingVolcano;
  if(missingDesert){Enemy e{DesertBossId,DesertBossType,DesertBossHome,DesertBossHome,0,100};e.alive=false;e.loot[4]=item(Kind::DawnShard);game.desertState.enemies.push_back(e);}
  if(missingVolcano){Enemy e{VolcanoBossId,VolcanoBossType,VolcanoBossHome,VolcanoBossHome,0,100};e.alive=false;e.loot[4]=item(Kind::DawnShard);game.volcanoState.enemies.push_back(e);}
  intro={};towerStory={};clearTransientUi();welcome=paused=help=false;game.messageTime=0;mouse={0,0};
 };
 struct Case{const char* name;StoryDialogueView view;int parts,kind;bool volcano,missingDesert,missingVolcano;};
 const Case cases[]={
  {"intro",storyDialogue(IntroDialogue),0,0,false,false,false},
  {"forest-return",storyDialogue(TowerReturnDialogue),1,1,false,false,false},
  {"desert-route",storyDialogue(TowerTalkDialogue),1,2,false,false,false},
  {"desert-return",storyDialogue(DesertReturnDialogue),2,1,false,false,false},
  {"desert-revisit",storyDialogue(DesertCompletedTalk),2,2,false,false,false},
  {"volcano-return",storyDialogue(VolcanoReturnDialogue),3,1,true,false,false},
  {"volcano-route",storyDialogue(VolcanoTalkDialogue),2,2,true,false,false},
  {"after-three",storyDialogue(VolcanoCompletedTalk),3,2,true,false,false},
  {"desert-shard-pending",storyDialogue(DesertShardPendingTalk),1,2,true,true,false},
  {"volcano-shard-pending",storyDialogue(VolcanoShardPendingTalk),2,2,true,false,true},
  {"volcano-return-skipped-desert",storyDialogue(VolcanoReturnMissingDesertDialogue),3,1,true,true,false},
  {"skipped-desert-revisit",storyDialogue(VolcanoMissingDesertTalk),3,2,true,true,false}
 };
 check(IsWindowHidden()&&!audioSystem.available,"all actual renderer diagnostics remain hidden with no audio device");
 for(const auto& c:cases){
  reset(c.parts,c.missingDesert,c.missingVolcano);transcript<<"\n=== "<<c.name<<" ===\n";
  if(c.kind==0){game=Game{};intro.begin();intro.enter(IntroPhase::Dialogue);}
  else if(c.kind==1){towerStory.beginReturn(c.parts,c.missingDesert);towerStory.enter(TowerPhase::Dialogue);}
  else towerStory.beginTalk(true,c.parts,c.volcano,c.missingDesert,c.missingVolcano);
  check(c.kind==0||towerStory.dialogue().lines==c.view.lines,std::string(c.name)+" selects the intended current-game dialogue");
  for(size_t index=0;index<c.view.size;index++){
   const auto& line=c.view.lines[index];totalLines++;auto rows=storyRows(line.text,1140,21);float bottom=716+(rows.size()-1)*31.5f+21;
   check(bottom<=introNextButton().y-3,std::string(c.name)+" line "+std::to_string(index)+" fits above the actual next/reveal control");
   check(storyPrefix(line.text,storyGlyphCount(line.text))==line.text,std::string(c.name)+" preserves all UTF-8 characters");
   transcript<<(line.mage?"Eldrin: ":"Wędrowiec: ")<<line.text<<"\n";
   if(c.kind==0){intro.line=index;intro.revealed=true;}else {towerStory.line=index;towerStory.revealed=true;}
   shot(std::string(c.name)+"-"+std::to_string(index));
  }
 }
 for(int parts:{1,2,3}){
  reset(parts);check(talkToMage(),"actual friendly-NPC callback starts the conversation");
  if(parts==3)check(towerStory.dialogue().lines==VolcanoCompletedTalk.data()&&towerStory.current().mage,"repeat NPC greeting after Varkhor does not replay the hero's victory announcement");
  float age=towerStory.age;towerAction(true,false,.05f,false);check(towerStory.age==age&&towerStory.line==0,"unfocused dialogue ignores input and pauses its clock");
  for(int frame=0;frame<100&&towerStory.phase==TowerPhase::Dialogue;frame++)towerAction(true,false,.2f,true);
  check(towerStory.phase==TowerPhase::Choice,"real dialogue advances to travel choices");
  auto rows=storyRows(mageTravelDescription(game,towerStory.offerVolcano),680,20);check(rows.size()<=3,"contextual travel description fits above fragment count and buttons");
  shot("travel-choice-"+std::to_string(parts));V before=game.player;Rectangle stay=desertStayButton();mouse={stay.x+10,stay.y+10};
  check(clickMageTravelChoice()&&!towerStory.active()&&distance(before,game.player)==0,"stay choice returns to tower control without teleporting");
 }
 reset(3);talkToMage();for(int frame=0;frame<100&&towerStory.phase==TowerPhase::Dialogue;frame++)towerAction(true,false,.2f,true);
 Rectangle button=desertTravelButton();mouse={button.x+20,button.y+20};check(clickMageTravelChoice(),"real travel choice starts the selected route");
 for(int frame=0;frame<180;frame++)towerAction(false,false,1.f/60,true);
 check(game.inDesert()&&!towerStory.active(),"travel after repeat dialogue still reaches the selected realm");
 for(int parts:{1,2,3}){
  reset(parts);for(int page=0;page<3;page++){
   check(storyRows(towerChronicleText(page,game),743,19).size()<=6,"chronicle page fits its actual text area at each milestone");
  }
 }
 reset(3);for(int floor=0;floor<TowerFloorCount;floor++){
  game.towerFloor=floor;const auto& objects=towerFurnishings(floor);
  for(int index=0;index<int(objects.size());index++)if(objects[index].type==6||objects[index].type==10||objects[index].type==12){
   towerPanelOpen=true;towerPanelObject=index;for(int page=0;page<3;page++){towerPage=page;shot("lore-"+std::to_string(floor)+"-"+std::to_string(objects[index].type)+"-"+std::to_string(page));}towerPanelOpen=false;
  }
 }
 for(float scale:{.8f,1.5f}){
  SetWindowSize(int(W*scale),int(H*scale));prepareCanvas(scale);reset(3);towerStory.beginTalk(true,3,true);towerStory.revealed=true;shot("repeat-scale-"+std::to_string(int(scale*100)));check(IsWindowHidden(),"resizing the diagnostic never restores or focuses its window");
 }
 check(totalLines==57,"all current-game dialogue turns were rendered and checked");
 std::ofstream summary(root/"summary.json");summary<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"conversations\":12,\"rendered_lines\":"<<totalLines<<",\"hidden\":true,\"audio_opened\":false,\"source\":\"current-game\",\"native_articy_modified\":false}";
 std::cout<<"Dialogue QA: "<<checks<<" checks, "<<failures<<" failures\n";audioSystem.shutdown();CloseWindow();return failures?2:0;
}
