#pragma once

bool runIntroQA(){
    fs::create_directories("qa-v24/intro");std::ofstream report("qa-v24/intro/tests.txt");int checks=0,failures=0;
    auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
    check(!audioSystem.available,"cinematic diagnostic never opens an audio device");
    game=Game{};startIntro();refreshDisplayLayout();
    check(intro.active()&&!welcome&&!paused&&!statsOpen&&!weaponWheelOpen,"normal prologue starts without a gameplay menu covering it");
    V start=game.player,enemyStart=game.enemies[0].pos;float hp=game.hp,mana=game.mana;
    for(int frame=0;frame<120;frame++)introAction(false,false,1.f/60);
    check(game.time==0&&distance(game.player,start)==0&&distance(game.enemies[0].pos,enemyStart)==0&&game.hp==hp&&game.mana==mana,
          "world, enemies, health and mana remain frozen during the prologue");
    mouse=screen(game.enemies[0].pos);leftClickGameplay({1,0});
    check(!gameplayInputAllowed()&&!worldPointerAllowed(mouse)&&!castGameplaySpell()&&!rightClickWorldOrInventory()&&game.swing==0&&game.mana==mana,
          "cinematic input cannot leak into combat, spells, loot or world picking");
    float time=intro.time;introAction(true,true,.05f,false);
    check(intro.time==time&&intro.phase==IntroPhase::Dialogue,"losing focus freezes dialogue and ignores keys");
    auto shot=[&](const char* path){
        renderScene();BeginDrawing();presentCanvas();Image actual=presentedImage();
        check(actual.data&&actual.width==GetRenderWidth()&&actual.height==GetRenderHeight(),"cinematic reaches the actual presentation buffer at native size");
        if(actual.data)ExportImage(actual,path);EndDrawing();finishFramePacing();return actual;
    };
    intro.line=0;intro.revealed=true;Image mage=shot("qa-v24/intro/mage-dialogue.png");
    int purple=0,lit=0;Color* pixels=LoadImageColors(mage);
    for(int y=0;y<mage.height;y++)for(int x=0;x<mage.width;x++){
        Color p=pixels[y*mage.width+x];lit+=int(p.r)+p.g+p.b>18;
        float logicalX=(x-presentationViewport.x)/renderScale,logicalY=(y-presentationViewport.y)/renderScale;
        if(logicalX>840&&logicalX<1030&&logicalY>210&&logicalY<525)purple+=p.b>p.g*1.3f&&p.r>p.g*1.1f&&p.b>45;
    }
    check(lit>mage.width*mage.height*.65&&purple>1200,"destination shows the observatory and a visible purple-robed mage rather than a black frame");
    UnloadImageColors(pixels);UnloadImage(mage);
    std::ofstream transcript("qa-v24/intro/dialogue.txt");
    for(size_t i=0;i<IntroDialogue.size();i++){
        intro.line=i;intro.revealed=true;transcript<<(intro.current().mage?"Eldrin: ":"Wędrowiec: ")<<intro.current().text<<"\n\n";
        auto rows=storyRows(intro.current().text,1140,21);
        bool fits=rows.size()<=3;for(const std::string& row:rows)fits&=textWidth(row,21)<=1140;
        check(fits,"complete dialogue turn fits above its controls without clipping");
    }
    intro.line=1;Image heroLine=shot("qa-v24/intro/hero-dialogue.png");UnloadImage(heroLine);
    intro.line=6;Image fragments=shot("qa-v24/intro/five-fragments.png");UnloadImage(fragments);
    intro.line=11;Image farewell=shot("qa-v24/intro/ruszaj-wiec.png");UnloadImage(farewell);
    bool startedFull=fullScreen();if(startedFull)toggleFullScreen();SetWindowSize(1152,720);PollInputEvents();refreshDisplayLayout();
    Image small=shot("qa-v24/intro/dialogue-1152.png");UnloadImage(small);
    auto longest=storyRows(IntroDialogue[6].text,1140,21);
    check(longest.size()<=3&&presentationViewport.x>=0&&presentationViewport.y>=0,"dialogue and cinematic controls fit the smallest supported window");
    intro.line=6;intro.revealed=false;intro.age=.25f;
    Image reveal=shot("qa-v24/intro/reveal-button-1152.png");UnloadImage(reveal);
    if(startedFull)toggleFullScreen();PollInputEvents();refreshDisplayLayout();
    game.player={14,14};intro.skip();intro.phase=IntroPhase::Charging;intro.age=1.3f;
    Image portal=shot("qa-v24/intro/portal.png");UnloadImage(portal);
    for(int i=0;i<200&&intro.phase!=IntroPhase::Arriving;i++)introAction(false,false,1.f/60);
    check(intro.phase==IntroPhase::Arriving&&distance(game.player,{8,19})==0&&!intro.arrivalPending,
          "teleport places the real hero at the existing safe forest spawn exactly once");
    intro.age=.75f;Image arrival=shot("qa-v24/intro/arrival.png");
    pixels=LoadImageColors(arrival);int blue=0;std::array<bool,232> blueRows{};Vector2 hero=screen(game.player);
    for(int y=0;y<arrival.height;y++)for(int x=0;x<arrival.width;x++){
        float xx=(x-presentationViewport.x)/renderScale,yy=(y-presentationViewport.y)/renderScale;
        if(std::abs(xx-hero.x)>65||yy<hero.y-215||yy>hero.y-25)continue;
        Color p=pixels[y*arrival.width+x];
        if(p.b>130&&p.b>p.r*1.4f&&p.b>p.g*1.03f){blue++;blueRows[size_t(std::clamp(int(hero.y-yy),0,231))]=true;}
    }
    check(blue>350&&std::count(blueRows.begin(),blueRows.end(),true)>130,
          "actual forest arrival shows a tall blue magic column surrounding the real hero");
    UnloadImageColors(pixels);UnloadImage(arrival);
    intro.age=1.5f;Image late=shot("qa-v24/intro/arrival-fading.png");UnloadImage(late);
    check(intro.fade()==0&&intro.active()&&!gameplayInputAllowed()&&game.time==0,
          "blue arrival remains visible after the flash clears and before gameplay resumes");
    for(int i=0;i<100&&intro.active();i++)introAction(false,false,1.f/60);
    check(!intro.active()&&gameplayInputAllowed()&&game.time==0,"arrival restores gameplay only after the transition with no hidden simulation time");
    Image forest=shot("qa-v24/intro/forest-after.png");UnloadImage(forest);
    float before=game.time;game.update(.05f,{1,0},false);check(game.time>before,"world simulation resumes after the prologue");
    startIntro();check(intro.phase==IntroPhase::Opening&&intro.line==0,"new adventure can replay the complete introduction");
    intro=IntroSequence{};game=Game{};welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=false;dragging={};mouse={0,0};
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
