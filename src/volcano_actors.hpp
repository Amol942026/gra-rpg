#pragma once
#include <optional>
#include "generated/ember_drake_turn_atlas.hpp"
#include "generated/slag_crawler_turn_atlas.hpp"
#include "generated/drake_direction_cels.hpp"
#include "generated/crawler_direction_cels.hpp"
#include "generated/fire_imp_turn_atlas.hpp"
#include "generated/fire_imp_cels.hpp"
#include "generated/obsidian_sentinel_turn_atlas.hpp"
#include "generated/cinderhorn_turn_atlas.hpp"
#include "generated/cinderhorn_synfig.hpp"
#include "generated/cinderhorn_fall_synfig.hpp"
#include "generated/volcano_motion.hpp"
#include "generated/varkhor_body_motion.hpp"
#include "generated/basalt_swing.hpp"
#include "generated/caldera_attack_motion.hpp"
#include "generated/ember_drake_motion.hpp"
#include "generated/slag_crawler_motion.hpp"
#include "generated/fire_imp_fall_data.hpp"
#include "generated/fire_imp_motion_data.hpp"
#include "generated/fire_imp_rig.hpp"
#include "generated/obsidian_sentinel_motion.hpp"
#include "enemy_death_settle.hpp"
#include "enemy_arm.hpp"
#include "volcano_corpse_material.hpp"
#include "ember_drake_mouth.hpp"
#include "painted_turn_shape.hpp"
#include "caldera_puppets.hpp"
#include "caldera_quadruped.hpp"
namespace varkhor_death_art {
inline void init();
inline void unload();
}
namespace volcano_actors::turn_art {inline void load();inline void unload();}
namespace volcano_actors::cinderhorn_turn {inline void load();inline void unload();}
namespace volcano_actors::salamander_art {inline void load();inline void unload();}
namespace actor_art {inline Vector2 reserveSpriteDepth();}
extern "C" void (*glfwGetProcAddress(const char*))(void);
namespace wraith_death_art {
inline void load();
inline void unload();
inline bool living(float,float,float,const AnimationState&,float,Color,const Vector2*,bool,bool,float);
}
// The original cutouts retain their walk cycles. Special gestures add Blender pose curves.
namespace volcano_actors {
inline constexpr const char* names[]={"slag-crawler","lava-salamander","fire-imp","ash-wraith","obsidian-sentinel","ember-drake","varkhor"};
inline constexpr float facingAngles[]={0,.4636476f,1.5707963f,2.677945f,3.14159265f,3.6052403f,4.712389f,5.8195377f};
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
 drake_mouth::load();
 caldera_puppets::load();
 caldera_quadruped::load();
 cinderhorn_turn::load();
 salamander_art::load();
 turn_art::load();
 if(!caldera_puppets::ready(caldera_puppets::Wraith))wraith_death_art::load();
}
inline void unload(){
 for(auto& a:atlases){
  if(a.art.id)UnloadTexture(a.art);
  if(a.silhouette.id)UnloadTexture(a.silhouette);
  if(a.relief.id)UnloadTexture(a.relief);
  a={};
 }
 volcano_corpse_material::unload();
 drake_mouth::unload();
 caldera_puppets::unload();
 caldera_quadruped::unload();
 cinderhorn_turn::unload();
 salamander_art::unload();
 turn_art::unload();
 wraith_death_art::unload();
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
 // The six drake rest transforms are the final authored Synfig fall key.
 {16,1.f,20,7,{247,233,218,255}, {}},
 // One fallen forearm and the far knee brace the bullhorn-shaped chest.
 {81,1.00f,42,9,{243,234,220,255}, {{{-7,6,15,1.12f,1.03f},{-10,-4,-24,1.05f,.97f},{8,-3,40,1.09f,.98f},
    {-4,2,-12,1.23f,1.07f},{4,-4,7,1.13f,1.05f},{-8,7,-31,1.13f,1.03f}}}},
};
static_assert(std::size(restingPoses)==std::size(names));

inline bool quadruped(int species){return species==0||species==1||species==5;}
inline Vector2 restPivot(int species,int part,int direction=0){
 if(species==SlagCrawlerType-VolcanoFirstType){
  const auto& xy=slag_crawler_motion::pivots[direction][part];
  return {xy[0],xy[1]};
 }
 if(species==EmberDrakeType-VolcanoFirstType){
  const auto& xy=ember_drake_motion::pivots[direction][part];
  return {xy[0],xy[1]};
 }
 if(species==FireImpType-VolcanoFirstType){
  const auto& xy=fire_imp_rig::pivots[direction][part];
  return {xy[0],xy[1]};
 }
 if(species==ObsidianSentinelType-VolcanoFirstType){
  const auto& xy=obsidian_sentinel_motion::pivots[direction][part];
  return {xy[0],xy[1]};
 }
 if(part>=4&&!quadruped(species))return {64+(part==4?-1.f:1.f)*(species==6?25:18),67};
 return {64,90};
}

inline float bodyScale(int species){return species==6?1.85f:species==5?1.15f:species==3?1.12f:species==4?1.1f:1.f;}
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
   auto part=rest.parts[p];
   if(species==EmberDrakeType-VolcanoFirstType){
    const auto& clip=ember_drake_motion::fall;
    const auto& key=clip.frames[clip.count-1][direction][p];
    part={key.x/128.f,key.y/128.f,key.turn/128.f,1+key.scaleX/4096.f,1+key.scaleY/4096.f};
   }
   const auto pivot=restPivot(species,p,direction);
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

