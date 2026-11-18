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
 {5,{{{-48,-48},{-48,176},{74.875f,176},{94.125f,-48}}},{77,76},{58,-4},61,.78f,.09f,10},
 {5,{{{94.125f,-48},{74.875f,176},{176,176},{176,-48}}},{93,79},{48,30},81,.87f,.14f,13},
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

using PartPose=volcano_actors::Part;
using TurnView=volcano_actors::turn_art::View;
using TurnRig=volcano_actors::turn_art::Rig;

inline std::array<PartPose,6> startingPose(const Enemy& e,TurnView view){
 const auto at=[&](int d){
  return volcano_actors::sentinel_art::sample(e.deathMotion,e.deathAction,e.deathSpecial,d);
 };
 const auto first=at(view.base);
 const auto next=view.t>0?at(view.nextBase()):first;
 std::array<PartPose,6> parts{};
 for(int p=0;p<6;++p){
  parts[p]=volcano_actors::sentinel_art::livingPart(first,p);
  if(view.t>0)parts[p]=volcano_actors::turn_art::interpolate(
      parts[p],volcano_actors::sentinel_art::livingPart(next,p),view.t);
 }
 return parts;
}

struct Pose {Vector2 center;float radians,scaleX,scaleY;};
inline Pose place(const Fragment& piece,const PartPose& source,float phase,float side,float shieldFacing){
 const auto key=sample((phase-piece.delay)/(.95f-piece.delay));
 const float scale=volcano_actors::bodyScale(Species);
 const float start=source.angle*DEG2RAD;
 const auto offset=rotate({(piece.origin.x-source.pivot.x)*source.scaleX,(piece.origin.y-source.pivot.y)*source.scaleY},start);
 const Vector2 from{(source.pivot.x-64+source.shift.x+offset.x)*scale,(source.pivot.y-115+source.shift.y+offset.y)*scale};
 // The western painting shows the entire face of the pavise and conceals
 // most of the gauntlet and cuirass. The pavise falls beside that arm; its
 // broad upright face and the newly exposed plates then tilt towards the floor.
 const bool shield=piece.part==4;
 const float tilt=shield?.45f:(piece.part==3||piece.part==5)?.30f:0.f;
 const float thickness=piece.thickness*(1-shieldFacing*tilt);
 Vector2 landing=piece.landing;
 if(shield){
  landing.x+=13.f*shieldFacing;
  landing.y+=(landing.y<0?9.f:-6.f)*shieldFacing;
 }
 return {{blend(from.x,landing.x*side,key.travel),blend(from.y,landing.y,key.travel)-piece.hop*key.lift},
         blend(start,piece.turn*side*DEG2RAD,key.spin),scale*blend(source.scaleX,1,key.flatten),
         scale*blend(source.scaleY,thickness,key.flatten)};
}

inline bool inCut(Vector2 local,const std::array<Vector2,4>& cut){
 for(std::size_t i=0;i<cut.size();i++){
  const auto a=cut[i],b=cut[(i+1)%cut.size()];
  if((b.x-a.x)*(local.y-a.y)-(b.y-a.y)*(local.x-a.x)>0)return false;
 }
 return true;
}

inline bool probeFragment(const volcano_actors::Atlas& atlas,const Fragment& piece,Pose pose,int direction,Vector2 point,const TurnRig* rig){
 const auto offset=rotate({point.x-pose.center.x,point.y-pose.center.y},-pose.radians);
 Vector2 local{piece.origin.x+offset.x/pose.scaleX,piece.origin.y+offset.y/pose.scaleY};
 if(!inCut(local,piece.cut))return false;
 if(rig){
  if(piece.part==5 && rig->arms[1].angle!=0)local=rig->arms[1].map(local,true);
  const auto& box=rig->partBox[piece.part];
  const float u=(local.x-box.x)/box.width,v=(local.y-box.y)/box.height;
  if(u<0||u>1||v<0||v>1)return false;
  using namespace volcano_actors::turn_art;
  return turnCoverage(rig->art,rig->angle,piece.part,0,u,v)>=100.f/255;
 }
 if(local.x<0||local.x>=Tile||local.y<0||local.y>=Tile)return false;
 const int u=int(local.x*atlas.cell/Tile),v=int(local.y*atlas.cell/Tile);
 return atlas.alpha[(piece.part*atlas.cell+v)*atlas.art.width+direction*atlas.cell+u]>=100;
}

