#pragma once

// Included in volcano_actors alongside the shared directional atlas renderer.
namespace desert_enemy_art {
using namespace ember::desert_enemy_motion;

inline Texture2D sickle{};
inline Shader sickleMask{};
inline int sickleShadow=-1;
inline std::array<unsigned char,96*96> sickleAlpha{};

inline void load(){
 if(sickle.id)return;
 Image image=LoadImage("assets/desert/reaper-turn/sickle.png");
 if(!image.data)return;
 if(image.width!=96||image.height!=96){UnloadImage(image);return;}
 ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
 const auto* ink=static_cast<const Color*>(image.data);
 for(size_t i=0;i<sickleAlpha.size();i++)sickleAlpha[i]=ink[i].a;
 const auto coverage=std::minmax_element(sickleAlpha.begin(),sickleAlpha.end());
 if(*coverage.first!=0||*coverage.second<200){UnloadImage(image);return;}
 sickle=LoadTextureFromImage(image);
 UnloadImage(image);
 SetTextureFilter(sickle,TEXTURE_FILTER_BILINEAR);
 sickleMask=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform int shadow;
out vec4 finalColor;
void main(){
 float coverage=texture(texture0,fragTexCoord).a;
 if(coverage<(shadow!=0?.55:.006))discard;
 finalColor=shadow!=0?vec4(fragColor.a):vec4(fragColor.rgb,coverage*fragColor.a);
}
)GLSL");
 sickleShadow=GetShaderLocation(sickleMask,"shadow");
}
inline void unload(){
 if(sickle.id)UnloadTexture(sickle);
 if(sickleMask.id&&sickleMask.id!=rlGetShaderIdDefault())UnloadShader(sickleMask);
 sickle={};sickleMask={};sickleShadow=-1;
}

inline float curve(const float (&values)[61],float time){
 const float sample=std::clamp(time,0.f,1.f)*60;
 const int at=std::min(59,int(sample));
 return turn_art::mix(values[at],values[at+1],sample-at);
}

inline float loop(float phase){return phase-std::floor(phase);}

inline float walkingWeight(const AnimationState& state,float attack){
 const float plant=std::clamp(attack/.24f,0.f,1.f);
 const float moving=state.motionInitialized?state.moveBlend:(state.moving?1.f:0.f);
 return moving*(1-plant*plant*(3-2*plant));
}

struct SickleMotion {
 float shoulder,elbow,wrist,roll;
};

inline SickleMotion sickleMotion(const AnimationState& state,float attack){
 using namespace ember::reaper_sickle_motion;
 const float weight=walkingWeight(state,attack);
 const auto sample=[&](const float (&walking)[61],const float (&striking)[61]){
  return weight*curve(walking,loop(state.phase))+(attack>=0?curve(striking,attack):0.f);
 };
 return {sample(walk_shoulder,attack_shoulder),sample(walk_elbow,attack_elbow),
         sample(walk_wrist,attack_wrist),sample(walk_roll,attack_roll)};
}

inline Vector2 rotated(Vector2 v,float degrees){
 const float c=std::cos(degrees*DEG2RAD),s=std::sin(degrees*DEG2RAD);
 return {c*v.x-s*v.y,s*v.x+c*v.y};
}

inline void carry(Part& limb,const Part& parent,float transfer){
 const Vector2 local{limb.pivot.x-parent.pivot.x,limb.pivot.y-parent.pivot.y};
 const auto at=rotated({local.x*parent.scaleX,local.y*parent.scaleY},parent.angle);
 limb.shift={(parent.shift.x+at.x-local.x)*transfer,(parent.shift.y+at.y-local.y)*transfer};
}

