#pragma once
#include "generated/caldera_puppet_motion.hpp"
#include "generated/fire_imp_synfig.hpp"

namespace world_shadows { extern Shader maskShader; }

namespace caldera_puppets {
constexpr int Cell=160,Columns=3,Rows=4;
constexpr int CoreWidth=84,CoreHeight=116,CoreColumns=24,CoreRows=15,CoreViews=CoreColumns*CoreRows;
constexpr int FirstCore=Columns*Rows;
enum Species {Imp,Wraith};
namespace imp_render { inline void load(); inline void unload(); }

inline std::vector<Vector2> alphaHull(const std::vector<unsigned char>& alpha,int stride,int x,int y,int w,int h){
 std::vector<Vector2> ends;
 for(int col=0;col<w;++col){
  int top=h,bottom=-1;
  for(int row=0;row<h;++row)if(alpha[(y+row)*stride+x+col]>=128){
   top=std::min(top,row);bottom=row;
  }
  if(bottom<0)continue;
  ends.push_back({(col+.5f)/w,(top+.5f)/h});
  if(bottom!=top)ends.push_back({(col+.5f)/w,(bottom+.5f)/h});
 }
 if(ends.size()<3)return ends;
 std::vector<Vector2> hull;
 const auto append=[&](auto first,auto last){
  const size_t start=hull.size();
  for(auto p=first;p!=last;++p){
   while(hull.size()>=start+2){
    const auto a=hull[hull.size()-2],b=hull.back();
    if((b.x-a.x)*(p->y-a.y)-(b.y-a.y)*(p->x-a.x)>0)break;
    hull.pop_back();
   }
   hull.push_back(*p);
  }
  hull.pop_back();
 };
 append(ends.begin(),ends.end());
 append(ends.rbegin(),ends.rend());
 return hull;
}

struct Artwork {
 Texture2D texture{};
 std::vector<unsigned char> alpha;
 std::vector<std::vector<Vector2>> support;
 int cell=Cell,height=Cell,columns=Columns;
 void load(const char* path,int cellSize=Cell,int cols=Columns,int rows=Rows,int cellHeight=Cell){
  if(texture.id)return;
  Image image=LoadImage(path);
  if(!image.data)return;
  if(image.width==cols*cellSize&&image.height==rows*cellHeight){
   cell=cellSize;height=cellHeight;columns=cols;
   ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
   const auto* pixels=static_cast<const Color*>(image.data);
   alpha.resize(image.width*image.height);
   for(size_t i=0;i<alpha.size();++i)alpha[i]=pixels[i].a;
   support.resize(cols*rows);
   for(int n=0;n<cols*rows;n++){
    const int ox=n%cols*cell+1,oy=n/cols*height+1;
    support[n]=alphaHull(alpha,image.width,ox,oy,cell-2,height-2);
   }
   texture=LoadTextureFromImage(image);
   SetTextureFilter(texture,TEXTURE_FILTER_BILINEAR);
  }
  UnloadImage(image);
 }
 Rectangle tile(int index)const{
  return {float(index%columns*cell+1),float(index/columns*height+1),float(cell-2),float(height-2)};
 }
 void unload(){if(texture.id)UnloadTexture(texture);*this={};}
};

inline std::array<Artwork,2> artwork;
inline Artwork impCore;
inline Shader solidColor{};
inline void load(){
 artwork[Imp].load("assets/volcano/fire-imp-puppet.png");
 artwork[Wraith].load("assets/volcano/ash-wraith-puppet.png");
 impCore.load("assets/volcano/fire-imp-core.png",CoreWidth,CoreColumns,CoreRows,CoreHeight);
 if(solidColor.id)return;
 constexpr const char* fragment=R"GLSL(#version 330
 in vec2 fragTexCoord;
 in vec4 fragColor;
 out vec4 finalColor;
 uniform sampler2D texture0;
 void main(){finalColor=vec4(fragColor.rgb,fragColor.a*texture(texture0,fragTexCoord).a);}
 )GLSL";
 solidColor=LoadShaderFromMemory(nullptr,fragment);
 imp_render::load();
}
inline void unload(){
 for(auto& art:artwork)art.unload();
 impCore.unload();
 if(solidColor.id&&solidColor.id!=rlGetShaderIdDefault())UnloadShader(solidColor);
 imp_render::unload();
 solidColor={};
}
inline bool ready(Species s){return artwork[s].texture.id!=0;}

