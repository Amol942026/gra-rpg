#pragma once
#include "animation.hpp"
#include "generated/creature_motion_data.hpp"

namespace ember::guardian_gait {
// Existing Synfig humanoid.sif: 64% of the cycle is linear ground contact.
// One stance covers the full forward/backward stride at the boss's scale.
// Use resolved movement (after collisions), in the same projection as Rig.
constexpr float Support=.64f,Stride=18.f,Scale=1.38f,LegLength=17.5f;
inline float phaseDistance(float dx,float dy){
 return std::hypot((dx-dy)*40.f,(dx+dy)*20.f/.52f)*Support/(2*Stride*Scale);
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
inline float reachHeight(float forward){return std::sqrt(std::max(0.f,4*LegLength*LegLength-forward*forward));}
inline float hipHeight(float phase){
 // Rise over the planted leg until its knee is straight. During double
 // support, the more extended foot sets the height; neither leg stretches.
 float height=1000;
 for(float offset:{0.f,.5f}){
  float p=std::fmod(phase+offset,1.f);auto key=creature_motion::sample(2,p);
  if(p<=Support)height=std::min(height,4+key.lift*6+reachHeight(key.travel*Stride));
 }
 return height;
}
inline float footHeight(float phase,float hip){
 auto key=creature_motion::sample(2,phase);
 // The recovering foot clears the ground and remains within the fixed leg
 // length while the opposite support leg raises the pelvis.
 return std::max(4+key.lift*6,hip-reachHeight(key.travel*Stride));
}
inline Knee knee(float forward,float hipZ,float footZ){
 float dz=footZ-hipZ,d=std::hypot(forward,dz);
 float bend=d>=2*LegLength-.0001f?0:std::sqrt(std::max(0.f,LegLength*LegLength-d*d*.25f));
 return {forward*.5f-dz/std::max(d,.001f)*bend,(hipZ+footZ)*.5f+forward/std::max(d,.001f)*bend};
}
}
