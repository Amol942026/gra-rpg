#pragma once
// Included in volcano_actors after the shared Synfig curve sampler.
namespace crawler_art {
struct Motion {
 drake_art::Bracket rest,walk,bank,bite,pivot;
 float walking,pivotWeight;
 int direction;
};
inline Motion sample(const AnimationState& state,float attack,int direction){
 using namespace slag_crawler_motion;
 const float breathing=state.idleTime*.5f;
 const float turning=std::clamp(.5f+.5f*state.turnLean,0.f,1.f);
 const float pivotWeight=std::abs(state.turnLean)*(1-state.moveBlend);
 return {drake_art::locate(idle,breathing-std::floor(breathing),direction),
         drake_art::locate(walk,state.phase,direction),
         std::abs(state.turnLean)>.001f?drake_art::locate(bank,turning,direction):drake_art::Bracket{},
         attack>=0?drake_art::locate(bite,attack,direction):drake_art::Bracket{},
         pivotWeight>.001f?drake_art::locate(state.turnLean>0?pivot_cw:pivot_ccw,state.pivotPhase,direction):drake_art::Bracket{},
         state.moveBlend,pivotWeight,direction};
}
inline Part livingPart(const Motion& motion,int part){
 const auto a=drake_art::offsets(motion.rest,part),b=drake_art::offsets(motion.walk,part);
 const auto attack=drake_art::offsets(motion.bite,part),turn=drake_art::offsets(motion.bank,part);
 const auto pivot=drake_art::offsets(motion.pivot,part);
 const auto blend=[&](float rest,float walking){return rest+(walking-rest)*motion.walking;};
 const auto& root=slag_crawler_motion::pivots[motion.direction][part];
 return {{root[0],root[1]},
         {blend(a.x,b.x)+turn.x+attack.x+pivot.x*motion.pivotWeight,
          blend(a.y,b.y)+turn.y+attack.y+pivot.y*motion.pivotWeight},
         blend(a.angle,b.angle)+turn.angle+attack.angle+pivot.angle*motion.pivotWeight,
         1+blend(a.sx,b.sx)+turn.sx+attack.sx,1+blend(a.sy,b.sy)+turn.sy+attack.sy+pivot.sy*motion.pivotWeight};
}
inline void settlePart(Part& part,int row,int first,int next,float blend,float phase,float impactSide){
 const auto& clip=slag_crawler_motion::fall;
 const auto a=drake_art::offsets(drake_art::locate(clip,phase,first),row);
 const auto b=drake_art::offsets(drake_art::locate(clip,phase,next),row);
 const auto mix=[blend](float l,float r){return l+(r-l)*blend;};
 part.shift.x+=mix(a.x,b.x);
 part.shift.y+=mix(a.y,b.y);
 part.angle+=mix(a.angle,b.angle)*impactSide;
 part.scaleX*=1+mix(a.sx,b.sx);
 part.scaleY*=1+mix(a.sy,b.sy);
}
}
