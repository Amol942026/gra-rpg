#pragma once
#include "hero_weapon_motion.hpp"
#include "power_attack.hpp"
#include "generated/power_attack_data.hpp"

namespace ember::power_motion {
inline bool usesOverheadGrip(PowerStyle style){return style==PowerStyle::Sword||style==PowerStyle::Axe;}
template<class Key>
inline auto frameBlend(const Key* track,float progress){
 const float frame=std::clamp(progress,0.f,1.f)*data::Frames;
 const int index=std::min(int(frame),data::Frames-1);
 return [a=track[index],b=track[index+1],t=frame-index](float Key::*member){
  return a.*member+(b.*member-a.*member)*t;
 };
}
inline data::Key sample(PowerStyle style,float progress,bool windup=false){
 static_assert(std::size(data::clips)==int(PowerStyle::Count)&&std::size(data::windups)==int(PowerStyle::Count));
 if(style==PowerStyle::Count)return {};
 using Key=data::Key;
 auto blend=frameBlend((windup?data::windups:data::clips)[int(style)],progress);
 return {blend(&Key::phase),blend(&Key::lean),blend(&Key::turn),blend(&Key::recoil)};
}
inline void body(hero_motion::Pose& pose,const data::Key& key){
 using namespace hero_motion;
 namespace bones=hero_motion::data;
 bend(pose,bones::Spine,mul(axisAngle({0,0,1},key.turn),axisAngle({1,0,0},-key.lean)));
 bend(pose,bones::Head,axisAngle({0,0,1},-key.turn*.65f));
 pose.bind();
}
inline void overheadGrip(hero_motion::Pose& pose,PowerStyle style,float progress,bool windup,float gripAvailable){
 using namespace hero_motion;
 using Key=data::OverheadKey;
 namespace bones=hero_motion::data;
 const bool axe=style==PowerStyle::Axe;
 auto blend=frameBlend((windup?data::overheadWindup:data::overheadRelease)[axe?1:0],progress);
 const float weight=blend(&Key::weight)*gripAvailable;
 if(weight<=0)return;
 const float pitch=blend(&Key::pitch)*sword_pose::Pi/180;
 const float yaw=blend(&Key::yaw)*sword_pose::Pi/180;
 const Vec heading{std::sin(yaw),std::cos(yaw),0};
 const Vec shaft=heading*std::cos(pitch)+Vec{0,0,std::sin(pitch)};
 // The cutting edge is on the negative width axis of the axe head.
 const Vec width=axe?Vec{0,-shaft.z,shaft.y}:Vec{heading.y,-heading.x,0};
 const Vec grip=pose.point(bones::Pelvis,{blend(&Key::handR),blend(&Key::handF),blend(&Key::handZ)});
 const Vec carried=pose.world[bones::HandR].p;
 const Vec shoulder=pose.world[bones::UpperArmR].p;
 const Vec carryPole=(pose.world[bones::ForearmR].p-shoulder)*(1.f/13);
 const Vec raisedPole{1,-.42f,-.21f};
 const Quat orientation=hero_carry::bladeOrientation(shaft,width);
 hero_carry::placeGrip(pose,false,carried*(1-weight)+grip*weight,
                       carryPole*(1-weight)+raisedPole*weight,
                       mix(pose.skin[bones::Weapon].q,orientation,weight));
 if(!axe){
  // Align the crossguard with the bent arm as the wrist leaves and returns to the carry pose.
  const auto blade=sword_pose::idleBlade();
  const Vec tip=pose.vector(bones::Weapon,Vec{blade.r,blade.f,blade.z}*(1/sword_pose::BladeLength));
  const Vec fore=pose.world[bones::ForearmR].p-pose.world[bones::HandR].p;
  const Vec guard{fore.y*tip.z-fore.z*tip.y,fore.z*tip.x-fore.x*tip.z,fore.x*tip.y-fore.y*tip.x};
  hero_weapon_motion::orientRightGrip(pose,hero_carry::bladeOrientation(tip,guard));
 }
}
}
