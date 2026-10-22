#pragma once
#include "animation.hpp"
#include <cmath>

namespace ember::treant_gait {
// The elemental contact curve is authored in Synfig (70% stance).
// Calibrate its stride against the actual isometric ground projection and
// the treant's 1.18 visual scale. Both feet stay planted during stance,
// whether the creature patrols, pursues, slows down or meets an obstacle.
constexpr float Support=.7f,Stride=22.f,Scale=1.18f,Lift=9.f,LegLength=23.5f;
inline float phaseDistance(float dx,float dy){
 float x=(dx-dy)*40.f,y=(dx+dy)*20.f/.52f;
 return std::hypot(x,y)*Support/(2*Stride*Scale);
}
inline void advance(AnimationState& state,float dx,float dy,float dt){
 float previous=state.phase;
 if(std::hypot(dx,dy)>.00001f){
  bool initialized=state.facingInitialized;state.face(dx,dy);
  state.targetAngle=std::atan2((dx+dy)*20.f,(dx-dy)*40.f);
  if(!initialized)state.facingAngle=state.targetAngle;
 }
 state.advance(dx,dy,dt,false,false,false);
 state.phase=std::fmod(previous+phaseDistance(dx,dy),1.f);
}
struct Knee {float forward,height;};
inline Knee knee(float forward,float hipZ,float footZ){
 float dz=footZ-hipZ,d=std::hypot(forward,dz);
 float bend=std::sqrt(std::max(0.f,LegLength*LegLength-d*d*.25f));
 return {forward*.5f-dz/std::max(d,.001f)*bend,(hipZ+footZ)*.5f+forward/std::max(d,.001f)*bend};
}
}
