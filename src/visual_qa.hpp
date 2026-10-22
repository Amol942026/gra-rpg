#pragma once
#include "native_world.hpp"
void saveTexture(RenderTexture2D target,const std::string& path) {
    Image im=LoadImageFromTexture(target.texture);
    ImageFlipVertical(&im);
    if(!ExportImage(im,path.c_str()))TraceLog(LOG_ERROR,"Cannot save visual QA: %s",path.c_str());
    UnloadImage(im);
}
void prepareCanvas(float pixelsPerUnit) {
    int width=std::max(1,int(std::floor(W*pixelsPerUnit)));
    int height=std::max(1,int(std::ceil(double(width)*H/W)));
    if(!canvas.id||canvas.texture.width!=width||canvas.texture.height!=height){
        if(canvas.id)UnloadRenderTexture(canvas);
        canvas=LoadRenderTexture(width,height);
        SetTextureFilter(canvas.texture,TEXTURE_FILTER_POINT);
    }
    renderScale=float(width)/W;
    typography.setScale(renderScale);
}
Rectangle presentationViewport{};
Rectangle fitViewport(int width,int height,int contentWidth,int contentHeight){
    return {std::floor((width-contentWidth)/2.f),std::floor((height-contentHeight)/2.f),float(contentWidth),float(contentHeight)};
}
Vector2 pointerToGame(Vector2 raw){
    return {(raw.x-viewport.x)*W/viewport.width,(raw.y-viewport.y)*H/viewport.height};
}
void refreshDisplayLayout(){
    prepareCanvas(std::min(GetRenderWidth()/float(W),GetRenderHeight()/float(H)));
    float dpiX=GetRenderWidth()/float(GetScreenWidth()),dpiY=GetRenderHeight()/float(GetScreenHeight());
    // Presentation uses framebuffer pixels; mouse coordinates use window units.
    // Keep them separate rather than applying the OS DPI scale to both again.
    presentationViewport=fitViewport(GetRenderWidth(),GetRenderHeight(),canvas.texture.width,canvas.texture.height);
    viewport={presentationViewport.x/dpiX,presentationViewport.y/dpiY,
              presentationViewport.width/dpiX,presentationViewport.height/dpiY};
    scale=viewport.width/W;
    // raylib 5.5 installs an inverse DPI mouse scale during InitWindow on
    // Windows and retains it after resizing/fullscreen. Our viewport already
    // handles that conversion; read raw client coordinates, just as drawn.
    SetMouseOffset(0,0);
    SetMouseScale(1,1);
    mouse=pointerToGame(GetMousePosition());
}
void presentCanvas(){
    // BeginDrawing() can carry a separate Windows DPI transform (e.g. 150%).
    // This final pass is explicitly one framebuffer pixel per canvas pixel.
    rlDrawRenderBatchActive();rlDisableScissorTest();
    bool shared=sync_present::begin(GetWindowHandle(),GetRenderWidth(),GetRenderHeight(),swapInterval);
    if(shared){rlEnableFramebuffer(sync_present::framebuffer());rlDisableBackfaceCulling();}
    ClearBackground(BLACK);
    rlViewport(0,0,GetRenderWidth(),GetRenderHeight());
    rlMatrixMode(RL_PROJECTION);rlLoadIdentity();
    // DirectX's first memory row is the top of the image, opposite to a native
    // OpenGL backbuffer. Write the shared surface in DirectX row order.
    if(shared)rlOrtho(0,GetRenderWidth(),0,GetRenderHeight(),0,1);
    else rlOrtho(0,GetRenderWidth(),GetRenderHeight(),0,0,1);
    rlMatrixMode(RL_MODELVIEW);rlLoadIdentity();
    DrawTexturePro(canvas.texture,{0,0,float(canvas.texture.width),-float(canvas.texture.height)},presentationViewport,{0,0},0,WHITE);
    if(shared){rlDrawRenderBatchActive();rlEnableBackfaceCulling();rlDisableFramebuffer();sync_present::end();}
}
Image presentedImage(){
    rlDrawRenderBatchActive();
    int width=0,height=0;
    if(auto* pixels=sync_present::readback(width,height))return {pixels,width,height,1,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    return {rlReadScreenPixels(GetRenderWidth(),GetRenderHeight()),GetRenderWidth(),GetRenderHeight(),1,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
}
void renderScene() {
    if(welcome){
        BeginTextureMode(canvas);ClearBackground(Ink);BeginMode2D({{0,0},{0,0},0,renderScale});
        native_world::beginOpaqueFrame();
        menu_scene::draw(menuClock);drawGameMenu();if(developerMenuOpen)drawDeveloperUi();
        native_world::finishOpaqueBackground();
        EndMode2D();EndTextureMode();performance.end(RenderStage::Hud,true);return;
    }
    if(intro.chamber()){
        world_shadows::prepare(true);
        BeginTextureMode(canvas);ClearBackground(Ink);BeginMode2D({{0,0},{0,0},0,renderScale});
        if(paused)native_world::beginOpaqueFrame();
        drawIntroChamber();drawGameMenu();drawDeveloperUi();if(paused)native_world::finishOpaqueBackground();EndMode2D();EndTextureMode();actor_art::finishShadowFrame();performance.end(RenderStage::Hud,true);return;
    }
    if(towerStory.chamber()){
        world_shadows::prepare(true);
        BeginTextureMode(canvas);ClearBackground(Ink);BeginMode2D({{0,0},{0,0},0,renderScale});
        if(paused)native_world::beginOpaqueFrame();
        drawTowerChamber();drawGameMenu();drawDeveloperUi();if(paused)native_world::finishOpaqueBackground();EndMode2D();EndTextureMode();actor_art::finishShadowFrame();performance.end(RenderStage::Hud,true);return;
    }
    hovered=pickWorld(mouse);
    performance.end(RenderStage::Picking);
    if(game.inDesert())desert_art::renderBackdrop(renderScale);
    world_shadows::prepare();
    BeginTextureMode(canvas);
    ClearBackground(Ink);
    Camera2D camera{{0,0},{0,0},0,renderScale};
    BeginMode2D(camera);
    native_world::beginOpaqueFrame();drawWorld();if(!paused)native_world::finishOpaqueBackground();actor_art::finishShadowFrame();performance.end(RenderStage::World,true);
    // World and text share the final pixel scale. The world is drawn directly
    // into this frame; there is no resized intermediate scene texture.
    drawWorldLabels();
    if(intro.active()){
        drawUI();drawIntroArrival();drawGameMenu();drawDeveloperUi();if(paused)native_world::finishOpaqueBackground();EndMode2D();EndTextureMode();performance.end(RenderStage::Hud,true);return;
    }
    drawUI();drawHoveredName();drawSkillHud();drawBowHud();drawWeaponWheel();tooltip();
    if(towerStory.active()){drawTowerStoryOverlay();drawGameMenu();drawDeveloperUi();if(paused)native_world::finishOpaqueBackground();EndMode2D();EndTextureMode();performance.end(RenderStage::Hud,true);return;}
    if(dragging.place!=Place::None){Item* p=game.get(dragging);if(p)icon(*p,mouse.x,mouse.y,.95f,.9f);}
    overlay();
    drawProgression();drawTowerPanel();
    drawAudioPanel();
    drawAudioButton(); // Always visible, including pause/help and the audio panel.
    drawDisplayButton();if(!towerPanelOpen)drawWeaponWheelButton();drawGameMenu();drawDeveloperUi();
    if(paused)native_world::finishOpaqueBackground();
    EndMode2D();EndTextureMode();performance.end(RenderStage::Hud,true);
}
void exportVisualQA() {
    fs::create_directories("qa-v02");
    fs::create_directories("assets/generated/v02");
    const char* names[]={"hero","slime","wolf","skeleton","guardian"};
    const char* states[]={"idle","walk","attack"};
    const char* compass[]={"E","SE","S","SW","W","NW","N","NE"};
    renderScale=1;typography.setScale(1);
    for(int actor=0;actor<5;actor++)for(int mode=0;mode<3;mode++){
        int frameWidth=actor==0&&mode==2?208:128;
        RenderTexture2D atlas=LoadRenderTexture(frameWidth*8,1152);
        BeginTextureMode(atlas);ClearBackground(BLANK);
        for(int direction=0;direction<8;direction++)for(int frame=0;frame<8;frame++){
            AnimationState pose;pose.direction=static_cast<Direction>(direction);
            pose.phase=frame/8.f;pose.moving=mode==1;pose.idleTime=frame*.25f;
            float attack=mode==2?std::sin((frame/7.f)*PI):0;
            actor_art::draw(actor,frame*float(frameWidth)+frameWidth*.5f,direction*144.f+125,1,pose,attack,true,true,Kind::Sword,false,BLANK,nullptr,-1,actor==0&&mode==2?frame/7.f:-1.f);
        }
        EndTextureMode();
        saveTexture(atlas,std::string("assets/generated/v02/")+names[actor]+"-"+states[mode]+".png");
        UnloadRenderTexture(atlas);
    }
    for(int mode=0;mode<3;mode++){
        RenderTexture2D sheet=LoadRenderTexture(1260,870);
        BeginTextureMode(sheet);ClearBackground(Ink);
        txt(mode==0?"OSIEM KIERUNKÓW — BEZCZYNNOŚĆ":mode==1?"OSIEM KIERUNKÓW — CHÓD":"OSIEM KIERUNKÓW — ATAK",30,20,24,Text);
        for(int direction=0;direction<8;direction++)center(compass[direction],direction*140.f+205,67,16,Gold);
        for(int actor=0;actor<5;actor++){
            const char* labels[]={"BOHATER","SZLAM","WILK","SZKIELET","STRAŻNIK"};
            txt(labels[actor],18,actor*148.f+162,14,Gold);
            for(int direction=0;direction<8;direction++){
                float x=direction*140.f+205,y=actor*148.f+220;
                diamond(x,y,52,25,rgb(30,46,44));
                AnimationState pose;pose.direction=static_cast<Direction>(direction);pose.phase=.25f;pose.moving=mode==1;
                actor_art::draw(actor,x,y,1,pose,mode==2?.9f:0,true,true);
            }
        }
        EndTextureMode();saveTexture(sheet,std::string("qa-v02/directions-")+states[mode]+".png");UnloadRenderTexture(sheet);
    }
    mouse={0,0};game=Game{};
    for(float size:{1280.f/1440,1.f,1.2f}){
        prepareCanvas(size);
        std::string suffix=std::to_string(canvas.texture.width)+"x"+std::to_string(canvas.texture.height)+".png";
        welcome=true;renderScene();saveTexture(canvas,"qa-v02/welcome-"+suffix);
        welcome=false;renderScene();saveTexture(canvas,"qa-v02/game-"+suffix);
    }
}
