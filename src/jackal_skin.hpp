#pragma once

namespace jackal_skin {
using caldera_quadruped::add;
using caldera_quadruped::cross;
using caldera_quadruped::length;
using caldera_quadruped::mix;
using caldera_quadruped::mul;
using caldera_quadruped::sub;
using namespace ember::desert_enemy_motion;

inline constexpr int Width=160,Height=144,Texels=2,Columns=4,Directions=8,LegBones=4;
inline constexpr int LegInfluences=4*LegBones,Influences=LegInfluences+2;
inline constexpr Vector2 Origin{80,119};
using Leg=std::array<Vector2,LegBones+1>;
struct Vertex {Vector2 point;std::array<unsigned char,Influences> weights;float mobility;};
struct Skin {
 std::array<Leg,4> legs;
 std::array<Vector2,3> head;
 std::vector<Vertex> vertices;
 std::vector<unsigned short> faces;
 float center=0,width=1;
};
inline std::array<Skin,Directions> skins;
inline Texture2D ink{};
inline Shader flat{};
inline std::vector<unsigned char> alpha;

inline bool readSkins(){
 int bytes=0;
 auto* data=LoadFileData("assets/desert/jackal-turn/painted-skin.bin",&bytes);
 if(!data)return false;
 const bool versionMatches=bytes>=4&&std::memcmp(data,"JSK3",4)==0;
 size_t pos=4;
 const auto take=[&](auto& value){
  if(pos+sizeof(value)>size_t(bytes))return false;
  std::memcpy(&value,data+pos,sizeof(value));
  pos+=sizeof(value);
  return true;
 };
 const auto point=[&](Vector2& value){
  return take(value.x)&&take(value.y)&&std::isfinite(value.x)&&std::isfinite(value.y)&&
         value.x>=0&&value.x<=Width&&value.y>=0&&value.y<=Height;
 };
 const auto read=[&](Skin& skin){
  unsigned short vertices=0,faces=0;
  if(!take(vertices)||!take(faces)||vertices<12||vertices>1600||faces<3||faces>7500||faces%3)return false;
  for(auto& leg:skin.legs)for(auto& node:leg)if(!point(node))return false;
  for(auto& node:skin.head)if(!point(node))return false;
  skin.vertices.resize(vertices);
  skin.faces.resize(faces);
  for(auto& node:skin.vertices){
   unsigned char x=0,y=0;
   if(!take(x)||!take(y)||x>Width||y>Height)return false;
   for(auto& weight:node.weights)if(!take(weight))return false;
   node.point={float(x),float(y)};
   node.mobility=*std::max_element(node.weights.begin(),node.weights.end())/255.f;
  }
  for(auto& index:skin.faces)if(!take(index)||index>=vertices)return false;
  for(size_t offset=0;offset<skin.faces.size();offset+=3){
   const auto a=skin.vertices[skin.faces[offset]].point;
   const auto b=skin.vertices[skin.faces[offset+1]].point;
   const auto c=skin.vertices[skin.faces[offset+2]].point;
   if(cross(sub(b,a),sub(c,a))>=-1.f)return false;
  }
  return true;
 };
 const bool valid=versionMatches&&std::all_of(skins.begin(),skins.end(),read)&&pos==size_t(bytes);
 UnloadFileData(data);
 if(!valid)skins={};
 return valid;
}

inline void load(){
 if(ink.id||!FileExists("assets/desert/jackal-turn/painted-views.png")||!readSkins())return;
 Image image=LoadImage("assets/desert/jackal-turn/painted-views.png");
 if(!image.data)return;
 if(image.width!=Width*Texels*Columns||image.height!=2*Height*Texels){UnloadImage(image);return;}
 ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
 const auto* color=static_cast<const Color*>(image.data);
 alpha.resize(size_t(image.width)*image.height);
 for(size_t i=0;i<alpha.size();i++)alpha[i]=color[i].a;
 for(int view=0;view<Directions;view++){
  const int left=view%Columns*Width*Texels,top=view/Columns*Height*Texels;
  int first=Width*Texels,last=-1;
  for(int y=0;y<Height*Texels;y++)for(int x=0;x<Width*Texels;x++){
   if(alpha[(top+y)*image.width+left+x]<115)continue;
   first=std::min(first,x);
   last=std::max(last,x);
  }
  if(last<=first){UnloadImage(image);alpha.clear();skins={};return;}
  skins[view].center=(first+last+1)/(2.f*Texels)-Origin.x;
  skins[view].width=(last-first+1)/float(Texels);
 }
 ink=LoadTextureFromImage(image);
 UnloadImage(image);
 if(!ink.id){alpha.clear();return;}
 GenTextureMipmaps(&ink);
 SetTextureFilter(ink,TEXTURE_FILTER_TRILINEAR);
 SetTextureFilter(ink,TEXTURE_FILTER_ANISOTROPIC_4X);
 flat=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
out vec4 finalColor;
void main(){
 float a=texture(texture0,fragTexCoord).a;
 if(a<.004)discard;
 finalColor=vec4(fragColor.rgb,a*fragColor.a);
}
)GLSL");
}
inline bool ready(){return ink.id!=0;}
inline void unload(){
 if(ink.id)UnloadTexture(ink);
 if(flat.id&&flat.id!=rlGetShaderIdDefault())UnloadShader(flat);
 flat={};ink={};alpha.clear();skins={};
}

