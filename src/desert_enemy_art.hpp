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
 const auto a=art.tile(view.sample,slot,0),b=art.tile(view.nextSample(),slot,0);
 const auto endX=[&](turn_art::Tile t,int frame){
  const auto& silhouette=art.contactContours[frame*6+slot];
  return t.ox+t.w*(silhouette[22].x+silhouette[23].x)*.5f;
 };
 const Vector2 rest{turn_art::mix(endX(a,view.sample),endX(b,view.nextSample()),view.mix)*.8f-limb.pivot.x,
                    std::max(12.f,turn_art::mix(a.oy+a.h,b.oy+b.h,view.mix)*.8f-limb.pivot.y)};
 const Vector2 goal{rest.x+target.x-limb.shift.x,rest.y+target.y-limb.shift.y};
 limb.angle=std::atan2(rest.x*goal.y-rest.y*goal.x,rest.x*goal.x+rest.y*goal.y)*RAD2DEG;
 limb.scaleY=std::clamp(std::hypot(goal.x,goal.y)/std::hypot(rest.x,rest.y),.74f,1.16f);
 limb.scaleX=std::max(.93f,1-.12f*(1-limb.scaleY));
}

inline std::array<Part,6> livingPose(const turn_art::Artwork& art,const turn_art::View& view,
                                     const AnimationState& state,float attack,float hit){
 const bool jackal=&art==&turn_art::jackal;
 const auto& sockets=jackal?dune_jackal_turn_atlas::pivots:glass_reaper_turn_atlas::pivots;
 const float heading=turn_art::heading(view);
 const Vector2 facing{std::cos(heading),std::sin(heading)};
 const auto headingOnGround=actorFacing(heading);
 const bool hasTravel=std::abs(state.travelAxis.x)+std::abs(state.travelAxis.y)>.01f;
 const Vector2 travel=hasTravel?Vector2{state.travelAxis.x,state.travelAxis.y*ActorGroundSquash}:
                               Vector2{headingOnGround.x,headingOnGround.y*ActorGroundSquash};
 const float gait=(state.motionInitialized?state.moveBlend:(state.moving?1.f:0.f))*
                  (attack<0?1.f:std::max(0.f,1-attack*7));
 const float walk=state.phase, wave=2*PI*walk;
 const float breathe=std::sin(state.idleTime*(jackal?4.6f:2.8f))*(1-gait);
 float bodyY=.72f*breathe-gait*(1.3f+.92f*std::cos(2*wave-.55f));
 float dash=0,crouch=0,neck=0,sweep=0,twist=0;
 float elbow=0;
 if(attack>=0){
  if(jackal){
   crouch=curve(jackal_crouch,attack);
   dash=curve(jackal_pounce,attack);
   neck=curve(jackal_neck,attack);
   bodyY+=9.1f*crouch-9.2f*std::max(0.f,dash);
  }else{
   crouch=curve(reaper_crouch,attack);
   sweep=curve(reaper_sickle,attack);
   twist=curve(reaper_turn,attack);
   elbow=curve(reaper_elbow,attack);
   bodyY+=5.2f*crouch-1.8f*twist;
  }
 }
 std::array<Part,6> parts{};
 for(int p=0;p<6;p++){
  const auto& a=sockets[view.base][p];
  const auto& b=sockets[view.nextBase()][p];
  parts[p].pivot={.8f*turn_art::mix(a[0],b[0],view.t),.8f*turn_art::mix(a[1],b[1],view.t)};
 }
 auto& trunk=parts[3];
 trunk.shift={facing.x*(dash*6-crouch*1.2f-hit*3)+gait*std::sin(wave-.45f)*1.27f+twist*3.8f,
              bodyY+facing.y*dash*1.9f+hit*1.9f};
 trunk.angle=facing.x*(gait*std::sin(2*wave-.4f)*(jackal?3.7f:2.8f)+neck*4.f)+twist*7.6f-state.turnLean*gait*3;
 auto& head=parts[0];
 carry(head,trunk,1);
 head.angle=trunk.angle*.67f+facing.x*(neck*10-gait*std::sin(2*wave+.28f)*4.7f)-twist*4.9f;
 head.shift.x+=facing.x*dash*3.7f;
 head.shift.y+=neck*2.4f-gait*.6f*std::sin(wave-.52f);
 for(const int p:{1,2,4,5}){
  auto& limb=parts[p];
  const bool left=p==1||p==4;
  if(jackal||p<3){
   constexpr float pawPhase[]={0,.53f,0,0,.80f,.28f};
   const float phase=loop(walk+(jackal?pawPhase[p]:(left?0.f:.5f)));
   const float reach=curve(jackal?jackal_stride:reaper_stride,phase)*gait*(jackal?12.2f:11.1f);
   const float lift=curve(jackal?jackal_lift:reaper_lift,phase)*gait*(jackal?8.9f:7.8f);
   carry(limb,trunk,jackal?.86f:.74f);
   const float plant=jackal?dash*(p>=4?12.f:-3.8f):p==2?std::max(0.f,twist)*7.f:0.f;
   const float kick=jackal&&p>=4?std::max(0.f,dash)*6.4f:0.f;
   step(limb,art,view,p,{travel.x*reach+facing.x*plant,travel.y*reach-lift+facing.y*plant*.42f-kick});
   continue;
  }
  const float stride=std::sin(wave+(left?PI:0.f)-.4f)*gait;
  const float side=std::clamp(facing.x*.78f+.30f,-.6f,1.f);
  carry(limb,trunk,1);
  limb.angle=trunk.angle+stride*(7+13*std::abs(facing.x))+(left?twist*22:-sweep*(38+43*std::abs(side))*side);
  limb.elbow=(left?-1.f:1.f)*(7*gait+stride*5)+
             (left?twist*16:(elbow*38+twist*4)*side);
 }
 return parts;
}