inline fire_imp_motion::Key impMotion(const fire_imp_motion::Key* keys,float phase){
 const float frame=std::clamp(phase,0.f,1.f)*60;
 const int first=std::min(int(frame),59);
 const float u=frame-first;
 const auto& a=keys[first];const auto& b=keys[first+1];
 const auto sample=[u](float start,float end){return start+(end-start)*u;};
 return {sample(a.travel,b.travel),sample(a.lift,b.lift),sample(a.rise,b.rise),sample(a.tilt,b.tilt),
         sample(a.tail,b.tail),sample(a.guard,b.guard),sample(a.cast,b.cast),sample(a.reach,b.reach),sample(a.ember,b.ember)};
}

struct Part {Vector2 pivot,shift;float angle=0,scaleX=1,scaleY=1,elbow=0;};
#include "varkhor_punch_arm.hpp"
inline varkhor_motion::Key bossMotion(float phase,const varkhor_motion::Key (&clip)[61]){
 const float frame=std::clamp(phase,0.f,1.f)*60;
 const int row=std::min(59,int(frame));
 const float t=frame-row;
 const auto& a=clip[row];const auto& b=clip[row+1];
 const auto blend=[t](float x,float y){return x+(y-x)*t;};
 return {blend(a.advance,b.advance),blend(a.drop,b.drop),blend(a.turn,b.turn),
         blend(a.left,b.left),blend(a.right,b.right),blend(a.handReach,b.handReach),blend(a.handLift,b.handLift),
         blend(a.shoulder,b.shoulder),blend(a.shoulderLift,b.shoulderLift),blend(a.lateralRaise,b.lateralRaise)};
}
inline void bossPlateRoot(Part& part,int plate,float forward,float side,varkhor_motion::Key k,float weight){
 if(plate==1||plate==2)return;
 const float turn=k.turn*(.62f+.38f*std::abs(forward))*weight;
 const float twist=turn*DEG2RAD;
 const Vector2 spine{64,86};
 if(plate>=4)part.pivot={plate==4?37.f:90.f,66.5f};
 else part.pivot=spine;
 const Vector2 fromSpine{part.pivot.x-spine.x,part.pivot.y-spine.y};
 part.shift={forward*k.advance*weight+(std::cos(twist)-1)*fromSpine.x-std::sin(twist)*fromSpine.y,
             (side*k.advance*.44f+k.drop)*weight+std::sin(twist)*fromSpine.x+(std::cos(twist)-1)*fromSpine.y};
 part.angle=turn;
}
inline void driveBossShoulder(Part& part,int plate,float forward,float side,varkhor_motion::Key k){
 const float flank=plate==4?-1.f:1.f;
 const float drive=varkhor_punch::frontView(forward)*(side>=0?1.f:.50f)*
                   enemy_death::window(k.handReach,.35f,1.f);
 part.shift.x+=forward*k.shoulder-flank*12.5f*drive;
 part.shift.y+=side*k.shoulder*.44f-k.shoulderLift+4.5f*drive;
}
inline Vector2 bossPunchEndpoint(int plate,float forward,float side,Vector2 target){
 constexpr auto impact=varkhor_motion::strike[27];
 Part root;
 bossPlateRoot(root,plate,forward,side,impact,1.f);
 driveBossShoulder(root,plate,forward,side,impact);
 const Vector2 local{target.x-(root.pivot.x-64+root.shift.x),target.y-(root.pivot.y-115+root.shift.y)};
 return varkhor_punch::Arm::rotate(local,-root.angle*DEG2RAD);
}
inline void poseBossPlate(Part& part,int plate,float forward,float side,varkhor_motion::Key k,float weight,int punch,
                          const Vector2* contact=nullptr){
 bossPlateRoot(part,plate,forward,side,k,weight);
 if(plate<4)return;
 if(punch>=4){
  if(plate==punch){
   driveBossShoulder(part,plate,forward,side,k);
   const auto joints=varkhor_punch::followFist(plate,forward,side,k.handReach,k.handLift,k.lateralRaise,contact);
   part.angle+=joints.upper;
   part.elbow=joints.lower;
  }else part.angle+=(punch==5?1.f:-1.f)*k.left;
 }else{
  const float profile=std::clamp(std::abs(forward)*2.6f,0.f,1.f);
  part.angle+=(plate==4?k.left:k.right)*(.72f+.28f*profile)*weight;
 }
}
inline std::optional<varkhor_punch::Target> punchTarget(const Enemy& e){
 if(e.type!=VolcanoBossType||!e.varkhorPunch.arm)return {};
 const auto target=e.varkhorPunch.target;
 const auto delta=target-e.pos;
 const float elevation=terrain_height::pixels(game.location,target)-terrain_height::pixels(game.location,e.pos);
 return varkhor_punch::Target{{(delta.x-delta.y)*40,(delta.x+delta.y)*20-elevation-72},e.varkhorPunch.arm};
}
#include "volcano_turn.hpp"
#include "cinderhorn_turn_pose.hpp"
#include "lava_salamander_art.hpp"
#include "ember_drake_turn.hpp"
#include "ember_drake_turn_gesture.hpp"
#include "ember_drake_pose.hpp"
#include "slag_crawler_pose.hpp"
#include "obsidian_sentinel_pose.hpp"
struct ImpAnimation {fire_imp_motion::Key idle,walk,opposite,cast;};

