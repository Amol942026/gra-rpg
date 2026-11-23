#pragma once
#include "animation.hpp"

namespace ember::cinderhorn_gait {
inline constexpr float Scale=1.45f;
inline constexpr int Frames=32;
inline constexpr int ChargeFrames=16;
inline constexpr float Stride=42.f,ForeSupport=41.f/60,HindSupport=40.f/60,HindLead=.23f;
inline constexpr float ChargeStride=73.f;
inline constexpr float TurnRate=7.3f;

inline void advance(AnimationState& state,float dx,float dy,float dt,bool faceMovement,bool rushing=false){
 const float phase=state.phase;
 state.advance(dx,dy,dt,false,faceMovement,false,TurnRate);
 const float stride=rushing?ChargeStride:Stride;
 const float travel=std::hypot((dx-dy)*40.f,(dx+dy)*20.f/ActorGroundSquash);
 state.phase=std::fmod(phase+travel/(stride*Scale),1.f);
}
}
