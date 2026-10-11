#pragma once
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <mutex>

// Flush each stage so a stalled driver leaves a useful last line on disk.
class StartupLog {
    std::ofstream file;
    std::mutex mutex;
    std::chrono::steady_clock::time_point start=std::chrono::steady_clock::now();
public:
    void open(){std::lock_guard<std::mutex> guard(mutex);file.open("emberfall-startup.log",std::ios::trunc);}
    void write(const char* message){
        std::lock_guard<std::mutex> guard(mutex);
        if(!file)return;
        auto elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count();
        file<<elapsed<<" ms: "<<message<<'\n';file.flush();
    }
};
inline StartupLog startupLog;

inline void startupTrace(int,const char* message,va_list args){
    char text[2048];std::vsnprintf(text,sizeof(text),message,args);startupLog.write(text);
}
