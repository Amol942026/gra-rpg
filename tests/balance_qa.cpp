// Real player UI, finite merchant supply, XP/save migration and Krita corpse.
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#include <iostream>
#undef main
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root/"corpse-views");
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall balance --no-audio");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 int checks=0,failures=0;std::ofstream log(root/"checks.txt");auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;log<<(ok?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 auto reset=[](){game=Game{};intro={};towerStory={};clearTransientUi();welcome=help=paused=false;mouse={0,0};};
 auto point=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
 auto shot=[&](const char* name){renderScene();saveTexture(canvas,(root/(std::string(name)+".png")).string());};
 check(PlayerBuild&&!audioSystem.available,"player code and all diagnostics keep the audio device closed");
 check(runProgressionQA(),"existing real stat allocation and spell UI regression suite");
 check(runSkillsQA(),"existing real skill unlocking, refund and confirmation regression suite");prepareCanvas(1);
 reset();openStats();mouse={0,0};shot("stats-readonly");
 unsigned frozenBuilds=pausedWorldCache.builds;renderScene();
 check(pausedWorldCache.ready&&pausedWorldCache.builds==frozenBuilds,"paused stat menu reuses one native-resolution world image");
 check(!worldCanvas.id&&pausedWorldCache.art.texture.width==canvas.texture.width&&pausedWorldCache.art.texture.height==canvas.texture.height,"menu cache preserves native pixels without resampling the moving world");
 Image im=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);int edits=0;
 for(int row=0;row<AttributeCount;row++)for(int y=274+row*56;y<308+row*56;y++)for(int x=940;x<1110;x++){Color c=pixels[y*im.width+x];edits+=c.r!=Panel.r||c.g!=Panel.g||c.b!=Panel.b;}
 check(edits==0,"stat sheet has no adjustment buttons or symbols when there are no points");UnloadImageColors(pixels);UnloadImage(im);
 point(statAdjust(0,true));clickProgression();check(statDraft==Attributes{}&&game.attributes==Attributes{},"read-only stat clicks cannot create or spend points");confirmStats();openSkills();mouse={0,0};shot("skills-readonly");
 im=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&im);pixels=LoadImageColors(im);edits=0;
 for(int row=0;row<SkillCount;row++)for(int y=313+row*133;y<341+row*133;y++)for(int x=986;x<1118;x++){Color c=pixels[y*im.width+x];edits+=c.r!=Panel.r||c.g!=Panel.g||c.b!=Panel.b;}
 check(edits==0,"skill sheet has no disabled plus/minus or editing controls without points");UnloadImageColors(pixels);UnloadImage(im);
 point(skillAdjust(0,true));clickSkills();check(skillDraft==SkillRanks{}&&game.skillRanks==SkillRanks{},"read-only skill clicks leave every skill locked");
 reset();game.xp=experienceForLevel(1)-1;Enemy victim{900,0,game.player,game.player,1,1};game.hit(victim,100);syncAdvancement();mouse={0,0};shot("stats-allocation");
 point(statAdjust(0,true));clickProgression();check(statDraft[0]==1&&game.attributes[0]==0,"DODAJ previews a point without training it");point(statAdjust(0,false));clickProgression();check(statDraft[0]==0,"COFNIJ returns only a pending point");
 for(int n=0;n<5;n++){point(statAdjust(0,true));clickProgression();}point(statsConfirm());clickProgression();mouse={0,0};shot("skills-allocation");
 point(skillCard(0));clickSkills();mouse={0,0};shot("skills-selected");point(skillAdjust(0,false));clickSkills();check(skillDraftRemaining()==1,"text refund action cancels a skill selection");
 point(skillCard(1));clickSkills();point(skillsConfirm());clickSkills();check(!game.allocationRequired&&game.skillRank(Skill::PiercingArrow)==1&&game.skillRank(Skill::StrongCut)==0,"only explicit confirmation learns the selected skill");
 // Every purchase entry point shares one stock, including the old MAX button.
 reset();game.enterMageTower();game.dawnQuest=DawnQuest::AtTower;game.towerFloor=MerchantFloor;game.player=MerchantPosition+V{-1,0};game.gold=1000;nearestTowerInteraction();
 point(merchantOfferRect(2));clickMerchantPanel(10);point(merchantMaxRect());clickMerchantPanel(10.1);check(merchantQuantity==20,"MAX caps arrow purchases at the actual shared stock of twenty");mouse={0,0};shot("arrows-full-stock");
 point(merchantTradeRect());clickMerchantPanel(10.2);check(game.merchantArrows==0&&game.ammoCount()==40,"MAX buys exactly twenty extra arrows and exhausts merchant supply");
 auto gold=game.goldBalance();int arrows=game.ammoCount();point(merchantOfferRect(2));clickMerchantPanel(11);clickMerchantPanel(11.1);
 check(game.goldBalance()==gold&&game.ammoCount()==arrows&&selectedMerchantStatus()==TradeResult::OutOfStock,"double-click cannot bypass exhausted stock or spend gold");mouse={0,0};shot("arrows-out-of-stock");
 closeTowerPanel();nearestTowerInteraction();check(game.merchantArrows==0,"reopening merchant preserves exhausted stock");
 saves::Snapshot snap;snap.game=game;saves::Store store(fs::absolute(root/"saves"));store.save(0,snap);game=store.load(0).game;
 check(game.merchantArrows==0,"on-disk save/load cannot reset arrow stock");closeTowerPanel();
 // Real death event, mandatory advancement, corpse hover/loot, new eight views.
 game.activateField(WorldLocation::Desert);game.player=DesertBossHome+V{2,1};auto* boss=game.findEnemy(DesertBossId);game.hit(*boss,10000);
 check(game.desertBossDead&&game.merchantArrows==20&&!boss->alive,"second boss death triggers exactly one replenishment event");
 if(game.attributePoints){Attributes points{};points[0]=game.attributePoints;check(game.distribute(points),"boss XP attributes can be confirmed under the slower curve");}
 if(game.skillAllocationPending()){SkillRanks points{};points[0]=game.skillPoints;check(game.trainSkills(points),"boss XP skill point remains gated until confirmation");}
 clearTransientUi();game.effects.clear();game.messageTime=0;mouse={0,0};shot("dune-fallen-world");
 Vector2 p=screen(boss->pos);bool picked=false;
 for(int y=-60;y<38&&!picked;y+=3)for(int x=-86;x<86&&!picked;x+=3){mouse={p.x+x,p.y+y};auto hit=pickWorld(mouse);picked=hit.kind==HoverKind::Corpse&&hit.index==DesertBossId;}
 check(picked&&rightClickWorldOrInventory()&&game.openedCorpse==DesertBossId,"new fallen art remains selectable through real PPM corpse looting");shot("dune-fallen-loot");game.closeCorpse();mouse={0,0};
 for(int direction=0;direction<8;direction++){
  const auto& sprite=fallen_art::sprites[DesertBossType][direction];im=LoadImageFromTexture(sprite.body);pixels=LoadImageColors(im);int solid=0,border=0,badAlpha=0,mismatch=0;
  for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++){
   Color c=pixels[y*im.width+x];solid+=c.a>=128;border+=c.a>32&&(x<4||y<4||x>=im.width-4||y>=im.height-4);badAlpha+=c.r>c.a||c.g>c.a||c.b>c.a;
   if(x%11==0&&y%11==0){Vector2 at{p.x+(x+.5f)/2-fallen_art::OriginX,p.y+(y+.5f)/2-fallen_art::OriginY};Enemy sample=*boss;sample.animation.direction=Direction(direction);mismatch+=corpseArt(sample,p.x,p.y,BLANK,&at)!=(c.a>=128);}
  }
  check(im.width==384&&im.height==256&&solid>4000&&border==0,"Krita corpse is complete in facing "+std::to_string(direction));check(badAlpha==0&&mismatch==0,"premultiplied edges and the cursor mask match the new corpse pixels");
  check(ExportImage(im,(root/"corpse-views"/(std::to_string(direction)+".png")).string().c_str()),"new runtime corpse texture exported");UnloadImageColors(pixels);UnloadImage(im);
 }
 snap.game=game;store.save(1,snap);auto restored=store.load(1);check(restored.game.desertBossDead&&restored.game.merchantArrows==20&&restored.game.findCorpse(DesertBossId),"boss death, remains and replenished arrows survive a real saved game");
 game.enterMageTower();game.dawnQuest=DawnQuest::AtTower;game.towerFloor=MerchantFloor;game.player=MerchantPosition+V{-1,0};nearestTowerInteraction();point(merchantOfferRect(2));clickMerchantPanel(20);clickMerchantPanel(20.1);
 check(game.merchantArrows==19&&game.ammoCount()==arrows+1,"the refreshed supply supports double-click purchases after returning to the tower");mouse={0,0};shot("arrows-restocked");
 check(!audioSystem.available,"all UI, death, loot and trade checks remained silent");log<<"Checks: "<<checks<<"\nFailures: "<<failures<<'\n';std::cout<<"Balance QA: "<<checks<<" checks, "<<failures<<" failures\n";
 CloseWindow();return failures?2:0;
}
