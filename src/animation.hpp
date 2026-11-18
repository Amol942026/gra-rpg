#pragma once
#include <cmath>
#include <algorithm>
#include "running_gait.hpp"
#include "sword_stow.hpp"

namespace ember {
// Clockwise screen directions, not the axes of the isometric map.
enum class Direction { E, SE, S, SW, W, NW, N, NE };
constexpr float AnimationPi = 3.14159265358979323846f;
inline constexpr float ActorGroundSquash=.52f;
struct ActorFacing {float x,y;};
inline ActorFacing actorFacing(float angle){
 const float x=std::cos(angle),y=std::sin(angle)/ActorGroundSquash;
 const float length=std::hypot(x,y);
 return {x/length,y/length};
}

inline Direction directionAtScreenAngle(float angle) {
    int sector = int(std::round(angle / (AnimationPi / 4)));
    return static_cast<Direction>((sector + 8) % 8);
}
inline Direction directionFor(float worldX, float worldY, Direction previous) {
    if (std::hypot(worldX, worldY) < 0.0001f) return previous;
    return directionAtScreenAngle(std::atan2((worldX + worldY) * 0.5f, worldX - worldY));
}

struct AnimationState {
    Direction direction = Direction::SE;
    float phase = 0; // Walking cycle advances with distance, independently of FPS.
    float idleTime = 0;
    bool moving = false;
    bool sprinting = false;
    float runBlend = 0;
    float swordStow = 0; // Saved melee carry timeline: 0 in hand, 1 stored.
    float moveBlend = 0, facingAngle = AnimationPi/4, targetAngle = AnimationPi/4;
    bool motionInitialized = false, facingInitialized = false;
    ActorFacing travelAxis{}; // Transient locomotion direction in unsquashed screen coordinates.
    float turnLean=0; // Filtered turning velocity for articulated directional sprites.
    float pivotPhase=0; // Transient footwork driven by angular travel while turning.
    void stopSprint(){sprinting=false;runBlend=0;}

    struct SprintStep {float end, average;};
    SprintStep sprintStep(bool sprint,float dt)const {
        constexpr double duration=.42;
        const float target=sprint?1.f:0.f;
        if(runBlend==target||dt<=0)return {runBlend,runBlend};
        // The saved weight uniquely determines the position on this reversible
        // ease-in/out. Feet, pace and equipment never restart when Shift changes.
        const double start=runBlend<.5f?std::sqrt(.5*runBlend):1-std::sqrt(.5*(1-runBlend));
        const double delta=(sprint?1:-1)*dt/duration;
        const double end=std::clamp(start+delta,0.,1.);
        const double edge=std::min(end,1-end);
        const double weight=end<.5?2*edge*edge:1-2*edge*edge;
        const auto integral=[](double u){
            const double edge=std::min(u,1-u);
            return std::max(0.,u-.5)+(2./3)*edge*edge*edge;
        };
        // Include any part of the step spent at full running/walking speed.
        const double mean=(integral(end)-integral(start)+(delta-end+start)*target)/delta;
        return {float(weight),float(std::clamp(mean,0.,1.))};
    }

    void face(float dx, float dy) {
        direction = directionFor(dx, dy, direction);
        targetAngle = int(direction)*(AnimationPi/4);
        if(!facingInitialized){facingAngle=targetAngle;facingInitialized=true;}
    }
    void faceAim(float dx,float dy,bool alignOnSpawn) {
        targetAngle=std::atan2((dx+dy)*.5f,dx-dy);
        if(dx*dx+dy*dy>=1.e-8f)direction=directionAtScreenAngle(targetAngle);
        if(!facingInitialized){
            facingAngle=alignOnSpawn?targetAngle:int(direction)*(AnimationPi/4);
            facingInitialized=true;
        }
    }
    void advanceFacing(float dt,float maxTurnRate) {
        const float turn=std::remainder(targetAngle-facingAngle,2*AnimationPi);
        if(maxTurnRate<=0) {
            facingAngle+=turn*(1-std::exp(-22.f*dt));
            return;
        }
        if(dt<=0)return;
        // Travel at a bounded rate, then ease into the target. Integrating the
        // two stages separately keeps the same heading across different frame rates.
        constexpr float response=15.f;
        const float angle=std::abs(turn),knee=maxTurnRate/response;
        const float cruise=std::min(dt,std::max(0.f,(angle-knee)/maxTurnRate));
        const float remaining=(angle-maxTurnRate*cruise)*std::exp(-response*(dt-cruise));
        const float step=std::copysign(angle-remaining,turn);
        facingAngle=std::remainder(facingAngle+step,2*AnimationPi);
        const float lean=std::clamp(step/(maxTurnRate*dt),-1.f,1.f);
        turnLean+=(lean-turnLean)*(1-std::exp(-12.f*dt));
    }
    void advance(float dx, float dy, float dt, bool sprint = false, bool faceMovement = true,bool allowStow=true,float maxTurnRate=0) {
        float travelled = std::hypot(dx, dy);
        moving = travelled > 0.00001f;
        idleTime += dt;
        motionInitialized=true;
        moveBlend=(moving?1.f:0.f)+(moveBlend-(moving?1.f:0.f))*std::exp(-18.f*std::max(0.f,dt));
        if(moveBlend<.0001f)moveBlend=0;
        sprinting=moving&&sprint;
        swordStow=sword_stow::advance(swordStow,sprinting&&allowStow,dt);
        const auto pace=sprintStep(sprinting,dt);
        runBlend=pace.end;
        if (moving) {
            if(faceMovement){
                if(maxTurnRate>0)faceAim(dx,dy,true);
                else face(dx,dy);
            }
            phase = std::fmod(phase + travelled * (0.38f-0.07f*pace.average), 1.0f);
        }
        advanceFacing(std::max(0.f,dt),maxTurnRate);
    }
    int walkFrame() const { return moving ? int(phase * 8) % 8 : 0; }
};
}