inline void step(Part& limb,const turn_art::Artwork& art,const turn_art::View& view,int slot,
                 Vector2 target){
 const auto foot=turn_art::footTip(art,view,slot);
 const auto bent=limb.elbow!=0&&turn_art::filtered()?turn_art::reaperKnee(limb.pivot,foot,limb.elbow).map(foot,false):foot;
 const Vector2 rest{bent.x-limb.pivot.x,bent.y-limb.pivot.y};
 const Vector2 goal{foot.x-limb.pivot.x+target.x-limb.shift.x,foot.y-limb.pivot.y+target.y-limb.shift.y};
 limb.angle=std::atan2(rest.x*goal.y-rest.y*goal.x,rest.x*goal.x+rest.y*goal.y)*RAD2DEG;
 const float length=std::clamp(std::hypot(goal.x,goal.y)/std::max(1.f,std::hypot(rest.x,rest.y)),.78f,1.15f);
 limb.scaleX=limb.scaleY=length;
}

inline void jackalStep(Part& limb,const turn_art::View& view,int slot,Vector2 target){
 const auto foot=turn_art::footTip(turn_art::jackal,view,slot);
 const Vector2 goal{foot.x+target.x-limb.pivot.x-limb.shift.x,foot.y+target.y-limb.pivot.y-limb.shift.y};
 const float distance=std::max(1.f,std::hypot(goal.x,goal.y));
 auto joint=turn_art::jackalKnee(view,slot,limb.pivot,foot,0);
 const Vector2 upper{joint.center.x-limb.pivot.x,joint.center.y-limb.pivot.y};
 const Vector2 shin{foot.x-joint.center.x,foot.y-joint.center.y};
 const float upperLength=std::max(1.f,std::hypot(upper.x,upper.y)),shinLength=std::max(1.f,std::hypot(shin.x,shin.y));
 const float restAngle=std::atan2(upper.x*shin.y-upper.y*shin.x,upper.x*shin.x+upper.y*shin.y);
 const float bend=std::acos(std::clamp((distance*distance-upperLength*upperLength-shinLength*shinLength)/
                                      (2*upperLength*shinLength),-1.f,1.f));
 const float bendSign=(slot>=4?1.f:-1.f)*std::clamp(3.f*std::cos(turn_art::heading(view)),-1.f,1.f);
 joint.angle=turn_art::filtered()?std::clamp(bend*bendSign-restAngle,-.95f,.95f):0.f;
 limb.elbow=joint.angle*RAD2DEG;
 const auto folded=joint.map(foot,false);
 const Vector2 line{folded.x-limb.pivot.x,folded.y-limb.pivot.y};
 limb.angle=std::atan2(line.x*goal.y-line.y*goal.x,line.x*goal.x+line.y*goal.y)*RAD2DEG;
 // At full reach the shoulder has a small amount of travel along the flank.
 // Both distances within the limb are invariant.
 const float glide=std::clamp(distance-std::hypot(line.x,line.y),-5.2f,5.2f)/distance;
 limb.shift.x+=goal.x*glide;
 limb.shift.y+=goal.y*glide;
}

