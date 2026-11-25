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
  return attack>=0?curve(striking,attack):weight*curve(walking,loop(state.phase));
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

inline void seatReaperShoulder(Part& arm,const Part& trunk,int part,float heading){
 carry(arm,trunk,1);
 const float front=std::max(0.f,std::sin(heading));
 const float inset=(part==4?-2.f:2.f)*front*front*front*front;
 const auto local=rotated({inset*trunk.scaleX,0},trunk.angle);
 arm.shift.x+=local.x;
 arm.shift.y+=local.y;
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

inline constexpr float ReaperScale=1.05f;

inline void reaperStride(Part& leg,turn_art::View view,int slot,Vector2 travel,
                         float forward,float lift,float bend,float toe,float gait,Vector2 target){
 const auto foot=turn_art::footTip(turn_art::reaper,view,slot);
 const float span=std::hypot(foot.x-leg.pivot.x,foot.y-leg.pivot.y);
 const float reach=span*std::cos(bend*.5f*DEG2RAD);
 const float hip=lift+std::sqrt(std::max(0.f,reach*reach-forward*forward));
 const auto knee=grounded_gait::knee(forward,hip,lift,span*.5f);
 const float ahead=knee.forward-.5f*forward;
 const float raised=knee.height-.5f*(hip+lift);
 const Vector2 delta{target.x-leg.shift.x,target.y-leg.shift.y};
 leg.kneeShift={.47f*delta.x+travel.x*ahead,.47f*delta.y+travel.y*ahead-raised};
 leg.footShift=delta;
 leg.footPitch=toe*std::cos(turn_art::heading(view));
 leg.footWeight=std::min(1.f,2*gait);
}

inline void reaperAttackLeg(Part& leg,turn_art::View view,int slot,Vector2 offset,
                            float headingX,float toe){
 const auto foot=turn_art::footTip(turn_art::reaper,view,slot);
 const auto rest=turn_art::reaperKnee(leg.pivot,foot,0);
 const float length=std::hypot(foot.x-leg.pivot.x,foot.y-leg.pivot.y);
 const float thigh=.47f*length,shin=.37f*length;
 const auto boot=rotated(rest.axis,toe);
 const Vector2 delta{offset.x-leg.shift.x,offset.y-leg.shift.y};
 const Vector2 toAnkle{foot.x-leg.pivot.x+delta.x-.16f*length*boot.x,
                       foot.y-leg.pivot.y+delta.y-.16f*length*boot.y};
 const float distance=std::max(.01f,std::hypot(toAnkle.x,toAnkle.y));
 const float reach=std::clamp(distance,thigh-shin+.01f,thigh+shin);
 const float along=(thigh*thigh-shin*shin+reach*reach)/(2*reach);
 // The bend follows the strike plane, so a head-on knee cannot kick sideways.
 const float forwardBend=std::sqrt(std::max(0.f,thigh*thigh-along*along))*headingX;
 const Vector2 knee{toAnkle.x*(along/reach)+toAnkle.y*(forwardBend/distance),
                    toAnkle.y*(along/reach)-toAnkle.x*(forwardBend/distance)};
 leg.kneeShift={leg.pivot.x+knee.x-rest.center.x,leg.pivot.y+knee.y-rest.center.y};
 leg.footShift=delta;
 leg.footPitch=toe;
 leg.footWeight=1;
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
 float stanceInset=0;
 if(!jackal&&gait>0){
  const float left=turn_art::footTip(art,view,1).x,right=turn_art::footTip(art,view,2).x;
  const float front=headingOnGround.y*headingOnGround.y;
  stanceInset=.5f*(left-right)*(1-ember::reaper_motion::StanceWidth)*front*gait;
 }
 const float walk=state.phase, wave=2*PI*walk;
 const float breathe=std::sin(state.idleTime*(jackal?4.6f:2.8f))*(1-gait);
 const auto bodyWalk=jackal?ember::reaper_motion::Walk{}:ember::reaper_motion::sampleWalk(walk);
 const auto cut=jackal||attack<0?ember::reaper_motion::Attack{}:ember::reaper_motion::sampleAttack(attack);
 float bodyY=.62f*breathe+gait*(jackal?1.65f*curve(jackal_weight,walk):bodyWalk.drop);
 float dash=0,crouch=0,neck=0,air=0;
 if(attack>=0){
  if(jackal){
   crouch=curve(jackal_crouch,attack);
   dash=curve(jackal_pounce,attack);
   neck=curve(jackal_neck,attack);
   air=curve(jackal_land,attack);
   bodyY+=8.3f*crouch-13.1f*air;
  }else{
   bodyY+=cut.drop;
  }
 }
 std::array<Part,6> parts{};
 for(int p=0;p<6;p++){
  if(jackal){
   const auto& a=dune_jackal_turn_atlas::pivots[view.sample][p];
   const auto& b=dune_jackal_turn_atlas::pivots[view.nextSample()][p];
   parts[p].pivot={.8f*turn_art::mix(a[0],b[0],view.mix),.8f*turn_art::mix(a[1],b[1],view.mix)};
  }else{
   parts[p].pivot=p>=4?turn_art::reaperShoulder(view,p):
                       turn_art::reaperAnchor(view,glass_reaper_turn_atlas::pivots,p);
  }
 }
 auto& trunk=parts[3];
 const float swingSide=std::tanh(facing.x*2.7f)*.81f+.19f*std::abs(facing.y);
 const float balance=jackal?curve(jackal_balance,walk):0;
 const float pitch=gait*(jackal?2.1f*balance:bodyWalk.pitch);
 const float sway=jackal?.67f*balance:bodyWalk.shift;
 trunk.shift={facing.x*(dash*7.3f-crouch*2.7f-hit*3+cut.shift)+gait*sway,
              bodyY+facing.y*(dash*3.4f+cut.shift*.52f)+hit*1.9f};
 trunk.angle=facing.x*(pitch+neck*7.f)+cut.pitch*swingSide-state.turnLean*gait*(jackal?1.2f:1.4f);
 auto& head=parts[0];
 carry(head,trunk,1);
 head.angle=trunk.angle*.63f+facing.x*(neck*12+gait*(jackal?-.95f*balance:bodyWalk.head))+
            cut.head*swingSide;
 head.shift.x+=facing.x*dash*2.9f;
 head.shift.y+=neck*1.8f-gait*(jackal?.35f:.32f)*std::sin(wave-.52f);
 for(const int p:{1,2,4,5}){
  auto& limb=parts[p];
  const bool left=p==1||p==4;
  if(jackal||p<3){
   constexpr float pawPhase[]={0,.50f,0,0,.77f,.27f};
   const float phase=loop(walk+(jackal?pawPhase[p]:(left?0.f:.5f)));
   const auto foot=jackal?ember::reaper_motion::Step{}:ember::reaper_motion::sampleStep(phase);
   const float reach=(jackal?curve(jackal_stride,phase)*desert_motion::JackalCycle*.30f:
                            foot.reach*desert_motion::ReaperCycle/ReaperScale)*gait;
   const float lift=(jackal?curve(jackal_lift,phase):foot.lift)*gait*(jackal?(p>=4?7.4f:6.3f):9.7f);
   carry(limb,trunk,1);
   if(jackal){
    const float plant=dash*(p>=4?12.f:-4.6f),kick=air*(p>=4?12.5f:9.8f);
    jackalStep(limb,view,p,{travel.x*reach+facing.x*plant,
                           travel.y*reach-lift+facing.y*plant*.52f-kick});
    continue;
   }
   const Vector2 walkTarget{travel.x*reach+(left?-stanceInset:stanceInset),travel.y*reach-lift};
   const bool segmented=turn_art::filtered();
   auto striking=limb;
   if(segmented)reaperStride(limb,view,p,travel,reach,lift,foot.knee*gait,foot.toe*gait,gait,walkTarget);
   else step(limb,art,view,p,walkTarget);
   if(attack>=0){
    // The leg opposite the weapon arm leads, while the rear foot supports the push.
    const Vector2 offset=left?Vector2{headingOnGround.x*cut.lead_reach,
      headingOnGround.y*ActorGroundSquash*cut.lead_reach-cut.lead_lift}:Vector2{};
    const float toe=(left?cut.lead_toe:cut.rear_toe)*headingOnGround.x;
    if(segmented)reaperAttackLeg(striking,view,p,offset,headingOnGround.x,toe);
    else step(striking,art,view,p,offset);
    const float weight=enemy_death::ease(attack/.14f)*(1-enemy_death::ease((attack-.94f)/.06f));
    limb=turn_art::interpolate(limb,striking,weight);
   }
   continue;
  }
  seatReaperShoulder(limb,trunk,p,heading);
  const float armProjection=attack<0?facing.x:swingSide;
  limb.angle=trunk.angle+armProjection*(left?gait*bodyWalk.free_arm+cut.free_arm:weapon.shoulder);
  limb.elbow=armProjection*(left?gait*bodyWalk.free_elbow+cut.free_elbow:weapon.elbow);
  if(p==5&&attack>=0){
   // The right hand rises on the target side before the downward cut.
   const float winding=facing.x+.65f*facing.y>=0?1.f:-1.f;
   limb.overhead=winding;
   // Each painted view starts with a different resting bend. Play the lift
   // from the Blender rig's neutral axes and return to the existing strike.
   const auto joint=turn_art::reaperJoint(view,2),palm=turn_art::reaperJoint(view,3);
   const float upperRest=std::atan2(limb.pivot.x-joint.x,joint.y-limb.pivot.y)*RAD2DEG;
   const float lowerRest=std::atan2(joint.x-palm.x,palm.y-joint.y)*RAD2DEG;
   const float align=enemy_death::ease(attack/.19f)*(1-enemy_death::ease((attack-.40f)/.05f));
   const float shoulderOffset=winding*(limb.pivot.x+limb.shift.x-trunk.pivot.x-trunk.shift.x);
   const float opening=align*enemy_death::ease((attack-.24f)/.12f);
   const float forwardPitch=opening*std::clamp(-shoulderOffset*2.8f,0.f,29.f)*std::abs(facing.x);
   limb.wristCounterturn=-winding*forwardPitch;
   limb.angle=trunk.angle+std::remainder(winding*(weapon.shoulder+forwardPitch),360.f)-align*upperRest;
   limb.elbow=winding*weapon.elbow-align*(lowerRest-upperRest);
  }
 }
 return parts;
}

inline std::array<Part,6> collapsePose(const turn_art::Artwork& art,const turn_art::View& view,
                                       const AnimationState& state,float attack,float hit,float fall,float side,
                                       SickleMotion weapon,const ember::reaper_motion::Fall& death){
 const bool jackal=&art==&turn_art::jackal;
 auto pose=livingPose(art,view,state,attack,hit,weapon);
 auto rest=livingPose(art,view,AnimationState{},-1,0);
 if(!jackal){
  auto& chest=rest[3];
  chest.shift={side*death.hip_shift,death.hip_drop};
  chest.angle=side*death.chest_pitch;
  chest.scaleX=1+death.chest_width;
  chest.scaleY=1+death.chest_length;
  const Part pelvis=chest;
  chest.angle+=side*death.back_arch;
  const auto& order=turn_art::reaperOrder[view.nearestBase()];
  const auto above=[&](int a,int b){
   return std::find(std::begin(order),std::end(order),a)>std::find(std::begin(order),std::end(order),b);
  };
  const float shoulderSpan=rest[4].pivot.x-rest[5].pivot.x;
  const float spread=death.arm_spread*std::clamp(shoulderSpan/10.f,-1.f,1.f);
  for(int p:{0,1,2,4,5}){
   auto& limb=rest[p];
   if(p>=4)seatReaperShoulder(limb,chest,p,turn_art::heading(view));
   else carry(limb,p==0?chest:pelvis,1);
   if(p==0){
    limb.angle=side*(death.head_tilt+.25f*death.back_arch);
    limb.shift.y+=2.25f*death.blend;
   }else if(p<3){
    const bool top=above(p,3-p);
    limb.angle=side*(top?death.leg_top:death.leg_low);
    limb.elbow=side*(top?death.knee_top:death.knee_low);
    limb.scaleY=1-death.ground*(top?.13f:.05f);
   }else{
    const bool top=above(p,9-p);
    limb.angle=side*(top?death.arm_top:death.arm_low)+(p==4?-spread:spread);
    limb.elbow=side*(top?death.elbow_top:death.elbow_low);
    limb.scaleY=1-death.ground*(top?.04f:.09f);
    if(p==4&&death.left_arm_settle>0){
     const float settle=death.left_arm_settle;
     limb.elbow=turn_art::mix(limb.elbow,side*8.f,settle);
     const auto palm=turn_art::reaperJoint(view,1);
     const auto elbow=turn_art::filtered()?turn_art::reaperElbow(view,p,limb.elbow):turn_art::SwordJoint{};
     const auto bent=elbow.map(palm,false);
     const float restAngle=std::atan2((bent.y-limb.pivot.y)*limb.scaleY,
                                     (bent.x-limb.pivot.x)*limb.scaleX);
     // Aim the relaxed hand down in the ground projection, independent of
     // the body roll. The socket stays seated in the same pauldron.
     const auto aim=rotated({-side,.48f},-side*death.roll);
     const float groundAngle=std::atan2(aim.y/(1-.025f*death.ground),aim.x);
     limb.angle+=settle*std::remainder((groundAngle-restAngle)*RAD2DEG-limb.angle,360.f);
    }
   }
  }
  const float winding=pose[5].overhead;
  for(int p=0;p<6;p++)pose[p]=turn_art::interpolate(pose[p],rest[p],death.blend);
  pose[5].overhead=winding;
  return pose;
 }
 const float fold=curve(jackal_fold,fall);
 const float drop=std::max(0.f,curve(jackal_drop,fall));
 const float buckle=curve(jackal_buckle,fall);
 const float heading=turn_art::heading(view);
 const float fore=std::cos(heading),depth=std::abs(std::sin(heading));
 const float pawSide=std::clamp(1.7f*fore,-1.f,1.f);
 const float tuck=pawSide+side*(1-std::abs(pawSide));
 auto& trunk=rest[3];
 trunk.shift={side*drop*3.f,buckle*6.f};
 trunk.scaleX=1+.025f*drop;
 trunk.scaleY=1-.02f*drop;
 trunk.angle=tuck*(buckle*12.f+drop*3.f);
 for(int p=0;p<6;p++){
  auto& limb=rest[p];
  if(p!=3)carry(limb,trunk,1);
  if(p==0){
   limb.angle+=tuck*(buckle*9.f+drop*12.f);
   limb.shift.y+=buckle*6.f+drop*(3.f+6.f*depth);
  }else if(p!=3){
   const bool near=p==2||p==5;
   limb.angle+=tuck*(p<3?-1.f:1.f)*(drop*(near?55.f:72.f)+buckle*(p<3?10.f:53.f));
   limb.scaleY*=1-drop*(near?.15f:.26f)-buckle*(p<3?.04f:.20f);
   limb.shift.y-=drop*(near?3.f:6.f);
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
 const auto paintedPalm=turn_art::reaperJoint(rig.angle,3);
 const float facing=std::cos(turn_art::heading(rig.angle));
 const float wristProjection=arm.overhead!=0?arm.overhead:facing*.81f+.19f;
 const float wrist=(motion.wrist*wristProjection+arm.wristCounterturn)*(1-fold);
 const float width=.39f*(.86f+.14f*std::abs(facing))*(1-.37f*std::clamp(motion.roll*(1-fold),0.f,1.f));
 return {rig.partPoint(5,paintedPalm),
         arm.angle+(rig.sickleArm.active?arm.elbow:elbow.angle*RAD2DEG)+19*facing+wrist,
         width,rig.inFrontOf(5,3)};
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
 if(x<0||x>=96||y<0||y>=96)return false;
 const float u=std::clamp(x-.5f,0.f,95.f),v=std::clamp(y-.5f,0.f,95.f);
 const int i=int(u),j=int(v),right=std::min(i+1,95),bottom=std::min(j+1,95);
 const float top=turn_art::mix(sickleAlpha[j*96+i],sickleAlpha[j*96+right],u-i);
 const float lower=turn_art::mix(sickleAlpha[bottom*96+i],sickleAlpha[bottom*96+right],u-i);
 return turn_art::mix(top,lower,v-j)>=111;
}

struct FallProjection {
 float hip,rotation,squash,fit=0;
 Vector2 point(Vector2 local)const{
  const auto q=rotated({local.x,(local.y-hip)*squash},rotation);
  return {q.x,q.y+hip+fit};
 }
 Vector2 local(Vector2 screen)const{
  const auto q=rotated({screen.x,screen.y-fit-hip},-rotation);
  return {q.x,q.y/squash+hip};
 }
};

inline float fitFallingBody(const turn_art::Rig& rig,const FallProjection& projection,
                            float blend,float settle){
 if(blend<=0&&settle<=0)return 0;
 float bottom=-1e6f;
 const int frame=rig.angle.nearestSample();
 for(int p=0;p<6;++p){
  const auto rect=rig.partBox[p];
  for(const auto point:rig.art.contactContours[frame*6+p]){
   const auto local=rig.partPoint(p,{rect.x+point.x*rect.width,rect.y+point.y*rect.height});
   bottom=std::max(bottom,projection.point(local).y);
  }
 }
 return (1.4f-bottom)*(bottom>1.4f?blend:enemy_death::ease(settle));
}

struct DroppedSickle {
 WeaponPose pose;
 float squash;
};
inline constexpr float GroundSickleSquash=.55f;

inline WeaponPose projectedSickle(const turn_art::Rig& rig,SickleMotion motion,float fold,
                                  const FallProjection& projection){
 auto held=weaponPose(rig,motion,fold);
 held.palm=projection.point(held.palm);
 held.angle+=projection.rotation;
 return held;
}

inline WeaponPose sickleLanding(const turn_art::Rig& corpse,const FallProjection& projection,
                                 SickleMotion motion,float side){
 const auto palm=projectedSickle(corpse,motion,1,projection).palm;
 const float angle=side>0?82.f:-98.f;
 WeaponPose best{};
 int mostVisible=-1;
 for(const auto offset:{Vector2{-side*7.5f,8.5f},{-side*10.5f,10.3f},{side*6.f,12.4f}}){
  for(float turn:{0.f,-45.f,45.f,-90.f,90.f,180.f}){
   const WeaponPose candidate{{palm.x+offset.x,palm.y+offset.y},angle+turn,.39f,true};
   const float c=std::cos(candidate.angle*DEG2RAD),s=std::sin(candidate.angle*DEG2RAD);
   int visible=0;
   for(int y=14;y<94;y+=4)for(int x=10;x<78;x+=4){
    if(sickleAlpha[y*96+x]<174)continue;
    const float u=(x-Grip.x)*candidate.width,v=(y-Grip.y)*SickleHeightScale;
    const Vector2 screen{candidate.palm.x+c*u-s*v,
                         candidate.palm.y+(s*u+c*v)*GroundSickleSquash};
    if(!corpse.hit(projection.local(screen))&&++visible>=14)return candidate;
   }
   if(visible>mostVisible){mostVisible=visible;best=candidate;}
  }
 }
 return best;
}

inline WeaponPose reaperFallGrip(turn_art::View view,const AnimationState& state,float attack,
                                 float hit,float side,SickleMotion motion,float phase,bool restingWeapon=false){
 const auto pose=ember::reaper_motion::sampleFall(phase);
 const auto parts=collapsePose(turn_art::reaper,view,state,attack,hit,phase,side,motion,pose);
 const turn_art::Rig rig(turn_art::reaper,view,0,parts,WHITE,side,attack);
 FallProjection projection{-44.f,side*pose.roll,1-.025f*pose.ground};
 projection.fit=fitFallingBody(rig,projection,pose.blend,pose.ground);
 return restingWeapon?sickleLanding(rig,projection,motion,side):
                      projectedSickle(rig,motion,pose.blend,projection);
}

inline DroppedSickle fallingSickle(turn_art::View view,const AnimationState& state,float attack,
                                  float hit,float side,SickleMotion motion,float phase,
                                  const turn_art::Rig& current,const FallProjection& projection){
 using namespace ember::reaper_motion;
 const auto ground=phase>=1?sickleLanding(current,projection,motion,side):
                            reaperFallGrip(view,state,attack,hit,side,motion,1,true);
 if(phase>=1)return {ground,GroundSickleSquash};

 // Carry the grip's momentum through release in the same screen projection.
 constexpr float Delta=1.f/120,Flight=1-SickleRelease;
 const auto hand=reaperFallGrip(view,state,attack,hit,side,motion,SickleRelease);
 const auto previous=reaperFallGrip(view,state,attack,hit,side,motion,SickleRelease-Delta);
 const float t=std::clamp((phase-SickleRelease)/Flight,0.f,1.f);
 const auto drift=[t](float distance,float velocity){
  // Limit the initial velocity to avoid doubling back before contact.
  const float limit=distance*1.8f;
  const float start=std::clamp(velocity,std::min(0.f,limit),std::max(0.f,limit));
  return distance*t+(start-distance)*t*(1-t)*(1-t);
 };
 const float dx=ground.palm.x-hand.palm.x,dy=ground.palm.y-hand.palm.y;
 const float vx=(hand.palm.x-previous.palm.x)*(Flight/Delta);
 const float vy=std::clamp((hand.palm.y-previous.palm.y)*(Flight/Delta),
                          -2.5f,.70f*std::max(0.f,dy));
 const float spin=std::remainder(ground.angle-hand.angle,360.f);
 const float angular=std::remainder(hand.angle-previous.angle,360.f)*(Flight/Delta);
 const float flatten=enemy_death::window(t,.45f,1.f);
 return {{{hand.palm.x+drift(dx,vx),hand.palm.y+vy*t+(dy-vy)*t*t},
           hand.angle+drift(spin,angular),turn_art::mix(hand.width,ground.width,flatten),true},
          turn_art::mix(1.f,GroundSickleSquash,flatten)};
}

inline void paintDroppedSickle(DroppedSickle dropped,Vector2 origin,float size,Color color,
                               bool mask,bool shadow){
 rlPushMatrix();
 rlTranslatef(origin.x+size*dropped.pose.palm.x,origin.y+size*dropped.pose.palm.y,0);
 rlScalef(size,size*dropped.squash,1);
 dropped.pose.palm={};
 paintWeapon(dropped.pose,color,mask,shadow);
 rlPopMatrix();
}

inline void relaxReaperLegs(std::array<Part,6>& parts,turn_art::View view,
                            const ember::reaper_motion::LegSettle& motion){
 const auto soleX=[&](int p){
  const auto& leg=parts[p];
  const auto sole=turn_art::footTip(turn_art::reaper,view,p);
  const auto knee=turn_art::filtered()?turn_art::reaperKnee(leg.pivot,sole,leg.elbow):turn_art::SwordJoint{};
  return turn_art::rigidPartPoint(leg,knee,sole).x;
 };
 const float outward=soleX(1)>=soleX(2)?1.f:-1.f;
 parts[1].angle-=outward*motion.leg_splay;
 parts[2].angle+=outward*motion.leg_splay;
 parts[1].restingFootTurn=-outward*motion.boot_turn;
 parts[2].restingFootTurn=outward*motion.boot_turn;
}

inline float reaperLegSettlePhase(const Enemy& e){
 if(!e.dying())return 1.f;
 return std::clamp((e.deathElapsed-enemy_death::groundContactTime(e.type))/
                    ember::reaper_motion::LegSettleSeconds,0.f,1.f);
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
 const auto death=fallen&&!jackal?ember::reaper_motion::sampleFall(fall):ember::reaper_motion::Fall{};
 const float drop=fallen&&jackal?curve(jackal_drop,fall):death.ground;
 const float hit=std::clamp((fallen?e.deathFlash:e.flash)/.16f,0.f,1.f);
 const auto motion=jackal?SickleMotion{}:sickleMotion(state,attack);
 const auto parts=fallen?collapsePose(art,view,state,attack,hit,fall,side,motion,death):
                         livingPose(art,view,state,attack,hit,motion);
 turn_art::Rig contactRig(art,view,0,parts,WHITE,side,attack);
 const float roll=fallen&&jackal?curve(jackal_roll,fall):death.roll;
 const float heading=turn_art::heading(view);
 const float endOn=std::abs(std::sin(heading));
 const float profile=std::clamp(std::cos(heading)*2.f,-1.f,1.f);
 const float jackalFlank=profile+side*(1-std::abs(profile));
 const float rotation=roll*(jackal?jackalFlank*(22.f+26.f*endOn):side);
 const float sy=1-std::max(0.f,drop)*(jackal?.07f:.025f);
 const float hip=jackal?-29.f:-44.f;
 FallProjection projection{hip,rotation,sy};
 if(fallen&&(drop>0||(!jackal&&death.blend>0))){
  const float settle=jackal?std::clamp((drop-.08f)*1.39f,0.f,1.f):death.ground;
  projection.fit=fitFallingBody(contactRig,projection,jackal?1.f:death.blend,settle);
 }
 const float size=scale*(jackal?1.f:ReaperScale);
 const bool armed=!jackal&&sickle.id;
 const bool released=armed&&fallen&&fall>=ember::reaper_motion::SickleRelease;
 const bool holding=armed&&!released;
 const auto weapon=holding?weaponPose(contactRig,motion,death.blend):WeaponPose{};
 const auto dropped=released?fallingSickle(view,state,attack,hit,side,motion,fall,contactRig,projection):
                             DroppedSickle{};
 std::array<Part,6> relaxedParts;
 std::optional<turn_art::Rig> relaxedRig;
 if(!jackal&&fall>=1){
  const float settle=reaperLegSettlePhase(e);
  if(settle>0){
   // The body and sickle keep their contact registration while the legs relax.
   relaxedParts=parts;
   relaxReaperLegs(relaxedParts,view,ember::reaper_motion::sampleLegSettle(settle));
   relaxedRig.emplace(art,view,0,relaxedParts,WHITE,side,attack);
  }
 }
 auto& rig=relaxedRig?*relaxedRig:contactRig;
 const bool heldInRig=holding&&turn_art::filtered()&&art.signedEdges;
 if(heldInRig)
  rig.hold(sickle,weapon.palm,{Grip.x/96,Grip.y/96},{96*weapon.width,96*SickleHeightScale},weapon.angle);
 if(probe){
  if(opacity<.43f)return false;
  const Vector2 screen{(probe->x-x)/size,(probe->y-y)/size};
  if(released){
   const float height=dropped.pose.palm.y+(screen.y-dropped.pose.palm.y)/dropped.squash;
   if(hitWeapon(dropped.pose,{screen.x,height}))return true;
  }
  const auto q=projection.local(screen);
  return rig.hit(q)||(holding&&hitWeapon(weapon,q));
 }
 rlPushMatrix();
 rlTranslatef(x,y+size*(hip+projection.fit),0);
 rlScalef(size,size,1);
 rlRotatef(rotation,0,0,1);
 rlScalef(1,sy,1);
 rlTranslatef(0,-hip,0);
 constexpr std::array<Vector2,4> rim{{{1.4f,0},{-1.4f,0},{0,1.4f},{0,-1.4f}}};
 if(holding&&!heldInRig&&outline.a&&!shadow)for(auto q:rim)paintWeapon(weapon,Fade(outline,opacity),true,false,q);
 if(outline.a&&!shadow){
  rig.begin({},0);
  for(auto q:rim)rig.paint(true,Fade(outline,opacity),q.x,q.y);
  if(turn_art::filtered())EndShaderMode();
 }
 const unsigned char shade=static_cast<unsigned char>(255-std::max(0.f,drop)*(jackal?18:9));
 const Color tint=shadow?WHITE:Color{255,shade,shade,255};
 if(holding&&!heldInRig&&!weapon.front)paintWeapon(weapon,Fade(tint,opacity),false,shadow);
 rig.begin({},0);
 rig.paint(shadow,Fade(tint,opacity),0,0,shadow);
 if(turn_art::filtered())EndShaderMode();
 if(holding&&!heldInRig&&weapon.front)paintWeapon(weapon,Fade(tint,opacity),false,shadow);
 rlPopMatrix();
 if(released){
  if(outline.a&&!shadow)for(auto q:rim)
   paintDroppedSickle(dropped,{x+size*q.x,y+size*q.y},size,Fade(outline,opacity),true,false);
  paintDroppedSickle(dropped,{x,y},size,Fade(tint,opacity),false,shadow);
 }
 return false;
}
}
