#pragma once
#include "grounded_gait.hpp"

namespace ember::forest_gait {
inline constexpr grounded_gait::Contact Wolf{1,.58f,17.6f,1.f,4.8f,15.5f,27.f};
inline constexpr float WolfHindDelay=.18f;
inline constexpr grounded_gait::Contact Skeleton{2,.64f,15.8f,1.f,6.3f,15.5f,34.f,.999f,.43f,11.f};
inline constexpr float PawHeight=2.6f;

struct WolfFoot {float forward,height,pitch;};
inline WolfFoot wolfFoot(float phase,float walk,bool right,bool front){
    // The rear paw lands first on each side. A planted paw countertracks the
    // actual distance advanced by grounded_gait::advance.
    phase+=(right?.5f:0.f)+(front?0.f:WolfHindDelay);
    phase-=std::floor(phase);
    if(phase<=Wolf.support)
        return {Wolf.stride*walk*(1-2*phase/Wolf.support),PawHeight,0};

    const float swing=(phase-Wolf.support)/(1-Wolf.support);
    const float tangent=-2*(1-Wolf.support)/Wolf.support;
    constexpr float ramp=.23f;
    const float cruise=(2-ramp*tangent)/(1-ramp);
    const auto rampTravel=[=](float time){
        const float u=time/ramp;
        return -1+tangent*time+(cruise-tangent)*ramp*u*u*u*(1-.5f*u);
    };
    // Integrated smoothstep ramps join the support velocity at both ends.
    // A short nearly uniform swing avoids whipping the paw back into place.
    const float travel=swing<ramp?rampTravel(swing):swing>1-ramp?-rampTravel(1-swing):
        rampTravel(ramp)+cruise*(swing-ramp);
    const float arc=std::sin(swing*AnimationPi);
    const float lift=arc*arc*arc;
    const float height=PawHeight+lift*Wolf.lift*walk;
    const float pitch=height>PawHeight?22.f*lift*std::cos(swing*AnimationPi)*walk:0.f;
    return {travel*Wolf.stride*walk,height,pitch};
}

struct WolfLeg {
    grounded_gait::Knee root,joint,wrist;
};
inline constexpr float wolfUpper(bool front){return front?9.2f:11.8f;}
inline constexpr float wolfLower(bool front){return front?15.3f:11.3f;}

inline WolfLeg solveWolfLeg(WolfFoot paw,float shoulder,float weight,bool front){
    const float upper=wolfUpper(front),lower=wolfLower(front);
    const float reach=upper+lower-.17f;
    const float f=weight*(front?.83f:.40f)+paw.forward*(front?.31f:.18f);
    const float pitch=paw.pitch*AnimationPi/180;
    const float c=std::cos(pitch),s=std::sin(pitch);
    const float setback=front?-.65f:-3.25f,height=front?3.45f:5.65f;
    const grounded_gait::Knee wrist{paw.forward+setback*c-height*s,paw.height+setback*s+height*c};
    const float df=wrist.forward-f;
    // Let the proximal joint slide at the edge of its reach, inside the
    // shoulder or flank. Both limb segments then solve at their own length.
    const float ceiling=wrist.height+std::sqrt(std::max(0.f,reach*reach-df*df));
    const float z=grounded_gait::softHeightLimit(shoulder,ceiling);
    const float dz=wrist.height-z,distance=std::hypot(df,dz);
    const float along=(upper*upper-lower*lower+distance*distance)/(2*distance);
    const float bend=(front?-1.f:1.f)*std::sqrt(std::max(0.f,upper*upper-along*along));
    return {{f,z},{f+(df*along-dz*bend)/distance,z+(dz*along+df*bend)/distance},wrist};
}
}
