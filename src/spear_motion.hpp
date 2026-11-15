#pragma once
#include "animation.hpp"
#include "hero_carry.hpp"
#include "items.hpp"
#include "power_motion.hpp"
#include "spear_timing.hpp"
#include "weapon_profiles.hpp"
#include "generated/spear_motion_data.hpp"

namespace ember::spear_motion {
inline data::Key sample(Kind kind,float progress){
 static_assert(std::size(data::keys)==SpearCount&&std::size(data::keys[0])==SpearFrames+1);
 const auto& keys=data::keys[int(kind)-FirstSpear];
 float frame=std::clamp(progress,0.f,1.f)*SpearFrames;
 int i=std::min(int(frame),SpearFrames-1);float t=frame-i;
 const auto& a=keys[i];const auto& b=keys[i+1];
 auto mix=[t](float x,float y){return x+(y-x)*t;};
 return {mix(a.rearR,b.rearR),mix(a.rearF,b.rearF),mix(a.rearZ,b.rearZ),mix(a.yaw,b.yaw),
         mix(a.pitch,b.pitch),mix(a.lane,b.lane),mix(a.lean,b.lean),
         mix(a.spacing,b.spacing),mix(a.slide,b.slide)};
}
inline float apply(hero_motion::Pose& pose,Kind kind,float progress,float gait,float stride){
 using namespace hero_motion;
 namespace bone=hero_motion::data;
 const auto key=sample(kind,progress);
 const float carry=gait*stride*(1-ease(std::min(progress,1-progress)/.14f));
 // Tip the upper body from the hips so the lunge keeps both feet anchored.
 bend(pose,bone::Spine,mul(axisAngle({0,0,1},key.yaw),axisAngle({1,0,0},-key.lean)),
      pose.world[bone::Pelvis].p);
 bend(pose,bone::Head,axisAngle({0,0,1},-key.yaw*.82f));
 pose.bind();
 const Vec rear{key.rearR,key.rearF+carry*.7f,key.rearZ+std::abs(carry)*.7f};
 const float pitch=key.pitch*.01745329252f,tilt=std::sqrt(1-key.lane*key.lane);
 const Vec direction{key.lane,tilt*std::cos(pitch),tilt*std::sin(pitch)};
 const Vec front=rear+direction*key.spacing;
 const auto grip=mul(pose.skin[bone::Pelvis].q,axisAngle({1,0,0},key.pitch-70));
 hero_carry::placeGrip(pose,false,pose.point(bone::Pelvis,rear),pose.vector(bone::Pelvis,{1,-.52f,-.2f}),grip);
 hero_carry::placeGrip(pose,true,pose.point(bone::Pelvis,front),pose.vector(bone::Pelvis,{-1,-.50f,-.33f}),grip);
 return key.slide;
}

inline hero_motion::Vec bladeTip(const hero_motion::Pose& pose,Kind kind,float slide){
 using namespace hero_motion;
 const Vec rear=pose.world[hero_motion::data::HandR].p;
 const Vec shaft=pose.world[hero_motion::data::HandL].p-rear;
 const Vec axis=shaft*(1/std::max(.001f,length(shaft)));
 const Vec across=Vec{axis.y,-axis.x,0}*(1/std::max(.001f,std::hypot(axis.x,axis.y)));
 const auto profile=weapon_profiles::spear(kind);
 return rear+axis*(slide+profile.tipOffset())+across*(profile.halfWidth*profile.point[3].x);
}

inline hero_motion::Vec impactTip(const AnimationState& state,Kind kind,bool charged){
 const float gait=state.motionInitialized?state.moveBlend:(state.moving?1.f:0.f);
 auto pose=hero_motion::evaluate(state.phase,gait,0,state.idleTime);
 float progress=float(SpearImpactFrame)/SpearFrames;
 if(charged){
  const auto& attack=powerAttack(PowerStyle::Spear);
  const auto powerKey=power_motion::sample(PowerStyle::Spear,attack.contact/attack.duration);
  power_motion::body(pose,powerKey);
  progress=powerKey.phase;
 }
 const float stride=state.moving?std::sin(state.phase*(2*AnimationPi)):0;
 const float slide=apply(pose,kind,progress,gait,stride);
 return bladeTip(pose,kind,slide);
}
}
