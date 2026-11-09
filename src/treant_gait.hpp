#pragma once
#include "grounded_gait.hpp"

namespace ember::treant_gait {
// Reuse the elemental Synfig clip with its original linear 70% contact.
inline constexpr grounded_gait::Contact Contact{5,.7f,22.f,1.18f,9.f,23.5f,48.2f};
inline constexpr float Support=Contact.support,Stride=Contact.stride,Scale=Contact.scale;
inline float phaseDistance(float dx,float dy){
    return grounded_gait::phaseDistance(dx,dy,Contact);
}
inline void advance(AnimationState& state,float dx,float dy,float dt){
    grounded_gait::advance(state,dx,dy,dt,Contact);
}
}