inline Vector2 add(Vector2 a,Vector2 b){return {a.x+b.x,a.y+b.y};}
inline Vector2 sub(Vector2 a,Vector2 b){return {a.x-b.x,a.y-b.y};}
inline Vector2 mul(Vector2 a,float s){return {a.x*s,a.y*s};}
inline float mix(float a,float b,float t){return a+(b-a)*t;}
inline Vector2 mix(Vector2 a,Vector2 b,float t){return add(a,mul(sub(b,a),t));}
inline float ease(float p){p=std::clamp(p,0.f,1.f);return p*p*(3-2*p);}
inline float between(float p,float a,float b){return ease((p-a)/(b-a));}
inline Vector2 rotate(Vector2 p,float radians){
 const float c=std::cos(radians),s=std::sin(radians);
 return {c*p.x-s*p.y,s*p.x+c*p.y};
}

// These are painted cutouts on a 2D rig. Facing moves their attachment points
// and changes which cutout is in front. Artwork UVs never depend on heading.
struct Bearing {
 float x,y;
 static float angle(const AnimationState& state){
  if(state.facingInitialized)return state.facingAngle;
  const float axis=int(state.direction)*PI/4;
  return std::atan2(.5f*std::sin(axis),std::cos(axis));
 }
 explicit Bearing(const AnimationState& state):Bearing(angle(state)){}
 explicit Bearing(float angle):x(actorFacing(angle).x),y(actorFacing(angle).y){}
 Vector2 at(float side,float front,float height)const{
  return {front*x-side*y,(front*y+side*x)*.52f-height};
 }
};

struct Piece {
 Vector2 center,half;
 float angle,depth;
 int tile;
 bool mirror;
 float light;
};
struct Pose {
 std::array<Piece,22> pieces{};
 size_t count=0;
 Vector2 palms[2]{};
 void plate(int cell,Vector2 center,float w,float h,float angle,float depth,bool mirror=false,float light=1){
  if(w>.08f&&h>.08f)pieces[count++]={center,{w*.5f,h*.5f},angle,depth,cell,mirror,light};
 }
 void joint(int cell,Vector2 start,Vector2 end,float width,float overlap,float depth,bool mirror=false,float light=1){
  const auto d=sub(end,start);
  plate(cell,mix(start,end,.5f),width,std::hypot(d.x,d.y)+overlap,std::atan2(-d.x,d.y),depth,mirror,light);
 }
};

struct Gesture {float crouch,gather,throwing,unwind;};
template<size_t N>
inline float track(const motion::Waypoint (&keys)[N],float time){
 if(time<=0)return keys[0].value;
 if(time>=1)return keys[N-1].value;
 const auto* end=std::upper_bound(keys+1,keys+N-1,time,[](float t,const auto& key){return t<key.time;});
 return mix(end[-1].value,end->value,between(time,end[-1].time,end->time));
}
inline float cycle(float time){return time-std::floor(time);}
template<size_t N>
inline float synfigTrack(const ember::fire_imp_synfig::Waypoint (&keys)[N],float time){
 if(time<=0)return keys[0].value;
 if(time>=1)return keys[N-1].value;
 const auto* end=std::upper_bound(keys+1,keys+N-1,time,[](float t,const auto& key){return t<key.time;});
 return mix(end[-1].value,end->value,(time-end[-1].time)/(end->time-end[-1].time));
}
inline Gesture gesture(Species species,float action){
 using namespace motion;
 if(species==Imp){
  using namespace ember::fire_imp_synfig;
  return {synfigTrack(cast_crouch,action),synfigTrack(cast_gather,action),
          synfigTrack(cast_thrust,action),synfigTrack(cast_release,action)};
 }
 return {track(wraithCast_crouch,action),track(wraithCast_gather,action),track(wraithCast_throwing,action),track(wraithCast_unwind,action)};
}

