#pragma once

// Included after the live desert rigs. These poses reuse their painted material
// cells and are drawn into the corpse cache from the same eight camera headings.
namespace desert_fallen_art {

struct Pose {
 float side;
 float time=1,fold=1,recoil=0;
 AnimationState state{};
 float attack=0;
 bool flash=false;
 float lower(float begin,float end)const{return enemy_death::window(time,begin,end);}
 float stride()const{return (state.motionInitialized?state.moveBlend:(state.moving?1.f:0.f))*(1-fold);}
 P3 point(float r,float f,float z)const{return {r*side,f,z};}
 hero_motion::Transform tilt(P3 center,float bank,float twist=0,float pitch=0)const{
  using namespace hero_motion;
  auto orientation=mul(axisAngle({0,0,1},twist*side),axisAngle({0,1,0},bank*side));
  return {{center.r*side,center.f,center.z},mul(orientation,axisAngle({1,0,0},pitch))};
 }
};

inline void scorpion(Rig& rig,Pose pose){
 const Color shell=pose.flash?Text:rgb(172,111,59),amber=rgb(222,153,79),joint=rgb(105,71,47);
 const Color groove=rgb(108,64,39),lip=rgb(237,173,97);
 const auto p=[&](float r,float f,float z){return pose.point(r,f,z);};
 const auto mix=[](P3 a,P3 b,float t){return a*(1-t)+b*t;};
 const float walk=pose.stride();
 const float walkBob=creature_motion::sample(3,pose.state.phase).bob*walk;
 const float front=pose.attack*(1-pose.fold);
 for(int i=0;i<4;i++)for(float side:{-1.f,1.f}){
  const float f=-19+i*10.f;
  const bool uphill=side<0;
  const auto step=creature_motion::sample(3,pose.state.phase+i*.25f+(side*pose.side<0?.5f:0));
  const float reach=step.travel*walk,fold=pose.lower(uphill?.28f:.09f,uphill?.85f:.72f);
  const auto root=p(side*9,f,13+walkBob+(uphill?-3:-6)*pose.fold);
  const auto knee=mix(p(side*(24+reach*2),f-4+reach*4,8),p(side*(uphill?23:26),f+(uphill?-10:-5),uphill?7.2f:5.f),fold);
  const auto shin=mix(p(side*(31+reach*2),f-10+reach*4,3+step.lift*walk*4),p(side*(uphill?31:32),f+(uphill?-5:1),3.1f),fold);
  const auto tip=mix(shin+p(side*2,-3,-1),p(side*(uphill?29:27),f+(uphill?1:7),1.2f),fold);
  rig.tapered(root,knee,4.65f,4.5f,shell,.76f,8);
  rig.rounded(knee,{4.1f,4.4f,4.2f},shade(joint,1.15f));
  rig.tapered(knee,shin,3.9f,3.55f,shade(shell,.93f),.57f,8);
  rig.tapered(shin,tip,2.2f,2.2f,groove,.13f);
 }
 const auto bank=pose.tilt({pose.recoil*2,-6,16.3f+walkBob-3.6f*pose.fold},21*pose.fold-6*pose.recoil);
 rig.setAttachment(&bank);
 rig.rounded({0,0,-1},{31,40,22},shade(shell,.83f),108);
 for(int i=0;i<5;i++){
  const float f=-15+i*7.f,width=25-std::abs(i-2)*2.6f;
  rig.rounded({0,f,3.2f},{width,12,18-std::abs(i-2)*1.3f},shade(shell,1.03f+i*.025f),108);
  rig.tube({{-width*.38f,f,8},{-width*.21f,f-1.1f,11},{0,f-1.2f,12.4f},{width*.21f,f-1.1f,11},{width*.38f,f,8}},1.25f,1.25f,amber);
 }
 rig.setAttachment(nullptr);
 const auto head=pose.tilt({2*pose.fold,14+3*front,13.5f-4*pose.fold},13*pose.fold,8*pose.fold);
 rig.setAttachment(&head);
 rig.rounded({0,0,0},{25,19,15},shade(shell,.86f),108);
 rig.rounded({0,6,-1},{15,10,10},shade(shell,1.05f),108);
 for(float side:{-1.f,1.f}){
  rig.tapered({side*3,7,5.2f},{side*7,5.8f,5.9f},1.05f,1.05f,groove);
  rig.spike({side*3,10,0},{side*5,14,-.5f},2.6f,2.8f,groove);
 }
 rig.setAttachment(nullptr);
 for(float side:{-1.f,1.f}){
  const float release=pose.lower(side<0?.28f:.18f,.89f);
  const float r=side*(25+(side<0?0:3)*release);
  const float f=38+13*front-(side<0?5:12)*release,z=16-(side<0?7.4f:10.2f)*release;
  const auto palm=p(r,f,z),elbow=p(side*(24-7*release),f-12,13-(side<0?5:7)*release);
  rig.tapered(p(side*8,18,12-4*pose.fold),elbow,7.2f,6.1f,shell,.88f,8);
  rig.rounded(elbow,{7.7f,8.5f,7.2f},joint,108);
  rig.tapered(elbow,palm,6.9f,6.6f,shell,1.13f,8);
  auto hinge=pose.tilt({r,f,z},-side*16*release,side*12*release);
  rig.setAttachment(&hinge);
  rig.rounded({0,0,0},{16,20,12.8f},shade(shell,1.23f),108);
  rig.tapered({-4.8f,5,1.1f},{-7.f,14,.1f},6.9f,6.3f,amber,.62f,8);
  rig.spike({-7,13.8f,.1f},{-3.8f,19,-1.65f},4.3f,4.1f,lip);
  rig.tapered({5.3f,5,1.8f},{7.1f,11,.9f},6.2f,6.1f,shell,.63f,8);
  rig.spike({7.1f,11,.9f},{3.4f,16,-1.5f},3.8f,3.8f,amber);
  rig.setAttachment(nullptr);
 }
 // The tail folds beside the thorax rather than losing the shape of its joints.
 constexpr std::array<P3,8> tail{{{0,-24,12},{3,-33,16},{13,-42,22},{24,-43,26},
                                  {34,-39,23},{40,-30,16},{39,-20,8},{34,-13,4.1f}}};
 P3 previous{};
 for(std::size_t i=0;i<tail.size();i++){
  const auto item=tail[i];const float n=float(i),a=n*.34f;
  const float unwind=pose.lower(.05f+n*.025f,.68f+n*.025f);
  const auto raised=p(std::sin(pose.state.idleTime*1.8f)*n*.45f*(1-pose.fold)*pose.side+n*pose.recoil,
                      -25-std::sin(a)*25+front*n*5,18+n*8-std::max(0.f,n-4)*9);
  const auto center=mix(raised,p(item.r,item.f,item.z),unwind);
  const float width=11.7f-float(i)*.84f;
  if(i)rig.tapered(previous,center,width*.69f,width*.63f,joint);
  rig.rounded(center,{width,width+1,width*.86f},shade(shell,1.02f+.024f*float(i)),108);
  rig.rounded(center+P3{0,-.7f,width*.40f},{width*.50f,width*.47f,1.7f},shade(amber,.99f));
  previous=center;
 }
 rig.spike(previous,previous+mix(p(0,14,-12),p(-10,2,-2.65f),pose.lower(.27f,.89f)),5.3f,4.1f,rgb(104,60,36));
}

inline void scarab(Rig& rig,Pose pose){
 const Color blue=pose.flash?Text:rgb(66,113,109),teal=pose.flash?Text:rgb(89,139,124),copper=pose.flash?Text:rgb(166,105,62);
 const Color bright=rgb(231,176,99),under=rgb(90,67,53),patina=rgb(49,92,84);
 const auto p=[&](float r,float f,float z){return pose.point(r,f,z);};
 const float walk=pose.stride(),front=pose.attack*(1-pose.fold);
 const float walkBob=creature_motion::sample(3,pose.state.phase).bob*walk;
 const float tip=pose.lower(.13f,.86f);
 for(int i=0;i<3;i++)for(float side:{-1.f,1.f}){
  const float f=-16+i*13.7f;const bool uphill=side<0;
  auto step=creature_motion::sample(3,pose.state.phase+i/3.f+(side*pose.side<0?.5f:0));
  const float fold=pose.lower(uphill?.30f:.08f,uphill?.93f:.69f),reach=step.travel*walk*6;
  const auto root=p(side*11,f,17+walkBob-(uphill?3:8)*tip);
  const auto knee=p(side*(25+fold*(-2+i*.8f)),f+reach-fold*(i==1?4:7),10-(uphill?3:5.1f)*fold);
  const auto ankle=p(side*(33-(uphill?8:3)*fold),f+reach-7+(uphill?8:12)*fold,2.7f+step.lift*walk*5);
  rig.tapered(root,knee,5.2f,5,copper,.81f,8);
  rig.rounded(knee,{5.6f,5.2f,5.4f},under,108);
  rig.tapered(knee,ankle,4.25f,3.65f,shade(copper,1.11f),.54f);
  rig.tapered(ankle,ankle+p(-side*4,4+fold*(uphill?1:3),-1.25f),2.1f,2.1f,under,.22f);
 }
 const auto bank=pose.tilt({2*pose.recoil,-2,24+walkBob-8.1f*tip},32*tip-12*pose.recoil,-8*tip);
 rig.setAttachment(&bank);
 rig.rounded({0,0,-4},{34,42,15},under,108);
 rig.rounded({0,0,0},{40,44,26},patina,109);
 rig.rounded({-8.3f,-1.2f,3},{19.6f,41.8f,29},blue,109);
 rig.rounded({8.3f,-1.7f,2},{19.6f,40.4f,27},teal,109);
 rig.tube({{0,-21,4},{0,-13,12.8f},{0,0,17.5f},{0,12,12.8f},{0,19,5}},1.55f,1.0f,under);
 for(float side:{-1.f,1.f}){
  const Color sidePaint=side<0?bright:shade(copper,1.18f);
  rig.tube({{side*6,-20,4},{side*14,-16,8},{side*18,-7,9.2f},{side*18,4,9.5f},{side*15,13,7.2f},{side*9,18,4}},1.45f,1.2f,sidePaint);
  for(int i=0;i<5;i++){
   const float f=-13+i*6.f;
   const float bow=std::sin((i+1)/6.f*PI);
   rig.tapered({side*5,f,12.6f+bow*3.5f},{side*10.1f,f-1.3f,12.5f+bow*3.6f},1.05f,1.05f,sidePaint);
   rig.tapered({side*10.0f,f-1.3f,12.5f+bow*3.6f},{side*15,f-3,10.4f+bow*2.7f},.8f,.8f,sidePaint);
  }
 }
 rig.setAttachment(nullptr);
 const auto head=pose.tilt({5*tip,19.9f+5*front,19-11.9f*tip},14*tip,-18*tip);
 rig.setAttachment(&head);
 rig.rounded({0,0,0},{24,20,14},copper,108);
 rig.rounded({0,2,2.1f},{15,14,9.7f},shade(copper,1.18f),108);
 for(float side:{-1.f,1.f}){
  rig.tapered({side*5,5.2f,4.8f},{side*8.6f,4.7f,4.4f},1.02f,1.2f,under);
  rig.tapered({side*7,6,-1},{side*11,14+front*11,-1.8f+front*3},4.1f,3.7f,shade(copper,.86f),.30f);
  rig.tube({{side*7,6.6f,3.5f},{side*12,11,2.8f},{side*17,12,0}},1.25f,1.2f,under);
 }
 rig.setAttachment(nullptr);
}

inline void cobra(Rig& rig,Pose pose){
 const Color gold=pose.flash?Text:rgb(182,136,69),scales=pose.flash?Text:rgb(153,112,61),belly=rgb(224,186,113);
 const Color dark=rgb(101,70,43),ivory=rgb(232,214,163);
 const auto p=[&](float r,float f,float z){return pose.point(r,f,z);};
 const auto mix=[](P3 a,P3 b,float t){return a*(1-t)+b*t;};
 const float walk=pose.stride(),strike=pose.attack*(1-pose.fold),body=pose.lower(.16f,.91f);
 const auto step=creature_motion::sample(4,pose.state.phase);
 std::vector<P3> coil;
 for(int i=0;i<=29;i++){
  const float t=i/29.f,angle=.28f+t*PI*1.94f;
  const float r=8.5f+16.5f*t,f=8+13*t;
  const auto raised=P3{std::cos(angle)*(23-8*t)+step.sway*walk*std::sin(i*.25f)*2,
                       std::sin(angle)*(19-8*t)-9,3.1f+7.9f*t};
  coil.push_back(mix(raised,p(std::cos(angle)*r,std::sin(angle)*f-13,3.1f+5.3f*t),body));
 }
 for(int i=0;i<8;i++){
  const float width=3.0f+.87f*i;
  rig.tapered(coil[i],coil[i+1],width,width*.89f,scales,(width+.87f)/width,10);
 }
 const std::vector<P3> outer(coil.begin()+8,coil.end());
 rig.tube(outer,12.3f,12.5f,scales,10);
 rig.spike(coil.front(),mix(coil.front()+P3{-4,3,-1.4f},p(4.2f,-11,1.55f),body),2.7f,2.f,dark);
 for(int i=9;i<=29;i+=2){
  const auto root=coil[i];const auto next=coil[i<29?i+1:i-1];
  const P3 delta=next-root;
  const float length=std::max(1.f,std::hypot(delta.r,delta.f));
  const P3 across{delta.f/length,-delta.r/length,0};
  rig.tube({root+across*4.3f+P3{0,0,3.6f},root+P3{0,0,6.1f},root-across*4.3f+P3{0,0,3.6f}},.98f,.83f,shade(gold,i%4==1?1.05f:.79f));
 }
 const std::array<P3,6> settled{{coil.back(),p(25,3,11),p(24,16,12.5f),p(17,26,12),p(10,37,8.7f),p(7,44,6.5f)}};
 std::vector<P3> neck{coil.back()};
 for(int i=1;i<=5;i++){
  const float n=i/5.f,fold=pose.lower(.27f-.17f*n,.93f-.09f*n);
  const float sway=std::sin(pose.state.idleTime*1.5f+i*.12f)*2*(1-body);
  const P3 raised{sway+n*pose.recoil*pose.side*9,4+step.sway*walk*3+strike*n*24,11+n*59};
  neck.push_back(mix(raised,settled[i],fold));
 }
 rig.tube(neck,15.4f,13.0f,gold,10);
 for(int i=0;i<4;i++){
  const auto root=neck[i+1];const auto forward=neck[i+2]-root;
  const P3 across{forward.f,-forward.r,0};const float k=7.f/std::max(1.f,std::hypot(across.r,across.f));
  rig.tapered(root+across*k+P3{0,0,.2f},root+across*k*.82f+forward*.72f+P3{0,0,-.2f},2.8f,1.3f,belly,.76f);
 }
 const float hoodFold=pose.lower(.18f,.88f);
 const auto hoodCenter=mix(P3{neck[4].r*pose.side,neck[4].f-2,neck[4].z-7},P3{14.5f,29.7f,11.4f},hoodFold);
 const auto hood=pose.tilt(hoodCenter,47*hoodFold,-25*hoodFold,74*(1-hoodFold));
 rig.setAttachment(&hood);
 rig.rounded({-8.8f,-2.2f,.2f},{25,28.4f,12.3f},scales,110);
 rig.rounded({7.9f,-2,-.7f},{22.2f,27.6f,10.4f},gold,110);
 for(float side:{-1.f,1.f}){
  rig.tube({{side*9,6.4f,3.4f},{side*15,1.4f,3.3f},{side*13,-5,3.6f},{side*9,-8,3.5f}},1.5f,1.0f,shade(dark,.91f));
  rig.tube({{side*10,4,4.2f},{side*12.9f,.5f,4.1f},{side*11.2f,-3.2f,4.4f}},.83f,.8f,ivory);
 }
 rig.setAttachment(nullptr);
 const float headFold=pose.lower(.10f,.84f);
 const auto headCenter=mix(P3{neck.back().r*pose.side,neck.back().f+8+strike*9,neck.back().z+6},P3{6.7f,43.2f,8.15f},headFold);
 const auto head=pose.tilt(headCenter,37*headFold,-22*headFold,10*(1-headFold));
 rig.setAttachment(&head);
 rig.rounded({0,0,1.1f},{20,22,13.8f},gold,110);
 rig.rounded({0,6,-3},{16.1f,13.2f,6.7f},belly);
 rig.tube({{-6,2.5f,-1.5f},{-6.5f,8.2f,-1.4f},{0,12,-1.3f},{6.5f,8.2f,-1.4f},{6,2.5f,-1.5f}},.76f,.75f,dark);
 for(float side:{-1.f,1.f}){
  rig.tapered({side*4.4f,5.4f,5.9f},{side*7.2f,2.7f,5.6f},1.16f,1.f,dark);
  if(headFold<1)rig.rounded({side*6,5.5f,5.4f},{3.2f,2.1f,2.8f*(1-headFold)},rgb(222,157,66));
  rig.spike({side*4.8f,8.2f,-1},{side*5.1f,9.2f,-4.45f},1.9f,1.70f,ivory);
 }
 rig.setAttachment(nullptr);
}

inline void dune(Rig& rig,Pose pose){
 const Color sand=pose.flash?Text:rgb(219,175,108),dark=pose.flash?Text:rgb(144,93,54),amber=rgb(246,202,105);
 const float walk=pose.stride(),front=pose.attack*(1-pose.fold);
 const float collapse=pose.lower(.06f,.91f);
 const float sway=std::sin(pose.state.idleTime*1.5f)*3*(1-pose.fold)
                  +creature_motion::sample(5,pose.state.phase).sway*walk*4-8*pose.recoil;
 const auto at=[&](P3 p){return sandCollapse(p,collapse,pose.side);};
 sandMound(rig,sway,sand,collapse,pose.side);
 // The limbs run into the same sand surface as the moving crest.
 const float sink=pose.lower(.12f,.83f),shrink=1-sink;
 if(shrink>.001f){
  for(int i=0;i<6;i++){
   const float side=i%2?1.f:-1.f,f=-29+i/2*25.f;
   const float step=creature_motion::sample(5,pose.state.phase+i/6.f).travel*walk;
   const auto knee=at({side*(43+step*5-19*sink),f,14}),foot=at({side*(59+step*7-26*sink),f+step*10,3});
   rig.rounded(knee,P3{30,25,25}*shrink,sand,111);
   rig.tapered(knee,foot,21*shrink,18*shrink,dark,.35f);
  }
  for(float side:{-1.f,1.f}){
   const auto shoulder=at({side*42,6,49+sway});
   const auto hand=at({side*(64-front*29-25*sink),27+front*48-12*sink,16+front*15});
   rig.tapered(shoulder,hand,28*shrink,23*shrink,sand,.35f);
   for(int i=0;i<4;i++)rig.form(hand+P3{(i-1.5f)*7,10+i*3.f,-4+i%2*3.f}*shrink,
                               P3{6,8,4}*shrink,shade(sand,.83f+i*.08f));
  }
 }
 const float open=1-pose.lower(.16f,.78f);
 for(float side:{-1.f,1.f}){
  const auto eye=at(sandFront(-13+side*13,83+sway+front*5,sway,1.5f));
  rig.rounded(eye,{14,4,1.6f+8.4f*open},dark);
  if(open>.001f){
   rig.rounded(eye+P3{0,2.6f*open,0},{9*open,3*open,5*open},rgb(255,223,118));
   rig.rounded(eye+P3{side,4.2f,1}*open,{4*open,2*open,2*open},Text);
  }
  rig.beam(at(sandFront(-13+side*6,91+sway+front*5,sway,1)),
           at(sandFront(-13+side*21,94+sway+front*5,sway,1)),1.2f+2.8f*open,1.2f+2.8f*open,sand);
 }
 rig.rounded(at(sandFront(-13,67+sway+front*5,sway,1)),{31,3,1.3f+4.7f*open},dark);
 if(open>.001f)for(int i=0;i<7;i++){
  const auto tooth=at(sandFront(-25+i*4.f,70+sway+front*5,sway,3));
  rig.spike(tooth,tooth+P3{0,1,-6}*open,1.8f*open,1.8f*open,amber);
 }
}

inline void drawPose(Rig& rig,int type,const Pose& pose){
 rig.setCreatureSurfaces();
 if(pose.time>0)rig.setGroundBounded();
 switch(type){
 case 8:scorpion(rig,pose);break;
 case 9:scarab(rig,pose);break;
 case 10:cobra(rig,pose);break;
 case DesertBossType:dune(rig,pose);break;
 }
}
}

inline void buildDesert(Rig& rig,int type,const AnimationState& state,float attack,bool flash){
 desert_fallen_art::drawPose(rig,type,{1,0,0,0,state,std::max(0.f,attack),flash});
}

inline void buildDesertCorpse(Rig& rig,int type,int side=1){
 desert_fallen_art::drawPose(rig,type,{side<0?-1.f:1.f});
}

inline void buildDesertDeath(Rig& rig,const Enemy& e,float progress){
 const float contact=enemy_death::groundContact(e.type);
 const float time=std::clamp(progress/contact,0.f,1.f);
 const float facing=e.deathMotion.facingInitialized?e.deathMotion.facingAngle:int(e.deathMotion.direction)*PI/4;
 const float rest=int(e.animation.direction)*PI/4;
 rig.setFacing(facing+std::remainder(rest-facing,2*PI)*enemy_death::window(time,.05f,.90f));
 const float fold=enemy_death::window(time,.06f,.89f);
 const float recoil=.45f*enemy_death::window(time,0,.21f)*(1-enemy_death::window(time,.24f,.58f));
 desert_fallen_art::drawPose(rig,e.type,{float(e.deathSide()),time,fold,recoil,e.deathMotion,std::max(0.f,e.deathAttack)});
}
