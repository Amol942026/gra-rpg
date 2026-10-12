#pragma once

bool runPresentationQA(){
    fs::create_directories("qa-v15");std::ofstream report("qa-v15/presentation-tests.txt");int checks=0,failures=0;
    auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
    auto pump=[](){BeginDrawing();ClearBackground(BLACK);EndDrawing();refreshDisplayLayout();};
    game=Game{};welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;dragging={};mouse={0,0};
    if(fullScreen())toggleFullScreen();pump();
    int initialWidth=GetScreenWidth(),initialHeight=GetScreenHeight();Vector2 initialPosition=GetWindowPosition();
    const Color markers[]={RED,LIME,BLUE,MAGENTA};
    auto stampCorners=[&](){
        BeginTextureMode(canvas);
        for(int i=0;i<4;i++)DrawRectangle(i%2?canvas.texture.width-15:1,i/2?canvas.texture.height-15:1,14,14,markers[i]);
        EndTextureMode();
    };
    auto visibleCorners=[&](Image im){
        int found=0;
        for(int i=0;i<4;i++){
            int x=int(presentationViewport.x)+(i%2?canvas.texture.width-8:8);
            int y=int(presentationViewport.y)+(i/2?canvas.texture.height-8:8);
            if(x<0||y<0||x>=im.width||y>=im.height)continue;
            Color c=GetImageColor(im,x,y),expected=markers[i];
            found+=std::abs(int(c.r)-expected.r)<=2&&std::abs(int(c.g)-expected.g)<=2&&std::abs(int(c.b)-expected.b)<=2;
        }
        return found;
    };
    // Include the final backbuffer pass: a correct offscreen screenshot alone
    // cannot reveal a second DPI transform cropping the right/bottom of the HUD.
    toggleFullScreen();pump();renderScene();stampCorners();
    BeginDrawing();ClearBackground(BLACK);
    Matrix inherited=rlGetMatrixModelview();Vector2 dpi=GetWindowScaleDPI();
    report<<"Native fullscreen="<<GetRenderWidth()<<'x'<<GetRenderHeight()<<", DPI="<<dpi.x<<','<<dpi.y
          <<", inherited drawing scale="<<inherited.m0<<','<<inherited.m5<<'\n';
    DrawTexturePro(canvas.texture,{0,0,float(canvas.texture.width),-float(canvas.texture.height)},viewport,{0,0},0,WHITE);
    Image legacy=presentedImage();report<<"Old presentation visible corners="<<visibleCorners(legacy)<<"/4\n";
    ExportImage(legacy,"qa-v15/legacy-native-fullscreen.png");UnloadImage(legacy);EndDrawing();
    for(int mode=0;mode<4;mode++){
        if(mode==1){toggleFullScreen();SetWindowSize(1152,720);pump();}
        if(mode==2){SetWindowSize(1280,720);pump();}
        if(mode==3){SetWindowSize(1600,900);pump();}
        refreshDisplayLayout();renderScene();stampCorners();
        report<<"Mode="<<mode<<", window="<<GetScreenWidth()<<'x'<<GetScreenHeight()<<", framebuffer="<<GetRenderWidth()<<'x'<<GetRenderHeight()
              <<", canvas="<<canvas.texture.width<<'x'<<canvas.texture.height<<'\n';
        check(presentationViewport.x>=0&&presentationViewport.y>=0&&presentationViewport.x+presentationViewport.width<=GetRenderWidth()
              &&presentationViewport.y+presentationViewport.height<=GetRenderHeight(),"entire game and HUD fit within the framebuffer");
        for(float dpiScale:{1.f,1.25f,1.5f,2.f}){
            BeginDrawing();ClearBackground(BLACK);
            // Emulate the inherited 100/125/150/200% transform without changing
            // Windows display settings. The production final pass must discard it.
            rlMatrixMode(RL_MODELVIEW);rlLoadIdentity();rlScalef(dpiScale,dpiScale,1);
            presentCanvas();Image actual=presentedImage();
            report<<"DPI transform="<<dpiScale<<", visible corners="<<visibleCorners(actual)<<"/4\n";
            check(visibleCorners(actual)==4,"all four corners survive actual GPU presentation at this DPI");
            UnloadImage(actual);EndDrawing();
        }
        bool mapped=true;
        for(int i=0;i<BagSize;i++){
            Rectangle r=bagRect(i);Vector2 center{r.x+r.width*.5f,r.y+r.height*.5f};
            Vector2 pointer{viewport.x+center.x*viewport.width/W,viewport.y+center.y*viewport.height/H};
            mouse=pointerToGame(pointer);Slot hit=hoverSlot();mapped&=hit.place==Place::Bag&&hit.index==i;
        }
        check(mapped,"all backpack slots, including the far right and bottom rows, remain clickable");
        mouse={0,0};renderScene();BeginDrawing();ClearBackground(BLACK);presentCanvas();
        Image screenshot=presentedImage();ExportImage(screenshot,TextFormat("qa-v15/presented-mode-%d.png",mode));UnloadImage(screenshot);EndDrawing();
    }
    // Frame-to-frame coverage must vary smoothly, including subpixel movement.
    game=Game{};game.enemies.clear();game.props.clear();Prop prop{game.player,0,1};prepareCanvas(1);
    RenderTexture2D sample=LoadRenderTexture(720,600);std::vector<unsigned char> previous;
    int stationaryFrames=0,maxAlphaJump=0;
    for(int frame=0;frame<45;frame++){
        game.time=.4f+frame/60.f;
        BeginTextureMode(sample);ClearBackground(BLANK);drawPropTexture(prop,propArt[0].texture,false,WHITE);EndTextureMode();
        Image im=LoadImageFromTexture(sample.texture);Color* pixels=LoadImageColors(im);int changes=0;
        if(!previous.empty())for(int i=0;i<720*600;i++){
            int difference=std::abs(int(pixels[i].a)-previous[i]);changes+=difference!=0;maxAlphaJump=std::max(maxAlphaJump,difference);
        }
        if(frame>0)stationaryFrames+=changes==0;
        previous.resize(720*600);for(int i=0;i<720*600;i++)previous[i]=pixels[i].a;
        UnloadImageColors(pixels);UnloadImage(im);
    }
    report<<"Tree temporal sample: stationary frames="<<stationaryFrames<<", max alpha change="<<maxAlphaJump<<'\n';
    check(stationaryFrames==0,"tree moves in every consecutive 60 FPS frame, including fractional pixels");
    check(maxAlphaJump<70,"tree edges change gradually instead of jumping between opaque pixels");
    // Observe antialiased water at the same 60 FPS cadence.
    previous.clear();stationaryFrames=0;int maxColorJump=0;
    game.player={3,15};
    for(int frame=0;frame<45;frame++){
        game.time=.4f+frame/60.f;
        BeginTextureMode(sample);ClearBackground(rgb(22,44,47));drawWindWater();EndTextureMode();
        Image im=LoadImageFromTexture(sample.texture);Color* pixels=LoadImageColors(im);int changes=0;
        if(!previous.empty())for(int i=0;i<720*600;i++){
            int difference=std::abs(int(pixels[i].g)-previous[i]);changes+=difference!=0;maxColorJump=std::max(maxColorJump,difference);
        }
        if(frame>0)stationaryFrames+=changes==0;
        previous.resize(720*600);for(int i=0;i<720*600;i++)previous[i]=pixels[i].g;
        UnloadImageColors(pixels);UnloadImage(im);
    }
    report<<"Water temporal sample: stationary frames="<<stationaryFrames<<", max green change="<<maxColorJump<<'\n';
    check(stationaryFrames==0&&maxColorJump<25,"water shifts continuously with soft edges and no abrupt flashes");
    UnloadRenderTexture(sample);
    SetWindowSize(initialWidth,initialHeight);SetWindowPosition(int(initialPosition.x),int(initialPosition.y));pump();
    game=Game{};welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;dragging={};mouse={0,0};
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
