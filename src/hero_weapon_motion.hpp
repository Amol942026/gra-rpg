#pragma once
#include "hero_carry.hpp"
#include "items.hpp"
#include "weapon_attack_timing.hpp"
#include "generated/melee_attack_data.hpp"

namespace ember::hero_weapon_motion {
inline data::Key sample(Kind kind,float progress){
 static_assert(std::size(data::axe)==AxeAttackFrames+1);
 static_assert(std::size(data::dagger)==DaggerAttackFrames+1);
 const bool axe=kind==Kind::Axe;
 const auto* keys=axe?data::axe:data::dagger;
 const int frames=axe?AxeAttackFrames:DaggerAttackFrames;
 const float frame=std::clamp(progress,0.f,1.f)*frames;
 const int index=std::min(int(frame),frames-1);
 const float weight=frame-index;
 const auto a=keys[index],b=keys[index+1];
 const auto lerp=[weight](float from,float to){return from+(to-from)*weight;};
 return {lerp(a.handR,b.handR),lerp(a.handF,b.handF),lerp(a.handZ,b.handZ),
         lerp(a.yaw,b.yaw),lerp(a.pitch,b.pitch),lerp(a.torsoYaw,b.torsoYaw),lerp(a.lean,b.lean)};
}

inline void orientRightGrip(hero_motion::Pose& pose,hero_motion::Quat orientation){
 using namespace hero_motion;
 namespace bone=hero_motion::data;
 pose.world[bone::Weapon].q=mul(orientation,restRotation(bone::Weapon));
 pose.world[bone::HandR].q=mul(orientation,restRotation(bone::HandR));
 pose.bind();
}

inline void carry(hero_motion::Pose& pose,Kind kind){
 using namespace hero_motion;
 namespace bone=hero_motion::data;
 hero_carry::melee(pose);
 if(kind!=Kind::Axe)return;
 // The cutting edge is on the negative side of the head's width axis.
 const auto shaft=sword_pose::idleBlade(0);
 const Vec axis=pose.vector(bone::Weapon,{shaft.r/37,shaft.f/37,shaft.z/37});
 const Vec across=pose.vector(bone::Spine,{0,-1,0});
 orientRightGrip(pose,hero_carry::bladeOrientation(axis,across));
}

inline void apply(hero_motion::Pose& pose,Kind kind,float progress){
 using namespace hero_motion;
 namespace bone=hero_motion::data;
 const auto key=sample(kind,progress);
 const float weight=ease(progress/.15f)*ease((1-progress)/.18f);
 bend(pose,bone::Spine,mul(axisAngle({0,0,1},key.torsoYaw),axisAngle({1,0,0},-key.lean)));
 bend(pose,bone::Head,axisAngle({0,0,1},-key.torsoYaw*.72f));
 pose.bind();
 carry(pose,kind);
 if(weight<=0)return;
 const Vec wrist=pose.world[bone::HandR].p;
 const Vec target=pose.point(bone::Pelvis,{key.handR,key.handF,key.handZ});
 const float yaw=key.yaw*sword_pose::Pi/180,pitch=key.pitch*sword_pose::Pi/180;
 const Vec heading{std::sin(yaw),std::cos(yaw),0};
 const Vec up{0,0,1};
 const Vec axis=heading*std::cos(pitch)+up*std::sin(pitch);
 // A raised axe leads with its front edge, which turns down through the chop.
 const Vec across=kind==Kind::Axe?up*std::cos(pitch)-heading*std::sin(pitch):Vec{heading.y,-heading.x,0};
 const Vec strikeAxis=pose.vector(bone::Pelvis,axis);
 const Quat carriedOrientation=pose.skin[bone::Weapon].q;
 const auto orientation=hero_carry::bladeOrientation(strikeAxis,pose.vector(bone::Pelvis,across));
 const Vec carriedPole=(pose.world[bone::ForearmR].p-pose.world[bone::UpperArmR].p)*(1.f/13);
 const Vec strikePole=pose.vector(bone::Pelvis,{1,-.42f,-.16f});
 hero_carry::placeGrip(pose,false,wrist*(1-weight)+target*weight,carriedPole*(1-weight)+strikePole*weight,
                       kind==Kind::Axe?mix(carriedOrientation,orientation,weight):carriedOrientation);
 if(kind==Kind::Dagger){
  const auto blade=sword_pose::idleBlade(0);
  const Vec carriedAxis=rotate(carriedOrientation,{blade.r/37,blade.f/37,blade.z/37});
  const Vec mixedAxis=carriedAxis*(1-weight)+strikeAxis*weight;
  const Vec tip=mixedAxis*(1/length(mixedAxis));
  const Vec fore=pose.world[bone::ForearmR].p-pose.world[bone::HandR].p;
  const Vec guard{fore.y*tip.z-fore.z*tip.y,fore.z*tip.x-fore.x*tip.z,fore.x*tip.y-fore.y*tip.x};
  orientRightGrip(pose,hero_carry::bladeOrientation(tip,guard));
 }
}
}
