#pragma once
#include "sync_present.hpp"

// The bundled desktop raylib backend links GLFW statically. Its public swap
// interval API lets 120/144/240 Hz displays present at an even refresh cadence.
extern "C" void glfwSwapInterval(int interval);
int refreshHz=60,swapInterval=1,pacingMonitor=-1;
double nextPacingCheck=0;
int intervalForRefresh(int hz){return std::clamp(hz/60,1,4);}
void updateFramePacing(bool force=false){
    if(!force&&GetTime()<nextPacingCheck)return;
    nextPacingCheck=GetTime()+1;
    int monitor=GetCurrentMonitor();
    int hz=GetMonitorRefreshRate(monitor);if(hz<1)hz=60;
    if(!force&&monitor==pacingMonitor&&hz==refreshHz)return;
    pacingMonitor=monitor;refreshHz=hz;swapInterval=intervalForRefresh(hz);
    SetWindowState(FLAG_VSYNC_HINT);
    // Never disable native V-sync as a substitute for frame pacing. When DXGI
    // is active it owns the sole swap and uses the same positive interval.
    glfwSwapInterval(sync_present::active()?0:swapInterval);
    // Exactly one frame clock: no second software limiter racing the refresh.
    SetTargetFPS(0);
}
void finishFramePacing(){
    sync_present::present(swapInterval);
}
const char* pacingName(){return sync_present::name();}
