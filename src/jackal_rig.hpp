#pragma once

namespace jackal_rig {
using namespace caldera_quadruped;
using caldera_quadruped::Material;
using caldera_quadruped::Mesh;
namespace motion=ember::jackal_motion;

inline Texture2D paint{};
inline Shader colorShader{};
inline int bearingUniform=-1;

inline bool ready(){return paint.id&&colorShader.id&&colorShader.id!=rlGetShaderIdDefault();}
inline void unload(){
 if(paint.id)UnloadTexture(paint);
 if(colorShader.id&&colorShader.id!=rlGetShaderIdDefault())UnloadShader(colorShader);
 paint={};colorShader={};
}
inline void load(){
 if(ready())return;
 const std::string root="assets/desert/jackal-turn/";
 if(!FileExists((root+"rig-paint.png").c_str()))return;
 paint=LoadTexture((root+"rig-paint.png").c_str());
 if(paint.width!=480||paint.height!=1280){unload();return;}
 SetTextureFilter(paint,TEXTURE_FILTER_BILINEAR);
 colorShader=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform vec2 bearing;
out vec4 finalColor;
void main(){
 if(fragTexCoord.y>1.){finalColor=fragColor;return;}
 float u=fragTexCoord.x;
 float row=floor(fragTexCoord.y*8.);
 float flankU=bearing.x<0.&&row<4.?1.-u:u;
 float turning=abs(bearing.y);
 float endU=u;
 float blend=smoothstep(.23,.94,turning);
 if(row==0.){
  // In a quarter view the exposed end of the chest or rump is localized
  // around its attachment, so its texture never washes across the whole flank.
  float nearDirection=bearing.x*(bearing.y>=0.?1.:-1.);
  float exposed=turning*(1.+.54*(2.*u-1.)*nearDirection);
  float cap=.34*nearDirection*(1.-turning);
  endU=clamp(.5+(u-.5-cap)/mix(.48,1.,turning),.033,.967);
  blend=smoothstep(.24,.94,exposed);
 }
 vec3 flank=texture(texture0,vec2(flankU/3.,fragTexCoord.y)).rgb;
 vec3 end=texture(texture0,vec2((endU+(bearing.y>=0.?1.:2.))/3.,fragTexCoord.y)).rgb;
 finalColor=vec4(mix(flank,end,blend)*fragColor.rgb,fragColor.a);
}
)GLSL");
 if(!ready()){unload();return;}
 bearingUniform=GetShaderLocation(colorShader,"bearing");
}

inline float track(const float (&samples)[61],float phase){
 const float frame=std::clamp(phase,0.f,1.f)*60.f;
 const int i=std::min(59,int(frame));
 return mix(samples[i],samples[i+1],frame-i);
}
inline float ease(float t){t=std::clamp(t,0.f,1.f);return t*t*(3.f-2.f*t);}

struct Pose {
 ActorFacing forward,travel;
 float phase,pace,bob,head,tail,crouch,drive,air,jaw,drop,roll,flank;
 float hips,shoulders,arch;

 Vector2 at(float length,float width,float height)const{
  const float across=width+roll*flank*3.8f;
  const float shift=length+drive;
  // An upright 2D cutout uses less of the ground-plane shear higher up.
  const float pitch=mix(ActorGroundSquash,.10f,std::clamp(height/77.f,0.f,1.f));
  return {shift*forward.x-across*forward.y,
          pitch*(shift*forward.y+across*forward.x)-height*(1-.75f*drop)+
          bob*(height/50.f)+crouch*(height/65.f)*7.f-9.f*air+flank*roll*length*.095f};
 }
 float spineOffset(float along)const{
  const float t=std::clamp((along+19.f)/38.f,0.f,1.f);
  return mix(hips,shoulders,t)+arch*(4*t*(1-t));
 }
 Vector2 mount(float along,float side,float height)const{
  auto point=at(along,side,height);
  point.y+=spineOffset(along);
  return point;
 }
};

