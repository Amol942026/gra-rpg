#pragma once

extern "C" void (*glfwGetProcAddress(const char*))(void);
enum class RenderStage { Update, Picking, Water, World, Hud, Present, Count };
struct PerformanceProbe {
    bool enabled=false;
    void (*finish)()=nullptr;
    double previous=0;
    int frames=0;
    std::array<std::vector<double>,int(RenderStage::Count)> samples;
    void begin(){
        if(!enabled)return;
        if(!finish)finish=reinterpret_cast<void(*)()>(glfwGetProcAddress("glFinish"));
        previous=GetTime();frames++;
    }
    void end(RenderStage stage,bool gpu=false){
        if(!enabled)return;
        if(gpu){rlDrawRenderBatchActive();if(finish)finish();}
        double now=GetTime();if(frames>60)samples[int(stage)].push_back((now-previous)*1000);previous=now;
    }
    void save(){
        if(!enabled)return;
        std::ofstream report("performance-profile.txt");
        report<<"CPU and completed GPU work by stage (diagnostic glFinish barriers, excluding frame pacing)\n";
        const char* names[]={"Simulation/input/audio","Pointer picking","Water","World/actors","HUD","Presentation"};
        for(int i=0;i<int(RenderStage::Count);i++){
            auto values=samples[i];if(values.empty())continue;std::sort(values.begin(),values.end());
            report<<names[i]<<": mean="<<std::accumulate(values.begin(),values.end(),0.0)/values.size()
                  <<" ms; p95="<<values[size_t(values.size()*.95)]<<" ms; max="<<values.back()<<" ms\n";
        }
        report<<"Native frame buffer="<<canvas.texture.width<<'x'<<canvas.texture.height<<"\nAudio device opened="<<audioSystem.available<<'\n';
    }
} performance;
