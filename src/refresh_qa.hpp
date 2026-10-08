#pragma once

bool runRefreshQA(){
    fs::create_directories("qa-v19");std::ofstream report("qa-v19/refresh-tests.txt");int checks=0,failures=0;
    auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
    check(!audioSystem.available,"refresh diagnostic opens no audio device");
    game=Game{};welcome=help=paused=audioOpen=statsOpen=spellBookOpen=false;dragging={};mouse={0,0};
    bool startedFull=fullScreen();
    for(int mode=0;mode<4;mode++){
        if(mode==0&&!fullScreen())toggleFullScreen();
        if(mode==1){toggleFullScreen();SetWindowSize(1152,720);}
        if(mode==2)SetWindowSize(1280,720);
        if(mode==3)toggleFullScreen();
        PollInputEvents();refreshDisplayLayout();
        report<<"Mode="<<mode<<" framebuffer="<<GetRenderWidth()<<'x'<<GetRenderHeight()<<'\n';
        for(int frame=0;frame<8;frame++){
            if(frame==2)game.startRoll({1,-1});
            game.update(1.f/60,{1,-1},false,true);renderScene();
            Color markers[4];BeginTextureMode(canvas);
            for(int corner=0;corner<4;corner++){
                markers[corner]={static_cast<unsigned char>(40+frame*21),static_cast<unsigned char>(50+corner*47),static_cast<unsigned char>(230-frame*19),255};
                DrawRectangle(corner%2?canvas.texture.width-18:2,corner/2?canvas.texture.height-18:2,16,16,markers[corner]);
            }
            EndTextureMode();Image expected=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&expected);
            unsigned swaps=sync_present::suppressedSwaps(),presents=sync_present::presentCount()+sync_present::occludedCount();
            BeginDrawing();presentCanvas();Image actual=presentedImage();bool current=true;
            auto* sourcePixels=static_cast<unsigned char*>(expected.data);auto* targetPixels=static_cast<unsigned char*>(actual.data);
            int mismatches=0,nonBlack=0;std::array<bool,32768> palette{};
            for(int y=0;y<expected.height;y++)for(int x=0;x<expected.width;x++){
                auto* a=sourcePixels+(size_t(y)*expected.width+x)*4;
                auto* b=targetPixels+(size_t(y+int(presentationViewport.y))*actual.width+x+int(presentationViewport.x))*4;
                // The final GL pass alpha-composites the canvas over black;
                // antialiased edges can have alpha below 255 in the canvas.
                mismatches+=std::abs((int(a[0])*a[3]+127)/255-b[0])>1
                          ||std::abs((int(a[1])*a[3]+127)/255-b[1])>1
                          ||std::abs((int(a[2])*a[3]+127)/255-b[2])>1;
                nonBlack+=int(b[0])+b[1]+b[2]>10;palette[((b[0]>>3)<<10)|((b[1]>>3)<<5)|(b[2]>>3)]=true;
            }
            int colors=std::count(palette.begin(),palette.end(),true);
            report<<"Frame="<<frame<<"; full-image mismatches="<<mismatches<<"; nonblack="<<nonBlack<<"; quantized colors="<<colors<<'\n';
            if(mode==0&&frame==0){ExportImage(expected,"qa-v19/expected-scene.png");ExportImage(actual,"qa-v19/actual-scene.png");}
            check(mismatches==0,"entire destination frame matches the scene and HUD, not only corner markers");
            check(nonBlack>expected.width*expected.height*.8&&colors>64,"destination contains a complete varied scene rather than a black or flat image");
            UnloadImage(expected);
            for(int corner=0;corner<4;corner++){
                int x=int(presentationViewport.x)+(corner%2?canvas.texture.width-10:10);
                int y=int(presentationViewport.y)+(corner/2?canvas.texture.height-10:10);
                Color c=GetImageColor(actual,x,y),e=markers[corner];current&=c.r==e.r&&c.g==e.g&&c.b==e.b;
            }
            UnloadImage(actual);EndDrawing();finishFramePacing();
            check(current,"current moving frame reaches every corner in correct orientation");
            check(IsWindowState(FLAG_VSYNC_HINT)&&swapInterval>=1,"V-sync stays enabled after resize/fullscreen change");
            if(sync_present::active()){
                check(sync_present::lastSyncInterval()==swapInterval,"DXGI uses the monitor's positive synchronized interval");
                check(sync_present::suppressedSwaps()==swaps+1&&sync_present::presentCount()+sync_present::occludedCount()==presents+1,
                      "one DXGI presentation replaces one native swap without duplicate display clocks");
            }
        }
    }
    report<<"Backend="<<pacingName()<<"; accepted presents="<<sync_present::presentCount()<<"; occluded="<<sync_present::occludedCount()
          <<"; suppressed swaps="<<sync_present::suppressedSwaps()<<"; fallback="<<sync_present::error()<<'\n';
    if(fullScreen()!=startedFull)toggleFullScreen();
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';game=Game{};return failures==0;
}
