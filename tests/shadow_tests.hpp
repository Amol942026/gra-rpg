#pragma once
#include "../src/shadow_math.hpp"
void shadowTests(){
 for(float height:{0.f,10.f,40.f,100.f,220.f})for(float size:{.6f,1.f,2.7f}){
  auto p=shadow_math::ground(103,67,height*size);
  check(std::abs((p.x-103)-shadow_math::LightX*height*size)<.0001f&&std::abs((p.y-67)-shadow_math::LightY*height*size)<.0001f,"shadow length follows physical height and object scale in a fixed world light");
  auto translated=shadow_math::ground(103.37f,66.82f,height*size);
  check(std::abs(translated.x-p.x-.37f)<.0001f&&std::abs(translated.y-p.y+.18f)<.0001f,"ground projection retains continuous subpixel camera movement");
 }
 auto floor=shadow_math::ground(51,82,0),below=shadow_math::ground(51,82,-30);
 check(floor.x==51&&floor.y==82&&below.x==51&&below.y==82,"floor points stay anchored and below-floor geometry never extends a surface shadow");
 auto tip=shadow_math::billboard(20,-100,0),foot=shadow_math::billboard(20,0,0);
 check(tip.x>foot.x&&tip.y>foot.y,"billboard crowns cast toward the ground on the same side as physical meshes");
}
