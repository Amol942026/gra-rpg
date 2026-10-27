#pragma once
#include "hero_motion.hpp"
#include "generated/hero_knockdown_data.hpp"
namespace ember::hero_knockdown {
// Native Synfig contact poses: airborne backfall, supine rest, side brace and
// slow get-up. Facing is resolved before take-off, never by twisting the body
// into an arbitrary sideways/forward fall while it is lying on the floor.
inline hero_motion::Pose evaluate(const hero_motion::Pose& base,float progress,hero_motion::Vec away){
 using namespace hero_motion;
 (void)away;if(progress<=0||progress>=1)return base;
 auto out=forward(sample(knockdown_synfig::clip,progress));
 float weight=ease(progress*knockdown_synfig::Duration/.12f)*ease((1-progress)*knockdown_synfig::Duration/.20f);
 for(int b=0;b<data::BoneCount;b++){out.world[b].p=base.world[b].p*(1-weight)+out.world[b].p*weight;out.world[b].q=mix(base.world[b].q,out.world[b].q,weight);}
 out.bind();return out;
}
}