inline float curve(const float (&track)[61],float phase){
 const float frame=std::clamp(phase,0.f,1.f)*60.f;
 const int at=std::min(int(frame),59);
 return mix(track[at],track[at+1],frame-at);
}
inline Vector2 rotate(Vector2 p,float angle){
 return {std::cos(angle)*p.x-std::sin(angle)*p.y,std::sin(angle)*p.x+std::cos(angle)*p.y};
}
inline float cycle(float phase){return phase-std::floor(phase);}
inline int direction(float angle){
 int nearest=0;
 float distance=PI;
 for(int i=0;i<Directions;i++){
  const float gap=std::abs(std::remainder(angle-facingAngles[i],2*PI));
  if(gap<distance){nearest=i;distance=gap;}
 }
 return nearest;
}
struct Turn {
 const Skin& from;
 const Skin& to;
 float blend,center,width,scale;
 Vector2 projected(const Skin& skin,Vector2 point)const{
  return {(point.x-Origin.x-skin.center)*width/skin.width+center,point.y-Origin.y};
 }
 Vector2 offset(const Skin& skin,Vector2 point,Vector2 a,Vector2 b)const{
  const auto expected=mix(projected(from,a),projected(to,b),blend);
  const auto delta=sub(expected,projected(skin,point));
  return {delta.x/scale,delta.y};
 }
};
inline Turn registration(float heading,int view){
 float angle=std::fmod(heading,2*PI);
 if(angle<0)angle+=2*PI;
 const int base=int(std::upper_bound(facingAngles+1,facingAngles+Directions,angle)-facingAngles)-1;
 const auto& a=skins[base];
 const auto& b=skins[(base+1)%Directions];
 const float end=base==Directions-1?2*PI:facingAngles[base+1];
 const float t=(angle-facingAngles[base])/(end-facingAngles[base]);
 const float width=mix(a.width,b.width,t);
 return {a,b,t,mix(a.center,b.center,t),width,width/skins[view].width};
}

struct Pose {
 int view;
 float heading,phase,gait,lean,bob,drive,leap,neck,bite,fall,fold,roll,side,shoulder,headWalk,tail;
 Vector2 travel;
};
inline Pose pose(const Enemy& e,float fall){
 const bool dead=fall>=0;
 const auto& a=dead&&e.deathMotion.facingInitialized?e.deathMotion:e.animation;
 const float heading=a.facingInitialized?a.facingAngle:facingAngles[int(a.direction)];
 const auto axis=actorFacing(heading);
 const auto velocity=a.travelAxis.x==0&&a.travelAxis.y==0?axis:a.travelAxis;
 const float attack=dead?e.deathAction:e.volcanoAction();
 const float fold=dead?curve(jackal_fold,fall):0;
 const float drop=dead?curve(jackal_drop,fall):0;
 const float roll=dead?curve(jackal_roll,fall):0;
 const float prep=std::clamp(attack/.24f,0.f,1.f);
 const float gait=(a.motionInitialized?a.moveBlend:float(a.moving))*(1-prep*prep*(3-2*prep))*(1-fold);
 const float hit=std::clamp((dead?e.deathFlash:e.flash)/.16f,0.f,1.f)*(1-fold);
 const float crouch=attack>=0?curve(jackal_crouch,attack)*(1-fold):0.f;
 const float drive=attack>=0?curve(jackal_pounce,attack)*(1-fold):0.f;
 const float leap=attack>=0?curve(jackal_land,attack)*(1-fold):0.f;
 const float neck=attack>=0?curve(jackal_neck,attack)*(1-fold):0.f;
 const float bite=attack>=0?curve(jackal_bite,attack)*(1-fold):0.f;
 const float balance=curve(jackal_balance,a.phase);
 const float incline=(balance*.047f+a.turnLean*.007f)*gait-std::cos(heading)*.043f*neck;
 const int side=e.deathScreenSide();
 return {direction(heading),heading,a.phase,gait,incline,
         gait*(1.35f+1.74f*curve(jackal_weight,a.phase))+crouch*6.25f-leap*9.8f+hit*1.7f,
         drive*6.6f-hit*2.5f,
         leap,neck,bite,drop,fold,roll,side?float(side):axis.x>=0?1.f:-1.f,
         balance*gait*2.4f,curve(jackal_head_walk,a.phase)*gait,
         curve(jackal_tail_walk,cycle(a.phase-.085f))*gait,
         {velocity.x,velocity.y*ActorGroundSquash}};
}