inline std::array<Part,6> collapsePose(const turn_art::Artwork& art,const turn_art::View& view,
                                       const AnimationState& state,float attack,float fall,float side){
 const bool jackal=&art==&turn_art::jackal;
 auto pose=livingPose(art,view,state,attack,0);
 auto rest=livingPose(art,view,AnimationState{},-1,0);
 const float fold=curve(jackal?jackal_fold:reaper_fold,fall);
 const float drop=std::max(0.f,curve(jackal?jackal_drop:reaper_drop,fall));
 const float heading=turn_art::heading(view);
 const float fore=std::cos(heading),depth=std::abs(std::sin(heading));
 const float pawSide=std::clamp(1.7f*fore,-1.f,1.f);
 const float tuck=pawSide+side*(1-std::abs(pawSide));
 auto& trunk=rest[3];
 trunk.scaleX=1+.05f*drop;
 trunk.scaleY=1-.035f*drop;
 trunk.angle=side*drop*(jackal?8.f:-5.f);
 for(int p=0;p<6;p++){
  auto& limb=rest[p];
  if(p!=3)carry(limb,trunk,1);
  if(p==0){
   limb.angle+=side*(jackal?-15.f:12.f)*drop;
   limb.shift.y+=drop*(jackal?8.f+8.f*depth:4.f);
   if(jackal)limb.shift.x+=side*drop*4.f*depth;
  }else if(p!=3){
   const bool near=p==2||p==5;
   const float sign=near?1.f:-1.f;
   if(jackal){
    limb.angle+=drop*tuck*(p<3?-1.f:1.f)*(near?58.f:73.f);
    limb.scaleY*=1-drop*(near?.20f:.31f);
    limb.shift.x+=side*drop*(near?3.f:-3.f);
    limb.shift.y-=drop*(near?3.f:6.f);
   }else{
    limb.angle+=side*sign*(p<3?29.f:25.f)*drop;
    limb.scaleY*=1-drop*.09f;
    limb.shift.x+=side*drop*(near?3.f:-1.f);
    limb.shift.y+=drop*3.f;
   }
   if(!jackal&&p>=4)limb.elbow=side*(near?-25.f:28.f)*drop;
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

inline WeaponPose weaponPose(const turn_art::Rig& rig,float attack,float fold){
 const auto& arm=rig.parts[5];
 const auto& elbow=rig.arms[1];
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
 const float wrist=attack>=0?curve(reaper_elbow,attack)*-14.f:0;
 return {{arm.pivot.x+arm.shift.x+local.x-64,arm.pivot.y+arm.shift.y+local.y-115},
         arm.angle+elbow.angle*RAD2DEG+19*facing+wrist+fold*12,
         .36f*(.78f+.22f*std::abs(facing)),facing>-.16f||(attack>.34f&&attack<.66f)};
}
inline void paintWeapon(WeaponPose weapon,Color tint,bool mask,bool shadow,Vector2 offset={}){
 if(mask||shadow){
  if(sickleShadow<0)return;
  BeginShaderMode(sickleMask);
  const int mode=shadow;
  SetShaderValue(sickleMask,sickleShadow,&mode,SHADER_UNIFORM_INT);
 }
 constexpr float heightScale=.36f;
 DrawTexturePro(sickle,{0,0,96,96},{weapon.palm.x+offset.x,weapon.palm.y+offset.y,96*weapon.width,96*heightScale},
                {Grip.x*weapon.width,Grip.y*heightScale},weapon.angle,tint);
 if(mask||shadow)EndShaderMode();
}
inline bool hitWeapon(WeaponPose weapon,Vector2 q){
 const auto local=rotated({q.x-weapon.palm.x,q.y-weapon.palm.y},-weapon.angle);
 const float x=local.x/weapon.width+Grip.x,y=local.y/.36f+Grip.y;
 return x>=0&&x<96&&y>=0&&y<96&&sickleAlpha[int(y)*96+int(x)]>=111;
}

inline bool draw(const Enemy& e,float x,float y,Color outline,const Vector2* probe,bool shadow,
                 float scale,float opacity=1,float fall=-1){
 const bool jackal=e.type==DuneJackalType;
 const auto& art=jackal?turn_art::jackal:turn_art::reaper;
 if(!art.ready()||opacity<=0)return false;
 const bool fallen=fall>=0;
 auto state=fallen?e.deathMotion:e.animation;
 if(!state.facingInitialized&&fallen)state=e.animation;
 const float attack=fallen?e.deathAction:e.volcanoAction();
 const auto view=turn_art::view(state,art);
 const float side=e.deathScreenSide()?float(e.deathScreenSide()):std::cos(turn_art::heading(view))>=0?1.f:-1.f;
 const float drop=fallen?curve(jackal?jackal_drop:reaper_drop,fall):0;
 const auto parts=fallen?collapsePose(art,view,state,attack,fall,side):
                         livingPose(art,view,state,attack,std::clamp(e.flash/.16f,0.f,1.f));
 turn_art::Rig rig(art,view,0,parts,WHITE,side,attack);
 const float roll=fallen?curve(jackal?jackal_roll:reaper_roll,fall):0;
 const float heading=turn_art::heading(view);
 const float endOn=std::abs(std::sin(heading));
 const float profile=std::clamp(std::cos(heading)*2.f,-1.f,1.f);
 const float jackalFlank=profile+side*(1-std::abs(profile));
 const float rotation=roll*(jackal?jackalFlank*(22.f+26.f*endOn):side*68.f);
 const float sy=1-std::max(0.f,drop)*(jackal?.12f:.035f);
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
     if(p>=4)center=rig.arms[p-4].map(center,false);
     const auto offset=rotated({(center.x-k.pivot.x)*k.scaleX,(center.y-k.pivot.y)*k.scaleY},k.angle);
     const auto q=project(k.pivot.x+k.shift.x-64+offset.x,k.pivot.y+k.shift.y-115+offset.y);
     edge=std::max(edge,q.y);
    }
    return edge;
   };
   bottom=std::max(bottom,turn_art::mix(support(view.sample),support(view.nextSample()),view.mix));
  }
  fit=(1.7f-bottom)*std::clamp(drop*2.f,0.f,1.f);
 }
 const float size=scale*(jackal?1.f:1.05f);
 const bool armed=!jackal&&sickle.id;
 auto weapon=armed?weaponPose(rig,fallen?-1:attack,std::max(0.f,drop)):WeaponPose{};
 if(armed&&fallen){
  const float groundAngle=(side>0?63.f:-117.f)-rotation;
  const float settle=curve(reaper_fold,fall)*std::max(0.f,drop);
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