inline ImpAnimation impSample(const AnimationState& state,float action){
 const float clock=state.idleTime/1.42f;
 return {impMotion(fire_imp_motion::idle,clock-std::floor(clock)),
         impMotion(fire_imp_motion::walk,state.phase),
         impMotion(fire_imp_motion::walk,std::fmod(state.phase+.5f,1.f)),
         action>=0?impMotion(fire_imp_motion::cast,action):fire_imp_motion::Key{}};
}
inline Part impPart(int part,int direction,float forward,float across,float blend,const ImpAnimation& motion){
 const float sign=part==1||part==4?-1.f:1.f;
 const float facing=forward<-.05f?-1.f:1.f;
 const auto& [idle,walk,opposite,cast]=motion;
 const float bodyTilt=((walk.tilt*blend+idle.tilt*(1-blend))*(1-cast.guard)+cast.tilt)*forward;
 const float rise=(walk.rise*blend+idle.rise*(1-blend))*(1-cast.guard)+cast.rise;
 const Vector2 bodyShift{forward*cast.reach*.72f,across*cast.reach*.32f-rise};
 Part pose{restPivot(FireImpType-VolcanoFirstType,part,direction),bodyShift,bodyTilt};
 if(part==1||part==2){
  // The stance segment moves linearly backwards by the actual projected stride.
  const auto& foot=part==1?walk:opposite;
  const float travel=foot.travel*blend;
  pose.shift={forward*travel,across*travel-foot.lift*blend};
  pose.angle=sign*foot.lift*1.55f*blend*forward;
 }else if(part==0){
  pose.angle+=facing*(idle.tail*(1-blend)+walk.tail*blend+cast.tail);
 }else if(part>=4){
  const float rotation=bodyTilt*DEG2RAD;
  const auto torso=restPivot(FireImpType-VolcanoFirstType,3,direction);
  const Vector2 pivotDelta{pose.pivot.x-torso.x,pose.pivot.y-torso.y};
  pose.shift.x+=(std::cos(rotation)-1)*pivotDelta.x-std::sin(rotation)*pivotDelta.y;
  pose.shift.y+=std::sin(rotation)*pivotDelta.x+(std::cos(rotation)-1)*pivotDelta.y;
  const float swing=part==4?walk.guard*blend+idle.guard*(1-blend):walk.cast*blend+idle.cast*(1-blend);
  pose.angle+=forward*swing;
  pose.scaleY=std::clamp(1+across*swing*.005f,.65f,1.18f);
  if(cast.guard>0){
   // Aim each painted palm at its own side of the orb, rotating about the
   // shoulder. Foreshortening is along the forearm; the shoulder stays seated.
   const auto& right=fire_imp_rig::palm[direction];
   const auto& left=fire_imp_rig::guardPalm[direction];
   const auto& palm=part==4?left:right;
   const auto& shoulders=fire_imp_rig::pivots[direction];
   Vector2 center{(shoulders[4][0]+shoulders[5][0])*.5f+bodyShift.x+forward*(17+12*cast.cast),
                  (shoulders[4][1]+shoulders[5][1])*.5f+bodyShift.y+14+across*(8+8*cast.cast)-5*cast.cast};
   const float rear=std::max(0.f,-across);
   // A rear-facing caster cups the charge past one shoulder so its head
   // does not cover the hands, and thrusts forward at chest height.
   center.x+=15*rear*rear;center.y+=rear*rear*(11+10*cast.cast);
   const float horizontal=right[0]>=left[0]?1.f:-1.f;
   const Vector2 spacing{horizontal*(5+6*std::abs(across)),forward*8+across*1.5f};
   const Vector2 target{center.x+sign*spacing.x,center.y+sign*spacing.y};
   const Vector2 aim{target.x-pose.pivot.x-pose.shift.x,target.y-pose.pivot.y-pose.shift.y};
   Vector2 posedPalm{palm[0],palm[1]};
   if(turn_art::imp.ready()&&turn_art::filtered()){
    auto elbow=turn_art::impElbow({direction,4*direction,0,0},part,0);
    const Vector2 upper{elbow.center.x-pose.pivot.x,elbow.center.y-pose.pivot.y};
    const Vector2 lower{palm[0]-elbow.center.x,palm[1]-elbow.center.y};
    const float a=std::hypot(upper.x,upper.y),b=std::hypot(lower.x,lower.y),distance=std::hypot(aim.x,aim.y);
    const float rest=std::atan2(upper.x*lower.y-upper.y*lower.x,upper.x*lower.x+upper.y*lower.y);
    const float bend=std::acos(std::clamp((distance*distance-a*a-b*b)/(2*a*b),-1.f,1.f));
    pose.elbow=std::clamp((std::copysign(bend,rest)-rest)*RAD2DEG,-56.f,56.f)*cast.guard;
    elbow.angle=pose.elbow*DEG2RAD;
    posedPalm=elbow.map(posedPalm,false);
   }
   const Vector2 arm{posedPalm.x-pose.pivot.x,posedPalm.y-pose.pivot.y};
   const float reach=std::clamp(std::hypot(aim.x,aim.y)/std::hypot(arm.x,arm.y),.91f,1.07f);
   const float wanted=(std::atan2(aim.y,aim.x)-std::atan2(arm.y*reach,arm.x))*RAD2DEG;
   pose.angle+=std::remainder(wanted-pose.angle,360.f)*cast.guard;
   pose.scaleY+=(reach-pose.scaleY)*cast.guard;
  }
 }
 return pose;
}
inline Part impViewPart(int layer,const turn_art::View& view,float blend,const ImpAnimation& motion){
 const auto at=[&](int d){return impPart(layer,d,std::cos(facingAngles[d]),std::sin(facingAngles[d]),blend,motion);};
 const Part first=at(view.base);
 if(view.t==0)return first;
 const Part next=at(view.nextBase());
 auto pose=turn_art::interpolate(first,next,view.t);
 pose.angle=first.angle+std::remainder(next.angle-first.angle,360.f)*view.t;
 return pose;
}
inline Vector2 pointOnPart(const Part& hand,const turn_art::View& view,const float (&points)[8][2],int impArm=-1){
 const auto& a=points[view.base];const auto& b=points[view.nextBase()];
 Vector2 local{turn_art::mix(a[0],b[0],view.t),turn_art::mix(a[1],b[1],view.t)};
 if(hand.elbow!=0){
  const auto joint=impArm>=4?turn_art::impElbow(view,impArm,hand.elbow):turn_art::swordJoint(view,hand.elbow);
  local=joint.map(local,false);
 }
 const float localX=(local.x-hand.pivot.x)*hand.scaleX;
 const float localY=(local.y-hand.pivot.y)*hand.scaleY;
 const float r=hand.angle*DEG2RAD,c=std::cos(r),s=std::sin(r);
 return {hand.pivot.x-64+hand.shift.x+localX*c-localY*s,hand.pivot.y-115+hand.shift.y+localX*s+localY*c};
}
inline Vector2 impPalm(const Part& hand,const turn_art::View& view){return pointOnPart(hand,view,fire_imp_rig::palm,5);}
inline Vector2 sentinelBladePoint(const Enemy& e){
 const auto* art=turn_art::forType(e.type);
 const int d=int(e.animation.direction);
 const auto view=art?turn_art::view(e.animation,*art):turn_art::View{d,d*4,0,0};
 const float special=game.volcanoSpecialPose(e);
 const auto arm=[&](int key){return sentinel_art::livingPart(sentinel_art::sample(e.animation,e.volcanoAction(),special,key),5);};
 auto posed=arm(view.base);
 if(view.t>0)posed=turn_art::interpolate(posed,arm(view.nextBase()),view.t);
 const auto local=pointOnPart(posed,view,obsidian_sentinel_motion::bladeGlow);
 const float scale=bodyScale(ObsidianSentinelType-VolcanoFirstType);
 return {local.x*scale,local.y*scale};
}
struct ImpHands {Vector2 left,right,center;};
inline ImpHands impHands(const Part& left,const Part& right,const turn_art::View& view){
 const auto a=pointOnPart(left,view,fire_imp_rig::guardPalm,4);
 const auto b=impPalm(right,view);
 return {a,b,{(a.x+b.x)*.5f,(a.y+b.y)*.5f}};
}
inline ImpHands impCastHands(const Enemy& owner,float phase){
 const auto& state=owner.alive?owner.animation:owner.deathMotion;
 if(caldera_puppets::ready(caldera_puppets::Imp)){
  const auto pose=caldera_puppets::fireImp(state,phase);
  return {pose.palms[0],pose.palms[1],caldera_puppets::mix(pose.palms[0],pose.palms[1],.5f)};
 }
 const int d=int(state.direction);
 const auto* art=turn_art::forType(FireImpType);
 const auto view=art?turn_art::view(state,*art):turn_art::View{d,d*4,0,0};
 const auto motion=impSample(state,phase);
 return impHands(impViewPart(4,view,state.moveBlend,motion),impViewPart(5,view,state.moveBlend,motion),view);
}
inline Vector2 impReleasePoint(const Enemy& owner){return impCastHands(owner,.45f).center;}
inline Vector2 impMuzzleAtHeading(float angle){
 if(caldera_puppets::ready(caldera_puppets::Imp))return caldera_puppets::muzzleAtHeading(caldera_puppets::Imp,angle);
 AnimationState st{};st.direction=directionAtScreenAngle(angle);st.facingInitialized=true;st.facingAngle=angle;
 const auto* art=turn_art::forType(FireImpType);
 const int d=int(st.direction);
 const auto view=art?turn_art::view(st,*art):turn_art::View{d,d*4,0,0};
 const auto motion=impSample(st,.45f);
 return impHands(impViewPart(4,view,0,motion),impViewPart(5,view,0,motion),view).center;
}

