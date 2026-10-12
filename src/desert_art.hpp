#pragma once
// Native geometry: sand, waxy cactus ribs, weathered stone and oasis fronds.
namespace desert_art {
RenderTexture2D terrain{};Texture2D minimap{};
void cactus(float x,float y,bool pads){
 softGroundShadow(x,y,38,13);
 auto stem=[&](float xx,float yy,float w,float h){
  ellipse(xx,yy-h+w*.5f,w*.5f,w*.57f,rgb(108,155,108));rect(xx-w*.5f,yy-h+w*.5f,w,h-w*.5f,rgb(60,116,92));ellipse(xx,yy,w*.5f,w*.22f,rgb(47,88,67));
  rect(xx-w*.34f,yy-h+w*.6f,w*.29f,h-w*.6f,rgb(93,147,101));line(xx-w*.18f,yy-4,xx-w*.18f,yy-h+w*.65f,rgb(156,183,123),1.2f);line(xx+w*.23f,yy-4,xx+w*.23f,yy-h+w*.55f,rgb(35,86,69),1.2f);
  for(int row=0;row<int(h/11);row++)for(int col=0;col<3;col++){float a=xx+(col-1)*w*.27f,b=yy-7-row*11;ellipse(a,b,.85f,.8f,rgb(196,198,144));line(a,b,a+(col-1)*2.6f,b-2.1f,rgb(223,220,161),.7f);}
 };
 auto flower=[](float xx,float yy){for(int i=0;i<6;i++){float a=i*PI/3;ellipse(xx+std::cos(a)*3,yy+std::sin(a)*2.5f,2.7f,2,rgb(207,97,112));}ellipse(xx,yy,2,1.4f,rgb(255,216,124));};
 if(!pads){stem(x,y,24,135);stem(x-31,y-58,16,48);stem(x+30,y-73,17,40);poly({{x-36,y-56},{x-5,y-43},{x-3,y-57},{x-29,y-67}},rgb(67,123,95));poly({{x+2,y-64},{x+35,y-69},{x+36,y-81},{x+2,y-79}},rgb(75,131,98));stem(x-29,y-60,16,52);stem(x+30,y-77,16,43);flower(x,y-138);}
 else{
  const Vector2 centers[]={{0,-27},{-18,-59},{19,-58},{-31,-88},{29,-87},{0,-93}};
  for(int i=0;i<6;i++){float xx=x+centers[i].x,yy=y+centers[i].y;ellipse(xx,yy,16,24,rgb(58,106,76));ellipse(xx-3,yy-2,13,22,rgb(103,147,96));ellipse(xx-8,yy-8,4,11,rgb(142,173,111));
   for(int row=-2;row<=2;row++)for(int col=-1;col<=1;col++){float px=xx+col*8,py=yy+row*7;ellipse(px,py,.8f,.8f,rgb(225,216,151));line(px,py,px-1.6f,py-1.8f,rgb(232,220,170),.6f);}
   if(i>2)flower(xx,yy-24);
  }
 }
 for(int i=0;i<6;i++)ellipse(x-25+i*10,y+(i%2)*3,4,1.5f,rgb(155,125,78));
}
void sandstone(float x,float y){
 softGroundShadow(x,y+2,53,17);
 const Color c[]={rgb(181,112,67),rgb(211,144,87),rgb(229,170,107)};
 for(int layer=0;layer<3;layer++){
  float yy=y-layer*18,w=43-layer*8;
  poly({{x-w,yy-7},{x-w*.68f,yy-26},{x-4,yy-37},{x+w*.7f,yy-28},{x+w,yy-9},{x+16,yy+7},{x-w*.55f,yy+4}},shade(c[layer],.88f));
  poly({{x-w,yy-7},{x-w*.68f,yy-26},{x-4,yy-37},{x+w*.7f,yy-28},{x+9,yy-10},{x-12,yy-3}},shade(c[layer],1.17f));
  poly({{x+9,yy-10},{x+w*.7f,yy-28},{x+w,yy-9},{x+16,yy+7}},shade(c[layer],.69f));
  for(int n=0;n<4;n++){float a=yy-6-n*4;line(x-w*.72f,a,x+5,a+8,shade(c[layer],.73f),1);line(x+7,a+8,x+w*.75f,a-3,shade(c[layer],.62f),.8f);}
  line(x-w*.63f,yy-25,x-3,yy-35,rgb(245,197,136),1.3f);
 }
 for(int i=0;i<10;i++){unsigned h=Game::hash(i,78);float xx=x+int(h%65)-32,yy=y-5-int((h>>8)%28);line(xx,yy,xx+3,yy+1,rgb(133,85,53),.8f);}
}
void obelisk(float x,float y){
 softGroundShadow(x,y,33,12);diamond(x,y,34,17,rgb(154,99,63));
 poly({{x-18,y-5},{x-14,y-118},{x,y-139},{x,y+4}},rgb(225,175,111));poly({{x,y+4},{x,y-139},{x+15,y-116},{x+20,y-6}},rgb(160,108,66));
 poly({{x-14,y-118},{x,y-139},{x+15,y-116},{x,y-109}},rgb(244,210,146));
 for(int n=0;n<6;n++){line(x-16,y-17-n*17,x,y-9-n*17,rgb(152,108,68),.8f);line(x,y-9-n*17,x+18,y-17-n*17,rgb(109,75,51),.8f);}
 for(int n=0;n<4;n++){float yy=y-32-n*16;line(x-10,yy,x-3,yy-8,rgb(107,72,48),1.4f);line(x-3,yy-8,x-4,yy+2,rgb(107,72,48),1.4f);}
 line(x+7,y-64,x+12,y-76,rgb(98,73,50),1.5f);line(x+12,y-76,x+6,y-87,rgb(98,73,50),1);
}
void shrub(float x,float y){
 softGroundShadow(x,y,29,9);for(int i=0;i<9;i++){float a=i*2.39996f,xx=x+std::cos(a)*(13+i%3*6),yy=y-25-i%4*5;line(x,y-2,xx,yy,rgb(118,96,64),2);line(xx,yy,xx+5,yy-9,rgb(183,150,91),1);line(xx,yy,xx-6,yy-6,rgb(167,128,77),1);}
 for(int i=0;i<6;i++)ellipse(x-17+i*7,y-10-i%2*9,5,2,rgb(195,166,100));
}
void palm(float x,float y){
 softGroundShadow(x,y,46,15);poly({{x-8,y},{x+2,y-159},{x+10,y-160},{x+9,y}},rgb(132,92,50));line(x-3,y-2,x+6,y-154,rgb(205,159,88),3);
 for(int i=0;i<14;i++){float yy=y-7-i*10;line(x-6+i*.5f,yy,x+9,yy-3,rgb(83,68,45),1.7f);line(x-5+i*.45f,yy-2,x+8,yy-5,rgb(225,179,101),1);}
 for(int i=0;i<10;i++){
  float a=i*2*PI/10,dx=std::cos(a)*56,dy=std::sin(a)*23-9;
  Vector2 last{x+6,y-157};for(int j=1;j<=9;j++){float t=j/9.f;Vector2 q{x+6+dx*t,y-157+dy*t-std::sin(t*PI)*19+t*t*17};line(last.x,last.y,q.x,q.y,rgb(109,153,75),1.7f);
   float w=std::sin(t*PI)*(10+i%3*2);poly({{q.x,q.y},{q.x-dx*.10f-w,q.y+8},{last.x,last.y},{q.x-dx*.09f+w,q.y+2}},rgb(49+i%3*8,109+i%3*11,63));line(q.x,q.y,q.x-dx*.1f-w,q.y+8,rgb(128,165,88),.8f);last=q;
  }
 }
 for(int i=0;i<7;i++)ellipse(x+4+(i%3)*3,y-143+(i/3)*5,3,4,rgb(150,83,45));
}
void portal(float x,float y){
 softGroundShadow(x,y,64,22);diamond(x,y,64,18,rgb(162,113,74));ellipse(x,y-83,39,63,rgb(65,49,87));ellipse(x-4,y-83,32,58,rgb(112,88,146));for(int i=0;i<8;i++)ellipse(x-8,y-89,26-i*2.3f,48-i*3.5f,rgb(164,130,205,18));
 for(int side:{-1,1})for(int row=0;row<7;row++){float xx=x+side*44,yy=y-row*19;poly({{xx-10,yy},{xx-10,yy-18},{xx+9,yy-22},{xx+13,yy-3}},side<0?rgb(229,184,126):rgb(163,116,79));line(xx-9,yy-18,xx+8,yy-21,rgb(246,208,152),1);}
 for(int i=0;i<9;i++){float a=PI+i*PI/9,b=PI+(i+1)*PI/9;poly({{x+std::cos(a)*53,y-127+std::sin(a)*49},{x+std::cos(b)*53,y-127+std::sin(b)*49},{x+std::cos(b)*35,y-127+std::sin(b)*33},{x+std::cos(a)*35,y-127+std::sin(a)*33}},rgb(220+i%2*12,173+i%2*10,113));}
 for(int i=0;i<5;i++){float yy=y-25-i*21;diamond(x-44,yy,3,5,rgb(154,119,182));}
}
void prop(int type,float x,float y){switch(type){case 7:cactus(x,y,false);break;case 8:cactus(x,y,true);break;case 9:sandstone(x,y);break;case 10:obelisk(x,y);break;case 11:shrub(x,y);break;case 12:palm(x,y);break;case 13:portal(x,y);break;}}
void groundTile(int x,int y,float sx,float sy){
 V pos{float(x),float(y)};float wave=std::sin(x*.17f+y*.095f)+.38f*std::sin(x*.11f-y*.25f);bool path=desertPath(pos);
 Color c=rgb(211+wave*7,170+wave*6,112+wave*4);diamond(sx,sy,40,20,c);
 for(int n=0;n<17;n++){unsigned k=Game::hash(x*31+n,y*37+53);float dx=int(k%73)-36,dy=int((k>>8)%35)-17;if(std::abs(dx)/40+std::abs(dy)/20>.86f)continue;
  line(sx+dx,sy+dy,sx+dx+3+k%5,sy+dy-1,shade(c,k%3?.91f:1.11f),.65f);
  if(path&&k%3==0){ellipse(sx+dx,sy+dy,2.3f,1.2f,shade(c,.72f));line(sx+dx-1,sy+dy-1,sx+dx+1,sy+dy-1,rgb(225,198,151),.8f);}
 }
}
void make(){
 terrain=LoadRenderTexture(TerrainWidth,TerrainHeight);BeginTextureMode(terrain);ClearBackground(BLANK);
 for(int sum=0;sum<MapSize*2;sum++)for(int x=1;x<MapSize-1;x++){int y=sum-x;if(y<1||y>=MapSize-1)continue;groundTile(x,y,TerrainOriginX+(x-y)*40,90+(x+y)*20);}
 // Feathered world-space ribbons avoid staircase-shaped tile edges on trails.
 auto project=[](V p){return Vector2{TerrainOriginX+(p.x-p.y)*40,90+(p.x+p.y)*20};};
 auto trail=[&](V a,V b){V d=b-a;float n=length(d);V normal{-d.y/n,d.x/n};
  for(float w:{1.75f,1.5f,1.25f,1.f})poly({project(a+normal*w),project(b+normal*w),project(b-normal*w),project(a-normal*w)},rgb(149,109,68,24));
  for(int i=0;i<int(n*8);i++){unsigned h=Game::hash(i,int(a.x*13));V p=a+d*(i/(n*8))+normal*((int(h%181)-90)*.01f);Vector2 q=project(p);ellipse(q.x,q.y,1.2f+h%2,.7f,rgb(130,101,66,125));}
 };
 const V road[]={{9,19},{21,20},{31,30},{44,33},{54,45},{57,55}};for(int i=0;i<5;i++)trail(road[i],road[i+1]);trail({21,20},{17,43});trail({31,30},{43,15});
 // Large smooth ridges are baked across tile boundaries; no square sand grid.
 for(int i=0;i<210;i++){unsigned h=Game::hash(i,443);V pos{4.f+h%64,4.f+(h>>8)%64};if(desertPath(pos)||distance(pos,DesertOasis)<6||distance(pos,DesertBossHome)<7)continue;
  float x=TerrainOriginX+(pos.x-pos.y)*40,y=90+(pos.x+pos.y)*20,w=38+h%54,hh=8+(h>>15)%13;
  poly({{x-w,y},{x-w*.4f,y-hh*.8f},{x+w*.15f,y-hh},{x+w,y+2},{x+w*.2f,y+hh*.7f}},rgb(208,159,96));
  poly({{x-w,y},{x-w*.4f,y-hh*.8f},{x+w*.15f,y-hh},{x+w,y+2},{x+w*.1f,y-3}},rgb(238,195,134));
  for(int ridge=0;ridge<3;ridge++)line(x-w*.52f,y+ridge*3,x+w*.35f,y+ridge*3+4,rgb(191,144,87),.6f);
 }
 float x=TerrainOriginX+(DesertOasis.x-DesertOasis.y)*40,y=90+(DesertOasis.x+DesertOasis.y)*20;
 for(int layer=0;layer<15;layer++)ellipse(x,y,220+(14-layer)*6,110+(14-layer)*3,rgb(136,154,91,10));ellipse(x,y,153,78,rgb(151,166,108));ellipse(x,y,140,70,rgb(209,192,128));ellipse(x,y,137,68,rgb(91,141,132));ellipse(x,y,126,61,rgb(41,112,132));ellipse(x-9,y-7,103,45,rgb(51,141,157));ellipseOutline(x,y,135,66,rgb(196,214,173));
 for(int i=0;i<28;i++){unsigned h=Game::hash(i,711);float a=i*2*PI/28,xx=x+std::cos(a)*144,yy=y+std::sin(a)*73;ellipse(xx,yy,3+h%4,2+h%2,rgb(127,149,83));line(xx,yy,xx+2,yy-7-h%6,rgb(103,129,63),1);}
 x=TerrainOriginX+(DesertBossHome.x-DesertBossHome.y)*40;y=90+(DesertBossHome.x+DesertBossHome.y)*20;
 ellipse(x,y,213,105,rgb(189,136,79));ellipse(x-8,y-6,194,91,rgb(220,174,108));for(int i=0;i<4;i++)ellipseOutline(x,y,173-i*9,82-i*4,rgb(200-i*3,145-i*2,81));
 for(int i=0;i<12;i++){float a=i*PI/6;diamond(x+std::cos(a)*192,y+std::sin(a)*94,5,3,rgb(148,104,66));}
 EndTextureMode();SetTextureFilter(terrain.texture,TEXTURE_FILTER_BILINEAR);
 Image mini=GenImageColor(180,112,BLANK);for(int y=0;y<112;y++)for(int x=0;x<180;x++){float delta=(x-90)*(MapSize-2)/78.f,sum=(y-6)*(MapSize-2)/48.f+2;V p{(sum+delta)*.5f,(sum-delta)*.5f};if(p.x>=1&&p.y>=1&&p.x<MapSize-1&&p.y<MapSize-1)ImageDrawPixel(&mini,x,y,desertWater(p)?rgb(64,168,181):desertPath(p)?rgb(136,100,64):rgb(202,160,101));}minimap=LoadTextureFromImage(mini);UnloadImage(mini);
}
void unload(){if(terrain.id)UnloadRenderTexture(terrain);if(minimap.id)UnloadTexture(minimap);terrain={};minimap={};}
void atmosphere(){
 for(int i=0;i<38;i++){float cycle=game.time*.095f+i*.071f,phase=cycle-std::floor(cycle),a=phase*phase*(3-2*phase);float x=std::fmod(i*181.f+a*180,WorldW+200.f)-100,y=120+std::fmod(i*79.3f,550.f);float alpha=std::sin(phase*PI);line(x,y,x+12+i%3*5,y-4,Fade(rgb(252,219,160),alpha*.24f),.7f);}
 Vector2 p=screen(DesertOasis);for(int i=0;i<5;i++){float age=std::fmod(game.time*.13f+i*.2f,1.f);ellipseOutline(p.x,p.y,24+age*89,12+age*40,Fade(rgb(175,223,218),(1-age)*.32f));}
 p=screen(DesertPortal);if(p.x>-90&&p.x<WorldW+90){ellipseOutline(p.x,p.y-82,27+std::sin(game.time*1.4f)*2,49,Fade(rgb(194,158,244),.55f));for(int i=0;i<8;i++){float age=std::fmod(game.time*.19f+i*.125f,1.f);ellipse(p.x+std::sin(i*2.4f+game.time)*24,p.y-13-age*140,1.5f,2,Fade(rgb(231,203,250),(1-age)*.8f));}}
}
}