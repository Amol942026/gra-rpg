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
 bool ready()const{return pages[0].color.id!=0;}
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
         at(a.scaleX,b.scaleX),at(a.scaleY,b.scaleY),at(a.elbow,b.elbow)};
}
struct View {
 int base=0,sample=0;
 float t=0,mix=0;
 int nextBase()const{return (base+1)%8;}
 int nextSample()const{return (sample+1)%32;}
 int nearestBase()const{return t<.5f?base:nextBase();}
};
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
 if(!art.ready()||!state.facingInitialized)return {key,key*4,0,0};
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
 const float f=t*4;
 const int sub=std::min(3,int(f));
 View result{base,4*base+sub,t,f-sub};
 if(!filtered()){
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
struct Locations {
 int uv0,uv1,axisU,axisV,guide0,guide1,order0,order1,lighting,phase,page1,material,flow,flowView;
 int elbowBox,elbowAxis,impNode;
};
inline Locations loc{};
inline void unloadPages(Artwork& art){
 for(auto& p:art.pages){if(p.color.id)UnloadTexture(p.color);if(p.mask.id)UnloadTexture(p.mask);p={};}
 if(art.flow.id)UnloadTexture(art.flow);
 art.flow={};art.vectors.clear();
}
inline void unload(){
 if(impCels.color.id)UnloadTexture(impCels.color);
 if(impCels.mask.id)UnloadTexture(impCels.mask);
 impContour.unload();
 impCels={};
 unloadPages(drake);unloadPages(crawler);unloadPages(imp);unloadPages(sentinel);unloadPages(cinderhorn);
 if(filtered())UnloadShader(program);
 program={};
}
inline void loadProgram(){
 if(program.id)return;
 // Sample both registered paintings into a single premultiplied layer. Resolve
 // their two possible wing/head depth orders inside the sprite, before it is
 // composited over the ground. This also keeps its coverage solid at mid-turn.
 const auto fragment=std::string("#version 330\n")+painted_turn_shape::shaderCode+R"GLSL(
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;
uniform sampler2D texture0, page1, material, turnFlow;
uniform int flowView,impNode;
uniform vec4 uv0[6], uv1[6], guide0[6], guide1[6];
uniform vec3 axisU[6], axisV[6], lighting[6];
uniform int order0[6],order1[6];
uniform vec3 phase;
uniform vec4 elbowBox[2];
uniform vec3 elbowAxis[2];
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
    if(impNode>=0){
        vec2 dim=vec2(textureSize(texture0,0)),px=coords*dim-.5;
        coords=(floor(px)+smoothstep(vec2(.13),vec2(.87),fract(px))+.5)/dim;
    }
    return tile.x>=1.0?texture(page1,coords):texture(texture0,coords);
}
vec2 displacement(vec2 uv,int part){
    vec2 cell=clamp(uv,vec2(.5/32.),vec2(1.-.5/32.));
    vec2 sampleAt=(vec2(float(flowView),float(part))+cell)/vec2(32.,6.);
    return (texture(turnFlow,sampleAt).rg*255.-128.)/384.;
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
void main() {
    // Detached pieces address one painted layer in its original coordinates.
    // A whole actor uses the regular untagged six-layer composition.
    int group=int(floor((fragTexCoord.x+256.0)/2048.0));
    float x=fragTexCoord.x-float(group)*2048.0;
    bool shadow=x>800.0,mask=x>300.0;
    vec3 point=vec3(x-(shadow?1024.0:mask?512.0:0.0),fragTexCoord.y,1.0);
    vec4 layers[6];
    for(int i=0;i<6;i++){
        layers[i]=vec4(0.0);
        if(group>0 && i!=group-1)continue;
        vec2 at=vec2(dot(axisU[i],point),dot(axisV[i],point));
        if(i>=4 && elbowAxis[i-4].z!=0.)at=unbend(at,elbowBox[i-4],elbowAxis[i-4]);
        if(any(lessThan(at,vec2(0))) || any(greaterThan(at,vec2(1))))continue;
        vec2 a=at,b=at;
        if(impNode!=-4 && flowView>=0 && phase.x>0. && phase.x<1.){
            // Follow the same painted feature from both views. Blending at a
            // fixed UV leaves two sword edges, horns or visors in one frame.
            vec2 d=displacement(at,i);
            d=displacement(at+(.5-phase.x)*d,i);
            a-=phase.x*d;b+=(1.-phase.x)*d;
        }
        vec4 paint;
        if(impNode==-4){
            vec2 dim=vec2(textureSize(texture0,0));
            vec4 size=dim.xyxy;
            paint=paintedShape(texture0,turnFlow,flowView,i,uv0[i]*size,uv1[i]*size,phase.x,at);
            paint.rgb*=paint.a;
        }
        else if(impNode==-3)paint=pigment(uv0[i],at);
        else if(impNode>=0)paint=impPaint(at,i);
        else if(impNode==-2)paint=phase.x<.5?pigment(uv0[i],a):pigment(uv1[i],b);
        else paint=mix(pigment(uv0[i],a),pigment(uv1[i],b),phase.x);
        if(impNode!=-4 && flowView>=0 && paint.a>.001){
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
    else{
        for(int i=0;i<6;i++){
            vec4 a=layers[order0[i]],b=layers[order1[i]];
            left=a+left*(1.-a.a);
            right=b+right*(1.-b.a);
        }
        // An opaque shield passes in front of the torso along its edge.
        // A dissolve between depth orders would show the cuirass through it.
        float orderMix=phase.y;
        if(impNode!=-1)orderMix=step(clamp(.5+point.x/112.,.24,.76),phase.y);
        else if(flowView>=0){
            float crossAt=clamp(.5+point.x/112.,.24,.76);
            orderMix=smoothstep(crossAt-.045,crossAt+.045,phase.y);
        }
        mixed=mix(left,right,orderMix);
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
       location("turnFlow"),location("flowView"),location("elbowBox[0]"),location("elbowAxis[0]"),location("impNode")};
 }
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
inline void load(){
 loadProgram();loadPages(drake);loadPages(crawler);loadPages(cinderhorn);
 if(!caldera_puppets::ready(caldera_puppets::Imp)){loadPages(imp);loadImpCels();}
 if(filtered())loadPages(sentinel);
}
inline Vector4 textureRect(const Artwork& art,const Tile& t){
 const auto& page=art.pages[t.page].color;
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
 float ax=u,ay=v,bx=u,by=v;
 const float t=angle.mix;
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
  auto d=displacement(art,angle.sample,part,u,v);
  d=displacement(art,angle.sample,part,u+(.5f-t)*d.x,v+(.5f-t)*d.y);
  ax-=t*d.x;ay-=t*d.y;bx+=(1-t)*d.x;by+=(1-t)*d.y;
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
struct Rig {
 const Artwork& art;
 View angle;
 int jaw;
 float materialWeight=0;
 SwordJoint arms[2]{};
 Rectangle box{},partBox[6]{};
 Vector4 uv0[6],uv1[6],guide0[6],guide1[6];
 Vector3 axisU[6],axisV[6],lighting[6];
 int order0[6],order1[6];
 const std::array<Part,6>& parts;
 Rig(const Artwork& artwork,View view,int gape,const std::array<Part,6>& pose,Color corpse,float flank,float action=-1):
       art(artwork),angle(view),jaw(gape),parts(pose){
  float left=1e6f,top=1e6f,right=-left,bottom=-top;
  constexpr float unit=128.f/160.f;
  order(art,view.base,order0,action);order(art,view.nextBase(),order1,action);
  for(int p=0;p<6;p++){
   const auto& a=art.tile(view.sample,p,jaw);const auto& b=art.tile(view.nextSample(),p,jaw);
   const auto blend=[&](float a,float b){return mix(a,b,view.mix)*unit;};
   auto& bounds=partBox[p];
   bounds={blend(a.ox,b.ox),blend(a.oy,b.oy),blend(a.w,b.w),blend(a.h,b.h)};
   const bool donor=&art==&imp && art.flow.id;
   uv0[p]=textureRect(art,donor?art.tile(view.base*4,p,jaw):a);
   uv1[p]=textureRect(art,donor?art.tile(view.nextBase()*4,p,jaw):b);
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
   axisU[p]={c/uw,s/uw,(k.pivot.x-bounds.x)/bounds.width-(c*px+s*py)/uw};
   axisV[p]={-s/vh,c/vh,(k.pivot.y-bounds.y)/bounds.height+(s*px-c*py)/vh};
   Rectangle envelope=bounds;
   if(p>=4 && k.elbow!=0){
    auto& joint=arms[p-4];
    joint=&art==&imp?impElbow(view,p,k.elbow):swordJoint(view,k.elbow);
    const float rx=std::max(std::abs(bounds.x-joint.center.x),std::abs(bounds.x+bounds.width-joint.center.x));
    const float ry=std::max(std::abs(bounds.y-joint.center.y),std::abs(bounds.y+bounds.height-joint.center.y));
    const float radius=std::hypot(rx,ry);
    envelope={joint.center.x-radius,joint.center.y-radius,2*radius,2*radius};
   }
   for(float x:{envelope.x,envelope.x+envelope.width})for(float y:{envelope.y,envelope.y+envelope.height}){
    const float dx=(x-k.pivot.x)*k.scaleX,dy=(y-k.pivot.y)*k.scaleY;
    const float xx=px+c*dx-s*dy,yy=py+s*dx+c*dy;
    left=std::min(left,xx);right=std::max(right,xx);top=std::min(top,yy);bottom=std::max(bottom,yy);
   }
   const auto shade=[&](int key){
    const int authored=art.authoredPart(key,p);
    const bool far=flank>0?(authored==1||authored==4):(authored==2||authored==5);
    return authored==3?1.f:authored==0?.88f:far?.78f:.98f;
   };
   const float l=mix(shade(view.base),shade(view.nextBase()),view.t)/255.f;
   lighting[p]={l*corpse.r,l*corpse.g,l*corpse.b};
  }
  box={left,top,right-left,bottom-top};
 }
 bool hit(Vector2 q)const{
  float uncovered=1;
  for(int p=0;p<6;p++){
   auto dot=[&](Vector3 axis){return q.x*axis.x+q.y*axis.y+axis.z;};
   float u=dot(axisU[p]),v=dot(axisV[p]);
   if(p>=4&&arms[p-4].angle!=0){
    const auto& joint=arms[p-4];
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
  Vector3 axes[2]{};Vector4 boxes[2]{};
  for(int arm=0;arm<2;arm++){
   const auto& joint=arms[arm];const auto& box=partBox[arm+4];
   axes[arm]={joint.axis.x,joint.axis.y,joint.angle};
   if(joint.angle!=0)boxes[arm]={(joint.center.x-box.x)/box.width,(joint.center.y-box.y)/box.height,box.width,box.height};
  }
  SetShaderValueV(program,loc.elbowAxis,axes,SHADER_UNIFORM_VEC3,2);
  if(arms[0].angle!=0||arms[1].angle!=0)SetShaderValueV(program,loc.elbowBox,boxes,SHADER_UNIFORM_VEC4,2);
  auto send=[](int loc,const void* ptr,int type){SetShaderValueV(program,loc,ptr,type,6);};
  send(loc.uv0,uv0,SHADER_UNIFORM_VEC4);send(loc.uv1,uv1,SHADER_UNIFORM_VEC4);
  send(loc.axisU,axisU,SHADER_UNIFORM_VEC3);send(loc.axisV,axisV,SHADER_UNIFORM_VEC3);
  send(loc.order0,order0,SHADER_UNIFORM_INT);send(loc.order1,order1,SHADER_UNIFORM_INT);
  if(collapse>0){
   send(loc.guide0,guide0,SHADER_UNIFORM_VEC4);send(loc.guide1,guide1,SHADER_UNIFORM_VEC4);
   send(loc.lighting,lighting,SHADER_UNIFORM_VEC3);SetShaderValueTexture(program,loc.material,relief);
  }
  if(art.pageCount>1)SetShaderValueTexture(program,loc.page1,art.pages[1].color);
  const bool deform=&art==&imp&&continuousImp();
  const int impNode=&art!=&imp?-1:deform?-4:impCels.color.id?-3:art.flow.id?angle.base*5+angle.sample%4:-2;
  SetShaderValue(program,loc.impNode,&impNode,SHADER_UNIFORM_INT);
  const ImpCelView cel(angle);
  const int flowView=deform?cel.first:art.flow.id&& &art!=&imp?angle.sample:-1;
  SetShaderValue(program,loc.flowView,&flowView,SHADER_UNIFORM_INT);
  if(deform)SetShaderValueTexture(program,loc.flow,impContour.flow);
  else if(art.flow.id&&impNode!=-3)SetShaderValueTexture(program,loc.flow,art.flow);
  const Vector3 phase{deform?cel.t:angle.mix,angle.t,collapse};
  SetShaderValue(program,loc.phase,&phase,SHADER_UNIFORM_VEC3);
 }
 void paint(bool mask,Color tint,float dx=0,float dy=0,bool shadow=false)const{
  if(!filtered()){
   const auto* rows=angle.t<.5f?order0:order1;
   for(int i=0;i<6;i++){
    const int p=rows[i];const auto& k=parts[p];const auto& b=partBox[p];
    const bool cel=&art==&imp&&impCels.color.id;
    const auto t=cel?ImpCelView(angle).tile(p):art.tile(angle.sample,p,jaw);
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
  const auto texture=&art==&imp&&continuousImp()?impContour.ink:page.color;
  rlSetTexture(texture.id);rlBegin(RL_QUADS);rlColor4ub(tint.r,tint.g,tint.b,tint.a);
  auto vertex=[&](float x,float y){rlTexCoord2f(x+(shadow?1024.f:mask?512.f:0.f),y);rlVertex2f(x+dx,y+dy);};
  vertex(box.x,box.y);vertex(box.x,box.y+box.height);
  vertex(box.x+box.width,box.y+box.height);vertex(box.x+box.width,box.y);
  rlEnd();rlSetTexture(0);
 }
};
}