inline bool puppetAlive(caldera_puppets::Species species,float x,float y,float size,const AnimationState& state,
                        float action,Color outline,const Vector2* probe,bool shadow,bool flash,float opacity){
 const auto pose=caldera_puppets::living(species,state,action);
 const bool hit=caldera_puppets::draw(species,pose,x,y,size,outline,probe,shadow,flash,opacity);
 if(!probe&&!shadow&&action>0&&action<.45f){
  const auto hand=species==caldera_puppets::Imp?caldera_puppets::mix(pose.palms[0],pose.palms[1],.5f):pose.palms[1];
  const Vector2 anchor{x+size*hand.x,y+size*hand.y};
  const float flame=enemy_death::window(action,.06f,.39f)*opacity;
  if(species==caldera_puppets::Imp)
   caldera_attack_art::fireOrb(anchor,.91f*size*caldera_attack_art::heldOrbScale,flame,action/.45f,0);
  else if(flame>0){
   DrawCircleV(anchor,5.6f*size*flame,Fade({255,117,50,255},.26f*opacity));
   DrawCircleV(anchor,2.7f*size*flame,Fade({255,184,87,255},.92f*opacity));
  }
 }
 return hit;
}

inline bool draw(int type,float x,float y,float size,const AnimationState& state,float action=-1,Color outline=BLANK,const Vector2* probe=nullptr,bool shadow=false,bool dead=false,bool flash=false,float special=-1,const enemy_death::Key* deathPose=nullptr,int deathScreenSide=0,float opacity=1,float deathPhase=0,const BrokenParts* broken=nullptr,std::optional<varkhor_punch::Target> target={}){
 if(type==LavaSalamanderType&&salamander_art::ready())
  return salamander_art::draw(x,y,size,state,action,dead?1.f:deathPose?deathPhase:0.f,
                             deathScreenSide,outline,probe,shadow,flash,opacity);
 if(caldera_quadruped::handles(type)){
  const float fall=dead?1.f:deathPose?deathPhase:0.f;
  return caldera_quadruped::draw(type,x,y,size,state,action,special,fall,deathScreenSide,outline,probe,shadow,flash,opacity);
 }
 if(type==FireImpType||type==AshWraithType){
  const auto species=type==FireImpType?caldera_puppets::Imp:caldera_puppets::Wraith;
  if(caldera_puppets::ready(species)){
   if(!dead&&!deathPose)return puppetAlive(species,x,y,size,state,action,outline,probe,shadow,flash,opacity);
   const float side=deathScreenSide?float(deathScreenSide):int(state.direction)>3?-1.f:1.f;
   const auto pose=caldera_puppets::fall(species,state,action,dead?1.f:deathPhase,side);
   return caldera_puppets::draw(species,pose,x,y,size,outline,probe,shadow,false,opacity);
  }
 }
 int id=std::clamp(type-VolcanoFirstType,0,6),direction=std::clamp(int(state.direction),0,7);const auto& atlas=atlases[id];if(!atlas.art.id)return false;
 if(type==AshWraithType&&!dead&&!deathPose)return wraith_death_art::living(x,y,size,state,action,outline,probe,shadow,flash,opacity);
 const float scale=bodyScale(id),s=size*scale;
 const bool drake=type==EmberDrakeType;
 const auto* artwork=turn_art::forType(type);
 const bool directionalDrake=drake&&artwork;
 const bool directionalCrawler=type==SlagCrawlerType&&artwork;
 const auto view=artwork?turn_art::view(state,*artwork):turn_art::View{direction,direction*4,0,0};
 if(artwork)direction=view.nearestBase();
 const bool varkhorMelee=type==VolcanoBossType&&action>=0&&special<0&&!dead&&!deathPose;
 const float heading=varkhorMelee&&state.facingInitialized?state.targetAngle:turn_art::heading(view);
 const float heightScale=directionalDrake?turn_art::drakeHeightScale(view):1.f;
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
 const bool fireImp=type==FireImpType;
 const auto drakePose=drake?drake_art::sample(state,action,special,view.base):drake_art::Motion{};
 const auto nextPose=directionalDrake?drake_art::sample(state,action,special,view.nextBase()):drake_art::Motion{};
 const auto crawlerPose=directionalCrawler?crawler_art::sample(state,action,view.base):crawler_art::Motion{};
 const auto nextCrawler=directionalCrawler?crawler_art::sample(state,action,view.nextBase()):crawler_art::Motion{};
 const int jaw=drake&&(directionalDrake||(drake_mouth::art.id&&direction<drake_mouth::Views))&&!dead?
     int(std::lround(drakePose.gape*drake_mouth::Levels*(deathPose?1-enemy_death::ease(t):1))):0;
 const auto drakeFall=drake&&(dead||deathPose)?drake_art::locate(ember_drake_motion::fall,dead?1.f:t,view.base):drake_art::Bracket{};
 const auto nextFall=directionalDrake&&(dead||deathPose)?drake_art::locate(ember_drake_motion::fall,dead?1.f:t,view.nextBase()):drake_art::Bracket{};
 const float halfPhase=std::fmod(state.phase+.5f,1.f);
 const bool warrior=type==ObsidianSentinelType;
 const auto sentinelPose=warrior?sentinel_art::sample(state,action,special,view.base):sentinel_art::Motion{};
 const auto nextSentinel=warrior&&view.t>0?sentinel_art::sample(state,action,special,view.nextBase()):sentinel_art::Motion{};
 const bool legacyMotion=!fireImp&&!warrior&&!drake&&!directionalCrawler;
 const auto control=legacyMotion?volcano_motion::sample(id,state.phase):volcano_motion::Key{};
 const auto opposite=legacyMotion?volcano_motion::sample(id,halfPhase):volcano_motion::Key{};
 const auto strike=legacyMotion?volcano_motion::sample(id,std::max(0.f,action),true):volcano_motion::Key{};
 const auto impAnimation=fireImp?impSample(state,action):ImpAnimation{};
 float blend=dead?0:state.moveBlend;
 const bool fourLegs=quadruped(id);
 const bool boss=type==VolcanoBossType&&!dead&&!deathPose;
 const bool eruption=boss&&special>=0;
 const int punch=varkhorMelee?(target?target->arm:forward<-.18f?4:5):-1;
 std::optional<Vector2> contact;
 if(punch>=4&&target)contact=bossPunchEndpoint(punch,forward,side,{target->fromFeet.x/s,target->fromFeet.y/s});
 const auto bossPose=boss?(eruption?bossMotion(special,varkhor_motion::eruption):
                      action>=0?bossMotion(action,varkhor_motion::strike):
                                bossMotion(state.phase,varkhor_motion::walk)):varkhor_motion::Key{};
 std::array<Part,6> parts{};
 for(int p=0;p<6;p++){
  auto& part=parts[p];part.pivot={64,90};
  if(fireImp)part=impViewPart(p,view,blend,impAnimation);
  else if(warrior){
   part=sentinel_art::livingPart(sentinelPose,p);
   if(view.t>0)part=turn_art::interpolate(part,sentinel_art::livingPart(nextSentinel,p),view.t);
  }
  else if(directionalCrawler){
   part=turn_art::interpolate(crawler_art::livingPart(crawlerPose,p),crawler_art::livingPart(nextCrawler,p),view.t);
  }
  else if(directionalDrake){
   const int a=drake_turn::sheet::authoredPart(view.base,p),b=drake_turn::sheet::authoredPart(view.nextBase(),p);
   Part left=drake_art::livingPart(drakePose,a),right=drake_art::livingPart(nextPose,b);
   if(dead||deathPose){
    const float release=dead?0.f:1-enemy_death::ease(t);
    left=drake_art::fallPart(left,drakeFall,a,turn,release);
    right=drake_art::fallPart(right,nextFall,b,turn,release);
   }
   part=turn_art::interpolate(left,right,view.t);
  }else if(drake)part=drake_art::livingPart(drakePose,p);
  else{
  if(p==1||p==2||(fourLegs&&p>=4)){auto k=p==1||p==5?control:opposite;float travel=k.travel*(VolcanoStrides[id]*.31f/scale)*blend;part.shift={std::cos(heading)*travel,std::sin(heading)*travel-k.lift*5*blend};}
  else{part.shift={control.sway*.45f*blend,-control.bob*1.2f*(.25f+.75f*blend)};}
  if(p>=4&&!fourLegs){part.pivot=restPivot(id,p);part.angle=control.sway*(p==4?-1:1)*4*blend;if(action>=0){part.angle+=strike.sway*(p==4?-2.5f:3.8f);part.shift.x+=strike.sway*std::cos(heading)*.55f;part.shift.y+=strike.sway*std::sin(heading)*.3f;}}
  if(fourLegs&&action>=0&&(p==3||p>=4)){part.shift.x+=strike.sway*std::cos(heading)*.3f;part.shift.y+=strike.sway*std::sin(heading)*.3f;}
  if(p==0)part.angle=control.sway*2.5f*blend;
  if(boss)poseBossPlate(part,p,forward,side,bossPose,eruption||action>=0?1.f:blend,punch,contact?&*contact:nullptr);
  }
  if(!directionalDrake&&drake&&(dead||deathPose)){
   const float release=dead?0.f:1-enemy_death::ease(t);
   part=drake_art::fallPart(part,drakeFall,p,turn,release);
  }else if(!drake&&(dead||deathPose)){
   auto pivot=restPivot(id,p,direction);
   if(directionalCrawler||fireImp){
    const auto a=restPivot(id,p,view.base),b=restPivot(id,p,view.nextBase());
    pivot={turn_art::mix(a.x,b.x,view.t),turn_art::mix(a.y,b.y,view.t)};
   }
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
   part.scaleX=join(part.scaleX,limb.scaleX);part.scaleY=join(part.scaleY,limb.scaleY);
   if(impFalling&&p>=4){
    part.elbow*=1-fold;
    // Pivot the shoulder with the chest, then add the arm's own delayed swing.
    const float angle=turn*imp.chest*DEG2RAD;
    const auto joint=parts[3].pivot;
    const Vector2 shoulder{part.pivot.x-joint.x,part.pivot.y-joint.y};
    part.shift.x+=shoulder.x*(std::cos(angle)-1)-shoulder.y*std::sin(angle);
    part.shift.y+=shoulder.x*std::sin(angle)+shoulder.y*(std::cos(angle)-1);
   }
  }
  if(directionalCrawler&&deathPose&&!dead&&t>0&&t<1)
   crawler_art::settlePart(part,p,view.base,view.nextBase(),view.t,t,turn);
  if(broken){
   part.shift.x+=(*broken)[p].offset.x;
   part.shift.y+=(*broken)[p].offset.y;
   part.angle+=(*broken)[p].turn;
  }
 }
 const float fallAngle=turn*(rest.turn*tip+(impFalling?0.f:fall.twist*recoil*(1-tip)));
 const float fallOffset=rest.pivotHeight*s*tip;
 const float squash=(towardRest(1,rest.squash)-fall.compression*knees)*heightScale;
 const float slide=fall.slide*(recoil+settle*.25f)*s;
 const float groundedY=artwork?turn_art::mix(atlas.ground[view.base][turn<0].lift,
                        atlas.ground[view.nextBase()][turn<0].lift,view.t):atlas.ground[direction][turn<0].lift;
 const float shiftX=turn*slide,shiftY=(side*slide*.45f+(fall.dip*knees+groundedY*tip)*s)*heightScale;
 const float layerAction=dead||(deathPose&&collapse>=.64f)?-1.f:action;
 std::optional<turn_art::Rig> turnRig;
 if(artwork)turnRig.emplace(*artwork,view,jaw,parts,rest.light,turn,layerAction);
 const float ember=fireImp&&!dead&&action>=0&&action<.45f?
     impAnimation.cast.ember*(deathPose?1-enemy_death::window(t,0,.18f):1.f):0.f;
 std::optional<varkhor_punch::Arm> punchArm;
 if(punch>=4&&std::abs(parts[punch].elbow)>.01f)punchArm.emplace(punch,parts[punch].elbow*DEG2RAD);
 const auto opaqueAt=[&](int part,Vector2 p){
  const int x=int(std::floor(p.x*atlas.cell/128)),y=int(std::floor(p.y*atlas.cell/128));
  return x>=0&&x<atlas.cell&&y>=0&&y<atlas.cell&&
     atlas.alpha[(part*atlas.cell+y)*atlas.art.width+direction*atlas.cell+x]>=100;
 };
 Vector2 localProbe{};
 if(probe){
  localProbe={probe->x-x-shiftX,(probe->y-y-shiftY)/squash};
  const float r=-fallAngle*DEG2RAD;
  localProbe={localProbe.x*std::cos(r)-localProbe.y*std::sin(r),localProbe.x*std::sin(r)+localProbe.y*std::cos(r)-fallOffset};
  if(turnRig)return turnRig->hit({localProbe.x/s,localProbe.y/s});
  for(int p=5;p>=0;p--){
   if(broken&&(*broken)[p].visibility<=0)continue;
   const auto& k=parts[p];Vector2 q{localProbe.x/s-(k.pivot.x-64+k.shift.x),localProbe.y/s-(k.pivot.y-115+k.shift.y)};float r=-k.angle*DEG2RAD;
   float u=(q.x*std::cos(r)-q.y*std::sin(r))/k.scaleX+k.pivot.x;
   float v=(q.x*std::sin(r)+q.y*std::cos(r))/k.scaleY+k.pivot.y;
   if(p==5&&jaw){
    if(drake_mouth::pick(direction,jaw,u,v))return true;
    continue;
   }
   if(p==punch&&punchArm){
    if(punchArm->contains({u,v},[&](Vector2 source){return opaqueAt(p,source);}))return true;
   }else if(opaqueAt(p,{u,v}))return true;
  }return false;
 }
 rlPushMatrix();
 if(shadow){float matrix[]={1,0,0,0,-shadow_math::LightX,-shadow_math::LightY,0,0,0,0,1,0,shadow_math::LightX*y,(1+shadow_math::LightY)*y,0,1};rlMultMatrixf(matrix);}
 rlTranslatef(x+shiftX,y+shiftY,0);
 if(dead||deathPose){rlScalef(1,squash,1);rlRotatef(fallAngle,0,0,1);rlTranslatef(0,fallOffset,0);}
 else if(heightScale!=1)rlScalef(1,heightScale,1);
 if(turnRig){
  rlScalef(s,s,1);
  turnRig->begin(atlas.relief,shadow?0.f:enemy_death::ease(collapse));
  if(!shadow&&outline.a)for(Vector2 delta:{Vector2{-1.5f,0},Vector2{1.5f,0},Vector2{0,-1.5f},Vector2{0,1.5f}})
      turnRig->paint(true,outline,delta.x/s,delta.y/s);
  Color tint=WHITE;tint.a=static_cast<unsigned char>(std::clamp(opacity,0.f,1.f)*255+.5f);
  turnRig->paint(shadow,tint,0,0,shadow);
  if(flash&&!shadow)turnRig->paint(true,Fade(WHITE,.48f));
  if(turn_art::filtered())EndShaderMode();
  if(ember>0&&!shadow)caldera_attack_art::fireOrb(impHands(parts[4],parts[5],view).center,caldera_attack_art::heldOrbScale,ember,action/.45f,0);
  rlPopMatrix();return false;
 }
 auto paint=[&](int p,Texture2D texture,Color tint,float dx=0,float dy=0){auto k=parts[p];Rectangle source{float(direction*atlas.cell),float(p*atlas.cell),float(atlas.cell),float(atlas.cell)};
  if(broken)tint.a=static_cast<unsigned char>(std::lround(tint.a*(*broken)[p].visibility));
  if(!tint.a)return;
  if(p==punch&&punchArm){
   rlPushMatrix();
   rlTranslatef((k.pivot.x-64+k.shift.x)*s+dx,(k.pivot.y-115+k.shift.y)*s+dy,0);
   rlRotatef(k.angle,0,0,1);
   rlScalef(k.scaleX*s,k.scaleY*s,1);
   punchArm->paint(texture,direction,p,tint,k.pivot);
   rlPopMatrix();
   return;
  }
  if(p==5&&jaw){
   // The open skull replaces the closed cutout within the same draw call.
   const auto& crop=drake_mouth::origin[direction];
   const float width=drake_mouth::Tile/1.5f;
   source={float(direction*drake_mouth::Tile),float((jaw-1)*drake_mouth::Tile),
           float(drake_mouth::Tile),float(drake_mouth::Tile)};
   DrawTexturePro(texture.id==atlas.silhouette.id?drake_mouth::mask:drake_mouth::art,source,
       {(k.pivot.x-64+k.shift.x)*s+dx,(k.pivot.y-115+k.shift.y)*s+dy,width*s*k.scaleX,width*s*k.scaleY},
       {(k.pivot.x-crop[0]/1.5f)*s*k.scaleX,(k.pivot.y-crop[1]/1.5f)*s*k.scaleY},k.angle,tint);
   return;
  }
  DrawTexturePro(texture,source,{(k.pivot.x-64+k.shift.x)*s+dx,(k.pivot.y-115+k.shift.y)*s+dy,128*s*k.scaleX,128*s*k.scaleY},
                 {k.pivot.x*s*k.scaleX,k.pivot.y*s*k.scaleY},k.angle,tint);
 };
 int frontOrder[]={0,1,2,3,4,5},backOrder[]={0,1,2,4,5,3},warriorLayers[6];
 const int* order=direction>=5?backOrder:frontOrder;
 if(warrior){turn_art::order(turn_art::sentinel,direction,warriorLayers,layerAction);order=warriorLayers;}
 const int farArm=forward>.3f?5:4;
 const int nearArm=9-farArm;
 const int impOrder[]={0,1,2,farArm,3,nearArm};
 const int impBackOrder[]={1,2,4,3,0,5};
 if(fireImp)order=direction>=5?impBackOrder:impOrder;
 const int drakeBackOrder[]={0,1,2,5,3,4};
 if(drake)order=direction>=5?drakeBackOrder:frontOrder;
 const int activeArmFront[]={0,1,2,9-punch,3,punch};
 if(punch>=4)order=side<-.37f?backOrder:activeArmFront;
 if(!shadow&&outline.a)for(Vector2 delta:{Vector2{-1.5f,0},Vector2{1.5f,0},Vector2{0,-1.5f},Vector2{0,1.5f}})for(int n=0;n<6;n++)paint(order[n],atlas.silhouette,outline,delta.x,delta.y);
 const bool relit=!shadow&&volcano_corpse_material::begin(atlas.relief,enemy_death::ease(collapse),atlas.authoredRest);
 for(int n=0;n<6;n++){
  const int p=order[n];
  const bool farLimb=turn>0?(p==1||p==4):(p==2||p==5);
  const float light=p==3?1.f:p==0?.88f:farLimb?.78f:.98f;
  Color tint=shadow?WHITE:!collapse?WHITE:rgb(int(std::lround(towardRest(255,rest.light.r*light))),
      int(std::lround(towardRest(255,rest.light.g*light))),int(std::lround(towardRest(255,rest.light.b*light))));
  tint.a=static_cast<unsigned char>(std::clamp(opacity,0.f,1.f)*255+.5f);
  if(relit&&p==5&&jaw){
   // This head is the final layer in the five frontal views. Its painted
   // pixels supply matching UVs while the captured jaw relaxes during death.
   EndShaderMode();
   volcano_corpse_material::begin(drake_mouth::art,enemy_death::ease(collapse),false);
  }
  paint(p,shadow?atlas.silhouette:atlas.art,tint);
 }
 if(relit)EndShaderMode();
 if(ember>0&&!shadow){
  const auto point=impHands(parts[4],parts[5],view).center;
  caldera_attack_art::fireOrb({point.x*s,point.y*s},s*caldera_attack_art::heldOrbScale,ember,action/.45f,0);
 }
 if(flash&&!shadow)for(int n=0;n<6;n++)paint(order[n],atlas.silhouette,Fade(WHITE,.48f));
 rlPopMatrix();return false;
}
}

