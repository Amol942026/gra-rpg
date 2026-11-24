#pragma once

namespace jackal_cels {
// Fixed cells exported from the layered Synfig project by tools/media/jackal_cels.py.
inline constexpr int Width=172,Height=150,Columns=9,Padding=2,Walk=28,Attack=17,Fall=14;
inline constexpr int Frames=Walk+1+Attack+2*Fall;
inline constexpr int SheetWidth=Columns*(Width+Padding),SheetHeight=((Frames+Columns-1)/Columns)*(Height+Padding);
inline constexpr float WorldWidth=138,WorldHeight=120,AnchorX=69,AnchorY=104;
struct Page {Texture2D ink{};std::vector<unsigned char> alpha;};
struct Facing {int page;bool mirrored;};
inline constexpr int BasePages=5;
inline constexpr std::array<int,9> Paintings{0,1,2,6,7,8,9,10,11};
inline constexpr std::array<Facing,desert_motion::JackalFacings> Facings{{
 {0,false},{5,false},{1,false},{6,false},{2,false},{6,true},{1,true},{5,true},
 {0,true},{8,true},{4,true},{7,true},{3,false},{7,false},{4,false},{8,false}
}};
inline std::array<Page,Paintings.size()> pages;
inline Shader flat{};
inline bool ready(){return std::all_of(pages.begin(),pages.begin()+BasePages,[](const auto& p){return p.ink.id!=0;});}
#include "jackal_turn_warp.hpp"

inline void unload(){
 for(auto& p:pages){if(p.ink.id)UnloadTexture(p.ink);p={};}
 if(flat.id&&flat.id!=rlGetShaderIdDefault())UnloadShader(flat);
 flat={};turn_warp::unload();
}

inline bool loadPages(const std::string& root,bool smooth){
 for(size_t i=0;i<pages.size();i++){
  const auto suffix=std::to_string(Paintings[i])+".png";
  const auto file=root+(smooth?"body-":"cel-")+suffix;
  Image image=LoadImage(file.c_str());
  if(!image.data||image.width!=SheetWidth||image.height!=SheetHeight){
   if(image.data)UnloadImage(image);
   if(smooth||i<BasePages)return false;
   continue;
  }
  ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  auto& p=pages[i];
  auto* pixels=static_cast<Color*>(image.data);
  p.alpha.resize(SheetWidth*SheetHeight);
  if(smooth){
   Image distance=LoadImage((root+"distance-"+suffix).c_str());
   if(!distance.data||distance.width!=SheetWidth||distance.height!=SheetHeight||
      distance.format!=PIXELFORMAT_UNCOMPRESSED_GRAYSCALE){
    if(distance.data)UnloadImage(distance);
    UnloadImage(image);return false;
   }
   const auto* encoded=static_cast<const unsigned char*>(distance.data);
   for(size_t k=0;k<p.alpha.size();k++)pixels[k].a=encoded[k];
   UnloadImage(distance);
  }
  for(size_t k=0;k<p.alpha.size();k++)p.alpha[k]=pixels[k].a;
  p.ink=LoadTextureFromImage(image);
  UnloadImage(image);
  if(!p.ink.id){
   if(smooth||i<BasePages)return false;
   p={};continue;
  }
  SetTextureFilter(p.ink,TEXTURE_FILTER_BILINEAR);
 }
 return true;
}

inline void load(){
 if(ready())return;
 const std::string root="assets/desert/jackal-turn/";
 const bool smooth=turn_warp::load(root);
 if(!loadPages(root,smooth)){
  unload();
  if(!smooth||!loadPages(root,false)){unload();return;}
 }
 flat=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
out vec4 finalColor;
void main(){finalColor=vec4(fragColor.rgb,fragColor.a*texture(texture0,fragTexCoord).a);}
)GLSL");
}