// The two landmark pixels of each part sit exactly on the pose's shared
// joint positions. Mirroring changes the paint coordinates of both sockets.
inline void impSegment(Pose& pose,int tile,Vector2 a,Vector2 b,Vector2 start,Vector2 end,
                       float width,float depth,bool mirror=false,float light=1){
 if(mirror){start.x=1-start.x;end.x=1-end.x;}
 const auto axis=sub(b,a);
 const float length=std::max(1.f,std::hypot(axis.x,axis.y));
 const float across=end.x-start.x;
 width=std::min(width,length*.89f/std::max(.01f,std::abs(across)));
 const float dx=across*width;
 const float height=std::sqrt(length*length-dx*dx)/(end.y-start.y);
 const float angle=std::atan2(axis.y,axis.x)-std::atan2((end.y-start.y)*height,dx);
 const auto center=sub(a,rotate({(start.x-.5f)*width,(start.y-.5f)*height},angle));
 pose.plate(tile,center,width,height,angle,depth,mirror,light);
}

inline Pose fireImp(const AnimationState& state,float action){
 using namespace ember::fire_imp_synfig;
 const int view=(int(std::lround(std::remainder(Bearing::angle(state),2*PI)*RAD2DEG))+CoreViews)%CoreViews;
 const Bearing heading(view*DEG2RAD);
 const auto cast=gesture(Imp,action);
 const float engaged=std::clamp(cast.gather+.8f*cast.throwing+.38f*cast.unwind,0.f,1.f);
 const float pace=state.moveBlend*(1-engaged);
 const float pivot=(1-pace)*std::abs(state.turnLean)*(1-engaged);
 const float tread=cycle(state.phase),breath=cycle(.72f*state.idleTime);
 const float shift=synfigTrack(turn_shift,cycle(state.pivotPhase));
 const float sway=synfigTrack(walk_weight,tread)*pace+shift*state.turnLean*(1-pace)*(1-engaged);
 const float hip=32.3f-5.2f*cast.crouch+synfigTrack(walk_hip,tread)*pace
                +synfigTrack(idle_breath,breath)*(1-engaged)*(1-pace);
 const auto pelvis=heading.at(sway,0,hip);
 const float lean=synfigTrack(idle_settle,breath)*(1-pace)*(1-engaged)
                  +2.4f*pace-4.4f*cast.gather+7.1f*cast.throwing-2.2f*cast.unwind;
 const float pitch=.032f*state.turnLean*(1-engaged)+std::atan2(heading.x*lean,27.f);
 const auto socket=[&](float side,float forward,float height){
  return add(pelvis,rotate(heading.at(side,forward,height),pitch));
 };

 Pose result;
 if(impCore.texture.id){
  constexpr float exportScale=1.5f;
  result.plate(FirstCore+view,add(pelvis,rotate({0,(CoreHeight*.5f-101)/exportScale},pitch)),
               (CoreWidth-2)/exportScale,(CoreHeight-2)/exportScale,pitch,0);
 }else{
  result.plate(heading.y>0?3:10,socket(0,0,14),28,39,pitch,0);
  result.plate(0,socket(0,2,37),24,23,pitch,.3f);
 }
 const float tailSwing=synfigTrack(walk_tail,tread)*pace+synfigTrack(idle_tail,breath)*(1-pace)*(1-engaged)
                       +synfigTrack(cast_tail,action);
 impSegment(result,9,socket(0,-2.6f,-1.2f),heading.at(tailSwing,-30,13+tailSwing*.34f),
            {.16f,.12f},{.90f,.92f},25,-31,false,.91f);

 for(int i=0;i<2;i++){
  const float side=i==0?-1.f:1.f;
  const float step=cycle(tread+i*.5f),turnStep=cycle(state.pivotPhase+i*.5f);
  const float reach=synfigTrack(walk_reach,step)*pace+synfigTrack(turn_toe,turnStep)*pivot;
  const float lift=synfigTrack(walk_lift,step)*pace+synfigTrack(turn_lift,turnStep)*pivot;
  const float light=.90f+.10f*ease(.5f+.5f*side*heading.x);
  const auto thigh=socket(side*6.4f,.8f,1.7f);
  const auto knee=heading.at(side*8.4f,3.5f+reach*.62f,15+lift*.62f-2.1f*cast.crouch);
  const auto hoof=heading.at(side*8.7f,reach-1.3f,2.7f+lift);
  const float legDepth=-7+side*heading.x*6+reach*heading.y*.15f;
  impSegment(result,6,thigh,knee,{.51f,.19f},{.54f,.84f},14,legDepth,i==0,light);
  impSegment(result,7,knee,hoof,{.48f,.18f},{.51f,.91f},11,legDepth+.1f,i==0,light);

  const float handReach=4.5f+synfigTrack(walk_arm,step)*pace+(10+side*5.8f*std::abs(heading.x))*cast.gather+24*cast.throwing;
  const auto shoulder=socket(side*11.4f,-.4f,21.5f);
  const auto hand=socket(side*(17.3f-8.5f*cast.gather-8.3f*cast.throwing),handReach,
                        -6.3f+15.5f*cast.gather+23*cast.throwing-3.5f*cast.unwind);
  const auto freeElbow=add(mix(shoulder,hand,.53f),heading.at(side*3,-2,-.8f));
  const auto bracedElbow=socket(side*17,handReach*.36f-1,2.8f+cast.throwing*10);
  const auto elbow=mix(freeElbow,bracedElbow,cast.gather);
  const float armDepth=side*heading.x*15+heading.y*(3+handReach*.52f);
  impSegment(result,4,shoulder,elbow,{.56f,.20f},{.46f,.83f},14,armDepth,i==0,light);
  impSegment(result,5,elbow,hand,{.38f,.17f},{.54f,.71f},13,armDepth+.1f,i==0,light);
  result.palms[i]=hand;
 }
 return result;
}