inline Pose pose(const AnimationState& a,float bite,float fall,int side){
 const auto heading=actorFacing(a.facingInitialized?a.facingAngle:facingAngles[int(a.direction)]);
 const auto travel=a.travelAxis.x==0&&a.travelAxis.y==0?heading:a.travelAxis;
 const auto phase=loopPhase(a.phase);
 const float drop=fall<0?0:track(motion::collapse,fall),roll=fall<0?0:track(motion::roll,fall);
 const float pace=(a.motionInitialized?a.moveBlend:float(a.moving))*(1-drop)*
                  (1-(bite<0?0:ease(bite/.20f)*(1-ease((bite-.66f)/.34f))));
 const float contact=1-drop;
 const auto attack=[&](const float (&curve)[61]){return bite<0?0.f:track(curve,bite)*contact;};
 const float crouch=attack(motion::crouch),air=attack(motion::air);
 // The fore feet meet the ground a quarter-cycle after the hind feet.
 const float hips=1.32f*track(motion::support,phase)*pace-.46f*crouch+.32f*air;
 const float shoulders=1.52f*track(motion::support,loopPhase(phase+.755f))*pace+2.35f*crouch-1.16f*air;
 const float arch=.64f*track(motion::spine,phase)*pace;
 return {heading,travel,phase,pace,track(motion::weight,phase)*pace,
         track(motion::head,phase)*pace,track(motion::tail,phase)*pace,
         crouch,attack(motion::pounce)*10.f,air,attack(motion::jaw),
         drop,roll,side?float(side):(heading.x>=0?1.f:-1.f),hips,shoulders,arch};
}

// Part numbers follow the rows of the painted atlas.
inline constexpr Material Body=DrakeScale,Neck=Membrane,Skull=DrakeFace,Snout=DrakeSkin,Ear=Plates,Tail=CrawlerFace;
inline constexpr Material ForeLeg=CrawlerSkin,HindLeg=Ivory;

struct Strip {Vector2 top,bottom;};
inline void feather(Mesh& mesh,Vertex a,Vertex b,Vector2 outward,Material skin){
 auto outsideA=a,outsideB=b;
 outsideA.point=add(a.point,outward);outsideB.point=add(b.point,outward);
 outsideA.color.a=outsideB.color.a=0;
 mesh.triangle(a,b,outsideA,skin);mesh.triangle(outsideA,b,outsideB,skin);
}
inline void stripe(Mesh& mesh,Strip a,Strip b,float left,float right,Material skin,Color tint=WHITE,bool soften=false){
 const auto underside=light(tint,.86f);
 const Vertex at{a.top,{left,0},tint},ab{a.bottom,{left,1},underside};
 const Vertex bt{b.top,{right,0},tint},bb{b.bottom,{right,1},underside};
 mesh.triangle(at,ab,bt,skin);mesh.triangle(bt,ab,bb,skin);
 if(soften){feather(mesh,at,bt,{0,-.66f},skin);feather(mesh,ab,bb,{0,.66f},skin);}
}

inline float torsoWidth(const Pose& p){
 const float end=p.forward.y>=0?13.6f:9.7f;
 return 2.f*std::hypot(30.5f*p.forward.x,end*p.forward.y);
}
inline Vector2 torsoCentre(const Pose& p){return p.mount(-1.f,0,49.5f);}
using TorsoProfile=std::array<Strip,13>;
inline TorsoProfile torsoProfile(const Pose& p){
 // The spine and belly are fixed contours in the same profile. The camera
 // projects their width. There are no independent outlines at the turn keys.
 constexpr std::array<float,13> back{0,8.7f,12.2f,13.2f,12.6f,11.7f,11.5f,12.0f,12.9f,13.3f,12.5f,9.5f,0};
 constexpr std::array<float,13> belly{0,7.7f,10.5f,11.3f,10.5f,9.3f,9.6f,11.3f,13.3f,14.9f,13.4f,9.6f,0};
 const auto center=torsoCentre(p);
 const float side=p.forward.x*p.forward.x;
 const float end=p.forward.y>=0?1.03f:.92f;
 const float dorsal=p.forward.y>=0?.81f:.84f;
 const float radius=torsoWidth(p)*.5f;
 const float spineCentre=p.spineOffset(-1.f);
 const auto section=[&](int i){
  const float t=float(i)/(back.size()-1),u=2*t-1;
  const int opposite=int(back.size())-1-i;
  const float tip=std::sqrt(std::max(0.f,1-u*u));
  const float top=mix(15.5f*tip*dorsal,p.forward.x>=0?back[i]:back[opposite],side);
  const float bottom=mix(13.6f*tip*end,p.forward.x>=0?belly[i]:belly[opposite],side);
  const float slope=u*(p.forward.x*.8f+p.roll*p.flank*3.7f);
  const float along=30.5f*u*p.forward.x+14.f*p.forward.y*(1-std::abs(p.forward.x));
  const float transfer=p.spineOffset(along)-spineCentre;
  const float compress=1-.30f*p.drop;
  const float x=center.x+u*radius;
  return Strip{{x,center.y+transfer+(slope-top)*compress},{x,center.y+transfer+(slope+bottom)*compress}};
 };
 TorsoProfile profile;
 for(int i=0;i<int(profile.size());i++)profile[i]=section(i);
 return profile;
}
inline void torso(Mesh& mesh,const TorsoProfile& profile){
 const float spans=profile.size()-1;
 for(int i=1;i<int(profile.size());i++)stripe(mesh,profile[i-1],profile[i],(i-1)/spans,i/spans,Body,WHITE,true);
}

