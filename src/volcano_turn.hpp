#pragma once

// Included inside volcano_actors after Part and facingAngles have been defined.
namespace turn_art {
using Tile=::volcano_turn_data::Tile;
namespace drake_data = ::ember_drake_turn_atlas;
namespace crawler_data = ::slag_crawler_turn_atlas;
static_assert(drake_data::Count==32 && drake_data::Parts==6 && drake_data::Cell==160 && drake_data::PageCount<=2);
static_assert(crawler_data::Count==32 && crawler_data::Parts==6 && crawler_data::Cell==160 && crawler_data::PageCount<=2);
struct Page {Texture2D color{},mask{};std::vector<unsigned char> alpha;};
struct Artwork {
 const char* folder;
 int pageCount;
 Tile (*tile)(int view,int part,int jaw);
 int (*authoredPart)(int key,int part);
 const int (*order)[6];
 std::array<Page,2> pages{};
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
inline const Artwork* forType(int type){
 if(type==SlagCrawlerType && crawler.ready())return &crawler;
 if(type==EmberDrakeType && drake.ready())return &drake;
 return nullptr;
}
inline Shader program{};
inline bool filtered(){return program.id && program.id!=rlGetShaderIdDefault();}
inline float mix(float a,float b,float t){return a+t*(b-a);}
inline Part interpolate(const Part& a,const Part& b,float t){
 const auto at=[t](float start,float end){return mix(start,end,t);};
 return {{at(a.pivot.x,b.pivot.x),at(a.pivot.y,b.pivot.y)},
         {at(a.shift.x,b.shift.x),at(a.shift.y,b.shift.y)},at(a.angle,b.angle),
         at(a.scaleX,b.scaleX),at(a.scaleY,b.scaleY)};
}
struct View {
 int base=0,sample=0;
 float t=0,mix=0;
 int nextBase()const{return (base+1)%8;}
 int nextSample()const{return (sample+1)%32;}
 int nearestBase()const{return t<.5f?base:nextBase();}
};
inline View view(const AnimationState& state,const Artwork& art){
 const int key=std::clamp(int(state.direction),0,7);
 if(!art.ready()||!state.facingInitialized)return {key,key*4,0,0};
 float radians=std::fmod(state.facingAngle,2*PI);
 if(radians<0)radians+=2*PI;
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
struct Locations {
 int uv0,uv1,axisU,axisV,guide0,guide1,order0,order1,lighting,phase,page1,material;
};
inline Locations loc{};
inline void unloadPages(Artwork& art){
 for(auto& p:art.pages){if(p.color.id)UnloadTexture(p.color);if(p.mask.id)UnloadTexture(p.mask);p={};}
}
inline void unload(){
 unloadPages(drake);unloadPages(crawler);
 if(filtered())UnloadShader(program);
 program={};
}
inline void loadProgram(){
 if(program.id)return;
 // Sample both registered paintings into a single premultiplied layer. Resolve
 // their two possible wing/head depth orders inside the sprite, before it is
 // composited over the ground. This also keeps its coverage solid at mid-turn.
 program=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;
uniform sampler2D texture0, page1, material;
uniform vec4 uv0[6], uv1[6], guide0[6], guide1[6];
uniform vec3 axisU[6], axisV[6], lighting[6];
uniform int order0[6],order1[6];
uniform vec3 phase;
vec4 pigment(vec4 tile,vec2 uv) {
    vec2 coords=vec2(fract(tile.x),tile.y)+uv*tile.zw;
    return tile.x>=1.0?texture(page1,coords):texture(texture0,coords);
}
void main() {
    bool shadow=fragTexCoord.x>800.0,mask=fragTexCoord.x>300.0;
    vec3 point=vec3(fragTexCoord.x-(shadow?1024.0:mask?512.0:0.0),fragTexCoord.y,1.0);
    vec4 layers[6];
    for(int i=0;i<6;i++){
        vec2 at=vec2(dot(axisU[i],point),dot(axisV[i],point));
        layers[i]=vec4(0.0);
        if(any(lessThan(at,vec2(0))) || any(greaterThan(at,vec2(1))))continue;
        vec4 paint=mix(pigment(uv0[i],at),pigment(uv1[i],at),phase.x);
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
    for(int i=0;i<6;i++){
        vec4 a=layers[order0[i]],b=layers[order1[i]];
        left=a+left*(1.-a.a);
        right=b+right*(1.-b.a);
    }
    vec4 mixed=mix(left,right,phase.y);
    if(shadow){
        if(mixed.a<.62)discard;
        finalColor=vec4(fragColor.a);
        return;
    }
    if(mixed.a<.0039)discard;
    finalColor=vec4(mask?fragColor.rgb:mixed.rgb/mixed.a,mixed.a*fragColor.a);
})GLSL");
 if(filtered()){
  auto location=[](const char* name){return GetShaderLocation(program,name);};
  loc={location("uv0[0]"),location("uv1[0]"),location("axisU[0]"),location("axisV[0]"),
       location("guide0[0]"),location("guide1[0]"),location("order0[0]"),location("order1[0]"),
       location("lighting[0]"),location("phase"),location("page1"),location("material")};
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
}
inline void load(){loadProgram();loadPages(drake);loadPages(crawler);}
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
inline float coverage(const Artwork& art,const Tile& t,float u,float v){
 const auto& p=art.pages[t.page];
 const float x=std::clamp(t.x+u*t.w-.5f,0.f,float(p.color.width-1));
 const float y=std::clamp(t.y+v*t.h-.5f,0.f,float(p.color.height-1));
 const int ix=int(x),iy=int(y),jx=std::min(ix+1,p.color.width-1),jy=std::min(iy+1,p.color.height-1);
 const auto* a=p.alpha.data()+iy*p.color.width;
 const auto* b=p.alpha.data()+jy*p.color.width;
 return mix(mix(a[ix],a[jx],x-ix),mix(b[ix],b[jx],x-ix),y-iy)/255.f;
}
inline void order(const Artwork& art,int base,int* rows){
 for(int p=0;p<6;++p)rows[p]=art.authoredPart(base,art.order[base][p]);
}
struct Rig {
 const Artwork& art;
 View angle;
 int jaw;
 float materialWeight=0;
 Rectangle box{},partBox[6]{};
 Vector4 uv0[6],uv1[6],guide0[6],guide1[6];
 Vector3 axisU[6],axisV[6],lighting[6];
 int order0[6],order1[6];
 const std::array<Part,6>& parts;
 Rig(const Artwork& artwork,View view,int gape,const std::array<Part,6>& pose,Color corpse,float flank):
       art(artwork),angle(view),jaw(gape),parts(pose){
  float left=1e6f,top=1e6f,right=-left,bottom=-top;
  constexpr float unit=128.f/160.f;
  order(art,view.base,order0);order(art,view.nextBase(),order1);
  for(int p=0;p<6;p++){
   const auto& a=art.tile(view.sample,p,jaw);const auto& b=art.tile(view.nextSample(),p,jaw);
   const auto blend=[&](float a,float b){return mix(a,b,view.mix)*unit;};
   auto& bounds=partBox[p];
   bounds={blend(a.ox,b.ox),blend(a.oy,b.oy),blend(a.w,b.w),blend(a.h,b.h)};
   uv0[p]=textureRect(art,a);uv1[p]=textureRect(art,b);
   guide0[p]=guideRect(art,view.base,p);guide1[p]=guideRect(art,view.nextBase(),p);
   const auto& k=parts[p];
   const float c=std::cos(k.angle*DEG2RAD),s=std::sin(k.angle*DEG2RAD);
   const float px=k.pivot.x+k.shift.x-64,py=k.pivot.y+k.shift.y-115;
   const float uw=bounds.width*k.scaleX,vh=bounds.height*k.scaleY;
   axisU[p]={c/uw,s/uw,(k.pivot.x-bounds.x)/bounds.width-(c*px+s*py)/uw};
   axisV[p]={-s/vh,c/vh,(k.pivot.y-bounds.y)/bounds.height+(s*px-c*py)/vh};
   for(float x:{bounds.x,bounds.x+bounds.width})for(float y:{bounds.y,bounds.y+bounds.height}){
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
   const float u=dot(axisU[p]),v=dot(axisV[p]);
   if(u<0||u>1||v<0||v>1)continue;
   const float a=coverage(art,art.tile(angle.sample,p,jaw),u,v);
   const float b=coverage(art,art.tile(angle.nextSample(),p,jaw),u,v);
   uncovered*=1-mix(a,b,angle.mix);
   if(uncovered<=1-100.f/255)return true;
  }
  return false;
 }
 void begin(Texture2D relief,float collapse){
  materialWeight=collapse;
  if(!filtered())return;
  BeginShaderMode(program);
  auto send=[](int loc,const void* ptr,int type){SetShaderValueV(program,loc,ptr,type,6);};
  send(loc.uv0,uv0,SHADER_UNIFORM_VEC4);send(loc.uv1,uv1,SHADER_UNIFORM_VEC4);
  send(loc.axisU,axisU,SHADER_UNIFORM_VEC3);send(loc.axisV,axisV,SHADER_UNIFORM_VEC3);
  send(loc.order0,order0,SHADER_UNIFORM_INT);send(loc.order1,order1,SHADER_UNIFORM_INT);
  if(collapse>0){
   send(loc.guide0,guide0,SHADER_UNIFORM_VEC4);send(loc.guide1,guide1,SHADER_UNIFORM_VEC4);
   send(loc.lighting,lighting,SHADER_UNIFORM_VEC3);SetShaderValueTexture(program,loc.material,relief);
  }
  if(art.pageCount>1)SetShaderValueTexture(program,loc.page1,art.pages[1].color);
  const Vector3 phase{angle.mix,angle.t,collapse};
  SetShaderValue(program,loc.phase,&phase,SHADER_UNIFORM_VEC3);
 }
 void paint(bool mask,Color tint,float dx=0,float dy=0,bool shadow=false)const{
  if(!filtered()){
   const auto* rows=angle.t<.5f?order0:order1;
   for(int i=0;i<6;i++){
    const int p=rows[i];const auto& k=parts[p];const auto& b=partBox[p];
    const auto& t=art.tile(angle.sample,p,jaw);const auto& page=art.pages[t.page];
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
  rlSetTexture(art.pages[0].color.id);rlBegin(RL_QUADS);rlColor4ub(tint.r,tint.g,tint.b,tint.a);
  auto vertex=[&](float x,float y){rlTexCoord2f(x+(shadow?1024.f:mask?512.f:0.f),y);rlVertex2f(x+dx,y+dy);};
  vertex(box.x,box.y);vertex(box.x,box.y+box.height);
  vertex(box.x+box.width,box.y+box.height);vertex(box.x+box.width,box.y);
  rlEnd();rlSetTexture(0);
 }
};
}
