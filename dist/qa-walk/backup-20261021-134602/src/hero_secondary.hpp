#pragma once
#include "generated/hero_secondary_data.hpp"
namespace ember::hero_secondary {
inline Key blend(Key a,Key b,float t){
 auto m=[&](float x,float y){return x+(y-x)*t;};
 return {m(a.spineRoll,b.spineRoll),m(a.spinePitch,b.spinePitch),m(a.headRoll,b.headRoll),m(a.armSwing,b.armSwing),m(a.capeSway,b.capeSway),m(a.capeLift,b.capeLift)};
}
inline Key sample(const Clip& clip,float phase){
 phase=clip.loop?phase-std::floor(phase):std::clamp(phase,0.f,1.f);
 float p=phase*(clip.frames-1);int a=std::min(int(p),clip.frames-2);
 return blend(clip.keys[a],clip.keys[a+1],p-a);
}
inline Key evaluate(float phase,float motion,float running,float idleTime,float attack=-1,bool strong=false,float rolling=-1){
 auto base=blend(sample(idle,idleTime/idle.duration),blend(sample(walk,phase),sample(run,phase),std::clamp(running,0.f,1.f)),std::clamp(motion,0.f,1.f));
 if(rolling>=0)return sample(roll,rolling);
 if(attack>=0){float w=std::clamp(std::min(attack/.12f,(1-attack)/.20f),0.f,1.f);w=w*w*(3-2*w);return blend(base,sample(strong?heavy:slash,attack),w);}
 return base;
}
}
