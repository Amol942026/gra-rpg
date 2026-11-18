#pragma once
// Included inside volcano_actors. The SIF and the renderer share all six cutout pivots.
namespace drake_art {
using ember_drake_motion::Clip;
using ember_drake_motion::Key;
struct Bracket {const Key* a=nullptr; const Key* b=nullptr; float mix=0;};

inline Bracket locate(const Clip& clip,float phase,int direction){
 const float time=std::clamp(phase,0.f,1.f)*clip.times[clip.count-1];
 const int last=clip.count-1;
 const auto* next=std::upper_bound(clip.times+1,clip.times+last,time);
 const int end=int(next-clip.times);
 return {clip.frames[end-1][direction],clip.frames[end][direction],
         (time-clip.times[end-1])/(clip.times[end]-clip.times[end-1])};
}
struct Offsets {float x=0,y=0,angle=0,sx=0,sy=0;};
inline Offsets offsets(const Bracket& interval,int part){
 if(!interval.a)return {};
 const auto& a=interval.a[part];const auto& b=interval.b[part];
 const auto at=[&](float start,float end){return start+(end-start)*interval.mix;};
 return {at(a.x,b.x)/128,at(a.y,b.y)/128,at(a.turn,b.turn)/128,
         at(a.scaleX,b.scaleX)/4096,at(a.scaleY,b.scaleY)/4096};
}
template<size_t N> inline float gapeAt(const unsigned char (&keys)[N],float phase){
 const float frame=std::clamp(phase,0.f,1.f)*(N-1);
 const size_t start=std::min(size_t(frame),N-2);
 return (keys[start]+(keys[start+1]-keys[start])*(frame-start))/255.f;
}
struct Motion {Bracket idle,walk,strike;int direction=0;float walkBlend=0,gape=0,turnLean=0;};
inline Motion sample(const AnimationState& state,float attack,float special,int direction=-1){
 if(direction<0)direction=std::clamp(int(state.direction),0,7);
 const float clock=state.idleTime*.5f;
 const auto strike=special>=0?locate(ember_drake_motion::breath,special,direction):
                   attack>=0?locate(ember_drake_motion::bite,attack,direction):Bracket{};
 const float gape=special>=0?gapeAt(ember_drake_motion::breathGape,special):
                  attack>=0?gapeAt(ember_drake_motion::biteGape,attack):0;
 return {locate(ember_drake_motion::idle,clock-std::floor(clock),direction),
         locate(ember_drake_motion::walk,state.phase,direction),strike,direction,state.moveBlend,gape,state.turnLean};
}
inline Part livingPart(const Motion& motion,int part){
 const auto idle=offsets(motion.idle,part),step=offsets(motion.walk,part),attack=offsets(motion.strike,part);
 const auto blend=[&](float atRest,float moving,float strike){
  return atRest+(moving-atRest)*motion.walkBlend+strike;
 };
 Part result{restPivot(EmberDrakeType-VolcanoFirstType,part,motion.direction),
         {blend(idle.x,step.x,attack.x),blend(idle.y,step.y,attack.y)},
         blend(idle.angle,step.angle,attack.angle),1+blend(idle.sx,step.sx,attack.sx),1+blend(idle.sy,step.sy,attack.sy)};
 applyDrakeTurnGesture(result,part,motion.direction,motion.turnLean);
 return result;
}
inline Part fallPart(const Part& live,const Bracket& clip,int part,float side,float release){
 const auto fold=offsets(clip,part);
 return {live.pivot,{live.shift.x*release+side*fold.x,live.shift.y*release+fold.y},
         live.angle*release+side*fold.angle,
         1+(live.scaleX-1)*release+fold.sx,1+(live.scaleY-1)*release+fold.sy};
}
inline Vector2 mouth(const Enemy& e,Vector2 anchor,float special){
 const auto view=drake_turn::view(e.animation);
 const auto motion=sample(e.animation,e.volcanoAction(),special,view.base);
 const auto other=sample(e.animation,e.volcanoAction(),special,view.nextBase());
 const auto head=turn_art::interpolate(livingPart(motion,5),livingPart(other,5),view.t);
 const auto& tip=ember_drake_motion::mouthTip[view.base];
 const auto& nextTip=ember_drake_motion::mouthTip[view.nextBase()];
 const float gape=motion.gape*drake_turn::mix(float(view.base<drake_mouth::Views),
              float(view.nextBase()<drake_mouth::Views),view.t)*(drake_turn::ready()||drake_mouth::art.id);
 // Ignite just behind the teeth as the lower jaw drops. A point on the
 // upper snout leaves a gap between the bright filament and the throat.
 const float heading=drake_turn::mix(facingAngles[view.base],view.base==7?2*PI:facingAngles[view.nextBase()],view.t);
 const float dx=(drake_turn::mix(tip[0],nextTip[0],view.t)-head.pivot.x-std::cos(heading)*6.f*gape)*head.scaleX;
 const float dy=(drake_turn::mix(tip[1],nextTip[1],view.t)-head.pivot.y+(.8f-1.2f*std::sin(heading))*gape)*head.scaleY;
 const float angle=head.angle*DEG2RAD,c=std::cos(angle),sn=std::sin(angle);
 const float x=head.pivot.x-64+head.shift.x+c*dx-sn*dy;
 const float y=head.pivot.y-115+head.shift.y+sn*dx+c*dy;
 const V lean=e.hitLean();
 const float scale=bodyScale(EmberDrakeType-VolcanoFirstType);
 return {anchor.x+(x-lean.x*y)*scale,anchor.y+y*(1-lean.y)*scale};
}
}
