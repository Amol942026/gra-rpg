#pragma once

namespace wraith_death_art::motion {
struct Key {float hem,cloth,hood,sleeves,flutter,extinguish;};
}
#include "generated/wraith_fold_motion.hpp"

namespace wraith_death_art {
constexpr int Species=3;
constexpr float Columns[]{22,30,38,46,54,61,68,75,82,90,98,106};
struct Fold {float textureY,drift,floorY,width,shade;};
// Lower bands reach the ground first. The reversals in floorY let the front
// overlap the fabric already on the floor, like an unweighted dropped robe.
constexpr Fold Bands[]{
 {24,22,-14,.83f,1}, {35,25,-17,.94f,1}, {46,28,-15,1.04f,.93f},
 {54,29,-11,1.06f,.79f}, {62,26,-8,1.14f,.91f}, {70,17,-7,1.25f,.97f},
 {77,10,-2,1.32f,.87f}, {86,2,2,1.38f,1}, {94,-9,-5,1.42f,.80f},
 {102,-15,4,1.40f,1}, {110,-19,1,1.33f,.84f}, {118,-25,10,1.16f,.98f},
 {124,-28,13,.99f,.88f},
};
constexpr std::size_t ColCount=std::size(Columns),RowCount=std::size(Bands);
struct Vertex {Vector2 point;float u,v;Color tint;};
using Mesh=std::array<std::array<Vertex,ColCount>,RowCount>;
using Robe=std::array<Mesh,6>;

inline float blend(float a,float b,float weight){return a+(b-a)*weight;}
inline motion::Key sample(float time){
 const float position=std::clamp(time,0.f,1.f)*(std::size(motion::keys)-1);
 const auto left=std::size_t(position);
 const auto a=motion::keys[left],b=motion::keys[std::min(left+1,std::size(motion::keys)-1)];
 const float t=position-left;
 return {blend(a.hem,b.hem,t),blend(a.cloth,b.cloth,t),blend(a.hood,b.hood,t),
         blend(a.sleeves,b.sleeves,t),blend(a.flutter,b.flutter,t),blend(a.extinguish,b.extinguish,t)};
}

inline float impactSide(const Enemy& enemy){
 const int side=enemy.deathScreenSide();
 return side?float(side):int(enemy.animation.direction)>3?-1.f:1.f;
}

inline Rectangle footprint(const Enemy& enemy){
 return {-41+impactSide(enemy)*3,-18,82,33};
}

struct PaintedPart {Vector2 pivot,shift;float angle;};
inline PaintedPart suspendedPart(const Enemy& enemy,int part,int direction){
 const auto& state=enemy.deathMotion;
 const auto control=volcano_motion::sample(Species,state.phase);
 const auto opposite=volcano_motion::sample(Species,std::fmod(state.phase+.5f,1.f));
 const auto strike=volcano_motion::sample(Species,std::max(0.f,enemy.deathAction),true);
 constexpr float headings[]{0,.4636476f,1.5707963f,2.677945f,3.14159265f,3.6052403f,4.712389f,5.8195377f};
 const float facing=headings[direction],walk=state.moveBlend;
 PaintedPart pose{{64,90},{control.sway*.45f*walk,-control.bob*1.2f*(.25f+.75f*walk)},0};
 if(part==1||part==2){
  const auto step=part==1?control:opposite;
  const float travel=step.travel*VolcanoStrides[Species]*.31f/volcano_actors::bodyScale(Species)*walk;
  pose.shift={std::cos(facing)*travel,std::sin(facing)*travel-step.lift*5*walk};
 }
 if(part==0)pose.angle=control.sway*2.5f*walk;
 if(part>=4){
  pose.pivot=volcano_actors::restPivot(Species,part);
  pose.angle=control.sway*(part==4?-1:1)*4*walk;
  if(enemy.deathAction>=0){
   pose.angle+=strike.sway*(part==4?-2.5f:3.8f);
   pose.shift.x+=strike.sway*std::cos(facing)*.55f;
   pose.shift.y+=strike.sway*std::sin(facing)*.3f;
  }
 }
 return pose;
}

inline Mesh mesh(const Enemy& enemy,int direction,int part,const motion::Key& key,float side){
 constexpr float Scale=1.12f;
 const auto suspended=suspendedPart(enemy,part,direction);
 const float radians=suspended.angle*DEG2RAD;
 const float cosine=std::cos(radians),sine=std::sin(radians);
 Mesh result{};
 for(std::size_t row=0;row<RowCount;row++){
  const auto& fold=Bands[row];
  const float y=fold.textureY;
  const float drape=blend(blend(key.hood,key.cloth,enemy_death::window(y,53,76)),
                          key.hem,enemy_death::window(y,80,113));
  const float wave=std::sin((y-42)*.18f+key.cloth*PI);
  for(std::size_t col=0;col<ColCount;col++){
   const float x=Columns[col],u=x-64;
   const Vector2 local{x-suspended.pivot.x,y-suspended.pivot.y};
   const Vector2 initial{suspended.pivot.x-64+suspended.shift.x+local.x*cosine-local.y*sine,
                         suspended.pivot.y-115+suspended.shift.y+local.x*sine+local.y*cosine};
   float weight=drape;
   float finalX=u*fold.width+side*(fold.drift+(part==0?5:0));
   float finalY=fold.floorY+side*u*.115f+u*u*.0045f+(part==0?3:0);
   if(part>=4){
    const float sleeveSign=part==4?-1.f:1.f;
    const float across=x-(part==4?42.f:86.f),length=y-75;
    // Keep the sewn shoulder with the settling bodice while the loose cuff
    // can continue outward. The first and settled poses keep their anchors.
    const float shoulder=(1-enemy_death::window(y,66,89))*3.2f*key.cloth*(1-key.cloth);
    finalX=blend(sleeveSign*34+side*4+across*.71f+sleeveSign*length*.51f,finalX,shoulder);
    finalY=blend(-6+sleeveSign*across*.23f+length*.31f+side*sleeveSign*5,finalY,shoulder);
    const float tip=enemy_death::window(y,66,105);
    weight=blend(blend(key.sleeves,key.hem,tip*.44f),drape,shoulder);
   }else if(part==1||part==2){
    finalX+=(part==1?-3:3)*key.hem;
    finalY+=part==1?-2.5f:2.5f;
   }
   const float edge=std::min(1.f,std::abs(u)/21);
   const float flap=key.flutter*(.26f+.74f*edge);
   const float px=blend(initial.x,finalX,weight)+flap*side*(4+8*wave);
   const float py=blend(initial.y,finalY,weight)+flap*(6*wave-2.5f*edge);
   const float eye=part==3
       ?(1-enemy_death::window(std::abs(x-64),4,12))*(1-enemy_death::window(std::abs(y-57),5,12))
       :0;
   const float shade=blend(1,fold.shade,weight)*(1-key.extinguish*eye*.69f);
   const Color tint{static_cast<unsigned char>(std::lround(255*shade)),
                    static_cast<unsigned char>(std::lround(255*shade)),
                    static_cast<unsigned char>(std::lround(255*std::min(1.f,shade+.035f*key.cloth))),
                    255};
   result[row][col]={{Scale*px,Scale*py},(direction+x/128)/8,(part+y/128)/6,tint};
  }
 }
 return result;
}

inline Robe robe(const Enemy& enemy,int direction,const motion::Key& key,float side){
 Robe result;
 for(int part=0;part<6;part++)result[part]=mesh(enemy,direction,part,key,side);
 return result;
}

inline const Robe& restingRobe(int direction,float side){
 // Built during the loot-thumbnail pass; world remains only reuse vertices.
 static const auto prepared=[]{
  std::array<std::array<Robe,2>,8> result;
  const Enemy empty{};
  for(int facing=0;facing<8;facing++)for(int flank=0;flank<2;flank++)
   result[facing][flank]=robe(empty,facing,motion::keys[std::size(motion::keys)-1],flank?-1.f:1.f);
  return result;
 }();
 return prepared[direction][side<0];
}

inline float cross(Vector2 a,Vector2 b,Vector2 c){
 return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);
}