inline Pose ashWraith(const AnimationState& state,float action){
 const Bearing heading(state);
 const auto cast=gesture(Wraith,action);
 const float time=state.idleTime;
 const float strength=std::max(cast.gather,cast.throwing);
 const float wind=track(motion::wraithFlight_hem,cycle(time*.64f+state.phase));
 const float wave=track(motion::wraithFlight_wave,cycle(time*.59f));
 const float shoulderHeight=63+3.1f*wave*(1-strength)-3*cast.crouch;
 const float drift=state.moveBlend*(1-strength)*10;
 const float lean=drift-7*cast.gather+11*cast.throwing;
 const auto mantle=heading.at(0,lean,shoulderHeight);
 const auto hem=heading.at(-2+3.2f*wind,-drift-7,15+wind*2.2f);
 Pose result;
 result.joint(4,add(mantle,{0,5}),add(hem,{0,14}),43,16,-9,false,.88f);
 for(int i=0;i<2;i++){
  const float side=i==0?-1.f:1.f;
  const float ripple=std::sin(time*4.9f+side*.65f);
  const float depth=side*heading.x*16;
  const auto waist=heading.at(side*8,lean-2,shoulderHeight-14);
  const auto strip=add(hem,heading.at(side*(10+3*ripple),4*ripple,-5));
  result.joint(5,waist,strip,21,7,depth-3,i>0,.86f+.10f*ease(.5f+.5f*side*heading.x));
  const auto shoulder=heading.at(side*15,lean-2,shoulderHeight);
  const float reach=5+6*drift*.2f+10*cast.gather+cast.throwing*(i==1?29:14);
  const auto cuff=heading.at(side*(20-7*cast.gather+3*cast.throwing),reach,
                             32+5*ripple*(1-strength)+9*cast.gather+17*cast.throwing-3*cast.unwind);
  const auto elbow=add(mix(shoulder,cuff,.51f),heading.at(side*4,-3,2));
  result.joint(6,shoulder,elbow,17,11,depth,i>0,.96f);
  result.joint(7,elbow,cuff,20,12,depth+.5f+heading.y*cast.throwing*8,i>0,.95f);
  result.palms[i]=cuff;
 }
 const auto chest=add(mantle,heading.at(0,3,-13));
 result.joint(3,chest,add(hem,heading.at(0,3,1)),26+5*std::abs(heading.y),9,heading.y*5,false,.96f);
 result.plate(2,add(mantle,{0,5}),37+8*std::abs(heading.y),24,-.15f*heading.x*drift/10,5);
 const auto hood=heading.at(0,lean+1,shoulderHeight+11);
 result.plate(0,hood,25,28,-.14f*heading.x*(1+cast.gather),24);
 const auto opening=add(hood,heading.at(0,8,-3));
 const float frontal=ease((heading.y+.20f)/.86f);
 result.plate(1,opening,22*frontal,23,-.13f*heading.x,25);
 result.plate(8,add(mantle,heading.at(0,8,-9)),6*frontal,6,0,20);
 return result;
}