struct Cel {Facing facing;int frame;};
inline int key(float phase,int count){return std::clamp(int(std::lround(phase*(count-1))),0,count-1);}
struct Pose {float heading;int frame;int held;};
inline Pose pose(const Enemy& e,float fall){
 const auto& a=fall>=0&&e.deathMotion.facingInitialized?e.deathMotion:e.animation;
 const float heading=a.facingInitialized?a.facingAngle:facingAngles[int(a.direction)];
 if(fall>=0)return {heading,Walk+1+Attack+int(e.deathScreenSide()>=0)*Fall+key(fall,Fall),a.celFacing};
 const float attack=e.volcanoAction();
 if(attack>=0)return {heading,Walk+1+key(attack,Attack),a.celFacing};
 if((a.motionInitialized?a.moveBlend:float(a.moving))<.22f)return {heading,Walk,a.celFacing};
 const float phase=a.phase-std::floor(a.phase);
 return {heading,int(std::lround(phase*Walk))%Walk,a.celFacing};
}
inline Cel celAt(Facing facing,int frame){
 if(facing.mirrored){
  if(frame<Walk)frame=(frame+Walk/2)%Walk;
  else if(frame>=Walk+1+Attack)frame=Walk+1+Attack+(frame-(Walk+1+Attack)+Fall)%(2*Fall);
 }
 return {facing,frame};
}
struct Turn {Cel a,b;int direction,frame;float amount;};
inline float seamBlend(float t){
 // Soften the edge of the two optical fields with a nearly uniform rate.
 constexpr float edge=.06f;
 const bool reverse=t>.5f;
 const float u=(reverse?1.f-t:t)/edge;
 if(u>=1)return t;
 const float eased=edge*u*(.85f+.15f*u*(2-u));
 return reverse?1.f-eased:eased;
}
inline Turn interpolate(Pose p){
 const float yaw=std::atan2(std::sin(p.heading)/ActorGroundSquash,std::cos(p.heading));
 const float track=yaw/desert_motion::JackalFacingStep;
 const float step=std::floor(track);
 const int from=(int(step)+desert_motion::JackalFacings)%desert_motion::JackalFacings;
 const int to=(from+1)%desert_motion::JackalFacings;
 const float t=track-step;
 return {celAt(Facings[from],p.frame),celAt(Facings[to],p.frame),from,p.frame,turn_warp::hasSizeFits?seamBlend(t):t};
}
inline Cel select(const Enemy& e,float fall){
 const auto p=pose(e,fall);
 auto facing=Facings[desert_motion::jackalFacing(p.heading,p.held)];
 if(!pages[facing.page].ink.id)facing=Facings[2*desert_motion::paintedFacing(p.heading)];
 return celAt(facing,p.frame);
}

inline Rectangle tile(int frame){
 return {float((frame%Columns)*(Width+Padding)+1),float((frame/Columns)*(Height+Padding)+1),Width,Height};
}

inline float sample(const Page& page,Rectangle source,Vector2 point){
 if(point.x<0||point.x>=Width||point.y<0||point.y>=Height)return 0;
 const float x=source.x+point.x-.5f,y=source.y+point.y-.5f;
 const int tx=int(x),ty=int(y);
 const auto at=[&](int px,int py){return float(page.alpha[py*SheetWidth+px]);};
 const float a=caldera_quadruped::mix(at(tx,ty),at(tx+1,ty),x-tx);
 const float b=caldera_quadruped::mix(at(tx,ty+1),at(tx+1,ty+1),x-tx);
 return caldera_quadruped::mix(a,b,y-ty);
}
inline float distance(Cel cel,Vector2 point){
 if(point.x<0||point.x>=Width||point.y<0||point.y>=Height)return -16;
 if(cel.facing.mirrored)point.x=Width-point.x;
 return (sample(pages[cel.facing.page],tile(cel.frame),point)-128)*.125f;
}
inline float transitionCoverage(const Turn& view,Vector2 point){
 using namespace turn_warp;
 const float t=view.amount;
 const auto f=lookup(flow,fieldOrigin(view.direction,view.frame),point,GridW,GridH);
 const float a=distance(view.a,{point.x+t*(f[2]-128)*.5f,point.y+t*(f[3]-128)*.5f});
 const float b=distance(view.b,{point.x+(1-t)*(f[0]-128)*.5f,point.y+(1-t)*(f[1]-128)*.5f});
 return std::clamp(caldera_quadruped::mix(a,b,t)+.5f,0.f,1.f);
}

