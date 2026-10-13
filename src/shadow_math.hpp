#pragma once
#include <algorithm>
namespace shadow_math {
// One world-space sun: upper left, above the scene. Height is in art pixels.
constexpr float LightX=.72f,LightY=.32f;
struct Point {float x,y;};
inline Point ground(float x,float y,float height){
 float h=std::max(0.f,height);return {x+LightX*h,y+LightY*h};
}
inline Point billboard(float x,float y,float anchorY){
 float h=anchorY-y;return {x+LightX*h,anchorY+LightY*h};
}
}