inline Pose living(Species type,const AnimationState& state,float action){
 return type==Imp?fireImp(state,action):ashWraith(state,action);
}

inline const std::vector<Vector2>& impHull(const Piece& part){
 const bool core=part.tile>=FirstCore;
 const auto& sheet=core?impCore:artwork[Imp];
 return sheet.support[core?part.tile-FirstCore:part.tile];
}

inline Vector2 impContourPoint(const Piece& part,Vector2 uv,float c,float s){
 const float x=(uv.x-.5f)*2*part.half.x*(part.mirror?-1:1);
 const float y=(uv.y-.5f)*2*part.half.y;
 return {part.center.x+c*x-s*y,part.center.y+s*x+c*y};
}

inline float supportEdge(const Piece& part){
 const float c=std::cos(part.angle),s=std::sin(part.angle);
 float edge=-1.e6f;
 for(auto uv:impHull(part))edge=std::max(edge,impContourPoint(part,uv,c,s).y);
 return edge;
}

inline Pose fall(Species type,const AnimationState& state,float action,float phase,float side){
 using namespace ember::fire_imp_synfig;
 const float release=type==Imp?synfigTrack(fall_relax,phase):between(phase,.20f,.65f);
 auto pose=living(type,state,release>=1?-1.f:action);
 // The loaded corpse only has the ordinary animation state. Release the
 // captured casting pose before touchdown so reloading reproduces that pose.
 if(action>=0&&release>0&&release<1){
  const auto relaxed=living(type,state,-1);
  for(size_t i=0;i<pose.count;++i){
   auto& part=pose.pieces[i];
   const auto& end=relaxed.pieces[i];
   part.center=mix(part.center,end.center,release);
   part.half=mix(part.half,end.half,release);
   part.angle+=std::remainder(end.angle-part.angle,2*PI)*release;
   part.depth=mix(part.depth,end.depth,release);
  }
 }
 const float buckles=type==Imp?synfigTrack(fall_buckle,phase):track(motion::wraithFall_buckles,phase);
 const float topples=type==Imp?synfigTrack(fall_roll,phase):track(motion::wraithFall_topples,phase);
 const float landed=type==Imp?synfigTrack(fall_land,phase):track(motion::wraithFall_landed,phase);
 const float loss=type==Imp?synfigTrack(fall_recoil,phase):track(motion::wraithFall_recoil,phase);
 if(type==Imp){
  if(!artwork[Imp].texture.id)return pose;
  const float roll=side*1.43f*topples;
  const Vector2 pivot{side*4.f,-9.f};
  const Vector2 tuck{side*(loss*2.4f),5.1f*buckles*(1-topples)};
  float ground=-1.e6f;
  for(size_t i=0;i<pose.count;++i){
   auto& part=pose.pieces[i];
   if(part.tile==9){
    const Vector2 rootOffset{(.16f-.5f)*2*part.half.x,(.12f-.5f)*2*part.half.y};
    const auto root=add(part.center,rotate(rootOffset,part.angle));
    const float curl=PI*.5f-std::atan2(.80f*part.half.y,.74f*part.half.x);
    part.angle+=std::remainder(curl-part.angle,2*PI)*synfigTrack(fall_curl,phase);
    part.center=sub(root,rotate(rootOffset,part.angle));
   }
   part.center=add(add(pivot,tuck),rotate(sub(part.center,pivot),roll));
   part.angle+=roll;
   part.light*=1-.13f*landed;
   ground=std::max(ground,supportEdge(part));
  }
  for(size_t i=0;i<pose.count;++i)pose.pieces[i].center.y+=(4.5f-ground)*landed;
  return pose;
 }
 for(size_t i=0;i<pose.count;++i){
  auto& part=pose.pieces[i];
  const Vector2 start=part.center;
   const float cloth=between(topples,.11f+std::clamp(-start.y/245.f,0.f,.34f),.89f);
   const auto folded=rotate({start.x*.91f,start.y*.45f},side*(1.12f+.35f*std::sin(part.tile)));
   const float height=std::min(14.f,part.half.x*.42f+std::abs(start.x)*.17f);
   part.center=mix(start,{folded.x,-height+folded.y*.16f},cloth);
   part.center.y+=3*buckles*(1-cloth);
   part.center.x+=side*loss*(1-cloth)*std::sin(float(part.tile));
   part.angle+=side*cloth*(1.04f+.46f*std::sin(part.tile));
   part.half.y*=1-.32f*landed;
   part.light*=1-.18f*landed;
 }
 return pose;
}