inline float footPhase(float phase,int slot){
 // The hind paw lands first, followed by the fore on the same side.
 return cycle(phase+(slot%2?.5f:0.f)+(slot>=2?.75f:0.f));
}
inline float supportFraction(int slot){return slot>=2?40.f/60.f:37.f/60.f;}

struct FootStep {float stride,lift,flex;};
inline FootStep footStep(const Pose& p,int slot){
 const float phase=footPhase(p.phase,slot),support=supportFraction(slot);
 constexpr float curveSupport=38.f/60.f;
 const float t=phase<support?phase/support*curveSupport:
               mix(curveSupport,1.f,(phase-support)/(1-support));
 const bool fore=slot>=2;
 return {curve(jackal_stride,t)*p.gait*desert_motion::JackalCycle*support*.50f,
         curve(jackal_lift,t)*p.gait*(fore?7.8f:6.9f),
         curve(fore?jackal_fore_flex:jackal_hind_flex,t)*p.gait};
}

inline Leg legPose(const Skin& skin,const Pose& p,int slot,const Turn& turn){
 const auto& rest=skin.legs[slot];
 const auto& other=skin.legs[slot^1];
 const bool fore=slot>=2;
 auto [stride,lift,flex]=footStep(p,slot);
 // In the painted side view, one foreleg is largely covered by the other.
 // Keep their shared painted edge together and let the visible leg lead.
 const float gap=length(sub(rest[3],other[3]))*turn.scale;
 const float hidden=std::clamp((8.f-gap)/5.f,0.f,1.f)*(rest.back().y<other.back().y?.89f:.12f);
 if(hidden>0){
  const auto neighbor=footStep(p,slot^1);
  stride=mix(stride,neighbor.stride,hidden);
  lift=mix(lift,neighbor.lift,hidden);
  flex=mix(flex,neighbor.flex,hidden);
 }
 const auto forward=actorFacing(p.heading);
 // Keep each paw in its own track. A lateral component from a sharp turn
 // should not pull a front leg across the chest or a hind leg across the tail.
 const float alignment=std::clamp(p.travel.x*forward.x+p.travel.y*forward.y/ActorGroundSquash,.55f,1.f);
 const float swingX=forward.x*alignment;
 const Vector2 reach{swingX*(stride+p.drive*(fore?1.14f:-.48f)),
                     p.travel.y*stride-lift-p.leap*(fore?5.f:2.9f)};
 const auto original=turn.projected(skin,rest.back());
 const Vector2 ribs{turn.center,-44.f};
 // Undo the torso's bounce and lean at the toe during stance. The y motion
 // left over is the depth of the step on the isometric ground plane.
 const auto onGround=sub(add(original,reach),{p.shoulder*std::cos(p.heading),p.bob});
 const auto relative=sub(add(ribs,rotate(sub(onGround,ribs),-p.lean)),original);
 const Vector2 step{relative.x/turn.scale,relative.y};
 const float pitch=flex*(fore?.32f:.42f)*swingX+p.fold*p.side*.43f;
 const auto toeSegment=rotate(sub(rest[4],rest[3]),pitch);

 Leg leg=rest;
 leg[4]=mix(add(rest[4],step),add(mix(rest[4],rest[0],.65f),{p.side*(fore?-7.7f:8.4f),2.1f}),p.fold);
 leg[3]=sub(leg[4],toeSegment);
 const auto lower=sub(leg[3],rest[3]);
 // The two pairs share the contact path but not the bending pattern.
 // Shoulder and elbow stay under the chest in stance. The hind stifle
 // swings forward while the hock follows behind it.
 leg[1]=add(rest[1],add(mul(lower,fore?.20f:.30f),{swingX*flex*(fore?.85f:2.8f),-1.3f*flex}));
 leg[2]=add(rest[2],add(mul(lower,fore?.52f:.69f),{swingX*flex*(fore?-2.1f:-2.25f),-1.5f*flex}));
 return leg;
}

