#pragma once

// Version 0.17's water algorithm is the independent visual reference for the
// viewport culling and sine reuse. Used only by the explicit diagnostic.
namespace performance_reference {
void wave(Vector2 p,float width,float phase,Color color){
    float feather=1.1f/renderScale;
    auto vertex=[&](float x,float y,float opacity){rlColor4ub(color.r,color.g,color.b,(unsigned char)(color.a*opacity));rlTexCoord2f(.5f,.5f);rlVertex2f(x,y);};
    rlSetTexture(rlGetTextureIdDefault());rlBegin(RL_QUADS);
    for(int i=0;i<14;i++){
        float a=-1+2.f*i/14,b=-1+2.f*(i+1)/14;
        float x0=p.x+a*width,x1=p.x+b*width;
        float y0=p.y+a*width*.12f+std::sin(a*PI+phase)*1.4f,y1=p.y+b*width*.12f+std::sin(b*PI+phase)*1.4f;
        float edges[]={-.38f-feather,-.38f,.38f,.38f+feather},coverage[]={0,1,1,0};
        for(int strip=0;strip<3;strip++){
            vertex(x0,y0+edges[strip],(1-a*a)*coverage[strip]);vertex(x0,y0+edges[strip+1],(1-a*a)*coverage[strip+1]);
            vertex(x1,y1+edges[strip+1],(1-b*b)*coverage[strip+1]);vertex(x1,y1+edges[strip],(1-b*b)*coverage[strip]);
        }
    }
    rlEnd();rlSetTexture(0);
}
void water(){
    int px=int(std::floor(game.player.x)),py=int(std::floor(game.player.y));
    for(int x=px-30;x<=px+30;x++)for(int y=py-30;y<=py+30;y++){
        if(x>=1&&x<=MapSize-2&&y>=1&&y<=MapSize-2)continue;
        unsigned hash=Game::hash(x,y);
        V origin{float(x)+float(hash%89)/100.f-.44f,float(y)+float((hash>>8)%89)/100.f-.44f};
        float cycle=game.time*.20f+float(hash%997)/997.f,travel=cycle-std::floor(cycle),envelope=std::sin(travel*PI);
        Vector2 p=screen(origin+V{.8f,-.24f}*(travel-.5f));
        if(p.x<-40||p.x>WorldW+40||p.y<-30||p.y>H+30)continue;
        float gust=windGust(origin,game.time),width=12+float(hash%14)+gust*4,phase=game.time*1.3f+origin.x*.8f+origin.y*.6f;
        wave(p,width,phase,Fade(rgb(100,156,151),envelope*envelope*(.20f+gust*.22f)));
        wave({p.x-5,p.y+8},width*.45f,phase+.8f,Fade(rgb(57,107,108),envelope*envelope*.35f));
    }
}
}

bool runPerformanceQA(){
    fs::create_directories("qa-v18");std::ofstream report("qa-v18/performance-tests.txt");int checks=0,failures=0;
    auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
    check(!audioSystem.available,"performance diagnostic has no audio device");
    check(!performance.enabled,"normal rendering never uses blocking GPU profiling barriers");
    RenderTexture2D sample=LoadRenderTexture(WorldW,H);renderScale=1;
    for(V camera:{V{15,15},V{2,15},V{MapWalkMax,15},V{15,2},V{15,MapWalkMax},V{1.6f,1.6f},V{MapWalkMax,MapWalkMax},V{21,5}})for(float time:{0.f,.65f,1.93f}){
        game.player=camera;game.time=time;
        BeginTextureMode(sample);ClearBackground(rgb(22,44,47));performance_reference::water();EndTextureMode();
        Image reference=LoadImageFromTexture(sample.texture);
        BeginTextureMode(sample);ClearBackground(rgb(22,44,47));drawWindWater();EndTextureMode();
        Image optimized=LoadImageFromTexture(sample.texture);Color* a=LoadImageColors(reference);Color* b=LoadImageColors(optimized);
        int maximum=0,largeChanges=0;
        for(int i=0;i<WorldW*H;i++){
            int delta=std::max({std::abs(int(a[i].r)-b[i].r),std::abs(int(a[i].g)-b[i].g),std::abs(int(a[i].b)-b[i].b)});
            maximum=std::max(maximum,delta);largeChanges+=delta>1;
        }
        report<<"Camera="<<camera.x<<','<<camera.y<<"; time="<<time<<"; max pixel delta="<<maximum<<"; changes >1="<<largeChanges<<'\n';
        check(maximum<=3&&largeChanges<50,"optimized water retains visible waves, including all shores and corners");
        UnloadImageColors(a);UnloadImageColors(b);UnloadImage(reference);UnloadImage(optimized);
    }
    UnloadRenderTexture(sample);refreshDisplayLayout();size_t prepared=typography.atlasCount();
    for(int scenario=0;scenario<10;scenario++){
        game=Game{};welcome=help=paused=audioOpen=statsOpen=spellBookOpen=false;mouse={0,0};dragging={};
        if(scenario==1){game.enemies[0].pos=game.player+V{2,-2};Vector2 p=screen(game.enemies[0].pos);mouse={p.x,p.y-18};}
        if(scenario==2){game.hit(game.enemies[0],1000);game.player=game.enemies[0].pos;game.openCorpse(game.enemies[0].id);}
        if(scenario==3){game.attack();game.effects.push_back({game.player,"-120",.8f,4});}
        if(scenario==4){Rectangle r=bagRect(0);mouse={r.x+20,r.y+20};}
        if(scenario==5)help=true;
        if(scenario==6)openStats();
        if(scenario==7)openSpellBook();
        if(scenario==8)audioOpen=true;
        if(scenario==9)game.hp=0;
        renderScene();check(typography.atlasCount()==prepared,"first use of combat, hover and menu text does not load font atlases mid-frame");
    }
    game=Game{};welcome=help=paused=audioOpen=statsOpen=spellBookOpen=false;mouse={0,0};dragging={};refreshDisplayLayout();renderScene();
    BeginDrawing();ClearBackground(BLACK);presentCanvas();Image screenshot=presentedImage();ExportImage(screenshot,"qa-v18/optimized-fullscreen.png");UnloadImage(screenshot);EndDrawing();finishFramePacing();
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}


