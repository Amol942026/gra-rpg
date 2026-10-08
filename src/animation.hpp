#pragma once
#include <cmath>

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

    void face(float dx, float dy) { direction = directionFor(dx, dy, direction); }
    void advance(float dx, float dy, float dt) {
        float travelled = std::hypot(dx, dy);
        moving = travelled > 0.00001f;
        idleTime += dt;
        if (moving) {
            face(dx, dy);
            phase = std::fmod(phase + travelled * 0.65f, 1.0f);
        } else {
            phase = 0;
        }
    }
    int walkFrame() const { return moving ? int(phase * 8) % 8 : 0; }
};
}
