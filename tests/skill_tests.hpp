#pragma once
void skillTests(){
 Game start=emptyArena();check(start.skillPoints==0&&start.skillRanks==SkillRanks{}&&!start.strongCut()&&!start.preparePiercingArrow(),"new character has no learned skill and cannot use either before allocation");
 earnLevel(start);check(start.skillPoints==1&&start.attributePoints==5&&start.allocationRequired,"each real level awards five attributes and one skill point");
 check(!start.trainSkills({1,0})&&start.skillPoints==1,"skills cannot be allocated before attributes");
 start.distribute({0,0,0,0,0,5});float time=start.time,mana=start.mana;V pos=start.player;
 start.update(.05f,{1,0},true);check(start.skillAllocationPending()&&start.time==time&&distance(pos,start.player)==0&&start.mana==mana,"skill stage preserves mandatory pause for movement, attacks and regeneration");
 check(!start.trainSkills({0,0})&&!start.trainSkills({-1,2})&&!start.trainSkills({1,1}),"zero, negative and excessive skill drafts cannot bypass allocation");
 check(start.trainSkills({1,0})&&start.skillRank(Skill::StrongCut)==1&&start.skillRank(Skill::PiercingArrow)==0&&!start.allocationRequired,"one point learns chosen skill and resumes after both stages");
 check(!start.trainSkills({1,0}),"same level cannot spend its skill point twice");
 start.xp=experienceForLevel(start.level)-1;Enemy next{901,0,start.player,start.player,1,1};start.hit(next,10);start.distribute({5,0,0,0,0,0});
 check(start.skillPoints==1&&start.trainSkills({0,1})&&start.skillRanks==SkillRanks{1,1},"next level supplies one new point and can unlock other skill");
 Game multi=emptyArena();multi.xp=experienceForLevel(1)+experienceForLevel(2)+10;Enemy fake{900,0,multi.player,multi.player,1,1};multi.hit(fake,10);
 check(multi.skillPoints==2&&multi.attributePoints==10,"multiple simultaneous advances retain one skill point per gained level");multi.distribute({10,0,0,0,0,0});
 check(multi.trainSkills({2,0})&&multi.skillRank(Skill::StrongCut)==2&&!multi.allocationRequired,"multiple earned points may increase chosen skill rank without deadlocking");
 auto ready=[](){Game g=emptyArena();g.skillRanks={1,1};return g;};
 Game g=ready();float damage=g.damage();g.enemies.push_back({80,0,g.player+V{1,0},g.player+V{1,0},500,500});g.enemies.push_back({81,0,g.player+V{-1,0},g.player+V{-1,0},500,500});
 check(g.strongCut()&&g.mana==90&&g.strongSwingActive()&&g.meleeDuration==StrongCutDuration,"strong cut spends exactly ten mana and starts its own longer animation");
 for(const auto& e:g.enemies)check(std::abs(e.hp-(500-damageAfterArmor(damage*1.5f,e.defense())))<.001f,"rank-one strong cut deals greater physical damage to every enemy in melee area");
 check(!g.strongCut()&&g.mana==90,"strong-cut cooldown prevents duplicate cast and mana spending");
 g=ready();g.mana=9;check(!g.strongCut()&&g.mana==9&&g.swing==0&&g.strongCutCd==0,"insufficient mana leaves strong cut entirely unchanged");
 for(Kind weapon:{Kind::Bow,Kind::Axe,Kind::Dagger,Kind::None}){g=ready();g.equipment[0]=item(weapon);check(!g.strongCut()&&g.mana==100&&g.swing==0,"strong cut requires a sword specifically, not any other melee or ranged weapon");}
 g=ready();g.skillRanks[0]=2;check(g.strongCutDamage()>damage*1.5f,"next strong-cut rank increases its damage");
 g=ready();g.strongCut();for(int i=0;i<30;i++)g.update(.05f,{},false);check(!g.strongSwingActive()&&g.strongCutCd==0&&g.attack()&&g.meleeDuration==SwingDuration&&!g.swingStrong,"normal attack after skill uses original animation and timing");
 g=ready();check(!g.preparePiercingArrow()&&g.mana==100&&g.ammoCount()==20,"piercing arrow is blocked while sword is equipped");g.selectWeapon(1);g.mana=19;
 check(!g.preparePiercingArrow()&&!g.piercingArrowArmed&&g.mana==19,"piercing preparation requires twenty mana but never spends on failure");
 g.mana=100;check(g.preparePiercingArrow()&&g.piercingArrowArmed&&g.mana==100&&g.ammoCount()==20,"preparation only arms next shot without spending mana or ammo");
 g.beginBow({1,0});g.bowDrawTime=BowDrawDuration;float normalDamage=g.arrowDamage(1);
 check(g.releaseBow()&&g.arrows.size()==1&&g.arrows[0].piercing&&g.arrows[0].damage==normalDamage&&g.mana==80&&g.ammoCount()==19&&!g.piercingArrowArmed,"piercing release spends twenty mana and one arrow, preserving normal bow damage exactly");
 check(!g.preparePiercingArrow()&&g.mana==80,"piercing cooldown blocks another preparation without charging twice");
 g=ready();g.selectWeapon(1);g.preparePiercingArrow();g.beginBow({1,0});g.cancelBow();check(g.mana==100&&g.ammoCount()==20&&!g.releaseBow()&&!g.piercingArrowArmed,"cancelled prepared shot consumes no resource and disarms skill");
 g.preparePiercingArrow();g.beginBow({1,0});g.mana=19;check(!g.releaseBow()&&g.mana==19&&g.ammoCount()==20&&g.arrows.empty(),"mana loss during drawing cannot fire a free piercing arrow or waste ammunition");
 g=ready();g.selectWeapon(1);g.mana=20;g.preparePiercingArrow();g.spell({1,0});check(g.mana==10&&!g.beginBow({1,0})&&!g.bowDrawing&&g.ammoCount()==20,"spending mana after preparation blocks drawing a now-unaffordable piercing shot");
 g=ready();g.selectWeapon(1);g.preparePiercingArrow();g.selectWeapon(0);check(!g.piercingArrowArmed&&!g.beginBow({1,0}),"changing to sword clears piercing preparation");
 g=ready();g.selectWeapon(1);g.preparePiercingArrow();g.startRoll({1,0});check(!g.piercingArrowArmed&&g.mana==100&&g.ammoCount()==20,"dodge cancels prepared arrow without cost");
 auto line=[](int rank=1){Game a=emptyArena();a.skillRanks={1,rank};a.selectWeapon(1);for(int i=0;i<4;i++){V p=a.player+V{3.f+i*2,0};a.enemies.push_back({100+i,1,p,p,500,500});}return a;};
 g=line();g.preparePiercingArrow();g.beginBow({1,0});g.bowDrawTime=BowDrawDuration;float hitDamage=g.arrowDamage(1);g.releaseBow();g.updateArrows(1);
 for(int i=0;i<4;i++)check(std::abs(g.enemies[i].hp-(i<3?500-damageAfterArmor(hitDamage,g.enemies[i].defense()):500))<.001f,"piercing arrow hits first three targets in travel order and stops before fourth");
 check(g.arrows.empty()&&g.pierceFlashes.size()==3,"piercing impact animation is emitted once for each actual hit");
 g=line(2);g.preparePiercingArrow();g.beginBow({1,0});g.bowDrawTime=BowDrawDuration;check(g.arrowDamage(1)==hitDamage,"upgrading piercing rank does not increase arrow damage");g.releaseBow();g.updateArrows(1);check(g.enemies[3].hp<500,"second piercing rank reaches one additional target");
 g=line();g.beginBow({1,0});g.bowDrawTime=BowDrawDuration;g.releaseBow();g.updateArrows(1);check(g.enemies[0].hp<500&&g.enemies[1].hp==500&&!g.arrows.size(),"ordinary bow arrow continues to stop on its first target");
 g=line();for(int i=1;i<4;i++)g.enemies[i].pos.x+=2;g.props.push_back({g.player+V{5,0},0,1});g.preparePiercingArrow();g.beginBow({1,0});g.bowDrawTime=BowDrawDuration;g.releaseBow();g.updateArrows(1);
 check(g.enemies[0].hp<500&&g.enemies[1].hp==500&&g.arrows.empty(),"piercing penetrates enemies but is stopped by scenery before targets behind it");
 g=ready();g.selectWeapon(1);V p=g.player+V{1,0};g.enemies.push_back({100,1,p,p,500,500});g.preparePiercingArrow();g.beginBow({1,0});g.bowDrawTime=BowDrawDuration;g.releaseBow();g.updateArrows(.02f);float after=g.enemies[0].hp;
 for(int i=0;i<5;i++)g.updateArrows(.02f);check(g.enemies[0].hp==after&&after<500,"same enemy cannot be hit again while arrow passes through its body across frames");
 for(int steps:{10,40,100}){g=line();g.preparePiercingArrow();g.beginBow({1,0});g.bowDrawTime=BowDrawDuration;g.releaseBow();for(int i=0;i<steps;i++)g.updateArrows(1.f/steps);check(g.enemies[0].hp<500&&g.enemies[2].hp<500&&g.enemies[3].hp==500,"piercing target count and hits are independent of update frequency");}
 g=line();g.xp=experienceForLevel(1)-1;for(int i=0;i<3;i++)g.enemies[i].hp=1;g.preparePiercingArrow();g.beginBow({1,0});g.bowDrawTime=BowDrawDuration;g.releaseBow();g.updateArrows(1);
 check(g.kills==3&&g.allocationRequired&&g.skillPoints>=1&&g.findCorpse(100)&&g.findCorpse(102)&&g.ground.empty(),"piercing segment completes its hits, corpse loot and XP before level pause");
 g=ready();g.hp=0;check(!g.strongCut()&&!g.preparePiercingArrow(),"dead hero cannot activate skills");
 g=ready();g.enterMageTower();check(!g.strongCut()&&!g.preparePiercingArrow(),"safe tower blocks both combat skills");
}
