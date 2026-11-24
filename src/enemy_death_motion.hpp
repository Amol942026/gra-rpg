#pragma once
#include "entity_info.hpp"
#include "generated/varkhor_ash_motion.hpp"
#include <array>

namespace ember::enemy_death {
struct Key {float collapse,recoil,settle;};
struct Mark {int frame;float collapse,recoil,settle;};
using Track=std::array<Mark,6>;
}
#include "generated/enemy_death_data.hpp"

namespace ember::enemy_death {
static_assert(std::size(data::tracks)==EnemyTypeCount);
inline constexpr float durationScale(int type){return type==DuneJackalType?.70f:1.f;}
inline float duration(int type){return data::tracks[type].back().frame*durationScale(type)/data::framesPerSecond;}
inline float endTime(int type){
 if(type==StoneGuardianType)return data::guardianSmoke.back().frame/data::framesPerSecond;
 return type==VolcanoBossType?float(varkhor_ash_motion::AirFrames)/varkhor_ash_motion::FramesPerSecond:duration(type);
}
inline float ease(float x){x=std::clamp(x,0.f,1.f);return x*x*(3-2*x);}
inline float window(float progress,float from,float to){return ease((progress-from)/(to-from));}
inline Key sampleTrack(const Track& track,float frame){
 frame=std::clamp(frame,0.f,float(track.back().frame));
 std::size_t i=1;
 while(i+1<track.size()&&frame>track[i].frame)++i;
 const auto a=track[i-1],b=track[i];
 const float t=ease((frame-a.frame)/(b.frame-a.frame));
 return {a.collapse+(b.collapse-a.collapse)*t,a.recoil+(b.recoil-a.recoil)*t,a.settle+(b.settle-a.settle)*t};
}
inline Key sample(int type,float progress){
 const auto& track=data::tracks[type];
 return sampleTrack(track,progress*track.back().frame);
}
inline float groundContact(int type){const auto& track=data::tracks[type];return float(track[3].frame)/track.back().frame;}
inline float groundContactTime(int type){return data::tracks[type][3].frame*durationScale(type)/data::framesPerSecond;}
}