struct FitKey {int angle,next,phase;float blend;};
inline FitKey fitKey(const Turn& view){
 using namespace turn_warp;
 const float u=view.amount*4;
 const int step=int(u);
 const int i=(view.direction*4+step)%FitSamples;
 return {i,(i+1)%FitSamples,std::min(view.frame,Walk),u-step};
}
inline turn_warp::SizeFit sizeFit(const Turn& view,float fall){
 using caldera_quadruped::mix;
 using namespace turn_warp;
 if(!hasSizeFits)return {};
 const auto [i,next,phase,t]=fitKey(view);
 const auto a=sizeFits[i][phase],b=sizeFits[next][phase];
 const float fade=fall<0?1.f:1.f-fall*fall*(3.f-2.f*fall);
 return {mix(1.f,mix(a.width,b.width,t),fade),mix(1.f,mix(a.upper,b.upper,t),fade),
         mix(1.f,mix(a.lower,b.lower,t),fade),mix(a.offset,b.offset,t)*fade};
}

inline turn_warp::TorsoFit torsoFit(const Turn& view,float fall){
 using caldera_quadruped::mix;
 using namespace turn_warp;
 if(!hasTorsoFits)return {};
 const auto [i,next,phase,t]=fitKey(view);
 const auto& a=torsoFits[i][phase];
 const auto& b=torsoFits[next][phase];
 const auto ease=[](float t){t=std::clamp(t,0.f,1.f);return t*t*(3.f-2.f*t);};
 float weight=1;
 if(fall>=0)weight=1-ease(fall/.65f);
 else if(view.frame>Walk){
  const float action=float(view.frame-Walk-1)/(Attack-1);
  weight=1-ease(std::min(action,1-action)/.34f);
 }
 const auto span=[&](SpanFit a,SpanFit b){
  return SpanFit{mix(a.start,b.start,t),mix(a.end,b.end,t),mix(1.f,mix(a.gain,b.gain,t),weight)};
 };
 return {span(a.length,b.length),span(a.depth,b.depth)};
}

inline float mapSpan(turn_warp::SpanFit span,float p,bool inverse,bool chest){
 using namespace turn_warp;
 if(span.gain==1)return p;
 const float shift=(span.end-span.start)*(1-span.gain)*.5f;
 float from=span.start,to=span.end,left=from+shift,right=to-shift;
 if(inverse){std::swap(from,left);std::swap(to,right);}
 if(p<=from){
  if(!chest)return p+left-from;
  if(p<=ChestTop)return p;
  return ChestTop+(p-ChestTop)*(left-ChestTop)/(from-ChestTop);
 }
 if(p>=to){
  if(!chest)return p+right-to;
  if(p>=LowerJoint)return p;
  return LowerJoint+(p-LowerJoint)*(right-LowerJoint)/(to-LowerJoint);
 }
 return left+(p-from)*(right-left)/(to-from);
}
inline Vector2 mapBody(const turn_warp::TorsoFit& fit,Vector2 p,bool inverse=false){
 return {mapSpan(fit.length,p.x,inverse,false),mapSpan(fit.depth,p.y,inverse,true)};
}

struct Canvas {
 Rectangle rect;
 turn_warp::SizeFit fit;
 float line()const{
  using namespace turn_warp;
  return GroundPixel+(LowerJoint-GroundPixel)*fit.upper+fit.offset;
 }
 bool bends()const{return fit.upper!=fit.lower;}
 Vector2 project(Vector2 p)const{
  using namespace turn_warp;
  const float x=Width*.5f+(p.x-Width*.5f)*fit.width;
  const float y=line()+(p.y-LowerJoint)*(p.y<=LowerJoint?fit.upper:fit.lower);
  return {rect.x+x*rect.width/Width,rect.y+y*rect.height/Height};
 }
 Vector2 source(Vector2 p)const{
  using namespace turn_warp;
  const float x=(p.x-rect.x)*Width/rect.width,y=(p.y-rect.y)*Height/rect.height;
  const float d=y-line();
  return {Width*.5f+(x-Width*.5f)/fit.width,LowerJoint+d/(d<=0?fit.upper:fit.lower)};
 }
};

