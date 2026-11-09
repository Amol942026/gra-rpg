#pragma once
extern "C" int glfwGetWindowAttrib(GLFWwindow*,int);

bool runEnvironmentQA(){
    if(fullScreen()){toggleFullScreen();BeginDrawing();EndDrawing();refreshDisplayLayout();}
    fs::create_directories("qa-v14");std::ofstream report("qa-v14/environment-display-tests.txt");int checks=0,failures=0;
    auto check=[&](bool ok,const char* label){checks++;if(!ok)failures++;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
    auto pump=[](){BeginDrawing();ClearBackground(Ink);EndDrawing();refreshDisplayLayout();};
    game=Game{};welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;dragging={};mouse={0,0};
    prepareCanvas(1);Prop prop{game.player,0,1};
    check(treeShift(prop,208,2)==0&&treeShift(prop,220,7)==0,"tree root and ground shadow remain anchored");
    check(std::abs(treeShift(prop,45,0)-treeShift(prop,45,2))>1,"tree crown changes position with wind");
    Prop other=prop;other.pos=other.pos+V{2,3};
    check(std::abs(treeShift(prop,45,2)-treeShift(other,45,2))>.2f,"trees have spatially staggered sway");
    check(std::abs(treeShift(prop,45,2))>std::abs(treeShift(prop,180,2)),"crown bends more than lower trunk");
    RenderTexture2D sample=LoadRenderTexture(720,600);std::vector<Color> previous;
    for(float size:{.8f,1.4f})for(float time:{0.f,1.1f,2.4f,4.7f}){
        prop.size=size;game.time=time;
        BeginTextureMode(sample);ClearBackground(BLANK);drawPropTexture(prop,propArt[0].texture,false,WHITE);EndTextureMode();
        Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);
        int compared=0,mismatches=0,opaque=0,changed=0;
        Vector2 base=screen(prop.pos);
        for(int y=int(base.y-177*size);y<int(base.y+17*size);y+=2)for(int x=int(base.x-75*size);x<int(base.x+75*size);x+=2){
            bool visible=pixels[y*720+x].a>=160,hit=propContains(prop,{x+.5f,y+.5f});
            compared++;mismatches+=visible!=hit;opaque+=visible;
        }
        report<<"Sway scale="<<size<<", t="<<time<<", silhouette mismatches="<<mismatches<<"/"<<compared<<'\n';
        check(opaque>400&&mismatches<float(compared)*.012f,"animated GPU silhouette and mouse picking agree");
        if(!previous.empty()){for(int i=0;i<720*600;i++)changed+=pixels[i].a!=previous[i].a;check(changed>150,"wind changes the rendered tree across frames");}
        previous.assign(pixels,pixels+720*600);UnloadImageColors(pixels);UnloadImage(im);
    }
    UnloadRenderTexture(sample);
    // Render the whole coastline to ensure the water moves while land stays still.
    game=Game{};game.player={3,15};game.props.clear();game.enemies.clear();game.messageTime=0;mouse={0,0};
    std::array<Image,2> images{};
    for(int i=0;i<2;i++){game.time=i*1.8f;renderScene();saveTexture(canvas,TextFormat("qa-v14/water-%d.png",i));images[i]=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&images[i]);}
    Color* a=LoadImageColors(images[0]);Color* b=LoadImageColors(images[1]);int waterChanges=0,landChanges=0,waterPixels=0;
    for(int y=340;y<670;y++)for(int x=30;x<WorldW-30;x++){
        V world=toWorld({x+.5f,y+.5f});int k=y*W+x;
        bool changed=a[k].r!=b[k].r||a[k].g!=b[k].g||a[k].b!=b[k].b;
        if(waterAt(world)&&world.x<-.5f){waterPixels++;waterChanges+=changed;}
        // Exclude actors, waves at the bank, tile contours and the bottom overlay.
        else if(world.x>1.5f&&world.x<5.5f&&world.y>8&&world.y<22&&std::abs(x-525)>100)landChanges+=changed;
    }
    report<<"Water changed pixels="<<waterChanges<<"/"<<waterPixels<<", land changes="<<landChanges<<'\n';
    check(waterChanges>700,"wind visibly animates the rendered water surface");
    check(landChanges<60,"water animation does not move or paint over land");
    UnloadImageColors(a);UnloadImageColors(b);for(Image im:images)UnloadImage(im);
    game=Game{};mouse={0,0};
    for(int i=0;i<3;i++){
        game.time=.6f+i*.44f;renderScene();saveTexture(canvas,TextFormat("qa-v14/forest-arrival-water-%d.png",i));
    }
    game=Game{};game.player={5,15};game.enemies.clear();game.props={{{3,15},0,1.4f},{{4,14},0,1},{{6,13},0,.85f}};
    for(int i=0;i<3;i++){
        game.time=i*1.1f;Vector2 base=screen(game.props[0].pos);
        mouse={base.x+treeShift(game.props[0],80,game.time)*1.4f,base.y+(80-208)*1.4f};
        check(pickWorld(mouse).kind==HoverKind::Prop,"swaying canopy retains hover and HUD information");
        renderScene();saveTexture(canvas,TextFormat("qa-v14/wind-trees-%d.png",i));
    }
    if(IsWindowHidden())ClearWindowState(FLAG_WINDOW_UNDECORATED);
    pump();
    int width=GetScreenWidth(),height=GetScreenHeight();
    auto centered=[](int monitor){
        const auto area=windowWorkArea(monitor);const auto frame=windowFrame();const auto p=GetWindowPosition();
        const float dx=2*(p.x-frame.left-area.x)+GetScreenWidth()+frame.left+frame.right-area.width;
        const float dy=2*(p.y-frame.top-area.y)+GetScreenHeight()+frame.top+frame.bottom-area.height;
        return area.width>0&&area.height>0&&std::abs(dx)<=2&&std::abs(dy)<=2&&
               GetScreenWidth()+frame.left+frame.right<=area.width&&GetScreenHeight()+frame.top+frame.bottom<=area.height;
    };
    const auto visible=[](){constexpr int GlfwVisible=0x00020004;return glfwGetWindowAttrib(glfwGetCurrentContext(),GlfwVisible)!=0;};
    // Real raylib window transitions, using the same F11 and Options actions as gameplay.
    welcome=true;paused=true;dragging={Place::Bag,0};audioDragging=0;
    check(displayAction(true),"F11 action is available even in welcome/pause");pump();
    int monitor=GetCurrentMonitor();
    check(fullScreen()&&GetScreenWidth()==GetMonitorWidth(monitor)&&GetScreenHeight()==GetMonitorHeight(monitor),"borderless fullscreen covers the monitor at its native resolution");
    check(dragging.place==Place::None&&audioDragging<0,"display transition cancels item and volume drags");
    check(welcome&&paused,"display transition preserves open menu and pause state");
    float logicalWidth=viewport.width,logicalHeight=viewport.height;
    check(std::abs(logicalWidth/logicalHeight-W/float(H))<.002f&&logicalWidth<=GetScreenWidth()&&logicalHeight<=GetScreenHeight(),"fullscreen viewport preserves aspect ratio within the monitor");
    bool bagMapping=true;
    for(int i=0;i<BagSize;i++){
        Rectangle r=bagRect(i);Vector2 p{r.x+r.width/2,r.y+r.height/2};
        Vector2 native{viewport.x+p.x*viewport.width/W,viewport.y+p.y*viewport.height/H};
        mouse={(native.x-viewport.x)*W/viewport.width,(native.y-viewport.y)*H/viewport.height};
        Slot hit=hoverSlot();bagMapping&=hit.place==Place::Bag&&hit.index==i;
    }
    check(bagMapping,"all 25 inventory cells map correctly in fullscreen");
    mouse={0,0};renderScene();saveTexture(canvas,"qa-v14/fullscreen-welcome.png");
    check(textWidth("Pełny ekran",17)<menuScreenChoice(true).width,"fullscreen label fits its Options button");
    welcome=paused=false;renderScene();saveTexture(canvas,"qa-v14/fullscreen-game.png");
    paused=true;menuPageTo(MenuPage::Options);auto windowChoice=menuScreenChoice(false);
    mouse={windowChoice.x+windowChoice.width/2,windowChoice.y+windowChoice.height/2};
    gameMenuEvent(false,0,false,true);check(!fullScreen()&&paused,"Options returns to windowed mode without leaving pause");pump();
    const auto area=windowWorkArea(monitor);const auto frame=windowFrame();
    check(!fullScreen()&&GetScreenWidth()==std::min(width,area.width-frame.left-frame.right)&&
          GetScreenHeight()==std::min(height,area.height-frame.top-frame.bottom),"window dimensions fit the work area with its decorated frame");
    check(centered(monitor),"the full window frame is centered in the monitor work area");
    check(IsWindowHidden()&&!visible(),"real fullscreen transitions leave the diagnostic window hidden");
    paused=false;mouse={500,450};check(!displayAction(false)&&!fullScreen(),"without F11 the world input does not toggle fullscreen");
    statsOpen=true;game.allocationRequired=true;float hp=game.hp,mana=game.mana,time=game.time;
    displayAction(true);pump();displayAction(true);pump();
    check(statsOpen&&game.allocationRequired&&game.hp==hp&&game.mana==mana&&game.time==time,"fullscreen switches preserve mandatory level allocation and game state");
    statsOpen=false;game.allocationRequired=false;
    SetWindowPosition(area.x+frame.left,area.y+frame.top-60);
    GLFWwindow* native=glfwGetCurrentContext();
    auto iconify=glfwSetWindowIconifyCallback(native,nullptr);
    glfwSetWindowIconifyCallback(native,iconify);
    check(iconify==windowIconifyChanged,"production restore handler is chained to the GLFW callback");
    if(iconify){
        // Replay iconify events through the owned hidden window's callback.
        // The real centering path runs without bringing any window onscreen.
        const auto before=GetWindowPosition();
        iconify(native,1);updateWindowPlacement();
        check(IsWindowMinimized()&&GetWindowPosition().y==before.y,"minimization leaves the window untouched");
        iconify(native,0);updateWindowPlacement();pump();
        check(!IsWindowMinimized()&&centered(monitor),"restoring a window from minimization recenters the decorated frame");
        check(IsWindowHidden()&&!visible(),"restore callback test never displays the diagnostic window");
    }
    game=Game{};welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;dragging={};mouse={0,0};
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
