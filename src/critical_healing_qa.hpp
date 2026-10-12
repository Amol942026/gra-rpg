#pragma once
bool runCriticalHealingQA(){
 fs::create_directories("qa-v30");std::ofstream report("qa-v30/combat-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 auto reset=[](){game=Game{};game.props.clear();game.enemies.clear();game.forage.clear();game.ground.clear();game.player={15,15};game.messageTime=0;intro={};towerStory={};developerActive=developerMenuOpen=towerPanelOpen=welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=false;statDraft={};skillDraft={};draftForLevel=skillDraftLevel=-1;dragging={};mouse={0,0};};
 auto point=[](Rectangle box){mouse={box.x+box.width/2,box.y+box.height/2};};
 auto seed=[](bool wanted){for(unsigned s=0;s<1000;s++){std::mt19937 rng(s);if((std::uniform_int_distribution<int>(1,100)(rng)<=CriticalChancePercent)==wanted){game.combatRng.seed(s);return;}}};
 auto shot=[](const char* name){std::string path=std::string("qa-v30/")+name+".png";renderScene();saveTexture(canvas,path.c_str());};
 reset();refreshDisplayLayout();check(!audioSystem.available,"diagnostic opens no audio device");
 game.xp=79;Enemy victim{900,0,game.player,game.player,1,1};game.hit(victim,10);syncAdvancement();check(statsOpen&&game.attributePoints==5&&game.skillPoints==1,"real advancement opens mandatory attributes and awards a skill point");
 for(int i=0;i<5;i++){point(statAdjust(0,true));clickProgression();}point(statsConfirm());clickProgression();check(skillsOpen&&game.skillAllocationPending(),"attributes confirmation opens skills while retaining pause");point(skillCard(2));clickProgression();check(skillDraft==SkillRanks{0,0,1}&&game.skillRank(Skill::Healing)==0,"healing card selects draft without learning before confirmation");
 for(float scale:{.8f,1.f,1.2f}){
  prepareCanvas(scale);mouse={0,0};shot(TextFormat("healing-menu-%d",int(scale*100)));check(skillCard(2).y+skillCard(2).height<skillsConfirm().y,"all three skill cards fit above confirmation");
  for(int i=0;i<SkillCount;i++)for(int j=0;j<5;j++)check(!CheckCollisionRecs(skillActionRect(i),actionRect(j)),"skill controls do not overlap ordinary action controls");
 }
 point(skillsConfirm());clickProgression();check(!skillsOpen&&!game.allocationRequired&&game.skillRank(Skill::Healing)==1&&game.skillPoints==0,"confirm learns healing through actual allocation UI and resumes game");game.effects.clear();
 for(bool bow:{false,true}){
  game.healingCd=0;game.hp=40;game.mana=100;game.selectWeapon(bow?1:0);point(skillActionRect(2));leftClickGameplay({});check(game.hp==70&&game.mana==80&&game.healingPulse>0&&!game.bowDrawing,"HUD healing restores thirty HP for twenty mana with sword and bow");
  for(int i=0;i<12;i++)game.update(.035f,{},false);mouse={0,0};shot(bow?"healing-bow":"healing-sword");
 }
 game.healingCd=0;game.hp=game.maxHp();float mana=game.mana;check(!activateGameplaySkill(Skill::Healing)&&game.mana==mana,"HUD refuses full-health healing without mana loss");
 openSkills();point(skillAdjust(2,true));clickProgression();check(skillDraftRemaining()==0&&skillDraft==SkillRanks{},"learned healing sheet cannot spend unearned points");confirmSkills();
 game.skillPoints=1;game.allocationRequired=true;openSkills();point(skillCard(0));clickProgression();point(skillCard(2));clickProgression();check(skillDraft==SkillRanks{1,0,0},"clicking already learned healing preserves a pending choice of another skill");confirmSkills();
 reset();game.skillRanks={1,1,1};game.enemies.push_back({810,3,{16,14},{16,14},5000,5000});game.enemies.push_back({811,0,{14,16},{14,16},5000,5000});seed(true);check(game.attack()&&game.effects.size()==2&&game.effects[0].critical&&game.effects[1].critical,"sword critical marks damage above both large guardian and small slime");
 for(float scale:{.8f,1.f,1.2f}){prepareCanvas(scale);mouse={0,0};size_t atlases=typography.atlasCount();shot(TextFormat("critical-sword-%d",int(scale*100)));check(typography.atlasCount()==atlases,"critical rendering reuses font atlases without first-hit GPU upload");}
 reset();game.skillRanks={1,1,1};game.enemies.push_back({812,1,{16,15},{16,15},5000,5000});seed(false);game.attack();check(!game.effects[0].critical,"normal damage has no critical label");shot("normal-damage");
 reset();game.skillRanks={1,1,1};game.enemies.push_back({813,2,{16,15},{16,15},5000,5000});seed(true);check(activateGameplaySkill(Skill::StrongCut)&&game.effects[0].critical&&game.strongSwingActive(),"strong-cut HUD route produces enhanced animation and critical feedback");shot("critical-strong-cut");
 reset();game.skillRanks={1,1,1};game.selectWeapon(1);for(int i=0;i<3;i++){V p=game.player+V{3.f+i*1.3f,0};game.enemies.push_back({820+i,1,p,p,5000,5000});}seed(true);point(skillActionRect(1));leftClickGameplay({});mouse=screen(game.enemies[0].pos);mouse.y-=18;leftClickGameplay({});game.bowDrawTime=BowDrawDuration;
 check(releaseWorldBow()&&game.arrows[0].critical&&game.arrows[0].piercing&&game.mana==80,"real bow mouse release retains critical and piercing metadata and twenty-mana cost");game.updateArrows(.35f);shot("critical-piercing");game.updateArrows(.5f);check(game.effects.size()==3&&game.effects[0].critical&&game.effects[1].critical&&game.effects[2].critical,"single critical piercing arrow labels all three actual impacts");
 reset();game.enemies.push_back({830,0,{18,15},{18,15},5000,5000});seed(true);mouse=screen(game.enemies[0].pos);mouse.y-=18;check(castGameplaySpell()&&game.fireballs[0].critical,"C spell handler keeps critical through cast");game.updateFireballs(.6f);mouse={0,0};shot("critical-fireball");check(!game.effects.empty()&&game.effects[0].critical,"fireball impact shows critical feedback");
 reset();game.skillRanks={1,1,1};game.enterMageTower();game.dawnQuest=DawnQuest::AtTower;game.hp=20;check(activateGameplaySkill(Skill::Healing)&&game.hp==50&&game.mana==80,"healing works through gameplay handler in safe tower");game.update(.05f,{},false);shot("healing-tower");
 report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';reset();refreshDisplayLayout();return failures==0;
}
