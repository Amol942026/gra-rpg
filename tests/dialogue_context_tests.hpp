#pragma once
#include "../src/story_context.hpp"
void dialogueContextTests(){
 Game game;check(game.recoveredDawnFragments()==0,"new adventure does not present uncollected fragments as recovered");
 game.player=ForestGuardian;auto* stone=game.findEnemy(13);game.hit(*stone,10000);game.allocationRequired=false;game.openedCorpse=13;
 check(game.dawnShardUnclaimed(WorldLocation::Forest)&&game.recoveredDawnFragments()==0,"defeating the first guardian leaves its story shard unclaimed");
 check(game.transfer({Place::Corpse,4,13},{Place::Bag,8}),"collect the real first fragment for dialogue progression");
 game.takeTowerReturn();game.enterMageTower();game.finishTowerReturn();game.player=MagePosition;check(game.travelToDesert(),"first fragment still unlocks the existing desert travel gate");
 auto* dune=game.findEnemy(DesertBossId);game.player=dune->pos;game.hit(*dune,10000);game.allocationRequired=false;
 check(game.dawnShardUnclaimed(WorldLocation::Desert)&&game.recoveredDawnFragments()==1,"a defeated second guardian does not imply that its shard was taken");
 game.player=DesertPortal;check(game.useDesertPortal(),"return through the actual desert portal without taking its fragment");game.player=MagePosition;
 check(game.canTravelVolcano(),"dialogue corrections preserve volcanic travel unlocked by guardian defeat");
 TowerSequence scene;scene.beginTalk(true,game.dawnFragments(),true,game.dawnShardUnclaimed(WorldLocation::Desert));
 check(scene.dialogue().lines==DesertShardPendingTalk.data(),"Eldrin identifies the missing desert fragment rather than claiming two pieces");
 check(game.travelToVolcano()&&game.dawnShardUnclaimed(WorldLocation::Desert),"missing-fragment context survives switching to a different field");
 auto* varkhor=game.findEnemy(VolcanoBossId);game.player=varkhor->pos;game.hit(*varkhor,10000);game.allocationRequired=false;game.openedCorpse=VolcanoBossId;
 check(game.transfer({Place::Corpse,4,VolcanoBossId},{Place::Bag,9}),"collect Varkhor's actual fragment before returning for the desert shard");
 check(game.dawnFragments()==3&&game.recoveredDawnFragments()==2&&!game.recoveredDawnPart(1)&&game.recoveredDawnPart(2),"read-only narrative count distinguishes the third milestone from two physically recovered pieces");
 scene.beginReturn(game.dawnFragments(),game.dawnShardUnclaimed(WorldLocation::Desert));
 check(scene.dialogue().lines==VolcanoReturnMissingDesertDialogue.data(),"out-of-order return warns about the desert shard instead of praising three recovered parts");
 game.takeTowerReturn();game.enterMageTower();game.finishTowerReturn();game.player=MagePosition;check(game.travelToDesert(),"old routes remain available after the third guardian");
 game.player=game.findCorpse(DesertBossId)->pos;game.openedCorpse=DesertBossId;
 check(game.transfer({Place::Corpse,4,DesertBossId},{Place::Bag,10})&&game.recoveredDawnFragments()==3&&!game.dawnShardUnclaimed(WorldLocation::Desert),"recovering the previously skipped shard corrects narrative facts without extra saved state");
 scene.beginTalk(true,3,true);check(scene.dialogue().lines==VolcanoCompletedTalk.data()&&scene.lineCount()==3,"repeat conversation after three parts uses its own dialogue rather than replaying the victory cutscene");
 scene.beginTalk(true,2,true,false,true);check(scene.dialogue().lines==VolcanoShardPendingTalk.data(),"defeated Varkhor with uncollected loot has a separate conversation");
 for(int parts:{1,2,3})for(int fps:{30,60,120}){
  scene.beginTalk(true,parts,parts>=2);for(int frame=0;frame<fps*80&&scene.phase!=TowerPhase::Choice;frame++)scene.update(1.f/fps);
  check(scene.phase==TowerPhase::Choice&&scene.chooseDesert()&&!scene.chooseDesert(),"every normal repeat dialogue reaches a single travel choice at varied update rates");
 }
 check(std::string(towerChronicleText(0,game)).find("Dawno temu")!=std::string::npos,"chronicle and prologue share the same ancient division of the Heart");
 check(std::string(mageTravelDescription(game,true)).find("Varkhor został pokonany")!=std::string::npos,"travel descriptions acknowledge a defeated guardian instead of saying he still guards the shard");
}
