#pragma once
#include "grounded_gait.hpp"
#include "generated/bandit_motion_data.hpp"

namespace ember::bandit_gait {
inline grounded_gait::Pose pose(const AnimationState& state,float walk,float hipOffset=0){
    return grounded_gait::fromStanding(state,walk,grounded_gait::Bandit,hipOffset);
}

inline bandit_motion::Key blend(bandit_motion::Key a,bandit_motion::Key b,float t){
    auto mix=[t](float x,float y){return x+(y-x)*t;};
    return {mix(a.lean,b.lean),mix(a.bob,b.bob),mix(a.arm,b.arm),mix(a.raise,b.raise),
            mix(a.cloth,b.cloth),mix(a.hip,b.hip)};
}

template<size_t N>
inline bandit_motion::Key sample(const bandit_motion::Key (&keys)[N],float t){
    float frame=std::clamp(t,0.f,1.f)*(N-1);
    size_t i=std::min(size_t(frame),N-2);
    return blend(keys[i],keys[i+1],frame-i);
}
}
