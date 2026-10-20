#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include "generated/hero_motion_data.hpp"
#include "hero_secondary.hpp"
namespace ember::hero_motion {
using Bone=data::Bone;
struct Vec {
 float x=0,y=0,z=0;
 Vec operator+(Vec b)const{return{x+b.x,y+b.y,z+b.z};}
 Vec operator-(Vec b)const{return{x-b.x,y-b.y,z-b.z};}
 Vec operator*(float s)const{return{x*s,y*s,z*s};}
};
inline float length(Vec v){return std::sqrt(v.x*v.x+v.y*v.y+v.z*v.z);}
struct Quat{float w=1,x=0,y=0,z=0;};
inline Quat conjugate(Quat q){return {q.w,-q.x,-q.y,-q.z};}
inline Quat mul(Quat a,Quat b){return {a.w*b.w-a.x*b.x-a.y*b.y-a.z*b.z,a.w*b.x+a.x*b.w+a.y*b.z-a.z*b.y,a.w*b.y-a.x*b.z+a.y*b.w+a.z*b.x,a.w*b.z+a.x*b.y-a.y*b.x+a.z*b.w};}
inline Vec rotate(Quat q,Vec p){auto r=mul(mul(q,{0,p.x,p.y,p.z}),conjugate(q));return {r.x,r.y,r.z};}
inline Quat mix(Quat a,Quat b,float t){
 float sign=a.w*b.w+a.x*b.x+a.y*b.y+a.z*b.z<0?-1.f:1.f;
 Quat q{a.w*(1-t)+b.w*t*sign,a.x*(1-t)+b.x*t*sign,a.y*(1-t)+b.y*t*sign,a.z*(1-t)+b.z*t*sign};
 float inverse=1/std::sqrt(std::max(.000001f,q.w*q.w+q.x*q.x+q.y*q.y+q.z*q.z));
 return {q.w*inverse,q.x*inverse,q.y*inverse,q.z*inverse};
}
inline Quat fromTo(Vec a,Vec b){
 float al=length(a),bl=length(b);a=a*(1/std::max(.00001f,al));b=b*(1/std::max(.00001f,bl));
 float d=a.x*b.x+a.y*b.y+a.z*b.z;
 if(d<-.9999f)return {0,1,0,0};
 Quat q{1+d,a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
 return mix(q,q,0);
}
struct Transform {Vec p;Quat q;};
using LocalPose=std::array<Transform,data::BoneCount>;
inline float ease(float t){t=std::clamp(t,0.f,1.f);return t*t*(3-2*t);}
inline Vec restHead(int b){auto p=data::restHeads[b];return {p[0],p[1],p[2]};}
inline Quat restRotation(int b){auto p=data::restRotations[b];return {p[0],p[1],p[2],p[3]};}
inline LocalPose sample(const data::Clip& clip,float phase){
 phase=clip.loop?phase-std::floor(phase):std::clamp(phase,0.f,1.f);
 float position=phase*(clip.frames-1);int a=std::min(int(position),clip.frames-2);float t=position-a;
 LocalPose out;
 for(int b=0;b<data::BoneCount;b++){
  const auto& x=clip.keys[a*data::BoneCount+b];const auto& y=clip.keys[(a+1)*data::BoneCount+b];
  out[b]={{x.p[0]*(1-t)+y.p[0]*t,x.p[1]*(1-t)+y.p[1]*t,x.p[2]*(1-t)+y.p[2]*t},
          mix({x.q[0],x.q[1],x.q[2],x.q[3]},{y.q[0],y.q[1],y.q[2],y.q[3]},t)};
 }
 return out;
}
inline LocalPose blend(const LocalPose& a,const LocalPose& b,float t){
 LocalPose out;for(int i=0;i<data::BoneCount;i++)out[i]={a[i].p*(1-t)+b[i].p*t,mix(a[i].q,b[i].q,t)};return out;
}
struct Pose {
 std::array<Transform,data::BoneCount> world,skin;
 std::array<std::array<float,12>,data::BoneCount> matrices;
 Vec point(int b,Vec p)const{auto& m=matrices[b];return {m[0]*p.x+m[1]*p.y+m[2]*p.z+m[3],m[4]*p.x+m[5]*p.y+m[6]*p.z+m[7],m[8]*p.x+m[9]*p.y+m[10]*p.z+m[11]};}
 Vec vector(int b,Vec p)const{auto& m=matrices[b];return {m[0]*p.x+m[1]*p.y+m[2]*p.z,m[4]*p.x+m[5]*p.y+m[6]*p.z,m[8]*p.x+m[9]*p.y+m[10]*p.z};}
 void bind(){
  for(int i=0;i<data::BoneCount;i++){
   skin[i]={world[i].p,mul(world[i].q,conjugate(restRotation(i)))};
   auto q=skin[i].q;auto r=restHead(i);auto& m=matrices[i];
   m={1-2*(q.y*q.y+q.z*q.z),2*(q.x*q.y-q.z*q.w),2*(q.x*q.z+q.y*q.w),0,
      2*(q.x*q.y+q.z*q.w),1-2*(q.x*q.x+q.z*q.z),2*(q.y*q.z-q.x*q.w),0,
      2*(q.x*q.z-q.y*q.w),2*(q.y*q.z+q.x*q.w),1-2*(q.x*q.x+q.y*q.y),0};
   m[3]=world[i].p.x-m[0]*r.x-m[1]*r.y-m[2]*r.z;
   m[7]=world[i].p.y-m[4]*r.x-m[5]*r.y-m[6]*r.z;
   m[11]=world[i].p.z-m[8]*r.x-m[9]*r.y-m[10]*r.z;
  }
 }
};
inline Pose forward(const LocalPose& local){
 Pose out;
 for(int b=0;b<data::BoneCount;b++){
  int parent=data::parents[b];
  out.world[b]=parent<0?local[b]:Transform{out.world[parent].p+rotate(out.world[parent].q,local[b].p),mul(out.world[parent].q,local[b].q)};
 }
 out.bind();return out;
}
inline Quat axisAngle(Vec axis,float degrees){float half=degrees*.00872664626f,s=std::sin(half);return {std::cos(half),axis.x*s,axis.y*s,axis.z*s};}
inline void bend(Pose& pose,int pivot,Quat delta){
 Vec origin=pose.world[pivot].p;
 for(int b=pivot;b<data::BoneCount;b++){
  int ancestor=b;while(ancestor>=0&&ancestor!=pivot)ancestor=data::parents[ancestor];
  if(ancestor<0)continue;
  auto& world=pose.world[b];world.p=origin+rotate(delta,world.p-origin);world.q=mul(delta,world.q);
 }
}
inline void secondary(Pose& pose,hero_secondary::Key key){
 bend(pose,data::Spine,mul(axisAngle({0,1,0},key.spineRoll),axisAngle({1,0,0},-key.spinePitch)));
 bend(pose,data::Head,axisAngle({0,1,0},key.headRoll));
 bend(pose,data::UpperArmL,axisAngle({1,0,0},key.armSwing));
 bend(pose,data::UpperArmR,axisAngle({1,0,0},-key.armSwing));
 pose.bind();
}
inline Pose evaluate(float phase,float motion,float run,float idleTime,float attack=-1,bool heavy=false,float roll=-1){
 auto idle=sample(data::IdleClip,idleTime/2);
 auto locomotion=blend(sample(data::WalkClip,phase),sample(data::RunClip,phase),std::clamp(run,0.f,1.f));
 auto local=blend(idle,locomotion,std::clamp(motion,0.f,1.f));
 if(roll>=0){
  float weight=ease(roll/.10f)*ease((1-roll)/.13f);
  local=blend(local,sample(data::RollClip,roll),weight);
 }else if(attack>=0){
  float weight=ease(attack/.12f)*ease((1-attack)/.20f);
  local=blend(local,sample(heavy?data::HeavyClip:data::SlashClip,attack),weight);
 }
 auto pose=forward(local);secondary(pose,hero_secondary::evaluate(phase,motion,run,idleTime,attack,heavy,roll));return pose;
}
inline Pose action(float progress,bool heavy=false){return forward(sample(heavy?data::HeavyClip:data::SlashClip,progress));}
inline void attach(Pose& pose,int bone,Vec start,Vec end,Vec restEnd){
 Quat skin=fromTo(restEnd-restHead(bone),end-start);
 pose.world[bone]={start,mul(skin,restRotation(bone))};pose.skin[bone]={start,skin};
}
}
