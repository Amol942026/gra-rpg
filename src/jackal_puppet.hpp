#pragma once

namespace jackal_puppet {
using namespace caldera_quadruped;
using caldera_quadruped::Mesh;
using namespace ember::desert_enemy_motion;

inline Texture2D ink{};
inline void load(){
 if(ink.id)return;
 ink=LoadTexture("assets/desert/jackal-turn/puppet-ink.png");
 if(ink.id){
  GenTextureMipmaps(&ink);
  SetTextureFilter(ink,TEXTURE_FILTER_TRILINEAR);
  SetTextureFilter(ink,TEXTURE_FILTER_ANISOTROPIC_4X);
 }
}
inline bool ready(){return ink.id!=0;}
inline void unload(){if(ink.id)UnloadTexture(ink);ink={};}

inline float curve(const float (&track)[61],float phase){
 const float frame=std::clamp(phase,0.f,1.f)*60.f;
 const int at=std::min(int(frame),59);
 return mix(track[at],track[at+1],frame-at);
}

struct Pose {
 ActorFacing forward,travel;
 float gait,walk,weight,rock,advance,crouch,air,jaw,fall,fold,side;

 Vector2 ground(float along,float across,float height)const{
  return {along*forward.x-across*forward.y,
          ActorGroundSquash*(along*forward.y+across*forward.x)-height};
 }
 Vector2 body(float along,float across,float height)const{
  const float rise=height*(1-.72f*fall)+weight+air*12.f-crouch*6.f+rock*along;
  const float shift=side*fold*5.2f;
  auto at=ground(along+advance*(1-fall),across*(1+fold*.21f),rise);
  at.x+=shift;
  at.y+=side*fold*along*.13f;
  return at;
 }
};

inline Pose pose(const AnimationState& state,float attack,float fall,int side,float recoil){
 const float heading=state.facingInitialized?state.facingAngle:facingAngles[int(state.direction)];
 const auto direction=actorFacing(heading);
 const auto velocity=state.travelAxis.x==0&&state.travelAxis.y==0?direction:state.travelAxis;
 const float weight=curve(jackal_weight,state.phase),rock=curve(jackal_balance,state.phase);
 const float approach=attack>=0?curve(jackal_pounce,attack):0.f;
 const float crouch=attack>=0?curve(jackal_crouch,attack):0.f;
 const float air=attack>=0?curve(jackal_land,attack):0.f;
 const float bite=attack>=0?curve(jackal_bite,attack):0.f;
 const float drop=fall>=0?curve(jackal_drop,fall):0.f,fold=fall>=0?curve(jackal_fold,fall):0.f;
 const float t=std::clamp(attack/.24f,0.f,1.f);
 const float walk=(state.motionInitialized?state.moveBlend:float(state.moving))*
                  (1-t*t*(3-2*t))*(1-fold);
 return {direction,velocity,state.phase,walk,.89f*weight*walk-.45f*recoil*(1-fold),
         rock*walk*.014f,(8.3f*approach-2.9f*recoil)*(1-fold),crouch*(1-fold),air*(1-fold),
         bite*(1-fold),drop,fold,side?float(side):direction.x<0?-1.f:1.f};
}

// Pigment comes from one fixed set of swatches. The geometry and the UVs
// share the same attachments on every frame, even while the feet turn.
inline constexpr Color Coat{84,84,84,255},LegFur{77,77,77,255},Face{86,86,86,255};
inline constexpr Color Cloth{117,117,117,255},Tail{96,96,96,255},Ear{68,68,68,255},Pads{71,71,71,255};

inline Vector2 sole(const Pose& p,float along,float across,float lift){
 const float fold=p.fold;
 const auto foot=p.ground(along*(1-.29f*fold)+p.advance*.45f,across*(1+.38f*fold),lift*(1-fold));
 return {foot.x+p.side*fold*5.2f,foot.y+p.side*fold*along*.10f};
}

inline void leg(Mesh& mesh,const Pose& p,bool fore,float side){
 const float phase=loopPhase(p.gait+(side>0?.5f:0.f)+(fore?.76f:0.f));
 const float travel=curve(jackal_stride,phase)*p.walk*desert_motion::JackalCycle*.30f;
 const float lift=curve(jackal_lift,phase)*p.walk*(fore?6.9f:5.8f);
 const float longStep=travel*(p.forward.x*p.travel.x+p.forward.y*p.travel.y);
 const float sideStep=travel*(p.forward.x*p.travel.y-p.forward.y*p.travel.x);
 const float base=fore?17.5f:-18.f;
 const float width=side*(fore?7.1f:7.4f);
 const float kick=fore?8.6f:6.3f;
 const float toeHeight=1.55f+lift+p.air*kick;
 const float toeX=base+longStep+(fore?1.f:-1.6f)+p.advance*(fore?.87f:-.29f);
 const float toeY=width+sideStep*.58f;
 const auto paw=sole(p,toeX,toeY,toeHeight);
 const auto wrist=sole(p,toeX-(fore?1.4f:3.5f),toeY,toeHeight+(fore?4.2f:5.0f));
 const auto hip=p.body(base,width,fore?31.f:29.f);
 const auto line=sub(wrist,hip);
 const float d=std::max(1.f,length(line));
 const float upper=fore?14.3f:15.4f,lower=fore?16.2f:14.9f;
 const float possible=std::min(d,upper+lower-.10f);
 const float along=(upper*upper-lower*lower+possible*possible)/(2*possible);
 const float flex=std::sqrt(std::max(0.f,upper*upper-along*along));
 const float profile=std::clamp(p.forward.x*2.5f,-1.f,1.f);
 const auto knee=add(hip,add(mul(line,along/d),mul({-line.y,line.x},flex/d*profile*(fore?-1.f:1.f))));
 const auto folded=p.body(base+(fore?-6.2f:6.8f),width+p.fold*p.side*(fore?5.f:7.2f),13.5f);
 const auto joint=mix(knee,folded,p.fold);
 const float near=.86f+.14f*p.forward.x*side;
 const Color fur=light(LegFur,near);
 mesh.ribbon(std::array<Joint,6>{{
  {hip,fore?5.1f:6.2f},{mix(hip,joint,.32f),fore?5.35f:6.6f},
  {joint,fore?3.24f:3.8f},{mix(joint,wrist,.62f),2.63f},{wrist,2.09f},{paw,2.53f}
 }},fur,DrakeSkin);
 mesh.ellipse(paw,{3.65f,1.f*p.forward.x},{0,1.95f},light(Pads,near),CrawlerSkin,18);
 const auto claws=sole(p,toeX+2.45f,toeY,toeHeight-.46f);
 mesh.ellipse(claws,{1.85f,.41f*p.forward.x},{0,.64f},Color{58,48,40,255},Paint,12);
}

inline void tail(Mesh& mesh,const Pose& p){
 std::array<Joint,9> curvePoints;
 for(unsigned i=0;i<curvePoints.size();i++){
  const float t=i/float(curvePoints.size()-1);
  const float wave=std::sin(2*PI*(p.gait-t*.21f))*p.walk*.65f;
  const float sweep=(wave+p.side*p.fold*2.f)*t;
  const auto point=p.body(-22.f-22.f*t,2.1f*t+sweep,31.f-21.f*t);
  const float width=mix(4.7f,1.9f,t)+2.15f*std::sin(PI*t);
  curvePoints[i]={point,i==curvePoints.size()-1?.21f:width};
 }
 mesh.ribbon(curvePoints,Tail,Plates);
 for(unsigned i=3;i<8;i++){
  const auto a=curvePoints[i].point,b=curvePoints[i+1].point;
  const auto v=sub(b,a);
  const auto n=mul({v.y,-v.x},1/std::max(1.f,length(v)));
  const auto root=add(a,mul(n,-curvePoints[i].radius*.74f));
  const auto tip=add(b,mul(n,-curvePoints[i].radius*1.08f));
  mesh.bone(root,tip,1.3f,.10f,light(Tail,.76f),Plates);
 }
}

inline void torso(Mesh& mesh,const Pose& p){
 const auto center=p.body(-.7f,0,29.1f);
 const float depth=std::abs(p.forward.y);
 const float width=std::hypot(27.0f*p.forward.x,10.3f*p.forward.y)+p.fold*3.7f;
 const auto outline=[&](Vector2 u){
  const float f=u.x*p.forward.x;
  const float waist=.9f-.26f*std::exp(-19.f*(f+.37f)*(f+.37f))+.30f*std::max(0.f,f);
  const float y=u.y*(u.y<0?10.1f:8.9f*waist+depth*2.f)*(1-.21f*p.fold);
  return add(center,{width*u.x,y-f*1.7f+u.x*(p.rock*22.f+p.side*.71f*p.fold)});
 };
 mesh.sculpt(outline,Coat,DrakeScale,{1,0},44);
 // Keep the readable locks of fur on the profile itself. The atlas supplies
 // the smaller grain, which is filtered when it falls below a screen pixel.
 constexpr Vector2 fur[]={{-.70f,-.24f},{-.51f,-.46f},{-.31f,-.28f},{-.14f,-.52f},
                          {.09f,-.36f},{.34f,-.57f},{.57f,-.30f},{-.55f,.23f},
                          {-.29f,.38f},{.02f,.27f},{.32f,.42f},{.52f,.21f}};
 for(size_t i=0;i<std::size(fur);i++){
  const auto p0=fur[i];
  const float drag=-.093f*p.forward.x;
  const auto root=outline(p0);
  const auto tip=outline({p0.x+drag,p0.y+.17f});
  const Color shade=i<7?Color{124,99,70,255}:Color{177,140,96,255};
  mesh.bone(root,tip,.47f,.05f,shade);
 }
 const auto throat=p.body(23.f,0,38.3f);
 const auto garland=p.body(18.f,0,35.2f);
 mesh.ribbon(std::array<Joint,4>{{{p.body(13.f,0,34.f),10.7f},{garland,9.8f},
                                 {throat,8.5f},{p.body(26.f,0,45.f),6.5f}}},
             light(Face,.99f),DrakeFace);
 for(int i=0;i<4;i++){
  const auto root=p.body(19.4f-i*2.2f,0,42.f-i*1.85f);
  const auto end=p.body(14.2f-i*2.5f,0,40.1f-i*1.78f);
  mesh.bone(root,end,1.65f,.14f,light(Coat,.98f),DrakeScale);
 }
}

inline void head(Mesh& mesh,const Pose& p){
 const float lean=3.6f*p.side*p.fold;
 const auto crown=p.body(26.9f,lean,48.3f),bridge=p.body(34.7f,lean,44.2f);
 const auto nose=p.body(41.7f,lean,42.0f);
 const auto chin=p.body(40.4f,lean,38.4f-4.9f*p.jaw);
 const auto muzzle=p.body(29.2f,lean,43.7f);
 // The ears and facial marks follow explicit points on the same head.
 for(float side:{-1.f,1.f}){
  const auto root=p.body(25.5f,lean+side*4.75f,51.1f);
  const auto outer=p.body(22.7f,lean+side*8.9f,55.0f);
  const auto inner=p.body(29.9f,lean+side*4.75f,54.5f);
  const auto tip=p.body(24.7f,lean+side*7.9f,66.7f);
  mesh.mark(root,outer,tip,light(Face,.86f),light(Face,.74f),Face,DrakeFace);
  mesh.mark(root,tip,inner,Face,light(Face,.98f),light(Face,1.14f),DrakeFace);
  const float visibility=std::clamp((p.forward.y+.53f)/.40f,0.f,1.f);
  if(visibility>.01f)
   mesh.mark(mix(root,tip,.18f),mix(outer,tip,.28f),mix(tip,inner,.20f),Ear,Ear,light(Ear,.78f),CrawlerFace);
 }
 mesh.ellipse(crown,{std::hypot(9.7f*p.forward.x,7.15f*p.forward.y),-1.2f*p.forward.x},
               {0,7.5f*(1-.15f*p.fold)},Face,DrakeFace,32);
 // The snout sits in front of the skull only on the visible side of the turn.
 const float foreshorten=std::clamp((p.forward.y+.91f)/.39f,0.f,1.f);
 if(foreshorten>.01f){
  const auto tip=mix(muzzle,nose,foreshorten);
  const auto lower=mix(muzzle,chin,foreshorten);
  if(p.jaw>.03f){
   mesh.mark(muzzle,tip,lower,{42,25,27,255},{45,26,26,255},{81,40,40,255});
   mesh.bone(muzzle,lower,2.3f,1.15f,light(Face,.82f),DrakeFace);
   const auto tooth=mix(bridge,tip,.32f);
   mesh.mark(tooth,add(tooth,{1.25f,0}),add(tooth,{.4f,2.7f*p.jaw}),{218,194,148,255},{249,225,180,255},{209,181,139,255});
  }
  mesh.ribbon(std::array<Joint,4>{{{muzzle,5.6f},{mix(muzzle,bridge,.64f),4.65f},
                                  {mix(muzzle,bridge,foreshorten),3.3f},{tip,1.95f}}},
              light(Face,1.01f),DrakeFace);
  mesh.ellipse(tip,{1.6f+.50f*std::abs(p.forward.y),-.16f*p.forward.x},{0,1.58f},
                {48,39,34,255},Paint,16);
 }
 for(float side:{-1.f,1.f}){
  const float visible=std::clamp((side*p.forward.x+.56f)/.40f,0.f,1.f)*
                      std::clamp((p.forward.y+.54f)/.31f,0.f,1.f);
  if(visible<=.01f)continue;
  const auto eye=p.body(31.4f,lean+side*5.95f,47.3f);
  mesh.ellipse(eye,{1.78f*visible,-.33f*p.forward.x*visible},{0,.98f*visible},{49,33,29,255},Paint,18);
  mesh.oval(add(eye,{.35f*p.forward.x*visible,.03f}),.87f*visible,.63f*visible,{201,148,65,255},Paint,14);
  mesh.oval(add(eye,{.62f*p.forward.x*visible,-.20f}),.33f*visible,.46f*visible,{41,29,27,255},Paint,12);
 }
}

inline void collar(Mesh& mesh,const Pose& p){
 const auto high=p.body(22.f,0,40.6f),low=p.body(19.8f,0,36.8f);
 const Vector2 across{std::hypot(p.forward.x*7.9f,p.forward.y*9.5f),
                     -1.35f*p.forward.x};
 mesh.ellipse(high,across,{0,3.35f},Cloth,Membrane,28);
 mesh.ellipse(low,mul(across,1.06f),{0,3.05f},light(Cloth,.78f),Membrane,28);
 const float show=std::clamp((p.forward.y+.86f)/.61f,0.f,1.f);
 if(show>.01f){
  const auto knot=p.body(22.6f,4.7f*p.forward.x,34.8f);
  const auto pendant=add(knot,{1.0f*p.forward.x,6.2f*show});
  mesh.bone(knot,pendant,.72f*show,.65f*show,{131,105,65,255});
  mesh.oval(pendant,2.18f*show,2.55f*show,{115,79,43,255},Paint,20);
  mesh.oval(add(pendant,{0,-.36f}),1.25f*show,1.43f*show,{208,162,83,255},Paint,16);
 }
}

inline void build(Mesh& mesh,const Pose& p){
 struct Foot {bool fore;float side;};
 std::array<Foot,4> ordered{{{false,-1},{false,1},{true,-1},{true,1}}};
 const auto depth=[&](Foot f){return (f.fore?1.f:-1.f)*p.forward.y+.45f*f.side*p.forward.x;};
 std::sort(ordered.begin(),ordered.end(),[&](Foot a,Foot b){return depth(a)<depth(b);});
 tail(mesh,p);
 for(auto foot:ordered)leg(mesh,p,foot.fore,foot.side);
 torso(mesh,p);
 collar(mesh,p);
 head(mesh,p);
 if(p.fall>0){
  float bottom=-1e6f;
  for(const auto& t:mesh.triangles)bottom=std::max({bottom,t.a.point.y,t.b.point.y,t.c.point.y});
  const float fit=(1.5f-bottom)*(p.fall>.91f?1.f:p.fall*p.fall);
  for(auto& t:mesh.triangles){t.a.point.y+=fit;t.b.point.y+=fit;t.c.point.y+=fit;}
 }
}

inline bool draw(const Enemy& e,float x,float y,Color outline,const Vector2* probe,bool shadow,
                 float scale,float opacity,float fall){
 const bool dead=fall>=0;
 const auto& state=dead&&e.deathMotion.facingInitialized?e.deathMotion:e.animation;
 const auto p=pose(state,dead?e.deathAction:e.volcanoAction(),fall,e.deathScreenSide(),
                   std::clamp((dead?e.deathFlash:e.flash)/.16f,0.f,1.f));
 thread_local Mesh mesh;
 thread_local std::array<float,15> stored{};
 const std::array<float,15> key{p.forward.x,p.forward.y,p.travel.x,p.travel.y,p.gait,p.walk,p.weight,p.rock,
                               p.advance,p.crouch,p.air,p.jaw,p.fall,p.fold,p.side};
 if(key!=stored){
  mesh.triangles.clear();
  if(!mesh.triangles.capacity())mesh.triangles.reserve(840);
  build(mesh,p);
  stored=key;
 }
 if(probe)return opacity>=.43f&&mesh.includes({(probe->x-x)/scale,(probe->y-y)/scale});
 rlPushMatrix();
 rlTranslatef(x,y,0);
 rlScalef(scale,scale,1);
 if(outline.a&&!shadow)
  for(Vector2 offset:{Vector2{-1.35f,0},Vector2{1.35f,0},Vector2{0,-1.35f},Vector2{0,1.35f}})
   mesh.paint({},outline,opacity,true,mul(offset,1/scale));
 mesh.paint(ink,WHITE,opacity,shadow);
 rlSetTexture(rlGetTextureIdDefault());
 rlPopMatrix();
 return false;
}
}
