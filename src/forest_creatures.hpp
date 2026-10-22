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
inline void buildForestCreature(Rig& r,int type,const AnimationState& s,float attack,bool flash){
 r.setCreatureSurfaces();r.reserve(1700);
 float walk=creatureWalk(s),t=s.idleTime,hit=std::clamp(attack,-.32f,1.f);
 int family=type==0?0:type==1||type==4?1:type==5?3:type==7?5:2;
 auto motion=creature_motion::sample(family,s.phase);
 float bob=motion.bob*walk+std::sin(t*2.1f)*.25f*(1-walk);
 Color bone=flash?Text:rgb(203,199,159),iron=flash?Text:rgb(139,162,151);
 if(type==0){
  Color gel=flash?Text:rgb(101,165,108);float squash=motion.sway*walk*.09f+std::sin(t*2.4f)*.018f;
  float h=32*(1+squash)-hit*5,lean=hit*9+motion.travel*walk*2;
  creatureSkin(r,{0,lean,h*.48f},{44*(1-squash)+hit*5,43*(1-squash),h},gel,0);
  for(float side:{-1.f,1.f}){
   r.rounded({side*15,lean+2,4},{18,21,7},shade(gel,.9f));
   creatureEye(r,{side*7,lean+18,h*.58f},5.2f,rgb(228,233,159));
  }
  r.tube({{-4,lean+20,9},{0,lean+21,8},{4,lean+20,9}},1,1,rgb(46,90,61));
  r.rounded({-11,lean+8,h*.74f},{4,3,7},rgb(194,224,160));
  r.rounded({-10,lean+10,h*.85f},{2,2,2},Text);
 }else if(type==1||type==4){
  bool boar=type==4;Color fur=flash?Text:boar?rgb(115,83,57):rgb(132,149,140);
  float bodyZ=(boar?25:28)+bob,front=boar?11:15,back=boar?-14:-16;
  for(float side:{-1.f,1.f})for(int pair=0;pair<2;pair++){
   float f=pair?front:back;auto k=creature_motion::sample(1,s.phase+(side>0?.5f:0)+(pair?.5f:0));
   P3 foot{side*(boar?9:8),f+k.travel*walk*(boar?5.5f:7),3+k.lift*walk*(boar?4.5f:6)};
   P3 hip{side*(boar?10:8),f,bodyZ-3},knee{side*(boar?10:8),f+(pair?4:-5)+k.travel*walk*2,14+bob};
   creatureJoint(r,hip,knee,boar?10:8,shade(fur,.87f),.7f);
   creatureJoint(r,knee,foot,boar?5.5f:4.3f,shade(fur,.8f),.66f);
   r.rounded(foot+P3{0,2,-1},{boar?8.f:6.5f,10,4},boar?rgb(51,44,35):rgb(76,87,79));
   for(float toe:{-1.4f,1.4f})r.tapered(foot+P3{toe,5,0},foot+P3{toe,7,-.5f},.6f,.6f,boar?Ink:bone);
  }
  creatureSkin(r,{0,-2,bodyZ},{boar?29.f:22.f,boar?41.f:45.f,boar?26.f:22.f},fur,boar?4:1);
  creatureSkin(r,{0,front,bodyZ+4},{boar?27.f:26.f,21,boar?25.f:27.f},shade(fur,.94f),boar?4:1);
  float reach=hit*(boar?10:8),hz=bodyZ+(boar?7:15)-hit*(boar?6:3);
  P3 head{0,boar?22+reach:25+reach,hz};
  creatureSkin(r,head,{boar?24.f:19.f,boar?25.f:21.f,boar?23.f:21.f},fur,boar?4:1);
  P3 muzzle=head+P3{0,boar?15.f:14.f,boar?-7.f:-5.f};
  creatureSkin(r,muzzle,{boar?19.f:11.f,boar?12.f:19.f,boar?12.f:10.f},boar?rgb(163,113,83):rgb(181,187,165),boar?14:1);
  r.rounded(muzzle+P3{0,boar?6.f:9.f,1},{boar?16.f:8.f,3,boar?9.f:5.f},boar?rgb(181,126,96):rgb(40,50,47));
  for(float side:{-1.f,1.f}){
   creatureEye(r,head+P3{side*(boar?8:6.6f),8,3},boar?3.5f:3.4f,Gold);
   r.spike(head+P3{side*7,-5,8},head+P3{side*10,-7,boar?19.f:25.f},boar?11:9,7,shade(fur,.76f));
   r.spike(head+P3{side*7,-2,10},head+P3{side*9,-3,boar?16.f:21.f},5,2,boar?rgb(156,107,84):rgb(181,163,133));
   if(boar)r.tube({muzzle+P3{side*8,0,-4},muzzle+P3{side*12,3,0},muzzle+P3{side*12,7,7}},3.3f,3.3f,bone);
   else {r.spike(muzzle+P3{side*4,4,-2},muzzle+P3{side*4,6,-7-hit*3},2,2,bone);r.rounded(head+P3{side*9,0,-5},{5,10,9},shade(fur,.95f));}
  }
  // Fine dorsal tufts replace square bristles. They follow the back curve.
  for(int n=0;n<9;n++){
   float f=-20+n*4.5f,z=bodyZ+10+std::sin(n*PI/9)*4;
   r.spike({0,f,z},{0,f-3,z+(boar?7:3)},boar?4:3,4,shade(fur,boar?.55f:.8f));
  }
  float wag=motion.sway*walk*2+std::sin(t*2.2f)*(boar?1:2);
  if(boar)r.tube({{0,-23,bodyZ},{4,-29,bodyZ+2},{7+wag,-30,bodyZ+5},{5+wag,-27,bodyZ+6}},2,2,fur);
  else {r.tube({{0,-24,bodyZ+2},{wag,-34,bodyZ+7},{wag*1.4f,-45,bodyZ+4},{wag,-48,bodyZ}},6,7,shade(fur,.8f));r.rounded({wag,-45,bodyZ+3},{7,10,8},shade(fur,.75f));}
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
 }else if(type==7){
  Color bark=flash?Text:rgb(111,91,58),leaf=flash?Text:rgb(70,113,67);float z=bob;
  for(float side:{-1.f,1.f}){
   auto k=creature_motion::sample(5,s.phase+(side>0?.5f:0));
   P3 foot{side*12,k.travel*walk*treant_gait::Stride,4+k.lift*walk*treant_gait::Lift};
   auto bend=treant_gait::knee(foot.f,40+z,foot.z);P3 knee{side*10,bend.forward,bend.height};
   creatureJoint(r,{side*8,0,40+z},knee,13,bark,.85f);creatureJoint(r,knee,foot,12,bark,.8f);
   for(int root=0;root<3;root++)r.tapered(foot,foot+P3{side*(root-1)*7,10+root*2.f,-3},5,4,bark,.15f);
   P3 elbow{side*26,3,56+z},hand{side*31,6+hit*24-motion.sway*walk*side*4,41+z+hit*7};
   creatureJoint(r,{side*16,0,70+z},elbow,10,bark);creatureJoint(r,elbow,hand,8,bark);
   for(int n=0;n<3;n++)r.tapered(hand+P3{side*(n-1)*3,0,0},hand+P3{side*(n-1)*6,10,-8},3.3f,3,bark,.3f);
   creatureSkin(r,{side*20,-2,75+z},{26,23,18},leaf,12);
   r.tapered({side*8,0,94+z},{side*19,-2,112+z},6,6,bark,.3f);
  }
  creatureSkin(r,{0,0,57+z},{31,26,46},bark,7);creatureSkin(r,{0,1,88+z},{25,24,27},bark,7);
  for(float side:{-1.f,1.f})creatureEye(r,{side*6,12,91+z},5.3f,rgb(222,207,111));
  r.tube({{-6,12,82+z},{0,14,80+z},{6,12,82+z}},2,2,rgb(52,48,32));
  for(int i=0;i<7;i++){
   float f=-5+(i%3)*5.f,rp=-22+i*7.f,wind=std::sin(t*1.8f+i)*.8f;
   creatureSkin(r,{rp+wind,f,112+(i%2)*6.f+z},{22,23,16},shade(leaf,.94f+i*.035f),12);
  }
  for(int n=0;n<5;n++){float f=7+n%2*2.f;r.tube({{-9+n*4.f,f,39+z},{-11+n*4.f,f+3,53+z},{-8+n*4.f,f,72+z}},.9f,.9f,shade(bark,.63f));}
 }else{
  bool skeleton=type==2,golem=type==3;Color body=flash?Text:golem?rgb(104,134,123):skeleton?bone:rgb(121,82,58);
  float thick=golem?12:skeleton?3.8f:7,spacing=golem?10:7,lift=bob*(golem?.5f:1);
  Color cloth=flash?Text:rgb(119,62,48),leather=rgb(83,61,44),skin=flash?Text:rgb(204,166,119);
  for(float side:{-1.f,1.f}){
   auto k=creature_motion::sample(2,s.phase+(side>0?.5f:0));P3 foot{side*spacing,k.travel*walk*7,4+k.lift*walk*6},knee{side*spacing,3+k.travel*walk*3,17+lift};
   creatureJoint(r,{side*spacing,0,31+lift},knee,thick,skeleton?bone:golem?body:rgb(65,76,66));
   creatureJoint(r,knee,foot,thick*.85f,skeleton?bone:golem?body:rgb(57,68,62));
   r.rounded(foot+P3{0,3,-1},{thick+2,thick+8,6},skeleton?bone:golem?shade(body,.86f):leather);
   if(skeleton)r.tapered(knee+P3{side*2,0,0},foot+P3{side*2,0,0},1.1f,1.1f,bone,.8f);
  }
  creatureSkin(r,{0,0,30+lift},{golem?29.f:20.f,14,9},body,golem?3:skeleton?2:6);
  if(skeleton){
   for(int i=0;i<7;i++)r.rounded({0,-3,33+i*3.5f+lift},{4.6f,5,3},shade(bone,.92f));
   r.tapered({0,-3,54+lift},{0,0,61+lift},3.5f,3.5f,bone,.85f);
   for(float side:{-1.f,1.f})r.tapered({0,-2,54+lift},{side*16,0,53+lift},2.8f,2.8f,bone,.9f);
   for(int j=0;j<5;j++)for(float side:{-1.f,1.f}){
    float z=37+j*4.1f+lift,w=9-j*.5f;
    r.tube({{side*2,-3,z},{side*w,0,z+1},{side*(w+1),5,z},{side*6,8,z-1},{side*2,8,z}},2,2,bone);
   }
   r.tapered({0,8,38+lift},{0,8,54+lift},2.5f,2,bone);
  }else{
   creatureSkin(r,{0,0,45+lift},{golem?36.f:26.f,golem?24.f:18.f,29},body,golem?3:6);
   if(!golem){
    creatureSkin(r,{0,-8,40+lift},{27,6,34},cloth,6);
    r.tapered({-10,9,54+lift},{10,10,33+lift},3,2,leather);
    r.rounded({0,0,31+lift},{27,19,5},leather);r.box({0,10,31+lift},{5,2,4},Gold);
    for(float side:{-1.f,1.f})r.rounded({side*11,3,29+lift},{6,8,8},leather);
   }else{
    r.tube({{-4,12,48+lift},{0,14,53+lift},{4,12,48+lift},{0,14,41+lift},{-4,12,48+lift}},1.8f,1.8f,rgb(219,210,129));
    for(float side:{-1.f,1.f})creatureSkin(r,{side*12,-3,57+lift},{10,13,8},rgb(87,125,78),12);
   }
  }
  for(float side:{-1.f,1.f}){
   float reach=hit*(side>0?24:9),shoulder=side*(golem?23:16);
   P3 elbow{shoulder+side*2,1-motion.sway*walk*side*2+reach*.45f,43+lift+hit*3},hand{side*(golem?27:20),5-motion.sway*walk*side*4+reach,33+lift+hit*11};
   creatureJoint(r,{shoulder,0,53+lift},elbow,thick,body,.75f);creatureJoint(r,elbow,hand,thick*.85f,skeleton?bone:golem?body:skin,.72f);
   creatureSkin(r,{shoulder,0,54+lift},{golem?17.f:skeleton?6.f:12.f,13,10},body,golem?3:skeleton?2:6);
   r.rounded(hand,{golem?13.f:6.f,golem?12.f:7.f,golem?13.f:6.f},skeleton?bone:golem?body:skin);
   if(side>0&&!golem){
    P3 tip=hand+P3{hit*-9,3+hit*26,34-hit*30};
    r.tapered(hand,tip,3.5f,2,iron,.3f);r.tapered(hand+P3{-6,2,3},hand+P3{6,2,3},2,2,Gold);
   }
  }
  float hz=67+lift;r.rounded({0,0,hz},{golem?25.f:skeleton?18.f:18.f,golem?23.f:17.f,golem?23.f:20.f},golem?body:skeleton?bone:skin,golem?103:skeleton?102:-1);
  if(skeleton){
   r.rounded({0,4,hz-7},{14,10,6},bone);
   for(float side:{-1.f,1.f})r.rounded({side*4.6f,7,hz+1},{6.2f,4,6},rgb(48,59,50));
   r.rounded({0,9,hz-3},{2.7f,1.2f,3.5f},rgb(74,76,55));
   for(int i=0;i<5;i++)r.rounded({-5+i*2.5f,8.5f,hz-7},{1.7f,2,2.6f},shade(bone,1.1f));
  }else if(golem){for(float side:{-1.f,1.f}){
   r.rounded({side*5.5f,10.8f,hz+2},{8,3,5},rgb(48,70,63));
   r.rounded({side*5.5f,12.2f,hz+2},{6,2,2.3f},Gold);
   r.tapered({side*2,11,hz+6},{side*10,8,hz+8},4,3,body,.7f);
  }}
  else{
   // A fitted hood leaves the face exposed, with cloth folds at the temples.
   creatureSkin(r,{0,-2,hz+7},{22,21,10},cloth,15);creatureSkin(r,{0,-7,hz-1},{21,7,22},cloth,15);
   for(float side:{-1.f,1.f}){creatureSkin(r,{side*8,0,hz},{5,15,20},cloth,15);creatureEye(r,{side*4,7.8f,hz+2},2.3f,rgb(88,110,85));}
   r.rounded({0,8.5f,hz-2},{3,4,4},skin);r.tapered({-3,8,hz-6},{3,8,hz-6},.8f,.8f,leather);
  }
 }
}