#include "wraith_death_art.hpp"
#include "sentinel_death_art.hpp"
#include "varkhor_death_art.hpp"

namespace volcano_actors {
using DeathDraw=bool(*)(const Enemy&,float,float,float,Color,const Vector2*,bool,float);
inline bool puppetDeath(const Enemy& e,float x,float y,float progress,Color outline,const Vector2* probe,bool shadow,float opacity){
 const auto species=e.type==FireImpType?caldera_puppets::Imp:caldera_puppets::Wraith;
 const float side=e.deathScreenSide()?float(e.deathScreenSide()):int(e.animation.direction)>3?-1.f:1.f;
 const bool landed=progress>=1.f;
 const auto pose=caldera_puppets::fall(species,landed?e.animation:e.deathMotion,landed?-1.f:e.deathAction,progress,side);
 return caldera_puppets::draw(species,pose,x,y,1,outline,probe,shadow,false,opacity);
}
inline DeathDraw specialDeathArt(int type){
 switch(type){
 case LavaSalamanderType:return salamander_art::ready()?salamander_art::fallen:nullptr;
 case FireImpType:return caldera_puppets::ready(caldera_puppets::Imp)?puppetDeath:nullptr;
 case AshWraithType:return caldera_puppets::ready(caldera_puppets::Wraith)?puppetDeath:wraith_death_art::draw;
 case ObsidianSentinelType:return sentinel_death_art::draw;
 case VolcanoBossType:return varkhor_death_art::ash.id?varkhor_death_art::draw:nullptr;
 default:return nullptr;
 }
}
inline Rectangle groundFootprint(const Enemy& e){
 if(e.type==LavaSalamanderType&&salamander_art::ready())return salamander_art::footprint(e);
 if(caldera_quadruped::handles(e.type))return caldera_quadruped::footprint(e.type,e.animation,e.deathScreenSide());
 if((e.type==FireImpType&&caldera_puppets::ready(caldera_puppets::Imp))||
    (e.type==AshWraithType&&caldera_puppets::ready(caldera_puppets::Wraith))){
  const float side=e.deathScreenSide()?float(e.deathScreenSide()):int(e.animation.direction)>3?-1.f:1.f;
  if(e.type==FireImpType)return caldera_puppets::impContact(e.animation,side);
  return {side>0?-8.f:-46.f,-3,54,9};
 }
 switch(e.type){
 case AshWraithType:return wraith_death_art::footprint(e);
 case ObsidianSentinelType:return sentinel_death_art::footprint(e);
 case VolcanoBossType:if(varkhor_death_art::ash.id)return varkhor_death_art::footprint(e);break;
 default:break;
 }
 const int species=e.type-VolcanoFirstType,direction=int(e.animation.direction);
 if(const auto* art=turn_art::forType(e.type)){
  const auto v=turn_art::view(e.animation,*art);
  const int side=flank(v.nearestBase(),e.deathScreenSide());
  const auto& a=atlases[species].ground[v.base][side].footprint;
  const auto& b=atlases[species].ground[v.nextBase()][side].footprint;
  const float heightScale=e.type==EmberDrakeType?turn_art::drakeHeightScale(v):1.f;
  return {turn_art::mix(a.x,b.x,v.t),turn_art::mix(a.y,b.y,v.t)*heightScale,
          turn_art::mix(a.width,b.width,v.t),turn_art::mix(a.height,b.height,v.t)*heightScale};
 }
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
  if(progress<=0)return draw(e.type,x,y,1,e.deathMotion,e.deathAction,outline,probe,shadow,false,false,e.deathSpecial,
                             nullptr,0,opacity,0,nullptr,punchTarget(e));
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
