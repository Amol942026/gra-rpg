#pragma once
#include "grounded_gait.hpp"

namespace ember::forest_gait {
inline constexpr grounded_gait::Contact Wolf{1,.63f,13.2f,1.f,7.5f,15.5f,27.f};
inline constexpr grounded_gait::Contact Skeleton{2,.64f,15.8f,1.f,6.3f,15.5f,34.f,.999f,.43f,11.f};
inline constexpr float PawHeight=2.6f;

inline grounded_gait::Leg wolfLeg(float phase,float walk,bool right,bool front,float shoulder){
    const auto key=creature_motion::sample(Wolf.clip,phase+(right==front?.5f:0.f));
    const float forward=key.travel*Wolf.stride*walk,height=PawHeight+key.lift*Wolf.lift*walk;
    const float bend=front?-1.f:1.f;
    auto joint=grounded_gait::knee(bend*forward,shoulder,height,Wolf.legLength);
    joint.forward*=bend;
    return {forward,height,joint};
}
}
