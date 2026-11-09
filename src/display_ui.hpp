#pragma once
#include "frame_pacing.hpp"

// GLFW reports the work area and the decorated frame in the same native
// coordinates used by SetWindowPosition, including per-monitor DPI.
struct GLFWwindow;
struct GLFWmonitor;
using WindowIconifyCallback=void (*)(GLFWwindow*,int);
extern "C" GLFWwindow* glfwGetCurrentContext();
extern "C" GLFWmonitor** glfwGetMonitors(int*);
extern "C" void glfwGetMonitorWorkarea(GLFWmonitor*,int*,int*,int*,int*);
extern "C" void glfwGetWindowFrameSize(GLFWwindow*,int*,int*,int*,int*);
extern "C" WindowIconifyCallback glfwSetWindowIconifyCallback(GLFWwindow*,WindowIconifyCallback);

struct WindowWorkArea {int x=0,y=0,width=0,height=0;};
struct WindowFrame {int left=0,top=0,right=0,bottom=0;};
struct WindowPlacement {int width=W,height=H;bool maximized=false;} windowPlacement;
WindowIconifyCallback previousIconifyCallback=nullptr;
bool centerAfterRestore=false;

bool fullScreen(){return IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)||IsWindowFullscreen();}
WindowWorkArea windowWorkArea(int monitor){
    int count=0;GLFWmonitor** monitors=glfwGetMonitors(&count);
    WindowWorkArea area;
    if(monitors&&count>0)glfwGetMonitorWorkarea(monitors[std::clamp(monitor,0,count-1)],&area.x,&area.y,&area.width,&area.height);
    return area;
}
WindowFrame windowFrame(){
    WindowFrame frame;
    glfwGetWindowFrameSize(glfwGetCurrentContext(),&frame.left,&frame.top,&frame.right,&frame.bottom);
    return frame;
}
void centerWindowed(int monitor,int width,int height){
    const auto area=windowWorkArea(monitor);
    const auto frame=windowFrame();
    const int clientWidth=area.width-frame.left-frame.right;
    const int clientHeight=area.height-frame.top-frame.bottom;
    if(clientWidth<=0||clientHeight<=0)return;
    SetWindowMinSize(std::min(1152,clientWidth),std::min(720,clientHeight));
    width=std::min(width,clientWidth);height=std::min(height,clientHeight);
    if(width!=GetScreenWidth()||height!=GetScreenHeight())SetWindowSize(width,height);
    SetWindowPosition(area.x+frame.left+(clientWidth-width)/2,area.y+frame.top+(clientHeight-height)/2);
}
void windowIconifyChanged(GLFWwindow* window,int iconified){
    if(previousIconifyCallback)previousIconifyCallback(window,iconified);
    if(!iconified)centerAfterRestore=true;
}
void watchWindowRestore(){
    previousIconifyCallback=glfwSetWindowIconifyCallback(glfwGetCurrentContext(),windowIconifyChanged);
}
void updateWindowPlacement(){
    if(!centerAfterRestore||IsWindowMinimized())return;
    centerAfterRestore=false;
    if(!fullScreen()&&!IsWindowMaximized())centerWindowed(GetCurrentMonitor(),GetScreenWidth(),GetScreenHeight());
}
void toggleFullScreen(){
    game.cancelBow();dragging={};audioDragging=-1;
    if(!fullScreen()){
        const bool maximized=IsWindowMaximized();
        if(maximized)RestoreWindow();
        windowPlacement={GetScreenWidth(),GetScreenHeight(),maximized};
        ToggleBorderlessWindowed();
    }else{
        const int monitor=GetCurrentMonitor();
        if(IsWindowFullscreen())ToggleFullscreen();else ToggleBorderlessWindowed();
        centerWindowed(monitor,windowPlacement.width,windowPlacement.height);
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
