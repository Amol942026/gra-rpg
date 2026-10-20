#pragma once
#include <algorithm>
#include "generated/sword_stow_data.hpp"
#include "sword_pose.hpp"
namespace ember::sword_stow {
using Key=data::Key;
constexpr float Duration=data::Duration;
inline constexpr sword_pose::Point Mouth{data::Mouth[0],data::Mouth[1],data::Mouth[2]},Axis{data::Axis[0],data::Axis[1],data::Axis[2]};
constexpr float BladeLength=37,SeatedGrip=5.2f;
inline Key sample(float phase){
 float p=std::clamp(phase,0.f,1.f)*(data::Frames-1);int a=std::min(int(p),data::Frames-2);float t=p-a;
 const auto& x=data::keys[a];const auto& y=data::keys[a+1];auto m=[&](float u,float v){return u+(v-u)*t;};
 return {m(x.reach,y.reach),m(x.alignment,y.alignment),m(x.insertion,y.insertion),m(x.yaw,y.yaw),m(x.lean,y.lean),
         m(x.pitch,y.pitch),m(x.handR,y.handR),m(x.handF,y.handF),m(x.handZ,y.handZ)};
}
inline float advance(float phase,bool store,float dt){return std::clamp(phase+(store?1:-1)*std::max(0.f,dt)/Duration,0.f,1.f);}
inline float visibleFraction(Key key){return (BladeLength-(BladeLength-SeatedGrip)*key.insertion)/BladeLength;}
}
