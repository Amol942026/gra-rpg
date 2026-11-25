#pragma once

// One painted 2D rig supplies every heading, including the pose at the first hit.
namespace cinderhorn_turn {
namespace art=caldera_quadruped;
using art::Mesh;
using art::Vertex;
using art::add;
using art::light;
using art::mix;
using art::mul;
using art::sub;
inline Texture2D ink{};
inline std::array<float,64> heights{};
inline constexpr Color fur{81,77,74,255};
inline constexpr Color horn{190,164,140,255},hoof{75,72,69,255};
inline constexpr float StandingHeight=82.f;
inline constexpr float PollForward=34.5f,PollHeight=49.7f;
inline constexpr float RumpForward=-37.f,MuzzleForward=50.f;
inline bool ready(){return ink.id!=0;}
inline float neckWeight(float forward){
 const float t=std::clamp((forward-13)/(PollForward-13),0.f,1.f);
 return t*t*(3-2*t);
}

template<size_t N>
inline Vector2 track(const cinderhorn_synfig::Key (&keys)[N],float time){
 const auto* next=std::upper_bound(keys+1,keys+N-1,time,[](float t,const auto& key){return t<key.time;});
 const auto& a=next[-1];const auto& b=next[0];
 const float t=std::clamp((time-a.time)/(b.time-a.time),0.f,1.f);
 return {mix(a.along,b.along,t),mix(a.lift,b.lift,t)};
}

struct SurfacePoint {Vector2 screen;float depth;};

struct Pose {
 float angle,phase,pace,idle,pivot,bob,pitch;
 Vector2 head;
 bool run;
 ActorFacing axes;
 Vector2 roll{1,0};
 Vector2 foreFold{},hindFold{};
 Vector2 weightShift{},tail{};
 float support=0,eyeOpen=1;
 SurfacePoint project(float forward,float side,float height)const{
  const float fromSpine=height-35.f;
  height+=fromSpine*(roll.x-1)+side*roll.y;
  side=side*roll.x-fromSpine*roll.y;
  const float ground=axes.y*forward+axes.x*side;
  return {{axes.x*forward-axes.y*side,ActorGroundSquash*ground-height},ground+ActorGroundSquash*height};
 }
 SurfacePoint mounted(float forward,float side,float height)const{
  return project(forward,side+weightShift.x,height+bob+pitch*forward+weightShift.y*side);
 }
 Vector2 neck(float forward,float height,float attached)const{
  return {forward+head.x*attached,height+head.y*attached};
 }
 Vector2 skull(float forward,float height)const{
  const float angle=std::clamp(head.y*.038f,-.34f,.11f);
  const float dx=forward-PollForward,dy=height-PollHeight;
  const float c=std::cos(angle),s=std::sin(angle);
  return {PollForward+head.x+c*dx-s*dy,PollHeight+head.y+s*dx+c*dy};
 }
 SurfacePoint face(float forward,float side,float height)const{
  const auto point=skull(forward,height);
  return mounted(point.x,side,point.y);
 }
};

inline Pose pose(const Enemy& e){
 const auto& a=e.animation;
 const float angle=a.facingInitialized?a.facingAngle:facingAngles[std::clamp(int(a.direction),0,7)];
 const bool run=e.boarRush>0;
 float action=0;
 if(run)action=.5f;
 else if(e.boarWindup>0)action=.5f*(1-e.boarWindup/CinderhornCharge.windup);
 else if(e.boarRecovery>0)action=1-.5f*e.boarRecovery/CinderhornCharge.recovery;
 else if(e.windup>0)action=.5f*(1-e.windup/meleeWindup(e.type));
 else if(e.strike>0)action=1-.5f*e.strike/meleeRecovery(e.type);
 const auto dip=track(cinderhorn_synfig::lower,action);
 const float movement=a.motionInitialized?a.moveBlend:float(a.moving);
 const float pace=!run&&(e.boarWindup>0||e.boarRecovery>0||e.windup>0||e.strike>0)?0.f:movement;
 const float error=std::abs(std::remainder(a.targetAngle-angle,2*PI));
 const float pivot=(1-pace)*std::min(.50f,error*.62f)*std::clamp(1+dip.y/11.f,0.f,1.f);
 const float phase=art::loopPhase(a.phase);
 const auto weight=run?track(cinderhorn_synfig::bodyRush,phase):track(cinderhorn_synfig::body,phase);
 const auto balance=run?track(cinderhorn_synfig::neckRush,phase):track(cinderhorn_synfig::neckWalk,phase);
 const float bob=.45f*std::sin(4*a.idleTime)*(1-pace)+weight.y*pace*(run?1.28f:1.f);
 const Vector2 head{dip.x+balance.x*pace,dip.y+balance.y*pace};
 Pose r{angle,phase,pace,a.idleTime,pivot,bob,weight.x*pace,head,run,actorFacing(angle)};
 if(!run)r.weightShift=mul(track(cinderhorn_synfig::walkLean,phase),pace);
 r.tail=mul(track(cinderhorn_synfig::tailWalk,phase),pace*(run?.60f:1.f));
 return r;
}

inline Pose fallingPose(const Enemy& last,int flank,float time){
 auto r=pose(last);
 const auto balance=track(cinderhorn_synfig::fallBalance,time);
 const float angle=float(flank)*balance.x*DEG2RAD;
 const float relaxed=enemy_death::window(time,0,.78f);
 r.roll={std::cos(angle),std::sin(angle)};
 r.support=balance.y;
 r.pace*=1-relaxed;
 r.pivot*=1-relaxed;
 r.bob*=1-relaxed;
 r.pitch*=1-relaxed;
 r.weightShift=mul(r.weightShift,1-relaxed);
 r.tail=mul(r.tail,1-relaxed);
 r.head=add(mul(r.head,1-relaxed),track(cinderhorn_synfig::fallHead,time));
 r.foreFold=track(cinderhorn_synfig::fallFront,time);
 r.hindFold=track(cinderhorn_synfig::fallHind,time);
 r.eyeOpen=1-enemy_death::window(time,.17f,.62f);
 return r;
}

inline Vector2 legBend(Vector2 shoulder,Vector2 ankle,bool fore){
 const float upper=fore?17.8f:17.f,lower=fore?18.4f:17.8f;
 const auto reach=sub(ankle,shoulder);
 const float distance=std::clamp(art::length(reach),.01f,upper+lower-.01f);
 const auto axis=mul(reach,1/std::max(.01f,art::length(reach)));
 const float span=(upper*upper-lower*lower+distance*distance)/(2*distance);
 const float bend=std::sqrt(std::max(0.f,upper*upper-span*span));
 const Vector2 normal{axis.y,-axis.x};
 return add(shoulder,add(mul(axis,span),mul(normal,fore?bend:-bend)));
}

struct SectionJoint {SurfacePoint joint;float radius;};

// Painted strips for the horns and tail.
template<size_t N>
inline void ribbon(Mesh& m,const std::array<SectionJoint,N>& joints,Color pigment,art::Material material,
                   float flankU=0){
 constexpr float lanes[]={-1,-.72f,0,.72f,1};
 constexpr float sheen[]={.76f,.96f,1.12f,.93f,.74f};
 constexpr float curvature[]={0,.65f,.94f,.65f,0};
 const auto section=[&](size_t i){
  const auto [joint,radius]=joints[i];
  const auto tangent=sub(joints[std::min(i+1,N-1)].joint.screen,joints[i?i-1:0].joint.screen);
  const auto normal=mul(Vector2{-tangent.y,tangent.x},radius/std::max(.08f,art::length(tangent)));
  const float u=float(i)/(N-1);
  std::array<Vertex,std::size(lanes)> strip;
  for(size_t lane=0;lane<strip.size();++lane){
   const float v=lanes[lane];
   const Vector2 uv=material==art::Ivory?Vector2{u,.5f+v*.47f}
                                :Vector2{flankU+.12f*v-.074f*u,.43f+.50f*u};
   strip[lane]={add(joint.screen,mul(normal,v)),uv,light(pigment,sheen[lane]*(1-.12f*u)),
                joint.depth+curvature[lane]*radius};
  }
  return strip;
 };
 auto a=section(0);
 for(size_t i=1;i<N;++i){
  const auto b=section(i);
  for(size_t lane=0;lane+1<a.size();++lane){
   m.triangle(a[lane],a[lane+1],b[lane],material);
   m.triangle(b[lane],a[lane+1],b[lane+1],material);
  }
  a=b;
 }
}

template<class Draw>
inline void detailAt(Mesh& m,float depth,Draw draw){
 const size_t start=m.triangles.size();
 draw();
 // Opaque detail is depth-tested with the body. A feathered edge would also
 // write depth where its color is transparent and hide the surface behind it.
 const auto first=m.triangles.begin()+start;
 m.triangles.erase(std::remove_if(first,m.triangles.end(),[](const auto& t){
  return t.a.color.a!=255||t.b.color.a!=255||t.c.color.a!=255;
 }),m.triangles.end());
 for(size_t i=start;i<m.triangles.size();++i){
  auto& t=m.triangles[i];
  t.a.depth=t.b.depth=t.c.depth=depth;
 }
}

struct HoofMotion {float along,lift,toe;};

inline float legPhase(float cycle,bool fore,int side,bool run){
 const float lead=fore?0.f:run?.21f:cinderhorn_gait::HindLead;
 return art::loopPhase(cycle+(side>0?0.f:.5f)+lead);
}

inline HoofMotion hoofMotion(const Pose& r,bool fore,int side){
 const float phase=legPhase(r.phase,fore,side,r.run);
 const auto stride=fore?(r.run?track(cinderhorn_synfig::rush,phase):track(cinderhorn_synfig::walk,phase))
                        :(r.run?track(cinderhorn_synfig::hindRush,phase):track(cinderhorn_synfig::hindWalk,phase));
 const auto turn=track(cinderhorn_synfig::walk,legPhase(2*r.idle,fore,side,false));
 const auto toe=fore?track(cinderhorn_synfig::walkToe,phase):track(cinderhorn_synfig::hindWalkToe,phase);
 const auto fold=fore?r.foreFold:r.hindFold;
 const float flank=side*r.roll.y;
 return {stride.x*r.pace+turn.x*r.pivot*.38f+fold.x*(1+.14f*flank),
         stride.y*r.pace+turn.y*r.pivot*.55f+fold.y*(1+.19f*flank),r.run?0.f:toe.x*r.pace};
}

// Every leg grows from a cutout in the painted flank. Its first loop shares
// the actual coat vertices, including their texture coordinates and lighting.
struct LegSeamPoint {Vertex coat;Vector2 radial;};
using LegSeam=std::vector<LegSeamPoint>;
using LegSeams=std::array<LegSeam,4>;
inline size_t legIndex(bool fore,int side){return (fore?2:0)+(side>0?1:0);}
inline Vector2 legSpan(bool fore){return fore?Vector2{10.8f,24.5f}:Vector2{-32.f,-17.f};}

inline void drawLeg(Mesh& m,const Pose& r,bool fore,int side,const LegSeam& seam){
 const auto [along,lift,toeAngle]=hoofMotion(r,fore,side);
 const float footLine=fore?17.9f:-20.f,footWidth=fore?12.6f:13.6f;
 const float root=(fore?18.6f:-21.25f)+(r.run?0.f:.085f*along*r.pace);
 const float width=fore?11.9f:10.9f,rootSide=side*width+r.weightShift.x;
 const float hipHeight=(fore?34.5f:34.15f)+r.bob+r.pitch*root+r.weightShift.y*side*width;
 const float out=side*(footWidth+1.8f+lift*.08f);
 const float ankleForward=footLine+along+(fore?.8f:-1.4f),ankleHeight=5.6f+lift*1.18f;
 const Vector2 hip{root,hipHeight},ankle{ankleForward,ankleHeight};
 const auto knee=legBend(hip,ankle,fore);
 const auto kneeCurve=mix(mix(hip,knee,.89f),mix(knee,ankle,.11f),.5f);
 const float toe=ankleForward+(fore?1.5f:1.9f)-.11f*lift;
 const auto pad=r.project(toe,out,2.1f+lift);
 // The broad haunch narrows into the hock, the shoulder into the foreleg.
 struct Section {Vector2 axis;float lateral,radius,thickness,t;};
 const Section sections[]={
  {mix(hip,knee,.32f),mix(rootSide,out,.17f),fore?6.15f:7.35f,fore?4.5f:5.4f,.19f},
  {mix(hip,knee,.72f),mix(rootSide,out,.34f),fore?4.8f:5.9f,fore?3.8f:4.5f,.35f},
  {kneeCurve,mix(rootSide,out,.46f),fore?4.15f:4.6f,3.6f,.49f},
  {mix(knee,ankle,.29f),mix(rootSide,out,.62f),3.65f,3.15f,.63f},
  {mix(knee,ankle,.76f),mix(rootSide,out,.88f),2.85f,2.45f,.82f},
  {ankle,out,2.7f,2.4f,.92f},
  {{toe,2.1f+lift},out,2.05f,2.05f,1.f}
 };
 std::array<Vector2,std::size(sections)> normals;
 for(size_t i=0;i<normals.size();++i){
  const auto v=sub(sections[std::min(i+1,normals.size()-1)].axis,i?sections[i-1].axis:hip);
  normals[i]=mul(Vector2{-v.y,v.x},1/std::max(.01f,art::length(v)));
 }
 const auto column=[&](const LegSeamPoint& edge){
  std::array<Vertex,std::size(sections)+1> vertices;
  vertices[0]=edge.coat;
  const auto [front,outer]=edge.radial;
  for(size_t i=0;i<std::size(sections);++i){
   const auto& joint=sections[i];
   const Vector2 center=add(joint.axis,mul(normals[i],front*joint.radius));
   const auto point=r.project(center.x,joint.lateral+side*outer*joint.thickness,center.y);
   const float shade=1.02f+.13f*outer*side*r.roll.x+.10f*front-.065f*joint.t;
   // Carry the same locks down from each rim point, including the inner leg.
   const Vector2 uv{edge.coat.uv.x-.055f*joint.t,
                    mix(edge.coat.uv.y,.95f-.07f*outer,joint.t)};
   const auto color=light(fur,shade);
   const float blend=std::min(1.f,2.4f*joint.t);
   vertices[i+1]={point.screen,uv,ColorLerp(edge.coat.color,color,blend),point.depth};
  }
  return vertices;
 };
 auto a=column(seam.back());
 for(size_t i=0;i<seam.size();++i){
  const auto& edge=seam[i];
  const auto b=column(edge);
  for(size_t j=1;j<a.size();++j){
   m.triangle(a[j-1],b[j-1],a[j],art::DrakeSkin);
   m.triangle(b[j-1],b[j],a[j],art::DrakeSkin);
  }
  // Short guard hairs turn with the coat. Both edges of each lock lie on
  // the same leg surface, so its width foreshortens naturally in profile.
  if(edge.radial.y>-.3f&&i%2==0){
   const size_t start=1+i%3;
   constexpr float widths[]={.03f,.23f,.16f,.01f};
   constexpr Color tones[]={{74,64,57,255},{139,123,106,255},{113,99,85,255},{88,76,66,255}};
   std::array<Vertex,2> previous{};
   for(size_t k=0;k<std::size(widths);++k){
    const size_t j=start+k;
    const float u=.58f+.12f*std::sin(float(i))-.10f*k;
    const auto hair=[&](float v){
     return Vertex{mix(a[j].point,b[j].point,v),{},tones[k],mix(a[j].depth,b[j].depth,v)+.09f};
    };
    const std::array<Vertex,2> strand{hair(u-widths[k]),hair(u+widths[k])};
    if(k){
     m.triangle(previous[0],previous[1],strand[0],art::Paint);
     m.triangle(strand[0],previous[1],strand[1],art::Paint);
    }
    previous=strand;
   }
  }
  a=b;
 }
 const float widthOnScreen=std::hypot(3.8f*r.axes.x,3.7f*r.axes.y);
 const float pitch=toeAngle*DEG2RAD*r.axes.x;
 const float c=std::cos(pitch),s=std::sin(pitch);
 const auto tip=[&](Vector2 v){return Vector2{c*v.x-s*v.y,s*v.x+c*v.y};};
 const Vector2 axis=tip({widthOnScreen,.59f*r.axes.x*r.axes.y});
 detailAt(m,pad.depth+3.15f,[&]{m.ellipse(pad.screen,axis,tip({0,2.36f}),hoof,art::CrawlerSkin,24);});
 const float frontToe=1.35f*r.axes.x;
 detailAt(m,pad.depth+3.25f,[&]{
  m.bone(add(pad.screen,tip({frontToe,-.72f})),add(pad.screen,tip({frontToe+.28f*r.axes.x,1.61f})),
         .35f,.23f,{44,42,41,255});
 });
}

inline void drawBody(Mesh& m,const Pose& r,LegSeams& seams){
 struct Section {float forward,width,back,rim,belly,sweep=0;};
 static constexpr Section profile[]={
  {RumpForward,0,35.2f,35.2f,35.2f},{-35.4f,5.5f,40.7f,35.6f,31.4f},{-32,9.5f,45.5f,36.3f,29.0f},
  {-27.4f,12.7f,48.2f,36.2f,27.6f},{-22.5f,13.4f,49.0f,35.6f,27.8f},{-17,11.9f,49.4f,36.1f,31.8f},
  {-12.5f,10.8f,50.3f,36.0f,32.3f},{-7.7f,12.3f,51.0f,35.7f,27.2f},{-2.6f,14.4f,51.5f,34.6f,22.2f},
  {4.4f,16.6f,52.5f,34.4f,22.5f},{10.8f,17.55f,55.f,35.0f,22.2f},{15.4f,17.1f,56.2f,35.6f,22.4f,.62f},
  {19.8f,16.1f,56.0f,36.8f,21.2f,.71f},{24.5f,13.5f,56.9f,40.6f,27.8f,1.65f},
  {29.5f,10.2f,57.0f,45.6f,35.4f,1.25f},{33.4f,8.7f,56.8f,48.2f,40.4f,.38f},
  {36.4f,8.35f,57.8f,49.5f,42.4f},{39.1f,7.65f,56.55f,48.6f,41.65f},
  {42.2f,6.25f,53.7f,47.5f,40.95f},{45.4f,4.85f,50.4f,46.5f,41.4f},
  {48,3.55f,48.2f,45.4f,42.4f},{49.4f,2.35f,46.7f,45.05f,43.2f},{MuzzleForward,0,45,45,45}
 };
 // Round the profile between the guides without overshooting a cheek,
 // the ridge of the back, or a closed end.
 static const auto sections=[] {
  constexpr size_t size=std::size(profile);
  std::vector<Section> result{profile[0]};
  for(size_t i=0;i+1<size;++i){
   const float length=profile[i+1].forward-profile[i].forward;
   const int steps=std::max(1,int(std::ceil(length/2.45f)));
   for(int n=1;n<=steps;++n){
    const float t=float(n)/steps;
    const auto curve=[&](float Section::* field){
     const auto slope=[&](size_t j){return (profile[j+1].*field-profile[j].*field)/
                                      (profile[j+1].forward-profile[j].forward);};
     const auto tangent=[&](size_t j){
      if(j==0)return slope(0);
      if(j+1==size)return slope(j-1);
      const float a=slope(j-1),b=slope(j);
      return a*b>0?2*a*b/(a+b):0.f;
     };
     const float a=profile[i].*field,b=profile[i+1].*field;
     const float da=tangent(i)*length,db=tangent(i+1)*length;
     return a+t*(da+t*(3*(b-a)-2*da-db+t*(2*(a-b)+da+db)));
    };
    result.push_back({mix(profile[i].forward,profile[i+1].forward,t),curve(&Section::width),
                      curve(&Section::back),curve(&Section::rim),curve(&Section::belly),curve(&Section::sweep)});
   }
  }
  return result;
 }();
 constexpr size_t around=16;
 static const auto ringAxes=[] {
  std::array<Vector2,around> axes;
  for(size_t i=0;i<around;++i){
   const float angle=2*PI*i/around;
   axes[i]={std::sin(angle),std::cos(angle)};
  }
  return axes;
 }();
 const auto coat=[&](Section s,Vector2 radial,float arc,float rise){
  const float intoHead=std::clamp((s.forward-30.8f)/(PollForward-30.8f),0.f,1.f);
  const float skullWeight=intoHead*intoHead*(3-2*intoHead);
  const auto [lateral,arch]=radial;
  // Keep some vertical plane on the rib cage, tapering into the round neck.
  const float rib=std::clamp((29-s.forward)/10.f,0.f,1.f);
  const float roundedFlank=rib*rib*(3-2*rib);
  const float forward=s.forward-(1-skullWeight)*1.5f*lateral*lateral*s.width/20+s.sweep*(2*arc-1);
  const float height=s.rim+(arch>0?s.back-s.rim:s.rim-s.belly)*arch*(1+.17f*roundedFlank*lateral*lateral);
  const auto point=mix(r.neck(forward,height,neckWeight(s.forward)),r.skull(forward,height),skullWeight);
  return r.mounted(point.x,(s.width*(1+.09f*roundedFlank*arch*arch)+rise)*lateral,point.y+rise*arch);
 };
 const auto coatVertex=[&](const Section& s,size_t i){
  const auto [lateral,arch]=ringAxes[i];
  const float arc=float(std::min(i,around-i))/(around/2);
  const Vector2 uv{.035f+.93f*(s.forward-RumpForward)/(MuzzleForward-RumpForward),.055f+.88f*arc};
  const auto surface=coat(s,ringAxes[i],arc,0);
  const float shade=.88f+.25f*(arch*r.roll.x+lateral*r.roll.y)+.061f*(lateral*r.roll.x-arch*r.roll.y);
  return Vertex{surface.screen,uv,light(fur,shade),surface.depth};
 };
 const auto ring=[&](const Section& s){
  std::array<Vertex,around> result;
  for(size_t i=0;i<around;++i)result[i]=coatVertex(s,i);
  return result;
 };
 constexpr size_t outer=3,inner=7;
 for(bool fore:{false,true}){
  const auto span=legSpan(fore);
  const auto sectionAt=[&](float x){
   return std::lower_bound(sections.begin(),sections.end(),x-.001f,
                           [](const Section& s,float f){return s.forward<f;});
  };
  const auto first=sectionAt(span.x),last=sectionAt(span.y);
  for(int side:{-1,1}){
   auto& seam=seams[legIndex(fore,side)];
   seam.clear();
   const auto append=[&](const Section& s,size_t j){
    const float x=std::clamp(2*(s.forward-span.x)/(span.y-span.x)-1,-1.f,1.f);
    const float y=1.f-2.f*float(j-outer)/float(inner-outer);
    // Map the rectangular rim to an oval limb without crowding its corners.
    seam.push_back({coatVertex(s,side>0?j:around-j),
                    {x*std::sqrt(1-.5f*y*y),y*std::sqrt(1-.5f*x*x)}});
   };
   for(auto s=first;s!=last;++s)append(*s,outer);
   for(size_t j=outer;j<inner;++j)append(*last,j);
   for(auto s=last;s!=first;--s)append(*s,inner);
   for(size_t j=inner;j>outer;--j)append(*first,j);
  }
 }
 const auto face=[&](Vertex a,Vertex b,Vertex c){
  if(art::cross(sub(b.point,a.point),sub(c.point,a.point))>.005f)m.triangle(a,b,c,art::DrakeSkin);
 };
 auto a=ring(sections.front());
 for(size_t section=1;section<sections.size();++section){
  const auto b=ring(sections[section]);
  const float forward=.5f*(sections[section-1].forward+sections[section].forward);
  const auto span=legSpan(forward>0);
  const bool socket=forward>span.x&&forward<span.y;
  for(size_t i=0;i<around;++i){
   if(socket&&((i>=outer&&i<inner)||(i>=around-inner&&i<around-outer)))continue;
   const auto j=(i+1)%around;
   face(a[i],b[i],b[j]);face(a[i],b[j],a[j]);
  }
  a=b;
 }

 // Long guard hairs break the outer edge of the coat and follow the same
 // moving surface as their roots. Their own depth hides the far-side tips.
 struct Tuft {float forward,angle,length,sweep,loft;};
 constexpr Tuft locks[]={
  {-20.5f,-.34f,10.2f,-.37f,2.25f},{-15.4f,.38f,11.f,.39f,2.55f},
  {-10.7f,-1.05f,10.6f,-.43f,1.70f},{-4.4f,.96f,11.7f,.51f,1.85f},
  {-1.5f,-.21f,9.7f,-.29f,2.70f},{4.6f,.24f,10.6f,.36f,2.65f},
  {6.2f,-.84f,11.1f,-.50f,2.15f},{10.9f,1.57f,9.1f,.41f,1.55f},
  {11.5f,-1.58f,9.6f,-.36f,1.58f},{15.5f,.72f,12.3f,.48f,2.25f},
  {17.6f,-.45f,10.9f,-.39f,2.95f},{19.2f,1.37f,10.1f,.44f,1.58f},
  {21.3f,-1.22f,11.8f,-.49f,2.14f},{24.0f,.29f,10.6f,.31f,3.05f},
  {27.6f,-.61f,11.35f,-.44f,2.81f},{31.3f,.76f,10.1f,.47f,2.54f}
 };
 const auto sample=[&](float forward,float angle,float rise){
  const auto next=std::upper_bound(sections.begin()+1,sections.end()-1,forward,
                                   [](float x,const Section& s){return x<s.forward;});
  const auto& a=next[-1];const auto& b=*next;
  const float t=std::clamp((forward-a.forward)/(b.forward-a.forward),0.f,1.f);
  const Section s{forward,mix(a.width,b.width,t),mix(a.back,b.back,t),mix(a.rim,b.rim,t),
                  mix(a.belly,b.belly,t),mix(a.sweep,b.sweep,t)};
  return coat(s,{std::sin(angle),std::cos(angle)},std::abs(angle)/PI,rise);
 };
 constexpr float along[]={0,.23f,.51f,.77f,.94f};
 constexpr float widths[]={.41f,.52f,.50f,.40f,.22f};
 constexpr Color shades[]={{51,45,40,255},{101,90,81,255},{154,139,126,255},{131,117,104,255},{104,91,81,255}};
 for(const auto& lock:locks)for(int hair=-1;hair<=1;++hair){
  std::array<SurfacePoint,std::size(along)> points;
  for(size_t i=0;i<points.size();++i){
   const float t=along[i],bow=t*(1-t);
   const float forward=lock.forward-(lock.length+1.7f*hair)*t;
   const float angle=lock.angle+lock.sweep*t+hair*(.05f+.10f*t)+.27f*bow;
   points[i]=sample(forward,angle,-.66f+lock.loft*t+4.7f*bow);
  }
  const auto stroke=[&](size_t i){
   const auto point=points[i];
   const auto d=sub(points[std::min(i+1,points.size()-1)].screen,points[i?i-1:0].screen);
   const auto normal=mul(Vector2{-d.y,d.x},widths[i]/std::max(.16f,art::length(d)));
   const Color tint=light(shades[i],.92f-.08f*hair+.10f*std::cos(lock.angle));
   const float depth=point.depth+.048f*hair;
   return std::array<Vertex,2>{{{add(point.screen,normal),{},tint,depth},
                               {sub(point.screen,normal),{},light(tint,.74f),depth}}};
  };
  auto a=stroke(0);
  for(size_t i=1;i<points.size();++i){
   const auto b=stroke(i);
   m.triangle(a[0],a[1],b[0],art::Paint);m.triangle(a[1],b[1],b[0],art::Paint);
   a=b;
  }
 }
}

inline void drawHorn(Mesh& m,const Pose& r,int side){
 const std::array<SectionJoint,11> hornCurve{{
  {r.face(36.2f,side*5.4f,53.8f),3.9f},{r.face(35,side*8.1f,57.6f),5.25f},
  {r.face(31.8f,side*10.9f,62.8f),5.25f},
  {r.face(25,side*13.8f,69),4.8f},{r.face(17.5f,side*17.8f,73),4.20f},
  {r.face(9,side*20.1f,74.2f),3.48f},{r.face(2,side*22.1f,72.6f),2.87f},
  {r.face(-4,side*22.8f,69.2f),2.12f},{r.face(-8,side*22.8f,65),1.45f},
  {r.face(-9.4f,side*21.6f,61.1f),.72f},{r.face(-9.4f,side*20.4f,58.7f),.10f}
 }};
 ribbon(m,hornCurve,light(horn,.87f+.13f*r.axes.x*side),art::Ivory);
}

inline void drawHeadDetails(Mesh& m,const Pose& r){
 const auto nose=r.face(49.25f,0,45.f);
 const float front=std::clamp((r.axes.y+.86f)*1.85f,0.f,1.f);
 struct EarPoint {float forward,side,height;};
 constexpr EarPoint ear[]={
  {37.4f,6.45f,52.2f},{35.9f,8.7f,52.75f},{34.2f,11.15f,52.25f},{33.4f,12.05f,51.45f},
  {33.62f,12.30f,50.77f},{34.7f,11.35f,50.0f},{36.4f,9.0f,49.65f},{37.7f,6.6f,50.35f}
 };
 for(int side:{-1,1}){
  const auto dish=r.face(35.65f,side*9.55f,51.05f);
  const Vertex middle{dish.screen,{.91f,.62f},light(fur,.73f),dish.depth};
  const auto edge=[&](size_t i){
   const auto [f,w,h]=ear[i];
   const auto p=r.face(f,side*w,h);
   const Vector2 uv{.86f+.019f*(f-33.4f),.48f+.10f*(52.75f-h)};
   return Vertex{p.screen,uv,light(fur,.94f+.1f*(h-51)),p.depth};
  };
  for(size_t i=0;i<std::size(ear);++i)m.triangle(middle,edge(i),edge((i+1)%std::size(ear)),art::DrakeSkin);
 }
 if(front>.01f){
  const Vector2 width{1.7f+1.85f*std::abs(r.axes.y),.37f*r.axes.x};
  detailAt(m,nose.depth+.55f,[&]{m.ellipse(nose.screen,mul(width,front),{0,1.94f*front},hoof,art::CrawlerSkin,20);});
 }
 for(int side:{-1,1}){
  const float visibility=std::clamp((r.axes.x*side+.42f)/.67f,0.f,1.f)*front;
  if(visibility<.01f)continue;
  const auto eye=r.face(40.05f,side*6.95f,51.6f);
  const auto eyeAxis=Vector2{2.16f*visibility,-.31f*r.axes.x*visibility};
  detailAt(m,eye.depth+.35f,[&]{
   m.ellipse(eye.screen,eyeAxis,{0,mix(.23f,1.19f,r.eyeOpen)*visibility},{29,25,24,255},art::Paint,14);
  });
  detailAt(m,eye.depth+.44f,[&]{
   m.ellipse(add(eye.screen,{0,-.12f}),mul(eyeAxis,.59f*r.eyeOpen),{0,.56f*visibility*r.eyeOpen},{222,139,66,255},art::Paint,12);
  });
  const auto nostril=r.face(49.05f,side*1.7f,45.55f);
  detailAt(m,std::max(nostril.depth,nose.depth+.55f)+.09f,[&]{
   m.oval(nostril.screen,.85f*visibility,.51f*visibility,{38,34,34,255},art::Paint,12);
  });
 }
}

inline Rectangle extent(const Mesh& mesh);

inline void drawTail(Mesh& m,const Pose& r){
 const auto joint=[&](float back,float side,float height,float radius,float swing){
  return SectionJoint{r.mounted(RumpForward+back,side+swing*r.tail.x,height+swing*r.tail.y),radius};
 };
 const std::array<SectionJoint,7> tail{{
  joint(5.6f,.1f,40.1f,2.45f,0),joint(2.1f,.4f,39.2f,2.9f,.09f),
  joint(-.6f,.8f,37.7f,3.0f,.27f),joint(-3.4f,1.5f,35.4f,2.82f,.48f),
  joint(-5.3f,2.55f,32.4f,2.40f,.71f),joint(-6.0f,3.35f,29.55f,1.55f,.87f),
  joint(-6.2f,3.85f,27.f,.40f,1)
 }};
 const Color hair{108,96,85,255};
 ribbon(m,tail,hair,art::DrakeSkin,.135f);
 for(float side:{-1.f,1.f}){
  const std::array<SectionJoint,3> tuft{{tail[3],
   joint(-5.25f,2.55f+side*1.55f,32.2f,1.10f,.71f),
   joint(-6.3f,3.55f+side*1.55f,28.65f+side*.78f,.36f,.91f)
  }};
  ribbon(m,tuft,light(hair,1+.09f*side),art::DrakeSkin,.135f);
 }
}

inline Rectangle build(Mesh& m,const Pose& r){
 thread_local LegSeams seams;
 drawBody(m,r,seams);
 const auto body=r.support>0?extent(m):Rectangle{};
 drawTail(m,r);
 for(int side:{-1,1}){
  for(bool fore:{false,true})drawLeg(m,r,fore,side,seams[legIndex(fore,side)]);
  drawHorn(m,r,side);
 }
 drawHeadDetails(m,r);
 return body;
}

inline Rectangle extent(const Mesh& mesh){
 float x=1000,y=1000,r=-1000,b=-1000;
 for(const auto& t:mesh.triangles)for(const auto& p:{t.a,t.b,t.c}){
  if(p.color.a<250)continue;
  x=std::min(x,p.point.x);y=std::min(y,p.point.y);
  r=std::max(r,p.point.x);b=std::max(b,p.point.y);
 }
 return {x,y,r-x,b-y};
}

inline void load(){
 art::loadGrain(ink,"assets/volcano/cinderhorn-materials.png");
 if(!ink.id)return;
 SetTextureFilter(ink,TEXTURE_FILTER_ANISOTROPIC_4X);
 if(heights.front()>0)return;
 Mesh reference;
 for(size_t dir=0;dir<heights.size();++dir){
  const float angle=2*PI*dir/heights.size();
  reference.triangles.clear();
  build(reference,{angle,0,0,0,0,0,0,{},false,actorFacing(angle)});
  heights[dir]=extent(reference).height;
 }
}

inline void unload(){if(ink.id)UnloadTexture(ink);ink={};}

inline float heightScale(float angle){
 const int count=static_cast<int>(heights.size());
 const float position=art::loopPhase(angle/(2*PI))*count;
 const auto i=int(position);const float t=position-i;
 const auto h=[&](int offset){return heights[(i+offset+count)%count];};
 const float a=h(-1),b=h(0),c=h(1),d=h(2);
 const float fitted=b+.5f*t*(c-a+t*(2*a-5*b+4*c-d+t*(3*(b-c)+d-a)));
 return StandingHeight/std::max(1.f,fitted);
}

struct Frame {
 Mesh mesh;
 Rectangle body{};
 float riseScale=1,drop=0;
 std::array<float,23> key{};
};

inline const Frame& frame(const Pose& r){
 thread_local Frame current;
 const std::array<float,23> key{r.angle,r.phase,r.pace,r.idle,r.pivot,r.head.x,r.head.y,r.bob,r.pitch,float(r.run),
   r.roll.x,r.roll.y,r.foreFold.x,r.foreFold.y,r.hindFold.x,r.hindFold.y,r.weightShift.x,r.weightShift.y,
   r.tail.x,r.tail.y,r.support,r.eyeOpen,1.f};
 if(key!=current.key){
  current.key=key;current.riseScale=heightScale(r.angle);
  current.mesh.triangles.clear();
  if(current.mesh.triangles.capacity()<1500)current.mesh.triangles.reserve(1500);
  current.body=build(current.mesh,r);
  current.drop=-r.support*(current.body.y+.80f*current.body.height);
 }
 return current;
}

inline const Frame& frame(const Enemy& e){return frame(pose(e));}

inline Rectangle groundFootprint(const Enemy& last,int flank){
 const auto body=frame(fallingPose(last,flank,1)).body;
 const float size=cinderhorn_gait::Scale;
 const float width=std::max(44.f,body.width*size*.74f);
 return {(body.x+body.width*.5f)*size-width*.5f,-8.f,width,12.f};
}

inline void paintSurface(const Mesh& mesh,Texture2D texture,float opacity){
 const auto band=actor_art::reserveSpriteDepth();
 const Vector2 depth{band.x+band.y*.5f,band.y/220.f};
 static const auto compare=reinterpret_cast<void(*)(unsigned)>(glfwGetProcAddress("glDepthFunc"));
 rlEnableDepthTest();
 if(compare)compare(0x0201); // GL_LESS
 if(opacity<1&&compare){
  rlColorMask(false,false,false,false);
  mesh.paint(texture,WHITE,1,true,{},depth);
  rlDrawRenderBatchActive();
  rlColorMask(true,true,true,true);
  compare(0x0202); // GL_EQUAL. Blend only the visible surface during the fall.
 }
 mesh.paint(texture,WHITE,opacity,false,{},depth);
 rlDrawRenderBatchActive();
 if(compare)compare(0x0201);
 rlDisableDepthTest();
}

inline void paintSurface(const Mesh& mesh,float opacity){paintSurface(mesh,ink,opacity);}

inline bool drawFrame(const Frame& f,float x,float y,Color outline,const Vector2* probe,bool shadow,float scale,float opacity){
 const float s=scale*cinderhorn_gait::Scale,sy=s*f.riseScale;
 y+=f.drop*sy;
 if(probe)return opacity>=.43f&&f.mesh.includes({(probe->x-x)/s,(probe->y-y)/sy});
 if(opacity<=0)return false;
 rlPushMatrix();rlTranslatef(x,y,0);rlScalef(s,sy,1);
 if(outline.a&&!shadow)for(const auto offset:{Vector2{-1.5f/s,0},Vector2{1.5f/s,0},Vector2{0,-1.5f/sy},Vector2{0,1.5f/sy}})
  f.mesh.paint(ink,outline,opacity,true,offset);
 if(shadow)f.mesh.paint(ink,WHITE,opacity,true);
 else paintSurface(f.mesh,ink,opacity);
 rlSetTexture(rlGetTextureIdDefault());rlPopMatrix();
 return false;
}

inline bool draw(const Enemy& e,float x,float y,Color outline,const Vector2* probe,bool shadow,float scale,float opacity){
 return drawFrame(frame(e),x,y,outline,probe,shadow,scale,opacity);
}

inline bool drawFallen(const Enemy& last,int flank,float time,float x,float y,Color outline,const Vector2* probe,bool shadow,float opacity){
 return drawFrame(frame(fallingPose(last,flank,time)),x,y,outline,probe,shadow,1,opacity);
}
}