struct LimbQuad {Vector2 origin,horizontal,vertical;Rectangle uv;float light;};
inline turn_warp::Leg limb(Cel cel,int slot){
 const bool mirror=cel.facing.mirrored;
 auto leg=turn_warp::rigs[cel.facing.page][cel.frame][mirror?(slot^1):slot];
 if(mirror){for(auto& point:leg.joints)point.x=Width-point.x;leg.pitch=-leg.pitch;}
 return leg;
}
inline std::array<Vector2,4> registeredRoots(const Turn& view,const turn_warp::TorsoFit& fit){
 using caldera_quadruped::mix;
 std::array<Vector2,4> source,result;
 for(int slot=0;slot<4;slot++){
  source[slot]=mix(limb(view.a,slot).joints[0],limb(view.b,slot).joints[0],view.amount);
  result[slot]=mapBody(fit,source[slot]);
 }
 // In a three-quarter view the near and far roots can straddle the warp.
 // Keep their midpoint on the registered shoulder or hip station.
 for(int i:{0,2}){
  const auto centre=mix(source[i],source[i+1],.5f);
  const float mean=(result[i].x+result[i+1].x)*.5f;
  const float correction=mapBody(fit,centre).x-mean;
  result[i].x+=correction;result[i+1].x+=correction;
 }
 return result;
}
inline std::array<LimbQuad,16> limbQuads(const Turn& view,const turn_warp::TorsoFit& fit){
 using caldera_quadruped::add;
 using caldera_quadruped::mix;
 using caldera_quadruped::mul;
 using caldera_quadruped::sub;
 using caldera_quadruped::length;
 const float heading=(view.direction+view.amount)*desert_motion::JackalFacingStep;
 const float sideways=std::cos(heading),ahead=std::sin(heading);
 const auto depth=[&](int leg){return (leg>=2?1.f:-1.f)*ahead+(leg%2?-1.f:1.f)*sideways*.48f;};
 std::array<int,4> order{0,1,2,3};
 std::sort(order.begin(),order.end(),[&](int a,int b){
  const float offset=depth(a)-depth(b);
  return offset==0?a<b:offset<0;
 });
 std::array<LimbQuad,16> quads{};
 const auto roots=registeredRoots(view,fit);
 size_t index=0;
 for(int slot:order){
  const auto a=limb(view.a,slot),b=limb(view.b,slot);
  const float t=view.amount;
  std::array<Vector2,4> joints;
  for(size_t i=0;i<joints.size();i++)joints[i]=mix(a.joints[i],b.joints[i],t);
  // Move the socket with the torso and keep the planted toe on its gait path.
  const auto correction=sub(roots[slot],joints[0]);
  joints[0]=roots[slot];
  joints[1]=add(joints[1],mul(correction,.58f));
  joints[2]=add(joints[2],mul(correction,.20f));
  const float pitch=mix(a.pitch,b.pitch,t)*DEG2RAD;
  const float light=std::clamp(.88f+depth(slot)*.10f,.78f,1.0f);
  for(int i=0;i<4;i++){
   const auto& part=turn_warp::parts[(slot>=2?4:0)+i];
   const auto start=joints[i];
   const auto along=i<3?sub(joints[i+1],start):Vector2{};
   const float span=length(along);
   const auto axis=i==3?Vector2{std::sin(pitch),std::cos(pitch)}:span>.01f?mul(along,1/span):Vector2{0,1};
   const Vector2 right{axis.y,-axis.x};
   quads[index++]={sub(start,add(mul(axis,part.lead),mul(right,part.half))),
                   mul(right,part.half*2),mul(axis,span+part.lead+part.trail),part.uv,light};
  }
 }
 return quads;
}
inline bool containsLimb(const LimbQuad& quad,Vector2 point){
 using caldera_quadruped::cross;
 using caldera_quadruped::sub;
 const auto local=sub(point,quad.origin);
 const float area=cross(quad.horizontal,quad.vertical);
 if(std::abs(area)<.001f)return false;
 const float u=cross(local,quad.vertical)/area,v=cross(quad.horizontal,local)/area;
 if(u<0||u>=1||v<0||v>=1)return false;
 const auto& tex=turn_warp::limbInk;
 const float x=quad.uv.x+u*quad.uv.width-.5f,y=quad.uv.y+v*quad.uv.height-.5f;
 const int ix=int(x),iy=int(y);
 const auto* p=tex.colors.data()+iy*tex.texture.width+ix;
 const float a=caldera_quadruped::mix(float(p[0].a),float(p[1].a),x-ix);
 const float b=caldera_quadruped::mix(float(p[tex.texture.width].a),float(p[tex.texture.width+1].a),x-ix);
 return caldera_quadruped::mix(a,b,y-iy)>=105;
}
struct LimbVertex {Vector2 p,uv;};
inline void paintLimbs(const std::array<LimbQuad,16>& limbs,Canvas canvas,Color tint,bool shaded){
 const auto& tex=turn_warp::limbInk.texture;
 const auto vertex=[&](LimbVertex v){
  const auto p=canvas.project(v.p);
  rlTexCoord2f(v.uv.x,v.uv.y);rlVertex2f(p.x,p.y);
 };
 const auto fan=[&](const auto& polygon,int count){
  for(int i=1;i<count-1;i++){vertex(polygon[0]);vertex(polygon[i]);vertex(polygon[i+1]);}
 };
 using caldera_quadruped::add;
 using caldera_quadruped::mix;
 const float seam=turn_warp::LowerJoint;
 rlCheckRenderBatchLimit(int(limbs.size())*18);
 // Switching the primitive type in rlBegin resets the batch texture.
 rlBegin(RL_TRIANGLES);rlSetTexture(tex.id);
 for(const auto& q:limbs){
  const float light=shaded?q.light:1.f;
  rlColor4ub(tint.r*light,tint.g*light,tint.b*light,tint.a);
  const float l=q.uv.x/tex.width,r=(q.uv.x+q.uv.width)/tex.width;
  const float top=q.uv.y/tex.height,bottom=(q.uv.y+q.uv.height)/tex.height;
  const auto end=add(q.origin,q.vertical);
  const std::array<LimbVertex,4> corners{{{q.origin,{l,top}},{end,{l,bottom}},
                                         {add(end,q.horizontal),{r,bottom}},{add(q.origin,q.horizontal),{r,top}}}};
  const int above=int(std::count_if(corners.begin(),corners.end(),[&](const auto& v){return v.p.y<seam;}));
  if(!canvas.bends()||above==0||above==4){fan(corners,4);continue;}
  // Split only segments crossing the height-registration line, so the mask
  // and the rendered joint use exactly the same transform.
  for(bool upper:{true,false}){
   std::array<LimbVertex,6> part;
   int count=0;
   for(size_t i=0;i<corners.size();i++){
    const auto a=corners[i],b=corners[(i+1)%corners.size()];
    const bool inside=(a.p.y<seam)==upper;
    const bool next=(b.p.y<seam)==upper;
    if(inside)part[count++]=a;
    if(inside!=next){
     const float t=(seam-a.p.y)/(b.p.y-a.p.y);
     part[count++]={mix(a.p,b.p,t),mix(a.uv,b.uv,t)};
    }
   }
   fan(part,count);
  }
 }
 rlEnd();rlSetTexture(0);
}

