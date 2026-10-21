#pragma once
namespace actor_art {
// Fallen figures keep their anatomy, palette and equipment, with relaxed,
// asymmetric limbs that rest on the terrain.
inline void buildFallenForest(Rig& rig,int type){
 rig.setSmoothSurfaces();rig.reserve(2400);
 Color bone=rgb(199,191,160),fur=rgb(129,144,135),darkFur=rgb(83,99,92),ivory=rgb(227,217,181);
 if(type==0){
  Color gel=rgb(92,143,89);rig.rounded({0,0,4.5f},{53,42,9},gel);
  rig.rounded({-5,-4,7},{34,27,10},shade(gel,1.12f));
  for(int i=0;i<7;i++)rig.rounded({-19+i*6.f,std::sin(i*1.7f)*15,2},{12,12,4},shade(gel,.93f));
  for(float side:{-1.f,1.f})rig.tapered({side*7-2,14,8},{side*7+2,14,8},1,1,rgb(41,65,42));
  rig.rounded({-10,-5,11},{12,6,1.8f},rgb(165,199,137));rig.rounded({8,3,9},{4,3,1.5f},rgb(153,192,126));
 }else if(type==1){
  rig.rounded({0,-3,9},{24,42,18},fur);rig.rounded({2,11,7},{23,25,13},rgb(159,170,151));
  rig.rounded({5,25,9},{20,22,17},fur);rig.rounded({10,35,6},{13,17,10},rgb(174,184,158));
  rig.rounded({10,42,7},{8,4,5},rgb(41,53,47));
  rig.spike({-2,22,15},{-4,19,25},8,6,darkFur);rig.spike({12,24,12},{17,21,19},7,6,darkFur);
  rig.tapered({10,30,13},{15,29,12},.9f,.9f,rgb(40,51,45));
  for(float side:{-1.f,1.f})for(float f:{-14.f,13.f}){
   rig.tube({{side*8,f,7},{side*16,f-7,4},{side*19,f+1,3}},5,5,darkFur);
   rig.rounded({side*19,f+3,3},{8,11,5},fur);
   for(int n=0;n<3;n++)rig.tapered({side*19-2+n*2.f,f+5,5},{side*19-2+n*2.f,f+8,4},.5f,.5f,darkFur);
  }
  rig.tube({{-2,-20,8},{-10,-31,5},{-20,-30,3},{-23,-22,2}},8,7,darkFur);
  for(int i=0;i<10;i++)rig.tapered({-5,-17+i*3.f,16},{-9,-19+i*3.f,12},.65f,.65f,shade(fur,i%2?.75f:1.17f));
 }else if(type==2){
  rig.tapered({0,-18,5},{0,18,5},4,4,bone);
  for(int i=0;i<6;i++){float f=-8+i*4.f,w=10-std::abs(i-2.5f);
   rig.tube({{-w,f,5},{-w+1,f,10},{0,f,14},{w-1,f,10},{w,f,5}},2.5f,2.5f,bone);
  }
  rig.rounded({0,-18,5},{16,11,7},bone);
  rig.rounded({3,29,7},{17,18,14},bone);rig.rounded({3,35,3},{13,9,5},shade(bone,.84f));
  for(float side:{-1.f,1.f})rig.rounded({3+side*4,34,10},{4.5f,3,4},rgb(48,52,44));
  rig.rounded({3,36,7},{2.5f,2,3},rgb(76,76,61));
  for(int n=0;n<5;n++)rig.rounded({-2+n*2.5f,39,4},{1.4f,1.5f,3},ivory);
  for(float side:{-1.f,1.f}){
   rig.tapered({side*9,14,7},{side*21,2,3},3.6f,3.6f,bone);rig.rounded({side*21,2,3},{5,5,5},bone);
   rig.tapered({side*21,2,3},{side*20,-12,2},3,3,bone);
   for(int n=0;n<3;n++)rig.tapered({side*20+n-1,-12,2},{side*20+n*2-2,-18,1.8f},.9f,.9f,bone);
   rig.tapered({side*5,-19,4},{side*12,-33,3},4.3f,4.3f,bone);rig.rounded({side*12,-33,3},{6,5,5},bone);
   rig.tapered({side*12,-33,3},{side*(side<0?7:17),-48+(side<0?4:0),2},3.7f,3.7f,bone);
   rig.rounded({side*(side<0?7:17),-46,2},{6,10,4},bone);
  }
  rig.tapered({-28,-14,2},{-23,16,2},3.5f,2,rgb(143,159,153));rig.tapered({-29,-7,3},{-20,-8,3},2,2,rgb(143,116,69));
 }else if(type==3){
  Color stone=rgb(103,127,116),moss=rgb(80,109,67),crack=rgb(46,65,60);
  rig.rounded({0,-6,14},{55,44,28},stone);rig.rounded({3,13,11},{46,26,22},shade(stone,.83f));
  rig.rounded({8,32,10},{26,24,21},shade(stone,1.09f));
  for(float side:{-1.f,1.f}){
   rig.rounded({side*29,9,8},{23,25,17},stone);rig.tube({{side*30,6,9},{side*40,side<0?-6.f:22.f,4}},15,14,shade(stone,.85f));
   rig.rounded({side*42,side<0?-14.f:26.f,4},{17,17,9},stone);
   rig.rounded({side*15,-29,7},{22,25,15},shade(stone,.9f));rig.rounded({side*17,-45,4},{23,18,9},shade(stone,.78f));
  }
  auto surface=[](float r,float f){return 14+14*std::sqrt(std::max(0.f,1-r*r/(27.5f*27.5f)-(f+6)*(f+6)/(22*22)));};
  const P3 crackPath[]={{-15,-17,0},{-6,-7,0},{5,-9,0},{13,4,0}};
  for(int j=0;j<3;j++){std::vector<P3> curve;for(int i=0;i<=8;i++){P3 point=crackPath[j]+(crackPath[j+1]-crackPath[j])*(i/8.f);point.z=surface(point.r,point.f)+.8f;curve.push_back(point);}rig.tube(curve,1.3f,.8f,crack);}
  for(float side:{-1.f,1.f})rig.tapered({8+side*5,34,21},{8+side*7,36,20},1.3f,1.1f,rgb(60,109,108));
  for(int n=0;n<5;n++){float r=-18+n*7.f,f=-12+(n%2)*12.f;rig.rounded({r,f,surface(r,f)+.5f},{7,7,2},shade(moss,1+n*.03f));}
  for(int n=0;n<5;n++)rig.rounded({-45+n*22.f,-22+(n%2)*12.f,2},{7+n%3*2.f,7,4},shade(stone,.85f));
 }else if(type==4){
  Color hide=rgb(121,88,61),hoof=rgb(55,46,35);
  rig.rounded({0,-4,10},{34,46,21},hide);rig.rounded({-3,24,8},{27,25,18},shade(hide,.92f));
  rig.rounded({0,38,6},{19,15,11},rgb(152,111,79));
  for(float side:{-1.f,1.f}){
   rig.rounded({side*5,43,8},{3,2.5f,2.5f},hoof);
   rig.tube({{side*6,39,7},{side*11,43,9},{side*16,39,13}},2.5f,2.5f,ivory);
   rig.spike({side*7-2,23,14},{side*12,17,19},9,6,shade(hide,.72f));
   for(float f:{-14.f,12.f}){rig.tube({{side*10,f,7},{side*20,f-8,4},{side*23,f-2,3}},6,6,hide);rig.rounded({side*23,f+1,3},{8,9,6},hoof);}
  }
  for(int i=0;i<8;i++)rig.tapered({-3,-19+i*4.f,19},{-3,-21+i*4.f,23},1.3f,1.3f,shade(hide,.56f));
  rig.tapered({4,32,14},{9,31,13},.8f,.8f,hoof);
 }else if(type==5){
  Color shell=rgb(62,78,67),venom=rgb(109,135,71);
  rig.rounded({0,-11,9},{27,31,18},shell);rig.rounded({1,14,5},{21,21,11},shade(shell,1.08f));
  for(int i=0;i<4;i++)for(float side:{-1.f,1.f}){
   float f=-19+i*10.f;rig.tube({{side*10,f,8},{side*24,f-6,5},{side*20,f+3,3},{side*12,f+6,2}},3,3,shade(shell,.78f));
   rig.rounded({side*24,f-6,5},{4,4,4},shade(shell,1.15f));
  }
  rig.rounded({0,-12,17},{8,19,2.8f},venom);
  for(int n=0;n<3;n++)rig.tapered({-8,-20+n*7.f,15},{7,-20+n*7.f,15},.8f,.8f,shade(shell,1.27f));
  for(float side:{-1.f,1.f})rig.spike({side*5,24,5},{side*8,28,2},2.3f,2.3f,ivory);
 }else if(type==6){
  Color cloth=rgb(121,65,48),skin=rgb(199,159,118),leather=rgb(88,64,43);
  rig.rounded({0,1,6},{27,36,13},cloth);rig.rounded({3,28,6},{18,20,13},skin);
  rig.rounded({1,21,9},{23,21,13},cloth);rig.rounded({5,34,8},{11,8,7},skin);
  rig.tapered({6,37,11},{11,35,10},.8f,.8f,rgb(82,59,44));
  rig.rounded({0,-13,7},{25,13,8},leather);rig.rounded({0,-6,12},{5,4,2},rgb(181,142,77));
  for(float side:{-1.f,1.f}){
   rig.tube({{side*10,13,6},{side*21,2,4}},7,7,cloth);rig.tapered({side*21,2,4},{side*21,-12,2},5,5,skin);rig.rounded({side*21,-14,2},{7,7,4},skin);
   rig.tube({{side*6,-15,5},{side*13,-29,4},{side*10,-41,2}},7,7,rgb(59,73,61));rig.rounded({side*10,-44,3},{9,13,6},leather);
  }
  for(int n=0;n<4;n++)rig.tapered({-8+n*5.f,-12,10},{-10+n*5.f,14,11},.8f,.8f,shade(cloth,.75f));
  rig.tapered({-27,1,2},{-28,22,2},3,2,rgb(174,191,181));rig.rounded({-27,-2,2},{4,7,4},leather);
 }else{
  Color bark=rgb(103,83,56),leaf=rgb(73,101,60);
  rig.tube({{0,-22,10},{0,5,13},{2,27,10}},25,22,bark);
  rig.rounded({3,31,10},{26,23,20},shade(bark,1.07f));
  for(float side:{-1.f,1.f}){
   rig.tube({{side*11,10,9},{side*30,16,5},{side*40,8,2}},8,8,shade(bark,.86f));
   rig.tube({{side*7,-22,7},{side*18,-36,3},{side*26,-37,2}},9,9,bark);
   rig.rounded({side*30,16,7},{21,21,11},leaf);
   for(int n=0;n<3;n++)rig.tapered({side*40,8,2},{side*(42+n*3.f),2+n*5.f,1},2.5f,2.5f,bark);
  }
  for(int i=0;i<7;i++)rig.tapered({-8,-17+i*6.f,20},{-5,-14+i*6.f,23},1.1f,1.1f,shade(bark,.55f));
  rig.rounded({-7,39,14},{26,24,12},leaf);rig.rounded({14,34,12},{25,23,13},shade(leaf,.92f));
  rig.tapered({2,37,16},{9,37,16},1.1f,1.1f,rgb(69,61,39));
 }
}
}
