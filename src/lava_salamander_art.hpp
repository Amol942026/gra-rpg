#pragma once

// The same hand-painted surfaces supply the walk, bite, hit outline and body.
namespace salamander_art {
namespace art=caldera_quadruped;
using art::Mesh;
using art::Vertex;
using art::mix;
using cinderhorn_turn::SectionJoint;
using cinderhorn_turn::SurfacePoint;

inline Texture2D materials{};
inline void load(){art::loadGrain(materials,"assets/volcano/lava-salamander-materials.png");}
inline void unload(){if(materials.id)UnloadTexture(materials);materials={};}
inline bool ready(){return materials.id!=0;}

struct Pose {
 ActorFacing axes;
 float phase,pace,bob,reach,gape,sway,tailWave;
 float fall,side,rollCos,rollSin;

 SurfacePoint project(float along,float across,float height)const{
  const float rear=std::clamp((-along-16.f)/49.f,0.f,1.f);
  const float neck=std::clamp((along-14.f)/29.f,0.f,1.f);
  across+=sway*(along/45.f)+rear*rear*tailWave;
  const float rollHeight=height-13.f;
  const float originalAcross=across;
  across=across*rollCos-rollHeight*rollSin;
  height=13.f+rollHeight*rollCos+originalAcross*rollSin;
  height=std::max(fall*.65f,height-6.4f*fall)+bob*std::clamp(height/10.f,0.f,1.f);
  along+=neck*reach;
  const float ground=axes.y*along+axes.x*across;
  return {{axes.x*along-axes.y*across,ActorGroundSquash*ground-height},
          ground+ActorGroundSquash*height};
 }
};

inline Pose pose(const AnimationState& state,float action,float time,int side){
 const float direction=state.facingInitialized?state.facingAngle:facingAngles[int(state.direction)%8];
 const float phase=art::loopPhase(state.phase);
 const float pace=state.motionInitialized?state.moveBlend:float(state.moving);
 const auto walk=volcano_motion::sample(LavaSalamanderType-VolcanoFirstType,phase);
 const float fall=enemy_death::window(time,.01f,.96f);
 const float active=1-fall;
 const float ready=enemy_death::window(action,0,.36f);
 const float snap=enemy_death::window(action,.38f,.56f);
 const float released=1-enemy_death::window(action,.60f,1.f);
 const float coil=side*fall*1.18f;
 return {actorFacing(direction),phase,pace*active,
         (.4f*std::sin(state.idleTime*3.5f)*(1-pace)+.78f*walk.bob*pace)*active,
         (8.f*snap-4.5f*ready)*released*active,
         (ready*.18f+snap*.82f)*released*active,
         (pace*walk.sway*2.2f+state.turnLean*.018f)*active,
         mix(3.1f*std::sin(state.idleTime*2.2f),9.4f*walk.sway,pace)*active+side*fall*11.f,
         fall,float(side),std::cos(coil),std::sin(coil)};
}

struct CrossSection {float forward,wide,top,bottom;};
inline constexpr CrossSection Profile[]{
 {-65,.10f,2,1.8f},{-61,1.1f,4.4f,1.4f},{-55,2.0f,8.0f,2.2f},
 {-48,3.2f,10.9f,3.0f},{-41,4.1f,12.5f,3.9f},{-35,5.7f,16.3f,5.4f},
 {-28,7.9f,20.5f,7.0f},{-21,10.9f,24.6f,8.5f},{-14,13.2f,27.9f,8.6f},
 {-6,14.3f,30.0f,8.2f},{2,14.8f,30.2f,7.7f},{10,13.7f,28.7f,8.0f},
 {16,12.1f,27.1f,9.4f},{20,9.4f,25.8f,11.3f},{24,8.9f,26.6f,12.4f},
 {28,10.6f,29.6f,12.9f},{32,10.1f,29.3f,13.6f},{36,8.8f,26.9f,14.1f},
 {41,7.0f,23.9f,14.8f},{44,5.2f,22.2f,15.6f},{45.5f,.14f,19.2f,18.9f}
};

inline CrossSection shape(float along){
 const auto next=std::upper_bound(Profile+1,Profile+std::size(Profile)-1,along,
  [](float f,const CrossSection& s){return f<s.forward;});
 const auto& a=next[-1];const auto& b=*next;
 const float t=std::clamp((along-a.forward)/(b.forward-a.forward),0.f,1.f);
 return {along,mix(a.wide,b.wide,t),mix(a.top,b.top,t),mix(a.bottom,b.bottom,t)};
}

inline SurfacePoint surface(const Pose& r,const CrossSection& s,float angle,float relief=0){
 const float c=std::cos(angle),lateral=std::sin(angle);
 const float middle=(s.top+s.bottom)*.5f;
 const float split=std::clamp((s.forward-24.f)/21.f,0.f,1.f);
 const float jaw=std::max(0.f,-c)*split*r.gape*5.5f;
 return r.project(s.forward,lateral*(s.wide+relief),middle+c*((s.top-s.bottom)*.5f+relief)-jaw);
}

inline void drawBody(Mesh& mesh,const Pose& r){
 constexpr int StripCount=20;
 constexpr Color skin{90,105,63,255},ridge{85,99,71,255},belly{107,91,57,255};
 const auto point=[&](const CrossSection& section,int strip,art::Material material){
  const float angle=2*PI*strip/StripCount;
  const float c=std::cos(angle),d=std::sin(angle);
  const auto p=surface(r,section,angle);
  const auto pigment=material==art::DrakeScale?ridge:material==art::CrawlerSkin?belly:skin;
  const float shade=.82f+.23f*c+.10f*d*(.45f*r.axes.x-.70f*r.axes.y);
  return Vertex{p.screen,{(section.forward+65.f)/111.f,.05f+.90f*std::acos(std::clamp(c,-1.f,1.f))/PI},
                art::light(pigment,shade*(1-.17f*r.fall)),p.depth};
 };
 for(size_t slice=1;slice<std::size(Profile);++slice)for(int strip=0;strip<StripCount;++strip){
  const float band=std::cos((strip+.5f)*2*PI/StripCount);
  const auto material=band>.81f&&Profile[slice].forward<23.f?art::DrakeScale:band<-.58f?art::CrawlerSkin:art::DrakeSkin;
  const auto a=point(Profile[slice-1],strip,material),b=point(Profile[slice-1],strip+1,material);
  const auto c=point(Profile[slice],strip,material),d=point(Profile[slice],strip+1,material);
  mesh.triangle(a,b,c,material);mesh.triangle(b,d,c,material);
 }
}

// A mark follows the curved surface instead of rotating independently of it.
inline void inlay(Mesh& mesh,const Pose& r,float along,float angle,float length,float width,float raised,
                 Color middle,Color rim,art::Material material=art::Paint){
 const auto point=[&](float f,float a,Color color,float u,float v){
  const auto p=surface(r,shape(f),a,raised);
  return Vertex{p.screen,{u,v},color,p.depth+.44f+raised*.6f};
 };
 const auto center=point(along,angle,middle,.41f,.40f);
 Vertex previous=point(along+length,angle,rim,.82f,.40f);
 for(int i=1;i<=12;i++){
  const float t=i*2*PI/12.f,c=std::cos(t),s=std::sin(t);
  const auto edge=point(along+length*c,angle+width*s,rim,.41f+.41f*c,.40f+.37f*s);
  mesh.triangle(center,previous,edge,material);
  previous=edge;
 }
}

inline void drawScutes(Mesh& mesh,const Pose& r){
 for(float flank:{-1.f,1.f}){
  for(float along:{-39.f,-31.f,-23.f,-15.f,-6.f,3.f,12.f,19.f}){
   const float ruff=along<-20.f?.74f:1.f;
   inlay(mesh,r,along,flank*.32f,4.25f*ruff,.18f,1.5f*ruff,
         {107,114,81,255},{55,65,48,255},art::DrakeScale);
  }
  struct Fleck {float f,a,length;};
  constexpr Fleck flecks[]{{-31,.94f,.80f},{-21,1.07f,1.32f},{-15,.72f,1.5f},
   {-9,1.23f,1.6f},{-1,.88f,2.2f},{7,1.17f,1.55f},{16,.77f,1.37f},{22,1.31f,1.0f}};
  const float fade=1-.44f*r.fall;
  for(const auto& f:flecks)
   inlay(mesh,r,f.f+flank*.65f,flank*f.a,f.length,.074f,.65f,
         art::light({222,151,52,255},fade),art::light({108,66,33,255},fade));
 }
}

inline void drawHead(Mesh& mesh,const Pose& r){
 for(float flank:{-1.f,1.f}){
  inlay(mesh,r,32.0f,flank*1.05f,3.9f,.19f,.63f,{126,137,92,255},{62,72,46,255},art::DrakeScale);
  inlay(mesh,r,32.3f,flank*1.24f,2.32f,.165f,1.0f,{32,32,24,255},{41,46,33,255});
  if(r.fall<.78f){
   const float fade=1-enemy_death::window(r.fall,.34f,.77f);
   inlay(mesh,r,32.62f,flank*1.23f,1.22f,.095f*fade,1.18f,
         {237,157,53,255},{171,102,35,255});
   inlay(mesh,r,32.69f,flank*1.225f,.31f,.10f*fade,1.32f,{33,23,19,255},{70,41,21,255});
  }
  inlay(mesh,r,41.05f,flank*.92f,.98f,.087f,.36f,{40,49,34,255},{92,105,64,255});
  const float open=.034f+.21f*r.gape;
  inlay(mesh,r,37.f,flank*(1.89f+.22f*r.gape),6.25f,open,.53f,
        {63,32,22,255},{55,51,35,255});
 }
}

inline void drawLeg(Mesh& mesh,const Pose& r,bool front,int flank){
 const float side=float(flank);
 const float stepPhase=art::loopPhase(r.phase+(front==(flank>0)?0.f:.5f)+(front?0.f:.085f));
 const auto step=volcano_motion::sample(LavaSalamanderType-VolcanoFirstType,stepPhase);
 const float axle=front?15.f:-17.f;
 const float stride=step.travel*(front?9.0f:10.7f)*r.pace;
 const float tuck=side*r.side*r.fall;
 const float pelvis=side*(front?8.5f:8.9f);
 const float reach=side*(front?20.0f:21.7f)*(1-.23f*tuck);
 const float heel=step.lift*5.1f*r.pace;
 const float foot=axle+stride+(front?4.1f:-3.8f)+4.0f*tuck;
 const auto root=r.project(axle,pelvis,16.3f);
 const auto shoulder=r.project(axle+(front?1.8f:-2.2f),side*13.8f,14.0f);
 const auto knee=r.project(axle+stride*.36f+(front?-2.8f:4.3f),reach*.85f,7.5f+heel*.48f);
 const auto wrist=r.project(foot,reach,1.1f+heel);
 const auto palm=r.project(foot+2.1f,reach+side*.7f,1.0f+heel*.84f);
 const Color pigment=art::light({90,104,61,255},1-.19f*r.fall);
 cinderhorn_turn::ribbon(mesh,std::array<SectionJoint,5>{{
    {root,5.05f},{shoulder,5.55f},{knee,3.5f},{wrist,2.15f},{palm,2.0f}}},
    pigment,art::DrakeSkin,front?.40f:.68f);
 for(int toe=-1;toe<=1;++toe){
  const float spread=side*toe;
  const auto joint=r.project(foot+3.f,reach+spread*1.4f,1.4f+heel*.83f);
  const auto pad=r.project(foot+5.55f-.52f*std::abs(toe),reach+spread*2.45f,1.05f+heel*.78f);
  const auto claw=r.project(foot+7.0f-.66f*std::abs(toe),reach+spread*3.1f,.50f+heel*.72f);
  cinderhorn_turn::ribbon(mesh,std::array<SectionJoint,3>{{{palm,1.25f},{joint,1.07f},{pad,.73f}}},
                        pigment,art::DrakeSkin,.63f);
  cinderhorn_turn::ribbon(mesh,std::array<SectionJoint,2>{{{pad,.73f},{claw,.16f}}},
                        {166,138,100,255},art::Ivory);
 }
}

inline const Mesh& frame(const Pose& r){
 thread_local Mesh mesh;
 thread_local std::array<float,13> previous{};
 const std::array<float,13> key{r.axes.x,r.axes.y,r.phase,r.pace,r.bob,r.reach,r.gape,r.sway,
                               r.tailWave,r.fall,r.side,r.rollCos,r.rollSin};
 if(key!=previous){
  mesh.triangles.clear();
  if(mesh.triangles.capacity()<1250)mesh.triangles.reserve(1250);
  drawBody(mesh,r);
  for(bool front:{false,true})for(int flank:{-1,1})drawLeg(mesh,r,front,flank);
  drawScutes(mesh,r);
  drawHead(mesh,r);
  previous=key;
 }
 return mesh;
}

inline bool draw(float x,float y,float scale,const AnimationState& state,float action,float fall,int side,
                 Color outline,const Vector2* probe,bool shadow,bool flash,float opacity){
 const int flank=side?side:(int(state.direction)>3?-1:1);
 const auto& mesh=frame(pose(state,action,fall,flank));
 if(probe)return opacity>=.43f&&mesh.includes({(probe->x-x)/scale,(probe->y-y)/scale});
 if(opacity<=0)return false;
 rlPushMatrix();
 rlTranslatef(x,y,0);rlScalef(scale,scale,1);
 if(!shadow&&outline.a)for(Vector2 d:{Vector2{-1.5f,0},Vector2{1.5f,0},Vector2{0,-1.5f},Vector2{0,1.5f}})
  mesh.paint(materials,outline,opacity,true,art::mul(d,1/scale));
 if(shadow)mesh.paint(materials,WHITE,opacity,true);
 else cinderhorn_turn::paintSurface(mesh,materials,opacity);
 if(flash&&!shadow)mesh.paint(materials,Fade(WHITE,.44f),opacity,true);
 rlSetTexture(rlGetTextureIdDefault());rlPopMatrix();
 return false;
}

inline const AnimationState& lastPose(const Enemy& e){
 return e.deathMotion.facingInitialized||e.deathMotion.motionInitialized?e.deathMotion:e.animation;
}
inline bool fallen(const Enemy& e,float x,float y,float time,Color outline,const Vector2* probe,bool shadow,float opacity){
 return draw(x,y,1,lastPose(e),e.deathAction,time,e.deathScreenSide(),outline,probe,shadow,false,opacity);
}
inline Rectangle footprint(const Enemy& e){
 const auto r=pose(lastPose(e),-1,1,e.deathScreenSide());
 const float half=std::hypot(24.f*r.axes.x,15.f*r.axes.y);
 return {-half,-6,half*2,12};
}
}