inline Vector2 insetSocket(const TorsoProfile& profile,Vector2 socket){
 const float left=profile.front().top.x,right=profile.back().top.x;
 const float column=std::clamp((socket.x-left)/(right-left),.035f,.965f)*(profile.size()-1);
 const int i=int(column);
 const float top=mix(profile[i].top.y,profile[i+1].top.y,column-i);
 const float bottom=mix(profile[i].bottom.y,profile[i+1].bottom.y,column-i);
 const float inset=std::min(2.15f,(bottom-top)*.31f);
 return {mix(left,right,column/(profile.size()-1)),std::clamp(socket.y,top+inset,bottom-inset)};
}

inline Vector2 skullSeat(const Pose& p){
 const auto natural=p.at(28.5f,0,66+p.head);
 // The skull is held above the chest at every view. A separate projection
 // of the neck and skull would pull the face down into the body head-on.
 const float rise=23.7f*(1-.61f*p.drop)+5.8f*p.jaw;
 const float crest=std::min(p.hips,p.shoulders)+std::min(0.f,p.arch);
 const float follow=std::min(p.shoulders,crest+.56f);
 return {natural.x,p.at(-1.f,0,49.5f).y-rise+follow+(p.forward.y*.65f-p.head)*(1-p.drop)};
}
inline Vector2 neckShoulder(const Pose& p){return p.mount(15.8f,0,48.f);}
inline Vector2 neckSkull(const Pose& p){return add(skullSeat(p),{0,4.1f*(1-.2f*p.drop)});}
inline void neck(Mesh& mesh,const Pose& p){
 const auto base=neckShoulder(p),seat=neckSkull(p);
 const auto collar=mix(base,seat,.51f);
 const auto neckMiddle=add(collar,{1.05f*p.forward.x,-1.4f*(1-p.drop)});
 const std::array<Vector2,5> axis{base,mix(base,neckMiddle,.48f),neckMiddle,mix(neckMiddle,seat,.56f),seat};
 const std::array<float,5> width{9.3f,10.8f,9.2f,7.25f,4.85f};
 for(int i=1;i<int(axis.size());i++){
  const auto a=axis[i-1],b=axis[i];
  const float va=1.f-(i-1)*.25f,vb=1.f-i*.25f;
  const float compression=(1-.17f*p.drop)*(1-std::abs(p.forward.y)*(p.forward.y>=0?.10f:.22f));
  const auto spanA=width[i-1]*compression,spanB=width[i]*compression;
  const Vertex al{add(a,{-spanA,0}),{0,va},light(WHITE,.91f)},ar{add(a,{spanA,0}),{1,va},WHITE};
  const Vertex bl{add(b,{-spanB,0}),{0,vb},WHITE},br{add(b,{spanB,0}),{1,vb},WHITE};
  mesh.triangle(al,ar,bl,Neck);mesh.triangle(bl,ar,br,Neck);
  feather(mesh,al,bl,{-.67f,0},Neck);feather(mesh,ar,br,{.67f,0},Neck);
 }
}

