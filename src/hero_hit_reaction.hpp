#pragma once
#include "hero_motion.hpp"
#include "generated/hero_hit_reaction_data.hpp"

namespace ember::hero_hit_reaction {
namespace rig = hero_motion;
namespace bones = hero_motion::data;

inline data::Key sample(float progress){
 const float frame=std::clamp(progress,0.f,1.f)*data::Frames;
 const int index=std::min(int(frame),data::Frames-1);
 const float fraction=frame-index;
 const auto a=data::samples[index],b=data::samples[index+1];
 return {a.lean+(b.lean-a.lean)*fraction,a.head+(b.head-a.head)*fraction,
         a.drop+(b.drop-a.drop)*fraction};
}

inline float dot(rig::Vec a,rig::Vec b){return a.x*b.x+a.y*b.y+a.z*b.z;}

inline float legDropLimit(const rig::Pose& pose,int hipBone,int kneeBone,int footBone){
 const auto hip=pose.world[hipBone].p,knee=pose.world[kneeBone].p,ankle=pose.world[footBone].p;
 const auto hipOffset=hip-ankle;
 const float reach=rig::length(knee-hip)+rig::length(ankle-knee);
 const float horizontal=hipOffset.x*hipOffset.x+hipOffset.y*hipOffset.y;
 return std::max(0.f,hipOffset.z+std::sqrt(std::max(0.f,reach*reach-horizontal)));
}

inline void flexLeg(rig::Pose& pose,int hipBone,int kneeBone,int footBone,float drop){
 auto& thigh=pose.world[hipBone];
 auto& shin=pose.world[kneeBone];
 const auto ankle=pose.world[footBone].p;
 const auto oldThigh=shin.p-thigh.p,oldShin=ankle-shin.p;
 const auto newHip=thigh.p-rig::Vec{0,0,drop};
 const auto hipToFoot=ankle-newHip;
 const float reach=rig::length(hipToFoot);
 if(reach<.001f)return;
 const auto direction=hipToFoot*(1/reach);
 const float upper=rig::length(oldThigh),lower=rig::length(oldShin);
 const float along=(upper*upper-lower*lower+reach*reach)/(2*reach);
 const float flex=std::sqrt(std::max(0.f,upper*upper-along*along));

 // Keep the existing gait's knee plane as the leg folds around its planted ankle.
 const auto oldReach=oldThigh+oldShin;
 auto pole=oldThigh-oldReach*(dot(oldThigh,oldReach)/std::max(.001f,dot(oldReach,oldReach)));
 pole=pole-direction*dot(pole,direction);
 if(dot(pole,pole)<.001f){
  const auto reference=std::abs(direction.y)<.9f?rig::Vec{0,1,0}:rig::Vec{1,0,0};
  pole=reference-direction*dot(reference,direction);
 }
 const auto knee=newHip+direction*along+pole*(flex/rig::length(pole));
 thigh={newHip,rig::mul(rig::fromTo(oldThigh,knee-newHip),thigh.q)};
 shin={knee,rig::mul(rig::fromTo(oldShin,ankle-knee),shin.q)};
}

inline void apply(rig::Pose& pose,float progress,rig::Vec away){
 if(progress<=0||progress>=1)return;
 const auto key=sample(progress);
 away.z=0;
 const float directionLength=rig::length(away);
 away=directionLength>.0001f?away*(1/directionLength):rig::Vec{0,-1,0};
 const rig::Vec axis{-away.y,away.x,0};
 const float drop=std::min({key.drop,legDropLimit(pose,bones::ThighL,bones::ShinL,bones::FootL),
                            legDropLimit(pose,bones::ThighR,bones::ShinR,bones::FootR)});
 if(drop>0){
  // Move the waist and its upper-body attachments vertically. The root and
  // the feet retain their original transforms, including during locomotion.
  for(int joint=bones::Pelvis;joint<bones::BoneCount;joint++)
   if(joint<bones::ThighL||joint>bones::FootR)pose.world[joint].p.z-=drop;
  flexLeg(pose,bones::ThighL,bones::ShinL,bones::FootL,drop);
  flexLeg(pose,bones::ThighR,bones::ShinR,bones::FootR,drop);
 }
 rig::bend(pose,bones::Spine,rig::axisAngle(axis,key.lean));
 rig::bend(pose,bones::Head,rig::axisAngle(axis,key.head));
 pose.bind();
}
}
