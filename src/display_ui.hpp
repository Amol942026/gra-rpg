#pragma once
#include "frame_pacing.hpp"

struct WindowPlacement { Vector2 position{};int width=W,height=H;bool maximized=false; } windowPlacement;
bool fullScreen(){return IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)||IsWindowFullscreen();}
void toggleFullScreen(){
    game.cancelBow();dragging={};audioDragging=-1;
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
void setFullScreen(bool enabled){
    if(fullScreen()!=enabled)toggleFullScreen();
}
bool displayAction(bool keyPressed){
    if(!keyPressed)return false;
    toggleFullScreen();return true;
}
