#pragma once
namespace merchant_art {
// Counter and trader share physical geometry, lighting and a silhouette shadow.
inline actor_art::Rig make(float x,float y){
 actor_art::Rig m(x,y,1,Direction::S,true);m.setSmoothSurfaces();
 Color coat=rgb(63,91,85),leather=rgb(132,85,49),skin=rgb(206,155,115),hair=rgb(77,52,38),wood=rgb(117,77,49),brass=rgb(202,168,94);
 for(float side:{-1.f,1.f}){
  m.rounded({side*8,-20,6},{13,21,12},shade(leather,.7f));
  m.rounded({side*8,-20,23},{12,14,29},rgb(54,61,65));
 }
 m.rounded({0,-20,45},{37,24,30},coat);
 m.rounded({0,-20,65},{32,23,36},coat);
 m.rounded({0,-7.5f,56},{26,3,41},leather);
 m.box({0,-5.5f,48},{30,2,4},shade(leather,.66f));m.rounded({0,-3.5f,48},{6,2,6},brass);
 for(float side:{-1.f,1.f}){
  m.tapered({side*13,-18,74},{side*21,-5,57},6,5,coat);
  m.tapered({side*21,-5,57},{side*20,13,51},5,4,coat);
  m.rounded({side*20,14,51},{9,11,7},skin);
  m.rounded({side*12,-20,91},{4,8,9},skin);
 }
 m.rounded({0,-20,92},{24,22,26},skin);
 m.rounded({0,-20,102},{27,24,9},hair);
 m.rounded({0,-20,108},{29,27,8},rgb(66,88,72));
 m.rounded({0,-14,105},{35,31,3},rgb(80,101,77));
 m.rounded({0,-6,86},{18,7,10},hair);
 m.rounded({0,-4,94},{5,7,7},shade(skin,1.08f));
 for(float side:{-1.f,1.f}){
  m.rounded({side*5,-7.5f,97},{4,2,3},rgb(237,225,198));
  m.rounded({side*5,-6,97},{1.6f,1,2.5f},Ink);
  m.tapered({side*8,-6.5f,100},{side*3,-6.5f,101},.9f,.9f,hair);
  m.tapered({side*3,-5.5f,88},{side*8,-6.5f,88},1.8f,1.8f,shade(hair,1.25f));
 }
 m.rounded({12,-2,51},{11,8,14},shade(leather,.7f));
 m.rounded({12,2,53},{3,1,4},brass);
 // Oak counter: rounded edge, panel, iron rivets, ledger, potion bottles, coins.
 for(float side:{-1.f,1.f})m.box({side*28,18,22},{7,23,44},shade(wood,.74f));
 m.box({0,33,23},{66,4,38},wood);m.box({0,35.5f,9},{68,2,4},brass);
 for(float side:{-1.f,1.f})m.box({side*26,35.5f,23},{3,2,34},brass);
 m.rounded({0,16,46},{75,40,7},shade(wood,1.3f));
 for(int i=0;i<6;i++)m.beam({-32,0+i*6.f,50},{32,0+i*6.f,50},.6f,.6f,shade(wood,.72f));
 m.box({2,19,51},{19,20,3},rgb(68,52,49));m.box({2,19,53},{17,18,1},rgb(228,211,171));
 for(int i=0;i<5;i++)m.beam({-4,14+i*2.f,54},{8,14+i*2.f,54},.5f,.5f,rgb(107,92,65));
 for(int i=0;i<2;i++){
  float r=-23+i*10.f;Color liquid=i?rgb(72,146,194):rgb(190,74,89);
  m.rounded({r,17,56},{8,8,11},liquid);m.rounded({r,17,64},{4,4,6},rgb(170,211,212));
  m.rounded({r,17,68},{5,5,3},leather);m.beam({r-2,21,53},{r-2,21,60},.7f,.7f,Text);
 }
 for(int i=0;i<4;i++)m.rounded({23+i%2*5.f,20+i/2*5.f,51+i%2},{6,6,2},brass);
 m.box({-40,-19,20},{17,24,38},wood);m.box({-40,-19,41},{18,25,4},shade(wood,1.2f));
 m.box({-40,-6,24},{2,1,8},brass);
 return m;
}
}
