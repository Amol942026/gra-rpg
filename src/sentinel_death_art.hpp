#pragma once
#include "generated/sentinel_shatter_data.hpp"

namespace sentinel_death_art {
constexpr int Species=4;
constexpr float Tile=128.f;
struct Fragment {
 int part;
 std::array<Vector2,4> cut;
 Vector2 origin,landing;
 float turn,thickness,delay,hop;
};

// Each shared edge is a cut through the same directional painting. The
// pauldrons split off the shield and sword and the boots leave the greaves.
inline constexpr std::array<Fragment,12> fragments{{
 {0,{{{0,0},{0,128},{128,128},{128,0}}},{64,85},{-10,-33},-18,.33f,.00f,3},
 {1,{{{0,0},{0,108},{128,105},{128,0}}},{57,94},{-54,-15},-59,.77f,.03f,7},
 {1,{{{0,108},{0,128},{128,128},{128,105}}},{57,113},{-55,28},-32,.86f,.05f,7},
 {2,{{{0,0},{0,105},{128,108},{128,0}}},{75,94},{8,26},72,.76f,.02f,9},
 {2,{{{0,105},{0,128},{128,128},{128,108}}},{75,113},{43,9},49,.82f,.07f,6},
 {3,{{{0,59},{0,128},{73,128},{62,51.25f}}},{54,73},{-11,8},-64,.76f,.04f,9},
 {3,{{{62,51.25f},{73,128},{128,128},{128,43}}},{73,71},{15,-5},74,.78f,.07f,11},
 {3,{{{0,0},{0,59},{128,43},{128,0}}},{64,42},{44,-22},48,.80f,.15f,14},
 {4,{{{0,0},{0,74},{128,80},{128,0}}},{44,65},{-40,-34},-48,.80f,.11f,11},
 {4,{{{0,74},{0,128},{128,128},{128,80}}},{45,92},{-41,9},-39,.76f,.06f,7},
 {5,{{{0,0},{0,128},{79,128},{90,0}}},{77,76},{58,-4},61,.78f,.09f,10},
 {5,{{{90,0},{79,128},{128,128},{128,0}}},{93,79},{48,30},81,.87f,.14f,13},
}};

struct FlyingChip {Vector2 origin,landing;float radius,angle,hop;};
inline constexpr std::array<FlyingChip,10> chips{{
 {{-10,-45},{-14,25},2.6f,24,14}, {{13,-54},{58,-12},3.2f,41,20},
 {{10,-43},{3,19},3.4f,-25,17},{{-11,-50},{-37,-27},2.7f,57,19},
 {{-15,-41},{-29,25},2.1f,-33,16},{{14,-48},{51,10},2.6f,12,14},
 {{7,-53},{18,-28},2.0f,50,23},{{-7,-43},{-63,-1},2.8f,-17,12},
 {{5,-44},{2,-32},2.7f,44,20},{{-8,-38},{-18,-23},2.1f,30,14},
}};

inline float blend(float from,float to,float amount){return from+(to-from)*amount;}

inline motion::Key sample(float phase){
 const float frame=std::clamp(phase,0.f,1.f)*(std::size(motion::frames)-1);
 const int left=int(frame),right=std::min(left+1,int(std::size(motion::frames))-1);
 const float t=frame-left;
 const auto a=motion::frames[left],b=motion::frames[right];
 return {blend(a.travel,b.travel,t),blend(a.lift,b.lift,t),blend(a.spin,b.spin,t),blend(a.flatten,b.flatten,t)};
}

inline Vector2 rotate(Vector2 p,float radians){
 const float c=std::cos(radians),s=std::sin(radians);
 return {p.x*c-p.y*s,p.x*s+p.y*c};
}

struct PartPose {Vector2 pivot{64,90},shift{};float angle=0;};

inline std::array<PartPose,6> startingPose(const Enemy& e,int direction){
 // Reproduce the live rig at the instant of the killing blow, including
 // a raised blade or a lifted foot. Rendered fragments share its pivots.
 const auto& state=e.deathMotion;
 constexpr float headings[]={0,.4636476f,1.5707963f,2.677945f,3.14159265f,3.6052403f,4.712389f,5.8195377f};
 const float x=std::cos(headings[direction]),y=std::sin(headings[direction]);
 const float side=x<-.1f?-1.f:1.f,walk=state.moveBlend,action=e.deathAction;
 const float scale=volcano_actors::bodyScale(Species);
 const auto stride=volcano_motion::sample(Species,state.phase);
 const auto opposite=volcano_motion::sample(Species,std::fmod(state.phase+.5f,1.f));
 std::array<PartPose,6> parts{};
 for(int i=0;i<6;i++){
  auto& part=parts[i];
  if(i==1||i==2){
   const auto key=i==1?stride:opposite;
   const float travel=key.travel*(VolcanoStrides[Species]*.31f/scale)*walk;
   part.shift={x*travel,y*travel-key.lift*5*walk};
  }else part.shift={stride.sway*.45f*walk,-stride.bob*1.2f*(.25f+.75f*walk)};
  if(i==0)part.angle=stride.sway*2.5f*walk;
  if(i>=4){
   part.pivot=volcano_actors::restPivot(Species,i);
   part.angle=stride.sway*(i==4?-1:1)*4*walk;
  }
  if(action>=0){
   const auto key=basalt_swing::sample(action);
   part.shift={x*key.lunge,y*key.lunge*.5f};
   part.angle=i==0||i==3?side*key.torso:0.f;
   if(i>=4){
    const Vector2 joint{part.pivot.x-64,part.pivot.y-90};
    const auto turned=rotate(joint,side*key.torso*DEG2RAD);
    part.shift.x+=turned.x-joint.x;
    part.shift.y+=turned.y-joint.y;
    part.angle=side*(key.torso+(i==5?key.blade:key.shield));
   }
  }
  if(e.deathSpecial>=0&&(i==0||i==3||i>=4)){
   const auto key=caldera_attack_motion::pose(0,e.deathSpecial);
   const float weight=i==3?1.f:.56f;
   part.shift.x+=x*key.advance*weight;
   part.shift.y+=(y*key.advance*.44f-key.lift)*weight;
   part.angle+=side*key.pitch;
   if(i==3){part.pivot={64+x*12,79};part.angle+=side*key.drive*.64f;}
   if(i>=4)part.angle+=side*key.drive*(i==4?-.36f:.72f);
  }
 }
 return parts;
}

struct Pose {Vector2 center;float radians,scaleX,scaleY;};
inline Pose place(const Fragment& piece,const PartPose& source,float phase,float side){
 const auto key=sample((phase-piece.delay)/(.95f-piece.delay));
 const float scale=volcano_actors::bodyScale(Species);
 const float start=source.angle*DEG2RAD;
 const auto offset=rotate({piece.origin.x-source.pivot.x,piece.origin.y-source.pivot.y},start);
 const Vector2 from{(source.pivot.x-64+source.shift.x+offset.x)*scale,(source.pivot.y-115+source.shift.y+offset.y)*scale};
 return {{blend(from.x,piece.landing.x*side,key.travel),blend(from.y,piece.landing.y,key.travel)-piece.hop*key.lift},
         blend(start,piece.turn*side*DEG2RAD,key.spin),scale,scale*blend(1,piece.thickness,key.flatten)};
}

inline bool inCut(Vector2 local,const std::array<Vector2,4>& cut){
 for(std::size_t i=0;i<cut.size();i++){
  const auto a=cut[i],b=cut[(i+1)%cut.size()];
  if((b.x-a.x)*(local.y-a.y)-(b.y-a.y)*(local.x-a.x)>0)return false;
 }
 return true;
}

inline bool probeFragment(const volcano_actors::Atlas& atlas,const Fragment& piece,Pose pose,int direction,Vector2 point){
 const auto offset=rotate({point.x-pose.center.x,point.y-pose.center.y},-pose.radians);
 const Vector2 local{piece.origin.x+offset.x/pose.scaleX,piece.origin.y+offset.y/pose.scaleY};
 if(!inCut(local,piece.cut)||local.x<0||local.x>=Tile||local.y<0||local.y>=Tile)return false;
 const int u=int(local.x*atlas.cell/Tile),v=int(local.y*atlas.cell/Tile);
 return atlas.alpha[(piece.part*atlas.cell+v)*atlas.art.width+direction*atlas.cell+u]>=100;
}

inline void paintFragment(const Fragment& piece,Pose pose,Vector2 anchor,int direction,Texture2D texture,Color tint){
 std::array<Vector2,4> points{},uv{};
 for(std::size_t i=0;i<piece.cut.size();i++){
  const auto p=piece.cut[i];
  const auto rotated=rotate({(p.x-piece.origin.x)*pose.scaleX,(p.y-piece.origin.y)*pose.scaleY},pose.radians);
  points[i]={rotated.x+anchor.x+pose.center.x,rotated.y+anchor.y+pose.center.y};
  uv[i]={(direction+p.x/Tile)/8.f,(piece.part+p.y/Tile)/6.f};
 }
 rlCheckRenderBatchLimit(6);
 rlBegin(RL_TRIANGLES);rlSetTexture(texture.id);
 rlColor4ub(tint.r,tint.g,tint.b,tint.a);
 for(int i:{0,1,2,0,2,3}){
  rlTexCoord2f(uv[i].x,uv[i].y);
  rlVertex2f(points[i].x,points[i].y);
 }
 rlEnd();
}

inline void paintChip(Vector2 base,float radius,float radians,Color outline,bool shadow,float opacity,int index){
 const auto vertex=[&](float x,float y){
  const auto p=rotate({x*radius,y*radius},radians);
  return Vector2{base.x+p.x,base.y+p.y};
 };
 const auto top=vertex(-.18f,-.94f),left=vertex(-1,.18f),bottom=vertex(.57f,.67f),right=vertex(.82f,-.23f),crest=vertex(.17f,-.18f);
 if(outline.a&&!shadow){
  DrawTriangle(vertex(-.18f,-1.35f),vertex(-1.30f,.18f),vertex(.70f,.98f),outline);
  DrawTriangle(vertex(.70f,.98f),vertex(1.15f,-.25f),vertex(-.18f,-1.35f),outline);
 }
 const Color lit=shadow?WHITE:index%3==0?rgb(115,143,150):rgb(94,116,124);
 const Color dark=shadow?WHITE:rgb(32,44,55);
 DrawTriangle(top,left,crest,Fade(lit,opacity));
 DrawTriangle(left,bottom,crest,Fade(dark,opacity));
 DrawTriangle(bottom,right,crest,Fade(shadow?WHITE:rgb(53,72,84),opacity));
 DrawTriangle(right,top,crest,Fade(lit,opacity));
}

inline Rectangle footprint(const Enemy&){return {-67,-33,134,64};}

inline bool draw(const Enemy& e,float x,float y,float phase,Color outline,const Vector2* probe,bool shadow,float opacity){
 const int direction=int(e.animation.direction);
 const auto& atlas=volcano_actors::atlases[Species];
 if(!atlas.art.id)return false;
 if(phase<=0)return volcano_actors::draw(e.type,x,y,1,e.deathMotion,e.deathAction,outline,probe,shadow,false,false,e.deathSpecial,nullptr,e.deathScreenSide(),opacity);
 const float side=volcano_actors::flank(direction,e.deathScreenSide())?-1.f:1.f;
 const auto source=phase<.95f?startingPose(e,direction):std::array<PartPose,6>{};
 std::array<Pose,fragments.size()> poses{};
 for(std::size_t i=0;i<fragments.size();i++)poses[i]=place(fragments[i],source[fragments[i].part],phase,side);
 if(probe){
  const Vector2 local{probe->x-x,probe->y-y};
  for(std::size_t i=0;i<fragments.size();i++)
   if(probeFragment(atlas,fragments[i],poses[i],direction,local))return true;
  return false;
 }

 rlPushMatrix();
 if(shadow){
  const float shear[]={1,0,0,0,-shadow_math::LightX,-shadow_math::LightY,0,0,0,0,1,0,
                       shadow_math::LightX*y,(1+shadow_math::LightY)*y,0,1};
  rlMultMatrixf(shear);
 }
 // Match the living layering as the armor separates. Once detached each
 // panel follows its own key, so the breastplate cannot carry the limbs.
 constexpr std::array<int,12> front{0,1,2,3,4,5,6,7,8,9,10,11};
 constexpr std::array<int,12> back{0,1,2,3,4,8,9,10,11,5,6,7};
 constexpr std::array<int,12> raised{0,1,2,3,4,10,11,5,6,7,8,9};
 const bool raisedBlade=phase<.5f&&e.deathAction>=.08f&&e.deathAction<.53f;
 const auto& order=direction>=5?back:raisedBlade?raised:front;
 if(outline.a&&!shadow)
  for(const auto delta:{Vector2{-1.4f,0},Vector2{1.4f,0},Vector2{0,-1.4f},Vector2{0,1.4f}})
   for(int i:order)paintFragment(fragments[i],poses[i],{x+delta.x,y+delta.y},direction,atlas.silhouette,outline);
 const float fracture=enemy_death::window(phase,.10f,.65f);
 const bool relit=!shadow&&volcano_corpse_material::begin(atlas.relief,fracture*.92f,atlas.authoredRest);
 for(int i:order){
  const Color paint=Fade(WHITE,opacity);
  paintFragment(fragments[i],poses[i],{x,y},direction,shadow?atlas.silhouette:atlas.art,paint);
 }
 rlSetTexture(0);
 if(relit)EndShaderMode();
 const auto scatter=sample((phase-.10f)/.78f);
 const float visibility=enemy_death::window(phase,.10f,.36f)*opacity;
 if(visibility>0)for(std::size_t i=0;i<chips.size();i++){
  const auto& chip=chips[i];
  const Vector2 at{x+side*blend(chip.origin.x,chip.landing.x,scatter.travel),
                   y+blend(chip.origin.y,chip.landing.y,scatter.travel)-chip.hop*scatter.lift};
  paintChip(at,chip.radius,side*chip.angle*DEG2RAD*scatter.spin,Fade(outline,visibility),shadow,visibility,int(i));
 }
 rlPopMatrix();
 return false;
}
}
