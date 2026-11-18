#pragma once
// Exported from art/hero/traveler-performance/bow-arms.sif.
namespace ember::hero_bow::data {
struct DrawKey {float t;hero_motion::Vec grip,nock;};
struct LooseKey {float t;hero_motion::Vec delta;};
inline constexpr DrawKey draw[]={
 {0.00000000f,{-5.0000f,24.0000f,52.0000f},{0.0000f,16.0000f,52.0000f}},
 {0.14285714f,{-3.5000f,25.0000f,54.0000f},{1.5000f,15.6000f,54.4000f}},
 {0.33333333f,{1.0000f,26.0000f,58.0000f},{6.0000f,14.2000f,58.4000f}},
 {0.61904762f,{5.0000f,27.2000f,60.5000f},{9.4000f,10.8000f,60.7000f}},
 {0.85714286f,{6.4000f,27.8000f,61.1000f},{10.4000f,6.9000f,61.5000f}},
 {1.00000000f,{6.7000f,28.0000f,61.2000f},{10.7000f,5.7000f,61.8000f}},
};
inline constexpr LooseKey loose[]={
 {0.00000000f,{0.0000f,0.0000f,0.0000f}},
 {0.08333333f,{1.0000f,-2.5000f,0.7000f}},
 {0.20833333f,{3.0000f,-6.7000f,1.2500f}},
 {0.37500000f,{3.6000f,-8.2000f,0.6000f}},
 {0.66666667f,{3.1000f,-5.0000f,-0.3000f}},
 {1.00000000f,{0.0000f,0.0000f,0.0000f}},
};
}