template<class Visit>
inline bool clothTriangles(const Mesh& vertices,Visit visit){
 // Submit the grounded hem first and lay the collapsing folds over it.
 for(std::size_t row=RowCount-1;row>0;row--)for(std::size_t col=1;col<ColCount;col++){
  const auto& a=vertices[row-1][col-1];
  const auto& b=vertices[row-1][col];
  const auto& c=vertices[row][col];
  const auto& d=vertices[row][col-1];
  if(visit(a,d,b)||visit(b,d,c))return true;
 }
 return false;
}

inline bool opaque(const Mesh& mesh,const volcano_actors::Atlas& atlas,Vector2 probe){
 return clothTriangles(mesh,[&](const Vertex& a,const Vertex& b,const Vertex& c){
  const float area=cross(a.point,b.point,c.point);
  if(std::abs(area)<.0001f)return false;
  const float u=cross(b.point,c.point,probe)/area,v=cross(c.point,a.point,probe)/area,w=1-u-v;
  if(u<0||v<0||w<0)return false;
  const int column=int((u*a.u+v*b.u+w*c.u)*atlas.art.width);
  const int row=int((u*a.v+v*b.v+w*c.v)*atlas.art.height);
  return column>=0&&column<atlas.art.width&&row>=0&&row<atlas.art.height
         &&atlas.alpha[row*atlas.art.width+column]>=100;
 });
}

