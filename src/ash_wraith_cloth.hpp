#pragma once
#include "generated/ash_wraith_rig.hpp"
#include "generated/ash_wraith_turn_atlas.hpp"
#include "generated/ash_wraith_painted.hpp"
#include "generated/ash_wraith_motion.hpp"
#include "ash_wraith_turn_material.hpp"
#include "ash_wraith_painted_motion.hpp"

namespace wraith_death_art {
namespace turn=ash_wraith_turn_atlas;
namespace painted=ash_wraith_painted;
constexpr int Species=3;
inline std::array<volcano_actors::Atlas,turn::PageCount> pages{};
inline volcano_actors::Atlas paintedPage{};
inline bool paintedReady(){return paintedPage.art.id!=0;}
inline bool registeredTurn(){return !paintedReady()&&rotation::ready();}
inline void unloadPage(volcano_actors::Atlas& page){
 for(auto texture:{page.art,page.silhouette,page.relief})if(texture.id)UnloadTexture(texture);
 page={};
}
inline void unload(){
 rotation::unload();
 painted_motion::unload();
 unloadPage(paintedPage);
 for(auto& p:pages)unloadPage(p);
}
inline bool loadPage(volcano_actors::Atlas& p,const std::string& artwork,const std::string& material){
 Image im=LoadImage(artwork.c_str());
 if(!im.data)return false;
 ImageFormat(&im,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
 p.art=LoadTextureFromImage(im);
 auto* c=static_cast<Color*>(im.data);
 p.alpha.resize(im.width*im.height);
 for(size_t n=0;n<p.alpha.size();n++){p.alpha[n]=c[n].a;c[n]={255,255,255,c[n].a};}
 p.silhouette=LoadTextureFromImage(im);
 UnloadImage(im);
 p.relief=LoadTexture(material.c_str());
 p.authoredRest=p.relief.id!=0;
 for(auto texture:{p.art,p.silhouette,p.relief})if(texture.id)SetTextureFilter(texture,TEXTURE_FILTER_BILINEAR);
 return p.art.id&&p.silhouette.id;
}
inline void load(){
 if(paintedReady()||pages[0].art.id)return;
 const std::string folder="assets/volcano/wraith-turn/";
 if(loadPage(paintedPage,folder+"painted-turn.png",folder+"painted-turn-relief.png")){
  painted_motion::load(paintedPage.art.width,paintedPage.art.height);
  return;
 }
 unloadPage(paintedPage);
 for(int i=0;i<turn::PageCount;i++){
  const auto suffix=std::to_string(i)+".png";
  if(!loadPage(pages[i],folder+"parts-"+suffix,folder+"material-"+suffix)){unload();return;}
 }
 rotation::load();
}

struct View {int index,base,sector,left,right,paintedKey,paintedLeft,paintedRight;float angle,t,fraction,paintedT;};
// In the original artwork the front three-quarter sleeves have already moved
// most of the way out from the profile. The rear three-quarter views expose
// slightly less sleeve. Calibrate their angles to that painted travel, keeping
// front, back and profiles on the exact axes used by the projectile.
constexpr float paintedAngles[]={0,61*DEG2RAD,PI/2,119*DEG2RAD,PI,233*DEG2RAD,3*PI/2,307*DEG2RAD};
inline View at(const AnimationState& state){
 const auto& angles=paintedAngles;
 float angle=state.facingInitialized?std::fmod(state.facingAngle,2*PI):angles[int(state.direction)];
 if(angle<0)angle+=2*PI;
 const auto first=int(std::upper_bound(angles+1,angles+8,angle)-angles)-1;
 const float next=first==7?2*PI:angles[first+1];
 const float t=(angle-angles[first])/(next-angles[first]);
 const float sample=(first+t)*4;
 const int key=pages[0].art.id?int(std::lround(sample))%32:(first+int(t>=.5f))%8*4;
 const int lo=int(std::floor(sample));
 const int nextPainted=int(std::upper_bound(painted::angles+1,painted::angles+painted::Count,angle)-painted::angles);
 const float previous=painted::angles[nextPainted-1];
 const float following=nextPainted==painted::Count?2*PI:painted::angles[nextPainted];
 const int art=(angle-previous<following-angle?nextPainted-1:nextPainted)%painted::Count;
 const float paintedT=(angle-previous)/(following-previous);
 return {key,((key+2)/4)%8,first,lo%32,(lo+1)%32,art,nextPainted-1,nextPainted%painted::Count,
         angle,t,sample-lo,paintedT};
}
inline Vector2 lerp(Vector2 a,Vector2 b,float w){return {a.x+(b.x-a.x)*w,a.y+(b.y-a.y)*w};}
struct Patch {const volcano_actors::Atlas* atlas;Rectangle image,canvas;};
inline Patch patch(const View& v,int layer){
 if(paintedReady()){
  const auto& t=painted::tiles[v.paintedKey][layer];
  constexpr float s=128.f/painted::Cell;
  const auto bounds=[&](int d){
   const auto& node=painted::tiles[d][layer];
   return Rectangle{node.ox*s,node.oy*s,node.w*s,node.h*s};
  };
  const auto canvas=painted_motion::ready()?painted_turn_shape::interpolate(bounds(v.paintedLeft),bounds(v.paintedRight),v.paintedT):
                                             bounds(v.paintedKey);
  return {&paintedPage,{float(t.x),float(t.y),float(t.w),float(t.h)},canvas};
 }
 if(pages[0].art.id){
  const auto& t=turn::tiles[v.index][layer];constexpr float u=128.f/turn::Cell;
  const auto& a=turn::tiles[v.left][layer];const auto& b=turn::tiles[v.right][layer];
  const auto interpolate=[&](int x,int y){return (x+(y-x)*v.fraction)*u;};
  Rectangle canvas{t.ox*u,t.oy*u,t.w*u,t.h*u};
  if(registeredTurn())canvas={interpolate(a.ox,b.ox),interpolate(a.oy,b.oy),
                               interpolate(a.w,b.w),interpolate(a.h,b.h)};
  return {&pages[t.page],{float(t.x),float(t.y),float(t.w),float(t.h)},
          canvas};
 }
 const auto& a=volcano_actors::atlases[Species];const float cell=float(a.cell);
 return {&a,{v.base*cell,layer*cell,cell,cell},{0,0,128,128}};
}
using Controls=std::array<Vector2,7>;
template<size_t N>
inline Controls track(const ash_wraith_motion::Key (&keys)[N],float t,bool cycle){
 const auto* last=keys+N;
 const auto* b=std::upper_bound(keys+1,last-1,t,[](float time,const auto& k){return time<k.time;});
 const auto* a=b-1;
 float w=std::clamp((t-a->time)/(b->time-a->time),0.f,1.f);
 Controls result{};
 if(!cycle){
  w=w*w*(3-2*w);
  for(size_t i=0;i<result.size();i++)result[i]=lerp(a->point[i],b->point[i],w);
  return result;
 }
 const auto& prior=(a==keys?keys[N-2]:a[-1]);
 const auto& next=(b==last-1?keys[1]:b[1]);
 for(size_t i=0;i<result.size();i++){
  const auto interpolate=[w](float p,float x,float y,float q){
   return x+.5f*w*((y-p)+w*((2*p-5*x+4*y-q)+w*(3*x-3*y+q-p)));
  };
  result[i]={interpolate(prior.point[i].x,a->point[i].x,b->point[i].x,next.point[i].x),
             interpolate(prior.point[i].y,a->point[i].y,b->point[i].y,next.point[i].y)};
 }
 return result;
}
struct Pose {
 View view{};
 Controls drift{},cast{};
 Vector2 pivot[6]{},palm[2]{};
 ActorFacing facing{};
 float travel=0;
};
inline Pose samplePose(const AnimationState& state,float action){
 Pose p{};p.view=at(state);p.facing=actorFacing(p.view.angle);
 const auto idle=track(ash_wraith_motion::floatKeys,std::fmod(state.idleTime/1.8f,1.f),true);
 const auto flight=track(ash_wraith_motion::flightKeys,std::fmod(state.idleTime/1.2f+state.phase*.26f,1.f),true);
 const float gather=action<0?0:enemy_death::window(action,.01f,.38f)*(1-enemy_death::window(action,.56f,.96f));
 p.travel=state.moveBlend*(1-gather);
 for(size_t i=0;i<p.drift.size();i++)p.drift[i]=lerp({},lerp(idle[i],flight[i],state.moveBlend),1-gather);
 if(action>=0)p.cast=track(ash_wraith_motion::castKeys,std::clamp(action,0.f,1.f),false);
 if(paintedReady()){
  const bool turning=painted_motion::ready();
  const int a=turning?p.view.paintedLeft:p.view.paintedKey;
  const int b=turning?p.view.paintedRight:a;
  const float t=turning?p.view.paintedT:0;
  const auto point=[&](const auto& controls,int n){
   return lerp({controls[a][n][0],controls[a][n][1]},{controls[b][n][0],controls[b][n][1]},t);
  };
  for(int part=0;part<6;part++)p.pivot[part]=point(painted::pivots,part);
  for(int side=0;side<2;side++)p.palm[side]=point(painted::palms,side);
  return p;
 }
 const int k=p.view.sector,k2=(k+1)%8;const float t=p.view.t;
 for(int part=0;part<6;part++){
  const auto& a=ash_wraith_rig::pivots[k][part];const auto& b=ash_wraith_rig::pivots[k2][part];
  p.pivot[part]=lerp({a[0],a[1]},{b[0],b[1]},t);
 }
 for(int side=0;side<2;side++){
  const auto& a=ash_wraith_rig::palms[k][side];const auto& b=ash_wraith_rig::palms[k2][side];
  p.palm[side]=lerp({a[0],a[1]},{b[0],b[1]},t);
 }
 return p;
}
inline Vector2 suspended(const Pose& p,int layer,Vector2 uv){
 const auto& f=p.facing;
 const auto& root=p.drift[0];const auto& thrust=p.cast[0];
 Vector2 q{uv.x-64+root.x+f.x*thrust.x,uv.y-121-root.y-thrust.y+f.y*thrust.x*.52f};
 const int n=layer==3?1:layer<3?layer+2:layer+1;
 const auto drift=p.drift[n],cast=p.cast[n];
 const auto pivot=p.pivot[layer];
 float influence=1;
 if(layer==3)influence=1-enemy_death::window(uv.y,pivot.y-8,pivot.y+19);
 else if(layer<3)influence=enemy_death::window(uv.y,pivot.y-3,110);
 else {
  const auto hand=p.palm[layer-4];
  const float dx=hand.x-pivot.x,dy=hand.y-pivot.y;
  influence=enemy_death::window((uv.x-pivot.x)*dx+(uv.y-pivot.y)*dy,0,std::max(1.f,dx*dx+dy*dy));
 }
 const float trail=layer<3?7.f*p.travel:0.f;
 // The right sleeve throws downrange; the other sleeve folds across the chest.
 const float ahead=layer==4?0:cast.x;
 const float across=layer==4?cast.x:0;
 q.x+=influence*(drift.x+f.x*(ahead-trail)+f.y*across);
 q.y+=influence*(-drift.y-cast.y+.52f*(f.y*(ahead-trail)-f.x*across));
 return q;
}
inline Vector2 releaseAtHeading(float angle){
 if(caldera_puppets::ready(caldera_puppets::Wraith))return caldera_puppets::muzzleAtHeading(caldera_puppets::Wraith,angle);
 AnimationState st{};st.facingInitialized=true;st.facingAngle=angle;
 const auto p=samplePose(st,.45f);
 const auto hand=suspended(p,5,p.palm[1]);const float s=volcano_actors::bodyScale(Species);
 return {hand.x*s,hand.y*s};
}
}
