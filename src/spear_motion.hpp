#pragma once
#include "hero_carry.hpp"
#include "spear_timing.hpp"
#include "generated/spear_motion_data.hpp"

namespace ember::spear_motion {
inline data::Key sample(float progress){
 static_assert(std::size(data::keys)==SpearFrames+1);
 float frame=std::clamp(progress,0.f,1.f)*SpearFrames;
 int i=std::min(int(frame),SpearFrames-1);float t=frame-i;
 const auto& a=data::keys[i];const auto& b=data::keys[i+1];
 auto mix=[t](float x,float y){return x+(y-x)*t;};
 return {mix(a.rearR,b.rearR),mix(a.rearF,b.rearF),mix(a.rearZ,b.rearZ),mix(a.yaw,b.yaw),
         mix(a.pitch,b.pitch),mix(a.lane,b.lane),mix(a.lean,b.lean)};
}
inline void apply(hero_motion::Pose& pose,float progress,float gait,float stride){
 using namespace hero_motion;
 namespace bone=hero_motion::data;
 const auto key=sample(progress);
 const float carry=progress<0?gait*stride:0;
 bend(pose,bone::Spine,mul(axisAngle({0,0,1},key.yaw),axisAngle({1,0,0},-key.lean)));
 bend(pose,bone::Head,axisAngle({0,0,1},-key.yaw*.82f));
 pose.bind();
 const Vec rear{key.rearR,key.rearF+carry*.7f,key.rearZ+std::abs(carry)*.7f};
 const float pitch=key.pitch*.01745329252f,tilt=std::sqrt(1-key.lane*key.lane);
 const Vec direction{key.lane,tilt*std::cos(pitch),tilt*std::sin(pitch)};
 const Vec front=rear+direction*13.4f;
 const auto grip=mul(pose.skin[bone::Pelvis].q,axisAngle({1,0,0},key.pitch-70));
 hero_carry::placeGrip(pose,false,pose.point(bone::Pelvis,rear),pose.vector(bone::Pelvis,{1,-.52f,-.2f}),grip);
 hero_carry::placeGrip(pose,true,pose.point(bone::Pelvis,front),pose.vector(bone::Pelvis,{-1,-.50f,-.33f}),grip);
}
}
