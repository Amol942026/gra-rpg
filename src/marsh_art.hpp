#pragma once
#include "generated/marsh_motion.hpp"
#include "marsh_footprints.hpp"
namespace marsh_art {
constexpr char AtlasPath[]="assets/media/mokradla/atlas.png";
constexpr char SceneryPath[]="assets/media/mokradla-osada/osada.png";
constexpr char WorksitePath[]="assets/media/mokradla-warsztaty/warsztaty.png";
// Cells share a ground anchor at 208 / 230 of their height. The hut uses double density.
constexpr Rectangle SceneryCells[]={{720,0,360,460},{1080,0,360,460},{0,0,720,920},{1440,0,360,460},
 {720,460,360,460},{1080,460,360,460},{1440,460,360,460},{0,920,360,460},{0,0,360,460},{360,0,360,460}};
constexpr int FirstWorksite=MarshFishTrap-MarshPropStart;
static_assert(std::size(SceneryCells)==PropTypeCount-MarshPropStart);
constexpr Rectangle ChestRow={360,920,768,256};
inline Texture2D atlas{},mini{},land{},peat{},scenery{},worksites{};
inline Texture2D contacts{};
constexpr int ContactWidth=200,ContactHeight=116,ContactLeft=-10,ContactTop=124;
constexpr int ContactDensity=2,ContactColumns=4;
inline float softFoot(float dx,float dy,float rx,float ry){
 const float d=dx*dx/(rx*rx)+dy*dy/(ry*ry);
 const float edge=std::max(0.f,1-d);
 return edge*edge;
}
inline void prepareContacts(){
 using namespace marsh_footprints;
 constexpr int w=ContactWidth*ContactDensity,h=ContactHeight*ContactDensity;
 constexpr int rows=(int(std::size(profiles))+ContactColumns-1)/ContactColumns;
 Image im=GenImageColor(ContactColumns*w,rows*h,BLANK);
 auto pixels=static_cast<Color*>(im.data);
 for(int id=0;id<int(std::size(profiles));id++){
  const auto& p=profiles[id];
  const float left=p.feet[0].x,right=p.feet[p.count-1].x;
  for(int y=0;y<h;y++)for(int x=0;x<w;x++){
   float u=ContactLeft+(x+.5f)/ContactDensity,v=ContactTop+(y+.5f)/ContactDensity;
   float row=groundRow(p,u);
   // Diffuse occlusion beneath the mass, strongest behind its bearing edge.
   const float along=(u-(left+right)*.5f)/((right-left)*.5f+8);
   float a=.42f*softFoot(along,v-row+p.bedDepth*.78f,1.05f,p.bedDepth*1.9f);
   for(int i=0;i<p.count;i++){
    const auto& f=p.feet[i];float dx=u-f.x,dy=v-f.y;
    a=std::max(a,.83f*softFoot(dx,dy,f.rx*1.35f,f.ry*1.45f));
    a=std::max(a,.50f*softFoot(dx,dy,f.rx*2.2f,f.ry*2.2f));
   }
   if(id==4)a=std::max(a,.52f*softFoot(u-82,v-207,8,3)); // grounded paddle blade
   pixels[(id/ContactColumns*h+y)*im.width+id%ContactColumns*w+x]={15,19,13,(unsigned char)std::lround(255*a)};
  }
 }
 contacts=LoadTextureFromImage(im);SetTextureFilter(contacts,TEXTURE_FILTER_BILINEAR);UnloadImage(im);
}
inline void seatScenery(Image& im,int first,int limit){
 using namespace marsh_footprints;
 // Darken the damp material just above each actual bearing surface and let the
 // last fraction of a pixel meet the soil. Open spaces beneath decking stay clear.
 ImageFormat(&im,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
 auto px=static_cast<Color*>(im.data);
 for(int id=first;id<limit;id++){
  Rectangle cell=SceneryCells[id];const float density=cell.width/180;
  const auto& p=profiles[id];
  for(int x=0;x<int(cell.width);x++){
   const float u=(x+.5f)/density,row=groundRow(p,u);
   if(u<p.feet[0].x-3||u>p.feet[p.count-1].x+3)continue;
   int bottom=std::min(int(cell.height)-1,int((row+5)*density));
   while(bottom>=int((row-3)*density)&&px[(int(cell.y)+bottom)*im.width+int(cell.x)+x].a<32)bottom--;
   const float bottomRow=(bottom+.5f)/density;
   const bool bearing=std::abs(bottomRow-row)<3;
   const float edge=bottomRow-1.25f+.48f*std::sin(u*1.19f)+.24f*std::sin(u*2.09f);
   const int end=bearing?bottom:int((row+2)*density);
   for(int y=int((row-6)*density);y<=end;y++){
    Color& c=px[(int(cell.y)+y)*im.width+int(cell.x)+x];if(!c.a)continue;
    float above=(bearing?edge:row)-(y+.5f)/density;
    float damp=std::clamp(1.f-std::max(0.f,above)/5.f,0.f,1.f)*.34f;
    c.r=(unsigned char)(c.r*(1-damp)+23*damp);c.g=(unsigned char)(c.g*(1-damp)+29*damp);c.b=(unsigned char)(c.b*(1-damp)+20*damp);
    // Only bury bottom edges, never punch a slit through a projecting root or oar.
    if(bearing&&above<1)
     c.a=(unsigned char)(c.a*std::clamp(above+.8f,0.f,1.f));
   }
  }
 }
}
inline Shader floorShader{};
inline int originLoc=-1,timeLoc=-1,detailLoc=-1;
inline bool floorReady=false;
constexpr int Cell=448;
inline std::array<Rectangle,8> cells{};
inline std::vector<unsigned char> alpha;
inline Texture2D loadScenery(const char* path,int width,int height,int first,int limit){
 Texture2D texture{};
 Image objects=LoadImage(path);
 if(objects.data&&objects.width==width&&objects.height==height){
  seatScenery(objects,first,limit);texture=LoadTextureFromImage(objects);SetTextureFilter(texture,TEXTURE_FILTER_BILINEAR);
 }
 if(objects.data)UnloadImage(objects);
 return texture;
}
inline void load(){
 scenery=loadScenery(SceneryPath,1800,1380,0,FirstWorksite);
 worksites=loadScenery(WorksitePath,720,460,FirstWorksite,int(std::size(SceneryCells)));
 prepareContacts();
 Image im=LoadImage(AtlasPath);
 if(im.data&&im.width==Cell*4&&im.height==Cell*2){
  atlas=LoadTextureFromImage(im);SetTextureFilter(atlas,TEXTURE_FILTER_BILINEAR);
  Color* px=LoadImageColors(im);alpha.resize(im.width*im.height);
  for(int i=0;i<im.width*im.height;i++)alpha[i]=px[i].a;
  UnloadImageColors(px);
  for(int i=4;i<8;i++){
   const int left=(i%4)*Cell,top=(i/4)*Cell;
   int x0=Cell,y0=Cell,x1=0,y1=0;
   for(int y=0;y<Cell;y++)for(int x=0;x<Cell;x++)if(alpha[(top+y)*im.width+left+x]>24){
    x0=std::min(x0,x);y0=std::min(y0,y);x1=std::max(x1,x);y1=std::max(y1,y);}
   if(x1>=x0)cells[i]={float(left+x0),float(top+y0),float(x1-x0+1),float(y1-y0+1)};
  }
 }
 if(im.data)UnloadImage(im);
}
inline void prop(int design,int w,int h){
 DrawTexturePro(design<FirstWorksite?scenery:worksites,SceneryCells[design],{0,0,float(w),float(h)},{},0,WHITE);
}
inline bool actor(const Enemy& e,float x,float y,Color edge,const Vector2* probe,bool shadow=false,float scale=1,float fall=-1){
 int species=e.type-PeatToad;Rectangle src=cells[4+species];if(!atlas.id||!src.width)return false;
 const bool right=std::cos(int(e.animation.direction)*PI/4)>.01f;
 const float sign=right?1.f:-1.f;
 const bool attack=e.windup>0||e.strike>0;
 int clip=fall>=0?2:attack?1:0;
 float t=fall>=0?fall:attack?(e.windup>0?.5f*(1-e.windup/meleeWindup(e.type)):.5f+.5f*(1-e.strike/meleeRecovery(e.type))):e.animation.moving?e.animation.phase:0;
 float u=std::clamp(t,0.f,1.f)*16;
 int first=std::min(15,int(u));
 const auto& a=marsh_motion::Clips[clip][species][first];
 const auto& b=marsh_motion::Clips[clip][species][first+1];
 const auto lerp=[&](float v,float w){return v+(w-v)*(u-first);};
 const float size=enemyInfo(e.type).visualHeight*scale*.92f;
 float w=size*(src.width/src.height)*lerp(a.width,b.width),h=size*lerp(a.height,b.height);
 float bob=lerp(a.y,b.y),tilt=lerp(a.angle,b.angle);
 if(fall<0&&species==2)bob+=4+2*std::sin(e.animation.idleTime*2);
 x+=sign*lerp(a.x,b.x)*scale;
 const float rotation=tilt*sign*(fall>=0?e.deathSide():1),rad=rotation*PI/180;
 Vector2 pivot={w*.5f,h};
 y-=bob*scale;
 const float depth=fall>=0?1-.45f*enemy_death::ease(fall):1.f;
 if(fall>=0){x-=std::sin(rad)*h*.5f;y-=std::abs(std::sin(rad))*w*.5f*depth;}
 if(probe){
  float dx=probe->x-x,dy=(probe->y-y)/depth;
  float u=(dx*std::cos(rad)+dy*std::sin(rad)+pivot.x)/w,v=(-dx*std::sin(rad)+dy*std::cos(rad)+pivot.y)/h;
  if(u<0||u>=1||v<0||v>=1)return false;
  int px=int(src.x)+(right?int(src.width)-1-int(u*src.width):int(u*src.width));
  return alpha[(int(src.y)+int(v*src.height))*atlas.width+px]>32;
 }
 if(right)src.width=-src.width;
 if(fall>=0){rlPushMatrix();rlTranslatef(0,y,0);rlScalef(1,depth,1);rlTranslatef(0,-y,0);}
 Rectangle dst{x,y,w,h};
 if(edge.a&&!shadow){for(Vector2 off:std::array<Vector2,4>{{{-2,0},{2,0},{0,-2},{0,2}}}){dst.x=x+off.x;dst.y=y+off.y;DrawTexturePro(atlas,src,dst,pivot,rotation,edge);}dst.x=x;dst.y=y;}
 DrawTexturePro(atlas,src,dst,pivot,rotation,shadow?WHITE:fall>=0?Color{180,182,160,255}:WHITE);if(fall>=0)rlPopMatrix();return false;
}
inline void prepareFloor(){
 constexpr int n=512;
 Image field=GenImageColor(n,n,BLACK),map=GenImageColor(180,112,BLANK);
 for(int y=0;y<n;y++)for(int x=0;x<n;x++){
  V p{(x+.5f)*MapSize/n,(y+.5f)*MapSize/n};
  float shore=marsh::shore(p),road=marsh::road(p);
  Color mask=rgb(int(255*std::clamp(.5f+shore*.125f,0.f,1.f)),int(255*std::clamp(1-road/3.f,0.f,1.f)),0);
  ImageDrawPixel(&field,x,y,mask);
  if(p.x>=1&&p.y>=1&&p.x<MapSize-1&&p.y<MapSize-1)
   ImageDrawPixel(&map,int(90+(p.x-p.y)*78/(MapSize-2)),int(6+(p.x+p.y-2)*48/(MapSize-2)),shore<-.55f?rgb(32,66,62):road<1.7f?rgb(143,126,83):rgb(74,83,57));
 }
 land=LoadTextureFromImage(field);mini=LoadTextureFromImage(map);UnloadImage(field);UnloadImage(map);
 SetTextureFilter(land,TEXTURE_FILTER_BILINEAR);SetTextureWrap(land,TEXTURE_WRAP_CLAMP);
 peat=LoadTexture("assets/media/mokradla-torf/torf.png");SetTextureWrap(peat,TEXTURE_WRAP_REPEAT);SetTextureFilter(peat,TEXTURE_FILTER_BILINEAR);
 floorShader=LoadShader(nullptr,"assets/shaders/mokradla.fs");
 if(!IsShaderValid(floorShader)||floorShader.id==rlGetShaderIdDefault())return;
 originLoc=GetShaderLocation(floorShader,"origin");timeLoc=GetShaderLocation(floorShader,"clockTime");detailLoc=GetShaderLocation(floorShader,"peat");
 int sizeLoc=GetShaderLocation(floorShader,"mapSize");
 floorReady=originLoc>=0&&timeLoc>=0&&detailLoc>=0&&sizeLoc>=0&&land.id&&peat.id;
 if(!floorReady)return;
 float worldSize=MapSize;SetShaderValue(floorShader,sizeLoc,&worldSize,SHADER_UNIFORM_FLOAT);
}
inline void drawContact(int id,V position,float scale){
 const Vector2 base=screen(position);
 const Rectangle dst={base.x+(ContactLeft-90)*scale,base.y+(ContactTop-208)*scale,ContactWidth*scale,ContactHeight*scale};
 if(!contacts.id||dst.x+dst.width<0||dst.x>WorldW||dst.y+dst.height<0||dst.y>H)return;
 constexpr int w=ContactWidth*ContactDensity,h=ContactHeight*ContactDensity;
 DrawTexturePro(contacts,{float(id%ContactColumns*w),float(id/ContactColumns*h),w,h},dst,{},0,WHITE);
 // A broken reflection touches submerged posts and roots, at their own water depths.
 if(marsh::shore(position)>6)return;
 const auto& p=marsh_footprints::profiles[id];
 for(int i=0;i<p.count;i+=p.count>4?2:1){
  const auto& f=p.feet[i];float dx=(f.x-90)*scale,dy=(f.y-208)*scale;
  const V foot=position+V{dx/80+dy/40,dy/40-dx/80};if(marsh::shore(foot)>-.4f)continue;
  const float shimmer=.5f+.5f*std::sin(game.time*1.7f+foot.x*.9f+foot.y*1.3f);
  const float rx=(f.rx+2+shimmer)*scale,ry=(.75f+.4f*shimmer)*scale;
  const float x=base.x+dx,y=base.y+dy-.25f*scale;
  const Color water{110,132,115,(unsigned char)(35+42*shimmer)};
  DrawLineEx({x-rx,y-ry*.65f},{x-.68f*rx,y+ry*.35f},.9f*scale,water);
  DrawLineEx({x-.68f*rx,y+ry*.35f},{x-.14f*rx,y+ry},.9f*scale,water);
 }
}
inline void ground(){
 if(!land.id)return;
 Vector2 origin=screenFlat({0,0});
 if(!floorReady){
  const float sx=40.f*(MapSize-2)/78.f,sy=20.f*(MapSize-2)/48.f;
  DrawTexturePro(mini,{0,0,float(mini.width),float(mini.height)},
   {origin.x-90*sx,origin.y+40-6*sy,mini.width*sx,mini.height*sy},{},0,WHITE);
  return;
 }
 SetShaderValue(floorShader,originLoc,&origin,SHADER_UNIFORM_VEC2);
 SetShaderValue(floorShader,timeLoc,&game.time,SHADER_UNIFORM_FLOAT);
 BeginShaderMode(floorShader);SetShaderValueTexture(floorShader,detailLoc,peat);
 DrawTexturePro(land,{0,0,float(land.width),float(land.height)},{0,0,WorldW,H},{},0,WHITE);EndShaderMode();
}
inline void unload(){for(auto tex:{atlas,land,mini,peat,scenery,worksites,contacts})if(tex.id)UnloadTexture(tex);if(floorShader.id&&floorShader.id!=rlGetShaderIdDefault())UnloadShader(floorShader);floorReady=false;alpha.clear();}
}
