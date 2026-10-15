#pragma once

bool runMotionQA(bool expectedFullscreen){
    fs::create_directories("qa-v16");std::ofstream report("qa-v16/motion-tests.txt");int checks=0,failures=0;
    auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
    check(fullScreen()==expectedFullscreen,"launch uses the requested mode (fullscreen by default)");
    check(!audioSystem.available,"development diagnostic has no audio output device");
    check(IsWindowState(FLAG_VSYNC_HINT)&&swapInterval>=1,"presentation synchronization is enabled");
    check(intervalForRefresh(60)==1&&intervalForRefresh(120)==2&&intervalForRefresh(144)==2&&intervalForRefresh(240)==4&&intervalForRefresh(360)==4,
          "refresh cadence uses whole monitor cycles");
    report<<"Monitor: "<<refreshHz<<" Hz; interval: "<<swapInterval<<"; clock: "<<pacingName()<<'\n';
    game=Game{};game.enemies.clear();welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;mouse={0,0};dragging={};
    refreshDisplayLayout();
    std::vector<double> times;double previous=0;V runStart=game.player,rollStart{},rollEnd{},runEnd{};
    for(int frame=0;frame<264;frame++){
        double begin=GetTime();
        if(frame>60)times.push_back((begin-previous)*1000);previous=begin;
        if(frame==60)runStart=game.player;
        if(frame==150){runEnd=game.player;rollStart=game.player;game.startRoll({1,-1});}
        if(frame==174)rollEnd=game.player;
        V move=frame<60?V{}:frame<174?V{1,-1}:V{-1,1};
        // Fixed input duration measures movement independently from GPU timing;
        // the real display path below still waits for each monitor presentation.
        game.update(1.f/60,move,false,true);
        renderScene();BeginDrawing();presentCanvas();EndDrawing();finishFramePacing();
    }
    check(std::abs(distance(runEnd,runStart)-WalkSpeed*RunMultiplier*1.5f)<.02f,"running keeps its intended speed while the camera scrolls");
    check(std::abs(distance(rollEnd,rollStart)-RollDuration*RollSpeed)<.02f,"rolling keeps its distance while the camera scrolls");
    std::sort(times.begin(),times.end());double mean=std::accumulate(times.begin(),times.end(),0.0)/times.size();
    double p95=times[size_t(times.size()*.95)],nominal=1000.0*swapInterval/refreshHz;
    report<<"Run/roll frames: mean="<<mean<<" ms, p95="<<p95<<" ms, FPS="<<1000/mean<<'\n';
    report<<"Final backend="<<pacingName()<<"; interval="<<sync_present::lastSyncInterval()
          <<"; accepted presents="<<sync_present::presentCount()<<"; occluded="<<sync_present::occludedCount()<<'\n';
    check(mean<nominal*1.12&&p95<nominal*1.5,"running and rolling maintain the monitor cadence without half-rate stalls");
    check(!worldCanvas.id,
          "world uses native resolution without repeated rescaling or filtering");
    // Fractional input must not change static glyph/texture coverage until the
    // camera advances to another physical pixel. This prevents edge crawling.
    game=Game{};game.props.clear();game.enemies.clear();game.forage.clear();game.player={15,12};game.messageTime=0;
    prepareCanvas(1.f);
    auto region=[&](){
        Image im=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);
        std::vector<unsigned char> sample;sample.reserve(150*80*3);
        for(int y=400;y<480;y++)for(int x=280;x<430;x++){Color c=pixels[y*im.width+x];sample.insert(sample.end(),{c.r,c.g,c.b});}
        UnloadImageColors(pixels);UnloadImage(im);return sample;
    };
    int stable=0,advanced=0,mismatch=0;Vector2 previousCamera{};std::vector<unsigned char> old;
    for(int frame=0;frame<12;frame++){
        float offset=frame*.12f/80;game.player={15+offset,12-offset};renderScene();auto pixels=region();Vector2 camera=cameraPixels();
        if(frame){bool sameCamera=camera.x==previousCamera.x&&camera.y==previousCamera.y;stable+=sameCamera;advanced+=!sameCamera;mismatch+=sameCamera?(pixels!=old):(pixels==old);}
        previousCamera=camera;old=std::move(pixels);
    }
    report<<"Fractional steps: stable native positions="<<stable<<", camera advances="<<advanced<<", coverage mismatches="<<mismatch<<'\n';
    check(stable>0&&advanced>0&&mismatch==0,"terrain retains sharp stable pixels during fractional camera phases");
    game.player={15,12};game.animation.moving=true;game.animation.phase=.1f;renderScene();
    Image a=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&a);
    game.animation.phase=.11f;renderScene();Image b=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&b);
    int changed=0;
    for(int y=320;y<440;y++)for(int x=480;x<570;x++){
        Color ca=GetImageColor(a,x,y),cb=GetImageColor(b,x,y);changed+=ca.r!=cb.r||ca.g!=cb.g||ca.b!=cb.b;
    }
    UnloadImage(a);UnloadImage(b);
    check(changed>30,"walk pose interpolates even within one of the old eight pose steps");
    V target=game.player+V{1.4f,-.6f};Vector2 projected=screen(target),returned=screen(toWorld(projected));
    check(distance(toWorld(projected),target)<.04f/renderScale&&std::hypot(projected.x-returned.x,projected.y-returned.y)<.0001f,"world picking follows the same rendered pixel anchors as the camera");
    game=Game{};game.player={8,19};game.animation.moving=true;game.animation.phase=.17f;refreshDisplayLayout();renderScene();
    BeginDrawing();ClearBackground(BLACK);presentCanvas();Image screenshot=presentedImage();ExportImage(screenshot,"qa-v16/fullscreen-motion.png");UnloadImage(screenshot);EndDrawing();finishFramePacing();
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';game=Game{};return failures==0;
}
