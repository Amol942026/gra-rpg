#pragma once
#include <array>
#include <algorithm>
#include <cmath>
// Independent of devices: transitions are deterministic and testable silently.
struct MusicBlend {
    std::array<float,4> weights{1,0,0,0};int selected=0;
    void update(float dt,int desired){
        selected=std::clamp(desired,0,3);float blend=1-std::exp(-std::max(0.f,dt)*2.4f);
        for(int i=0;i<4;i++)weights[i]+=((i==selected?1.f:0.f)-weights[i])*blend;
    }
};