inline std::array<Part,6> livingPose(const turn_art::Artwork& art,const turn_art::View& view,
                                     const AnimationState& state,float attack,float hit,SickleMotion weapon={}){
 const bool jackal=&art==&turn_art::jackal;
 const float heading=turn_art::heading(view);
 const Vector2 facing{std::cos(heading),std::sin(heading)};
 const auto headingOnGround=actorFacing(heading);
 const bool hasTravel=std::abs(state.travelAxis.x)+std::abs(state.travelAxis.y)>.01f;
 const Vector2 travel=hasTravel?Vector2{state.travelAxis.x,state.travelAxis.y*ActorGroundSquash}:
                               Vector2{headingOnGround.x,headingOnGround.y*ActorGroundSquash};
 const float gait=walkingWeight(state,attack);
 const float walk=state.phase, wave=2*PI*walk;
 const float breathe=std::sin(state.idleTime*(jackal?4.6f:2.8f))*(1-gait);
 float bodyY=.62f*breathe+gait*(jackal?1.65f*curve(jackal_weight,walk):1.5f+2.8f*std::cos(2*wave-.72f));
 float dash=0,crouch=0,neck=0,sweep=0,twist=0,air=0;
 if(attack>=0){
  if(jackal){
   crouch=curve(jackal_crouch,attack);
   dash=curve(jackal_pounce,attack);
   neck=curve(jackal_neck,attack);
   air=curve(jackal_land,attack);
   bodyY+=8.3f*crouch-13.1f*air;
  }else{
   crouch=curve(reaper_crouch,attack);
   sweep=curve(reaper_sickle,attack);
   twist=curve(reaper_turn,attack);
   bodyY+=7.6f*crouch-2.9f*twist;
  }
 }
 std::array<Part,6> parts{};
 for(int p=0;p<6;p++){
  if(jackal){
   const auto& a=dune_jackal_turn_atlas::pivots[view.sample][p];
   const auto& b=dune_jackal_turn_atlas::pivots[view.nextSample()][p];
   parts[p].pivot={.8f*turn_art::mix(a[0],b[0],view.mix),.8f*turn_art::mix(a[1],b[1],view.mix)};
  }else{
   const auto& a=glass_reaper_turn_atlas::pivots[view.base][p];
   const auto& b=glass_reaper_turn_atlas::pivots[view.nextBase()][p];
   parts[p].pivot={.8f*turn_art::mix(a[0],b[0],view.t),.8f*turn_art::mix(a[1],b[1],view.t)};
  }
 }
 auto& trunk=parts[3];
 const float swingSide=std::tanh(facing.x*2.7f)*.81f+.19f*std::abs(facing.y);
 const float balance=jackal?curve(jackal_balance,walk):0;
 const float pitch=gait*(jackal?2.1f*balance:3.4f*std::sin(2*wave-.44f));
 const float sway=jackal?.67f*balance:1.9f*std::sin(wave-.35f);
 trunk.shift={facing.x*(dash*7.3f-crouch*2.7f-hit*3+twist*4.6f)+gait*sway,
              bodyY+facing.y*(dash*3.4f+twist*3)+hit*1.9f};
 trunk.angle=facing.x*(pitch+neck*7.f)+twist*10.f*swingSide-state.turnLean*gait*(jackal?1.2f:4.f);
 auto& head=parts[0];
 carry(head,trunk,1);
 const float counter=jackal?.95f*balance:4.5f*std::sin(2*wave+.28f);
 head.angle=trunk.angle*.63f+facing.x*(neck*12-gait*counter)-twist*5.3f*swingSide;
 head.shift.x+=facing.x*dash*2.9f;
 head.shift.y+=neck*1.8f-gait*(jackal?.35f: .9f)*std::sin(wave-.52f);
 for(const int p:{1,2,4,5}){
  auto& limb=parts[p];
  const bool left=p==1||p==4;
  if(jackal||p<3){
   constexpr float pawPhase[]={0,.50f,0,0,.77f,.27f};
   const float phase=loop(walk+(jackal?pawPhase[p]:(left?0.f:.5f)));
   const float reach=curve(jackal?jackal_stride:reaper_stride,phase)*gait*
                     (jackal?desert_motion::JackalCycle*.30f:desert_motion::ReaperCycle*19/60);
   const float lift=curve(jackal?jackal_lift:reaper_lift,phase)*gait*(jackal?(p>=4?7.4f:6.3f):9.7f);
   carry(limb,trunk,1);
   const float plant=jackal?dash*(p>=4?12.f:-4.6f):p==2?std::max(0.f,twist)*11.f:-std::max(0.f,twist)*2.3f;
   const float kick=jackal?air*(p>=4?12.5f:9.8f):0.f;
   const Vector2 target{travel.x*reach+facing.x*plant,travel.y*reach-lift+facing.y*plant*.52f-kick};
   if(jackal)jackalStep(limb,view,p,target);
   else{
    limb.elbow=-(lift*3.6f+crouch*(left?15.f:24.f))*swingSide;
    step(limb,art,view,p,target);
   }
   continue;
  }
  const float stride=std::sin(wave+(left?PI:0.f)-.68f)*gait;
  carry(limb,trunk,1);
  limb.angle=trunk.angle+(left?stride*(11+16*std::abs(facing.x))+twist*24.f:weapon.shoulder*swingSide);
  limb.elbow=left?-9*gait-stride*7+twist*21.f:weapon.elbow*swingSide;
  if(!left)limb.shift.y+=std::max(0.f,sweep)*facing.y*4.6f-4.f*std::max(0.f,-sweep);
 }
 return parts;
}

