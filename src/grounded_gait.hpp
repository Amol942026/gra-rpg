#pragma once
#include "animation.hpp"
#include "generated/creature_motion_data.hpp"
#include <array>

namespace ember::grounded_gait {
struct Contact {
    int clip;
    float support,stride,scale,lift,legLength,idleHip;
    float reachRatio=.9925f,hipCrest=.18f,swingAllowance=0.f;
};
// The human leg extends to about five degrees of flexion on the planted foot.
inline constexpr Contact Bandit{2,.64f,19.f,1.f,7.f,16.7f,32.f,.999f,.55f,14.f};

// Rig projects body-relative forward units through the .52 ground squash.
// Convert resolved movement into the same units before advancing contact.
inline float phaseDistance(float dx,float dy,const Contact& contact){
    return std::hypot((dx-dy)*40.f,(dx+dy)*20.f/.52f)*contact.support/(2*contact.stride*contact.scale);
}
inline void advance(AnimationState& state,float dx,float dy,float dt,const Contact& contact){
    const float previous=state.phase;
    if(std::hypot(dx,dy)>.00001f){
        const bool initialized=state.facingInitialized;
        state.face(dx,dy);
        state.targetAngle=std::atan2((dx+dy)*20.f,(dx-dy)*40.f);
        if(!initialized)state.facingAngle=state.targetAngle;
    }
    state.advance(dx,dy,dt,false,false,false);
    state.phase=std::fmod(previous+phaseDistance(dx,dy,contact),1.f);
}

struct Knee {float forward,height;};
inline Knee knee(float forward,float hipZ,float footZ,float legLength){
    const float dz=footZ-hipZ,d=std::hypot(forward,dz);
    const float bend=std::sqrt(std::max(0.f,legLength*legLength-d*d*.25f));
    const float ratio=bend/std::max(d,.001f);
    return {forward*.5f-dz*ratio,(hipZ+footZ)*.5f+forward*ratio};
}

inline constexpr float FootHeight=4;
struct Leg {float forward,height;Knee joint;};
struct Pose {float hip;std::array<Leg,2> legs;};
// Stay below both height limits, blending their slopes during weight transfer.
inline float softHeightLimit(float a,float b){
    constexpr float blendWidth=.8f;
    const float overlap=std::max(0.f,blendWidth-std::abs(a-b));
    return std::min(a,b)-overlap*overlap/(4*blendWidth);
}

inline Pose pose(const AnimationState& state,float walk,const Contact& contact){
    const float reach=2*contact.legLength*contact.reachRatio;
    const float shift=std::sin((state.phase-contact.support*.5f)*2*AnimationPi);
    const float walkingHip=FootHeight+reach+contact.hipCrest-1.8f*shift*shift;
    const float idleHip=contact.idleHip+std::sin(state.idleTime*2.1f)*.25f;
    const float hip=idleHip+(walkingHip-idleHip)*walk;
    Pose result{};
    std::array<float,2> hipLimits{},verticalReach{};
    for(int side=0;side<2;side++){
        const auto key=creature_motion::sample(contact.clip,state.phase+side*.5f);
        Leg& leg=result.legs[side];
        leg.forward=key.travel*contact.stride*walk;
        leg.height=FootHeight+key.lift*contact.lift*walk;
        verticalReach[side]=std::sqrt(std::max(0.f,reach*reach-leg.forward*leg.forward));
        hipLimits[side]=leg.height+verticalReach[side]+key.lift*contact.swingAllowance*walk;
    }
    result.hip=softHeightLimit(hip,softHeightLimit(hipLimits[0],hipLimits[1]));
    for(int side=0;side<2;side++){
        Leg& leg=result.legs[side];
        // Only swing keys allow extra clearance. A planted foot still fixes
        // its own reach limit and stays at ground height.
        if(contact.swingAllowance>0)leg.height=std::max(leg.height,result.hip-verticalReach[side]);
        leg.joint=knee(leg.forward,result.hip,leg.height,contact.legLength);
    }
    return result;
}
}
