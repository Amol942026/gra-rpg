#pragma once
#include "animation.hpp"

namespace ember::cinderhorn_gait {
inline constexpr float Scale=1.45f;
inline constexpr int Frames=32;
inline constexpr int ChargeFrames=16;
inline constexpr float Stride=35.f,Support=.625f;
inline constexpr float ChargeStride=73.f;

inline void advance(AnimationState& state,float dx,float dy,float dt,bool faceMovement,bool rushing=false){
 const float phase=state.phase;
 state.advance(dx,dy,dt,false,faceMovement,false);
 const float stride=rushing?ChargeStride:Stride;
 state.phase=std::fmod(phase+std::hypot((dx-dy)*40.f,(dx+dy)*20.f)/(stride*Scale),1.f);
}
}
