#pragma once
#include "marsh_turn_art.hpp"

namespace marsh_art {
struct ActorFrame {
 int species,bearing;
 clips::Clip clip;
 int frame;
 float residual,phase,gaitPhase,gaitWeight;
};
inline float fallPhase(const Enemy& enemy){
 return enemy.alive?-1.f:enemy.corpseGrounded()?1.f:enemy.deathProgress()/enemy_death::groundContact(enemy.type);
}
inline ActorFrame actorFrame(const Enemy& e,float fall=-1.f){
 const auto& motion=fall>=0&&e.deathMotion.facingInitialized?e.deathMotion:e.animation;
 float yaw=int(motion.direction)*PI/4;
 if(motion.facingInitialized){
  const auto axis=actorFacing(motion.facingAngle);
  yaw=std::atan2(axis.y,axis.x);
 }
 const float coordinate=yaw*(Facings/(2.f*PI));
 const int nearest=int(std::lround(coordinate)),bearing=(nearest+Facings)%Facings;
 const float gait=motion.phase-std::floor(motion.phase);
 float weight=motion.motionInitialized?motion.moveBlend:(motion.moving?1.f:0.f);
 auto clip=clips::Walk;
 float phase=gait;
 if(fall>=0){clip=clips::Death;phase=fall;}
 else if(e.abilityBusy()){
  const auto& spec=enemyAbilityProfile(e.type);
  clip=clips::Attack;
  if(e.abilityAge<spec.windup)phase=.5f*e.abilityAge/spec.windup;
  else if(e.abilityAge<spec.windup+spec.active)phase=.5f+.25f*(e.abilityAge-spec.windup)/spec.active;
  else phase=.75f+.25f*(e.abilityAge-spec.windup-spec.active)/spec.recovery;
 }else if(e.windup>0||e.strike>0){
  clip=clips::Attack;
  phase=e.windup>0?.5f*(1-e.windup/meleeWindup(e.type)):1-.5f*e.strike/meleeRecovery(e.type);
 }
 const bool contact=clip==clips::Attack&&phase>=.44f&&phase<=.59f;
 if(fall<0&&e.flash>0&&!contact){clip=clips::Hurt;phase=1-e.flash/enemyHitRecovery(e.type);}
 phase=std::clamp(phase,0.f,1.f);
 if(clip!=clips::Walk){
  const float edge=clip==clips::Death?phase:std::min(phase,1.f-phase);
  const float t=std::clamp(edge/.18f,0.f,1.f);
  weight*=1-t*t*(3-2*t);
 }
 int frame=clip==clips::Walk?int(phase*clips::Frames):int(std::lround(phase*(clips::Frames-1)));
 return {e.type-PeatToad,bearing,clip,std::min(frame,clips::Frames-1),coordinate-nearest,phase,gait,std::clamp(weight,0.f,1.f)};
}
inline float actorScale(int s){return enemyInfo(PeatToad+s).visualHeight*.92f/turn::RestHeight[s];}
inline int viewIndex(const ActorFrame& pose){
 const int count=int(paintings[pose.species].views.size());
 const int index=int(std::lround((pose.bearing+pose.residual)*(count/Facings)));
 return (index+count)%count;
}
inline const ViewRect& viewRect(const ActorFrame& pose){
 return paintings[pose.species].views[viewIndex(pose)];
}
inline void beginInk(const ActorFrame& pose,int mode){
 const auto& rect=viewRect(pose);
 const auto& texture=paintings[pose.species].pages[rect.page].texture;
 const Vector4 limits{(rect.x+.5f)/texture.width,(rect.y+.5f)/texture.height,
                      (rect.x+rect.w-.5f)/texture.width,(rect.y+rect.h-.5f)/texture.height};
 BeginShaderMode(bodyInk);
 SetShaderValue(bodyInk,inkLimits,&limits,SHADER_UNIFORM_VEC4);
 SetShaderValue(bodyInk,inkMode,&mode,SHADER_UNIFORM_INT);
}
struct Sample {int a,b;float t;};
inline Sample sampleAt(clips::Clip clip,float phase){
 const bool cycle=clip==clips::Walk;
 const float f=phase*(cycle?clips::Frames:clips::Frames-1);
 const int a=std::min(int(f),clips::Frames-1),b=cycle?(a+1)%clips::Frames:std::min(a+1,clips::Frames-1);
 return {int(clip)*clips::Frames+a,int(clip)*clips::Frames+b,f-a};
}
struct Surface {
 std::array<Vector2,Vertices> vertex;
 std::array<clips::Foot,2> feet;
 Vector2 light;
 Rectangle bounds;
};
inline float interpolate(float a,float b,float t){return a+(b-a)*t;}
inline Surface buildSurface(const ActorFrame& p){
 Surface mesh{};
 const int s=p.species,w=turn::CellW[s],h=turn::CellH[s];
 const auto& rig=rigs[s];
 const int near=p.bearing,far=(near+(p.residual>=0?1:-1)+Facings)%Facings;
 const float yawMix=std::abs(p.residual);
 const auto gait=sampleAt(clips::Walk,p.gaitPhase),action=sampleAt(p.clip,p.phase);
 // An action begins and ends on its neutral key. Carry the underlying stride
 // through those boundaries, and use the same weights for the feet and lamp.
 std::array<int,5> keys{};
 std::array<float,5> weights{};
 int count=0;
 auto add=[&](int key,float weight){
  if(std::abs(weight)<.000001f)return;
  for(int i=0;i<count;i++)if(keys[i]==key){weights[i]+=weight;return;}
  keys[count]=key;weights[count++]=weight;
 };
 if(p.clip==clips::Walk)add(0,1-p.gaitWeight);
 else {add(action.a,1-action.t);add(action.b,action.t);add(0,-p.gaitWeight);}
 add(gait.a,(1-gait.t)*p.gaitWeight);add(gait.b,gait.t*p.gaitWeight);
 // Interpolate physical joint positions separately from the registered pigment.
 for(int i=0;i<count;i++){
  const auto* a=&rig.offsets[(near*Poses+keys[i])*Vertices];
  const auto* b=&rig.offsets[(far*Poses+keys[i])*Vertices];
  const float wa=(1-yawMix)*weights[i]/16.f,wb=yawMix*weights[i]/16.f;
  for(int j=0;j<Vertices;j++){
   auto& v=mesh.vertex[j];
   v.x+=a[j].x*wa+b[j].x*wb;v.y+=a[j].y*wa+b[j].y*wb;
  }
  const auto& ma=rig.marks[near*Poses+keys[i]];
  const auto& mb=rig.marks[far*Poses+keys[i]];
  mesh.light.x+=ma.light[0]*wa+mb.light[0]*wb;mesh.light.y+=ma.light[1]*wa+mb.light[1]*wb;
  for(int j=0;j<2;j++){
   auto& f=mesh.feet[j];
   f.x+=ma.foot[j][0]*wa+mb.foot[j][0]*wb;f.y+=ma.foot[j][1]*wa+mb.foot[j][1]*wb;
   f.rx+=ma.foot[j][2]*wa+mb.foot[j][2]*wb;
  }
 }
 Vector2 low{1e6f,1e6f},high{-1e6f,-1e6f};
 for(int row=0;row<Side;row++)for(int col=0;col<Side;col++){
  const int j=row*Side+col;auto& v=mesh.vertex[j];
  v.x+=float(col*w)/Divisions-turn::AnchorX[s];
  v.y+=float(row*h)/Divisions-turn::AnchorY[s];
  low.x=std::min(low.x,v.x);low.y=std::min(low.y,v.y);
  high.x=std::max(high.x,v.x);high.y=std::max(high.y,v.y);
 }
 mesh.bounds={low.x,low.y,high.x-low.x,high.y-low.y};
 if(mesh.feet[0].x>mesh.feet[1].x)std::swap(mesh.feet[0],mesh.feet[1]);
 return mesh;
}
inline const Surface& surface(const ActorFrame& p){
 struct Entry { ActorFrame key{};Surface mesh{};unsigned revision=0; };
 // Last poses are shared by shadow, body, outline and pointer probes.
 static std::array<Entry,48> entries{};
 static unsigned next=0;
 auto same=[&](const Entry& e){
  const auto& k=e.key;
  return e.revision==rigRevision&&k.species==p.species&&k.bearing==p.bearing&&k.clip==p.clip&&
         k.residual==p.residual&&k.phase==p.phase&&k.gaitPhase==p.gaitPhase&&k.gaitWeight==p.gaitWeight;
 };
 if(next&&same(entries[(next-1)%entries.size()]))return entries[(next-1)%entries.size()].mesh;
 for(const auto& e:entries)if(same(e))return e.mesh;
 auto& e=entries[(next++)%entries.size()];
 e={p,buildSurface(p),rigRevision};return e.mesh;
}
inline std::array<clips::Foot,2> supports(const ActorFrame& p){return rigs[p.species].marks.empty()?std::array<clips::Foot,2>{}:surface(p).feet;}
inline Vector2 lightCore(const ActorFrame& p){return rigs[p.species].marks.empty()?Vector2{}:surface(p).light;}
inline float registeredAlpha(const ActorFrame& pose,float x,float y){
 const auto& rect=viewRect(pose);
 const auto& page=paintings[pose.species].pages[rect.page];
 x=std::clamp(x-rect.left-.5f,0.f,float(rect.w-1));
 y=std::clamp(y-rect.top-.5f,0.f,float(rect.h-1));
 const int ix=int(x),iy=int(y),jx=std::min(ix+1,rect.w-1),jy=std::min(iy+1,rect.h-1);
 const auto at=[&](int px,int py){return page.alpha[(rect.y+py)*page.texture.width+rect.x+px];};
 return interpolate(interpolate(at(ix,iy),at(jx,iy),x-ix),interpolate(at(ix,jy),at(jx,jy),x-ix),y-iy)/255.f;
}
inline bool hit(const ActorFrame& pose,const Surface& mesh,Vector2 point){
 if(!CheckCollisionPointRec(point,mesh.bounds))return false;
 auto inTriangle=[&](int a,int b,int c){
  const auto va=mesh.vertex[a],vb=mesh.vertex[b],vc=mesh.vertex[c];
  const float bx=vb.x-va.x,by=vb.y-va.y,cx=vc.x-va.x,cy=vc.y-va.y,det=bx*cy-by*cx;
  if(det>=-.00001f)return false;
  const float dx=point.x-va.x,dy=point.y-va.y;
  const float wb=(dx*cy-dy*cx)/det,wc=(bx*dy-by*dx)/det,wa=1-wb-wc;
  if(std::min({wa,wb,wc})<-.000002f)return false;
  const float u=(a%Side*wa+b%Side*wb+c%Side*wc)/Divisions;
  const float v=(a/Side*wa+b/Side*wb+c/Side*wc)/Divisions;
  // Raylib's source-alpha blend writes coverage squared into a clear RGBA
  // target. Use the visible mask cutoff for thin roots and fronds too.
  const float coverage=registeredAlpha(pose,u*turn::CellW[pose.species],v*turn::CellH[pose.species]);
  return coverage*coverage>=8.f/255;
 };
 for(int row=0;row<Divisions;row++)for(int col=0;col<Divisions;col++){
  const int a=row*Side+col,b=a+Side,c=b+1,d=a+1;
  const auto& v=mesh.vertex;
  if(point.y<std::min({v[a].y,v[b].y,v[c].y,v[d].y})||point.x<std::min({v[a].x,v[b].x,v[c].x,v[d].x})||
     point.y>std::max({v[a].y,v[b].y,v[c].y,v[d].y})||point.x>std::max({v[a].x,v[b].x,v[c].x,v[d].x}))continue;
  if(inTriangle(a,b,c)||inTriangle(a,c,d))return true;
 }
 return false;
}
template<class Project>inline void drawSurface(const ActorFrame& p,const Surface& mesh,Color tint,Project project){
 const auto& rect=viewRect(p);
 const auto& texture=paintings[p.species].pages[rect.page].texture;
 const float w=turn::CellW[p.species],h=turn::CellH[p.species];
 std::array<Vector2,Vertices> points;
 std::array<float,Side> us,vs;
 for(int i=0;i<Side;i++){
  us[i]=(rect.x-rect.left+w*i/Divisions)/texture.width;
  vs[i]=(rect.y-rect.top+h*i/Divisions)/texture.height;
 }
 for(int i=0;i<Vertices;i++)points[i]=project(mesh.vertex[i]);
 rlCheckRenderBatchLimit(Divisions*Divisions*4);rlSetTexture(texture.id);rlBegin(RL_QUADS);
 rlColor4ub(tint.r,tint.g,tint.b,tint.a);
 auto emit=[&](int x,int y){const auto v=points[y*Side+x];rlTexCoord2f(us[x],vs[y]);rlVertex2f(v.x,v.y);};
 for(int y=0;y<Divisions;y++)for(int x=0;x<Divisions;x++){emit(x,y);emit(x,y+1);emit(x+1,y+1);emit(x+1,y);}
 rlEnd();rlSetTexture(0);
}
inline bool actor(const Enemy& e,float x,float y,Color outline=BLANK,const Vector2* probe=nullptr,
                  bool shadow=false,float size=1,float fall=-1.f){
 const auto p=actorFrame(e,fall);
 const float scale=actorScale(p.species)*size;
 if(scale<=0||rigs[p.species].marks.empty())return false;
 const auto& mesh=surface(p);
 if(probe)return hit(p,mesh,{(probe->x-x)/scale,(probe->y-y)/scale});
 auto draw=[&](Color color,float dx=0,float dy=0){drawSurface(p,mesh,color,[&](Vector2 v){return Vector2{x+dx+scale*v.x,y+dy+scale*v.y};});};
 if(outline.a&&!shadow){
  beginInk(p,1);draw(outline,-1,0);draw(outline,1,0);draw(outline,0,-1);draw(outline,0,1);EndShaderMode();
 }
 Color tint=!shadow&&fall<0&&e.flash>0?Color{255,214,208,255}:WHITE;
 if(!shadow&&p.clip==clips::Death){
  const float fade=p.phase*p.phase;
  tint=ColorLerp(tint,p.species==2?Color{132,121,93,255}:Color{213,205,187,255},fade);
 }
 beginInk(p,shadow?2:0);
 draw(tint);
 EndShaderMode();
 return true;
}
}
