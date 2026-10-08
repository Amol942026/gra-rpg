#pragma once
#include "frame_pacing.hpp"

struct WindowPlacement { Vector2 position{};int width=W,height=H;bool maximized=false; } windowPlacement;
bool fullScreen(){return IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)||IsWindowFullscreen();}
Rectangle displayButton(){return {704,25,181,36};}
void toggleFullScreen(){
    dragging={};audioDragging=-1;
    if(!fullScreen()){
        windowPlacement={GetWindowPosition(),GetScreenWidth(),GetScreenHeight(),IsWindowMaximized()};
        if(windowPlacement.maximized)RestoreWindow();
        ToggleBorderlessWindowed();
    }else{
        if(IsWindowFullscreen())ToggleFullscreen();else ToggleBorderlessWindowed();
        SetWindowSize(windowPlacement.width,windowPlacement.height);
        SetWindowPosition(int(windowPlacement.position.x),int(windowPlacement.position.y));
        if(windowPlacement.maximized)MaximizeWindow();
    }
    updateFramePacing(true);
}
bool displayAction(bool keyPressed,bool leftPressed){
    if(!keyPressed&&!(leftPressed&&inside(displayButton())))return false;
    toggleFullScreen();return true;
}
bool displayInput(){return displayAction(IsKeyPressed(KEY_F11),IsMouseButtonPressed(MOUSE_BUTTON_LEFT));}
void drawDisplayButton(){
    Rectangle r=displayButton();panel(r,inside(r)?Card:Panel);
    // The icon leaves room for readable text at the smallest supported size.
    for(int i=0;i<4;i++){
        float x=r.x+12+(i%2)*12,y=r.y+11+(i/2)*13;
        line(x,y,x+(i%2?-4:4),y,Gold,1.5f);
        line(x,y,x,y+(i/2?-4:4),Gold,1.5f);
    }
    center(fullScreen()?"OKNO · F11":"PEŁNY EKRAN · F11",r.x+108,r.y+10,14,Gold);
}
