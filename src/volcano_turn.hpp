#pragma once

// Included inside volcano_actors after Part and facingAngles have been defined.
namespace turn_art {
using Tile=::volcano_turn_data::Tile;
namespace drake_data = ::ember_drake_turn_atlas;
namespace crawler_data = ::slag_crawler_turn_atlas;
namespace imp_data = ::fire_imp_turn_atlas;
namespace sentinel_data = ::obsidian_sentinel_turn_atlas;
static_assert(drake_data::Count==32 && drake_data::Parts==6 && drake_data::Cell==160 && drake_data::PageCount<=2);
static_assert(crawler_data::Count==32 && crawler_data::Parts==6 && crawler_data::Cell==160 && crawler_data::PageCount<=2);
static_assert(imp_data::Count==32 && imp_data::Parts==6 && imp_data::Cell==160 && imp_data::PageCount<=2);
static_assert(sentinel_data::Count==32 && sentinel_data::Parts==6 && sentinel_data::Cell==160 && sentinel_data::PageCount<=2);
struct Page {Texture2D color{},mask{};std::vector<unsigned char> alpha;};
using SupportContour=std::array<Vector2,24>;
inline Page impCels{};
inline constexpr int ImpPaintedDirections=16;
inline painted_turn_shape::Material impContour;
struct Artwork {
 const char* folder;
 int pageCount;
 Tile (*tile)(int view,int part,int jaw);
 int (*authoredPart)(int key,int part);
 const int (*order)[6];
 std::array<Page,2> pages{};
 Texture2D flow{};
 std::vector<Color> vectors;
 bool signedEdges=false;
 std::array<Tile,8> shell{};
 std::array<float,32> standingHeight{};
 std::array<Page,2> cels{};
 std::vector<SupportContour> contactContours;
 bool ready()const{return pages[0].color.id!=0;}
 bool directCels()const{return cels[0].color.id!=0;}
};
inline constexpr int drakeOrder[8][6]={
 {0,1,2,3,4,5},{0,1,2,3,4,5},{0,1,2,3,4,5},{0,1,2,3,4,5},
 {0,1,2,3,4,5},{0,1,2,5,3,4},{0,1,2,5,3,4},{0,1,2,5,3,4}
};
// The shell hides the far paws. Viewed from behind, the segmented tail
// crosses in front of the rump while the forelegs recede behind it.
inline constexpr int crawlerOrder[8][6]={
 {0,1,4,3,2,5},{0,1,4,3,2,5},{0,1,2,3,4,5},{0,2,5,3,1,4},
 {0,2,5,3,1,4},{5,2,4,3,0,1},{4,5,3,1,2,0},{4,1,5,3,0,2}
};
inline Artwork drake{"assets/volcano/drake-turn/",drake_data::PageCount,
 [](int view,int part,int jaw){
  const auto& t=part==5 && jaw ? drake_data::mouth[view][jaw-1] : drake_data::tiles[view][part];
  return Tile{t.page,t.x,t.y,t.w,t.h,t.ox,t.oy};
 },drake_data::authoredPart,drakeOrder};
inline Artwork crawler{"assets/volcano/crawler-turn/",crawler_data::PageCount,
 [](int view,int part,int){return crawler_data::tiles[view][part];},
 crawler_data::authoredPart,crawlerOrder};
inline constexpr int impOrder[8][6]={
 {0,1,2,5,3,4},{0,1,2,5,3,4},{0,1,2,4,3,5},{0,1,2,4,3,5},
 {0,1,2,4,3,5},{0,1,2,4,3,5},{0,1,2,4,3,5},{0,1,2,4,3,5}
};
inline Artwork imp{"assets/volcano/imp-turn/",imp_data::PageCount,
 [](int view,int part,int){return imp_data::tiles[view][part];},imp_data::authoredPart,impOrder};
inline Artwork sentinel{"assets/volcano/sentinel-turn/",sentinel_data::PageCount,
 [](int view,int part,int){return sentinel_data::tiles[view][part];},sentinel_data::authoredPart,obsidian_sentinel_motion::order};
inline constexpr int cinderhornOrder[8][6]={
 {0,4,3,1,5,2},{0,4,3,1,5,2},{0,1,3,4,5,2},{1,5,3,0,4,2},
 {1,5,3,0,4,2},{5,4,2,3,1,0},{4,5,2,3,0,1},{4,5,2,3,0,1}
};
inline Artwork cinderhorn{"assets/volcano/cinderhorn-turn/",cinderhorn_turn_atlas::PageCount,
 [](int view,int part,int){return cinderhorn_turn_atlas::tiles[view][part];},
 cinderhorn_turn_atlas::authoredPart,cinderhornOrder};
inline constexpr int jackalOrder[8][6]={
 {1,4,3,2,5,0},{1,4,3,2,5,0},{1,2,3,4,5,0},{2,5,3,1,4,0},
 {2,5,3,1,4,0},{5,4,0,2,3,1},{4,5,0,3,1,2},{4,5,0,1,3,2}
};
inline constexpr int reaperOrder[8][6]={
 {1,4,2,3,0,5},{2,4,1,3,0,5},{2,1,3,0,4,5},{1,5,2,3,0,4},
 {2,5,1,3,0,4},{4,1,3,2,0,5},{4,1,2,3,0,5},{4,1,3,2,0,5}
};
inline Artwork jackal{"assets/desert/jackal-turn/",dune_jackal_turn_atlas::PageCount,
 [](int view,int part,int){return dune_jackal_turn_atlas::tiles[view][part];},
 dune_jackal_turn_atlas::authoredPart,jackalOrder};
inline Artwork reaper{"assets/desert/reaper-turn/",glass_reaper_turn_atlas::PageCount,
 [](int view,int part,int){return glass_reaper_turn_atlas::tiles[view][part];},
 glass_reaper_turn_atlas::authoredPart,reaperOrder};
inline constexpr float ReaperFloor=143.5f,ReaperHeight=117.f;
struct ReaperProjection {
 float floor=ReaperFloor,scale=1.f;
 std::array<float,2> soles{{ReaperFloor,ReaperFloor}};
 std::array<Vector2,2> shoulders{};
};
inline std::array<ReaperProjection,glass_reaper_turn_atlas::Count> reaperProjection{};
inline float reaperY(int frame,float y){
 const auto& shape=reaperProjection[frame];
 return .8f*(ReaperFloor+(y-shape.floor)*shape.scale);
}
static_assert(dune_jackal_turn_atlas::Count==glass_reaper_turn_atlas::Count);
static_assert(dune_jackal_turn_atlas::Count%8==0);
inline bool singleDirectionalPaint(const Artwork& art){return &art==&jackal||&art==&reaper;}
inline Shader program{};
inline bool filtered(){return program.id && program.id!=rlGetShaderIdDefault();}
inline bool continuousImp(){return impCels.color.id&&impContour.ready()&&filtered();}
inline const Artwork* forType(int type){
 if(type==SlagCrawlerType && crawler.ready())return &crawler;
 if(type==EmberDrakeType && drake.ready())return &drake;
 if(type==FireImpType && imp.ready())return &imp;
 // Fragment sampling and picking use the same fractional direction shader.
 if(type==ObsidianSentinelType && sentinel.ready() && filtered())return &sentinel;
 return nullptr;
}
inline float mix(float a,float b,float t){return a+t*(b-a);}
inline Part interpolate(const Part& a,const Part& b,float t){
 const auto at=[t](float start,float end){return mix(start,end,t);};
 return {{at(a.pivot.x,b.pivot.x),at(a.pivot.y,b.pivot.y)},
         {at(a.shift.x,b.shift.x),at(a.shift.y,b.shift.y)},at(a.angle,b.angle),
         at(a.scaleX,b.scaleX),at(a.scaleY,b.scaleY),at(a.elbow,b.elbow),
         {at(a.kneeShift.x,b.kneeShift.x),at(a.kneeShift.y,b.kneeShift.y)},
         {at(a.footShift.x,b.footShift.x),at(a.footShift.y,b.footShift.y)},
         at(a.footPitch,b.footPitch),at(a.footWeight,b.footWeight),
         at(a.overhead,b.overhead),at(a.wristCounterturn,b.wristCounterturn),
         at(a.restingFootTurn,b.restingFootTurn)};
}
struct View {
 int base=0,sample=0;
 float t=0,mix=0;
 int count=32;
 int nextBase()const{return (base+1)%8;}
 int nextSample()const{return (sample+1)%count;}
 int nearestSample()const{return mix<.5f?sample:nextSample();}
 int nearestBase()const{return t<.5f?base:nextBase();}
};
inline float heading(View view){
 return mix(facingAngles[view.base],view.base==7?2*PI:facingAngles[view.nextBase()],view.t);
}
inline bool rigidShell(const Artwork& art,int part){return &art==&crawler&&(art.signedEdges||art.directCels())&&part==3;}
inline Rectangle shellBounds(View view){
 const auto direction=actorFacing(heading(view));
 // A single isometric projection of the same shell as the body turns.
 const float width=std::hypot(102.f*direction.x,68.f*direction.y);
 const float depth=62+12*direction.y*direction.y;
 return {64-.4f*width,72*.8f,.8f*width,.8f*depth};
}
inline float drakeHeightScale(View view){
 if(!drake.signedEdges&&!drake.directCels())return 1;
 const auto& height=drake.standingHeight;
 const int i=view.sample;
 const float prev=height[(i+31)%32],start=height[i],end=height[(i+1)%32],next=height[(i+2)%32];
 const float t=view.mix;
 const float observed=start+.5f*t*(end-prev+t*(2*prev-5*start+4*end-next+t*(3*(start-end)+next-prev)));
 return 98.f/std::max(1.f,observed);
}
static_assert(drake_direction_cels::Count==crawler_direction_cels::Count);
struct DirectionCels {
 static constexpr int Count=drake_direction_cels::Count;
 int first,next;
 float fraction;
 explicit DirectionCels(View view){
  const float position=(view.sample+view.mix)*(Count/32);
  const int index=int(position);
  first=index%Count;next=(index+1)%Count;fraction=position-index;
 }
 int orderCel()const{return fraction<.5f?first:next;}
};
inline Tile celTile(const Artwork& art,int cel,int part,int jaw){
 if(&art==&crawler)return crawler_direction_cels::tiles[cel][part];
 return drake_direction_cels::tiles[cel][part==5&&jaw>0?5+jaw:part];
}
struct ImpCelView {
 int first,next,nearest;
 float t;
 explicit ImpCelView(View angle){
  const float position=(angle.base+angle.t)*2;
  const int node=int(position);
  first=node%ImpPaintedDirections;
  next=(first+1)%ImpPaintedDirections;
  t=position-node;
  nearest=t<.5f?first:next;
 }
 static const auto& cell(int d,int p){return fire_imp_cels::sheet[d*(fire_imp_cels::Steps/2)][p];}
 static Rectangle canvas(int d,int p){
  const auto& tile=cell(d,p);
  constexpr float unit=.8f/fire_imp_cels::RasterScale;
  return {unit*tile.ox,unit*tile.oy,unit*tile.w,unit*tile.h};
 }
 Rectangle bounds(int p)const{
  if(!continuousImp())return canvas(nearest,p);
  return painted_turn_shape::interpolate(canvas(first,p),canvas(next,p),t);
 }
 static Rectangle pixels(int d,int p){
  const auto& c=cell(d,p);
  return {float(c.x),float(c.y),float(c.w),float(c.h)};
 }
 Tile tile(int p,int direction=-1)const{
  const auto& c=cell(direction<0?nearest:direction,p);
  return {0,c.x,c.y,c.w,c.h,0,0};
 }
};
inline View view(const AnimationState& state,const Artwork& art){
 const int key=std::clamp(int(state.direction),0,7);
 const bool single=singleDirectionalPaint(art);
 const int count=single?dune_jackal_turn_atlas::Count:32,steps=count/8;
 if(!art.ready()||!state.facingInitialized)return {key,key*steps,0,0,count};
 float radians=std::fmod(state.facingAngle,2*PI);
 if(radians<0)radians+=2*PI;
 if(&art==&imp&&impCels.color.id){
  if(continuousImp()){
   const float position=radians/(PI/4);
   const int sector=std::min(int(position),7);
   const float t=std::clamp(position-sector,0.f,1.f);
   const float sample=(sector+t)*4;
   return {sector,int(sample)%32,t,sample-int(sample)};
  }
  const int key=int(std::lround(radians*(ImpPaintedDirections/(2*PI))))%ImpPaintedDirections;
  return {key/2,key*2,(key%2)*.5f,0};
 }
 int base=int(std::upper_bound(facingAngles+1,facingAngles+8,radians)-facingAngles)-1;
 const float end=base==7?2*PI:facingAngles[base+1];
 const float t=std::clamp((radians-facingAngles[base])/(end-facingAngles[base]),0.f,1.f);
 const float f=t*steps;
 const int sub=std::min(steps-1,int(f));
 View result{base,steps*base+sub,t,f-sub,count};
 if(!filtered()&&!single){
  result.sample=(result.sample+int(result.mix>=.5f))%32;
  result.base=result.sample/4;result.t=(result.sample%4)*.25f;result.mix=0;
 }
 return result;
}
// The sword and bracer stay rigid beyond the elbow. Only the narrow
// articulated cuff between the upper and lower arm changes shape.
struct SwordJoint {
 Vector2 center{},axis{};
 float angle=0;
 Vector2 map(Vector2 point,bool inverse)const{
  if(angle==0)return point;
  const Vector2 v{point.x-center.x,point.y-center.y};
  const float bearing=std::atan2(axis.x*v.y-axis.y*v.x,axis.x*v.x+axis.y*v.y);
  constexpr float inner=.81f,outer=2.28f;
  const float weight=inverse?(bearing>angle+inner?(outer-bearing)/(outer-inner-angle):
                              bearing<angle-inner?(outer+bearing)/(outer-inner+angle):1.f):
                             (outer-std::abs(bearing))/(outer-inner);
  const float rot=(inverse?-angle:angle)*std::clamp(weight,0.f,1.f);
  const float c=std::cos(rot),s=std::sin(rot);
  return {center.x+c*v.x-s*v.y,center.y+s*v.x+c*v.y};
 }
};
inline SwordJoint swordJoint(View view,float angle){
 using namespace obsidian_sentinel_motion;
 Vector2 elbow{mix(elbows[view.base][0],elbows[view.nextBase()][0],view.t),
               mix(elbows[view.base][1],elbows[view.nextBase()][1],view.t)};
 Vector2 axis{mix(bladeGlow[view.base][0],bladeGlow[view.nextBase()][0],view.t)-elbow.x,
              mix(bladeGlow[view.base][1],bladeGlow[view.nextBase()][1],view.t)-elbow.y};
 const float norm=std::hypot(axis.x,axis.y);
 return {elbow,{axis.x/norm,axis.y/norm},angle*DEG2RAD};
}
inline SwordJoint impElbow(View view,int part,float angle){
 // Elbow centers traced on the eight 160-pixel paintings, in authored arm order.
 static constexpr Vector2 elbows[8][2]={
  {{70,69},{97,82}},{{46,72},{101,79}},{{46,75},{109,73}},{{59,79},{114,72}},
  {{63,82},{90,69}},{{39,74},{88,69}},{{45,72},{102,72}},{{72,69},{121,74}}
 };
 const auto a=elbows[view.base][part-4],b=elbows[view.nextBase()][part-4];
 const Vector2 center{.8f*mix(a.x,b.x,view.t),.8f*mix(a.y,b.y,view.t)};
 const auto& palm=part==4?fire_imp_rig::guardPalm:fire_imp_rig::palm;
 Vector2 axis{mix(palm[view.base][0],palm[view.nextBase()][0],view.t)-center.x,
              mix(palm[view.base][1],palm[view.nextBase()][1],view.t)-center.y};
 const float length=std::hypot(axis.x,axis.y);
 return {center,{axis.x/length,axis.y/length},angle*DEG2RAD};
}
template<std::size_t N>
inline Vector2 reaperAnchor(View view,const short (&points)[8][N][2],int point){
 constexpr int Steps=glass_reaper_turn_atlas::Count/8;
 const auto at=[&](int frame){
  const int from=frame/Steps;
  const float t=float(frame%Steps)/Steps;
  const auto& a=points[from][point];
  const auto& b=points[(from+1)%8][point];
  return Vector2{.8f*mix(a[0],b[0],t),reaperY(frame,mix(a[1],b[1],t))};
 };
 const auto a=at(view.sample),b=at(view.nextSample());
 return {mix(a.x,b.x,view.mix),mix(a.y,b.y,view.mix)};
}
inline Vector2 reaperJoint(View view,int joint){
 const auto a=glass_reaper_turn_atlas::armJoints[view.sample][joint];
 const auto b=glass_reaper_turn_atlas::armJoints[view.nextSample()][joint];
 return {.8f*mix(a[0],b[0],view.mix),mix(reaperY(view.sample,a[1]),reaperY(view.nextSample(),b[1]),view.mix)};
}
inline Vector2 reaperShoulder(View view,int part){
 const auto a=reaperProjection[view.sample].shoulders[part-4];
 const auto b=reaperProjection[view.nextSample()].shoulders[part-4];
 return {mix(a.x,b.x,view.mix),mix(a.y,b.y,view.mix)};
}
inline SwordJoint reaperElbow(View view,int part,float angle){
 const int row=2*(part-4);
 const auto center=reaperJoint(view,row),palm=reaperJoint(view,row+1);
 const float dx=palm.x-center.x,dy=palm.y-center.y;
 const float length=std::max(1.f,std::hypot(dx,dy));
 return {center,{dx/length,dy/length},angle*DEG2RAD};
}
struct Locations {
 int uv0,uv1,axisU,axisV,guide0,guide1,order0,order1,lighting,phase,page1,material,flow,flowView;
 int jointBox,jointAxis,impNode,edgeJaw,shellAngle,solidLayerOrder,legU,legV,legSplit,reaperCoat,reaperShoulders;
 int heldInk,heldU,heldV,heldActive,forearmU,forearmV,forearmSplit;
};
inline Locations loc{};
inline void unloadCelPages(Artwork& art){
 for(auto& page:art.cels){
  if(page.color.id)UnloadTexture(page.color);
  page={};
 }
}
inline void unloadPages(Artwork& art){
 for(auto& p:art.pages){if(p.color.id)UnloadTexture(p.color);if(p.mask.id)UnloadTexture(p.mask);p={};}
 if(art.flow.id)UnloadTexture(art.flow);
 art.flow={};art.vectors.clear();art.signedEdges=false;
 art.contactContours.clear();
 unloadCelPages(art);
}
inline void unload(){
 if(impCels.color.id)UnloadTexture(impCels.color);
 if(impCels.mask.id)UnloadTexture(impCels.mask);
 impContour.unload();
 impCels={};
 unloadPages(drake);unloadPages(crawler);unloadPages(imp);unloadPages(sentinel);unloadPages(cinderhorn);
 unloadPages(jackal);unloadPages(reaper);
 if(filtered())UnloadShader(program);
 program={};
}
inline void loadProgram(){
 if(program.id)return;
 // Resolve the projected parts and their depth before compositing over the ground.
 const auto fragment=std::string("#version 330\n")+painted_turn_shape::shaderCode+R"GLSL(
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;
uniform sampler2D texture0, page1, material, turnFlow;
uniform int flowView,impNode,edgeJaw;
uniform float shellAngle;
uniform vec4 uv0[6], uv1[6], guide0[6], guide1[6];
uniform vec3 axisU[6], axisV[6], lighting[6];
uniform int order0[6],order1[6];
uniform vec3 phase;
uniform vec4 jointBox[6];
uniform vec3 jointAxis[6];
uniform vec3 legU[4],legV[4],legSplit[4];
uniform vec3 reaperCoat;
uniform vec4 reaperShoulders[2];
uniform sampler2D heldInk;
uniform vec3 heldU,heldV;
uniform int heldActive;
uniform vec3 forearmU,forearmV,forearmSplit;
uniform int solidLayerOrder;
vec2 unbend(vec2 uv,vec4 box,vec3 axis){
    vec2 q=(uv-box.xy)*box.zw;
    if(dot(q,q)<.000001)return uv;
    float v=atan(axis.x*q.y-axis.y*q.x,dot(axis.xy,q));
    const float inner=.81,outer=2.28;
    float bend=axis.z;
    float w=v>bend+inner?(outer-v)/(outer-inner-bend):
            v<bend-inner?(outer+v)/(outer-inner+bend):1.;
    float a=-bend*clamp(w,0.,1.),c=cos(a),s=sin(a);
    return box.xy+vec2(c*q.x-s*q.y,s*q.x+c*q.y)/box.zw;
}
vec4 pigment(vec4 tile,vec2 uv) {
    if(any(lessThan(uv,vec2(0))) || any(greaterThan(uv,vec2(1))))return vec4(0.0);
    vec2 coords=vec2(fract(tile.x),tile.y)+uv*tile.zw;
    if(impNode>=0 || impNode==-5){
        vec2 dim=vec2(tile.x>=1.0?textureSize(page1,0):textureSize(texture0,0));
        vec2 px=coords*dim-.5;
        coords=(floor(px)+smoothstep(vec2(.13),vec2(.87),fract(px))+.5)/dim;
    }
    return tile.x>=1.0?texture(page1,coords):texture(texture0,coords);
}
vec4 reaperPigment(vec4 tile,vec2 uv){
    vec4 soft=pigment(tile,uv);
    if(soft.a<.78)return soft;
    vec2 dims=vec2(textureSize(texture0,0));
    vec2 px=(tile.xy+uv*tile.zw)*dims-.5;
    vec2 crisp=(floor(px)+smoothstep(vec2(.07),vec2(.93),fract(px))+.5)/dims;
    vec4 detail=texture(texture0,crisp);
    float weight=.90*smoothstep(.78,.98,min(soft.a,detail.a));
    soft.rgb=mix(soft.rgb,detail.rgb*soft.a/max(.001,detail.a),weight);
    return soft;
}
vec4 directPaint(int part,vec2 at,float t){
    vec4 a=pigment(uv0[part],at),b=pigment(uv1[part],at);
    float opacity=mix(a.a,b.a,t);
    if(opacity<.0039)return vec4(0);
    vec3 ca=a.rgb/max(a.a,.0039),cb=b.rgb/max(b.a,.0039);
    float share=t*b.a/opacity;
    // Nearby cels agree on broad tone. Keep a narrow transition at contrast
    // edges so eyes, teeth and plate ridges do not trail a second image.
    float detail=smoothstep(.13,.42,length(ca-cb));
    float pigmentMix=mix(share,smoothstep(.30,.70,share),detail);
    float solid=smoothstep(.12,.88,opacity);
    return vec4(mix(ca,cb,pigmentMix)*solid,solid);
}
vec2 displacement(vec2 uv,int part){
    vec2 cell=clamp(uv,vec2(.5/32.),vec2(1.-.5/32.));
    if(part==5 && edgeJaw>1)part+=edgeJaw-1;
    vec2 sampleAt=(vec2(float(flowView),float(part))+cell)*32./vec2(textureSize(turnFlow,0));
    return (texture(turnFlow,sampleAt).rg*255.-128.)/384.;
}
vec4 registeredTexel(vec4 tile,vec2 uv){
    vec2 dim=vec2(textureSize(texture0,0));
    vec2 top=tile.xy*dim,size=tile.zw*dim;
    vec2 at=top+uv*size;
    vec2 inside=clamp(at,top+.5,top+size-.5);
    vec4 paint=texture(texture0,inside/dim);
    paint.a=(255.*paint.a-128.)/12.-length(at-inside);
    if(impNode==-6 && paint.a>.65){
        vec2 pos=inside-.5;
        vec2 crisp=floor(pos)+smoothstep(vec2(.07),vec2(.93),fract(pos))+.5;
        paint.rgb=mix(paint.rgb,texture(texture0,crisp/dim).rgb,.9*smoothstep(.65,1.6,paint.a));
    }
    return paint;
}
vec4 registeredPaint(int part,vec2 uv,vec2 a,vec2 b,float turn){
    vec4 one=registeredTexel(uv0[part],a),two=registeredTexel(uv1[part],b);
    float coverage=clamp(.5+mix(one.a,two.a,turn),0.,1.);
    if(impNode==-6){
        // These samples follow the same transported paint. Prefer a valid
        // surface when the changing contour exposes an edge.
        float left=(1.-turn)*clamp(1.+one.a,.04,1.);
        float right=turn*clamp(1.+two.a,.04,1.);
        return vec4(mix(one.rgb,two.rgb,right/(left+right))*coverage,coverage);
    }
    // The edge follows the registered shape. One painting supplies the brush
    // detail at a time, including when the head passes in front of a wing.
    float seam=clamp(.50+.36*(uv.y-.5)+.24*(uv.x-.5),.12,.88);
    bool next=turn>=seam;
    if(one.a<-.30&&two.a>.18)next=true;
    if(two.a<-.30&&one.a>.18)next=false;
    return vec4((next?two.rgb:one.rgb)*coverage,coverage);
}
vec4 impPaint(vec2 at,int part){
    // Only coordinates move between the four guide intervals. Each point
    // reads one original brush mark, including its original edge coverage.
    vec2 cell=clamp(at,vec2(.5/48.),vec2(1.-.5/48.));
    vec2 pos=(vec2(float(impNode),float(part))+cell)/vec2(40.,6.);
    vec4 offsets=(mix(texture(turnFlow,pos),texture(turnFlow,pos+vec2(1./40.,0.)),phase.x)*255.-128.)/255.;
    float seam=.50+.34*(at.x-.5)+.27*(at.y-.5)+.013*(float(part)-2.5);
    vec4 paint=phase.y<seam?pigment(uv0[part],at+offsets.rg):pigment(uv1[part],at+offsets.ba);
    if(paint.a>0.)paint*=smoothstep(.08,.92,paint.a)/paint.a;
    return paint;
}
vec4 overLayer(vec4 top,vec4 below){
    return top+below*(1.-top.a);
}
vec4 reaperLayers(vec4 layers[6],int drawOrder[6],float coat,float shoulder){
    vec4 normal=vec4(0),seated=vec4(0),coatNormal=vec4(0),coatSeated=vec4(0);
    int nearLeg=1;
    for(int i=0;i<6;i++){
        int part=drawOrder[i];
        if(part==1||part==2)nearLeg=part;
    }
    vec4 legs=overLayer(layers[nearLeg],layers[3-nearLeg]);
    for(int i=0;i<6;i++){
        int part=drawOrder[i];
        vec4 layer=layers[part];
        normal=overLayer(layer,normal);
        if(part==3)seated=overLayer(layers[5],seated);
        if(part!=5)seated=overLayer(layer,seated);
        if(coat<=0.||part==1||part==2)continue;
        coatNormal=overLayer(layer,coatNormal);
        if(part==3)coatSeated=overLayer(layers[5],coatSeated);
        if(part!=5)coatSeated=overLayer(layer,coatSeated);
        if(part==3){
            coatNormal=overLayer(legs,coatNormal);
            coatSeated=overLayer(legs,coatSeated);
        }
    }
    return mix(mix(normal,seated,shoulder),mix(coatNormal,coatSeated,shoulder),coat);
}
void main() {
    // Detached pieces address one painted layer in its original coordinates.
    // A whole actor uses the regular untagged six-layer composition.
    int group=int(floor((fragTexCoord.x+256.0)/2048.0));
    float x=fragTexCoord.x-float(group)*2048.0;
    bool shadow=x>800.0,mask=x>300.0;
    vec3 point=vec3(x-(shadow?1024.0:mask?512.0:0.0),fragTexCoord.y,1.0);
    vec4 layers[6];
    float coat=0.;
    for(int i=0;i<6;i++){
        layers[i]=vec4(0.0);
        if(group>0 && i!=group-1)continue;
        vec3 samplePoint=point;
        if(i>=4){
            vec4 shoulder=reaperShoulders[i-4];
            if(shoulder.z!=0.){
                vec2 d=point.xy-shoulder.xy;
                float a=-shoulder.z*smoothstep(shoulder.w,shoulder.w+8.5,length(d));
                float c=cos(a),s=sin(a);
                samplePoint.xy=shoulder.xy+vec2(c*d.x-s*d.y,s*d.x+c*d.y);
            }
        }
        vec2 at=vec2(dot(axisU[i],samplePoint),dot(axisV[i],samplePoint));
        if(i==5&&dot(forearmSplit,point)>0.)
            at=vec2(dot(forearmU,point),dot(forearmV,point));
        if(i==1||i==2){
            int shin=2*(i-1),boot=shin+1;
            if(dot(legSplit[boot],point)>0.)
                at=vec2(dot(legU[boot],point),dot(legV[boot],point));
            else if(dot(legSplit[shin],point)>0.)
                at=vec2(dot(legU[shin],point),dot(legV[shin],point));
        }
        if(jointAxis[i].z!=0.)at=unbend(at,jointBox[i],jointAxis[i]);
        if(any(lessThan(at,vec2(0))) || any(greaterThan(at,vec2(1))))continue;
        if(i==3&&!mask&&!shadow)
            coat=reaperCoat.y*smoothstep(reaperCoat.x-.025,reaperCoat.x+.025,at.y);
        vec2 a=at,b=at;
        bool shell=i==3&&shellAngle>=0.;
        if(!shell && impNode!=-4 && flowView>=0 && phase.x>0. && phase.x<1.){
            // Follow the same painted feature from both views. Blending at a
            // fixed UV leaves two sword edges, horns or visors in one frame.
            vec2 d=displacement(at,i);
            d=displacement(at+(.5-phase.x)*d,i);
            a-=phase.x*d;b+=(1.-phase.x)*d;
        }
        vec4 paint;
        if(edgeJaw>0)paint=registeredPaint(i,at,a,b,shell?shellAngle:phase.x);
        else if(impNode==-4){
            vec2 dim=vec2(textureSize(texture0,0));
            vec4 size=dim.xyxy;
            paint=paintedShape(texture0,turnFlow,flowView,i,uv0[i]*size,uv1[i]*size,phase.x,at);
            paint.rgb*=paint.a;
        }
        else if(impNode==-5)paint=directPaint(i,at,phase.x);
        else if(impNode==-6)paint=mask||shadow?pigment(uv0[i],at):reaperPigment(uv0[i],at);
        else if(impNode==-3)paint=pigment(uv0[i],at);
        else if(impNode>=0)paint=impPaint(at,i);
        else if(impNode==-2)paint=phase.x<.5?pigment(uv0[i],a):pigment(uv1[i],b);
        else paint=mix(pigment(uv0[i],a),pigment(uv1[i],b),phase.x);
        if(edgeJaw==0 && impNode!=-4 && flowView>=0 && paint.a>.001){
            float k=4.*phase.x*(1.-phase.x);
            float opacity=mix(paint.a,smoothstep(.10,.90,paint.a),k);
            paint*=opacity/paint.a;
        }
        if(!mask && phase.z>0.0 && paint.a>0.0039){
            vec2 ga=guide0[i].xy+at*guide0[i].zw,gb=guide1[i].xy+at*guide1[i].zw;
            vec4 guide=mix(texture(material,ga),texture(material,gb),phase.y);
            // Neighboring screen pixels may belong to a different layer or
            // fall outside the cutout. Read the painted relief itself to keep
            // the ground pose deterministic at those silhouettes.
            float gx=mix(textureOffset(material,ga,ivec2(1,0)).a,textureOffset(material,gb,ivec2(1,0)).a,phase.y);
            float gy=mix(textureOffset(material,ga,ivec2(0,1)).a,textureOffset(material,gb,ivec2(0,1)).a,phase.y);
            vec3 normal=normalize(vec3(-19.0*(gx-guide.a),19.0*(gy-guide.a),1));
            float crown=smoothstep(.12,.91,guide.a);
            float diffuse=max(0.0,dot(normal,normalize(vec3(-.52,.71,.62))));
            vec3 cooled=clamp(paint.rgb/paint.a+guide.rgb-vec3(128./255.),0.,1.)*lighting[i];
            cooled=cooled*(.80+.12*crown+.25*diffuse)+vec3(.023,.025,.028)*crown*
                   pow(max(0.,dot(normal,normalize(vec3(-.22,.55,.81)))),5.);
            paint.rgb=mix(paint.rgb,cooled*paint.a,phase.z);
        }
        layers[i]=paint;
    }
    vec4 left=vec4(0),right=vec4(0);
    vec4 mixed;
    if(group>0)mixed=layers[group-1];
    else if(impNode==-6&&solidLayerOrder==0){
        if(heldActive!=0){
            vec2 grip=vec2(dot(heldU,point),dot(heldV,point));
            if(all(greaterThanEqual(grip,vec2(0)))&&all(lessThanEqual(grip,vec2(1)))){
                vec4 held=texture(heldInk,grip);
                held.rgb*=held.a;
                layers[5]=overLayer(layers[5],held);
            }
        }
        // The pauldron remains under the cuirass while the forearm can
        // pass in front. The moving seam resolves occlusion on one silhouette.
        vec4 socket=reaperShoulders[1];
        float shoulder=1.-smoothstep(socket.w+1.,socket.w+2.,length(point.xy-socket.xy));
        float seam=clamp(.5+point.x/90.,.22,.78);
        float reveal=smoothstep(seam-.009,seam+.009,phase.y);
        if(reveal<=0.)mixed=reaperLayers(layers,order0,coat,shoulder);
        else if(reveal>=1.)mixed=reaperLayers(layers,order1,coat,shoulder);
        else mixed=mix(reaperLayers(layers,order0,coat,shoulder),reaperLayers(layers,order1,coat,shoulder),reveal);
    }
    else{
        for(int i=0;i<6;i++){
            vec4 a=layers[order0[i]];
            left=a+left*(1.-a.a);
            if(impNode!=-5 && solidLayerOrder==0){
                vec4 b=layers[order1[i]];
                right=b+right*(1.-b.a);
            }
        }
        // An opaque shield passes in front of the torso along its edge.
        // A dissolve between depth orders would show the cuirass through it.
        float orderMix=phase.y;
        if(edgeJaw>0||impNode!=-1)orderMix=step(clamp(.5+point.x/112.,.24,.76),phase.y);
        else if(flowView>=0){
            float crossAt=clamp(.5+point.x/112.,.24,.76);
            orderMix=smoothstep(crossAt-.045,crossAt+.045,phase.y);
        }
        mixed=impNode==-5||solidLayerOrder!=0?left:mix(left,right,orderMix);
        if(coat>0.){
            // The belt covers the hip sockets. The lower cloth goes behind
            // the legs, so it cannot hide a moving shin or boot.
            vec4 behind=vec4(0);
            for(int i=0;i<6;i++){
                int part=order0[i];
                if(part==1||part==2)continue;
                vec4 layer=layers[part];
                behind=layer+behind*(1.-layer.a);
                if(part==3){
                    int nearLeg=int(reaperCoat.z);
                    vec4 farPaint=layers[3-nearLeg],nearPaint=layers[nearLeg];
                    behind=farPaint+behind*(1.-farPaint.a);
                    behind=nearPaint+behind*(1.-nearPaint.a);
                }
            }
            mixed=mix(mixed,behind,coat);
        }
    }
    if(shadow){
        if(mixed.a<.62)discard;
        finalColor=vec4(fragColor.a);
        return;
    }
    if(mixed.a<.0039)discard;
    finalColor=vec4(mask?fragColor.rgb:mixed.rgb/mixed.a,mixed.a*fragColor.a);
})GLSL";
 program=LoadShaderFromMemory(nullptr,fragment.c_str());
 if(filtered()){
  auto location=[](const char* name){return GetShaderLocation(program,name);};
  loc={location("uv0[0]"),location("uv1[0]"),location("axisU[0]"),location("axisV[0]"),
       location("guide0[0]"),location("guide1[0]"),location("order0[0]"),location("order1[0]"),
       location("lighting[0]"),location("phase"),location("page1"),location("material"),
       location("turnFlow"),location("flowView"),location("jointBox[0]"),location("jointAxis[0]"),location("impNode"),location("edgeJaw"),location("shellAngle"),location("solidLayerOrder"),
       location("legU[0]"),location("legV[0]"),location("legSplit[0]"),location("reaperCoat"),location("reaperShoulders[0]"),
       location("heldInk"),location("heldU"),location("heldV"),location("heldActive"),
       location("forearmU"),location("forearmV"),location("forearmSplit")};
 }
}
inline void calibrateRegisteredViews(Artwork& art){
 const auto& page=art.pages[0];
 const auto alpha=[&](const Tile& tile,int x,int y){return page.alpha[(tile.y+y)*page.color.width+tile.x+x];};
 if(&art==&crawler){
  for(int key=0;key<8;key++){
   const auto tile=art.tile(key*4,3,0);
   int left=tile.w,top=tile.h,right=0,bottom=0;
   for(int y=0;y<tile.h;y++)for(int x=0;x<tile.w;x++)if(alpha(tile,x,y)>=128){
    left=std::min(left,x);top=std::min(top,y);right=std::max(right,x+1);bottom=std::max(bottom,y+1);
   }
   auto fitted=tile;
   if(right>left&&bottom>top){
    left=std::max(0,left-1);top=std::max(0,top-1);
    right=std::min(int(tile.w),right+1);bottom=std::min(int(tile.h),bottom+1);
    fitted.x+=left;fitted.y+=top;fitted.w=right-left;fitted.h=bottom-top;
   }
   art.shell[key]=fitted;
  }
  return;
 }
 for(int view=0;view<32;view++){
  constexpr int extent=160;
  std::array<bool,extent*extent> opaque{};
  for(int p=0;p<6;p++){
   const auto tile=art.tile(view,p,0);
   for(int y=0;y<tile.h;y++)for(int x=0;x<tile.w;x++)
    if(alpha(tile,x,y)>180)opaque[(tile.oy+y)*extent+tile.ox+x]=true;
  }
  std::array<int,extent> rows{};
  for(size_t i=0;i<opaque.size();i++)rows[i/extent]+=opaque[i];
  const int fringe=int(std::count(opaque.begin(),opaque.end(),true)*.012f);
  // Small horn and claw antialiasing pixels cannot move the measure.
  int first=0,last=extent-1,above=0,below=0;
  while(first<last&&(above+=rows[first])<=fringe)++first;
  while(last>first&&(below+=rows[last])<=fringe)--last;
  art.standingHeight[view]=float(last-first);
 }
}
inline void loadRegisteredPaint(Artwork& art){
 const std::string folder=art.folder;
 Image ink=LoadImage((folder+"turn-ink.png").c_str()),map=LoadImage((folder+"turn-flow.png").c_str());
 auto& page=art.pages[0];
 const int rows=&art==&drake?14:6;
 const int directions=&art==&reaper?glass_reaper_turn_atlas::Count:32;
 if(ink.data&&map.data&&ink.width==page.color.width&&ink.height==page.color.height&&
    map.width==directions*32&&map.height==rows*32){
  ImageFormat(&ink,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  ImageFormat(&map,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  const auto paint=LoadTextureFromImage(ink),flow=LoadTextureFromImage(map);
  if(paint.id&&flow.id){
   if(&art==&drake||&art==&crawler)calibrateRegisteredViews(art);
   UnloadTexture(page.color);page.color=paint;art.flow=flow;art.signedEdges=true;
   SetTextureFilter(paint,TEXTURE_FILTER_BILINEAR);
   SetTextureFilter(flow,TEXTURE_FILTER_BILINEAR);
   const auto* pigment=static_cast<const Color*>(ink.data);
   for(size_t i=0;i<page.alpha.size();++i)page.alpha[i]=pigment[i].a;
   const auto* vectors=static_cast<const Color*>(map.data);
   art.vectors.assign(vectors,vectors+map.width*map.height);
  }else{
   if(paint.id)UnloadTexture(paint);
   if(flow.id)UnloadTexture(flow);
  }
 }
 if(ink.data)UnloadImage(ink);
 if(map.data)UnloadImage(map);
}
inline bool loadDirectionalCels(Artwork& art){
 const int count=&art==&drake?drake_direction_cels::PageCount:crawler_direction_cels::PageCount;
 for(int i=0;i<count;i++){
  const std::string path=std::string(art.folder)+"turn-cels-"+std::to_string(i)+".png";
  if(!FileExists(path.c_str())){unloadCelPages(art);return false;}
  Image image=LoadImage(path.c_str());
  if(!image.data){unloadCelPages(art);return false;}
  ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  auto& page=art.cels[i];
  const auto* pixels=static_cast<const Color*>(image.data);
  page.alpha.resize(size_t(image.width)*image.height);
  for(size_t n=0;n<page.alpha.size();n++)page.alpha[n]=pixels[n].a;
  ImageAlphaPremultiply(&image);
  page.color=LoadTextureFromImage(image);
  UnloadImage(image);
  if(!page.color.id){unloadCelPages(art);return false;}
  SetTextureFilter(page.color,TEXTURE_FILTER_BILINEAR);
 }
 if(&art==&drake)calibrateRegisteredViews(art);
 return true;
}
inline void loadPages(Artwork& art){
 if(art.ready())return;
 for(int i=0;i<art.pageCount;i++){
  const auto path=std::string(art.folder)+"parts-"+std::to_string(i)+".png";
  Image image=LoadImage(path.c_str());
  if(!image.data){unloadPages(art);return;}
  ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  auto& page=art.pages[i];page.alpha.resize(image.width*image.height);
  auto* pixels=static_cast<Color*>(image.data);
  for(size_t n=0;n<page.alpha.size();n++)page.alpha[n]=pixels[n].a;
  if(filtered())ImageAlphaPremultiply(&image);
  page.color=LoadTextureFromImage(image);
  if(!filtered()){
   for(size_t n=0;n<page.alpha.size();n++)pixels[n]={255,255,255,page.alpha[n]};
   page.mask=LoadTextureFromImage(image);
   SetTextureFilter(page.mask,TEXTURE_FILTER_BILINEAR);
  }
  UnloadImage(image);
  if(!page.color.id){unloadPages(art);return;}
  SetTextureFilter(page.color,TEXTURE_FILTER_BILINEAR);
 }
 if(filtered()&&(&art==&drake||&art==&crawler)&&!loadDirectionalCels(art))loadRegisteredPaint(art);
 if(filtered()&& &art==&cinderhorn)loadRegisteredPaint(art);
 if(filtered() && (&art==&imp || &art==&sentinel)){
  const std::string path=std::string(art.folder)+"turn-flow.png";
  Image directions=LoadImage(path.c_str());
  const int nodes=&art==&imp?40:32,grid=&art==&imp?48:32;
  if(directions.data && directions.width==nodes*grid && directions.height==6*grid){
   ImageFormat(&directions,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
   const auto* first=static_cast<const Color*>(directions.data);
   art.vectors.assign(first,first+directions.width*directions.height);
   art.flow=LoadTextureFromImage(directions);
   SetTextureFilter(art.flow,TEXTURE_FILTER_BILINEAR);
  }
  if(directions.data)UnloadImage(directions);
 }
}
inline void loadImpCels(){
 if(impCels.color.id)return;
 Image image=LoadImage("assets/volcano/imp-turn/directional-cels.png");
 if(!image.data)return;
 ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
 auto* pixels=static_cast<Color*>(image.data);
 impCels.alpha.resize(image.width*image.height);
 for(size_t i=0;i<impCels.alpha.size();i++)impCels.alpha[i]=pixels[i].a;
 if(filtered())ImageAlphaPremultiply(&image);
 impCels.color=LoadTextureFromImage(image);
 if(!filtered()){
  for(size_t i=0;i<impCels.alpha.size();i++)pixels[i]={255,255,255,impCels.alpha[i]};
  impCels.mask=LoadTextureFromImage(image);SetTextureFilter(impCels.mask,TEXTURE_FILTER_BILINEAR);
 }
 UnloadImage(image);
 SetTextureFilter(impCels.color,TEXTURE_FILTER_BILINEAR);
 if(filtered())impContour.load("assets/volcano/imp-turn/shape",impCels.color.width,impCels.color.height);
}
inline void loadContactContours(Artwork& art){
 if(!art.ready()||!art.contactContours.empty())return;
 constexpr int count=dune_jackal_turn_atlas::Count;
 art.contactContours.resize(count*6);
 for(int view=0;view<count;view++)for(int p=0;p<6;p++){
  const auto tile=art.tile(view,p,0);
  const auto& page=art.pages[tile.page];
  auto& contour=art.contactContours[view*6+p];
  for(int row=0;row<12;row++){
   int left=tile.w,right=-1,loY=tile.h,hiY=-1;
   for(int y=row*tile.h/12;y<(row+1)*tile.h/12;y++){
    const auto* alpha=page.alpha.data()+(tile.y+y)*page.color.width+tile.x;
    for(int x=0;x<tile.w;x++)if(alpha[x]>=150){
     left=std::min(left,x);right=std::max(right,x);
     loY=std::min(loY,y);hiY=std::max(hiY,y);
    }
   }
   const float px=right>=0?float(left+.5f)/tile.w:.5f;
   const float qx=right>=0?float(right+.5f)/tile.w:.5f;
   const float py=right>=0?float(loY+hiY+1)*.5f/tile.h:.5f;
   contour[row*2]={px,py};
   contour[row*2+1]={qx,py};
  }
 }
}
inline void calibrateReaper(){
 if(!reaper.ready())return;
 for(int frame=0;frame<glass_reaper_turn_atlas::Count;frame++){
  const auto edge=[&](int part,bool top){
   const auto tile=reaper.tile(frame,part,0);
   const auto& page=reaper.pages[tile.page];
   for(int i=0;i<tile.h;i++){
    const int y=top?i:tile.h-1-i;
    const auto* row=page.alpha.data()+(tile.y+y)*page.color.width+tile.x;
    if(std::any_of(row,row+tile.w,[](unsigned char value){return value>=125;}))
     return tile.oy+y+.5f;
   }
   return tile.oy+tile.h*.5f;
  };
  auto& projection=reaperProjection[frame];
  projection.soles={edge(1,false),edge(2,false)};
  projection.floor=std::max(projection.soles[0],projection.soles[1]);
  projection.scale=ReaperHeight/std::max(1.f,projection.floor-edge(0,true));
  const auto chest=reaper.tile(frame,3,0);
  const auto alpha=[&](Tile tile,int x,int y){
   x-=tile.ox;y-=tile.oy;
   const auto& page=reaper.pages[tile.page];
   return x>=0&&x<tile.w&&y>=0&&y<tile.h?
          int(page.alpha[(tile.y+y)*page.color.width+tile.x+x]):0;
  };
  constexpr int steps=glass_reaper_turn_atlas::Count/8;
  const auto& pivots=glass_reaper_turn_atlas::pivots;
  const int base=frame/steps;
  const float t=float(frame%steps)/steps;
  for(int p:{4,5}){
   const float x=mix(pivots[base][p][0],pivots[(base+1)%8][p][0],t);
   const float y=mix(pivots[base][p][1],pivots[(base+1)%8][p][1],t);
   const auto arm=reaper.tile(frame,p,0);
   Vector2 socket{x,y};
   float best=0;
   // Seat the joint inside the painted intersection of arm and breastplate.
   for(int row=int(y)-5;row<=int(y)+6;row++)for(int col=int(x)-8;col<=int(x)+8;col++){
    if(std::min(alpha(arm,col,row),alpha(chest,col,row))<148)continue;
    float support=0;
    for(int dy=-3;dy<=3;dy++)for(int dx=-3;dx<=3;dx++)
     support+=std::max(0,std::min(alpha(arm,col+dx,row+dy),alpha(chest,col+dx,row+dy))-118)*
              (4-std::max(std::abs(dx),std::abs(dy)));
    const float dx=col+.5f-x,dy=row+.5f-y;
    const float score=support/(1.f+.02f*(dx*dx+dy*dy));
    if(score>best){socket={col+.5f,row+.5f};best=score;}
   }
   projection.shoulders[p-4]={.8f*socket.x,reaperY(frame,socket.y)};
  }
 }
}
inline void load(){
 loadProgram();
 if(!caldera_puppets::ready(caldera_puppets::Imp)){loadPages(imp);loadImpCels();}
 if(filtered())loadPages(sentinel);
 if(!jackal_blender::ready()&&!jackal_rig::ready()&&!jackal_cels::ready()&&!jackal_puppet::ready()&&!jackal_skin::ready()){
  loadPages(jackal);
  loadContactContours(jackal);
 }
 const bool reaperLoaded=reaper.ready();
 loadPages(reaper);
 if(!reaperLoaded)calibrateReaper();
 loadContactContours(reaper);
 if(!reaperLoaded&&filtered()&&reaper.ready())loadRegisteredPaint(reaper);
}
inline Vector4 textureRect(const Artwork& art,const Tile& t){
 const auto& page=art.pages[t.page].color;
 return {float(t.page)+float(t.x)/page.width,float(t.y)/page.height,
         float(t.w)/page.width,float(t.h)/page.height};
}
inline Vector4 celRect(const Artwork& art,const Tile& t){
 const auto& page=art.cels[t.page].color;
 return {float(t.page)+float(t.x)/page.width,float(t.y)/page.height,
         float(t.w)/page.width,float(t.h)/page.height};
}
inline Vector4 guideRect(const Artwork& art,int key,int row){
 const auto t=art.tile(key*4,row,0);
 constexpr float cell=160;
 return {(key+t.ox/cell)/8.f,(art.authoredPart(key,row)+t.oy/cell)/6.f,
         t.w/(8*cell),t.h/(6*cell)};
}
inline float coverage(const Page& p,const Tile& t,float u,float v,bool sharp){
 if(u<0||v<0||u>1||v>1)return 0;
 const float x=std::clamp(t.x+u*t.w-.5f,0.f,float(p.color.width-1));
 const float y=std::clamp(t.y+v*t.h-.5f,0.f,float(p.color.height-1));
 const int ix=int(x),iy=int(y),jx=std::min(ix+1,p.color.width-1),jy=std::min(iy+1,p.color.height-1);
 const auto* a=p.alpha.data()+iy*p.color.width;
 const auto* b=p.alpha.data()+jy*p.color.width;
 const auto weight=[&](float f){
  if(!sharp)return f;
  f=std::clamp((f-.13f)/.74f,0.f,1.f);
  return f*f*(3-2*f);
 };
 const float fx=weight(x-ix),fy=weight(y-iy);
 return mix(mix(a[ix],a[jx],fx),mix(b[ix],b[jx],fx),fy)/255.f;
}
inline float coverage(const Artwork& art,const Tile& t,float u,float v){
 return coverage(art.pages[t.page],t,u,v,&art==&imp&&art.flow.id);
}
inline float signedEdge(const Artwork& art,const Tile& t,float u,float v){
 const float x=std::clamp(u,.5f/t.w,1-.5f/t.w),y=std::clamp(v,.5f/t.h,1-.5f/t.h);
 const float encoded=coverage(art.pages[t.page],t,x,y,false)*255;
 return (encoded-128)/12-std::hypot((u-x)*t.w,(v-y)*t.h);
}
inline Vector2 flowSample(const std::vector<Color>& vectors,int width,int n,int view,int part,float u,float v,bool right,float range){
 const float x=std::clamp(u*n-.5f,0.f,float(n-1)),y=std::clamp(v*n-.5f,0.f,float(n-1));
 const int ix=int(x),iy=int(y),jx=std::min(ix+1,n-1),jy=std::min(iy+1,n-1);
 const auto* top=vectors.data()+(n*part+iy)*width+n*view;
 const auto* bottom=vectors.data()+(n*part+jy)*width+n*view;
 const auto component=[&](unsigned char Color::*channel){
  const float a=mix(top[ix].*channel,top[jx].*channel,x-ix);
  const float b=mix(bottom[ix].*channel,bottom[jx].*channel,x-ix);
  return (mix(a,b,y-iy)-128)/range;
 };
 return right?Vector2{component(&Color::b),component(&Color::a)}:Vector2{component(&Color::r),component(&Color::g)};
}
inline Vector2 displacement(const Artwork& art,int view,int part,float u,float v,bool right=false){
 const bool impMap=&art==&imp;
 return flowSample(art.vectors,art.flow.width,impMap?48:32,view,part,u,v,right,impMap?255.f:384.f);
}
inline float turnCoverage(const Artwork& art,View angle,int part,int jaw,float u,float v){
 if(singleDirectionalPaint(art)&&!art.signedEdges)return coverage(art,art.tile(angle.nearestSample(),part,jaw),u,v);
 if(art.directCels()){
  const DirectionCels cells(angle);
  const auto alpha=[&](int index){
   const auto tile=celTile(art,index,part,jaw);
   return coverage(art.cels[tile.page],tile,u,v,true);
  };
  const float t=cells.fraction,opacity=mix(alpha(cells.first),alpha(cells.next),t);
  const float f=std::clamp((opacity-.12f)/.76f,0.f,1.f);
  return f*f*(3-2*f);
 }
 float ax=u,ay=v,bx=u,by=v;
 const float t=angle.mix;
 if(rigidShell(art,part)){
  const float a=signedEdge(art,art.shell[angle.base],u,v),b=signedEdge(art,art.shell[angle.nextBase()],u,v);
  return std::clamp(.5f+mix(a,b,angle.t),0.f,1.f);
 }
 if(&art==&imp){
  if(impCels.color.id){
   const ImpCelView cel(angle);
   if(continuousImp())return impContour.coverage(cel.pixels(cel.first,part),cel.pixels(cel.next,part),
                                                cel.first,part,cel.t,{u,v});
   return coverage(impCels,cel.tile(part),u,v,false);
  }
  if(!art.flow.id)return coverage(art,art.tile(t<.5f?angle.sample:angle.nextSample(),part,jaw),u,v);
  const int node=angle.base*5+angle.sample%4;
  const bool right=angle.t>=.50f+.34f*(u-.5f)+.27f*(v-.5f)+.013f*(part-2.5f);
  const auto a=displacement(art,node,part,u,v,right),b=displacement(art,node+1,part,u,v,right);
  const float alpha=coverage(art,art.tile(4*(right?angle.nextBase():angle.base),part,jaw),
                             u+mix(a.x,b.x,t),v+mix(a.y,b.y,t));
  const float f=std::clamp((alpha-.08f)/.84f,0.f,1.f);
  return f*f*(3-2*f);
 }
 if(art.flow.id && t>0 && t<1){
  const int row=art.signedEdges&&part==5?part+jaw:part;
  auto d=displacement(art,angle.sample,row,u,v);
  d=displacement(art,angle.sample,row,u+(.5f-t)*d.x,v+(.5f-t)*d.y);
  ax-=t*d.x;ay-=t*d.y;bx+=(1-t)*d.x;by+=(1-t)*d.y;
 }
 if(art.signedEdges){
  const float a=signedEdge(art,art.tile(angle.sample,part,jaw),ax,ay);
  const float b=signedEdge(art,art.tile(angle.nextSample(),part,jaw),bx,by);
  return std::clamp(.5f+mix(a,b,t),0.f,1.f);
 }
 const float a=coverage(art,art.tile(angle.sample,part,jaw),ax,ay);
 const float b=coverage(art,art.tile(angle.nextSample(),part,jaw),bx,by);
 const float opacity=mix(a,b,t);
 if(!art.flow.id)return opacity;
 const float f=std::clamp((opacity-.10f)/.8f,0.f,1.f);
 return mix(opacity,f*f*(3-2*f),4*t*(1-t));
}
inline void order(const Artwork& art,int base,int* rows,float action=-1){
 const int* layers=art.order[base];
 for(int p=0;p<6;++p)rows[p]=art.authoredPart(base,layers[p]);
 if(base<5 && &art==&sentinel && action>=.08f && action<.39f){
  // As the blade rises, keep the sword shoulder behind the helmet.
  auto torso=std::find(rows,rows+6,3),blade=std::find(rows,rows+6,5);
  if(blade>torso)std::rotate(torso,blade,blade+1);
 }
 if((base==2||base==3) && &art==&sentinel && action>=.39f && action<.92f){
  // From the front the gauntlet passes across the shield during the cut.
  auto blade=std::find(rows,rows+6,5);
  std::rotate(blade,blade+1,rows+6);
 }
}
inline Vector2 footTip(const Artwork& art,View view,int part){
 const auto tip=[&](int frame){
  const auto tile=art.tile(frame,part,0);
  const auto& contour=art.contactContours[frame*6+part];
  const float y=&art==&reaper&&part>=1&&part<=2?
       reaperY(frame,reaperProjection[frame].soles[part-1]):.8f*(tile.oy+tile.h);
  return Vector2{.8f*(tile.ox+tile.w*(contour[22].x+contour[23].x)*.5f),y};
 };
 const auto a=tip(view.sample),b=tip(view.nextSample());
 return {mix(a.x,b.x,view.mix),mix(a.y,b.y,view.mix)};
}
inline SwordJoint reaperKnee(Vector2 hip,Vector2 foot,float angle){
 const Vector2 axis{foot.x-hip.x,foot.y-hip.y};
 const float length=std::max(1.f,std::hypot(axis.x,axis.y));
 return {{hip.x+.47f*axis.x,hip.y+.47f*axis.y},{axis.x/length,axis.y/length},angle*DEG2RAD};
}
inline SwordJoint jackalKnee(View view,int part,Vector2 hip,Vector2 foot,float angle){
 const Vector2 leg{foot.x-hip.x,foot.y-hip.y};
 const float span=part>=4?.48f:.44f;
 // The foreleg's wrist and the hind leg's hock bend in opposite directions.
 // Near head-on they are foreshortened, so their lateral displacement vanishes.
 const float offset=(part>=4?-.13f:.17f)*std::cos(heading(view));
 const Vector2 center{hip.x+span*leg.x-offset*leg.y,hip.y+span*leg.y+offset*leg.x};
 const Vector2 shin{foot.x-center.x,foot.y-center.y};
 const float length=std::max(1.f,std::hypot(shin.x,shin.y));
 return {center,{shin.x/length,shin.y/length},angle*DEG2RAD};
}
inline float cross2(Vector2 a,Vector2 b){return a.x*b.y-a.y*b.x;}
inline Vector2 unit2(Vector2 v){
 const float length=std::max(.001f,std::hypot(v.x,v.y));
 return {v.x/length,v.y/length};
}

struct SickleArmProjection {
 Vector2 sourceElbow{},elbow{},sourceAcross{},across{},sourceAxis[2]{},axis[2]{};
 Vector3 split{};
 bool active=false;

 Vector2 map(Vector2 source,int section=-1)const{
  const Vector2 q{source.x-sourceElbow.x,source.y-sourceElbow.y};
  if(section<0)section=cross2(q,sourceAcross)>=0?1:0;
  const float inv=1/cross2(sourceAxis[section],sourceAcross);
  const float along=cross2(q,sourceAcross)*inv,width=cross2(sourceAxis[section],q)*inv;
  return {elbow.x+axis[section].x*along+across.x*width,elbow.y+axis[section].y*along+across.y*width};
 }
 void rows(Rectangle box,int section,Vector3& u,Vector3& v)const{
  const auto forward=axis[section],source=sourceAxis[section];
  const float inv=1/cross2(forward,across);
  const Vector2 g{across.y*inv,-across.x*inv},h{-forward.y*inv,forward.x*inv};
  const Vector2 x{source.x*g.x+sourceAcross.x*h.x,source.x*g.y+sourceAcross.x*h.y};
  const Vector2 y{source.y*g.x+sourceAcross.y*h.x,source.y*g.y+sourceAcross.y*h.y};
  u={x.x/box.width,x.y/box.width,(sourceElbow.x-box.x-x.x*elbow.x-x.y*elbow.y)/box.width};
  v={y.x/box.height,y.y/box.height,(sourceElbow.y-box.y-y.x*elbow.x-y.y*elbow.y)/box.height};
 }
};

inline SickleArmProjection projectSickleArm(View view,const Part& part){
 SickleArmProjection result{};
 const auto joint=reaperJoint(view,2),grip=reaperJoint(view,3);
 const Vector2 base{part.pivot.x+part.shift.x-64,part.pivot.y+part.shift.y-115};
 const Vector2 segments[]={{joint.x-part.pivot.x,joint.y-part.pivot.y},{grip.x-joint.x,grip.y-joint.y}};
 for(int i=0;i<2;i++){
  const auto bone=segments[i];
  const float length=std::max(.01f,std::hypot(bone.x,bone.y));
  const float angle=(part.angle+i*part.elbow)*DEG2RAD;
  const float c=std::cos(angle),s=std::sin(angle);
  result.sourceAxis[i]=unit2(bone);
  result.axis[i]={(c*bone.x*part.scaleX-s*bone.y*part.scaleY)/length,
                  (s*bone.x*part.scaleX+c*bone.y*part.scaleY)/length};
 }
 const auto right=[](Vector2 a,Vector2 b){
  a=unit2(a);b=unit2(b);
  const auto axis=unit2({a.x+b.x,a.y+b.y});
  return Vector2{-axis.y,axis.x};
 };
 result.sourceElbow=joint;
 const float span=std::hypot(segments[0].x,segments[0].y);
 result.elbow={base.x+span*result.axis[0].x,base.y+span*result.axis[0].y};
 result.sourceAcross=right(result.sourceAxis[0],result.sourceAxis[1]);
 result.across=right(result.axis[0],result.axis[1]);
 result.split={result.across.y,-result.across.x,
              -result.across.y*result.elbow.x+result.across.x*result.elbow.y};
 result.active=cross2(result.axis[0],result.across)>.10f&&cross2(result.axis[1],result.across)>.10f&&
               cross2(result.sourceAxis[0],result.sourceAcross)>.10f&&cross2(result.sourceAxis[1],result.sourceAcross)>.10f;
 return result;
}

// The thigh, shin and boot meet at shared cross-sections. Each pixel still
// samples the same painting, including through both joint boundaries.
struct LegProjection {
 Vector2 sourceKnee{},sourceAxis{},knee{},ankle{},across{},axis[3]{};
 float ankleFromKnee=0;
 Vector3 split[2]{};
 bool active=false;

 Vector2 map(Vector2 source,int section=-1)const{
  const Vector2 q{source.x-sourceKnee.x,source.y-sourceKnee.y};
  float along=q.x*sourceAxis.x+q.y*sourceAxis.y;
  const float width=cross2(sourceAxis,q);
  if(section<0)section=along<0?0:along<ankleFromKnee?1:2;
  const auto direction=axis[section];
  const auto base=section==2?ankle:knee;
  if(section==2)along-=ankleFromKnee;
  return {base.x+direction.x*along+across.x*width,base.y+direction.y*along+across.y*width};
 }

 void rows(Rectangle box,int section,Vector3& u,Vector3& v)const{
  const auto direction=axis[section];
  const float inverse=1/cross2(direction,across);
  const Vector2 along{across.y*inverse,-across.x*inverse};
  const Vector2 width{-direction.y*inverse,direction.x*inverse};
  const Vector2 gx{sourceAxis.x*along.x-sourceAxis.y*width.x,sourceAxis.x*along.y-sourceAxis.y*width.y};
  const Vector2 gy{sourceAxis.y*along.x+sourceAxis.x*width.x,sourceAxis.y*along.y+sourceAxis.x*width.y};
  const auto target=section==2?ankle:knee;
  const float shift=section==2?ankleFromKnee:0.f;
  u={gx.x/box.width,gx.y/box.width,(sourceKnee.x+shift*sourceAxis.x-box.x-gx.x*target.x-gx.y*target.y)/box.width};
  v={gy.x/box.height,gy.y/box.height,(sourceKnee.y+shift*sourceAxis.y-box.y-gy.x*target.x-gy.y*target.y)/box.height};
 }
};

inline Vector2 rigidPartPoint(const Part& part,const SwordJoint& joint,Vector2 source){
 const auto point=joint.map(source,false);
 const float angle=part.angle*DEG2RAD,c=std::cos(angle),s=std::sin(angle);
 const float dx=(point.x-part.pivot.x)*part.scaleX,dy=(point.y-part.pivot.y)*part.scaleY;
 return {part.pivot.x+part.shift.x-64+c*dx-s*dy,part.pivot.y+part.shift.y-115+s*dx+c*dy};
}

inline LegProjection reaperLeg(View view,int slot,const Part& part,const SwordJoint& bend){
 LegProjection result{};
 const auto foot=footTip(reaper,view,slot);
 const auto source=reaperKnee(part.pivot,foot,0);
 const auto hip=rigidPartPoint(part,{},part.pivot);
 auto knee=rigidPartPoint(part,{},source.center);
 auto boot=rigidPartPoint(part,bend,foot);
 knee.x+=part.kneeShift.x;knee.y+=part.kneeShift.y;
 boot.x+=part.footShift.x;boot.y+=part.footShift.y;
 const Vector2 upper{knee.x-hip.x,knee.y-hip.y},lower{boot.x-knee.x,boot.y-knee.y};
 const float length=std::max(1.f,std::hypot(foot.x-part.pivot.x,foot.y-part.pivot.y));
 constexpr float Thigh=.47f,Shin=.37f,Boot=1-Thigh-Shin;
 const auto a=unit2(upper),b=unit2(lower);
 const auto along=unit2({a.x+b.x,a.y+b.y});
 const float width=std::sqrt(part.scaleX*part.scaleY);
 const float bootAngle=(part.angle+part.footPitch)*DEG2RAD;
 const float c=std::cos(bootAngle),s=std::sin(bootAngle);
 const Vector2 bootAxis{source.axis.x*c-source.axis.y*s,source.axis.x*s+source.axis.y*c};
 Vector2 restingAnkle{knee.x+lower.x*(Shin/(1-Thigh)),knee.y+lower.y*(Shin/(1-Thigh))};
 if(part.restingFootTurn!=0){
  const float a=part.restingFootTurn*DEG2RAD,c=std::cos(a),s=std::sin(a);
  const float dx=restingAnkle.x-boot.x,dy=restingAnkle.y-boot.y;
  restingAnkle={boot.x+c*dx-s*dy,boot.y+s*dx+c*dy};
 }
 const Vector2 walkingAnkle{boot.x-bootAxis.x*length*Boot*width,boot.y-bootAxis.y*length*Boot*width};
 const auto depth=unit2({mix(along.x,bootAxis.x,part.footWeight),mix(along.y,bootAxis.y,part.footWeight)});
 result.knee=knee;
 result.ankle={mix(restingAnkle.x,walkingAnkle.x,part.footWeight),mix(restingAnkle.y,walkingAnkle.y,part.footWeight)};
 result.sourceKnee=source.center;
 result.sourceAxis=source.axis;
 result.ankleFromKnee=length*Shin;
 result.across={-depth.y*width,depth.x*width};
 result.axis[0]={upper.x/(length*Thigh),upper.y/(length*Thigh)};
 result.axis[1]={(result.ankle.x-knee.x)/(length*Shin),(result.ankle.y-knee.y)/(length*Shin)};
 result.axis[2]={(boot.x-result.ankle.x)/(length*Boot),(boot.y-result.ankle.y)/(length*Boot)};
 result.split[0]={depth.x,depth.y,-depth.x*knee.x-depth.y*knee.y};
 result.split[1]={depth.x,depth.y,-depth.x*result.ankle.x-depth.y*result.ankle.y};
 result.active=std::all_of(std::begin(result.axis),std::end(result.axis),
                            [&](Vector2 axis){return cross2(axis,result.across)>.02f;});
 return result;
}

struct Rig {
 const Artwork& art;
 View angle;
 int jaw;
 float materialWeight=0;
 SwordJoint joints[6]{};
 LegProjection legs[2]{};
 SickleArmProjection sickleArm{};
 Rectangle box{},partBox[6]{};
 Vector4 uv0[6],uv1[6],guide0[6],guide1[6];
 Vector3 axisU[6],axisV[6],lighting[6];
 Vector3 lowerLegU[4]{},lowerLegV[4]{},legSplit[4]{},coat{};
 Vector4 shoulders[2]{};
 Texture2D held{};
 Vector3 heldU{},heldV{};
 Vector3 forearmU{},forearmV{};
 int order0[6],order1[6];
 const std::array<Part,6>& parts;
 Rig(const Artwork& artwork,View view,int gape,const std::array<Part,6>& pose,Color corpse,float flank,float action=-1):
       art(artwork),angle(view),jaw(gape),parts(pose){
  float left=1e6f,top=1e6f,right=-left,bottom=-top;
  constexpr float unit=128.f/160.f;
  const bool cels=art.directCels();
  const DirectionCels cells(view);
  if(cels){
   const int cell=cells.orderCel();
   const auto& layers=&art==&drake?drake_direction_cels::order[cell]:crawler_direction_cels::order[cell];
   std::copy_n(layers,6,order0);
   std::copy_n(layers,6,order1);
  }else if(singleDirectionalPaint(art)&&!art.signedEdges){
   order(art,view.nearestBase(),order0,action);
   std::copy_n(order0,6,order1);
  }else{
   order(art,view.base,order0,action);order(art,view.nextBase(),order1,action);
  }
  for(int p=0;p<6;p++){
   const auto& a=art.tile(view.sample,p,jaw);const auto& b=art.tile(view.nextSample(),p,jaw);
   const auto blend=[&](float a,float b){return mix(a,b,view.mix)*unit;};
   auto& bounds=partBox[p];
   bounds={blend(a.ox,b.ox),blend(a.oy,b.oy),blend(a.w,b.w),blend(a.h,b.h)};
   if(&art==&reaper&&p<3&&!art.signedEdges){
    // Move the center continuously, while the selected painting keeps the
    // width and floor registration it was rendered at.
    const int frame=view.nearestSample();
    const auto current=art.tile(frame,p,jaw);
    const float center=blend(a.ox+a.w*.5f,b.ox+b.w*.5f);
    bounds.width=.8f*current.w;
    bounds.x=center-bounds.width*.5f;
    bounds.y=reaperY(frame,current.oy);
    bounds.height=reaperY(frame,current.oy+current.h)-bounds.y;
   }else if(&art==&reaper){
    // The cutouts follow the same angular interpolation,
    // even when a different painting supplies the texels.
    bounds.y=mix(reaperY(view.sample,a.oy),reaperY(view.nextSample(),b.oy),view.mix);
    bounds.height=mix(reaperY(view.sample,a.oy+a.h),reaperY(view.nextSample(),b.oy+b.h),view.mix)-bounds.y;
   }
   const bool donor=&art==&imp && art.flow.id;
   const auto first=singleDirectionalPaint(art)&&!art.signedEdges?art.tile(view.nearestSample(),p,jaw):
                    donor?art.tile(view.base*4,p,jaw):a;
   uv0[p]=textureRect(art,first);
   uv1[p]=textureRect(art,donor?art.tile(view.nextBase()*4,p,jaw):b);
   if(rigidShell(art,p)){
    bounds=shellBounds(view);
    if(!cels){
     uv0[p]=textureRect(art,art.shell[view.base]);
     uv1[p]=textureRect(art,art.shell[view.nextBase()]);
    }
   }
   if(cels){
    uv0[p]=celRect(art,celTile(art,cells.first,p,jaw));
    uv1[p]=celRect(art,celTile(art,cells.next,p,jaw));
   }
   if(&art==&imp&&impCels.color.id){
    const ImpCelView cel(view);
    bounds=cel.bounds(p);
    const auto& page=impCels.color;
    const auto rect=[&](const Tile& tile){
     return Vector4{float(tile.x)/page.width,float(tile.y)/page.height,float(tile.w)/page.width,float(tile.h)/page.height};
    };
    uv0[p]=rect(cel.tile(p,continuousImp()?cel.first:-1));
    uv1[p]=rect(cel.tile(p,continuousImp()?cel.next:-1));
   }
   guide0[p]=guideRect(art,view.base,p);guide1[p]=guideRect(art,view.nextBase(),p);
   const auto& k=parts[p];
   const float c=std::cos(k.angle*DEG2RAD),s=std::sin(k.angle*DEG2RAD);
   const float px=k.pivot.x+k.shift.x-64,py=k.pivot.y+k.shift.y-115;
   const float uw=bounds.width*k.scaleX,vh=bounds.height*k.scaleY;
   const bool rooted=&art==&reaper&&p>=4&&filtered();
   const float basis=(rooted?parts[3].angle:k.angle)*DEG2RAD;
   const float bc=rooted?std::cos(basis):c,bs=rooted?std::sin(basis):s;
   if(rooted){
    const float front=p==5?std::max(0.f,std::sin(heading(view))):0.f;
    // The right pauldron sits on the breastplate in frontal views. Start
    // the arm's independent swing below its whole plate.
    shoulders[p-4]={px,py,k.angle*DEG2RAD-basis,5.5f+3.5f*front*front};
   }
   axisU[p]={bc/uw,bs/uw,(k.pivot.x-bounds.x)/bounds.width-(bc*px+bs*py)/uw};
   axisV[p]={-bs/vh,bc/vh,(k.pivot.y-bounds.y)/bounds.height+(bs*px-bc*py)/vh};
   Rectangle envelope=bounds;
   if(k.elbow!=0&&(p>=4||(singleDirectionalPaint(art)&&(p==1||p==2)))&&filtered()){
    auto& joint=joints[p];
    joint=&art==&jackal?jackalKnee(view,p,k.pivot,footTip(art,view,p),k.elbow):
          p<3?reaperKnee(k.pivot,footTip(art,view,p),k.elbow):
          &art==&imp?impElbow(view,p,k.elbow):&art==&reaper?reaperElbow(view,p,k.elbow):swordJoint(view,k.elbow);
    const float rx=std::max(std::abs(bounds.x-joint.center.x),std::abs(bounds.x+bounds.width-joint.center.x));
    const float ry=std::max(std::abs(bounds.y-joint.center.y),std::abs(bounds.y+bounds.height-joint.center.y));
    const float radius=std::hypot(rx,ry);
    envelope={joint.center.x-radius,joint.center.y-radius,2*radius,2*radius};
   }
   const auto extend=[&](Vector2 at){
    left=std::min(left,at.x);right=std::max(right,at.x);
    top=std::min(top,at.y);bottom=std::max(bottom,at.y);
   };
   if(rooted&&shoulders[p-4].z!=0){
    const float radius=shoulders[p-4].w+8.5f;
    extend({px-radius,py-radius});
    extend({px+radius,py+radius});
   }
   if(&art==&reaper&&p>=1&&p<=2&&filtered()){
    auto& leg=legs[p-1];
    leg=reaperLeg(view,p,k,joints[p]);
    if(leg.active){
     joints[p]={};
     leg.rows(bounds,0,axisU[p],axisV[p]);
     const int shin=2*(p-1);
     for(int section=0;section<2;section++){
      leg.rows(bounds,section+1,lowerLegU[shin+section],lowerLegV[shin+section]);
      legSplit[shin+section]=leg.split[section];
     }
    }
   }
   if(&art==&reaper&&p==5&&k.overhead!=0&&filtered()){
    sickleArm=projectSickleArm(view,k);
    if(sickleArm.active){
     joints[p]={};
     shoulders[1].z=0;
     sickleArm.rows(bounds,0,axisU[p],axisV[p]);
     sickleArm.rows(bounds,1,forearmU,forearmV);
    }
   }
   if(p==5&&sickleArm.active){
    for(float x:{bounds.x,bounds.x+bounds.width})for(float y:{bounds.y,bounds.y+bounds.height}){
     extend(sickleArm.map({x,y},0));
     extend(sickleArm.map({x,y},1));
    }
   }else if(p>=1&&p<=2&&legs[p-1].active){
    for(float x:{bounds.x,bounds.x+bounds.width})for(float y:{bounds.y,bounds.y+bounds.height}){
     for(int section=0;section<3;section++)extend(legs[p-1].map({x,y},section));
    }
   }else{
    for(float x:{envelope.x,envelope.x+envelope.width})for(float y:{envelope.y,envelope.y+envelope.height}){
     const float dx=(x-k.pivot.x)*k.scaleX,dy=(y-k.pivot.y)*k.scaleY;
     extend({px+c*dx-s*dy,py+s*dx+c*dy});
    }
   }
   const auto shade=[&](int key){
    const int authored=art.authoredPart(key,p);
    const bool far=flank>0?(authored==1||authored==4):(authored==2||authored==5);
    return authored==3?1.f:authored==0?.88f:far?.78f:.98f;
   };
   const float l=mix(shade(view.base),shade(view.nextBase()),view.t)/255.f;
   lighting[p]={l*corpse.r,l*corpse.g,l*corpse.b};
  }
  if(&art==&reaper){
   const int nearLeg=std::find(order0,order0+6,1)>std::find(order0,order0+6,2)?1:2;
   const auto& trunkBox=partBox[3];
   coat={(parts[3].pivot.y+8.f-trunkBox.y)/trunkBox.height,
         std::max(parts[1].footWeight,parts[2].footWeight),float(nearLeg)};
  }
  box={left,top,right-left,bottom-top};
 }
 void hold(Texture2D texture,Vector2 pivot,Vector2 grip,Vector2 size,float degrees){
  held=texture;
  const float c=std::cos(degrees*DEG2RAD),s=std::sin(degrees*DEG2RAD);
  heldU={c/size.x,s/size.x,grip.x-(c*pivot.x+s*pivot.y)/size.x};
  heldV={-s/size.y,c/size.y,grip.y+(s*pivot.x-c*pivot.y)/size.y};
  float left=box.x,top=box.y,right=box.x+box.width,bottom=box.y+box.height;
  for(float u:{0.f,1.f})for(float v:{0.f,1.f}){
   const float dx=(u-grip.x)*size.x,dy=(v-grip.y)*size.y;
   const float x=pivot.x+c*dx-s*dy,y=pivot.y+s*dx+c*dy;
   left=std::min(left,x);right=std::max(right,x);
   top=std::min(top,y);bottom=std::max(bottom,y);
  }
  box={left,top,right-left,bottom-top};
 }
 // The shoulder follows the torso at the socket. The arm's additional twist
 // reaches full strength below it. Radius stays invariant in both directions.
 Vector2 shoulderPoint(int part,Vector2 point,bool inverse)const{
  if(part<4||shoulders[part-4].z==0)return point;
  const auto socket=shoulders[part-4];
  const float x=point.x-socket.x,y=point.y-socket.y;
  const float t=std::clamp((std::hypot(x,y)-socket.w)/8.5f,0.f,1.f);
  const float rot=(inverse?-socket.z:socket.z)*t*t*(3-2*t);
  const float c=std::cos(rot),s=std::sin(rot);
  return {socket.x+c*x-s*y,socket.y+s*x+c*y};
 }
 Vector2 partPoint(int part,Vector2 point)const{
  if(part==5&&sickleArm.active)return sickleArm.map(point);
  if(part>=1&&part<=2&&legs[part-1].active)return legs[part-1].map(point);
  if(part>=4&&shoulders[part-4].z!=0){
   auto rooted=parts[part];
   rooted.angle=parts[3].angle;
   return shoulderPoint(part,rigidPartPoint(rooted,joints[part],point),false);
  }
  return rigidPartPoint(parts[part],joints[part],point);
 }
 bool inFrontOf(int part,int other)const{
  return std::find(order0,order0+6,part)>std::find(order0,order0+6,other);
 }
 bool hit(Vector2 q)const{
  float uncovered=1;
  for(int p=0;p<6;p++){
   const auto point=shoulderPoint(p,q,true);
   auto dot=[&](Vector3 axis){return point.x*axis.x+point.y*axis.y+axis.z;};
   float u=dot(axisU[p]),v=dot(axisV[p]);
   if(p==5&&dot(sickleArm.split)>0){u=dot(forearmU);v=dot(forearmV);}
   if(p>=1&&p<=2){
    const int shin=2*(p-1),boot=shin+1;
    const int section=dot(legSplit[boot])>0?boot:dot(legSplit[shin])>0?shin:-1;
    if(section>=0){u=dot(lowerLegU[section]);v=dot(lowerLegV[section]);}
   }
   if(joints[p].angle!=0){
    const auto& joint=joints[p];
    const auto& box=partBox[p];
    const auto source=joint.map({box.x+u*box.width,box.y+v*box.height},true);
    u=(source.x-box.x)/box.width;v=(source.y-box.y)/box.height;
   }
   if(u<0||u>1||v<0||v>1)continue;
   uncovered*=1-turnCoverage(art,angle,p,jaw,u,v);
   if(uncovered<=1-100.f/255)return true;
  }
  return false;
 }
 void begin(Texture2D relief,float collapse){
  materialWeight=collapse;
  if(!filtered())return;
  BeginShaderMode(program);
  Vector3 axes[6]{};Vector4 boxes[6]{};
  for(int p=0;p<6;p++){
   const auto& joint=joints[p];const auto& box=partBox[p];
   axes[p]={joint.axis.x,joint.axis.y,joint.angle};
   if(joint.angle!=0)boxes[p]={(joint.center.x-box.x)/box.width,(joint.center.y-box.y)/box.height,box.width,box.height};
  }
  auto send=[](int loc,const void* ptr,int type){SetShaderValueV(program,loc,ptr,type,6);};
  send(loc.jointAxis,axes,SHADER_UNIFORM_VEC3);send(loc.jointBox,boxes,SHADER_UNIFORM_VEC4);
  SetShaderValueV(program,loc.legU,lowerLegU,SHADER_UNIFORM_VEC3,4);
  SetShaderValueV(program,loc.legV,lowerLegV,SHADER_UNIFORM_VEC3,4);
  SetShaderValueV(program,loc.legSplit,legSplit,SHADER_UNIFORM_VEC3,4);
  SetShaderValue(program,loc.reaperCoat,&coat,SHADER_UNIFORM_VEC3);
  SetShaderValueV(program,loc.reaperShoulders,shoulders,SHADER_UNIFORM_VEC4,2);
  const Vector3 armSplit=sickleArm.active?sickleArm.split:Vector3{};
  SetShaderValue(program,loc.forearmSplit,&armSplit,SHADER_UNIFORM_VEC3);
  if(sickleArm.active){
   SetShaderValue(program,loc.forearmU,&forearmU,SHADER_UNIFORM_VEC3);
   SetShaderValue(program,loc.forearmV,&forearmV,SHADER_UNIFORM_VEC3);
  }
  const int heldActive=held.id!=0;
  SetShaderValue(program,loc.heldActive,&heldActive,SHADER_UNIFORM_INT);
  if(heldActive){
   SetShaderValueTexture(program,loc.heldInk,held);
   SetShaderValue(program,loc.heldU,&heldU,SHADER_UNIFORM_VEC3);
   SetShaderValue(program,loc.heldV,&heldV,SHADER_UNIFORM_VEC3);
  }
  send(loc.uv0,uv0,SHADER_UNIFORM_VEC4);send(loc.uv1,uv1,SHADER_UNIFORM_VEC4);
  send(loc.axisU,axisU,SHADER_UNIFORM_VEC3);send(loc.axisV,axisV,SHADER_UNIFORM_VEC3);
  send(loc.order0,order0,SHADER_UNIFORM_INT);send(loc.order1,order1,SHADER_UNIFORM_INT);
  if(collapse>0){
   send(loc.guide0,guide0,SHADER_UNIFORM_VEC4);send(loc.guide1,guide1,SHADER_UNIFORM_VEC4);
   send(loc.lighting,lighting,SHADER_UNIFORM_VEC3);SetShaderValueTexture(program,loc.material,relief);
  }
  if(art.directCels()&&art.cels[1].color.id)SetShaderValueTexture(program,loc.page1,art.cels[1].color);
  else if(art.pageCount>1)SetShaderValueTexture(program,loc.page1,art.pages[1].color);
  const bool deform=&art==&imp&&continuousImp();
  const int impNode=&art==&reaper?-6:art.directCels()?-5:singleDirectionalPaint(art)?-3:&art!=&imp?-1:
                    deform?-4:impCels.color.id?-3:art.flow.id?angle.base*5+angle.sample%4:-2;
  SetShaderValue(program,loc.impNode,&impNode,SHADER_UNIFORM_INT);
  const int edgeJaw=art.signedEdges?jaw+1:0;
  SetShaderValue(program,loc.edgeJaw,&edgeJaw,SHADER_UNIFORM_INT);
  const int solidOrder=singleDirectionalPaint(art)&&!art.signedEdges;
  SetShaderValue(program,loc.solidLayerOrder,&solidOrder,SHADER_UNIFORM_INT);
  const float shell=rigidShell(art,3)?angle.t:-1;
  SetShaderValue(program,loc.shellAngle,&shell,SHADER_UNIFORM_FLOAT);
  const ImpCelView cel(angle);
  const int flowView=deform?cel.first:art.flow.id&& &art!=&imp?angle.sample:-1;
  SetShaderValue(program,loc.flowView,&flowView,SHADER_UNIFORM_INT);
  if(deform)SetShaderValueTexture(program,loc.flow,impContour.flow);
  else if(art.flow.id&&impNode!=-3)SetShaderValueTexture(program,loc.flow,art.flow);
  const Vector3 phase{art.directCels()?DirectionCels(angle).fraction:deform?cel.t:angle.mix,angle.t,collapse};
  SetShaderValue(program,loc.phase,&phase,SHADER_UNIFORM_VEC3);
 }
 void paint(bool mask,Color tint,float dx=0,float dy=0,bool shadow=false)const{
  if(!filtered()){
   const auto* rows=angle.t<.5f?order0:order1;
   for(int i=0;i<6;i++){
    const int p=rows[i];const auto& k=parts[p];const auto& b=partBox[p];
    const bool cel=&art==&imp&&impCels.color.id;
    const int sample=singleDirectionalPaint(art)?angle.nearestSample():angle.sample;
    const auto t=cel?ImpCelView(angle).tile(p):art.tile(sample,p,jaw);
    const auto& page=cel?impCels:art.pages[t.page];
    Color color=tint;
    if(!mask && materialWeight>0){
     color.r=static_cast<unsigned char>(tint.r*mix(1,lighting[p].x,materialWeight));
     color.g=static_cast<unsigned char>(tint.g*mix(1,lighting[p].y,materialWeight));
     color.b=static_cast<unsigned char>(tint.b*mix(1,lighting[p].z,materialWeight));
    }
    DrawTexturePro(mask?page.mask:page.color,{float(t.x),float(t.y),float(t.w),float(t.h)},
      {k.pivot.x-64+k.shift.x+dx,k.pivot.y-115+k.shift.y+dy,b.width*k.scaleX,b.height*k.scaleY},
      {(k.pivot.x-b.x)*k.scaleX,(k.pivot.y-b.y)*k.scaleY},k.angle,color);
   }
   return;
  }
  const auto& page=&art==&imp&&impCels.color.id?impCels:art.pages[0];
  const auto texture=art.directCels()?art.cels[0].color:&art==&imp&&continuousImp()?impContour.ink:page.color;
  rlSetTexture(texture.id);rlBegin(RL_QUADS);rlColor4ub(tint.r,tint.g,tint.b,tint.a);
  auto vertex=[&](float x,float y){rlTexCoord2f(x+(shadow?1024.f:mask?512.f:0.f),y);rlVertex2f(x+dx,y+dy);};
  vertex(box.x,box.y);vertex(box.x,box.y+box.height);
  vertex(box.x+box.width,box.y+box.height);vertex(box.x+box.width,box.y);
  rlEnd();rlSetTexture(0);
 }
};
}
