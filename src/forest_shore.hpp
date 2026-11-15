#pragma once

namespace forest_shore {
inline constexpr const char* Material=R"GLSL(
float bankShape(float along,float side){
 return .08+.64*sin(along*.54+side*1.62)+.33*sin(along*1.21+side*2.04)
     +.15*sin(along*2.8+side*.84)+.08*sin(along*6.7+side);
}
float bankDistance(vec2 p,float high){
 vec4 land=vec4(p-.5,vec2(high)-p);
 land+=vec4(bankShape(p.y,0.),bankShape(p.x,1.),bankShape(p.y,2.),bankShape(p.x,3.));
 return min(min(land.x,land.y),min(land.z,land.w));
}
float cliffRim(float along,float side,vec2 limits){
 float corner=clamp(min(along-limits.x,limits.y-along)*.5,0.,1.);
 return .29+corner*(.15*sin(along*.69+side*2.3)+.10*sin(along*1.92+side*3.4));
}
float forestCliffInset(vec2 p,vec2 limits){
 vec4 edge=vec4(p-vec2(limits.x),vec2(limits.y)-p);
 edge+=vec4(cliffRim(p.y,0.,limits),cliffRim(p.x,1.,limits),cliffRim(p.y,2.,limits),cliffRim(p.x,3.,limits));
 return min(min(edge.x,edge.y),min(edge.z,edge.w));
}
)GLSL";

inline float cliffRim(float along,int side){
 const float corner=std::clamp(std::min(along-MapWalkMin,MapWalkMax-along)*.5f,0.f,1.f);
 return .29f+corner*(.15f*std::sin(along*.69f+side*2.3f)+.10f*std::sin(along*1.92f+side*3.4f));
}
inline V walkEdgePoint(float along,int side,float inset){
 switch(side){
  case 0:return {MapWalkMin+inset,along};
  case 1:return {along,MapWalkMin+inset};
  case 2:return {MapWalkMax-inset,along};
  default:return {along,MapWalkMax-inset};
 }
}

inline void bakeRimGrass(){
 auto sedge=[](V p,float scale){
  const float x=TerrainOriginX+(p.x-p.y)*40,y=90+(p.x+p.y)*20;
  const unsigned seed=Game::hash(int(p.x*107),int(p.y*107));
  for(int i=0;i<13;i++){
   const unsigned blade=Game::hash(int(seed%100000)+i,93);
   const float bx=x+(int(blade%41)-20)*scale,by=y+(int((blade>>8)%14)-7)*scale;
   const float h=(13+(blade>>13)%30)*scale,tip=bx+(int((blade>>19)%27)-13)*scale;
   const Color green=rgb(54+(blade>>6)%40,68+(blade>>11)%36,39+(blade>>18)%22);
   poly({{bx-1.7f*scale,by},{bx+1.9f*scale,by},{bx+(tip-bx)*.35f,by-h*.68f},{tip,by-h}},green);
   if(i%5==0)line(bx,by,tip,by-h,shade(green,1.18f),.9f*scale);
  }
 };
 for(int side=0;side<4;side++)for(int site=6;site<MapSize-6;site++){
  const unsigned seed=Game::hash(site+151,side+519);
  const float along=site+((seed>>9)%73)*.013f;
  if((seed>>18)%7<2){
   sedge(walkEdgePoint(along,side,.46f),.32f+((seed>>14)%20)*.013f);
   if((seed>>24)%5<2)sedge(walkEdgePoint(along+.36f,side,.67f),.33f);
  }
 }
}
}
