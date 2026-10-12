#pragma once

bool runProgressionQA(){
    fs::create_directories("qa-v13");std::ofstream report("qa-v13/progression-tests.txt");int checks=0,failures=0;
    auto check=[&](bool ok,const char* label){checks++;if(!ok)failures++;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
    auto pointAt=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
    game=Game{};game.props.clear();game.enemies.clear();game.ground.clear();game.player={15,15};
    welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;statDraft={};draftForLevel=-1;dragging={};
    prepareCanvas(1.2f);pointAt(statsButton());
    check(clickProgression()&&statsOpen,"HUD stats button opens the level-one character sheet");
    check(!worldPointerAllowed(mouse),"character sheet blocks world interactions");
    renderScene();saveTexture(canvas,"qa-v13/character-sheet.png");
    check(confirmStats()&&!statsOpen,"character sheet can close when no points are pending");
    game.xp=79;Enemy defeated{91,0,{18,15},{18,15},1,36};game.hit(defeated,10);syncAdvancement();
    check(game.level==2&&game.attributePoints==5&&statsOpen,"real kill and XP automatically open five-point level menu");
    check(!confirmStats()&&statsOpen&&game.allocationRequired,"confirm cannot bypass unspent level points");
    pointAt(statAdjust(0,true));clickProgression();
    check(statDraft[0]==1&&game.attributePoints==5&&game.attribute(Attribute::Strength)==0,"plus button changes only the allocation preview");
    pointAt(statAdjust(0,false));clickProgression();
    check(statDraft[0]==0&&draftRemaining()==5,"minus button returns a pending point");
    pointAt(statAdjust(0,false));clickProgression();check(statDraft[0]==0,"minus cannot remove points from earlier levels");
    for(int index:{0,0,1,4,5}){pointAt(statAdjust(index,true));clickProgression();}
    check(draftRemaining()==0&&!adjustStat(2,true),"five points are the exact spending limit");
    float before=game.time;V location=game.player;game.update(.05f,{1,0},true);
    check(game.time==before&&distance(game.player,location)==0,"mandatory menu freezes actual simulation");
    mouse={0,0};
    for(float factor:{.8f,1.f,1.2f}){prepareCanvas(factor);renderScene();saveTexture(canvas,TextFormat("qa-v13/level-menu-%d.png",int(factor*100)));}
    pointAt(statsConfirm());clickProgression();
    check(!statsOpen&&skillsOpen&&game.skillAllocationPending()&&game.attributePoints==0,"stats confirmation opens skills while mandatory pause remains");
    adjustSkill(0,true);check(confirmSkills()&&!game.allocationRequired&&!skillsOpen,"skill confirmation applies one point and resumes play");
    check(game.damage()==18&&game.magicDamage()==31&&game.maxHp()==110&&game.maxMana()==105,"HUD values match the committed allocation");
    game.chooseSpell(Spell::None);activateMouseAction(1,{});
    check(spellBookOpen&&!statsOpen&&game.fireballs.empty(),"clicking spell tile opens selection without casting");
    check(!worldPointerAllowed({600,400}),"spell selection prevents interaction with the map");
    mouse={0,0};renderScene();saveTexture(canvas,"qa-v13/spell-book.png");
    pointAt(spellChoice());clickProgression();
    check(!spellBookOpen&&game.selectedSpell==Spell::Fireball,"clicking fireball card equips the chosen spell");
    V target=game.player+V{3,-3};game.enemies.push_back({44,0,target,target,100,100});game.enemies[0].windup=100;
    Vector2 p=screen(target);mouse={p.x,p.y-18};
    check(pickWorld(mouse).kind==HoverKind::Enemy,"aiming pointer identifies the visible monster");
    float mana=game.mana;check(castGameplaySpell()&&game.mana==mana-10,"C cast handler spends ten mana");
    check(distance(normalized(game.fireballs[0].velocity),normalized(target-game.player))<.001f,"fireball aims at monster position even when cursor is over its head");
    for(int i=0;i<5;i++)game.update(.05f,{},false);
    mouse={0,0};renderScene();saveTexture(canvas,"qa-v13/fireball-flight.png");
    for(int i=0;i<5;i++)game.update(.05f,{},false);
    check(game.enemies[0].hp==69&&game.fireballs.empty()&&!game.explosions.empty(),"projectile hits with allocated magical damage and creates explosion");
    renderScene();saveTexture(canvas,"qa-v13/fireball-impact.png");
    game=Game{};game.props.clear();game.enemies.clear();game.ground.clear();game.player={15,15};game.xp=79;
    V victim=game.player+V{2,-2};game.enemies.push_back({44,0,victim,victim,1,36});game.enemies[0].windup=100;game.spell(victim-game.player);
    for(int i=0;i<30&&!game.allocationRequired;i++)game.update(.05f,{},false);
    syncAdvancement();check(game.allocationRequired&&statsOpen&&game.attributePoints==5,"a fireball kill also pauses and opens allocation automatically");
    game.sounds.clear();game.sound(Cue::LevelUp,game.player);int played=audioSystem.playedEffects;
    audioSystem.update(.05f,game,true,true);
    check(!audioSystem.available||audioSystem.playedEffects>played,"level fanfare is allowed while mandatory menu pauses combat audio");
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