inline void muzzle(Mesh& mesh,const Pose& p,Vector2 anchor,Vector2 nose,float reveal){
 const auto line=sub(nose,anchor);
 const auto across=mul({-line.y,line.x},1/std::max(1.0f,length(line)));
 const auto root=mul(across,4.9f*reveal),tip=mul(across,2.45f*reveal);
 stripe(mesh,{sub(anchor,root),add(anchor,root)},{sub(nose,tip),add(nose,tip)},.06f,.98f,Snout);
 if(p.jaw>.05f){
  const auto end=add(nose,{-.9f*p.forward.x,5.9f*p.jaw*reveal});
  mesh.mark(anchor,nose,end,{43,24,24,255},{49,28,27,255},{68,34,34,255});
  mesh.bone(anchor,end,2.5f*reveal,1.2f*reveal,WHITE,Snout);
  const auto tooth=mix(anchor,nose,.78f);
  mesh.mark(add(tooth,{-.9f,.4f}),add(tooth,{.8f,.4f}),add(tooth,{0,2.45f*p.jaw}),
            {210,196,155,255},{235,216,177,255},{218,201,163,255});
 }
 mesh.ellipse(nose,{2.35f*reveal,0},{0,1.38f*reveal},{55,41,34,255},Paint,16);
}

inline void head(Mesh& mesh,const Pose& p){
 const auto center=skullSeat(p);
 const auto shift=sub(center,p.at(28.5f,0,66.f+p.head));
 for(float side:{-1.f,1.f}){
  const float folded=1.f-.33f*p.drop;
  const auto root=add(p.at(26.9f,side*4.9f,68.7f+p.head),shift);
  const auto apex=add(p.at(24.7f,side*7.3f,80.8f+p.head-2.5f*p.roll),shift);
  const auto outer=add(mix(root,apex,.35f),{-3.5f*folded,-.5f});
  const auto inner=add(mix(root,apex,.28f),{3.1f*folded,.2f});
  const auto left=add(root,{-3.1f*folded,.7f}),right=add(root,{3.9f*folded,1.0f});
  const Color shade=light(WHITE,.93f+.06f*p.forward.x*side);
  const Vertex a{left,{.05f,.97f},shade},b{outer,{.06f,.47f},shade},c{apex,{.47f,.02f},WHITE};
  const Vertex d{inner,{.94f,.52f},WHITE},e{right,{.9f,.97f},shade};
  mesh.triangle(a,b,c,Ear);mesh.triangle(a,c,e,Ear);mesh.triangle(e,c,d,Ear);
 }
 const float wide=std::hypot(9.5f*p.forward.x,8.7f*p.forward.y);
 const auto shape=[&](Vector2 local){
  const float taper=1.f-.19f*std::max(0.f,local.y);
  return add(center,{local.x*wide*taper+p.forward.x*local.y*1.45f,
                     local.y*(8.15f-.8f*p.drop)});
 };
 mesh.sculpt(shape,WHITE,Skull,{1,0},28);
 const float reveal=ease((p.forward.y+.86f)/.53f)*(1-.93f*(1-p.jaw)*ease((p.forward.y-.71f)/.29f));
 if(reveal>.01f){
  const auto root=add(center,{4.1f*p.forward.x,1.1f});
  const auto nose=mix(root,add(center,{15.8f*p.forward.x,5.8f-1.7f*p.drop}),reveal);
  muzzle(mesh,p,root,nose,reveal);
 }
}

inline void tail(Mesh& mesh,const Pose& p){
 constexpr std::array<float,10> radius{4.3f,5.2f,6.3f,6.8f,6.3f,5.4f,4.0f,3.0f,1.5f,.1f};
 std::array<Joint,radius.size()> points;
 for(size_t i=0;i<points.size();i++){
  const float t=float(i)/(points.size()-1);
  const float wave=t*t*(2.8f*p.tail+p.flank*p.roll*4.f);
  points[i]={p.mount(-25.5f-20.4f*t,wave,49.f-32.f*std::pow(t,1.16f)),radius[i]};
 }
 mesh.ribbon(points,WHITE,Tail);
}

