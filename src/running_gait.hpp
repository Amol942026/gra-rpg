#pragma once
#include <cmath>
#include <algorithm>
namespace ember {
// A relaxed bent arm swings in the shoulder's forward/back plane. Both
// segments retain their length; the opposite arm is half a cycle ahead.
struct RunningArm {float elbowForward,elbowHeight,handForward,handHeight;};
inline RunningArm runningArm(float phase,float side,float shoulderHeight){
 constexpr float pi=3.14159265358979323846f,length=13.f,bend=1.32f;
 float angle=-.68f*std::cos(phase*2*pi+(side<0?pi:0))-.10f;
 float elbowForward=length*std::sin(angle),elbowHeight=shoulderHeight-length*std::cos(angle);
 return {elbowForward,elbowHeight,elbowForward+length*std::sin(angle+bend),elbowHeight-length*std::cos(angle+bend)};
}
struct RunningLeg {float footForward,footHeight,kneeForward,kneeHeight;bool planted;};
inline RunningLeg runningLeg(float phase,float side,float hipHeight){
 constexpr float pi=3.14159265358979323846f,length=17.f,contact=.42f;
 float t=std::fmod(phase+(side<0?.5f:0.f)+2,1.f);
 bool planted=t<contact;float u=planted?t/contact:(t-contact)/(1-contact);
 float f=(planted?1.f:-1.f)*13*std::cos(pi*u);
 float z=4+(planted?0:12*std::pow(std::sin(pi*u),2));
 float dz=z-hipHeight,distance=std::max(.001f,std::hypot(f,dz));
 float bend=std::sqrt(std::max(0.f,length*length-distance*distance*.25f));
 return {f,z,f*.5f-dz/distance*bend,hipHeight+dz*.5f+f/distance*bend,planted};
}
}
