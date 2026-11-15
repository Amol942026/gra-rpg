#pragma once
// Original Krita surface marks on the existing 2D articulated renderer.
// Synfig contact curves are scaled to each creature's anatomy. No new models.
inline float creatureWalk(const AnimationState& s){return s.motionInitialized?s.moveBlend:(s.moving?1.f:0.f);}
inline void creatureSkin(Rig& r,P3 p,P3 size,Color color,int material){r.rounded(p,size,color,100+material);}
inline void creatureJoint(Rig& r,P3 a,P3 b,float width,Color color,float end=.72f,int material=-1){
 r.tapered(a,b,width,width,color,end,material);
 r.rounded(a,{width,width,width},color,material<0?-1:100+material,false,-1,true);
}
inline void creatureEye(Rig& r,P3 p,float size,Color iris){
 r.rounded(p,{size*1.25f,2,size},rgb(37,45,36));
 r.rounded(p+P3{0,1.1f,0},{size*.8f,1.4f,size*.72f},iris);
 r.rounded(p+P3{.15f,1.8f,.3f},{size*.22f,.7f,size*.6f},Ink);
 r.rounded(p+P3{-.35f,2,.65f},{.7f,.5f,.7f},Text);
}
#include "bandit_art.hpp"
#include "forest_detail_art.hpp"
#include "forest_boar_art.hpp"

inline float heavyMeleePreview(float phase,float attack,float contact){
 if(phase>=0)return phase;
 return attack<0?-attack*contact:attack>0?contact+(1-attack)*(1-contact):-1;
}
inline void buildForestCreature(Rig& r,int type,const AnimationState& s,float attack,bool flash,float guardianThrow=-1,float enemyMelee=-1){
 r.setCreatureSurfaces();r.reserve(1700);
 const float walk=creatureWalk(s);
 switch(type){
 case 0:forest_detail::slime(r,s,walk,attack,flash);return;
 case 1:forest_detail::wolf_art::draw(r,s,walk,heavyMeleePreview(enemyMelee,attack,meleeWindup(type)/(meleeWindup(type)+meleeRecovery(type))),flash);return;
 case 2:forest_detail::skeleton(r,s,walk,attack,flash);return;
 case BoarType:forest_boar_art::draw(r,s,walk,attack,enemyMelee,flash);return;
 case 6:buildBandit(r,s,attack,flash);return;
 case 7:forest_detail::treant(r,s,walk,heavyMeleePreview(enemyMelee,attack,TreantSwingWindup/(TreantSwingWindup+TreantSwingRecovery)),flash);return;
 }
 float t=s.idleTime,hit=std::clamp(attack,-.32f,1.f);
 int family=type==5?3:2;
 auto motion=creature_motion::sample(family,s.phase);
 float bob=motion.bob*walk+std::sin(t*2.1f)*.25f*(1-walk);
 Color bone=flash?Text:rgb(203,199,159);
 if(type==5){
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
  const bool throwing=guardianThrow>=0;
  const float punchPhase=heavyMeleePreview(enemyMelee,attack,GuardianPunchWindup/(GuardianPunchWindup+GuardianPunchRecovery));
  const auto throwPose=throwing?guardian_motion::sample(guardianThrow):guardian_motion::Pose{};
  const auto punch=punchPhase>=0?forest_melee::sample(forest_melee::guardianPunch,punchPhase):forest_melee::guardianPunch[0];
  const float lean=throwing?throwPose.bodyForward:punch.lean,lower=throwing?throwPose.bodyUp:punch.drop;
  lift+=lower;
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
  if(!throwing)r.setBodyYaw(-punch.twist*DEG2RAD);
  creatureSkin(r,{0,lean,45+lift},{36,24,29},stone,3);
  r.tube({{-4,12+lean,48+lift},{0,14+lean,53+lift},{4,12+lean,48+lift},{0,14+lean,41+lift},{-4,12+lean,48+lift}},1.8f,1.8f,rgb(219,210,129));
  for(float side:{-1.f,1.f})creatureSkin(r,{side*12,-3+lean,57+lift},{10,13,8},rgb(87,125,78),12);
  for(float side:{-1.f,1.f}){
   P3 shoulder{side*23,lean,53+lift};
   P3 elbow{},hand{};
   if(throwing){
    float bounce=lift-lower;
    elbow={side*throwPose.elbowSide,throwPose.elbowForward,throwPose.elbowUp+bounce};
    hand={side*throwPose.handSide,throwPose.handForward,throwPose.handUp+bounce};
   }else{
    const auto idle=forest_melee::guardianPunch[0];
    const auto motionKey=side>0?punch:idle;
    const float sway=punchPhase<0?motion.sway*walk*side*4:0;
    const auto arm=enemy_arm::solve({shoulder.r,shoulder.f,shoulder.z},
     {shoulder.r+side*motionKey.side,shoulder.f+motionKey.forward-sway,shoulder.z+motionKey.up},
     enemy_arm::GuardianUpper,enemy_arm::GuardianForearm);
    elbow={arm.elbow.side,arm.elbow.forward,arm.elbow.up};
    hand={arm.hand.side,arm.hand.forward,arm.hand.up};
   }
   creatureJoint(r,shoulder,elbow,12,stone,.75f);
   creatureJoint(r,elbow,hand,12*.85f,stone,.72f);
   creatureSkin(r,shoulder+P3{0,0,1},{17,13,10},stone,3);
   r.rounded(hand,{13,12,13},stone);
  }
  float hz=67+lift;
  r.rounded({0,lean*.8f,hz},{25,23,23},stone,103);
  for(float side:{-1.f,1.f}){
   r.rounded({side*5.5f,10.8f+lean*.8f,hz+2},{8,3,5},rgb(48,70,63));
   r.rounded({side*5.5f,12.2f+lean*.8f,hz+2},{6,2,2.3f},Gold);
   r.tapered({side*2,11+lean*.8f,hz+6},{side*10,8+lean*.8f,hz+8},4,3,stone,.7f);
  }
  r.setBodyYaw(0);
 }
}
