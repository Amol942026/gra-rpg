#pragma once
#include <algorithm>
#include <cmath>
namespace ember::sword_pose {
constexpr float Pi=3.14159265358979323846f,UpperArm=13,Forearm=13,BladeLength=37;
struct Point {
 float r=0,f=0,z=0;
 Point operator+(Point b)const{return {r+b.r,f+b.f,z+b.z};}
 Point operator-(Point b)const{return {r-b.r,f-b.f,z-b.z};}
 Point operator*(float n)const{return {r*n,f*n,z*n};}
};
inline float dot(Point a,Point b){return a.r*b.r+a.f*b.f+a.z*b.z;}
inline float length(Point p){return std::sqrt(dot(p,p));}
inline Point normalized(Point p){return p*(1/std::max(.00001f,length(p)));}
inline Point mix(Point a,Point b,float t){return a*(1-t)+b*t;}
inline float ease(float t){t=std::clamp(t,0.f,1.f);return t*t*(3-2*t);}
inline Point turn(Point p,float angle){float c=std::cos(angle),s=std::sin(angle);return {p.r*c+p.f*s,p.f*c-p.r*s,p.z};}
struct Arm {Point hand,elbow;};
inline Arm solveArm(Point shoulder,Point target,float side=1){
 Point delta=target-shoulder;float distance=length(delta);
 float reach=std::clamp(distance,.2f,UpperArm+Forearm-.15f);
 Point direction=distance<.00001f?Point{0,0,-1}:normalized(delta);Point hand=shoulder+direction*reach;
 Point pole{side,-.12f,.12f};Point perpendicular=pole-direction*dot(pole,direction);
 if(length(perpendicular)<.001f){pole=std::abs(direction.f)<.9f?Point{0,1,0}:Point{0,0,1};perpendicular=pole-direction*dot(pole,direction);}
 Point bend=normalized(perpendicular);
 float along=(UpperArm*UpperArm-Forearm*Forearm+reach*reach)/(2*reach);
 float height=std::sqrt(std::max(0.f,UpperArm*UpperArm-along*along));
 return {hand,shoulder+direction*along+bend*height};
}
inline Point blade(float yaw,float pitch){
 float flat=std::cos(pitch);return Point{std::sin(yaw)*flat,std::cos(yaw)*flat,std::sin(pitch)}*BladeLength;
}
inline Point idleBlade(float running=0){
 // Carrying changes the orientation of a rigid blade, never its length.
 float p=(75.5297f+168.f*std::clamp(running,0.f,1.f))*Pi/180;
 return blade(0,p);
}
struct Pose {Point hand,elbow,blade;float torsoYaw=0;};
inline float cutStart(bool heavy){return heavy?.32f:.22f;}
inline float cutEnd(bool heavy){return heavy?.70f:.62f;}
inline Pose sample(float progress,float lift=0,bool heavy=false){
 float p=std::clamp(progress,0.f,1.f),start=cutStart(heavy),end=cutEnd(heavy);
 Point idle{20,3,33+lift},ready{heavy?24.f:23.f,heavy?-7.f:-5.f,(heavy?51.f:47.f)+lift};
 Point middle{heavy?8.f:10.f,heavy?24.f:23.f,44+lift},finish{3,15,(heavy?36.f:38.f)+lift};
 float coil=(heavy?30.f:20.f)*Pi/180,follow=(heavy?-32.f:-23.f)*Pi/180;
 float initialPitch=75.5297f*Pi/180,readyPitch=(heavy?56.f:46.f)*Pi/180,finishPitch=-17*Pi/180;
 float initialYaw=0,readyYaw=(heavy?98.f:80.f)*Pi/180,finishYaw=(heavy?-92.f:-75.f)*Pi/180;
 Point hand;float yaw,pitch,torso;
 if(p<start){
  float t=ease(p/start);hand=mix(idle,ready,t);yaw=initialYaw+(readyYaw-initialYaw)*t;
  pitch=initialPitch+(readyPitch-initialPitch)*t;torso=coil*t;
 }else if(p<end){
  float t=ease((p-start)/(end-start)),one=1-t;
  // Quadratic curve passes through a forward reach without a corner at contact.
  Point control=middle*2-(ready+finish)*.5f;
  hand=ready*(one*one)+control*(2*one*t)+finish*(t*t);
  yaw=readyYaw+(finishYaw-readyYaw)*t;pitch=readyPitch+(finishPitch-readyPitch)*t;
  torso=coil+(follow-coil)*t;
 }else{
  float t=ease((p-end)/(1-end));hand=mix(finish,idle,t);
  yaw=finishYaw*(1-t);pitch=finishPitch+(initialPitch-finishPitch)*t;torso=follow*(1-t);
 }
 Arm arm=solveArm({15,0,53+lift},hand);
 return {arm.hand,arm.elbow,blade(yaw,pitch),torso};
}
inline float trailStrength(float progress,bool heavy){
 float start=cutStart(heavy),end=cutEnd(heavy);
 if(progress<=start||progress>=end+.07f)return 0;
 return ease((progress-start)/.06f)*(1-ease((progress-end)/.07f));
}
}
