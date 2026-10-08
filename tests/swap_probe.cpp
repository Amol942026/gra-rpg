// Silent diagnostic: deliberately never opens an audio device.
#include "raylib.h"
#include <fstream>
extern "C" void glfwSwapInterval(int interval);
extern "C" const char* glfwGetVersionString();
extern "C" void (*glfwGetProcAddress(const char*))(void);
extern "C" __declspec(dllimport) long __stdcall DwmFlush(void);
int main(){
    std::ofstream out("dist/swap-probe.txt");
    SetTraceLogLevel(LOG_WARNING);SetConfigFlags(FLAG_WINDOW_HIGHDPI|FLAG_WINDOW_RESIZABLE|FLAG_MSAA_4X_HINT|FLAG_VSYNC_HINT);
    InitWindow(1440,900,"Emberfall silent display diagnostic");SetTargetFPS(0);
    using GetString=const unsigned char*(*)(unsigned int);
    auto getString=reinterpret_cast<GetString>(glfwGetProcAddress("glGetString"));
    out<<glfwGetVersionString()<<"\nRenderer: "<<getString(0x1F01)<<"\n";out.flush();
    auto finish=reinterpret_cast<void(*)()>(glfwGetProcAddress("glFinish"));
    ToggleBorderlessWindowed();
    for(int mode=0;mode<3;mode++){
        glfwSwapInterval(mode==2?1:0);double start=0;
        for(int frame=0;frame<120;frame++){
            if(frame==30)start=GetTime();
            BeginDrawing();ClearBackground(BLACK);DrawRectangle(100,100,200,200,RED);if(mode==1)finish();EndDrawing();
            if(mode<2)DwmFlush();
        }
        out<<"Mode "<<mode<<" FPS: "<<90/(GetTime()-start)<<"\n";out.flush();
    }
    CloseWindow();
}