inline void transitionPass(const Turn& view,Canvas canvas,Color ink,int mode,const turn_warp::TorsoFit& fit={}){
 using namespace turn_warp;
 const auto a=tile(view.a.frame),b=tile(view.b.frame);
 const Vector4 cels{a.x,a.y,b.x,b.y};
 const Vector4 yaw{view.amount,float(view.a.facing.mirrored),float(view.b.facing.mirrored),0};
 const auto field=fieldOrigin(view.direction,view.frame);
 BeginShaderMode(shader);
 SetShaderValueTexture(shader,uniforms.other,pages[view.b.facing.page].ink);
 SetShaderValueTexture(shader,uniforms.fields,flow.texture);
 SetShaderValue(shader,uniforms.cels,&cels,SHADER_UNIFORM_VEC4);
 SetShaderValue(shader,uniforms.angles,&yaw,SHADER_UNIFORM_VEC4);
 SetShaderValue(shader,uniforms.flowTile,&field,SHADER_UNIFORM_VEC2);
 SetShaderValue(shader,uniforms.mode,&mode,SHADER_UNIFORM_INT);
 const bool depth=fit.depth.gain!=1,length=fit.length.gain!=1;
 const std::array<float,4> xs{0,length?fit.length.start:float(Width),fit.length.end,Width};
 const std::array<float,6> ys{0,depth?ChestTop:(canvas.bends()?LowerJoint:float(Height)),
                             depth?fit.depth.start:float(Height),fit.depth.end,LowerJoint,Height};
 const int columns=length?3:1,rows=depth?5:(canvas.bends()?2:1);
 const auto quad=[&](Vector2 offset){
  rlCheckRenderBatchLimit(columns*rows*4);
  rlBegin(RL_QUADS);rlSetTexture(pages[view.a.facing.page].ink.id);
  rlColor4ub(ink.r,ink.g,ink.b,ink.a);
  for(int i=0;i<rows;i++)for(int j=0;j<columns;j++){
   const float left=xs[j],right=xs[j+1],top=ys[i],bottom=ys[i+1];
   const auto first=canvas.project(mapBody(fit,{left,top})),last=canvas.project(mapBody(fit,{right,bottom}));
   const float x=first.x+offset.x,y=first.y+offset.y,r=last.x+offset.x,b=last.y+offset.y;
   rlTexCoord2f(left/Width,top/Height);rlVertex2f(x,y);
   rlTexCoord2f(left/Width,bottom/Height);rlVertex2f(x,b);
   rlTexCoord2f(right/Width,bottom/Height);rlVertex2f(r,b);
   rlTexCoord2f(right/Width,top/Height);rlVertex2f(r,y);
  }
  rlEnd();rlSetTexture(0);
 };
 if(mode==1)for(Vector2 d:{Vector2{1.3f,0},Vector2{-1.3f,0},Vector2{0,1.3f},Vector2{0,-1.3f}})quad(d);
 else quad({});
 EndShaderMode();
}