inline std::array<Vector2,4> leg(const Pose& p,bool fore,int flank){
 const float phase=loopPhase(p.phase+(flank>0?.5f:0.f)+(fore?.755f:0.f));
 const float reach=track(motion::step,phase)*p.pace*16.9f;
 const float lift=track(motion::lift,phase)*p.pace*(fore?8.1f:8.6f);
 const float along=fore?18.7f:-18.8f;
 const float high=(fore?41.f:44.8f)-p.spineOffset(along);
 const float hipWidth=flank*(fore?6.9f:6.0f);
 const float advance=reach*(p.forward.x*p.travel.x+p.forward.y*p.travel.y);
 const float lateral=reach*(p.forward.x*p.travel.y-p.forward.y*p.travel.x);
 const float targetToe=along+(fore?1.5f:-2.8f)+advance+p.air*(fore?8.4f:-3.2f)+(fore?-9.f:8.1f)*p.drop;
 const float width=hipWidth+lateral*.56f+flank*(1.1f+3.2f*p.drop);
 const float toeHeight=1.15f+lift+p.air*(fore?5.5f:2.5f);
 const float pawLength=fore?1.5f:3.1f,wristHeight=toeHeight+(fore?4.8f:6.1f);
 // Solve the bend in the leg's side plane with constant link lengths.
 const float upper=fore?15.8f:20.8f,lower=fore?22.2f:24.2f;
 const float vertical=wristHeight-high;
 const float horizontal=std::sqrt(std::max(1.f,(upper+lower-.04f)*(upper+lower-.04f)-vertical*vertical));
 const float wrist=along+std::clamp(targetToe-pawLength-along,-horizontal,horizontal);
 const float toe=wrist+pawLength;
 const Vector2 delta{wrist-along,wristHeight-high};
 const float span=std::max(1.f,length(delta));
 const float projection=(upper*upper-lower*lower+span*span)/(2*span);
 const float flex=std::sqrt(std::max(0.f,upper*upper-projection*projection))*(fore?-1.f:1.f);
 const auto unit=mul(delta,1/span);
 const auto bend=add(mul(unit,projection),mul({-unit.y,unit.x},flex));
 const auto ankle=p.at(wrist,width,wristHeight+16.f*p.drop);
 const auto foot=p.at(toe,width,toeHeight+20.f*p.drop);
 auto knee=p.at(along+bend.x,(hipWidth+width)*.5f,high+bend.y);
 if(p.drop>0)knee=mix(knee,p.at(along+(fore?-6.8f:7.4f),width,22.5f),p.drop);
 return {p.at(along,hipWidth,high),knee,ankle,foot};
}

inline void legStrips(Mesh& mesh,const Pose& p,const TorsoProfile& profile){
 struct Slot {bool fore;int flank;};
 std::array<Slot,4> slots{{{false,-1},{false,1},{true,-1},{true,1}}};
 const auto order=[&](Slot s){return (s.fore?1.f:-1.f)*p.forward.y+s.flank*p.forward.x*.48f;};
 std::sort(slots.begin(),slots.end(),[&](Slot a,Slot b){
  const float delta=order(a)-order(b);
  return delta==0?int(a.fore)*3+a.flank<int(b.fore)*3+b.flank:delta<0;
 });
 for(auto slot:slots){
  const auto [root,bend,ankle,toe]=leg(p,slot.fore,slot.flank);
  const auto socket=insetSocket(profile,root);
  const auto knee=add(bend,mul(sub(socket,root),.42f));
  const auto foot=sub(toe,ankle);
  const auto tip=add(toe,mul(foot,1.75f/std::max(1.f,length(foot))));
  const auto kneeA=mix(knee,socket,.21f),kneeB=mix(knee,ankle,.21f);
  const auto hinge=mix(knee,mix(kneeA,kneeB,.5f),.22f);
  const float thigh=slot.fore?4.7f:5.85f;
  const auto tint=light(WHITE,std::clamp(.91f+order(slot)*.07f,.78f,1.f));
  mesh.ribbon(std::array<Joint,9>{{
   {socket,thigh*.88f},{mix(socket,kneeA,.32f),thigh},{kneeA,3.25f},
   {hinge,2.8f},{kneeB,2.55f},{mix(kneeB,ankle,.67f),2.13f},
   {ankle,1.8f},{toe,2.12f},{tip,.48f}
  }},tint,slot.fore?ForeLeg:HindLeg);
 }
}