struct Bone {
 Vector2 start,target,axis,targetAxis;
 float alongScale;
 Vector2 offset(Vector2 point)const{
  const auto relative=sub(point,start);
  const float along=relative.x*axis.x+relative.y*axis.y;
  const float across=cross(axis,relative);
  return sub(add(add(target,mul(targetAxis,along*alongScale)),
                 mul({-targetAxis.y,targetAxis.x},across)),point);
 }
};
inline Bone bone(Vector2 a,Vector2 b,Vector2 x,Vector2 y){
 const auto axis=sub(b,a),targetAxis=sub(y,x);
 const float bindLength=std::max(length(axis),.01f);
 const float movedLength=std::max(length(targetAxis),.01f);
 return {a,x,mul(axis,1/bindLength),mul(targetAxis,1/movedLength),movedLength/bindLength};
}

struct Frame {
 const Skin* skin=nullptr;
 std::vector<Vector2> positions,deltas;
 int view=0;
 float strain=1,meanStride=1;
};

inline float applyMotion(Frame& frame){
 const auto& skin=*frame.skin;
 float amount=1;
 // The winding check is the final guard for poses at the edge of the rig.
 for(size_t i=0;i<skin.faces.size();i+=3){
  const int a=skin.faces[i],b=skin.faces[i+1],c=skin.faces[i+2];
  const auto du=sub(frame.deltas[b],frame.deltas[a]),dv=sub(frame.deltas[c],frame.deltas[a]);
  if(du.x==0&&du.y==0&&dv.x==0&&dv.y==0)continue;
  const auto u=sub(frame.positions[b],frame.positions[a]),v=sub(frame.positions[c],frame.positions[a]);
  const float area=cross(u,v);
  const float bindArea=cross(sub(skin.vertices[b].point,skin.vertices[a].point),
                             sub(skin.vertices[c].point,skin.vertices[a].point));
  const float threshold=bindArea*.10f;
  const float linear=cross(u,dv)+cross(du,v),quadratic=cross(du,dv);
  const auto facing=[&](float t){return area+t*(linear+t*quadratic)<=threshold;};
  const float worst=quadratic<0&&linear>0?std::min(amount,-linear/(2*quadratic)):amount;
  if(!facing(worst)){
   float lo=0,hi=worst;
   for(int test=0;test<10;test++){
    const float at=(lo+hi)*.5f;
    if(facing(at))lo=at;
    else hi=at;
   }
   amount=lo;
  }
 }
 for(size_t i=0;i<frame.positions.size();i++)
  frame.positions[i]=add(frame.positions[i],mul(frame.deltas[i],amount));
 return amount;
}

inline Vector2 areaGradient(Vector2 a,Vector2 b){return {a.y-b.y,b.x-a.x};}

inline void relaxSkin(Frame& frame){
 const auto& skin=*frame.skin;
 // Restore the area of the few triangles squeezed by the bends. The
 // correction stays near the affected edge instead of shortening all steps.
 for(int pass=0;pass<7;pass++){
  bool corrected=false;
  for(size_t offset=0;offset<skin.faces.size();offset+=3){
   const size_t i=pass%2?skin.faces.size()-offset-3:offset;
   const auto ia=skin.faces[i],ib=skin.faces[i+1],ic=skin.faces[i+2];
   auto& a=frame.positions[ia];
   auto& b=frame.positions[ib];
   auto& c=frame.positions[ic];
   const auto ra=skin.vertices[ia].point,rb=skin.vertices[ib].point,rc=skin.vertices[ic].point;
   const float limit=.24f*cross(sub(rb,ra),sub(rc,ra));
   const float error=cross(sub(b,a),sub(c,a))-limit;
   if(error<=.015f)continue;
   const float wa=skin.vertices[ia].mobility,wb=skin.vertices[ib].mobility,wc=skin.vertices[ic].mobility;
   const auto ga=areaGradient(b,c),gb=areaGradient(c,a),gc=areaGradient(a,b);
   const auto square=[](Vector2 v){return v.x*v.x+v.y*v.y;};
   const float mass=wa*square(ga)+wb*square(gb)+wc*square(gc);
   if(mass<.001f)continue;
   const float extent=std::max({wa*std::sqrt(square(ga)),wb*std::sqrt(square(gb)),wc*std::sqrt(square(gc))});
   const float correction=-std::min(error/mass,1.65f/std::max(.01f,extent));
   a=add(a,mul(ga,correction*wa));b=add(b,mul(gb,correction*wb));c=add(c,mul(gc,correction*wc));
   corrected=true;
  }
  if(!corrected)break;
 }
 float progress=0,energy=0;
 for(size_t i=0;i<skin.vertices.size();i++){
  const auto desire=frame.deltas[i];
  const auto actual=sub(frame.positions[i],skin.vertices[i].point);
  progress+=desire.x*actual.x+desire.y*actual.y;
  energy+=desire.x*desire.x+desire.y*desire.y;
  frame.deltas[i]=actual;
  frame.positions[i]=skin.vertices[i].point;
 }
 frame.strain=applyMotion(frame);
 frame.meanStride=energy>.001f?std::clamp(progress/energy*frame.strain,0.f,1.f):1.f;
}

