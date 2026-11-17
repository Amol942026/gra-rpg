#pragma once
#include "wind_motion.hpp"
// Native geometry: sand, waxy cactus ribs, weathered stone and oasis fronds.
namespace desert_art {
RenderTexture2D terrain{};Texture2D minimap{};
inline const auto& ovalCircle(){static const auto circle=[](){std::array<Vector2,128> points{};for(int i=0;i<128;i++)points[i]={std::cos(i*2*PI/128),std::sin(i*2*PI/128)};return points;}();return circle;}
void duneFootDust(const DuneDust& dust){
 const Vector2 origin=screen(dust.center());
 if(origin.x<-70||origin.x>WorldW+70||origin.y<-70||origin.y>H+70)return;
 constexpr Color sand[]={{248,219,155,255},{143,91,52,255},{225,166,98,255},{255,230,174,255}};
 const float age=dust.age(),opening=1-(1-age)*(1-age);
 const float lift=4*age*(1-age);
 const Vector2 radial{(dust.drift.x-dust.drift.y)*40,(dust.drift.x+dust.drift.y)*20};
 const Vector2 tangent{(dust.drift.x+dust.drift.y)*40,(dust.drift.y-dust.drift.x)*20};
 const float pixel=1/renderScale;
 rlSetTexture(rlGetTextureIdDefault());
 rlCheckRenderBatchLimit(int(dust.grains)*4);
 rlBegin(RL_QUADS);
 rlTexCoord2f(0,0);
 for(unsigned i=0;i<dust.grains;i++){
  const unsigned seed=Game::hash(int(dust.seed&0x7FFFFFFFu),int(i));
  const float distance=(float(seed&127u)/127-.5f)*(.09f+1.22f*opening);
  const float spread=(float((seed>>7)&127u)/127-.5f)*(.19f+1.22f*opening);
  const float height=lift*(2+((seed>>14)&63u)*.24f);
  const float x=std::floor((origin.x+radial.x*distance+tangent.x*spread)*renderScale)*pixel;
  const float y=std::floor((origin.y+radial.y*distance+tangent.y*spread-height)*renderScale)*pixel;
  const float alpha=(1-age)*(.38f+((seed>>20)&31u)*.018f);
  const Color tint=Fade(sand[i%4],alpha);
  rlColor4ub(tint.r,tint.g,tint.b,tint.a);
  // Pixel-aligned specks keep their scale and do not blink at fractional positions.
  rlVertex2f(x,y);
  rlVertex2f(x,y+pixel);
  rlVertex2f(x+pixel,y+pixel);
  rlVertex2f(x+pixel,y);
 }
 rlEnd();
 rlSetTexture(0);
}
// The wide oasis exposes the corners of raylib's default ellipse. Use cached
// fine curves for shores, ripples and the organic silhouettes in this realm.
void oval(float x,float y,float rx,float ry,Color col){
 const auto& circle=ovalCircle();int stride=std::max(rx,ry)>35?1:2;
 for(int i=0;i<128;i+=stride){auto a=circle[i],b=circle[(i+stride)%128];DrawTriangle({x,y},{x+b.x*rx,y+b.y*ry},{x+a.x*rx,y+a.y*ry},col);}
}
void ovalOutline(float x,float y,float rx,float ry,Color col){
 const auto& circle=ovalCircle();int stride=std::max(rx,ry)>35?1:2;
 for(int i=0;i<128;i+=stride){auto a=circle[i],b=circle[(i+stride)%128];line(x+a.x*rx,y+a.y*ry,x+b.x*rx,y+b.y*ry,col,.8f);}
}

void curve(Vector2 a,Vector2 b,Vector2 c,Vector2 d,Color color,float width=1){
 DrawSplineSegmentBezierCubic(a,b,c,d,width,color);
 oval(a.x,a.y,width*.5f,width*.5f,color);oval(d.x,d.y,width*.5f,width*.5f,color);
}
// Closed Catmull-Rom contours and interpolated light are baked only at startup.
void softShape(std::initializer_list<Vector2> controls,Color color){
 std::vector<Vector2> points(controls);Vector2 center{};for(auto p:points){center.x+=p.x/points.size();center.y+=p.y/points.size();}
 auto sample=[&](int i,float t){int count=int(points.size());auto a=points[(i+count-1)%count],b=points[i],c=points[(i+1)%count],d=points[(i+2)%count];
  auto value=[&](float a,float b,float c,float d){return .5f*((2*b)+(-a+c)*t+(2*a-5*b+4*c-d)*t*t+(-a+3*b-3*c+d)*t*t*t);};return Vector2{value(a.x,b.x,c.x,d.x),value(a.y,b.y,c.y,d.y)};};
 float w=1,h=1;for(auto p:points){w=std::max(w,std::abs(p.x-center.x));h=std::max(h,std::abs(p.y-center.y));}
 auto light=[&](Vector2 p){return shade(color,.98f-.15f*(p.x-center.x)/w-.17f*(p.y-center.y)/h);};
 rlSetTexture(rlGetTextureIdDefault());
 for(int i=0;i<int(points.size());i++)for(int n=0;n<8;n++){
  Vector2 a=sample(i,n/8.f),b=sample(i,(n+1)/8.f);if((a.x-center.x)*(b.y-center.y)-(a.y-center.y)*(b.x-center.x)>0)std::swap(a,b);
  rlCheckRenderBatchLimit(3);rlBegin(RL_TRIANGLES);for(auto v:{center,a,b}){Color c=light(v);rlColor4ub(c.r,c.g,c.b,c.a);rlTexCoord2f(0,0);rlVertex2f(v.x,v.y);}rlEnd();
 }rlSetTexture(0);
}

void cactus(float x,float y,bool pads){

 auto stem=[&](float xx,float yy,float w,float h){
  softShape({{xx-w*.5f,yy-4},{xx-w*.5f,yy-h+w*.55f},{xx-w*.32f,yy-h},{xx+w*.3f,yy-h},{xx+w*.5f,yy-h+w*.55f},{xx+w*.5f,yy-4},{xx,yy+3}},rgb(74,126,94));
  for(int rib=0;rib<5;rib++){float dx=(rib-2)*w*.19f;curve({xx+dx,yy-5},{xx+dx,yy-h*.45f},{xx+dx,yy-h+w*.2f},{xx+dx*.6f,yy-h+w*.14f},rib%2?rgb(114,161,110):rgb(42,99,78),rib%2?1.3f:.9f);}
  for(int row=0;row<int((h-7)/11);row++)for(int col=0;col<3;col++){float a=xx+(col-1)*w*.27f,b=yy-7-row*11;oval(a,b,.85f,.8f,rgb(196,198,144));line(a,b,a+(col-1)*2.6f,b-2.1f,rgb(223,220,161),.7f);}
 };
 auto flower=[](float xx,float yy){for(int i=0;i<6;i++){float a=i*PI/3;oval(xx+std::cos(a)*3,yy+std::sin(a)*2.5f,2.7f,2,rgb(207,97,112));}oval(xx,yy,2,1.4f,rgb(255,216,124));};
 if(!pads){
  // Curved elbows connect the upright arms without polygonal joints.
  for(float side:{-1.f,1.f}){float yy=y-(side<0?55:72);curve({x+side*5,yy+6},{x+side*31,yy+12},{x+side*32,yy-5},{x+side*31,yy-20},rgb(61,116,89),17);curve({x+side*5-2,yy+2},{x+side*26,yy+6},{x+side*28,yy-6},{x+side*29,yy-20},rgb(105,153,104),5);stem(x+side*31,yy-13,17,43);}
  stem(x,y,24,135);flower(x,y-138);
 }
 else{
  const Vector2 centers[]={{0,-27},{-18,-59},{19,-58},{-31,-88},{29,-87},{0,-93}};
  for(int i=0;i<6;i++){float xx=x+centers[i].x,yy=y+centers[i].y;oval(xx,yy,16,24,rgb(58,106,76));oval(xx-3,yy-2,13,22,rgb(103,147,96));oval(xx-8,yy-8,4,11,rgb(142,173,111));
   for(int row=-2;row<=2;row++)for(int col=-1;col<=1;col++){float px=xx+col*8,py=yy+row*7;oval(px,py,.8f,.8f,rgb(225,216,151));line(px,py,px-1.6f,py-1.8f,rgb(232,220,170),.6f);}
   if(i>2)flower(xx,yy-24);
  }
 }
 for(int i=0;i<6;i++)oval(x-25+i*10,y+(i%2)*3,4,1.5f,rgb(155,125,78));
}
void sandstone(float x,float y){

 const Color c[]={rgb(181,112,67),rgb(211,144,87),rgb(229,170,107)};
 for(int layer=0;layer<3;layer++){
  float yy=y-layer*18,w=43-layer*8;
  softShape({{x-w,yy-7},{x-w*.82f,yy-23},{x-12,yy-35},{x+w*.52f,yy-30},{x+w*.94f,yy-15},{x+w*.9f,yy-3},{x+14,yy+7},{x-w*.57f,yy+4}},c[layer]);
  for(int n=0;n<5;n++){float a=yy-5-n*4;curve({x-w*.82f,a-4},{x-w*.3f,a+5},{x+w*.28f,a+5},{x+w*.84f,a-2},shade(c[layer],n%2?.76f:1.09f),n%2?1.f:.7f);}
  curve({x-w*.7f,yy-24},{x-w*.34f,yy-34},{x+5,yy-34},{x+w*.51f,yy-29},rgb(245,197,136),1.1f);
 }
 for(int i=0;i<10;i++){unsigned h=Game::hash(i,78);float xx=x+int(h%65)-32,yy=y-5-int((h>>8)%28);line(xx,yy,xx+3,yy+1,rgb(133,85,53),.8f);}
}
void obelisk(float x,float y){
diamond(x,y,34,17,rgb(154,99,63));
 poly({{x-17,y-5},{x-15,y-115},{x-12,y-120},{x-2,y-137},{x,y-137},{x,y+3}},rgb(225,175,111));poly({{x,y+3},{x,y-137},{x+2,y-137},{x+13,y-119},{x+15,y-115},{x+19,y-7},{x+17,y-4}},rgb(160,108,66));
 poly({{x-12,y-120},{x-2,y-137},{x+2,y-137},{x+13,y-119},{x,y-109}},rgb(244,210,146));
 line(x,y-108,x,y+1,rgb(238,192,127),1.6f);
 for(int n=0;n<6;n++){line(x-16,y-17-n*17,x,y-9-n*17,rgb(152,108,68),.8f);line(x,y-9-n*17,x+18,y-17-n*17,rgb(109,75,51),.8f);}
 for(int n=0;n<4;n++){float yy=y-32-n*16;line(x-10,yy,x-3,yy-8,rgb(107,72,48),1.4f);line(x-3,yy-8,x-4,yy+2,rgb(107,72,48),1.4f);}
 line(x+7,y-64,x+12,y-76,rgb(98,73,50),1.5f);line(x+12,y-76,x+6,y-87,rgb(98,73,50),1);
}
void shrub(float x,float y){
for(int i=0;i<9;i++){float a=i*2.39996f,xx=x+std::cos(a)*(13+i%3*6),yy=y-25-i%4*5;curve({x,y-2},{x+std::cos(a)*5,y-15},{xx-3,yy+7},{xx,yy},rgb(118,96,64),1.8f);curve({xx,yy},{xx+2,yy-2},{xx+5,yy-5},{xx+5,yy-9},rgb(183,150,91),.9f);curve({xx,yy},{xx-2,yy-3},{xx-5,yy-4},{xx-6,yy-6},rgb(167,128,77),.9f);}
 for(int i=0;i<6;i++)oval(x-17+i*7,y-10-i%2*9,5,2,rgb(195,166,100));
}
void palm(float x,float y){

 curve({x,y-2},{x-6,y-56},{x+10,y-117},{x+6,y-159},rgb(132,92,50),16);
 curve({x-3,y-2},{x-8,y-56},{x+6,y-117},{x+3,y-158},rgb(205,159,88),4);
 for(int i=0;i<14;i++){float yy=y-7-i*10,xx=x+std::sin(i*.14f)*3;curve({xx-7,yy-2},{xx-3,yy+1},{xx+3,yy-1},{xx+7,yy-4},rgb(83,68,45),1.5f);curve({xx-6,yy-4},{xx-2,yy-1},{xx+2,yy-3},{xx+6,yy-6},rgb(225,179,101),.8f);}
 for(int i=0;i<10;i++){
  float a=i*2*PI/10,dx=std::cos(a)*56,dy=std::sin(a)*23-9;
  auto point=[&](float t){return Vector2{x+6+dx*t,y-157+dy*t-std::sin(t*PI)*19+t*t*17};};
  Vector2 root=point(0),tip=point(1);curve(root,{root.x+dx*.33f,root.y-24+dy*.33f},{root.x+dx*.66f,root.y-24+dy*.66f},tip,rgb(109,153,75),1.7f);
  for(int j=1;j<10;j++){float t=j/10.f;Vector2 q=point(t);float w=std::sin(t*PI)*(10+i%3*2);
   for(float side:{-1.f,1.f}){Vector2 end{q.x-dx*.10f+side*w,q.y+7};curve(q,{q.x+side*w*.5f,q.y+1},{end.x,end.y-4},end,rgb(49+i%3*8,109+i%3*11,63),2.7f*(1-t)+.6f);curve(q,{q.x+side*w*.4f,q.y},{end.x,end.y-4},end,rgb(128,165,88),.6f);}
  }
 }
 for(int i=0;i<7;i++)oval(x+4+(i%3)*3,y-143+(i/3)*5,3,4,rgb(150,83,45));
}
void portal(float x,float y){
diamond(x,y,64,18,rgb(162,113,74));oval(x,y-83,39,63,rgb(65,49,87));oval(x-4,y-83,32,58,rgb(112,88,146));for(int i=0;i<8;i++)oval(x-8,y-89,26-i*2.3f,48-i*3.5f,rgb(164,130,205,18));
 for(int side:{-1,1})for(int row=0;row<7;row++){float xx=x+side*44,yy=y-row*19;poly({{xx-10,yy},{xx-10,yy-18},{xx+9,yy-22},{xx+13,yy-3}},side<0?rgb(229,184,126):rgb(163,116,79));line(xx-9,yy-18,xx+8,yy-21,rgb(246,208,152),1);}
 for(int i=0;i<54;i++){float a=PI+i*PI/54,b=PI+(i+1)*PI/54;poly({{x+std::cos(a)*53,y-127+std::sin(a)*49},{x+std::cos(b)*53,y-127+std::sin(b)*49},{x+std::cos(b)*35,y-127+std::sin(b)*33},{x+std::cos(a)*35,y-127+std::sin(a)*33}},rgb(220+(i/6)%2*12,173+(i/6)%2*10,113));}
 for(int i=1;i<9;i++){float a=PI+i*PI/9;line(x+std::cos(a)*36,y-127+std::sin(a)*34,x+std::cos(a)*52,y-127+std::sin(a)*48,rgb(157,112,72),.7f);}
 for(int i=0;i<5;i++){float yy=y-25-i*21;diamond(x-44,yy,3,5,rgb(154,119,182));}
}
void prop(int type,float x,float y){switch(type){case 7:cactus(x,y,false);break;case 8:cactus(x,y,true);break;case 9:sandstone(x,y);break;case 10:obelisk(x,y);break;case 11:shrub(x,y);break;case 12:palm(x,y);break;case 13:portal(x,y);break;}}
#include "desert_scenery.hpp"
#include "desert_roads.hpp"
#include "oasis_water.hpp"

void groundTile(int x,int y,float sx,float sy){
 V pos{float(x),float(y)};float wave=std::sin(x*.17f+y*.095f)+.38f*std::sin(x*.11f-y*.25f);bool path=desertPath(pos);
 Color c=rgb(211+wave*7,170+wave*6,112+wave*4);diamond(sx,sy,40,20,c);
 for(int n=0;n<17;n++){unsigned k=Game::hash(x*31+n,y*37+53);float dx=int(k%73)-36,dy=int((k>>8)%35)-17;if(std::abs(dx)/40+std::abs(dy)/20>.86f)continue;
  line(sx+dx,sy+dy,sx+dx+3+k%5,sy+dy-1,shade(c,k%3?.91f:1.11f),.65f);
  if(path&&k%3==0){oval(sx+dx,sy+dy,2.3f,1.2f,shade(c,.72f));line(sx+dx-1,sy+dy-1,sx+dx+1,sy+dy-1,rgb(225,198,151),.8f);}
 }
}
void make(){
 makeScenery();
 oasis_water::init();
 terrain=render_targets::colorOnly(TerrainCacheWidth,TerrainCacheHeight);BeginTextureMode(terrain);ClearBackground(BLANK);BeginMode2D({{0,0},{0,0},0,TerrainBakeScale});
 const bool painted=terrain_paint::bake(false);
 if(!painted)for(int sum=0;sum<MapSize*2;sum++)for(int x=1;x<MapSize-1;x++){int y=sum-x;if(y<1||y>=MapSize-1)continue;groundTile(x,y,TerrainOriginX+(x-y)*40,90+(x+y)*20);}
 drawRoads(!painted);
 // Large smooth ridges are baked across tile boundaries; no square sand grid.
 if(!painted)for(int i=0;i<420*MapSize*MapSize/(PreviousMapSize*PreviousMapSize);i++){unsigned h=Game::hash(i,443);V pos{4.f+h%(MapSize-8),4.f+(h>>8)%(MapSize-8)};if(desertPath(pos)||distance(pos,DesertOasis)<6||distance(pos,DesertBossHome)<7)continue;
  float x=TerrainOriginX+(pos.x-pos.y)*40,y=90+(pos.x+pos.y)*20,w=38+h%54,hh=8+(h>>15)%13;
   softShape({{x-w,y},{x-w*.48f,y-hh*.7f},{x+w*.12f,y-hh},{x+w*.74f,y-hh*.3f},{x+w,y+2},{x+w*.2f,y+hh*.7f},{x-w*.52f,y+hh*.4f}},rgb(220,173,109));
   curve({x-w*.94f,y},{x-w*.25f,y-hh*1.23f},{x+w*.22f,y-hh*1.27f},{x+w*.91f,y+1},rgb(244,203,143),1.1f);
   for(int ridge=0;ridge<3;ridge++)curve({x-w*.55f,y+ridge*2},{x-w*.18f,y+ridge*2-3},{x+w*.18f,y+ridge*2},{x+w*.52f,y+ridge*2+2},rgb(195,148,90),.6f);
 }
 float x=TerrainOriginX+(DesertOasis.x-DesertOasis.y)*40,y=90+(DesertOasis.x+DesertOasis.y)*20;
 beginTransparentLayer();
 for(int layer=0;layer<12;layer++)oval(x,y,158+(11-layer)*5,78+(11-layer)*2.5f,rgb(137,121,76,14));
 oval(x,y,143,72,rgb(121,116,77,182));
 if(!oasis_water::ready())oval(x,y,137,68,rgb(46,99,102));
 native_world::endBlendMode();
 x=TerrainOriginX+(DesertBossHome.x-DesertBossHome.y)*40;y=90+(DesertBossHome.x+DesertBossHome.y)*20;
 oval(x,y,213,105,rgb(189,136,79));oval(x-8,y-6,194,91,rgb(220,174,108));for(int i=0;i<4;i++)ovalOutline(x,y,173-i*9,82-i*4,rgb(200-i*3,145-i*2,81));
 for(int i=0;i<12;i++){float a=i*PI/6;diamond(x+std::cos(a)*192,y+std::sin(a)*94,5,3,rgb(148,104,66));}
 EndMode2D();EndTextureMode();SetTextureFilter(terrain.texture,TEXTURE_FILTER_BILINEAR);
 terrain_surface::prepare(WorldLocation::Desert,terrain.texture);
 Image mini=GenImageColor(180,112,BLANK);for(int y=0;y<112;y++)for(int x=0;x<180;x++){float delta=(x-90)*(MapSize-2)/78.f,sum=(y-6)*(MapSize-2)/48.f+2;V p{(sum+delta)*.5f,(sum-delta)*.5f};if(p.x>=1&&p.y>=1&&p.x<MapSize-1&&p.y<MapSize-1)ImageDrawPixel(&mini,x,y,desertWater(p)?rgb(64,168,181):desertPath(p)?rgb(136,100,64):rgb(202,160,101));}minimap=LoadTextureFromImage(mini);UnloadImage(mini);
}
void unload(){unloadScenery();oasis_water::unload();if(terrain.id)UnloadRenderTexture(terrain);if(minimap.id)UnloadTexture(minimap);terrain={};minimap={};}
void atmosphere(){
 drawSandWind();
 Vector2 p=screen(DesertPortal);if(p.x>-90&&p.x<WorldW+90){ovalOutline(p.x,p.y-82,27+std::sin(game.time*1.4f)*2,49,Fade(rgb(194,158,244),.55f));for(int i=0;i<8;i++){float age=std::fmod(game.time*.19f+i*.125f,1.f);oval(p.x+std::sin(i*2.4f+game.time)*24,p.y-13-age*140,1.5f,2,Fade(rgb(231,203,250),(1-age)*.8f));}}
}
}
