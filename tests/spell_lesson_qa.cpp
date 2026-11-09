// Real player UI, Eldrin conversation choices, save and elemental renderer.
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#include <iostream>
#undef main
int main(int argc,char** argv){
 if(argc!=4||std::string(argv[1])!="--no-audio"||std::string(argv[3])!="--background-test")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 std::ofstream report(root/"checks.txt");int checks=0,failures=0;auto check=[&](bool ok,const char* text){checks++;failures+=!ok;report<<(ok?"PASS ":"FAIL ")<<text<<'\n';report.flush();};
 auto point=[](Rectangle r){mouse={r.x+r.width*.5f,r.y+r.height*.5f};};
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall elemental lessons silent QA");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 check(!IsAudioDeviceReady()&&IsWindowHidden(),"player QA stays hidden with audio hardware closed");
 check(spell_art::atlas.id&&spell_art::atlas.width==768&&spell_art::atlas.height==768,"all elemental graphics preload once in the real player renderer");
 game=Game{};intro={};towerStory={};clearStoryMenus();game.enemies.clear();game.props.clear();game.player={15,15};mouse={0,0};
 auto capture=[&](const std::string& name){renderScene();saveTexture(canvas,(root/(name+".png")).string().c_str());};
 openSpellBook();capture("book-locked");
 for(int i=0;i<3;i++){point(spellConfirm(i));clickProgression();check(game.learnedSpells==0&&spellBookOpen&&game.fireballs.empty(),"normal spell book cannot grant an unknown spell for free");}
 point(spellBookClose());clickProgression();check(!castGameplaySpell()&&game.mana==100,"C cannot cast before any lesson has been bought");
 game=Game{};clearStoryMenus();game.bossDead=true;game.collectedFragments=1;game.enterMageTower();game.dawnQuest=DawnQuest::AtTower;game.player=MagePosition+V{-1.4f,0};game.gold=6000;
 check(talkToMage(),"E / NPC talk handler starts the real Eldrin conversation after the first guardian");
 for(int n=0;n<200&&towerStory.phase!=TowerPhase::Choice;n++)towerAction(true,false,.1f);
 check(towerStory.phase==TowerPhase::Choice,"existing fantasy dialogue reaches the travel and lessons choice");capture("eldrin-choice");
 auto travel=desertTravelButton(),lesson=mageLessonButton(),stay=desertStayButton();check(travel.y+travel.height<lesson.y&&lesson.y+lesson.height<stay.y,"new lesson choice has spacing and does not overlap travel or stay");
 point(mageLessonButton());check(clickMageTravelChoice()&&spellBookOpen&&spellLessonOpen&&!towerStory.active(),"Eldrin choice opens the paid lessons panel and closes dialogue cleanly");capture("eldrin-lessons");
 game.gold=599;point(spellConfirm(0));clickProgression();check(game.gold==599&&!game.knowsSpell(Spell::Fireball)&&!spellBookNotice.empty(),"insufficient payment remains rejected through the actual purchase button");capture("not-enough-gold");
 game.gold=6000;point(spellConfirm(0));clickProgression();check(game.knowsSpell(Spell::Fireball)&&game.gold==5400&&spellBookOpen,"paid fire lesson charges exactly its displayed price");
 point(spellConfirm(0));clickProgression();check(!spellBookOpen&&game.gold==5400&&game.selectedSpell==Spell::Fireball,"clicking a known spell equips it without charging twice");
 openSpellLessons();point(spellConfirm(1));clickProgression();point(spellConfirm(2));clickProgression();
 check(game.learnedSpells==7&&game.gold==900&&game.selectedSpell==Spell::LightningOrb,"ice and lightning buttons charge their displayed prices and remember independent knowledge");capture("all-elements-learned");
 ember::saves::Snapshot saved{game,{},{},"2026-11-09"};auto bytes=ember::saves::encode(saved);auto restored=ember::saves::decode(bytes);
 check(restored.game.learnedSpells==7&&restored.game.gold==900&&restored.game.selectedSpell==Spell::LightningOrb,"real UI purchases and balance survive a complete save/reload");
 point(spellBookClose());clickProgression();game=std::move(restored.game);game.activateField(WorldLocation::Forest);game.player={15,15};game.enemies.clear();game.props.clear();game.messageTime=0;
 const char* names[]={"fire","ice","lightning"};
 for(int i=0;i<3;i++){
  Spell spell=Spell(i+1);game.enemies.clear();game.fireballs.clear();game.explosions.clear();game.mana=100;game.spellCd=0;game.chooseSpell(spell);game.enemies.push_back({810,0,{18,15},{18,15},1000,1000});game.enemies[0].windup=100;
  Vector2 p=screen(game.enemies[0].pos);mouse={p.x,p.y-18};check(castGameplaySpell()&&game.fireballs.size()==1&&game.fireballs[0].spell==spell,"actual cast input selects the bought element and aims at a visible target");
  game.update(.05f,{},false);mouse={0,0};capture(std::string(names[i])+"-flight-a");game.update(.05f,{},false);capture(std::string(names[i])+"-flight-b");
  for(int frame=0;frame<100&&!game.fireballs.empty();frame++)game.update(.02f,{},false);
  check(game.enemies[0].hp<1000&&game.fireballs.empty()&&!game.explosions.empty()&&game.explosions.back().spell==spell,"rendered projectile impacts with its own elemental burst");capture(std::string(names[i])+"-impact");
  openSpellBook();capture(std::string(names[i])+"-selected-book");point(spellBookClose());clickProgression();
 }
 check(!IsAudioDeviceReady(),"no graphical or transaction diagnostic initializes audio");
 spell_art::unload();inventory_icons::unload();audioSystem.shutdown();sync_present::shutdown();CloseWindow();
 std::ofstream summary(root/"summary.json");summary<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"window_hidden\":true,\"audio_device_opened\":false,\"audio_played\":false,\"spells\":3}";
 std::cout<<"Native spell lessons: "<<checks<<" checks, "<<failures<<" failures\n";return failures?2:0;
}
