#pragma once

// Feed the game's own GLFW event callback, without moving the Windows cursor
// or sending mouse events to any other application.
struct GLFWwindow;
using PointerCallback=void (*)(GLFWwindow*,double,double);
extern "C" GLFWwindow* glfwGetCurrentContext(void);
extern "C" PointerCallback glfwSetCursorPosCallback(GLFWwindow*,PointerCallback);
extern "C" void glfwGetWindowSize(GLFWwindow*,int*,int*);
extern "C" void glfwGetFramebufferSize(GLFWwindow*,int*,int*);
extern "C" void glfwGetCursorPos(GLFWwindow*,double*,double*);

bool runPointerQA(){
    fs::create_directories("qa-v17");std::ofstream report("qa-v17/pointer-tests.txt");int checks=0,failures=0;
    auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
    GLFWwindow* window=glfwGetCurrentContext();
    PointerCallback callback=glfwSetCursorPosCallback(window,nullptr);glfwSetCursorPosCallback(window,callback);
    check(callback!=nullptr,"real raylib mouse event callback is available");if(!callback)return false;
    check(!audioSystem.available,"pointer diagnostics never open the audio device");
    callback(window,1746,658);Vector2 before=GetMousePosition();
    report<<"Native startup: client pointer=1746,658; raylib pointer="<<before.x<<','<<before.y<<'\n';
    refreshDisplayLayout();Vector2 after=GetMousePosition();
    report<<"After layout: raylib pointer="<<after.x<<','<<after.y<<'\n';
    check(std::abs(after.x-1746)<.01f&&std::abs(after.y-658)<.01f,"native startup reads unscaled client mouse coordinates");
    auto pump=[](){BeginDrawing();ClearBackground(BLACK);EndDrawing();refreshDisplayLayout();};
    auto reset=[](){game=Game{};game.props.clear();game.enemies.clear();game.ground.clear();game.player={15,15};
        welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;dragging={};nearbyPage=0;};
    auto point=[&](Vector2 p){
        int ww,wh,fw,fh;glfwGetWindowSize(window,&ww,&wh);glfwGetFramebufferSize(window,&fw,&fh);
        // Start at the actual presented pixel, then convert to GLFW client units.
        // Do not reuse viewport/pointerToGame to generate expected input.
        double x=(presentationViewport.x+p.x*canvas.texture.width/W)*ww/fw;
        double y=(presentationViewport.y+p.y*canvas.texture.height/H)*wh/fh;
        callback(window,x,y);refreshDisplayLayout();
    };
    auto centerOf=[](Rectangle r){return Vector2{r.x+r.width*.5f,r.y+r.height*.5f};};
    if(!fullScreen())toggleFullScreen();pump();
    for(int mode=0;mode<5;mode++){
        if(mode==1){toggleFullScreen();SetWindowSize(1152,720);pump();}
        if(mode==2){SetWindowSize(1280,720);pump();}
        if(mode==3){SetWindowSize(1600,900);pump();}
        if(mode==4){displayAction(true,false);pump();}
        reset();
        report<<"Mode="<<mode<<", fullscreen="<<fullScreen()<<", window="<<GetScreenWidth()<<'x'<<GetScreenHeight()<<'\n';
        for(float inheritedDpi:{1.f,1.25f,1.5f,2.f}){
            report<<"Inherited mouse DPI="<<inheritedDpi<<'\n';bool bag=true,equipment=true;
            for(int i=0;i<BagSize;i++){
                SetMouseScale(1/inheritedDpi,1/inheritedDpi);SetMouseOffset(-13,7);
                point(centerOf(bagRect(i)));Slot hit=hoverSlot();bag&=hit.place==Place::Bag&&hit.index==i;
            }
            check(bag,"all 25 presented backpack cells hit through the real input callback");
            for(int i=0;i<6;i++){
                SetMouseScale(1/inheritedDpi,1/inheritedDpi);SetMouseOffset(-13,7);
                point(centerOf(equipRect(i)));Slot hit=hoverSlot();equipment&=hit.place==Place::Equip&&hit.index==i;
            }
            check(equipment,"all six presented equipment slots hit through the real input callback");
        }
        game.bag[24]=item(Kind::Sword,1);point(centerOf(bagRect(24)));leftClickGameplay({});
        point(centerOf(equipRect(0)));finishDrag();
        check(game.equipment[0].tier==1&&game.bag[24].tier==0,"drag from the bottom-right bag cell equips the weapon");
        V target=game.player+V{2,-2};game.enemies.push_back({901,0,target,target,100,100});
        Vector2 enemyPoint=screen(target);enemyPoint.y-=18;point(enemyPoint);
        check(pickWorld(mouse).kind==HoverKind::Enemy&&inspectWorld(pickWorld(mouse)).maxHp==100,"presented monster hover matches the information HUD");
        check(castGameplaySpell()&&distance(normalized(game.fireballs.back().velocity),normalized(target-game.player))<.001f,"fireball aims at the monster under the presented pointer");
        game.enemies[0].pos=game.player+V{1,-1};game.hit(game.enemies[0],1000);
        Enemy* body=game.findCorpse(901);body->loot={};body->loot[0]=item(Kind::Gold,0,11);
        Vector2 base=screen(body->pos),bodyPoint{};bool found=false;
        for(int y=-30;y<20&&!found;y+=2)for(int x=-40;x<40;x+=2){
            Vector2 p{base.x+x,base.y+y};if(pickWorld(p).kind==HoverKind::Corpse){bodyPoint=p;found=true;break;}
        }
        point(bodyPoint);check(found&&rightClickWorldOrInventory()&&game.openedCorpse==901,"right-click handler opens the body at its presented position");
        point(centerOf(lootRect(0)));leftClickGameplay({});point(centerOf(bagRect(19)));finishDrag();
        check(body->loot[0].empty()&&game.bag[19].kind==Kind::Gold&&game.bag[19].count==11,"presented corpse loot drags into the far-right backpack column");
        point({660,530});V ground=toWorld(mouse),expected=toWorld({660,530});
        check(distance(ground,expected)<.001f,"terrain picking matches the displayed point");
        point(centerOf(displayButton()));check(inside(displayButton()),"fullscreen HUD button uses the corrected pointer");
        if(presentationViewport.x>0){callback(window,0,GetScreenHeight()*.5);refreshDisplayLayout();
            check(mouse.x<0&&hoverSlot().place==Place::None&&!worldPointerAllowed(mouse),"black margin stays outside gameplay and inventory");}
    }
    // Restore raylib's event state to the actual hardware cursor, without moving it.
    double x,y;glfwGetCursorPos(window,&x,&y);callback(window,x,y);refreshDisplayLayout();
    reset();report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
