#pragma once
#include <cmath>
#include <algorithm>
#include "running_gait.hpp"
#include "sword_stow.hpp"

namespace ember {
// Clockwise screen directions, not the axes of the isometric map.
enum class Direction { E, SE, S, SW, W, NW, N, NE };
constexpr float AnimationPi = 3.14159265358979323846f;

inline Direction directionFor(float worldX, float worldY, Direction previous) {
    if (std::hypot(worldX, worldY) < 0.0001f) return previous;
    float angle = std::atan2((worldX + worldY) * 0.5f, worldX - worldY);
    int sector = int(std::round(angle / (AnimationPi / 4)));
    return static_cast<Direction>((sector + 8) % 8);
}

struct AnimationState {
    Direction direction = Direction::SE;
    float phase = 0; // Walking cycle advances with distance, independently of FPS.
    float idleTime = 0;
    bool moving = false;
    bool sprinting = false;
    float runBlend = 0;
    float swordStow = 0; // Reversible Synfig timeline: 0 in hand, 1 in scabbard.
    float moveBlend = 0, facingAngle = AnimationPi/4, targetAngle = AnimationPi/4;
    bool motionInitialized = false, facingInitialized = false;
    void stopSprint(){sprinting=false;runBlend=0;}

    void face(float dx, float dy) {
        direction = directionFor(dx, dy, direction);
        targetAngle = int(direction)*(AnimationPi/4);
        if(!facingInitialized){facingAngle=targetAngle;facingInitialized=true;}
    }
    void advance(float dx, float dy, float dt, bool sprint = false, bool faceMovement = true,bool allowStow=true) {
        float travelled = std::hypot(dx, dy);
        moving = travelled > 0.00001f;
        idleTime += dt;
        motionInitialized=true;
        moveBlend=(moving?1.f:0.f)+(moveBlend-(moving?1.f:0.f))*std::exp(-18.f*std::max(0.f,dt));
        if(moveBlend<.0001f)moveBlend=0;
        sprinting=moving&&sprint;
        swordStow=sword_stow::advance(swordStow,sprinting&&allowStow,dt);
        float target=sprinting?1.f:0.f;
        float before=runBlend,decay=std::exp(-14.f*std::max(0.f,dt));
        float meanBlend=dt>0?target+(before-target)*(1-decay)/(14.f*dt):before;
        runBlend=target+(before-target)*decay;
        if(runBlend<.0001f)runBlend=0;
        if(runBlend>.9999f)runBlend=1;
        if (moving) {
            if(faceMovement)face(dx, dy);
            phase = std::fmod(phase + travelled * (0.38f-0.07f*meanBlend), 1.0f);
        }
        float turn=std::remainder(targetAngle-facingAngle,2*AnimationPi);
        facingAngle+=turn*(1-std::exp(-22.f*std::max(0.f,dt)));
    }
    int walkFrame() const { return moving ? int(phase * 8) % 8 : 0; }
};
}
