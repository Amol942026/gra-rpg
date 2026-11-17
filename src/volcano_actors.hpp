#pragma once
#include "generated/volcano_motion.hpp"
#include "generated/basalt_swing.hpp"
#include "generated/caldera_attack_motion.hpp"
#include "generated/fire_imp_fall_data.hpp"
#include "enemy_death_settle.hpp"
#include "volcano_corpse_material.hpp"
namespace varkhor_death_art {
inline void init();
inline void unload();
}
// The original cutouts retain their walk cycles. Special gestures add Blender pose curves.
namespace volcano_actors {
inline constexpr const char* names[]={"slag-crawler","lava-salamander","fire-imp","ash-wraith","obsidian-sentinel","ember-drake","varkhor"};
struct GroundPose {float lift=0;Rectangle footprint{};};
struct Atlas {
 Texture2D art{},silhouette{},relief{};
 std::vector<unsigned char> alpha;
 std::array<std::array<GroundPose,2>,8> ground{};
 int cell=128;
 bool authoredRest=false;
};
inline std::array<Atlas,7> atlases{};
inline void calibrateContact(Atlas& atlas,int species);
inline void load(){
 if(!volcano_corpse_material::shader.id)volcano_corpse_material::init();
 for(int i=0;i<7;i++){
 auto& a=atlases[i];if(a.art.id)continue;Image im=LoadImage((std::string("assets/volcano/")+names[i]+".png").c_str());if(!im.data)continue;
 if(im.width<8||im.width%8||im.height!=im.width/8*6){UnloadImage(im);continue;}
 ImageFormat(&im,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
 a.cell=im.width/8;a.art=LoadTextureFromImage(im);auto* pixels=static_cast<Color*>(im.data);a.alpha.resize(im.width*im.height);
 for(int n=0;n<im.width*im.height;n++){a.alpha[n]=pixels[n].a;pixels[n]={255,255,255,pixels[n].a};}
 calibrateContact(a,i);
 a.relief=volcano_corpse_material::loadPainted(names[i]);
 a.authoredRest=a.relief.id!=0;
 if(!a.authoredRest)a.relief=volcano_corpse_material::buildGuide(a.alpha,a.cell,im.width,im.height);
 a.silhouette=LoadTextureFromImage(im);UnloadImage(im);SetTextureFilter(a.art,TEXTURE_FILTER_BILINEAR);SetTextureFilter(a.silhouette,TEXTURE_FILTER_BILINEAR);
}
 varkhor_death_art::init();
}
inline void unload(){
 for(auto& a:atlases){
  if(a.art.id)UnloadTexture(a.art);
  if(a.silhouette.id)UnloadTexture(a.silhouette);
  if(a.relief.id)UnloadTexture(a.relief);
  a={};
 }
 volcano_corpse_material::unload();
 varkhor_death_art::unload();
}
struct FallStyle {
 float twist,dip,slide,compression;
 float buckle,tip,headLag,farLag,nearLag,headSnap;
 std::array<float,6> recoil,settle;
};
inline constexpr FallStyle falls[]={
 // The crawler loses its near claw, then draws the far pair under its shell.
 {9,7,4,.08f, .09f,.29f,.32f,.22f,.06f,14, {0,21,-27,-13,27,-29}, {0,-8,7,-3,-7,8}},
 // The salamander drops its snout first; the tail sweeps past the hips.
 {-11,10,9,.055f, .09f,.25f,.08f,.33f,.17f,27, {-12,-5,24,25,12,-16}, {5,3,-5,7,-3,4}},
 // The imp throws one arm out, folds its knees and then drops a shoulder.
 {-16,8,-4,.025f, .15f,.43f,.35f,.13f,.29f,19, {8,18,-20,-10,-32,27}, {-6,-15,13,9,19,-16}},
 // The hood tips down through spiraling sleeves and lower tatters.
 {23,6,-5,.085f, .16f,.39f,.23f,.43f,.27f,18, {22,12,-18,-32,24,-29}, {-14,-6,7,17,-9,10}},
 // The greaves give way before the shield edge and sword land on opposite sides.
 {-8,15,3,.09f, .04f,.34f,.32f,.27f,.18f,16, {-2,-34,38,10,-22,-19}, {0,8,-9,6,-9,11}},
 // The drake drops its breast with the near talons, then folds the high wing.
 {-8,8,-4,.025f, .09f,.25f,.14f,.37f,.13f,23, {-23,10,-16,-17,22,-25}, {12,-12,19,14,-14,16}},
 // Varkhor's heavy chest drops fast while the far arm and helmet follow its shoulders.
 {-9,21,-4,.085f, .03f,.23f,.25f,.24f,.10f,22, {-10,-25,42,20,-40,31}, {5,14,-19,13,22,-24}},
};
static_assert(std::size(falls)==std::size(names));
struct RestPart {float x,y,angle,scaleX=1,scaleY=1;};
struct RestPose {
 float turn,squash,pivotHeight,contactDepth;
 Color light;
 std::array<RestPart,6> parts;
};
inline constexpr RestPose restingPoses[]={
 // A curved shell keeps its thickness while the claws fold beneath its weight.
 {14,1.f,16,5,{246,239,227,255}, {{{0,4,-20,1.03f,1.06f},{7,-9,-48,.98f,.98f},{-1,-15,37,.99f,.96f},
    {-1,2,-11,1.025f,1.31f},{4,-10,44,1.01f,.98f},{-9,-13,-37,1.f,1.f}}}},
 // The chest rests on its flank. The far claws fold inward and the near ones splay.
 {16,1.f,18,5,{246,242,224,255}, {{{-4,4,-19,1.08f,1.04f},{5,-12,-43,1.02f,1.f},{-8,-16,44,1.01f,.98f},
    {0,0,-12,1.035f,1.37f},{3,-13,34,1.01f,1.f},{-6,-13,-41,1.01f,1.02f}}}},
 // Raised ribcage and one curled knee, with the other knee and the tail low.
 {76,.99f,30,9,{250,226,210,255}, {{{-4,2,29,1.07f,.98f},{-8,-5,-44,1.01f,.96f},{5,1,42,1.04f,.98f},
    {-2,1,-15,1.27f,1.03f},{0,-3,-33,1.08f,.98f},{-4,4,20,1.07f,.99f}}}},
 // The hollow hood and shoulders sit above two separately trailing folds.
 {72,1.00f,35,8,{227,238,252,255}, {{{-6,8,26,1.17f,1.00f},{-5,-8,6,1.01f,.96f},{7,-5,33,1.01f,.95f},
    {-4,3,-14,1.24f,1.06f},{-5,9,-24,1.05f,.96f},{7,6,34,1.06f,.97f}}}},
 // The raised breastplate bears against the far shoulder and bent greaves.
 {78,1.00f,36,9,{234,243,253,255}, {{{-7,2,12,1.11f,1.02f},{-9,-4,-39,1.03f,.93f},{8,-2,39,1.08f,.99f},
    {-4,2,-17,1.22f,1.06f},{9,-3,33,1.06f,.96f},{-11,9,-20,1.09f,1.03f}}}},
 // Only the wing membranes narrow as they fold around the rib cage and talons.
 {16,1.f,20,7,{247,233,218,255}, {{{3,-5,-10,1.01f,.68f},{3,-14,-38,1.01f,.98f},{-5,-16,36,1.01f,.98f},
    {0,0,-13,1.045f,1.38f},{4,-17,44,1.02f,.98f},{-6,-15,-41,1.02f,.98f}}}},
 // One fallen forearm and the far knee brace the bullhorn-shaped chest.
 {81,1.00f,42,9,{243,234,220,255}, {{{-7,6,15,1.12f,1.03f},{-10,-4,-24,1.05f,.97f},{8,-3,40,1.09f,.98f},
    {-4,2,-12,1.23f,1.07f},{4,-4,7,1.13f,1.05f},{-8,7,-31,1.13f,1.03f}}}},
};
static_assert(std::size(restingPoses)==std::size(names));

inline bool quadruped(int species){return species==0||species==1||species==5;}
inline Vector2 restPivot(int species,int part){
 if(part>=4&&!quadruped(species))return {64+(part==4?-1.f:1.f)*(species==6?25:18),67};
 return {64,90};
}

inline float bodyScale(int species){return species==6?1.85f:species==3?1.12f:species==4?1.1f:1.f;}
inline int flank(int direction,int impact){return impact?impact<0:direction>3;}

// Project the chest edge and the deepest extremity once, using the same
// transforms as the painted pose. The chest supports the weight while a wing,
// elbow or hoof can lie closer to the viewer on the isometric plane.
inline void calibrateContact(Atlas& atlas,int species){
 const auto& rest=restingPoses[species];
 const float pixel=128.f/atlas.cell;
 for(int direction=0;direction<8;direction++)for(int flank=0;flank<2;flank++){
  const float side=flank?-1.f:1.f;
  const float a=side*rest.turn*DEG2RAD,s=std::sin(a),c=std::cos(a);
  const float scale=bodyScale(species);
  constexpr int Span=384,Origin=Span/2;
  std::array<float,Span> underside;
  underside.fill(-1e6f);
  std::vector<float> contour;
  float extremity=-1e6f;
  for(int p=0;p<6;p++){
   const auto& part=rest.parts[p];
   const auto pivot=restPivot(species,p);
   const float b=(rest.turn+part.angle)*side*DEG2RAD;
   const float sb=std::sin(b),cb=std::cos(b);
   const float px=pivot.x-64+side*part.x,py=pivot.y-115+part.y+rest.pivotHeight;
   const float shiftX=c*px-s*py,shiftY=s*px+c*py;
   for(int row=0;row<atlas.cell;row++){
    const int start=(p*atlas.cell+row)*atlas.art.width+direction*atlas.cell;
    const float y=((row+.5f)*pixel-pivot.y)*part.scaleY;
    for(int i=0;i<atlas.cell;i++){
     const int column=sb<0?i:atlas.cell-1-i;
     if(atlas.alpha[start+column]<128)continue;
     const float x=((column+.5f)*pixel-pivot.x)*part.scaleX;
     const float screenY=rest.squash*(shiftY+sb*x+cb*y);
     extremity=std::max(extremity,screenY);
     if(p!=3)break; // A row's extreme sample is sufficient for a limb.
     const int slot=int(std::floor(shiftX+cb*x-sb*y))+Origin;
     if(slot>=0&&slot<Span)underside[slot]=std::max(underside[slot],screenY);
    }
   }
  }
  for(float y:underside)if(y>-1e5f)contour.push_back(y);
  if(contour.empty())continue;
  auto edge=contour.begin()+contour.size()*7/10;
  std::nth_element(contour.begin(),edge,contour.end());
  const float belly=*edge;
  const float lift=rest.contactDepth-std::max(belly,extremity-(species==5?17.f:12.f));
  int left=Span,right=-1,count=0;
  float sum=0;
  for(int x=0;x<Span;x++)if(underside[x]>=belly-8){
   left=std::min(left,x);right=std::max(right,x);
   sum+=std::min(underside[x],belly+3);++count;
  }
  const float width=float(right-left+1),radius=std::clamp(width*.15f,4.5f,9.f);
  const float centerY=lift+sum/count+2;
  atlas.ground[direction][flank]={lift,{(left-Origin-width*.015f)*scale,(centerY-radius)*scale,
                                      width*1.03f*scale,2*radius*scale}};
 }
}

inline float stagedFall(float t,float delay,float first){
 return first*enemy_death::window(t,0.f,delay)+(1-first)*enemy_death::window(t,delay,1.f);
}

inline fire_imp_fall::Key impFall(float phase){
 const float frame=phase*fire_imp_fall::ContactFrame;
 const int first=std::min(int(frame),fire_imp_fall::ContactFrame-1);
 const float mix=frame-first;
 const auto a=fire_imp_fall::keys[first],b=fire_imp_fall::keys[first+1];
 const auto sample=[mix](float start,float end){return start+(end-start)*mix;};
 return {sample(a.roll,b.roll),sample(a.sag,b.sag),sample(a.chest,b.chest),sample(a.nearKnee,b.nearKnee),
         sample(a.farKnee,b.farKnee),sample(a.nearArm,b.nearArm),sample(a.farArm,b.farArm),sample(a.tail,b.tail)};
}

struct BrokenPart {Vector2 offset{};float turn=0,visibility=1;};
using BrokenParts=std::array<BrokenPart,6>;

inline bool draw(int type,float x,float y,float size,const AnimationState& state,float action=-1,Color outline=BLANK,const Vector2* probe=nullptr,bool shadow=false,bool dead=false,bool flash=false,float special=-1,const enemy_death::Key* deathPose=nullptr,int deathScreenSide=0,float opacity=1,float deathPhase=0,const BrokenParts* broken=nullptr){
 int id=std::clamp(type-VolcanoFirstType,0,6),direction=std::clamp(int(state.direction),0,7);const auto& atlas=atlases[id];if(!atlas.art.id)return false;
 const float scale=bodyScale(id),s=size*scale;
 static constexpr float angles[]={0,.4636476f,1.5707963f,2.677945f,3.14159265f,3.6052403f,4.712389f,5.8195377f};float heading=angles[direction];
 const auto& fall=falls[id];
 const auto& rest=restingPoses[id];
 const float collapse=dead?1.f:deathPose?deathPose->collapse:0.f,recoil=deathPose?deathPose->recoil:0.f,settle=deathPose?deathPose->settle:0.f;
 const float forward=std::cos(heading),side=std::sin(heading);
 const float turn=deathScreenSide?float(deathScreenSide):direction>3?-1.f:1.f;
 const float t=std::clamp(deathPhase,0.f,1.f);
 const bool impFalling=type==FireImpType&&deathPose&&!dead;
 const auto imp=impFalling?impFall(t):fire_imp_fall::Key{};
 const float tip=dead?1.f:impFalling?imp.roll:stagedFall(t,fall.tip,.24f);
 const float knees=impFalling?imp.sag:enemy_death::window(t,fall.buckle,fall.tip+.12f)*(1-tip);
 const auto towardRest=[collapse](float current,float rest){return current*(1-collapse)+rest*collapse;};
 auto control=volcano_motion::sample(id,state.phase),opposite=volcano_motion::sample(id,std::fmod(state.phase+.5f,1.f));
 auto strike=volcano_motion::sample(id,std::max(0.f,action),true);
 float blend=dead?0:state.moveBlend;
 const bool fourLegs=quadruped(id);
 bool specialPose=id>=4&&special>=0&&!dead;
 auto gesture=specialPose?caldera_attack_motion::pose(id-4,special):caldera_attack_motion::Pose{};
 struct Part {Vector2 pivot,shift;float angle=0,scaleX=1,scaleY=1;};std::array<Part,6> parts{};
 for(int p=0;p<6;p++){
  auto& part=parts[p];part.pivot={64,90};
  if(p==1||p==2||(fourLegs&&p>=4)){auto k=p==1||p==5?control:opposite;float travel=k.travel*(VolcanoStrides[id]*.31f/scale)*blend;part.shift={std::cos(heading)*travel,std::sin(heading)*travel-k.lift*5*blend};}
  else{part.shift={control.sway*.45f*blend,-control.bob*1.2f*(.25f+.75f*blend)};}
  if(p>=4&&!fourLegs){part.pivot=restPivot(id,p);part.angle=control.sway*(p==4?-1:1)*4*blend;if(action>=0){part.angle+=strike.sway*(p==4?-2.5f:3.8f);part.shift.x+=strike.sway*std::cos(heading)*.55f;part.shift.y+=strike.sway*std::sin(heading)*.3f;}}
  if(fourLegs&&action>=0&&(p==3||p>=4)){part.shift.x+=strike.sway*std::cos(heading)*.3f;part.shift.y+=strike.sway*std::sin(heading)*.3f;}
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
  if(specialPose&&(p==0||p==3||(!fourLegs&&p>=4))){
   const float forward=std::cos(heading),side=std::sin(heading),mirror=forward<-.1f?-1.f:1.f;
   const float weight=p==3?1.f:.56f;
   part.shift.x+=forward*gesture.advance*weight;
   part.shift.y+=(side*gesture.advance*.44f-gesture.lift)*weight;
   part.angle+=mirror*gesture.pitch;
   if(p==3){part.pivot={64+forward*12,79};part.angle+=mirror*gesture.drive*.64f;}
   if(p>=4)part.angle+=mirror*gesture.drive*(p==4?-.36f:.72f);
  }
  if(dead||deathPose){
   const auto pivot=restPivot(id,p);
   const auto& limb=rest.parts[p];
   const bool far=turn>0?(p==1||p==4):(p==2||p==5);
   const float delay=p==3?fall.tip:p==0?fall.headLag:far?fall.farLag:fall.nearLag;
   const float fold=dead?1.f:stagedFall(t,delay,.16f);
   const auto join=[fold](float start,float rest){return start+(rest-start)*fold;};
   const float headNod=p==0&&!impFalling?fall.headSnap*enemy_death::window(t,delay,delay+.19f)*(1-fold):0;
   const bool leg=p==1||p==2||(p>=4&&fourLegs);
   const float buckle=leg?(impFalling?(far?imp.farKnee:imp.nearKnee):knees*(far?.65f:1.f)):0;
   const float follow=impFalling?(p==0?imp.tail:p==3?imp.chest:p>=4?(far?imp.farArm:imp.nearArm):0.f):
                                     fall.recoil[p]*recoil*(1-fold)+fall.settle[p]*settle+headNod;
   part.pivot={join(part.pivot.x,pivot.x),join(part.pivot.y,pivot.y)};
   part.shift={join(part.shift.x,turn*limb.x)+(follow*.13f+7*buckle)*forward,
               join(part.shift.y,limb.y)+std::abs(headNod)*.19f-std::abs(follow)*.10f-8*buckle};
   part.angle=join(part.angle,turn*limb.angle)+turn*(follow+(far?19.f:-24.f)*buckle);
   part.scaleX=join(1,limb.scaleX);part.scaleY=join(1,limb.scaleY);
   if(impFalling&&p>=4){
    // Pivot the shoulder with the chest, then add the arm's own delayed swing.
    const float angle=turn*imp.chest*DEG2RAD;
    const Vector2 shoulder{part.pivot.x-64,part.pivot.y-90};
    part.shift.x+=shoulder.x*(std::cos(angle)-1)-shoulder.y*std::sin(angle);
    part.shift.y+=shoulder.x*std::sin(angle)+shoulder.y*(std::cos(angle)-1);
   }
  }
  if(broken){
   part.shift.x+=(*broken)[p].offset.x;
   part.shift.y+=(*broken)[p].offset.y;
   part.angle+=(*broken)[p].turn;
  }
 }
 const float fallAngle=turn*(rest.turn*tip+(impFalling?0.f:fall.twist*recoil*(1-tip)));
 const float fallOffset=rest.pivotHeight*s*tip;
 const float squash=towardRest(1,rest.squash)-fall.compression*knees;
 const float slide=fall.slide*(recoil+settle*.25f)*s;
 const float groundedY=atlas.ground[direction][turn<0].lift;
 const float shiftX=turn*slide,shiftY=side*slide*.45f+(fall.dip*knees+groundedY*tip)*s;
 Vector2 localProbe{};
 if(probe){
  localProbe={probe->x-x-shiftX,(probe->y-y-shiftY)/squash};
  const float r=-fallAngle*DEG2RAD;
  localProbe={localProbe.x*std::cos(r)-localProbe.y*std::sin(r),localProbe.x*std::sin(r)+localProbe.y*std::cos(r)-fallOffset};
  for(int p=5;p>=0;p--){
   if(broken&&(*broken)[p].visibility<=0)continue;
   const auto& k=parts[p];Vector2 q{localProbe.x/s-(k.pivot.x-64+k.shift.x),localProbe.y/s-(k.pivot.y-115+k.shift.y)};float r=-k.angle*DEG2RAD;
   float u=(q.x*std::cos(r)-q.y*std::sin(r))/k.scaleX+k.pivot.x;
   float v=(q.x*std::sin(r)+q.y*std::cos(r))/k.scaleY+k.pivot.y;
   int px=int(std::floor(u*atlas.cell/128)),py=int(std::floor(v*atlas.cell/128));
   if(px>=0&&px<atlas.cell&&py>=0&&py<atlas.cell&&atlas.alpha[(p*atlas.cell+py)*atlas.art.width+direction*atlas.cell+px]>=100)return true;
  }return false;
 }
 rlPushMatrix();
 if(shadow){float matrix[]={1,0,0,0,-shadow_math::LightX,-shadow_math::LightY,0,0,0,0,1,0,shadow_math::LightX*y,(1+shadow_math::LightY)*y,0,1};rlMultMatrixf(matrix);}
 rlTranslatef(x+shiftX,y+shiftY,0);
 if(dead||deathPose){rlScalef(1,squash,1);rlRotatef(fallAngle,0,0,1);rlTranslatef(0,fallOffset,0);}
 auto paint=[&](int p,Texture2D texture,Color tint,float dx=0,float dy=0){auto k=parts[p];Rectangle source{float(direction*atlas.cell),float(p*atlas.cell),float(atlas.cell),float(atlas.cell)};
  if(broken)tint.a=static_cast<unsigned char>(std::lround(tint.a*(*broken)[p].visibility));
  if(!tint.a)return;
  DrawTexturePro(texture,source,{(k.pivot.x-64+k.shift.x)*s+dx,(k.pivot.y-115+k.shift.y)*s+dy,128*s*k.scaleX,128*s*k.scaleY},
                 {k.pivot.x*s*k.scaleX,k.pivot.y*s*k.scaleY},k.angle,tint);
 };
 int frontOrder[]={0,1,2,3,4,5},backOrder[]={0,1,2,4,5,3},raisedSwordOrder[]={0,1,2,5,3,4};
 // A raised sword passes behind the helmet during anticipation; the shield
 // remains in front. The forward cut returns the blade to the foreground.
 const int* order=direction>=5?backOrder:id==4&&collapse<.64f&&action>=.08f&&action<.53f?raisedSwordOrder:frontOrder;
 if(!shadow&&outline.a)for(Vector2 delta:{Vector2{-1.5f,0},Vector2{1.5f,0},Vector2{0,-1.5f},Vector2{0,1.5f}})for(int n=0;n<6;n++)paint(order[n],atlas.silhouette,outline,delta.x,delta.y);
 const bool relit=!shadow&&volcano_corpse_material::begin(atlas.relief,enemy_death::ease(collapse),atlas.authoredRest);
 for(int n=0;n<6;n++){
  const int p=order[n];
  const bool farLimb=turn>0?(p==1||p==4):(p==2||p==5);
  const float light=p==3?1.f:p==0?.88f:farLimb?.78f:.98f;
  Color tint=shadow?WHITE:!collapse?WHITE:rgb(int(std::lround(towardRest(255,rest.light.r*light))),
      int(std::lround(towardRest(255,rest.light.g*light))),int(std::lround(towardRest(255,rest.light.b*light))));
  tint.a=static_cast<unsigned char>(std::clamp(opacity,0.f,1.f)*255+.5f);
  paint(p,shadow?atlas.silhouette:atlas.art,tint);
 }
 if(relit)EndShaderMode();
 if(flash&&!shadow)for(int n=0;n<6;n++)paint(order[n],atlas.silhouette,Fade(WHITE,.48f));
 rlPopMatrix();return false;
}
}

#include "wraith_death_art.hpp"
#include "sentinel_death_art.hpp"
#include "varkhor_death_art.hpp"

namespace volcano_actors {
using DeathDraw=bool(*)(const Enemy&,float,float,float,Color,const Vector2*,bool,float);
inline DeathDraw specialDeathArt(int type){
 switch(type){
 case AshWraithType:return wraith_death_art::draw;
 case ObsidianSentinelType:return sentinel_death_art::draw;
 case VolcanoBossType:return varkhor_death_art::ash.id?varkhor_death_art::draw:nullptr;
 default:return nullptr;
 }
}
inline Rectangle groundFootprint(const Enemy& e){
 switch(e.type){
 case AshWraithType:return wraith_death_art::footprint(e);
 case ObsidianSentinelType:return sentinel_death_art::footprint(e);
 case VolcanoBossType:if(varkhor_death_art::ash.id)return varkhor_death_art::footprint(e);break;
 default:break;
 }
 const int species=e.type-VolcanoFirstType,direction=int(e.animation.direction);
 return atlases[species].ground[direction][flank(direction,e.deathScreenSide())].footprint;
}
inline bool rest(const Enemy& e,float x,float y,Color outline=BLANK,const Vector2* probe=nullptr,bool shadow=false,float opacity=1){
 if(auto render=specialDeathArt(e.type))return render(e,x,y,1,outline,probe,shadow,opacity);
 return draw(e.type,x,y,1,e.animation,-1,outline,probe,shadow,true,false,-1,nullptr,e.deathScreenSide(),opacity);
}
inline bool death(const Enemy& e,float x,float y,float progress,Color outline=BLANK,const Vector2* probe=nullptr,bool shadow=false,float opacity=1){
 const float contact=enemy_death::groundContact(e.type);
 if(progress>=contact)return rest(e,x,y,outline,probe,shadow,opacity);
 if(auto render=specialDeathArt(e.type)){
  if(progress<=0)return draw(e.type,x,y,1,e.deathMotion,e.deathAction,outline,probe,shadow,false,false,e.deathSpecial,nullptr,0,opacity);
  return render(e,x,y,progress/contact,outline,probe,shadow,opacity);
 }
 auto pose=enemy_death::sample(e.type,progress);
 // Ease the layered sprites into their final angles as the body reaches the ground.
 const float motion=1-enemy_death::landingFit(e.type,progress);
 pose.collapse=1-(1-pose.collapse)*motion;
 pose.recoil*=motion;
 pose.settle*=motion;
 return draw(e.type,x,y,1,e.deathMotion,e.deathAction,outline,probe,shadow,false,false,e.deathSpecial,&pose,e.deathScreenSide(),opacity,progress/contact);
}
}
