#pragma once

namespace wraith_death_art::motion {
struct Key {float hem,cloth,hood,sleeves,flutter,extinguish;};
}
#include "generated/wraith_fold_motion.hpp"
#include "ash_wraith_cloth.hpp"

namespace wraith_death_art {

struct Fold {float textureY,drift,floorY,width,shade;};
// Lower bands reach the ground first. The reversals in floorY let the front
// overlap the fabric already on the floor, like an unweighted dropped robe.
constexpr Fold Bands[]{
 {0,16,-10,.84f,1}, {15,20,-13,.85f,1}, {24,22,-14,.83f,1}, {35,25,-17,.94f,1}, {46,28,-15,1.04f,.93f},
 {54,29,-11,1.06f,.79f}, {62,26,-8,1.14f,.91f}, {70,17,-7,1.25f,.97f},
 {77,10,-2,1.32f,.87f}, {86,2,2,1.38f,1}, {94,-9,-5,1.42f,.80f},
 {102,-15,4,1.40f,1}, {110,-19,1,1.33f,.84f}, {118,-25,10,1.16f,.98f},
 {128,-28,13,.99f,.88f},
};
constexpr std::size_t ColCount=6,RowCount=10;
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

inline Fold foldAt(float y){
 const auto* upper=std::upper_bound(Bands+1,Bands+std::size(Bands)-1,y,[](float v,const auto& band){return v<band.textureY;});
 const auto& a=upper[-1];const auto& b=*upper;
 const float t=std::clamp((y-a.textureY)/(b.textureY-a.textureY),0.f,1.f);
 return {y,blend(a.drift,b.drift,t),blend(a.floorY,b.floorY,t),blend(a.width,b.width,t),blend(a.shade,b.shade,t)};
}
inline Mesh mesh(const Pose& pose,int part,const motion::Key& key,float side){
 const float scale=volcano_actors::bodyScale(Species);
 const auto texture=patch(pose.view,part);
 Mesh result{};
 for(std::size_t row=0;row<RowCount;row++){
  const float v=float(row)/(RowCount-1),y=texture.canvas.y+texture.canvas.height*v;
  const auto fold=foldAt(y);
  const float drape=blend(blend(key.hood,key.cloth,enemy_death::window(y,35,72)),
                          key.hem,enemy_death::window(y,70,107));
  const float wave=std::sin((y-42)*.18f+key.cloth*PI);
  for(std::size_t col=0;col<ColCount;col++){
   const float h=float(col)/(ColCount-1),x=texture.canvas.x+texture.canvas.width*h,u=x-64;
   const auto initial=suspended(pose,part,{x,y});
   auto& vertex=result[row][col];
   vertex.u=h;
   vertex.v=v;
   if(!key.cloth&&!key.hem&&!key.sleeves&&!key.hood&&!key.flutter){
    vertex.point={scale*initial.x,scale*initial.y};vertex.tint=WHITE;
    continue;
   }
   float weight=drape;
   float finalX=u*fold.width+side*(fold.drift+(part==0?5:0));
   float finalY=fold.floorY+side*u*.115f+u*u*.0045f+(part==0?3:0);
   if(part>=4){
    const auto pivot=pose.pivot[part],palm=pose.palm[part-4];
    const float dx=palm.x-pivot.x,dy=palm.y-pivot.y;
    const float tip=enemy_death::window((x-pivot.x)*dx+(y-pivot.y)*dy,0,std::max(1.f,dx*dx+dy*dy));
    const float spread=std::clamp((palm.x-64)/22.f,-1.f,1.f);
    finalX+=tip*(spread*(10.f+(y-pivot.y)*.14f)-side*4.f);
    finalY+=tip*(-5.f+2.5f*spread*side+(y-palm.y)*.34f);
    weight=blend(drape,key.sleeves,tip);
   }else if(part==3){
    // The crown flops over the bunched mantle. Keep the hollow lip of
    // the hood thick enough to read once the rest of the garment is down.
    const auto neck=pose.pivot[3];
    const float hood=1-enemy_death::window(y,neck.y+1,neck.y+18);
    const float crown=neck.y-y,across=x-neck.x;
    finalX=blend(finalX,side*(19.f+crown*.56f)+across*.86f,hood);
    finalY=blend(finalY,-12.f-crown*.61f+side*across*.22f+across*across*.009f,hood);
   }else if(part==1||part==2){
    finalX+=(part==1?-3:3)*key.hem;
    finalY+=part==1?-2.5f:2.5f;
   }
   const float edge=std::min(1.f,std::abs(u)/21);
   const float flap=key.flutter*(.26f+.74f*edge);
   const float px=blend(initial.x,finalX,weight)+flap*side*(4+8*wave);
   const float py=blend(initial.y,finalY,weight)+flap*(6*wave-2.5f*edge);
   const float eye=part==3
       ?(1-enemy_death::window(std::abs(x-64),4,12))*(1-enemy_death::window(std::abs(y-35),5,12))
       :0;
   const float shade=blend(1,fold.shade,weight)*(1-key.extinguish*eye*.69f);
   const Color tint{static_cast<unsigned char>(std::lround(255*shade)),
                    static_cast<unsigned char>(std::lround(255*shade)),
                    static_cast<unsigned char>(std::lround(255*std::min(1.f,shade+.035f*key.cloth))),
                    255};
   vertex.point={scale*px,scale*py};vertex.tint=tint;
  }
 }
 return result;
}

inline Robe robe(const Pose& pose,const motion::Key& fold,float side){
 Robe result;
 for(int part=0;part<6;part++)result[part]=mesh(pose,part,fold,side);
 return result;
}
// Geometry stays in the exact landed pose when the body becomes lootable.
// An arbitrarily angled corpse keeps its own geometry throughout the final
// settling frames. Nearby headings can use different fractional cloth shapes.
inline const Robe& restingRobe(const Pose& pose,float side){
 struct Cached {float angle=-1;Robe mesh;};
 static std::array<std::array<std::optional<Cached>,2>,32> prepared{};
 auto& cached=prepared[pose.view.index][side<0];
 if(!cached||cached->angle!=pose.view.angle)
  cached=Cached{pose.view.angle,robe(pose,motion::keys[std::size(motion::keys)-1],side)};
 return cached->mesh;
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

inline bool opaque(const Mesh& mesh,const View& view,int part,const Patch& texture,Vector2 probe){
 return clothTriangles(mesh,[&](const Vertex& a,const Vertex& b,const Vertex& c){
  const float area=cross(a.point,b.point,c.point);
  if(std::abs(area)<.0001f)return false;
  const float u=cross(b.point,c.point,probe)/area,v=cross(c.point,a.point,probe)/area,w=1-u-v;
  if(u<0||v<0||w<0)return false;
  const Vector2 uv{u*a.u+v*b.u+w*c.u,u*a.v+v*b.v+w*c.v};
  if(rotation::ready())return rotation::solid(view.left,view.right,view.fraction,part,uv);
  const auto& atlas=*texture.atlas;
  const int column=int(texture.image.x+uv.x*texture.image.width);
  const int row=int(texture.image.y+uv.y*texture.image.height);
  return column>=0&&column<atlas.art.width&&row>=0&&row<atlas.art.height
         &&atlas.alpha[row*atlas.art.width+column]>=100;
 });
}

inline void paint(const Mesh& mesh,const Patch& texture,Texture2D art,int part,float x,float y,bool shadow,unsigned char alpha,Color solid=BLANK){
 const bool registered=rotation::ready();
 const float textureTag=(shadow?32.f:solid.a?16.f:0.f)+2.f*part;
 rlCheckRenderBatchLimit(int((RowCount-1)*(ColCount-1)*6));
 rlSetTexture(art.id);rlBegin(RL_TRIANGLES);rlSetTexture(art.id);
 clothTriangles(mesh,[&](const Vertex& a,const Vertex& b,const Vertex& c){
  const bool flip=(cross(a.point,b.point,c.point)>0)!=shadow;
  for(const auto* vertex:{&a,flip?&c:&b,flip?&b:&c}){
   const auto p=vertex->point;
   const auto tint=solid.a?solid:vertex->tint;
   rlColor4ub(tint.r,tint.g,tint.b,static_cast<unsigned char>((tint.a*alpha+127)/255));
   if(registered)rlTexCoord2f(textureTag+vertex->u,vertex->v);
   else rlTexCoord2f((texture.image.x+texture.image.width*vertex->u)/art.width,
                     (texture.image.y+texture.image.height*vertex->v)/art.height);
   rlVertex2f(x+p.x-(shadow?shadow_math::LightX*p.y:0),
              y+(shadow?-shadow_math::LightY*p.y:p.y));
  }
  return false;
 });
 rlEnd();rlSetTexture(0);
}

inline bool drawRobe(const Robe& mesh,const Pose& pose,float x,float y,float size,float folds,
                     Color outline,const Vector2* probe,bool shadow,bool flash,unsigned char alpha){
 // Left and right are anatomical layers across the turn. The hood always
 // covers both shoulder seams, including the transition to a profile.
 constexpr int fixedOrder[]={0,1,2,4,5,3};
 const auto& order=rotation::ready()?fixedOrder:ash_wraith_rig::order[pose.view.base];
 if(probe){
  const Vector2 local{(probe->x-x)/size,(probe->y-y)/size};
  for(int part:order)if(opaque(mesh[part],pose.view,part,patch(pose.view,part),local))return true;
  return false;
 }
 rlPushMatrix();rlTranslatef(x,y,0);rlScalef(size,size,1);
 const bool registered=rotation::ready();
 if(registered)rotation::begin(pose.view.left,pose.view.right,pose.view.fraction,pose.view.angle,enemy_death::ease(folds),pages[0].relief);
 const auto mask=[&](float dx,float dy,Color c){
  for(int part:order){
   const auto texture=patch(pose.view,part);
   paint(mesh[part],texture,registered?rotation::weave:texture.atlas->silhouette,part,dx,dy,shadow,alpha,c);
  }
 };
 if(shadow)mask(0,0,WHITE);
 else{
  if(outline.a)for(auto offset:{Vector2{-1.5f,0},Vector2{1.5f,0},Vector2{0,-1.5f},Vector2{0,1.5f}})
   mask(offset.x/size,offset.y/size,outline);
  for(int part:order){
   const auto texture=patch(pose.view,part);
   const auto& atlas=*texture.atlas;
   const bool lit=!registered&&volcano_corpse_material::begin(atlas.relief,enemy_death::ease(folds),atlas.authoredRest);
   paint(mesh[part],texture,registered?rotation::weave:atlas.art,part,0,0,false,alpha);
   if(lit)EndShaderMode();
  }
  if(flash)mask(0,0,Fade(WHITE,.48f));
 }
 if(registered){if(shadow)BeginShaderMode(world_shadows::maskShader);else EndShaderMode();}
 rlPopMatrix();return false;
}
inline unsigned char opacityByte(float opacity){
 return static_cast<unsigned char>(std::lround(255*std::clamp(opacity,0.f,1.f)));
}
inline bool living(float x,float y,float size,const AnimationState& st,float action,Color outline,
                   const Vector2* probe,bool shadow,bool flash,float opacity){
 const auto alpha=opacityByte(opacity);if(!alpha||size<=0)return false;
 const auto pose=samplePose(st,action);
 const auto cloth=robe(pose,{},1);
 const bool hit=drawRobe(cloth,pose,x,y,size,0,outline,probe,shadow,flash,alpha);
 if(!probe&&!shadow&&action>0&&action<.45f){
  const float heat=enemy_death::window(action,.09f,.40f);
  if(heat>0){
   const auto hand=suspended(pose,5,pose.palm[1]);
   const float scale=size*volcano_actors::bodyScale(Species);
   const Vector2 center{x+scale*hand.x,y+scale*hand.y};
   const float pulse=(.86f+.14f*std::cos(action*23))*heat*size;
   DrawCircleV(center,5.7f*pulse,Fade({255,118,44,255},.23f*opacity));
   DrawCircleV(center,2.9f*pulse,Fade({255,163,74,255},.83f*opacity));
   DrawCircleV(center,1.3f*pulse,Fade({255,231,167,255},opacity));
  }
 }
 return hit;
}
inline bool draw(const Enemy& enemy,float x,float y,float normalizedTime,Color outline,
                 const Vector2* probe,bool shadow,float opacity){
 const auto alpha=opacityByte(opacity);
 if(!volcano_actors::atlases[Species].art.id||!alpha)return false;
 // Use the saved lethal-hit frame for every phase, even if AI has since stopped.
 const auto pose=samplePose(normalizedTime>=1?enemy.animation:enemy.deathMotion,enemy.deathAction);
 const float side=impactSide(enemy);
 if(normalizedTime>=1)
  return drawRobe(restingRobe(pose,side),pose,x,y,1,1,outline,probe,shadow,false,alpha);
 const auto fold=sample(normalizedTime);
 const auto active=robe(pose,fold,side);
 return drawRobe(active,pose,x,y,1,fold.cloth,outline,probe,shadow,false,alpha);
}
}
