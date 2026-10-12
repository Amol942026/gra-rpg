#pragma once
// Included in actor_art after Rig; every part turns with the same eight-direction rig.
inline void buildDesert(Rig& rig,int type,const AnimationState& state,float attack,bool flash){
 float cycle=state.phase*2*PI,t=state.idleTime,walk=state.moving?1.f:0.f,hit=std::max(0.f,attack);
 Color sand=flash?Text:rgb(219,175,108),dark=flash?Text:rgb(144,93,54),amber=rgb(246,202,105);
 if(type==8){
  Color shell=flash?Text:rgb(177,113,55);rig.form({0,-8,15},{25,31,17},shell);rig.form({0,13,13},{23,20,14},shade(shell,.8f));
  for(int i=0;i<4;i++)for(float side:{-1.f,1.f}){float f=-17+i*10.f,step=std::sin(cycle+i*PI*.5f+(side<0?PI:0))*walk;P3 a{side*9,f,13},b{side*(24+step*2),f-4+step*4,8},c{side*(32+step*2),f-12+step*4,2+std::max(0.f,step)*4};rig.beam(a,b,3.4f,3.4f,shade(shell,.8f));rig.beam(b,c,2.8f,2.8f,shade(shell,.67f));}
  for(float side:{-1.f,1.f}){P3 elbow{side*24,24+hit*7,13},hand{side*25,38+hit*13,16};rig.beam({side*10,17,12},elbow,5,5,shell);rig.beam(elbow,hand,6,6,shell);rig.form(hand,{13,18,12},shade(shell,1.14f));rig.spike(hand+P3{-side*4,4,0},hand+P3{-side*6,16,1},5,5,shell);rig.spike(hand+P3{side*4,4,0},hand+P3{side*5,14,1},5,5,shade(shell,.75f));}
  P3 previous{0,-22,15};for(int i=0;i<8;i++){float a=i*.34f;P3 q{std::sin(t*1.8f)*i*.45f,-25-std::sin(a)*25+hit*i*5,18+i*8.f-std::max(0,i-4)*9.f};rig.form(q,{10-i*.65f,10-i*.5f,10-i*.35f},shade(shell,1.f+i*.025f));rig.beam(previous,q,5,5,shell);previous=q;}rig.spike(previous,previous+P3{0,14,-12},6,5,dark);
  for(float side:{-1.f,1.f})rig.form({side*5,21,19},{4,3,3},Ink);
  for(int i=0;i<4;i++)rig.beam({-9,-19+i*7.f,23},{9,-19+i*7.f,23},1,1,rgb(232,177,86));
 }else if(type==9){
  Color shell=flash?Text:rgb(68,116,114),copper=flash?Text:rgb(184,125,68);
  for(int i=0;i<3;i++)for(float side:{-1.f,1.f}){float f=-14+i*13.f,step=std::sin(cycle+i*2+(side<0?PI:0))*walk;P3 knee{side*25,f+step*6,10},foot{side*33,f-7+step*6,2+std::max(0.f,step)*5};rig.beam({side*11,f,17},knee,3.4f,3.4f,copper);rig.beam(knee,foot,2.5f,2.5f,copper);}
  rig.rounded({0,-2,24},{35,40,30},shell);rig.rounded({0,20,19},{25,19,18},copper);rig.beam({0,-20,30},{0,16,34},1.6f,1.6f,amber);
  for(int i=0;i<6;i++)for(float side:{-1.f,1.f})rig.beam({side*3,-16+i*6.f,37-i%2},{side*14,-19+i*6.f,30},1.2f,1.2f,shade(copper,1.13f));
  for(float side:{-1.f,1.f}){rig.form({side*8,27,23},{5,3,5},amber);rig.spike({side*6,27,14},{side*11,38+hit*15,21},5,5,copper);rig.beam({side*9,22,25},{side*15,34,31},1.5f,1.5f,dark);}
 }else if(type==10){
  Color scales=flash?Text:rgb(192,145,69),belly=flash?Text:rgb(239,214,145);
  for(int i=0;i<12;i++){float a=i*.52f+t*.18f;rig.form({std::cos(a)*(23-i*.8f),-9+std::sin(a)*(19-i*.7f),5+i*.7f},{12,13,11},shade(scales,.82f+i*.015f));}
  P3 previous{0,0,12};for(int i=0;i<7;i++){float z=15+i*9.f;P3 q{std::sin(t*1.5f+i*.1f)*2,4+std::sin(cycle)*walk*3+hit*i*3,z};rig.form(q,{14+i*.9f,13,13},scales);rig.form(q+P3{0,6,0},{10,3,10},belly);if(i)rig.beam(previous,q,10,10,scales);previous=q;}
  P3 head=previous+P3{0,7+hit*9,10};rig.rounded(head,{22,24,16},scales);
  for(float side:{-1.f,1.f}){rig.rounded(previous+P3{side*10,-2,-7},{14,12,34},scales);rig.form(previous+P3{side*12,5,-8},{5,2,10},dark);rig.form(head+P3{side*6,8,3},{5,3,4},amber);rig.form(head+P3{side*6,10,3},{1.5f,1,3.5f},Ink);rig.spike(head+P3{side*5,11,-4},head+P3{side*5,14,-10},2,2,Text);}
  rig.beam(head+P3{0,12,-3},head+P3{0,22+hit*9,-6},1,1,rgb(166,57,63));
 }else{
  // A mobile dune with flowing flanks and sand tendrils, rather than a stone humanoid.
  rig.reserve(1500);float sway=std::sin(t*1.5f)*3+std::sin(cycle)*walk*4;
  rig.rounded({0,-6,27},{137,111,53},shade(sand,.9f));rig.rounded({-7,-10,61+sway},{119,93,99},sand);rig.rounded({-29,-19,99+sway},{76,63,54},shade(sand,1.08f));rig.rounded({35,-4,43},{65,69,61},shade(sand,.96f));
  rig.spike({-39,-27,111+sway},{-53,-38,145+sway},25,19,shade(sand,1.11f));
  for(int ring=0;ring<5;ring++)for(int i=0;i<14;i++){float a=i*2*PI/14,radius=51-ring*7.f,z=13+ring*18.f+sway*ring*.12f;P3 p{std::cos(a)*radius,std::sin(a)*(radius*.8f)-5,z},q{std::cos(a+.28f)*radius,std::sin(a+.28f)*(radius*.8f)-5,z+2};rig.beam(p,q,1.1f,1.1f,ring%2?rgb(245,204,138):rgb(185,136,76));}
  for(int i=0;i<6;i++){float side=i%2?1.f:-1.f,f=-29+i/2*25.f;float step=std::sin(cycle+i*1.3f)*walk;P3 knee{side*(43+step*5),f,14},foot{side*(59+step*7),f+step*10,3};rig.form(knee,{30,25,25},sand);rig.tapered(knee,foot,21,18,dark,.35f);}
  for(float side:{-1.f,1.f}){P3 shoulder{side*42,6,49+sway},hand{side*(64-hit*29),27+hit*48,16+hit*15};rig.tapered(shoulder,hand,28,23,sand,.35f);for(int i=0;i<4;i++)rig.form(hand+P3{(i-1.5f)*7,10+i*3.f,-4+i%2*3.f},{6,8,4},shade(sand,.83f+i*.08f));}
  P3 face{0,38,78+sway+hit*5};for(float side:{-1.f,1.f}){rig.form(face+P3{side*13,0,5},{14,4,10},dark);rig.form(face+P3{side*13,3,5},{9,3,5},rgb(255,223,118));rig.form(face+P3{side*14,5,6},{4,2,2},Text);rig.beam(face+P3{side*6,2,13},face+P3{side*21,-1,16},5,5,sand);}
  rig.form(face+P3{0,5,-11},{31,3,6},dark);for(int i=0;i<7;i++)rig.spike(face+P3{-12+i*4.f,7,-8},face+P3{-12+i*4.f,8,-15},2,2,amber);
  for(int i=0;i<10;i++){float a=i*2.39996f+std::sin(t*.8f)*.06f;rig.form({std::cos(a)*42,std::sin(a)*35-8,37+i%4*18.f},{5,6,3},i%2?shade(sand,1.14f):dark);}
 }
}
inline void buildDesertCorpse(Rig& rig,int type){
 Color sand=rgb(197,150,85),shell=rgb(132,94,57);
 if(type==11){rig.blob(56,24,sand);for(int i=0;i<9;i++){float a=i*2.39996f;rig.form({std::cos(a)*55,std::sin(a)*39,4},{14,15,7},shade(sand,.86f+i*.015f));}rig.form({0,8,23},{10,8,4},rgb(232,187,104));}
 else if(type==8){rig.form({0,-3,7},{29,38,13},shell);for(int i=0;i<4;i++)for(float side:{-1.f,1.f})rig.beam({side*8,-17+i*10.f,7},{side*29,-23+i*10.f,2},3,3,shell);rig.form({5,-31,5},{10,25,9},shell);for(float side:{-1.f,1.f})rig.form({side*23,27,5},{13,18,8},shell);}
 else if(type==9){rig.rounded({0,0,8},{36,43,16},rgb(63,108,105));for(int i=0;i<3;i++)for(float side:{-1.f,1.f})rig.beam({side*10,-16+i*14.f,7},{side*31,-22+i*14.f,2},3,3,shell);rig.form({0,24,5},{24,20,9},shell);}
 else{for(int i=0;i<13;i++){float a=i*.49f;rig.form({std::sin(a)*20,-22+i*4.f,5},{12,13,8},sand);}rig.form({0,32,4},{22,21,7},sand);}
}