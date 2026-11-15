#pragma once
#include "generated/weapon_stow_data.hpp"
#include "hero_carry.hpp"
#include "items.hpp"

namespace ember::weapon_stow {
using namespace hero_motion;
namespace bone=hero_motion::data;
inline constexpr bool supported(Kind shape){
 return shape==Kind::Dagger||shape==Kind::Axe||shape==Kind::Spear;
}
inline constexpr Vec DaggerMouth{17.6f,6.8f,35.3f};
inline constexpr Vec AxeSocket{-2,-14,31},SpearSocket{-3,-14,43};
inline Vec shaft(float yaw,float pitch){
 const float a=yaw*AnimationPi/180,b=pitch*AnimationPi/180;
 return {std::sin(a)*std::cos(b),std::cos(a)*std::cos(b),std::sin(b)};
}
inline Vec storedAxis(Kind shape){
 if(shape==Kind::Dagger)return shaft(37,-78);
 return shape==Kind::Axe?Vec{0,0,1}:shaft(90,70);
}
inline Vec socket(Kind shape,float backDepth=14){
 if(shape==Kind::Dagger)return DaggerMouth-storedAxis(shape)*3.4f;
 const Vec rest=shape==Kind::Axe?AxeSocket:SpearSocket;
 return {rest.x,-backDepth,rest.z};
}
inline data::Key sample(Kind shape,float phase){
 const auto& keys=shape==Kind::Dagger?data::dagger:shape==Kind::Axe?data::axe:data::spear;
 constexpr int frames=std::size(data::axe)-1;
 const float f=std::clamp(phase,0.f,1.f)*frames;
 const int i=std::min(int(f),frames-1);const float w=f-i;
 const auto a=keys[i],b=keys[i+1];
 const auto lerp=[w](float x,float y){return x+(y-x)*w;};
 return {lerp(a.right,b.right),lerp(a.forward,b.forward),lerp(a.height,b.height),
         lerp(a.yaw,b.yaw),lerp(a.pitch,b.pitch),lerp(a.reach,b.reach),lerp(a.seat,b.seat)};
}
struct Mount {Vec grip,axis,across;float daggerVisible=1,spearSlide=0;};

inline Mount apply(Pose& pose,const Pose& natural,Kind shape,float phase,float spearSlide,float backDepth=14){
 const auto key=sample(shape,phase);
 const bool dagger=shape==Kind::Dagger,spear=shape==Kind::Spear;
 const int anchor=dagger?bone::Pelvis:bone::Spine;
 const Vec mount=pose.point(anchor,socket(shape,backDepth));
 const Vec mountedAxis=pose.vector(anchor,storedAxis(shape));
 const Quat carried=pose.skin[bone::Weapon].q;
 const auto blade=sword_pose::idleBlade(0);
 const Vec bladeAxis{blade.r,blade.f,blade.z};
 Vec hand=pose.world[bone::HandR].p,axis=spear?pose.world[bone::HandL].p-hand:rotate(carried,bladeAxis);
 axis=axis*(1/std::max(.001f,length(axis)));
 if(phase<=0)return {hand,axis,rotate(carried,{1,0,0}),1,spearSlide};
 const float blend=std::min(1.f,phase*7.5f);
 const Vec authoredAxis=pose.vector(anchor,shaft(key.yaw,key.pitch));
 const Vec localWidth=dagger?Vec{.80f,-.60f,0}:Vec{0,-1,0};
 const float edge=shape==Kind::Axe?-1.f:1.f;
 const Vec restAxis=storedAxis(shape);
 const Vec backWidth=dagger?localWidth:Vec{restAxis.z*edge,0,-restAxis.x*edge};
 const Quat seated=hero_carry::bladeOrientation(mountedAxis,pose.vector(anchor,backWidth));
 const Quat moving=hero_carry::bladeOrientation(authoredAxis,pose.vector(anchor,localWidth));
 const Quat authored=dagger?moving:mix(moving,seated,ease((phase-.38f)/.245f));
 const Quat carriedAxis=spear?fromTo(bladeAxis,axis):carried;
 const auto orientation=mix(carriedAxis,authored,ease(blend));
 const float fit=dagger?0.f:(14-backDepth)*ease((phase-.40f)/.225f);
 const Vec authoredGrip=pose.point(anchor,{key.right,key.forward+fit,key.height});
 const Vec target=key.seat>=.9999f?mount:authoredGrip;
 const bool latched=key.seat>=.9999f;
 const Vec naturalHand=natural.world[bone::HandR].p;
 if(latched)hand=naturalHand*(1-key.reach)+mount*key.reach;
 else hand=hand*(1-key.reach)+target*key.reach;
 const Vec shoulder=pose.world[bone::UpperArmR].p;
 const Vec carryPole=(pose.world[bone::ForearmR].p-shoulder)*(1.f/13);
 const Vec freePole=(natural.world[bone::ForearmR].p-natural.world[bone::UpperArmR].p)*(1.f/13);
 const Vec pole=(latched?freePole:carryPole)*(1-key.reach)+pose.vector(anchor,{1,-.23f,.29f})*key.reach;
 hero_carry::placeGrip(pose,false,hand,pole,latched?mix(natural.skin[bone::HandR].q,seated,key.reach):orientation);
 if(spear){
  // The supporting hand lets go early and returns to the normal running swing.
  const float release=ease(phase/.36f);
  const auto left=pose.world[bone::HandL];
  const Vec free=natural.world[bone::HandL].p;
  const Vec leftShoulder=pose.world[bone::UpperArmL].p;
  const Vec leftPole=(pose.world[bone::ForearmL].p-leftShoulder)*(1.f/13);
  const Vec freeLeftPole=(natural.world[bone::ForearmL].p-natural.world[bone::UpperArmL].p)*(1.f/13);
  hero_carry::placeGrip(pose,true,left.p*(1-release)+free*release,leftPole*(1-release)+freeLeftPole*release,
      mix(pose.skin[bone::HandL].q,natural.skin[bone::HandL].q,release));
 }
 if(phase>=1){
  for(int index:{bone::UpperArmL,bone::ForearmL,bone::HandL,bone::UpperArmR,bone::ForearmR,bone::HandR})
   pose.world[index]=natural.world[index];
 }
 const float lock=dagger?ease((key.seat-.86f)/.14f):ease(key.seat);
 const Vec grip=pose.world[bone::HandR].p*(1-lock)+mount*lock;
 const Quat weapon=mix(orientation,seated,lock);
 pose.world[bone::Weapon]={grip,mul(weapon,restRotation(bone::Weapon))};
 pose.bind();
 axis=rotate(weapon,bladeAxis);axis=axis*(1/length(axis));
 return {grip,axis,rotate(weapon,{1,0,0}),dagger?1-.88f*key.seat:1,spearSlide*(1-ease(blend))};
}
}