inline std::array<Part,6> collapsePose(const turn_art::Artwork& art,const turn_art::View& view,
                                       const AnimationState& state,float attack,float hit,float fall,float side,
                                       SickleMotion weapon={}){
 const bool jackal=&art==&turn_art::jackal;
 auto pose=livingPose(art,view,state,attack,hit,weapon);
 auto rest=livingPose(art,view,AnimationState{},-1,0);
 const float fold=curve(jackal?jackal_fold:reaper_fold,fall);
 const float drop=std::max(0.f,curve(jackal?jackal_drop:reaper_drop,fall));
 const float buckle=curve(jackal?jackal_buckle:reaper_buckle,fall);
 const float heading=turn_art::heading(view);
 const float fore=std::cos(heading),depth=std::abs(std::sin(heading));
 const float pawSide=std::clamp(1.7f*fore,-1.f,1.f);
 const float tuck=pawSide+side*(1-std::abs(pawSide));
 auto& trunk=rest[3];
 trunk.shift={side*drop*(jackal?3.f:6.f),buckle*(jackal?6.f:10.f)};
 trunk.scaleX=1+.025f*drop;
 trunk.scaleY=1-.02f*drop;
 trunk.angle=jackal?tuck*(buckle*12.f+drop*3.f):side*(buckle*10.f-drop*7.f);
 for(int p=0;p<6;p++){
  auto& limb=rest[p];
  if(p!=3)carry(limb,trunk,1);
  if(p==0){
   limb.angle+=(jackal?tuck:side)*(buckle*9.f+drop*(jackal?12.f:15.f));
   limb.shift.y+=buckle*(jackal?6.f:4.f)+drop*(jackal?3.f+6.f*depth:2.f);
  }else if(p!=3){
   const bool near=p==2||p==5;
   const float sign=near?1.f:-1.f;
   if(jackal){
    limb.angle+=tuck*(p<3?-1.f:1.f)*(drop*(near?55.f:72.f)+buckle*(p<3?10.f:53.f));
    limb.scaleY*=1-drop*(near?.15f:.26f)-buckle*(p<3?.04f:.20f);
    limb.shift.y-=drop*(near?3.f:6.f);
   }else{
    limb.angle+=side*(p<3?sign*(drop*24.f+buckle*(near?37.f:22.f)):sign*drop*23.f+buckle*17.f);
    limb.scaleY*=1-drop*.06f-buckle*(p<3?.15f:0.f);
    limb.elbow=side*(p<3?-(drop*(near?26.f:39.f)+buckle*46.f):drop*(near?-26.f:32.f)+buckle*16.f);
   }
  }
  pose[p]=turn_art::interpolate(pose[p],limb,fold);
 }
 return pose;
}

struct WeaponPose {
 Vector2 palm{};
 float angle=0,width=.36f;
 bool front=false;
};
inline constexpr Vector2 Grip{48,32};