inline void articulate(Frame& frame,const Pose& p){
 frame.skin=&skins[p.view];
 frame.view=p.view;
 const auto& skin=*frame.skin;
 frame.positions.resize(skin.vertices.size());
 frame.deltas.resize(skin.vertices.size());
 const auto fitView=registration(p.heading,p.view);
 std::array<Bone,LegInfluences> bones{};
 for(int slot=0;slot<4;slot++){
  const auto leg=legPose(skin,p,slot,fitView);
  const auto& rest=skin.legs[slot];
  for(int i=0;i<LegBones;i++)bones[slot*LegBones+i]=bone(rest[i],rest[i+1],leg[i],leg[i+1]);
 }
 const auto turnHead=fitView.offset(skin,skin.head[1],fitView.from.head[1],fitView.to.head[1]);
 const auto turnNeck=fitView.offset(skin,skin.head[0],fitView.from.head[0],fitView.to.head[0]);
 const float forwards=std::cos(p.heading),sideways=std::sin(p.heading);
 const auto headRise=add({forwards*-1.25f*p.neck,p.neck*2.1f+p.headWalk*2.45f},mul(turnHead,.94f));
 const Vector2 tailMove{p.tail*(2.1f*sideways+.70f*forwards),p.tail*(1.56f*forwards+.60f)};
 const float nod=forwards*(p.neck*.13f+p.headWalk*.065f+p.fold*.21f);
 for(size_t i=0;i<skin.vertices.size();i++){
  const auto& v=skin.vertices[i];
  Vector2 gait{};
  for(size_t j=0;j<bones.size();j++)if(v.weights[j])
   gait=add(gait,mul(bones[j].offset(v.point),v.weights[j]/255.f));
  const auto skull=sub(v.point,skin.head[0]);
  const auto muzzle=skin.head[2];
  const float lip=std::clamp((v.point.y-muzzle.y+.8f)*.47f,0.f,1.f)*
                  std::clamp(1-std::abs(v.point.x-muzzle.x)*.117f,0.f,1.f)*
                  std::clamp((sideways+.83f)*2.4f,0.f,1.f);
  const auto tuck=add(headRise,{forwards*p.fold*2.1f,lip*p.bite*3.4f+p.fold*14.f});
  const auto headDelta=add(sub(rotate(skull,nod),skull),tuck);
  const float neckSkin=std::clamp(1-length(skull)/16.f,0.f,1.f);
  frame.deltas[i]=add(add(gait,mul(headDelta,v.weights[LegInfluences]/255.f)),
                      add(mul(tailMove,v.weights[LegInfluences+1]/255.f),mul(turnNeck,.32f*neckSkin)));
  frame.positions[i]=add(v.point,frame.deltas[i]);
 }
 relaxSkin(frame);
 const Vector2 slide{forwards*(p.drive+p.shoulder),.55f*sideways*p.drive+p.bob};
 const Vector2 ribs{fitView.center,-44.f};
 float bottom=-1e6f;
 for(size_t i=0;i<skin.vertices.size();i++){
  auto local=sub(frame.positions[i],Origin);
  local.x=(local.x-skin.center)*fitView.scale+fitView.center;
  local=add(ribs,rotate(sub(local,ribs),p.lean));
  // Roll onto the flank. Shorten the screen height of the raised head and
  // folded legs while keeping the painted volume around the ribs.
  const float central=std::clamp(local.y,-57.f,-37.f);
  const float grounded=central+(local.y-central)*.24f;
  local.y=mix(local.y,grounded,p.fall);
  const float sideRoll=p.side*(.075f+.59f*std::abs(sideways));
  const float pitchAlongBack=.35f*forwards;
  local=rotate(local,(sideRoll+pitchAlongBack)*p.roll);
  frame.positions[i]=add(local,add(slide,{p.side*6.f*p.fold,3.f*p.fall}));
  bottom=std::max(bottom,frame.positions[i].y);
 }
 if(p.fall>0){
  const float fit=(5.f-bottom)*p.fall*p.fall*(3-2*p.fall);
  for(auto& v:frame.positions)v.y+=fit;
 }
}