inline void drawMesh(const Mesh& mesh,Color tint,float opacity,bool solid,Vector2 offset={}){
 rlCheckRenderBatchLimit(int(mesh.triangles.size())*4+4);
 rlBegin(RL_QUADS);rlSetTexture(solid?rlGetTextureIdDefault():paint.id);
 const auto emit=[&](Vertex v,Material material){
  const Color c=solid?tint:v.color;
  const float alpha=(solid?tint.a/255.f:1.f)*v.color.a*opacity;
  rlColor4ub(c.r,c.g,c.b,int(alpha+.5f));
  rlTexCoord2f(.03f+.94f*v.uv.x,material==Paint?1.5f:(material+.03f+.94f*v.uv.y)/8.f);
  rlVertex2f(v.point.x+offset.x,v.point.y+offset.y);
 };
 for(const auto& t:mesh.triangles){emit(t.a,t.material);emit(t.b,t.material);emit(t.c,t.material);emit(t.c,t.material);}
 rlEnd();rlSetTexture(0);
}

enum Layer {TailLayer,LegLayer,NeckLayer,TorsoLayer,HeadLayer};
struct Anatomy {std::array<Mesh,5> layers;};
inline void build(Anatomy& model,const Pose& p){
 auto& layers=model.layers;
 for(auto& mesh:layers)mesh.triangles.clear();
 const auto profile=torsoProfile(p);
 tail(layers[TailLayer],p);
 legStrips(layers[LegLayer],p,profile);
 neck(layers[NeckLayer],p);
 // The chest masks the neck root and the upper legs. The head only covers
 // the upper neck, using the same skull seat for both meshes.
 torso(layers[TorsoLayer],profile);
 head(layers[HeadLayer],p);
 if(p.drop>0){
  float bodyBottom=-200.f,bottom=-200.f;
  for(size_t i=0;i<layers.size();i++)for(const auto& t:layers[i].triangles){
   const float y=std::max({t.a.point.y,t.b.point.y,t.c.point.y});
   bottom=std::max(bottom,y);
   if(i!=LegLayer)bodyBottom=std::max(bodyBottom,y);
  }
  const float support=mix(bottom,bodyBottom,ease((p.drop-.48f)/.52f));
  const float shift=(2.f-support)*ease(p.drop);
  for(auto& mesh:layers)for(auto& t:mesh.triangles){t.a.point.y+=shift;t.b.point.y+=shift;t.c.point.y+=shift;}
 }
}

inline bool draw(const Enemy& e,float x,float y,Color outline,const Vector2* probe,bool shadow,
                 float scale,float opacity,float fall){
 const auto& a=fall>=0&&e.deathMotion.facingInitialized?e.deathMotion:e.animation;
 const auto p=pose(a,fall>=0?e.deathAction:e.volcanoAction(),fall,e.deathScreenSide());
 const std::array<float,19> key{p.forward.x,p.forward.y,p.travel.x,p.travel.y,p.phase,p.pace,p.bob,p.head,
                                p.tail,p.crouch,p.drive,p.air,p.jaw,p.drop,p.roll,p.flank,p.hips,p.shoulders,p.arch};
 thread_local std::array<float,19> cached{};
 thread_local Anatomy model;
 if(key!=cached){build(model,p);cached=key;}
 if(probe){
  if(opacity<.43f)return false;
  const Vector2 q{(probe->x-x)/scale,(probe->y-y)/scale};
  return std::any_of(model.layers.begin(),model.layers.end(),[&](const auto& mesh){return mesh.includes(q);});
 }
 const Vector2 bearing{p.forward.x,p.forward.y};
 rlPushMatrix();rlTranslatef(x,y,0);rlScalef(scale,scale,1);
 if(outline.a&&!shadow){
  for(Vector2 shift:{Vector2{-1.3f,0},Vector2{1.3f,0},Vector2{0,-1.3f},Vector2{0,1.3f}})
   for(const auto& mesh:model.layers)drawMesh(mesh,outline,opacity,true,mul(shift,1/scale));
 }
 if(!shadow){BeginShaderMode(colorShader);SetShaderValue(colorShader,bearingUniform,&bearing,SHADER_UNIFORM_VEC2);}
 for(const auto& mesh:model.layers)drawMesh(mesh,WHITE,opacity,shadow);
 if(!shadow)EndShaderMode();
 rlPopMatrix();
 return false;
}
}
