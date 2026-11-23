#pragma once
#include "animation.hpp"
#include "generated/desert_locomotion_data.hpp"
#include <iterator>

namespace ember::desert_motion {
inline constexpr float ScarabStride=16.f;
inline constexpr float CobraWavelength=77.f;
inline constexpr float DuneDustDuration=.64f;
inline constexpr unsigned DuneDustGrains=320;
inline constexpr unsigned DuneRimGrains=128;

inline data::Key sample(float phase){
 constexpr int frames=int(std::size(data::samples))-1;
 const float frame=(phase-std::floor(phase))*frames;
 const int index=int(frame);
 const float t=frame-index;
 const auto a=data::samples[index],b=data::samples[index+1];
 return {a.travel+(b.travel-a.travel)*t,a.lift+(b.lift-a.lift)*t,a.sway+(b.sway-a.sway)*t};
}

inline void advance(AnimationState& state,float dx,float dy,float dt,bool faceMovement,float cycleLength,float turnRate=0){
 const float phase=state.phase;
 const float x=(dx-dy)*40.f,y=(dx+dy)*20.f;
 const float travelled=std::hypot(x,y/ActorGroundSquash);
 if(travelled>.00001f){
  state.travelAxis={x/travelled,y/(ActorGroundSquash*travelled)};
  if(faceMovement){
   const bool known=state.facingInitialized;
   state.face(dx,dy);
   state.targetAngle=std::atan2(y,x);
   if(!known)state.facingAngle=state.targetAngle;
  }
 }
 state.advance(dx,dy,dt,false,false,false,turnRate);
 if(state.moving)state.phase=std::fmod(phase+travelled/cycleLength,1.f);
}

inline ActorFacing scarabStepAxis(const AnimationState& state){
 const auto travel=state.travelAxis;
 if(travel.x==0&&travel.y==0)return {0,1};
 const auto forward=actorFacing(state.facingInitialized?state.facingAngle:int(state.direction)*AnimationPi/4);
 return {travel.x*forward.y-travel.y*forward.x,travel.x*forward.x+travel.y*forward.y};
}

struct DuneFoot {float right,forward;};
inline constexpr DuneFoot DuneDustRim[]={
 {57,-3},{51,21},{38,42},{22,50},{5,56},{-22,52},{-40,39},{-52,21},
 {-57,-3},{-52,-22},{-39,-40},{-24,-51},{-5,-56},{22,-51},{41,-38},{53,-21}
};
inline DuneFoot duneFoot(int leg,float step,float sink=0){
 const float side=leg%2?1.f:-1.f;
 return {side*(59+step*7-26*sink),-29+(leg/2)*25.f+step*10};
}
}