inline bool draw(const Enemy& e,float x,float y,Color outline,const Vector2* probe,bool shadow,
                 float scale,float opacity,float fall){
 const Rectangle dest{x-AnchorX*scale,y-AnchorY*scale,WorldWidth*scale,WorldHeight*scale};
 if(turn_warp::active){
  const auto view=interpolate(pose(e,fall));
  const Canvas canvas{dest,sizeFit(view,fall)};
  const auto fit=torsoFit(view,fall);
  const auto limbs=limbQuads(view,fit);
  if(probe){
   const Vector2 point=canvas.source(*probe);
   if(opacity<.43f)return false;
   const Vector2 body=mapBody(fit,point,true);
   const bool inside=body.x>=0&&body.x<Width&&body.y>=0&&body.y<Height;
   return (inside&&transitionCoverage(view,body)>=105.f/255)||
          std::any_of(limbs.begin(),limbs.end(),[&](const auto& q){return containsLimb(q,point);});
  }
  if(outline.a&&!shadow){
   const auto tint=Fade(outline,opacity);
   if(flat.id&&flat.id!=rlGetShaderIdDefault()){
    BeginShaderMode(flat);
    for(Vector2 d:{Vector2{1.3f,0},Vector2{-1.3f,0},Vector2{0,1.3f},Vector2{0,-1.3f}})
     paintLimbs(limbs,{{dest.x+d.x,dest.y+d.y,dest.width,dest.height},canvas.fit},tint,false);
    EndShaderMode();
   }
   transitionPass(view,canvas,tint,1,fit);
  }
  paintLimbs(limbs,canvas,Fade(WHITE,opacity),!shadow);
  transitionPass(view,canvas,Fade(WHITE,opacity),shadow?2:0,fit);
  if(shadow&&world_shadows::maskShader.id)BeginShaderMode(world_shadows::maskShader);
  return false;
 }
 const auto cel=select(e,fall);
 const auto& page=pages[cel.facing.page];
 auto source=tile(cel.frame);
 if(probe){
  const float x=(probe->x-dest.x)/dest.width;
  return opacity>=.43f&&sample(page,source,{(cel.facing.mirrored?1-x:x)*Width,(probe->y-dest.y)*Height/dest.height})>=105;
 }
 if(cel.facing.mirrored)source.width=-source.width;
 if(outline.a&&!shadow&&flat.id&&flat.id!=rlGetShaderIdDefault()){
  BeginShaderMode(flat);
  for(Vector2 d:{Vector2{1.3f,0},Vector2{-1.3f,0},Vector2{0,1.3f},Vector2{0,-1.3f}})
   DrawTexturePro(page.ink,source,{dest.x+d.x,dest.y+d.y,dest.width,dest.height},{},0,Fade(outline,opacity));
  EndShaderMode();
 }
 DrawTexturePro(page.ink,source,dest,{},0,Fade(WHITE,opacity));
 return false;
}
}