inline WeaponPose weaponPose(const turn_art::Rig& rig,SickleMotion motion,float fold){
 const auto& arm=rig.parts[5];
 const auto& elbow=rig.joints[5];
 auto paintedPalm=turn_art::reaperJoint(rig.angle,3);
 const auto& glove=rig.partBox[5];
 const auto covered=[&](Vector2 point){
  return turn_art::turnCoverage(rig.art,rig.angle,5,0,(point.x-glove.x)/glove.width,(point.y-glove.y)/glove.height);
 };
 if(covered(paintedPalm)<.57f){
  Vector2 total{};
  float weights=0;
  for(float dy:{-6.f,-3.f,0.f,3.f,6.f})for(float dx:{-6.f,-3.f,0.f,3.f,6.f}){
   const Vector2 candidate{paintedPalm.x+dx,paintedPalm.y+dy};
   const float weight=std::max(0.f,covered(candidate)-.57f)/(1+dx*dx+dy*dy);
   weights+=weight;total.x+=weight*candidate.x;total.y+=weight*candidate.y;
  }
  if(weights>0)paintedPalm={total.x/weights,total.y/weights};
 }
 const auto palm=elbow.map(paintedPalm,false);
 const auto local=rotated({(palm.x-arm.pivot.x)*arm.scaleX,(palm.y-arm.pivot.y)*arm.scaleY},arm.angle);
 const float facing=std::cos(turn_art::heading(rig.angle));
 const float wrist=motion.wrist*(facing*.81f+.19f)*(1-fold);
 const float width=.39f*(.86f+.14f*std::abs(facing))*(1-.37f*std::clamp(motion.roll*(1-fold),0.f,1.f));
 return {{arm.pivot.x+arm.shift.x+local.x-64,arm.pivot.y+arm.shift.y+local.y-115},
         arm.angle+elbow.angle*RAD2DEG+19*facing+wrist,
         width,facing>-.16f};
}
inline constexpr float SickleHeightScale=.39f;
inline void paintWeapon(WeaponPose weapon,Color tint,bool mask,bool shadow,Vector2 offset={}){
 if(mask||shadow){
  if(sickleShadow<0)return;
  BeginShaderMode(sickleMask);
  const int mode=shadow;
  SetShaderValue(sickleMask,sickleShadow,&mode,SHADER_UNIFORM_INT);
 }
 DrawTexturePro(sickle,{0,0,96,96},{weapon.palm.x+offset.x,weapon.palm.y+offset.y,96*weapon.width,96*SickleHeightScale},
                {Grip.x*weapon.width,Grip.y*SickleHeightScale},weapon.angle,tint);
 if(mask||shadow)EndShaderMode();
}
inline bool hitWeapon(WeaponPose weapon,Vector2 q){
 const auto local=rotated({q.x-weapon.palm.x,q.y-weapon.palm.y},-weapon.angle);
 const float x=local.x/weapon.width+Grip.x,y=local.y/SickleHeightScale+Grip.y;
 return x>=0&&x<96&&y>=0&&y<96&&sickleAlpha[int(y)*96+int(x)]>=111;
}

