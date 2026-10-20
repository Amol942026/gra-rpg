#pragma once
#include "../src/hero_sheath.hpp"
// Independent swept-segment tests in the actual head/torso coordinate frames.
// Radii include hair/armour plus the widest sword's blade half-width.
namespace stow_clearance {
using namespace ember::hero_motion;
inline float dot(Vec a,Vec b){return a.x*b.x+a.y*b.y+a.z*b.z;}
inline float segmentEllipsoid(const Pose& p,int bone,Vec a,Vec b,Vec center,Vec radii){
 auto local=[&](Vec v){return rotate(conjugate(p.skin[bone].q),v-p.world[bone].p)+restHead(bone)-center;};
 auto scaled=[&](Vec v){return Vec{v.x/radii.x,v.y/radii.y,v.z/radii.z};};
 a=scaled(local(a));b=scaled(local(b));Vec d=b-a;
 float t=std::clamp(-dot(a,d)/std::max(.000001f,dot(d,d)),0.f,1.f);
 return length(a+d*t);
}
struct Clearance{float head,torso;};
inline Clearance measure(const Pose& p,ember::sword_stow::Key key){
 auto v=ember::sword_pose::idleBlade(0);Vec blade=p.vector(data::Weapon,{v.r,v.f,v.z});
 Vec hand=p.world[data::Weapon].p,a=hand+blade*.10f,b=hand+blade*ember::sword_stow::visibleFraction(key);
 Vec cross=p.vector(data::Weapon,{7,0,0}),guard=hand+blade*.10f;
 auto clearance=[&](int bone,Vec center,Vec radii){
  return std::min({segmentEllipsoid(p,bone,a,b,center,radii),
                   segmentEllipsoid(p,bone,hand-blade*.20f,hand+blade*.06f,center,radii),
                   segmentEllipsoid(p,bone,guard-cross,guard+cross,center,radii)});
 };
 return {clearance(data::Head,{0,0,72},{15,15,19}),
         clearance(data::Spine,{0,0,46},{16.5f,13,17})};
}
}
