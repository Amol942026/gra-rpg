#pragma once
// Included inside volcano_actors, after the shared Synfig curve sampler.
namespace sentinel_art {
struct Motion {
 drake_art::Bracket rest{}, walk{}, gesture{};
 float walking{};
 int direction{};
 float elbow{};
};
template<std::size_t N>
inline float flex(const unsigned char (&times)[N],const short (&angles)[N][8],int direction,float phase){
 const float time=std::clamp(phase,0.f,1.f)*times[N-1];
 const auto end=int(std::upper_bound(times+1,times+N-1,time)-times);
 const float t=(time-times[end-1])/(times[end]-times[end-1]);
 return (angles[end-1][direction]+(angles[end][direction]-angles[end-1][direction])*t)/128.f;
}
inline Motion sample(const AnimationState& state,float attack,float special,int direction){
 using namespace obsidian_sentinel_motion;
 const float breath=state.idleTime*.5f;
 const auto gesture=special>=0 ? drake_art::locate(cleave,special,direction)
                    : attack>=0 ? drake_art::locate(strike,attack,direction) : drake_art::Bracket{};
 return {drake_art::locate(idle,breath-std::floor(breath),direction),
         drake_art::locate(walk,state.phase,direction),gesture,state.moveBlend,direction,
         special>=0 ? flex(cleaveTimes,cleaveElbow,direction,special) :
         attack>=0 ? flex(strikeTimes,strikeElbow,direction,attack) : 0.f};
}
inline Part livingPart(const Motion& motion,int part){
 const auto a=drake_art::offsets(motion.rest,part);
 const auto b=drake_art::offsets(motion.walk,part);
 const auto c=drake_art::offsets(motion.gesture,part);
 const auto mix=[&](float rest,float walk){return rest+(walk-rest)*motion.walking;};
 const auto& joint=obsidian_sentinel_motion::pivots[motion.direction][part];
 return {{joint[0],joint[1]},{mix(a.x,b.x)+c.x,mix(a.y,b.y)+c.y},
         mix(a.angle,b.angle)+c.angle,1+mix(a.sx,b.sx)+c.sx,1+mix(a.sy,b.sy)+c.sy,
         part==5 ? motion.elbow : 0.f};
}
}
