#pragma once
#include "shadow_math.hpp"
// Contact points measured on the painted sprites, in logical scenery-cell pixels.
// Each point has its own depth in the isometric ground plane.
namespace marsh_footprints {
struct Foot {float x,y,rx,ry;};
constexpr Foot willow[]={{20,189,8,2},{42,204,12,3},{68,203,15,4},{89,193,18,6},{111,200,14,4},{131,204,9,3},{154,191,7,2}};
constexpr Foot reeds[]={{49,195,6,2},{68,201,9,3},{91,205,13,4},{107,203,9,3},{128,195,6,2}};
constexpr Foot hut[]={{11,190,4.8f,2.6f},{46,198,4.5f,2.7f},{83,207,5.4f,3},{114,207,5,2.7f},{151,201,5,2.4f},{166,184,3.5f,2.2f}};
constexpr Foot belfry[]={{24,201,10,4},{45,203,15,4},{77,204,18,4},{96,201,15,4},{127,205,16,4},{148,199,9,3}};
constexpr Foot hull[]={{35,189,5,3},{49,197,10,4},{66,192,11,5},{91,184,13,5},{110,175,10,4},{131,162,6,3},{143,145,3,2}};
constexpr Foot net[]={{45,206,6,3},{136,197,5,2.8f}};
constexpr Foot mill[]={{23,203,10,4},{46,182,4,2},{92,206,16,5},{134,201,8,4},{158,198,6,3}};
constexpr Foot shrine[]={{57,201,7,3},{75,205,14,4},{91,207,16,4},{110,202,11,3},{126,196,5,2}};
constexpr Foot fishTrap[]={{50,202,3,1.5f},{65,207,10,2.5f},{78,203.5f,8,2.5f},{94,196,6,2},{106,192,3,2}};
constexpr Foot peatCart[]={{43,189.5f,6,2.4f},{108,161,5,2},{136,207,3.6f,1.8f},{160,197,3.2f,1.6f}};
// Chest coordinates use the same (90,208) origin as props, translated from the 256 px cells.
constexpr Foot closed[]={{2,199,5,3},{40,227,8,4},{50,227,11,4},{91,214,13,5},{142,201,12,5},{169,197,6,3}};
constexpr Foot opened[]={{12,200,6,3},{45,227,8,4},{53,227,10,4},{92,216,14,5},{142,205,11,5},{170,201,6,3}};
constexpr Foot gate[]={{42,213,14,4},{59,211,9,4},{86,202,8,3},{121,212,12,4},{138,213,10,4}};
struct Profile {const Foot* feet;int count;float relief,bedDepth;};
template<size_t N> constexpr Profile profile(const Foot(&feet)[N],float relief,float bedDepth){
 return {feet,int(N),relief,bedDepth};
}
constexpr Profile profiles[]={profile(willow,.81f,10),profile(reeds,.76f,5),profile(hut,.73f,14),
 profile(belfry,.78f,10),profile(hull,.40f,7),profile(net,.74f,4),profile(mill,.74f,11),profile(shrine,.73f,8),
 profile(fishTrap,.43f,4),profile(peatCart,.67f,5),profile(closed,.67f,12),profile(opened,.67f,12),profile(gate,.78f,8)};
constexpr int ClosedChest=PropTypeCount-MarshPropStart,OpenChest=ClosedChest+1,Portal=ClosedChest+2;
static_assert(std::size(profiles)==Portal+1);
inline float groundRow(const Profile& p,float x){
 for(int i=1;i<p.count;i++){
  const Foot& a=p.feet[i-1];const Foot& b=p.feet[i];
  if(x<=b.x)return a.y+(b.y-a.y)*std::clamp((x-a.x)/(b.x-a.x),0.f,1.f);
 }
 return p.feet[p.count-1].y;
}
inline Vector2 castPoint(const Profile& profile,float x,float y){
 const float h=std::max(0.f,groundRow(profile,x)-y);
 return {x+shadow_math::LightX*h*profile.relief,y+h*(1+shadow_math::LightY*profile.relief)};
}
}
