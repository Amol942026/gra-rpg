#pragma once
#include "animation.hpp"
#include "generated/forest_boar_motion_data.hpp"
#include "generated/forest_boar_walk_data.hpp"
#include <array>

namespace ember::forest_boar_motion {
enum class Action { Melee, Windup, Rush, Recovery };
struct Phase { Action action; float progress; };
inline float encoded(Action action,float progress){return 2.f*int(action)+std::clamp(progress,0.f,1.f);}
inline Phase phase(float value){
    const auto action=Action(int(std::max(0.f,value)) / 2);
    return {action,std::clamp(value-2.f*int(action),0.f,1.f)};
}
inline data::Key sample(Phase phase){
    const data::Key* clips[]={data::melee,data::windup,data::rush,data::recovery};
    const auto* clip=clips[int(phase.action)];
    const float frame=phase.progress*data::Frames;
    const int i=std::min(int(frame),data::Frames-1);
    const float u=frame-i;
    const auto mix=[u](float a,float b){return a+(b-a)*u;};
    const auto a=clip[i],b=clip[i+1];
    return {mix(a.lean,b.lean),mix(a.drop,b.drop),mix(a.pitch,b.pitch),mix(a.reach,b.reach),
            mix(a.headPitch,b.headPitch),mix(a.tail,b.tail),mix(a.scrape,b.scrape)};
}

struct Gait {float support,stride,clearance;};
inline constexpr Gait Walk{.60f,13.8f,3.85f}, Gallop{.43f,13.4f,7.5f};
inline constexpr float Hoof=2.45f;
inline constexpr float WalkHindDelay=.19f;
inline data::WalkKey walkKey(float phase){
    const float frame=std::clamp(phase,0.f,1.f)*data::WalkFrames;
    const int i=std::min(int(frame),data::WalkFrames-1);
    const float t=frame-i;
    const auto a=data::walkCycle[i],b=data::walkCycle[i+1];
    const auto mix=[t](float x,float y){return x+(y-x)*t;};
    return {mix(a.lift,b.lift),mix(a.rock,b.rock),mix(a.bob,b.bob),mix(a.pitch,b.pitch)};
}
inline constexpr Gait gait(float run){
    return {Walk.support+(Gallop.support-Walk.support)*run,Walk.stride+(Gallop.stride-Walk.stride)*run,
            Walk.clearance+(Gallop.clearance-Walk.clearance)*run};
}
inline float blend(float t){t=std::clamp(t,0.f,1.f);return t*t*(3-2*t);}
inline float gallopBlend(Phase pose){
    if(pose.action==Action::Rush)return blend(pose.progress/.19f);
    if(pose.action==Action::Recovery)return 1-blend(pose.progress/.32f);
    return 0;
}
inline void advance(AnimationState& state,float dx,float dy,float dt,Phase animation,bool faceMovement){
    const float start=state.phase;
    const float travelled=std::hypot((dx-dy)*40.f,(dx+dy)*20.f/.52f);
    if(faceMovement&&travelled>.00001f){
        const bool initialized=state.facingInitialized;
        state.face(dx,dy);
        state.targetAngle=std::atan2((dx+dy)*20.f,(dx-dy)*40.f);
        if(!initialized)state.facingAngle=state.targetAngle;
    }
    state.advance(dx,dy,dt,false,false,false);
    const auto foot=gait(gallopBlend(animation));
    state.phase=std::fmod(start+travelled*foot.support/(2*foot.stride),1.f);
}

struct Foot {float forward,height,rock=0.f;};
// While planted, the hoof travels backward at the same rate as the body moves
// forward. A Hermite swing fixes the support velocity on both landings.
inline Foot hoof(float cycle,float walk,float run,bool right,bool front){
    const auto contact=gait(run);
    const float stagger=(right?.5f:0.f)+(front?0.f:WalkHindDelay);
    const float rushStagger=front?(right?.12f:0.f):(right?.65f:.51f);
    float phase=cycle+stagger+(rushStagger-stagger)*run;
    phase-=std::floor(phase);
    if(phase<=contact.support)return {(1-2*phase/contact.support)*contact.stride*walk,Hoof};
    const float u=(phase-contact.support)/(1-contact.support),u2=u*u,u3=u2*u;
    const float tangent=-2*(1-contact.support)/contact.support;
    const float travel=-4*u3+6*u2-1+tangent*(2*u3-3*u2+u);
    const auto step=walkKey(u);
    const float gallopLift=std::sin(u*AnimationPi);
    const float lift=step.lift+(gallopLift*gallopLift-step.lift)*run;
    return {travel*contact.stride*walk,Hoof+lift*contact.clearance*walk,step.rock*walk};
}

struct Joint {float forward,height;};
struct LegDimensions {float upper,lower,cannon;};
inline constexpr LegDimensions legDimensions(bool front){
    return front?LegDimensions{9.1f,11.8f,4.8f}:LegDimensions{10.3f,9.0f,7.3f};
}
inline constexpr float hoofAnchor(bool front){return front?13.8f:-14.1f;}
inline constexpr float limbRootHeight(bool front){return front?-.3f:-2.55f;}
inline Joint ankle(Foot step,bool front){
    const auto shape=legDimensions(front);
    const float angle=(front?-.63f*step.forward+.48f*step.rock:
                              -14.f-.67f*step.forward-.41f*step.rock)*AnimationPi/180;
    return {hoofAnchor(front)+step.forward+shape.cannon*std::sin(angle),
            step.height+shape.cannon*std::cos(angle)};
}
inline float maximumHipHeight(float hipForward,Joint ankle,bool front){
    const auto shape=legDimensions(front);
    const float horizontal=ankle.forward-hipForward;
    const float reach=shape.upper+shape.lower-.16f;
    return ankle.height+std::sqrt(std::max(0.f,reach*reach-horizontal*horizontal));
}
inline Joint knee(float forward,float height,bool front){
    const auto shape=legDimensions(front);
    const float upper=shape.upper,lower=shape.lower;
    const float distance=std::hypot(forward,height);
    const float reach=std::clamp(distance,1.f,upper+lower-.01f);
    const float length=(reach*reach+upper*upper-lower*lower)/(2*reach);
    const float offset=std::sqrt(std::max(0.f,upper*upper-length*length))*(front?-1.f:1.f);
    const float norm=1/std::max(.001f,distance);
    return {(forward*length-height*offset)*norm,(height*length+forward*offset)*norm};
}

struct Leg {bool front;float side,rock;Joint hip,knee,hock,foot;};
struct LegPose {float center,pitch;std::array<Leg,4> legs;};
inline LegPose legs(const AnimationState& state,Phase phase,float walk,float run,const data::Key& key){
    const auto gaitKey=walkKey(state.phase);
    const float cycle=state.phase*2*AnimationPi;
    const float pitch=key.pitch+walk*((1-run)*gaitKey.pitch+run*4.25f*std::sin(cycle));
    const float bob=walk*((1-run)*gaitKey.bob+run*2.7f*std::cos(2*cycle))
                    +(1-walk)*std::sin(state.idleTime*2.6f)*.25f;
    LegPose result{26.2f+key.drop+bob,pitch,{}};
    const float angle=pitch*AnimationPi/180, sine=std::sin(angle),cosine=std::cos(angle);
    float heightLimit=result.center+4;
    int index=0;
    // Legs stay on their own sagittal tracks. All four roots move together
    // with the body when the next contact reaches its extension.
    for(bool front:{false,true})for(bool right:{false,true}){
        const float side=(right?1.f:-1.f)*(front?8.6f:7.7f);
        auto step=hoof(state.phase,walk,run,right,front);
        if(phase.action==Action::Windup&&front&&right){
            step.forward-=2.85f*key.scrape;
            step.height+=3.4f*key.scrape;
            step.rock+=9.f*key.scrape;
        }
        const float rootForward=hoofAnchor(front),rootHeight=limbRootHeight(front);
        // A longer step carries the shoulder or hip slightly toward its own
        // hoof. The contact stays on the ground track and the body stays level.
        const float proximalShift=.23f*(1-run)*step.forward;
        const Joint root{key.lean+rootForward*cosine-rootHeight*sine+proximalShift,
                         rootForward*sine+rootHeight*cosine};
        const Joint hock=ankle(step,front);
        heightLimit=std::min(heightLimit,maximumHipHeight(root.forward,hock,front)-root.height);
        result.legs[index++]={front,side,step.rock,root,{},hock,{rootForward+step.forward,step.height}};
    }
    constexpr float cushion=.65f;
    const float overlap=std::max(0.f,cushion-std::abs(result.center-heightLimit));
    result.center=std::min(result.center,heightLimit)-overlap*overlap/(4*cushion);
    for(auto& limb:result.legs){
        limb.hip.height+=result.center;
        const auto bend=knee(limb.hock.forward-limb.hip.forward,limb.hock.height-limb.hip.height,limb.front);
        limb.knee={limb.hip.forward+bend.forward,limb.hip.height+bend.height};
    }
    return result;
}
}
