#pragma once
#include "generated/volcano_motion.hpp"
#include "generated/basalt_swing.hpp"
// Native Krita cutouts, articulated by saved Synfig tracks. No mesh generation.
namespace volcano_actors {
inline constexpr const char* names[]={"slag-crawler","lava-salamander","fire-imp","ash-wraith","obsidian-sentinel","ember-drake","varkhor"};
struct Atlas {Texture2D art{},silhouette{};std::vector<unsigned char> alpha;int cell=128;};
inline std::array<Atlas,7> atlases{};
inline void load(){for(int i=0;i<7;i++){
 auto& a=atlases[i];if(a.art.id)continue;Image im=LoadImage((std::string("assets/volcano/")+names[i]+".png").c_str());if(!im.data)continue;
 a.cell=im.width/8;a.art=LoadTextureFromImage(im);auto* pixels=static_cast<Color*>(im.data);a.alpha.resize(im.width*im.height);
 for(int n=0;n<im.width*im.height;n++){a.alpha[n]=pixels[n].a;pixels[n]={255,255,255,pixels[n].a};}
 a.silhouette=LoadTextureFromImage(im);UnloadImage(im);SetTextureFilter(a.art,TEXTURE_FILTER_BILINEAR);SetTextureFilter(a.silhouette,TEXTURE_FILTER_BILINEAR);
}}
inline void unload(){for(auto& a:atlases){if(a.art.id)UnloadTexture(a.art);if(a.silhouette.id)UnloadTexture(a.silhouette);a={};}}
inline bool draw(int type,float x,float y,float size,const AnimationState& state,float action=-1,Color outline=BLANK,const Vector2* probe=nullptr,bool shadow=false,bool dead=false,bool flash=false){
 int id=std::clamp(type-VolcanoFirstType,0,6),direction=std::clamp(int(state.direction),0,7);const auto& atlas=atlases[id];if(!atlas.art.id)return false;
 float bodyScale=id==6?1.85f:id==3?1.12f:id==4?1.1f:1.f,s=size*bodyScale;
 static constexpr float angles[]={0,.4636476f,1.5707963f,2.677945f,3.14159265f,3.6052403f,4.712389f,5.8195377f};float heading=angles[direction];
 auto control=volcano_motion::sample(id,state.phase),opposite=volcano_motion::sample(id,std::fmod(state.phase+.5f,1.f));
 auto strike=volcano_motion::sample(id,std::max(0.f,action),true);
 float blend=dead?0:state.moveBlend;
 bool quadruped=id==0||id==1||id==5;
 struct Part {Vector2 pivot,shift;float angle;};std::array<Part,6> parts{};
 for(int p=0;p<6;p++){
  auto& part=parts[p];part.pivot={64,90};
  if(p==1||p==2||(quadruped&&p>=4)){auto k=p==1||p==5?control:opposite;float travel=k.travel*(VolcanoStrides[id]*.31f/bodyScale)*blend;part.shift={std::cos(heading)*travel,std::sin(heading)*travel-k.lift*5*blend};}
  else{part.shift={control.sway*.45f*blend,-control.bob*1.2f*(.25f+.75f*blend)};}
  if(p>=4&&!quadruped){part.pivot={64+(p==4?-1.f:1.f)*(id==6?25:18),67};part.angle=control.sway*(p==4?-1:1)*4*blend;if(action>=0){part.angle+=strike.sway*(p==4?-2.5f:3.8f);part.shift.x+=strike.sway*std::cos(heading)*.55f;part.shift.y+=strike.sway*std::sin(heading)*.3f;}}
  if(quadruped&&action>=0&&(p==3||p>=4)){part.shift.x+=strike.sway*std::cos(heading)*.3f;part.shift.y+=strike.sway*std::sin(heading)*.3f;}
  if(p==0)part.angle=control.sway*2.5f*blend;
  if(id==4&&action>=0&&!dead){
   auto key=basalt_swing::sample(action);float sign=std::cos(heading)<-.1f?-1.f:1.f;
   part.shift={std::cos(heading)*key.lunge,std::sin(heading)*key.lunge*.5f};part.angle=0;
   if(p==0||p==3)part.angle=sign*key.torso;
   if(p==4||p==5){
    float r=sign*key.torso*DEG2RAD;Vector2 joint{part.pivot.x-64,part.pivot.y-90};
    part.shift.x+=joint.x*std::cos(r)-joint.y*std::sin(r)-joint.x;part.shift.y+=joint.x*std::sin(r)+joint.y*std::cos(r)-joint.y;
    part.angle=sign*(key.torso+(p==5?key.blade:key.shield));
   }
  }
  if(dead){part.shift={};part.angle=p==1?-18:p==2?24:p==4?-32:p==5?39:0;}
 }
 float fallAngle=direction>3?-75.f:75.f;
 Vector2 localProbe{};
 if(probe){
  localProbe={probe->x-x,probe->y-y};
  if(dead){localProbe.y=(probe->y-y+3)/.42f;float r=-fallAngle*DEG2RAD;localProbe={localProbe.x*std::cos(r)-localProbe.y*std::sin(r),localProbe.x*std::sin(r)+localProbe.y*std::cos(r)-25*s};}
  for(int p=5;p>=0;p--){const auto& k=parts[p];Vector2 q{localProbe.x/s-(k.pivot.x-64+k.shift.x),localProbe.y/s-(k.pivot.y-115+k.shift.y)};float r=-k.angle*DEG2RAD;
   float u=q.x*std::cos(r)-q.y*std::sin(r)+k.pivot.x,v=q.x*std::sin(r)+q.y*std::cos(r)+k.pivot.y;
   int px=int(std::floor(u*atlas.cell/128)),py=int(std::floor(v*atlas.cell/128));
   if(px>=0&&px<atlas.cell&&py>=0&&py<atlas.cell&&atlas.alpha[(p*atlas.cell+py)*atlas.art.width+direction*atlas.cell+px]>=100)return true;
  }return false;
 }
 rlPushMatrix();
 if(shadow){float matrix[]={1,0,0,0,-shadow_math::LightX,-shadow_math::LightY,0,0,0,0,1,0,shadow_math::LightX*y,(1+shadow_math::LightY)*y,0,1};rlMultMatrixf(matrix);}
 rlTranslatef(x,y-(dead?3:0),0);if(dead){rlScalef(1,.42f,1);rlRotatef(fallAngle,0,0,1);rlTranslatef(0,25*s,0);}
 auto paint=[&](int p,Texture2D texture,Color tint,float dx=0,float dy=0){auto k=parts[p];Rectangle source{float(direction*atlas.cell),float(p*atlas.cell),float(atlas.cell),float(atlas.cell)};
  DrawTexturePro(texture,source,{(k.pivot.x-64+k.shift.x)*s+dx,(k.pivot.y-115+k.shift.y)*s+dy,128*s,128*s},{k.pivot.x*s,k.pivot.y*s},k.angle,tint);
 };
 int frontOrder[]={0,1,2,3,4,5},backOrder[]={0,1,2,4,5,3};const int* order=direction>=5?backOrder:frontOrder;
 if(!shadow&&outline.a)for(Vector2 delta:{Vector2{-1.5f,0},Vector2{1.5f,0},Vector2{0,-1.5f},Vector2{0,1.5f}})for(int n=0;n<6;n++)paint(order[n],atlas.silhouette,outline,delta.x,delta.y);
 for(int n=0;n<6;n++)paint(order[n],shadow?atlas.silhouette:atlas.art,dead?rgb(171,158,167):WHITE);
 if(flash&&!shadow)for(int n=0;n<6;n++)paint(order[n],atlas.silhouette,Fade(WHITE,.48f));
 rlPopMatrix();return false;
}
}