inline Rectangle impContact(const AnimationState& state,float side){
 const auto pose=fall(Imp,state,-1,1,side);
 const auto& trunk=pose.pieces[0];
 const float centerY=supportEdge(trunk)-8;
 float left=1.e6f,right=-1.e6f;
 for(size_t i=0;i<pose.count;++i){
  const auto& part=pose.pieces[i];
  const auto& hull=impHull(part);
  if(hull.empty())continue;
  const float c=std::cos(part.angle),s=std::sin(part.angle);
  auto before=impContourPoint(part,hull.back(),c,s);
  for(auto uv:hull){
   const auto point=impContourPoint(part,uv,c,s);
   if((before.y<centerY)!=(point.y<centerY)){
    const float across=mix(before.x,point.x,(centerY-before.y)/(point.y-before.y));
    left=std::min(left,across);right=std::max(right,across);
   }
   before=point;
  }
 }
 if(left>right){left=trunk.center.x-20;right=trunk.center.x+20;}
 return {left-3,centerY-6.5f,right-left+6,13};
}

inline Vector2 muzzle(Species type,const AnimationState& state,float action){
 const auto pose=living(type,state,action);
 return type==Imp?mix(pose.palms[0],pose.palms[1],.5f):pose.palms[1];
}
inline Vector2 muzzleAtHeading(Species type,float angle){
 AnimationState state;state.facingAngle=angle;state.facingInitialized=true;
 return muzzle(type,state,.45f);
}

struct Sprite {const Artwork& art;Rectangle source;};
inline Sprite spriteFor(const Artwork& body,int tile){
 if(tile>=FirstCore)return {impCore,impCore.tile(tile-FirstCore)};
 return {body,body.tile(tile)};
}

inline bool opaque(Sprite sprite,const Piece& piece,Vector2 point){
 const auto local=rotate(sub(point,piece.center),-piece.angle);
 float u=.5f+.5f*local.x/piece.half.x;
 const float v=.5f+.5f*local.y/piece.half.y;
 if(u<0||u>=1||v<0||v>=1)return false;
 if(piece.mirror)u=1-u;
 const auto rect=sprite.source;
 const int x=int(rect.x)+std::min(int(rect.width)-1,int(u*rect.width));
 const int y=int(rect.y)+std::min(int(rect.height)-1,int(v*rect.height));
 return sprite.art.alpha[y*sprite.art.texture.width+x]>=100;
}