inline void paintFragment(const Fragment& piece,Pose pose,Vector2 anchor,int direction,Texture2D texture,Color tint,const TurnRig* rig,float mode){
 std::array<Vector2,4> points{},uv{};
 for(std::size_t i=0;i<piece.cut.size();i++){
  auto p=piece.cut[i];
  if(!rig && piece.part==5){
   const float bounded=std::clamp(p.y,0.f,Tile);
   if(p.x>0 && p.x<Tile)p.x-=(bounded-p.y)*11.f/Tile;
   p={std::clamp(p.x,0.f,Tile),bounded};
  }
  const auto rotated=rotate({(p.x-piece.origin.x)*pose.scaleX,(p.y-piece.origin.y)*pose.scaleY},pose.radians);
  points[i]={rotated.x+anchor.x+pose.center.x,rotated.y+anchor.y+pose.center.y};
  if(rig)uv[i]={2048.f*(piece.part+1)+mode+p.x-64,p.y-115};
  else uv[i]={(direction+p.x/Tile)/8.f,(piece.part+p.y/Tile)/6.f};
 }
 rlCheckRenderBatchLimit(6);
 rlBegin(RL_TRIANGLES);rlSetTexture(rig?rig->art.pages[0].color.id:texture.id);
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
 const bool bronze=index==2||index==7;
 // The directional shader has finished. The shadow target stores opacity
 // in red as well as alpha, including these small airborne chips.
 const auto coverage=static_cast<unsigned char>(std::clamp(opacity,0.f,1.f)*255+.5f);
 const Color mask{coverage,coverage,coverage,coverage};
 const auto pigment=[&](Color paint){return shadow?mask:Fade(paint,opacity);};
 const Color lit=pigment(bronze?rgb(175,133,79):rgb(126,124,141));
 DrawTriangle(top,left,crest,lit);
 DrawTriangle(left,bottom,crest,pigment(rgb(34,31,43)));
 DrawTriangle(bottom,right,crest,pigment(bronze?rgb(105,77,51):rgb(72,71,88)));
 DrawTriangle(right,top,crest,lit);
}

inline Rectangle footprint(const Enemy&){return {-67,-33,134,64};}

inline bool draw(const Enemy& e,float x,float y,float phase,Color outline,const Vector2* probe,bool shadow,float opacity){
 using namespace volcano_actors;
 const auto* art=turn_art::forType(e.type);
 const int key=int(e.animation.direction);
 const TurnView view=art?turn_art::view(e.animation,*art):TurnView{key,4*key,0,0};
 const int direction=view.nearestBase();
 const auto& atlas=volcano_actors::atlases[Species];
 if(!atlas.art.id)return false;
 if(phase<=0)return volcano_actors::draw(e.type,x,y,1,e.deathMotion,e.deathAction,outline,probe,shadow,false,false,e.deathSpecial,nullptr,e.deathScreenSide(),opacity);
 const float side=volcano_actors::flank(direction,e.deathScreenSide())?-1.f:1.f;
 const auto source=phase<.95f?startingPose(e,view):std::array<PartPose,6>{};
 // The shatter shader reads exactly one component of the turning sprite at
 // its unposed local UV. Each stone then has an independent rigid transform.
 std::array<Part,6> unposed{};
 // Keep the bend of the captured sword arm as the plates first separate.
 unposed[5].elbow=source[5].elbow*(1-enemy_death::window(phase,.06f,.64f));
 std::optional<TurnRig> directional;
 if(art)directional.emplace(*art,view,0,unposed,restingPoses[Species].light,side);
 auto* rig=directional?&*directional:nullptr;
 std::array<Pose,fragments.size()> poses{};
 const float west=view.base==4?1-view.t:view.nextBase()==4?view.t:0.f;
 const float shieldFacing=west*west*(3-2*west);
 for(std::size_t i=0;i<fragments.size();i++)
  poses[i]=place(fragments[i],source[fragments[i].part],phase,side,shieldFacing);
 if(probe){
  const Vector2 local{probe->x-x,probe->y-y};
  for(std::size_t i=0;i<fragments.size();i++)
   if(probeFragment(atlas,fragments[i],poses[i],direction,local,rig))return true;
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
 int layers[6];
 turn_art::order(turn_art::sentinel,direction,layers,phase<.5f?e.deathAction:-1.f);
 std::array<int,fragments.size()> order{};int index=0;
 for(int p:layers)for(size_t i=0;i<fragments.size();++i)if(fragments[i].part==p)order[index++]=int(i);
 const float fracture=enemy_death::window(phase,.10f,.65f)*.92f;
 if(rig)rig->begin(atlas.relief,shadow?0.f:fracture);
 if(outline.a&&!shadow)
  for(const auto delta:{Vector2{-1.4f,0},Vector2{1.4f,0},Vector2{0,-1.4f},Vector2{0,1.4f}})
   for(int i:order)paintFragment(fragments[i],poses[i],{x+delta.x,y+delta.y},direction,atlas.silhouette,outline,rig,512.f);
 const bool relit=!rig&&!shadow&&volcano_corpse_material::begin(atlas.relief,fracture,atlas.authoredRest);
 for(int i:order){
  const Color paint=Fade(WHITE,opacity);
  paintFragment(fragments[i],poses[i],{x,y},direction,shadow?atlas.silhouette:atlas.art,paint,rig,shadow?1024.f:0.f);
 }
 rlSetTexture(0);
 if(rig||relit)EndShaderMode();
 const auto scatter=sample((phase-.10f)/.78f);
 const float visibility=enemy_death::window(phase,.10f,.36f)*opacity;
 if(visibility>0)for(std::size_t i=0;i<chips.size();i++){
  const auto& chip=chips[i];
  const Vector2 at{x+side*blend(chip.origin.x,chip.landing.x,scatter.travel),
                   y+blend(chip.origin.y,chip.landing.y,scatter.travel)-chip.hop*scatter.lift};
  const Color edge=outline.a?Fade(outline,visibility*outline.a/255.f):BLANK;
  paintChip(at,chip.radius,side*chip.angle*DEG2RAD*scatter.spin,edge,shadow,visibility,int(i));
 }
 rlPopMatrix();
 return false;
}
}
