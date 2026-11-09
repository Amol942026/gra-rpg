#pragma once
// Original Krita surface marks on the existing 2D articulated renderer.
// Synfig contact curves are scaled to each creature's anatomy. No new models.
inline float creatureWalk(const AnimationState& s){return s.motionInitialized?s.moveBlend:(s.moving?1.f:0.f);}
inline void creatureSkin(Rig& r,P3 p,P3 size,Color color,int material){r.rounded(p,size,color,100+material);}
inline void creatureJoint(Rig& r,P3 a,P3 b,float width,Color color,float end=.72f){
 r.tapered(a,b,width,width,color,end);r.rounded(a,{width,width,width},color);
}
inline void creatureEye(Rig& r,P3 p,float size,Color iris){
 r.rounded(p,{size*1.25f,2,size},rgb(37,45,36));
 r.rounded(p+P3{0,1.1f,0},{size*.8f,1.4f,size*.72f},iris);
 r.rounded(p+P3{.15f,1.8f,.3f},{size*.22f,.7f,size*.6f},Ink);
 r.rounded(p+P3{-.35f,2,.65f},{.7f,.5f,.7f},Text);
}
#include "bandit_art.hpp"
#include "forest_detail_art.hpp"

inline void buildForestCreature(Rig& r,int type,const AnimationState& s,float attack,bool flash,float guardianThrow=-1){
 r.setCreatureSurfaces();r.reserve(1700);
 const float walk=creatureWalk(s);
 switch(type){
 case 0:forest_detail::slime(r,s,walk,attack,flash);return;
 case 1:forest_detail::wolf(r,s,walk,attack,flash);return;
 case 2:forest_detail::skeleton(r,s,walk,attack,flash);return;
 case 6:buildBandit(r,s,attack,flash);return;
 case 7:forest_detail::treant(r,s,walk,attack,flash);return;
 }
 float t=s.idleTime,hit=std::clamp(attack,-.32f,1.f);
 int family=type==4?1:type==5?3:2;
 auto motion=creature_motion::sample(family,s.phase);
 float bob=motion.bob*walk+std::sin(t*2.1f)*.25f*(1-walk);
 Color bone=flash?Text:rgb(203,199,159);
 if(type==4){
  Color fur=flash?Text:rgb(115,83,57);
  float bodyZ=25+bob;
  for(float side:{-1.f,1.f})for(int pair=0;pair<2;pair++){
   float f=pair?11.f:-14.f;auto k=creature_motion::sample(1,s.phase+(side>0?.5f:0)+(pair?.5f:0));
   P3 foot{side*9,f+k.travel*walk*5.5f,3+k.lift*walk*4.5f};
   P3 hip{side*10,f,bodyZ-3},knee{side*10,f+(pair?4:-5)+k.travel*walk*2,14+bob};
   creatureJoint(r,hip,knee,10,shade(fur,.87f),.7f);
   creatureJoint(r,knee,foot,5.5f,shade(fur,.8f),.66f);
   r.rounded(foot+P3{0,2,-1},{8,10,4},rgb(51,44,35));
   for(float toe:{-1.4f,1.4f})r.tapered(foot+P3{toe,5,0},foot+P3{toe,7,-.5f},.6f,.6f,Ink);
  }
  creatureSkin(r,{0,-2,bodyZ},{29,41,26},fur,4);
  creatureSkin(r,{0,11,bodyZ+4},{27,21,25},shade(fur,.94f),4);
  P3 head{0,22+hit*10,bodyZ+7-hit*6};
  creatureSkin(r,head,{24,25,23},fur,4);
  P3 muzzle=head+P3{0,15,-7};
  creatureSkin(r,muzzle,{19,12,12},rgb(163,113,83),14);
  r.rounded(muzzle+P3{0,6,1},{16,3,9},rgb(181,126,96));
  for(float side:{-1.f,1.f}){
   creatureEye(r,head+P3{side*8,8,3},3.5f,Gold);
   r.spike(head+P3{side*7,-5,8},head+P3{side*10,-7,19},11,7,shade(fur,.76f));
   r.spike(head+P3{side*7,-2,10},head+P3{side*9,-3,16},5,2,rgb(156,107,84));
   r.tube({muzzle+P3{side*8,0,-4},muzzle+P3{side*12,3,0},muzzle+P3{side*12,7,7}},3.3f,3.3f,bone);
  }
  for(int n=0;n<9;n++){
   float f=-20+n*4.5f,z=bodyZ+10+std::sin(n*PI/9)*4;
   r.spike({0,f,z},{0,f-3,z+7},4,4,shade(fur,.55f));
  }
  float wag=motion.sway*walk*2+std::sin(t*2.2f);
  r.tube({{0,-23,bodyZ},{4,-29,bodyZ+2},{7+wag,-30,bodyZ+5},{5+wag,-27,bodyZ+6}},2,2,fur);
 }else if(type==5){
  Color shell=flash?Text:rgb(62,77,67),venom=rgb(168,192,92);float z=18+bob;
  for(int i=0;i<4;i++)for(float side:{-1.f,1.f}){
   auto k=creature_motion::sample(3,s.phase+i*.25f+(side>0?.5f:0));float f=-16+i*10.f;
   P3 root{side*10,f,z-4},joint{side*24,f-4+k.travel*walk*2,z+4},foot{side*34,f-8+k.travel*walk*6,1.5f+k.lift*walk*5};
   creatureJoint(r,root,joint,4.3f,shell,.72f);creatureJoint(r,joint,foot,3.2f,shade(shell,.88f),.35f);
   r.rounded(joint,{4.5f,4.5f,4.5f},shade(shell,1.15f));
  }
  creatureSkin(r,{0,-12,z+3},{29,33,25},shell,5);
  r.tapered({0,-2,z+1},{0,8+hit*6,z},6,6,shade(shell,.85f),.75f);
  creatureSkin(r,{0,12+hit*6,z},{22,23,18},shade(shell,1.16f),5);
  for(float side:{-1.f,1.f}){
   for(int eye=0;eye<3;eye++)creatureEye(r,{side*(3+eye*3.3f),23+hit*6-eye,23-eye*2+bob},eye?2.2f:4,venom);
   r.tube({{side*5,23+hit*6,14+bob},{side*9,29+hit*9,11+bob},{side*6,33+hit*9,6+bob}},2.7f,2.7f,bone);
   for(int n=0;n<4;n++)r.spike({side*11,-24+n*6.f,z+8},{side*15,-25+n*6.f,z+11},1.2f,1.2f,shade(shell,1.2f));
  }
 }else{
  Color stone=flash?Text:rgb(104,134,123);
  float lift=bob*.5f;
  bool throwing=guardianThrow>=0;auto throwPose=guardian_motion::sample(std::max(0.f,guardianThrow));
  float lean=throwing?throwPose.bodyForward:0,lower=throwing?throwPose.bodyUp:0;lift+=lower;
  float gaitHip=guardian_gait::hipHeight(s.phase);
  lift+=(gaitHip-31-(lift-lower))*walk;
  for(float side:{-1.f,1.f}){
   auto k=creature_motion::sample(2,s.phase+(side>0?.5f:0));
   P3 foot{side*10,k.travel*walk*guardian_gait::Stride,
           4+(guardian_gait::footHeight(s.phase+(side>0?.5f:0),gaitHip)-4)*walk};
   P3 knee{side*10,3+k.travel*walk*3-lower*.55f,17+lift*(throwing?.45f:1.f)};
   auto bend=guardian_gait::knee(foot.f-lean*.35f,31+lift,foot.z);
   knee.f+=(lean*.35f+bend.forward-knee.f)*walk;knee.z+=(bend.height-knee.z)*walk;
   creatureJoint(r,{side*10,lean*.35f,31+lift},knee,12,stone);
   creatureJoint(r,knee,foot,12*.85f,stone);
   r.rounded(foot+P3{0,3,-1},{14,20,6},shade(stone,.86f));
  }
  creatureSkin(r,{0,lean*.35f,30+lift},{29,14,9},stone,3);
  creatureSkin(r,{0,lean,45+lift},{36,24,29},stone,3);
  r.tube({{-4,12+lean,48+lift},{0,14+lean,53+lift},{4,12+lean,48+lift},{0,14+lean,41+lift},{-4,12+lean,48+lift}},1.8f,1.8f,rgb(219,210,129));
  for(float side:{-1.f,1.f})creatureSkin(r,{side*12,-3+lean,57+lift},{10,13,8},rgb(87,125,78),12);
  for(float side:{-1.f,1.f}){
   float reach=hit*(side>0?24:9),shoulder=side*23;
   P3 elbow{shoulder+side*2,1-motion.sway*walk*side*2+reach*.45f,43+lift+hit*3};
   P3 hand{side*27,5-motion.sway*walk*side*4+reach,33+lift+hit*11};
   if(throwing){
    float bounce=lift-lower;
    elbow={side*throwPose.elbowSide,throwPose.elbowForward,throwPose.elbowUp+bounce};
    hand={side*throwPose.handSide,throwPose.handForward,throwPose.handUp+bounce};
   }
   creatureJoint(r,{shoulder,lean,53+lift},elbow,12,stone,.75f);
   creatureJoint(r,elbow,hand,12*.85f,stone,.72f);
   creatureSkin(r,{shoulder,lean,54+lift},{17,13,10},stone,3);
   r.rounded(hand,{13,12,13},stone);
  }
  float hz=67+lift;
  r.rounded({0,lean*.8f,hz},{25,23,23},stone,103);
  for(float side:{-1.f,1.f}){
   r.rounded({side*5.5f,10.8f+lean*.8f,hz+2},{8,3,5},rgb(48,70,63));
   r.rounded({side*5.5f,12.2f+lean*.8f,hz+2},{6,2,2.3f},Gold);
   r.tapered({side*2,11+lean*.8f,hz+6},{side*10,8+lean*.8f,hz+8},4,3,stone,.7f);
  }
 }
}
