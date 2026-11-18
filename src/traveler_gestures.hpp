#pragma once
#include "hero_motion.hpp"
#include "power_attack.hpp"
#include "generated/traveler_gestures_data.hpp"

namespace ember::traveler_gesture {
inline Key blend(Key a,Key b,float t){
 return {a.hemSway+(b.hemSway-a.hemSway)*t,a.hemLift+(b.hemLift-a.hemLift)*t,
         a.neckPitch+(b.neckPitch-a.neckPitch)*t,a.headRoll+(b.headRoll-a.headRoll)*t};
}
inline Key sample(Gesture which,float phase){
 const auto& track=keys[int(which)];
 const float f=std::clamp(phase,0.f,1.f)*Frames;
 const int i=std::min(int(f),Frames-1);
 return blend(track[i],track[i+1],f-i);
}
struct Take {Gesture gesture=Gesture::Count;float phase=0;};
inline Take action(Kind weapon,float strike,PowerStyle power,float age,float winding,float bowDraw,float bowRelease,
                   float stow,bool stowing,float flinch,float fall){
 if(fall>=0)return {Gesture::Fall,fall};
 if(flinch>=0)return {Gesture::Flinch,flinch};
 if(weapon==Kind::Bow){
  if(bowRelease>=0)return {power==PowerStyle::Bow?Gesture::BowPower:Gesture::BowLoose,bowRelease};
  if(bowDraw>=0)return {Gesture::BowDraw,bowDraw};
 }
 constexpr Gesture windups[]={Gesture::SwordCharge,Gesture::AxeCharge,Gesture::DaggerCharge,Gesture::SpearCharge,Gesture::BowDraw};
 constexpr Gesture releases[]={Gesture::SwordPower,Gesture::AxePower,Gesture::DaggerPower,Gesture::SpearPower,Gesture::BowPower};
 static_assert(std::size(windups)==int(PowerStyle::Count)&&std::size(releases)==int(PowerStyle::Count));
 if(power!=PowerStyle::Count)return {winding>=0?windups[int(power)]:releases[int(power)],winding>=0?winding:age};
 if(strike>=0){
  switch(weapon){
  case Kind::Axe:return {Gesture::Axe,strike};
  case Kind::Dagger:return {Gesture::Dagger,strike};
  case Kind::Spear:return {Gesture::Spear,strike};
  default:return {};
  }
 }
 if(stow>0&&stow<1){
  const bool back=weapon==Kind::Axe||weapon==Kind::Spear;
  if(stowing)return {back?Gesture::BackStow:Gesture::BeltStow,stow};
  return {back?Gesture::BackDraw:Gesture::BeltDraw,1-stow};
 }
 return {};
}

inline void apply(hero_motion::Pose& pose,hero_secondary::Key& coat,Take take,float draw){
 if(take.gesture==Gesture::Count)return;
 auto key=sample(take.gesture,take.phase);
 if(take.gesture==Gesture::BowLoose||take.gesture==Gesture::BowPower){
  // When the player lets go early, start at the actual tension of the string.
  const float tension=std::clamp(draw/std::max(.0001f,1-take.phase),0.f,1.f);
  const auto held=sample(Gesture::BowDraw,tension);
  const auto full=sample(take.gesture,0);
  key=blend({},key,tension);
  const float settle=1-hero_motion::ease(take.phase/.32f);
  key.hemSway+=(held.hemSway-full.hemSway*tension)*settle;
  key.hemLift+=(held.hemLift-full.hemLift*tension)*settle;
  key.neckPitch+=(held.neckPitch-full.neckPitch*tension)*settle;
  key.headRoll+=(held.headRoll-full.headRoll*tension)*settle;
 }
 coat.capeSway+=key.hemSway;
 coat.capeLift+=key.hemLift;
 using namespace hero_motion;
 const auto head=data::Head;
 const auto tilt=mul(axisAngle(pose.vector(head,{0,1,0}),key.headRoll),
                     axisAngle(pose.vector(head,{1,0,0}),key.neckPitch));
 pose.world[head].q=mul(tilt,pose.world[head].q);
 pose.bindBone(head);
}
}