inline void paint(const Mesh& mesh,Texture2D art,float x,float y,bool shadow,unsigned char alpha,Color solid=BLANK){
 rlCheckRenderBatchLimit(int((RowCount-1)*(ColCount-1)*6));
 rlSetTexture(art.id);rlBegin(RL_TRIANGLES);rlSetTexture(art.id);
 clothTriangles(mesh,[&](const Vertex& a,const Vertex& b,const Vertex& c){
  const bool flip=cross(a.point,b.point,c.point)>0;
  for(const auto* vertex:{&a,flip?&c:&b,flip?&b:&c}){
   const auto p=vertex->point;
   const auto tint=solid.a?solid:vertex->tint;
   rlColor4ub(tint.r,tint.g,tint.b,static_cast<unsigned char>((tint.a*alpha+127)/255));
   rlTexCoord2f(vertex->u,vertex->v);
   rlVertex2f(x+p.x-(shadow?shadow_math::LightX*p.y:0),
              y+(shadow?-shadow_math::LightY*p.y:p.y));
  }
  return false;
 });
 rlEnd();rlSetTexture(0);
}

inline bool drawRobe(const Robe& meshes,const volcano_actors::Atlas& atlas,int direction,float x,float y,
                     float folds,Color outline,const Vector2* probe,bool shadow,unsigned char alpha){
 const auto order=direction>=5?std::array{0,1,2,4,5,3}:std::array{0,1,2,3,4,5};
 if(probe){
  for(int part:order)if(opaque(meshes[part],atlas,{probe->x-x,probe->y-y}))return true;
  return false;
 }
 if(shadow){
  for(int part:order)paint(meshes[part],atlas.silhouette,x,y,true,alpha,WHITE);
  return false;
 }
 if(outline.a)for(Vector2 delta:{Vector2{-1.5f,0},Vector2{1.5f,0},Vector2{0,-1.5f},Vector2{0,1.5f}})
  for(int part:order)paint(meshes[part],atlas.silhouette,x+delta.x,y+delta.y,false,alpha,outline);
 const bool lit=volcano_corpse_material::begin(atlas.relief,enemy_death::ease(folds),atlas.authoredRest);
 for(int part:order)paint(meshes[part],atlas.art,x,y,false,alpha);
 if(lit)EndShaderMode();
 return false;
}

inline bool draw(const Enemy& enemy,float x,float y,float normalizedTime,Color outline,
                 const Vector2* probe,bool shadow,float opacity){
 const auto& atlas=volcano_actors::atlases[Species];
 const auto alpha=static_cast<unsigned char>(std::lround(255*std::clamp(opacity,0.f,1.f)));
 if(!atlas.art.id||!alpha)return false;
 const int direction=int(enemy.animation.direction);
 const float side=impactSide(enemy);
 if(normalizedTime>=1)
  return drawRobe(restingRobe(direction,side),atlas,direction,x,y,1,outline,probe,shadow,alpha);
 const auto key=sample(normalizedTime);
 const auto active=robe(enemy,direction,key,side);
 return drawRobe(active,atlas,direction,x,y,key.cloth,outline,probe,shadow,alpha);
}
}
