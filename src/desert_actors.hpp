#pragma once
// Included in actor_art after Rig; every part turns with the same eight-direction rig.
struct SandRing{float r,f,wr,wf,z;};
inline constexpr std::array<SandRing,8> SandRings={{{0,-6,69,56,1},{0,-6,68,55,18},{-4,-7,58,49,40},{-10,-9,53,43,64},{-19,-13,43,34,88},{-29,-19,30,24,111},{-38,-24,16,13,133},{-42,-26,0,0,143}}};
inline SandRing sandRing(float t){
 int k=std::min(6,int(t));t-=k;auto a=SandRings[std::max(0,k-1)],b=SandRings[k],c=SandRings[k+1],d=SandRings[std::min(7,k+2)];
 auto value=[&](float a,float b,float c,float d){return .5f*(2*b+(-a+c)*t+(2*a-5*b+4*c-d)*t*t+(-a+3*b-3*c+d)*t*t*t);};
 return {value(a.r,b.r,c.r,d.r),value(a.f,b.f,c.f,d.f),value(a.wr,b.wr,c.wr,d.wr),value(a.wf,b.wf,c.wf,d.wf),value(a.z,b.z,c.z,d.z)};
}
inline P3 sandFront(float r,float z,float sway,float protrusion=1){
 float low=0,high=7,restZ=z/(1+sway/143.f);for(int i=0;i<12;i++){float mid=(low+high)*.5f;if(sandRing(mid).z<restZ)low=mid;else high=mid;}
 auto ring=sandRing((low+high)*.5f);float ratio=(r-ring.r)/std::max(1.f,ring.wr);return {r,ring.f+ring.wf*std::sqrt(std::max(0.f,1-ratio*ratio))+protrusion,z};
}
// One continuous wind-eroded surface, avoiding stacked overlapping spheres.
inline P3 sandCollapse(P3 p,float fall,float side){
 const float z=p.z/143.f,base=(1-z)*(1-z);
 return {p.r*(1+.27f*fall*base)+side*fall*(5+25*z),
         p.f*(1+.17f*fall*base)+fall*(4+15*z),.58f+(p.z-.58f)*(1-.73f*fall)};
}
inline void sandMound(Rig& rig,float sway,Color col,float fall=0,float side=1){
 constexpr int sides=24,bands=32;
 struct Vertex{P3 point,normal;};
 static const auto mesh=[](){
  constexpr int sides=24,bands=32;
  std::array<std::array<Vertex,sides>,bands+1> result{};
  for(int j=0;j<=bands;j++){auto p=sandRing(j*7.f/bands);
   for(int i=0;i<sides;i++){float angle=i*2*PI/sides;result[j][i].point={p.r+std::cos(angle)*p.wr,p.f+std::sin(angle)*p.wf,p.z};}
  }
  for(int j=0;j<=bands;j++)for(int i=0;i<sides;i++){P3 around=result[j][(i+1)%sides].point-result[j][(i+sides-1)%sides].point,up=result[std::min(bands,j+1)][i].point-result[std::max(0,j-1)][i].point;
   P3 n{around.f*up.z-around.z*up.f,around.z*up.r-around.r*up.z,around.r*up.f-around.f*up.r};float len=std::sqrt(n.r*n.r+n.f*n.f+n.z*n.z);result[j][i].normal=len>.001f?n*(1/len):P3{0,0,1};
  }return result;
 }();
 std::array<std::array<Color,sides>,bands+1> colors{};
 for(int j=0;j<=bands;j++)for(int i=0;i<sides;i++)colors[j][i]=shade(col,rig.surfaceLight(mesh[j][i].normal)*(1+.018f*std::sin(mesh[j][i].point.z*.32f+i*.25f)));
 auto point=[&](int j,int i){
  P3 p=mesh[j][i].point;p.z+=sway*p.z/143.f;
  // The crest runs downslope while the same material bands spread at the foot.
  return sandCollapse(p,fall,side);
 };
 for(int j=0;j<bands;j++)for(int i=0;i<sides;i++){int n=(i+1)%sides;rig.surfaceQuad(point(j,i),point(j,n),point(j+1,n),point(j+1,i),colors[j][i],colors[j][n],colors[j+1][n],colors[j+1][i]);
  rig.materialQuad(point(j,i),point(j,n),point(j+1,n),point(j+1,i),11,i/24.f,j/32.f,(i+1)/24.f,(j+1)/32.f,Fade(shade(WHITE,rig.surfaceLight(mesh[j][i].normal)),.25f));}
 // Fine sand ripples follow the same curved skin, just above its surface.
 for(int j:{6,10,14,18,22,26})for(int i=0;i<sides;i++){int n=(i+1)%sides;P3 p=point(j,i)+mesh[j][i].normal*.7f,q=point(j,n)+mesh[j][n].normal*.7f;Color c=shade(col,rig.surfaceLight((mesh[j][i].normal+mesh[j][n].normal)*.5f)*.88f);rig.face(p-P3{0,0,.28f},q-P3{0,0,.28f},q+P3{0,0,.28f},p+P3{0,0,.28f},c);}
 for(int i=0;i<16;i++){int j=8+i%5*4,k=(i*9)%sides;P3 p=point(j,k)+mesh[j][k].normal*.9f;rig.rounded(p,{2.4f,2.4f,1.7f},shade(col,i%2?1.08f:.91f));}
}
#include "desert_fallen_art.hpp"