inline void drawPiece(Sprite sprite,const Piece& piece,Vector2 anchor,float size,bool shadow,Color color){
 const Vector2 half=piece.half;
 const auto point=[&](float x,float y){
  auto p=add(piece.center,rotate({x,y},piece.angle));
  if(shadow)p={p.x-shadow_math::LightX*p.y,-shadow_math::LightY*p.y};
  return add(anchor,mul(p,size));
 };
 const Vector2 quad[]{point(-half.x,-half.y),point(-half.x,half.y),point(half.x,half.y),point(half.x,-half.y)};
 const auto rect=sprite.source;
 const float w=float(sprite.art.texture.width),h=float(sprite.art.texture.height);
 float left=rect.x/w,right=left+rect.width/w;
 const float top=rect.y/h,bottom=top+rect.height/h;
 if(piece.mirror)std::swap(left,right);
 const Vector2 uv[]{{left,top},{left,bottom},{right,bottom},{right,top}};
 rlColor4ub(color.r,color.g,color.b,color.a);
 for(int i=0;i<4;i++){
  const int n=shadow?(4-i)%4:i;
  rlTexCoord2f(uv[n].x,uv[n].y);
  rlVertex2f(quad[n].x,quad[n].y);
 }
}

#include "fire_imp_composite.hpp"

inline bool draw(Species type,Pose pose,float x,float y,float size,Color outline,const Vector2* probe,bool shadow,bool flash,float opacity){
 const auto& art=artwork[type];
 if(opacity<=0||size<=0)return false;
 const auto end=pose.pieces.begin()+pose.count;
 if(probe){
  const Vector2 local{(probe->x-x)/size,(probe->y-y)/size};
  return std::any_of(pose.pieces.begin(),end,[&](const Piece& piece){return opaque(spriteFor(art,piece.tile),piece,local);});
 }
 for(auto piece=pose.pieces.begin()+1;piece<end;++piece){
  const auto current=*piece;
  auto place=piece;
  for(;place!=pose.pieces.begin()&&current.depth<(place-1)->depth;--place)*place=*(place-1);
  *place=current;
 }
 const auto alpha=static_cast<unsigned char>(std::lround(255*std::clamp(opacity,0.f,1.f)));
 if(type==Imp&&imp_render::ready()){
  imp_render::draw(pose,{x,y},size,outline,shadow,flash,alpha,opacity);
  return false;
 }
 const auto pass=[&](Vector2 point,Color color,bool paint){
  rlCheckRenderBatchLimit(int(pose.count)*4);
  unsigned int currentTexture=0;
  for(auto p=pose.pieces.begin();p!=end;++p){
   const auto sprite=spriteFor(art,p->tile);
   if(currentTexture!=sprite.art.texture.id){
    if(currentTexture)rlEnd();
    currentTexture=sprite.art.texture.id;
    rlSetTexture(currentTexture);rlBegin(RL_QUADS);
   }
   if(paint){const auto tone=static_cast<unsigned char>(255*p->light);color={tone,tone,tone,alpha};}
   drawPiece(sprite,*p,point,size,shadow,color);
  }
  if(currentTexture)rlEnd();
  rlSetTexture(0);
 };
 if(shadow){pass({x,y},{255,255,255,alpha},false);return false;}
 // The outline uses the same alpha texture as the colour, with RGB supplied by the shader.
 const bool canOutline=solidColor.id&&solidColor.id!=rlGetShaderIdDefault();
 if(outline.a&&canOutline){
  BeginShaderMode(solidColor);
  outline.a=static_cast<unsigned char>(outline.a*opacity);
  for(const auto offset:{Vector2{-1.5f,0},Vector2{1.5f,0},Vector2{0,-1.5f},Vector2{0,1.5f}})pass(add({x,y},offset),outline,false);
  EndShaderMode();
 }
 pass({x,y},WHITE,true);
 if(flash&&canOutline){
  BeginShaderMode(solidColor);
  pass({x,y},{255,255,255,static_cast<unsigned char>(alpha*.43f)},false);
  EndShaderMode();
 }
 return false;
}
}