inline Vector2 texel(int view,Vector2 node){
 return {Texels*(node.x+view%Columns*Width),Texels*(node.y+view/Columns*Height)};
}
inline bool pointIn(const Frame& frame,Vector2 p){
 const auto& s=*frame.skin;
 for(size_t i=0;i<s.faces.size();i+=3){
  const int ia=s.faces[i],ib=s.faces[i+1],ic=s.faces[i+2];
  const auto a=frame.positions[ia],b=frame.positions[ib],c=frame.positions[ic];
  if(p.x<std::min({a.x,b.x,c.x})||p.x>std::max({a.x,b.x,c.x})||
     p.y<std::min({a.y,b.y,c.y})||p.y>std::max({a.y,b.y,c.y}))continue;
  const auto u=sub(b,a),v=sub(c,a),w=sub(p,a);
  const float det=cross(u,v);
  if(std::abs(det)<.0001f)continue;
  const float x=cross(w,v)/det,y=cross(u,w)/det;
  if(x<0||y<0||x+y>1)continue;
  const auto base=s.vertices[ia].point;
  const auto uv=texel(frame.view,add(base,add(mul(sub(s.vertices[ib].point,base),x),
                                             mul(sub(s.vertices[ic].point,base),y))));
  const int tx=std::clamp(int(uv.x),0,ink.width-1),ty=std::clamp(int(uv.y),0,ink.height-1);
  if(alpha[ty*ink.width+tx]>103)return true;
 }
 return false;
}
inline void paint(const Frame& frame,Color color,Vector2 offset={}){
 const auto& s=*frame.skin;
 rlCheckRenderBatchLimit(static_cast<int>((s.faces.size()/3)*4));
 rlSetTexture(ink.id);
 rlBegin(RL_QUADS);
 rlColor4ub(color.r,color.g,color.b,color.a);
 const auto emit=[&](int index){
  const auto screen=frame.positions[index];
  const auto uv=texel(frame.view,s.vertices[index].point);
  rlTexCoord2f(uv.x/ink.width,uv.y/ink.height);
  rlVertex2f(screen.x+offset.x,screen.y+offset.y);
 };
 for(size_t i=0;i<s.faces.size();i+=3){
  emit(s.faces[i]);emit(s.faces[i+1]);emit(s.faces[i+2]);emit(s.faces[i+2]);
 }
 rlEnd();
 rlSetTexture(0);
}

inline bool draw(const Enemy& e,float x,float y,Color outline,const Vector2* probe,bool shadow,
                 float scale,float opacity,float fall){
 const auto p=pose(e,fall);
 thread_local Frame frame;
 thread_local std::array<float,19> cached{};
 const std::array<float,19> key{float(p.view),p.heading,p.phase,p.gait,p.lean,p.bob,p.drive,p.leap,
                               p.neck,p.bite,p.fall,p.fold,p.roll,p.side,p.shoulder,p.headWalk,p.tail,p.travel.x,p.travel.y};
 if(!frame.skin||key!=cached){articulate(frame,p);cached=key;}
 if(probe)return opacity>=.43f&&pointIn(frame,{(probe->x-x)/scale,(probe->y-y)/scale});
 rlPushMatrix();
 rlTranslatef(x,y,0);
 rlScalef(scale,scale,1);
 if(!shadow&&outline.a&&flat.id!=rlGetShaderIdDefault()&&flat.id){
  BeginShaderMode(flat);
  const auto color=Fade(outline,opacity);
  for(Vector2 offset:{Vector2{-1.35f,0},Vector2{1.35f,0},Vector2{0,-1.35f},Vector2{0,1.35f}})
   paint(frame,color,mul(offset,1/scale));
  EndShaderMode();
 }
 paint(frame,Fade(WHITE,opacity));
 rlPopMatrix();
 return false;
}
}
