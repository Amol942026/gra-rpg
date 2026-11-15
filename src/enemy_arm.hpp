#pragma once
#include <algorithm>
#include <cmath>

namespace enemy_arm {
struct Point {
 float side,forward,up;
 Point operator+(Point b)const{return {side+b.side,forward+b.forward,up+b.up};}
 Point operator-(Point b)const{return {side-b.side,forward-b.forward,up-b.up};}
 Point operator*(float s)const{return {side*s,forward*s,up*s};}
};
inline float length(Point p){return std::sqrt(p.side*p.side+p.forward*p.forward+p.up*p.up);}
struct Arm {Point elbow,hand;};
inline constexpr float GuardianUpper=17.f,GuardianForearm=16.f;
inline Arm solve(Point shoulder,Point target,float upper,float forearm){
 const Point delta=target-shoulder;
 const float requested=length(delta);
 const Point axis=requested>.001f?delta*(1/requested):Point{0,0,-1};
 const float reach=std::clamp(requested,std::abs(upper-forearm)+.10f,upper+forearm-.24f);
 const float along=(upper*upper-forearm*forearm+reach*reach)/(2*reach);
 const float bend=std::sqrt(std::max(0.f,upper*upper-along*along));
 Point pole{0,axis.up,-axis.forward};
 if(length(pole)<.001f)pole={0,-1,0};
 return {shoulder+axis*along+pole*(bend/length(pole)),shoulder+axis*reach};
}
}
