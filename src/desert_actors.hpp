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
inline void sandMound(Rig& rig,float sway,Color col){
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
 auto point=[&](int j,int i){P3 p=mesh[j][i].point;p.z+=sway*p.z/143.f;return p;};
 for(int j=0;j<bands;j++)for(int i=0;i<sides;i++){int n=(i+1)%sides;rig.surfaceQuad(point(j,i),point(j,n),point(j+1,n),point(j+1,i),colors[j][i],colors[j][n],colors[j+1][n],colors[j+1][i]);}
 // Fine sand ripples follow the same curved skin, just above its surface.
 for(int j:{6,10,14,18,22,26})for(int i=0;i<sides;i++){int n=(i+1)%sides;P3 p=point(j,i)+mesh[j][i].normal*.7f,q=point(j,n)+mesh[j][n].normal*.7f;Color c=shade(col,rig.surfaceLight((mesh[j][i].normal+mesh[j][n].normal)*.5f)*.88f);rig.face(p-P3{0,0,.28f},q-P3{0,0,.28f},q+P3{0,0,.28f},p+P3{0,0,.28f},c);}
 for(int i=0;i<16;i++){int j=8+i%5*4,k=(i*9)%sides;P3 p=point(j,k)+mesh[j][k].normal*.9f;rig.rounded(p,{2.4f,2.4f,1.7f},shade(col,i%2?1.08f:.91f));}
}
inline void buildDesert(Rig& rig,int type,const AnimationState& state,float attack,bool flash){
 rig.setSmoothSurfaces();
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
  std::vector<P3> coil;for(int i=0;i<=24;i++){float a=i*.26f+t*.18f;coil.push_back({std::cos(a)*(23-i*.4f),-9+std::sin(a)*(19-i*.35f),5+i*.35f});}rig.tube(coil,12,11,shade(scales,.9f));
  std::vector<P3> neck;P3 previous{};for(int i=0;i<=12;i++){float z=15+i*4.5f;P3 q{std::sin(t*1.5f+i*.05f)*2,4+std::sin(cycle)*walk*3+hit*i*1.5f,z};neck.push_back(q);previous=q;}rig.tube(neck,17,13,scales);
  for(int i=0;i<12;i++){P3 q=neck[i];rig.rounded(q+P3{0,6.1f,0},{11,2.7f,5.3f},belly);}
  P3 head=previous+P3{0,7+hit*9,10};rig.rounded(head,{22,24,16},scales);
  for(float side:{-1.f,1.f}){rig.rounded(previous+P3{side*10,-2,-7},{14,12,34},scales);rig.form(previous+P3{side*12,5,-8},{5,2,10},dark);rig.form(head+P3{side*6,8,3},{5,3,4},amber);rig.form(head+P3{side*6,10,3},{1.5f,1,3.5f},Ink);rig.spike(head+P3{side*5,11,-4},head+P3{side*5,14,-10},2,2,Text);}
  rig.beam(head+P3{0,12,-3},head+P3{0,22+hit*9,-6},1,1,rgb(166,57,63));
 }else{
  // A mobile dune with flowing flanks and sand tendrils, rather than a stone humanoid.
  rig.reserve(1500);float sway=std::sin(t*1.5f)*3+std::sin(cycle)*walk*4;
  sandMound(rig,sway,sand);

  for(int i=0;i<6;i++){float side=i%2?1.f:-1.f,f=-29+i/2*25.f;float step=std::sin(cycle+i*1.3f)*walk;P3 knee{side*(43+step*5),f,14},foot{side*(59+step*7),f+step*10,3};rig.form(knee,{30,25,25},sand);rig.tapered(knee,foot,21,18,dark,.35f);}
  for(float side:{-1.f,1.f}){P3 shoulder{side*42,6,49+sway},hand{side*(64-hit*29),27+hit*48,16+hit*15};rig.tapered(shoulder,hand,28,23,sand,.35f);for(int i=0;i<4;i++)rig.form(hand+P3{(i-1.5f)*7,10+i*3.f,-4+i%2*3.f},{6,8,4},shade(sand,.83f+i*.08f));}
  for(float side:{-1.f,1.f}){
   P3 eye=sandFront(-13+side*13,83+sway+hit*5,sway,1.5f);rig.rounded(eye,{14,4,10},dark);rig.rounded(eye+P3{0,2.6f,0},{9,3,5},rgb(255,223,118));rig.rounded(eye+P3{side,4.2f,1},{4,2,2},Text);
   rig.beam(sandFront(-13+side*6,91+sway+hit*5,sway,1),sandFront(-13+side*21,94+sway+hit*5,sway,1),4,4,sand);
  }
  rig.rounded(sandFront(-13,67+sway+hit*5,sway,1),{31,3,6},dark);
  for(int i=0;i<7;i++){P3 tooth=sandFront(-25+i*4.f,70+sway+hit*5,sway,3);rig.spike(tooth,tooth+P3{0,1,-6},1.8f,1.8f,amber);}

 }
}
inline void buildDesertCorpse(Rig& rig,int type){
 rig.setSmoothSurfaces();rig.reserve(2000);Color sand=rgb(205,159,94),shell=rgb(157,103,53),copper=rgb(180,128,72);
 if(type==11){
  rig.rounded({0,0,9},{115,93,19},sand);rig.rounded({-11,-12,12},{74,56,20},shade(sand,1.07f));
  for(int row=0;row<6;row++){
   std::vector<P3> ripple;float f=-29+row*10.f;
   for(int n=0;n<=24;n++){float r=-42+n*3.5f,ff=f+std::sin(r*.085f)*2;
    float a=9+9.5f*std::sqrt(std::max(0.f,1-r*r/(57.5f*57.5f)-ff*ff/(46.5f*46.5f)));
    float b=12+10*std::sqrt(std::max(0.f,1-(r+11)*(r+11)/(37*37)-(ff+12)*(ff+12)/(28*28)));
    ripple.push_back({r,ff,std::max(a,b)+.7f});}
   rig.tube(ripple,.8f,.7f,shade(sand,.88f));
  }
  rig.rounded({7,10,16},{14,13,7},rgb(155,106,56));rig.tapered({2,6,20},{11,15,19},1.1f,1.1f,rgb(246,202,117));
  for(int i=0;i<12;i++){float a=i*2.39996f;rig.rounded({std::cos(a)*57,std::sin(a)*42,2},{7+i%3*2.f,9,3.5f},shade(sand,.87f+i*.008f));}
 }else if(type==8){
  rig.rounded({0,-4,6},{29,38,13},shell);rig.rounded({0,19,5},{24,20,11},shade(shell,.85f));
  for(int i=0;i<4;i++)for(float side:{-1.f,1.f}){float f=-18+i*10.f;rig.tube({{side*9,f,7},{side*26,f-7,4},{side*21,f+2,2}},2.8f,2.8f,shade(shell,.75f));}
  for(float side:{-1.f,1.f}){rig.tube({{side*9,21,5},{side*22,29,5}},5,5,shell);rig.rounded({side*23,32,5},{14,18,10},shell);rig.spike({side*21,38,5},{side*16,46,3},4,4,shell);rig.spike({side*27,38,5},{side*23,44,3},4,4,shade(shell,.8f));}
  std::vector<P3> tail{{0,-23,7},{7,-32,6},{17,-38,5},{23,-31,3},{18,-21,2}};rig.tube(tail,6,6,shell);
  for(int n=0;n<int(tail.size());n++)rig.rounded(tail[n],{8-n*.5f,8-n*.5f,7-n*.4f},shade(shell,1.05f));
  rig.spike(tail.back(),{10,-15,2},4,3,rgb(85,57,37));
  for(int n=0;n<4;n++)rig.tapered({-9,-17+n*8.f,12},{9,-17+n*8.f,12},.8f,.8f,copper);
 }else if(type==9){
  Color teal=rgb(67,115,109);rig.rounded({0,-3,8},{37,44,17},teal);rig.rounded({3,24,5},{25,21,10},copper);
  rig.tapered({0,-23,13},{0,17,15},1.5f,1.5f,rgb(222,175,98));
  for(int i=0;i<5;i++)for(float side:{-1.f,1.f})rig.tapered({side*2,-17+i*7.f,16},{side*14,-20+i*7.f,12},.8f,.8f,copper);
  for(int i=0;i<3;i++)for(float side:{-1.f,1.f}){float f=-17+i*14.f;rig.tube({{side*11,f,6},{side*25,f-7,4},{side*18,f+2,2}},2.7f,2.7f,shade(copper,.8f));}
  for(float side:{-1.f,1.f})rig.tapered({3+side*6,30,8},{3+side*9,28,8},.8f,.8f,rgb(58,56,36));
 }else{
  Color scale=rgb(192,149,75),belly=rgb(223,192,117);std::vector<P3> body;
  for(int i=0;i<=24;i++)body.push_back({std::sin(i*.28f)*23,-30+i*2.5f,4+std::sin(i*.4f)*.8f});
  rig.tube(body,11,10,scale);
  for(int i=1;i<24;i+=2)rig.rounded(body[i]+P3{0,0,4.6f},{3.5f,4,1.1f},shade(scale,i%4?1.11f:.82f));
  rig.rounded({-3,34,5},{26,22,8},scale);rig.rounded({-4,40,5},{20,22,12},scale);
  rig.rounded({-4,46,3},{15,11,4},belly);
  for(float side:{-1.f,1.f})rig.tapered({-4+side*5,46,9},{-4+side*8,44,8},.8f,.8f,rgb(59,49,29));
 }
}
