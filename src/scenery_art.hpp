#pragma once
#include "forest_bush_art.hpp"

// Static detail is baked once at startup; wind still deforms the finished
// crown texture and the pointer mask follows exactly the same deformation.
void leafTuft(float x,float y,float w,float h,Color c,unsigned seed){
 ellipse(x,y,w,h,shade(c,.77f));
 poly({{x-w,y},{x-w*.64f,y-h*.75f},{x-w*.07f,y-h},{x+w*.56f,y-h*.82f},{x+w,y-h*.18f},{x+w*.77f,y+h*.55f},{x+w*.2f,y+h*.85f},{x-w*.58f,y+h*.65f}},c);
 ellipse(x-w*.23f,y-h*.24f,w*.51f,h*.43f,shade(c,1.14f));
 for(int i=0;i<6;i++){unsigned k=Game::hash(int(seed)+i,27);float xx=x+((k%101)/100.f-.5f)*w*1.5f,yy=y+(((k>>8)%101)/100.f-.5f)*h*1.3f;
  line(xx-2,yy+1,xx+1,yy-2,shade(c,i%2?1.32f:.62f),1.2f);}
}
void tree(float x,float y,float s){

 poly({{x-9*s,y},{x-5*s,y-164*s},{x+4*s,y-164*s},{x+11*s,y+2*s}},rgb(73,58,40));
 line(x-2*s,y-5*s,x-1*s,y-160*s,rgb(128,92,53),3*s);
 for(int i=0;i<6;i++){float side=i%2?-1.f:1.f;line(x,y-(8+i*11)*s,x+side*(14-i)*s,y-(3+i*9)*s,rgb(79,67,42),3*s);}
 for(int j=0;j<5;j++){
  float yy=y-(42+j*27)*s,w=(62-j*10)*s;
  poly({{x,y-(88+j*25)*s},{x+w*.72f,yy-8*s},{x+w,yy+6*s},{x+w*.47f,yy+4*s},{x+w*.65f,yy+16*s},{x+3*s,yy+20*s},{x-w*.72f,yy+15*s},{x-w*.48f,yy+5*s},{x-w,yy+8*s},{x-w*.63f,yy-7*s}},rgb(29+j*3,67+j*5,51+j*3));
  for(int n=0;n<9;n++){float dx=(n-4)*w*.17f,dy=(std::abs(n-4)*2+(n%3)*3)*s;
   leafTuft(x+dx,yy-dy, (11+(n+j)%3*2)*s,(6+(n+j)%2*2)*s,rgb(42+j*5+n%3*5,87+j*6+n%2*8,59+j*4),j*91+n);
  }
  line(x-w*.52f,yy+7*s,x-4*s,yy-10*s,rgb(73+j*4,114+j*4,73+j*3),1.4f*s);
 }
 for(int n=0;n<4;n++)ellipse(x+(-15+n*11)*s,y+(n%2)*3*s,6*s,2*s,rgb(58,83,47));
}
void deciduousTree(float x,float y,float s,bool birch){
Color trunk=birch?rgb(186,192,166):rgb(102,77,49);
 poly({{x-10*s,y},{x-5*s,y-140*s},{x+5*s,y-153*s},{x+12*s,y}},shade(trunk,.78f));
 poly({{x-7*s,y},{x-4*s,y-143*s},{x+1*s,y-153*s},{x+3*s,y}},trunk);
 line(x-3*s,y-8*s,x-1*s,y-126*s,shade(trunk,1.25f),2*s);
 for(int n=0;n<9;n++){float yy=y-(12+n*14)*s;line(x-5*s,yy,x+(n%2?3:7)*s,yy-2*s,birch?rgb(69,76,61):rgb(64,49,35),2.2f*s);}
 for(float side:{-1.f,1.f}){
  line(x,y-74*s,x+side*38*s,y-133*s,trunk,5*s);line(x+side*21*s,y-108*s,x+side*50*s,y-138*s,trunk,3*s);
  line(x+side*3*s,y-3*s,x+side*24*s,y+3*s,shade(trunk,.75f),4*s);
 }
 Color leaf=birch?rgb(99,137,71):rgb(53,101,63);
 // Broad overlapping masses, followed by finer leaves, keep a solid canopy.
 for(int n=0;n<11;n++){float a=n*2.39996f,xx=x+std::cos(a)*(19+n%3*10)*s,yy=y-(119+n%4*15)*s;
  leafTuft(xx,yy,(birch?25:30)*s,(21+n%3*3)*s,shade(leaf,.78f+n*.035f),n+birch*97);
 }
 for(int n=0;n<28;n++){unsigned k=Game::hash(n,birch?49:53);float dx=((k%101)/100.f-.5f)*100,dy=(((k>>8)%101)/100.f-.5f)*64;
  if(dx*dx/2500+dy*dy/1024>1)continue;
  leafTuft(x+dx*s,y-(144-dy)*s,(7+n%3)*s,(4+n%2)*s,shade(leaf,.95f+(k%7)*.055f),k);
 }
 for(int i=0;i<5;i++)ellipse(x+(-18+i*9)*s,y+(i%2)*3*s,5*s,1.6f*s,rgb(91,97,53));
}
void rock(float x,float y,float s){

 poly({{x-36*s,y-8*s},{x-29*s,y-30*s},{x-10*s,y-47*s},{x+12*s,y-42*s},{x+31*s,y-23*s},{x+36*s,y-3*s},{x+13*s,y+9*s},{x-12*s,y+8*s}},rgb(65,80,76));
 poly({{x-36*s,y-8*s},{x-29*s,y-30*s},{x-10*s,y-47*s},{x+12*s,y-42*s},{x+6*s,y-20*s},{x-13*s,y-14*s}},rgb(112,126,110));
 poly({{x+6*s,y-20*s},{x+12*s,y-42*s},{x+31*s,y-23*s},{x+36*s,y-3*s},{x+10*s,y+5*s}},rgb(83,101,96));
 line(x-27*s,y-29*s,x-10*s,y-43*s,rgb(165,170,143),1.4f*s);
 line(x+8*s,y-36*s,x+3*s,y-20*s,rgb(52,66,61),1.7f*s);line(x+3*s,y-20*s,x+14*s,y-13*s,rgb(52,66,61),1.3f*s);
 for(int n=0;n<20;n++){unsigned h=Game::hash(n,38);float dx=(int(h%55)-27)*s,dy=(int((h>>8)%24)-21)*s;
  ellipse(x+dx,y+dy,(2+n%3)*s,1.2f*s,rgb(67+h%23,96+h%25,57+h%15));}
 for(int i=0;i<5;i++)line(x+(-22+i*9)*s,y-8*s,x+(-20+i*9)*s,y-4*s,rgb(130,144,113),s);
}
void bush(float x,float y,float s){
 if(forest_bush_art::draw(x,y,s))return;
 for(int n=0;n<12;n++){float dx=((n*13)%51-25)*s,dy=(n%4)*7*s;
  leafTuft(x+dx,y-10*s-dy,(10+n%3)*s,(8+n%2)*s,rgb(49+n%4*8,88+n%3*10,55+n%2*7),n*17);
 }
 for(int n=0;n<7;n++){float xx=x+(n*17%43-21)*s,yy=y-(n*11%28+6)*s;ellipse(xx,yy,2.4f*s,2.5f*s,rgb(186,92,72));ellipse(xx-s*.5f,yy-s*.7f,s*.7f,s*.7f,rgb(226,157,108));}
}
void column(float x,float y,float s){
diamond(x,y,35*s,17*s,rgb(83,99,85));
 poly({{x-22*s,y-8*s},{x-22*s,y-105*s},{x,y-94*s},{x,y+4*s}},rgb(96,111,101));
 poly({{x,y+4*s},{x,y-94*s},{x+22*s,y-105*s},{x+22*s,y-8*s}},rgb(62,81,77));
 diamond(x,y-105*s,27*s,13*s,rgb(162,167,139));diamond(x-2*s,y-105*s,16*s,7*s,rgb(130,143,116));
 for(int n=0;n<4;n++){float xx=x-(7+n*4)*s;line(xx,y-18*s,xx,y-(82+n*3)*s,rgb(129,141,121),2*s);}
 for(int n=0;n<3;n++)line(x+(5+n*5)*s,y-16*s,x+(5+n*5)*s,y-85*s,rgb(42,60,56),2*s);
 for(float yy:{-15.f,-86.f}){line(x-23*s,y+yy*s,x,y+(yy+11)*s,rgb(60,77,68),3*s);line(x,y+(yy+11)*s,x+23*s,y+yy*s,rgb(40,59,55),3*s);}
 line(x-20*s,y-66*s,x-7*s,y-57*s,rgb(44,62,55),1.6f*s);line(x-7*s,y-57*s,x-11*s,y-43*s,rgb(44,62,55),1.6f*s);
 for(int n=0;n<7;n++)leafTuft(x+(-18+n*5)*s,y-(n%3*9+3)*s,5*s,2.8f*s,rgb(65+n%2*12,105+n%3*6,62),n);
}
void mushrooms(float x,float y,float s){

 for(int n=0;n<6;n++){float xx=x+(n*17%43-21)*s,yy=y-(n%2)*8*s,r=(7+n%3*2)*s;
  line(xx,yy,xx,yy-(9+n%3*3)*s,rgb(157,150,108),4*s);line(xx-s,yy-s,xx-s,yy-(8+n%3*3)*s,rgb(231,218,166),1.5f*s);
  ellipse(xx,yy-(10+n%3*3)*s,r,5*s,rgb(109+n*7,51+n*3,39));ellipse(xx,yy-(12+n%3*3)*s,r*.9f,4*s,rgb(180+n*6,89+n*3,54));
  for(int k=0;k<3;k++)ellipse(xx+(k-1)*r*.45f,yy-(12+n%3*3)*s-(k%2)*s,1.4f*s,s,rgb(234,214,167));
 }
}
void detailedGroundTile(int x,int y,float sx,float sy){
 unsigned h=Game::hash(x,y);bool path=Game::onPath(x,y);
 ForestZone zone=forestZone({float(x),float(y)});
 bool glade=forestClearing({float(x),float(y)});
 Color c=path?rgb(88+h%8,99+h%9,78+h%8):zone==ForestZone::Heart?rgb(40+h%9,67+h%10,47+h%8):
  zone==ForestZone::Spiders?rgb(44+h%8,67+h%9,57+h%7):zone==ForestZone::Birches||zone==ForestZone::Grove?
  rgb(62+h%8,85+h%9,51+h%7):rgb(46+h%8,73+h%9,53+h%7);
 if(glade&&!path)c=shade(c,1.14f);
 diamond(sx,sy,40,20,c);
 // Sparse mottling replaces the conspicuous square grid on grass.
 for(int n=0;n<22;n++){unsigned k=Game::hash(x*29+n,y*31);float dx=int(k%71)-35,dy=int((k>>8)%33)-16;
  if(std::abs(dx)/40+std::abs(dy)/20>.9f)continue;
  if(path){
   float w=5+k%5,hh=2+(k>>6)%3;Color stone=shade(c,.92f+(k%7)*.05f);
   poly({{sx+dx-w,sy+dy},{sx+dx-w*.55f,sy+dy-hh},{sx+dx+w*.55f,sy+dy-hh*.9f},{sx+dx+w,sy+dy},{sx+dx+w*.4f,sy+dy+hh},{sx+dx-w*.7f,sy+dy+hh*.8f}},stone);
   line(sx+dx-w*.55f,sy+dy-hh,sx+dx+w*.5f,sy+dy-hh*.9f,shade(stone,1.17f),.7f);
  }else{
   ellipse(sx+dx,sy+dy,3+k%4,1+(k>>5)%2,shade(c,k%2?.87f:1.09f));
   Color grass=rgb(70+k%21,99+k%24,57+k%16);line(sx+dx,sy+dy,sx+dx-1,sy+dy-3-k%3,grass,.75f);
   if(k%3==0)line(sx+dx,sy+dy,sx+dx+2,sy+dy-3,shade(grass,.8f),.7f);
   if(k%(glade?7:29)==0){ellipse(sx+dx,sy+dy-4,1.5f,1,rgb(194,176,111));ellipse(sx+dx+2,sy+dy-3,1,1,rgb(211,193,130));}
  }
 }
 if(!path&&h%5==0){float xx=sx-9,yy=sy+3;ellipse(xx,yy,5,2,rgb(32,52,37));line(xx-5,yy,xx+7,yy-5,rgb(101,83,51),1.5f);line(xx+1,yy-2,xx+1,yy-6,rgb(101,83,51),1);}
 if(x==MapSize-2||y==MapSize-2){
  poly({{sx-40,sy},{sx,sy+20},{sx,sy+49},{sx-40,sy+29}},rgb(52,64,46));
  poly({{sx,sy+20},{sx+40,sy},{sx+40,sy+29},{sx,sy+49}},rgb(36,49,38));
  line(sx-36,sy+6,sx,sy+25,rgb(64,76,48),3);line(sx,sy+25,sx+36,sy+6,rgb(38,54,39),3);
  for(int n=0;n<8;n++){float dx=-31+n*9.f,yy=sy+27-std::abs(dx)*.48f;line(sx+dx,yy,sx+dx+2,yy+8+n%3,rgb(30,40,32),1.3f);}
 }
}