inline bool draw(const Enemy& e,float x,float y,Color outline,const Vector2* probe,bool shadow,
                 float scale,float opacity=1,float fall=-1){
 const bool jackal=e.type==DuneJackalType;
 if(jackal&&jackal_blender::ready())
  return jackal_blender::draw(e,x,y,outline,probe,shadow,scale,opacity,fall);
 if(jackal&&jackal_rig::ready())
  return jackal_rig::draw(e,x,y,outline,probe,shadow,scale,opacity,fall);
 if(jackal&&jackal_cels::ready())
  return jackal_cels::draw(e,x,y,outline,probe,shadow,scale,opacity,fall);
 if(jackal&&jackal_skin::ready())
  return jackal_skin::draw(e,x,y,outline,probe,shadow,scale,opacity,fall);
 if(jackal&&jackal_puppet::ready())
  return jackal_puppet::draw(e,x,y,outline,probe,shadow,scale,opacity,fall);
 const auto& art=jackal?turn_art::jackal:turn_art::reaper;
 if(!art.ready()||opacity<=0)return false;
 const bool fallen=fall>=0;
 auto state=fallen?e.deathMotion:e.animation;
 if(!state.facingInitialized&&fallen)state=e.animation;
 const float attack=fallen?e.deathAction:e.volcanoAction();
 const auto view=turn_art::view(state,art);
 const float side=e.deathScreenSide()?float(e.deathScreenSide()):std::cos(turn_art::heading(view))>=0?1.f:-1.f;
 const float drop=fallen?curve(jackal?jackal_drop:reaper_drop,fall):0;
 const float hit=std::clamp((fallen?e.deathFlash:e.flash)/.16f,0.f,1.f);
 const auto motion=jackal?SickleMotion{}:sickleMotion(state,attack);
 const auto parts=fallen?collapsePose(art,view,state,attack,hit,fall,side,motion):
                         livingPose(art,view,state,attack,hit,motion);
 turn_art::Rig rig(art,view,0,parts,WHITE,side,attack);
 const float roll=fallen?curve(jackal?jackal_roll:reaper_roll,fall):0;
 const float heading=turn_art::heading(view);
 const float endOn=std::abs(std::sin(heading));
 const float profile=std::clamp(std::cos(heading)*2.f,-1.f,1.f);
 const float jackalFlank=profile+side*(1-std::abs(profile));
 const float rotation=roll*(jackal?jackalFlank*(22.f+26.f*endOn):side*68.f);
 const float sy=1-std::max(0.f,drop)*(jackal?.07f:.025f);
 const float hip=jackal?-29.f:-44.f;
 const auto project=[&](float px,float py){
  const auto q=rotated({px,(py-hip)*sy},rotation);
  return Vector2{q.x,q.y+hip};
 };
 float fit=0;
 if(drop>0){
  float bottom=-1e6f;
  for(int p=0;p<6;p++){
   const auto& k=parts[p];
   const auto& rect=rig.partBox[p];
   const auto support=[&](int frame){
    float edge=-1e6f;
    for(const auto point:art.contactContours[frame*6+p]){
     Vector2 center{rect.x+point.x*rect.width,rect.y+point.y*rect.height};
     center=rig.joints[p].map(center,false);
     const auto offset=rotated({(center.x-k.pivot.x)*k.scaleX,(center.y-k.pivot.y)*k.scaleY},k.angle);
     const auto q=project(k.pivot.x+k.shift.x-64+offset.x,k.pivot.y+k.shift.y-115+offset.y);
     edge=std::max(edge,q.y);
    }
    return edge;
   };
   bottom=std::max(bottom,support(view.nearestSample()));
  }
  const float settle=std::clamp((drop-.08f)*1.39f,0.f,1.f);
  fit=(1.4f-bottom)*(bottom>1.4f?1.f:settle*settle*(3-2*settle));
 }
 const float size=scale*(jackal?1.f:1.05f);
 const bool armed=!jackal&&sickle.id;
 const float fold=fallen?curve(reaper_fold,fall):0;
 auto weapon=armed?weaponPose(rig,motion,fold):WeaponPose{};
 if(armed&&fallen){
  const float groundAngle=(side>0?63.f:-117.f)-rotation;
  const float settle=fold*std::max(0.f,drop);
  weapon.angle+=std::remainder(groundAngle-weapon.angle,360.f)*settle;
 }
 if(probe){
  const auto point=rotated({(probe->x-x)/size,(probe->y-y)/size-fit-hip},-rotation);
  const Vector2 q{point.x,point.y/sy+hip};
  return opacity>=.43f&&(rig.hit(q)||(armed&&hitWeapon(weapon,q)));
 }
 rlPushMatrix();
 rlTranslatef(x,y+size*(hip+fit),0);
 rlScalef(size,size,1);
 rlRotatef(rotation,0,0,1);
 rlScalef(1,sy,1);
 rlTranslatef(0,-hip,0);
 constexpr std::array<Vector2,4> rim{{{1.4f,0},{-1.4f,0},{0,1.4f},{0,-1.4f}}};
 if(armed&&outline.a&&!shadow)for(auto q:rim)paintWeapon(weapon,Fade(outline,opacity),true,false,q);
 if(outline.a&&!shadow){
  rig.begin({},0);
  for(auto q:rim)rig.paint(true,Fade(outline,opacity),q.x,q.y);
  if(turn_art::filtered())EndShaderMode();
 }
 const unsigned char shade=static_cast<unsigned char>(255-std::max(0.f,drop)*18);
 const Color tint=shadow?WHITE:Color{255,shade,shade,255};
 if(armed&&!weapon.front)paintWeapon(weapon,Fade(tint,opacity),false,shadow);
 rig.begin({},0);
 rig.paint(shadow,Fade(tint,opacity),0,0,shadow);
 if(turn_art::filtered())EndShaderMode();
 if(armed&&weapon.front)paintWeapon(weapon,Fade(tint,opacity),false,shadow);
 rlPopMatrix();
 return false;
}
}
