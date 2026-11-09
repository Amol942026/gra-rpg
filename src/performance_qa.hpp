#pragma once

bool runPerformanceQA(){
    fs::create_directories("qa-v18");std::ofstream report("qa-v18/performance-tests.txt");int checks=0,failures=0;
    auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
    check(!audioSystem.available,"performance diagnostic has no audio device");
    check(!performance.enabled,"normal rendering never uses blocking GPU profiling barriers");
    RenderTexture2D sample=LoadRenderTexture(WorldW,H);renderScale=1;
    check(forest_water::cameraLocation>=0&&forest_water::timeLocation>=0,"the forest water shader has live projection and time uniforms");
    auto drawSurface=[&](float time){
        game.time=time;BeginTextureMode(sample);ClearBackground(BLANK);forest_water::draw();EndTextureMode();
        Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);return im;
    };
    for(V camera:{ForestArrival,V{MapWalkMin,72},V{MapWalkMax,72},V{72,MapWalkMin},V{72,MapWalkMax}}){
        game.player=camera;
        Image first=drawSurface(.4f),second=drawSurface(1.8f);
        auto* a=static_cast<Color*>(first.data);auto* b=static_cast<Color*>(second.data);
        int changed=0,dark=0,bright=0;
        for(int i=0;i<WorldW*H;i++)if(a[i].a){
            dark+=a[i].g<95;bright+=a[i].g>155;changed+=std::abs(int(a[i].g)-b[i].g)>8;
        }
        report<<"Coast "<<camera.x<<','<<camera.y<<"; wave pixels="<<changed<<"; bright crests="<<bright<<'\n';
        check(changed>12000&&dark>10000&&bright>1000,"each shore has varied, readable and time-dependent wave crests");
        UnloadImage(first);UnloadImage(second);
    }
    game.player={72,72};Image inland=drawSurface(1.f);
    auto* pixels=static_cast<Color*>(inland.data);
    check(std::none_of(pixels,pixels+WorldW*H,[](Color c){return c.a!=0;}),"inland frames skip the water pass");
    UnloadImage(inland);
    game.player=ForestArrival;
    BeginTextureMode(sample);ClearBackground(BLANK);terrain_surface::draw(terrain.texture);EndTextureMode();
    Image shore=LoadImageFromTexture(sample.texture);ImageFlipVertical(&shore);
    pixels=static_cast<Color*>(shore.data);
    int stray=0,ground=0;Vector2 origin=screenFlat({0,0});
    for(int y=0;y<H;y++)for(int x=0;x<WorldW;x++)if(pixels[y*WorldW+x].a>30){
        float dx=(x+.5f-origin.x)/80,dy=(y+.5f-origin.y)/40;
        float wx=dx+dy,wy=dy-dx;
        if(wx<-3||wy>MapSize+2)stray++;else ground++;
    }
    check(stray==0&&ground>100000,"the start corner has continuous land and no wrapped grass in the water");
    UnloadImage(shore);
    using namespace forest_wave_crests;
    const auto arrivalCrests=collect(origin,1.f);
    check(forest_wave_crests::timeLocation>=0&&std::any_of(arrivalCrests.crests.begin(),arrivalCrests.crests.begin()+arrivalCrests.count,
          [](const Crest& c){return c.height>=60&&c.center.x>=40&&c.center.x<WorldW-40&&c.center.y>120&&c.center.y<H;}),
          "visible high crests are active near the forest arrival");
    int displayedCrests=0;
    for(int frame=0;frame<20;frame++){
        const auto waves=collect(origin,frame*.35f);
        for(int i=0;i<waves.count;i++){
            const Crest& c=waves.crests[i];
            if(c.center.x>=0&&c.center.x<WorldW&&c.center.y>=0&&c.center.y<H&&c.opacity>=.20f&&c.height>=10)
                displayedCrests++;
        }
    }
    report<<"Visible crests across 20 arrival samples: "<<displayedCrests<<'\n';
    check(displayedCrests>=180,"the forest arrival sustains the denser crest field over a full wave cycle");
    Crest crest{{224,505},{.94f,.342f},{-.342f,.94f},137,98,48,1,2.4f,.45f};
    auto profileTop=[&](const Crest& shape){
        BeginTextureMode(sample);ClearBackground(BLANK);
        BeginShaderMode(forest_wave_crests::material);
        SetShaderValue(forest_wave_crests::material,forest_wave_crests::timeLocation,&game.time,SHADER_UNIFORM_FLOAT);
        drawProfile(shape,origin,forest_water::variation,game.time,Face,false);
        EndShaderMode();EndTextureMode();
        Image profile=LoadImageFromTexture(sample.texture);ImageFlipVertical(&profile);
        auto* colors=static_cast<Color*>(profile.data);int top=H;
        for(int y=0;y<H&&top==H;y++)for(int x=213;x<235;x++)if(colors[y*WorldW+x].a>90){top=y;break;}
        UnloadImage(profile);return top;
    };
    int raisedTop=profileTop(crest);crest.height=0;
    int flatTop=profileTop(crest);
    report<<"Raised crest silhouette: "<<flatTop-raisedTop<<" pixels above the flat profile"<<'\n';
    check(raisedTop>=0&&flatTop<H&&flatTop-raisedTop>=55,"large waves physically extend above their projected water footprint");
    game.player={72,72};
    check(collect(screenFlat({0,0}),1.4f).count==0,"raised waves are culled in the interior of the forest");
    UnloadRenderTexture(sample);refreshDisplayLayout();size_t prepared=typography.atlasCount();
    for(int scenario=0;scenario<10;scenario++){
        game=Game{};welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;mouse={0,0};dragging={};
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
    game=Game{};welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;mouse={0,0};dragging={};refreshDisplayLayout();renderScene();
    BeginDrawing();ClearBackground(BLACK);presentCanvas();Image screenshot=presentedImage();ExportImage(screenshot,"qa-v18/optimized-fullscreen.png");UnloadImage(screenshot);EndDrawing();finishFramePacing();
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}


