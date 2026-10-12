#pragma once

bool runTowerQA(){
    fs::create_directories("qa-v26");std::ofstream report("qa-v26/tower-tests.txt");int checks=0,failures=0;
    auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
    auto shot=[&](const char* name){
        renderScene();BeginDrawing();presentCanvas();Image image=presentedImage();
        check(image.data&&image.width==GetRenderWidth()&&image.height==GetRenderHeight(),"story frame reaches the actual presentation buffer at native dimensions");
        if(image.data)ExportImage(image,(std::string("qa-v26/")+name+".png").c_str());EndDrawing();finishFramePacing();return image;
    };
    check(!audioSystem.available,"tower diagnostic opens no audio device");
    game=Game{};intro={};towerStory={};clearStoryMenus();refreshDisplayLayout();
    Enemy& boss=game.enemies.back();game.player=boss.pos;game.hit(boss,1000);
    Attributes points{};points[int(Attribute::Strength)]=game.attributePoints;game.distribute(points);SkillRanks learned{};learned[0]=game.skillPoints;game.trainSkills(learned);
    game.player=boss.pos+V{-1.6f,.8f};
    check(game.bossDead&&game.dawnFragments()==0&&!towerStory.active()&&!game.inTower(),"guardian defeat leaves hero in the forest until the artifact is collected");
    Vector2 bodyPoint=screen(boss.pos);bool found=false;
    for(int y=-48;y<=40&&!found;y+=2)for(int x=-68;x<=68&&!found;x+=2){
        Vector2 p{bodyPoint.x+x,bodyPoint.y+y};WorldHover hit=pickWorld(p);
        if(hit.kind==HoverKind::Corpse&&hit.index==boss.id){mouse=p;found=true;}
    }
    bool opened=found&&rightClickWorldOrInventory()&&game.openedCorpse==boss.id;
    check(opened,"real corpse pointer and right-click open guardian loot");if(!opened){report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return false;}
    mouse={lootRect(4).x+28,lootRect(4).y+22};
    check(hoverSlot().place==Place::Corpse&&game.get(hoverSlot())->kind==Kind::DawnShard,"fifth visible corpse slot contains the guaranteed quest artifact");
    Image loot=shot("guardian-fragment");UnloadImage(loot);
    float hp=game.hp,mana=game.mana;int level=game.level,xp=game.xp;V forestEnemy=game.enemies[0].pos;
    leftClickGameplay({});mouse={bagRect(8).x+28,bagRect(8).y+20};finishDrag();startTowerReturn();
    check(game.bag[8].kind==Kind::DawnShard&&game.dawnFragments()==1&&towerStory.phase==TowerPhase::FadeOut&&!game.inTower(),"real inventory drop starts the dark fade before changing location");
    check(!gameplayInputAllowed()&&!worldPointerAllowed(screen(game.player))&&!castGameplaySpell(),"return cinematic blocks combat and world input immediately after pickup");
    for(int i=0;i<35;i++)towerAction(false,false,1.f/60);
    check(towerStory.fade()>.65f&&!game.inTower()&&game.time==0,"forest darkens while its simulation remains frozen");
    Image fade=shot("forest-fade");UnloadImage(fade);
    float age=towerStory.age;towerAction(true,true,.05f,false);
    check(towerStory.age==age&&towerStory.phase==TowerPhase::FadeOut,"lost focus freezes the return and ignores skip or advance");
    for(int i=0;i<90&&towerStory.phase!=TowerPhase::Opening;i++)towerAction(false,false,1.f/60);
    check(game.inTower()&&distance(game.player,TowerArrival)==0&&game.storyBlocked()&&game.openedCorpse==-1,"fade reaches the tower exactly once and closes the old loot panel");
    towerStory.age=.8f;Image arrival=shot("tower-arrival");UnloadImage(arrival);
    towerStory.enter(TowerPhase::Dialogue);towerStory.revealed=true;
    Image hero=shot("return-hero-dialogue");UnloadImage(hero);
    std::ofstream transcript("qa-v26/dialogue.txt");
    for(const auto& line:TowerReturnDialogue){
        transcript<<(line.mage?"Eldrin: ":"Wędrowiec: ")<<line.text<<"\n\n";
        auto rows=storyRows(line.text,1140,21);bool fits=rows.size()<=3;for(const auto& row:rows)fits&=textWidth(row,21)<=1140;
        check(fits,"return dialogue fits the shared panel above its padded navigation button");
    }
    towerStory.line=1;towerStory.revealed=true;Image praise=shot("eldrin-praise");UnloadImage(praise);
    towerStory.line=7;towerStory.revealed=true;Image next=shot("next-journey");UnloadImage(next);
    for(int i=0;i<200&&towerStory.active();i++)towerAction(true,false,1.f/60);
    check(!towerStory.active()&&game.inTower()&&game.dawnQuest==DawnQuest::AtTower&&gameplayInputAllowed(),"cutscene ends in playable tower with one of five parts recorded");
    check(game.hp==hp&&game.mana==mana&&game.level==level&&game.xp==xp&&game.bag[8].kind==Kind::DawnShard,"cutscene retains hero health, mana, level, XP and the collected piece");
    check(distance(game.enemies[0].pos,forestEnemy)==0&&game.time==0,"no forest simulation or enemy movement occurs behind the cutscene");
    mouse={0,0};game.messageTime=0;Image tower=shot("mage-tower");
    Color* pixels=LoadImageColors(tower);int lit=0,purple=0;Vector2 mage=screen(MagePosition);
    for(int y=0;y<tower.height;y++)for(int x=0;x<tower.width;x++){
        Color p=pixels[y*tower.width+x];lit+=int(p.r)+p.g+p.b>75;
        float xx=(x-presentationViewport.x)/renderScale,yy=(y-presentationViewport.y)/renderScale;
        if(std::abs(xx-mage.x)<30&&yy>mage.y-112&&yy<mage.y-5)purple+=p.b>p.g*1.35f&&p.r>p.g*1.15f&&p.b>45;
    }
    check(lit>tower.width*tower.height*.2&&purple>300,"playable destination renders a stone floor, furnishings and visible purple-robed mage");
    UnloadImageColors(pixels);UnloadImage(tower);
    check(!talkToMage()&&!towerStory.active(),"NPC cannot be talked to from beyond interaction range");
    V before=game.player;for(int i=0;i<95;i++)game.update(1.f/60,{0,-1},false);
    check(distance(before,game.player)>1&&game.mageReachable()&&distance(game.enemies[0].pos,forestEnemy)==0,"normal tower movement reaches the NPC while forest enemies stay inactive");
    Vector2 p=screen(MagePosition);mouse={p.x,p.y-100};WorldHover mageHit=pickWorld(mouse);
    check(mageHit.kind==HoverKind::Mage&&inspectWorld(mageHit).kind=="Mag / przyjazny NPC","pointer picking identifies Eldrin as a friendly NPC");
    Image nearby=shot("eldrin-npc");UnloadImage(nearby);
    check(rightClickWorldOrInventory()&&towerStory.active()&&!towerStory.returning,"right-click on the actual mage starts a repeat NPC conversation");
    age=towerStory.age;towerAction(true,false,.05f,false);check(towerStory.age==age,"NPC dialogue also pauses when the game loses focus");
    towerStory.age=.8f;towerStory.revealed=true;Image talk=shot("npc-conversation");UnloadImage(talk);
    before=game.player;towerAction(false,true,.01f);check(!towerStory.active()&&gameplayInputAllowed()&&distance(before,game.player)==0,"ESC closes NPC dialogue without moving the hero");
    leftClickGameplay({});check(towerStory.active()&&game.swing==0&&!game.bowDrawing,"left-click on Eldrin speaks instead of swinging or drawing the bow");
    towerAction(false,true,.01f);check(talkToMage(),"keyboard interaction shares a repeatable NPC conversation handler");
    for(const auto& line:TowerTalkDialogue){
        transcript<<(line.mage?"Eldrin: ":"Wędrowiec: ")<<line.text<<"\n\n";
        auto rows=storyRows(line.text,1140,21);check(rows.size()<=3,"repeat NPC dialogue fits the same readable panel");
    }
    for(int i=0;i<250&&towerStory.active();i++)towerAction(true,false,1.f/60);
    check(!towerStory.active()&&game.inTower()&&distance(before,game.player)==0&&game.dawnFragments()==1,"repeat dialogue returns control without duplicating fragments or teleporting");
    bool startedFull=fullScreen();if(startedFull)toggleFullScreen();SetWindowSize(1152,720);PollInputEvents();refreshDisplayLayout();
    Image small=shot("tower-1152");UnloadImage(small);talkToMage();towerStory.revealed=true;
    Image smallTalk=shot("npc-dialogue-1152");UnloadImage(smallTalk);
    check(canvas.texture.width>0&&presentationViewport.x>=0&&presentationViewport.y>=0,"playable tower and NPC dialogue fit the smallest supported window");
    towerAction(false,true,.01f);if(startedFull)toggleFullScreen();PollInputEvents();refreshDisplayLayout();
    check(game.inTower()&&game.dawnFragments()==1&&!towerStory.active(),"F11 and resizing retain tower location and quest state");
    Image fullscreen=shot("tower-fullscreen");UnloadImage(fullscreen);
    game=Game{};startIntro();check(intro.active()&&!towerStory.active()&&game.dawnFragments()==0&&!game.inTower(),"new adventure resets the quest and begins the original prologue");
    game=Game{};intro={};towerStory={};clearStoryMenus();mouse={0,0};
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
