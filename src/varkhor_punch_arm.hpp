#pragma once

// Both rock plates remain rigid. The exposed cross-section at the hinge
// sweeps out a small elbow joint as the lower plate rotates.
namespace varkhor_punch {
struct Vertex {Vector2 point,source;};
using Triangle=std::array<Vertex,3>;
struct Target {Vector2 fromFeet;int arm;};

struct Joints {Vector2 shoulder,elbow,fist;};
inline Joints binding(int plate){
 if(plate==4)return {{37.f,66.5f},{35.6f,82.5f},{30.8f,103.1f}};
 return {{90.f,66.5f},{92.5f,82.5f},{97.6f,103.2f}};
}

struct Arm {
 Vector2 elbow;
 std::array<Triangle,16> triangles{};
 int count=0;

 static Vector2 rotate(Vector2 v,float angle){
  const float c=std::cos(angle),s=std::sin(angle);
  return {v.x*c-v.y*s,v.x*s+v.y*c};
 }

 Arm(int plate,float bend):elbow(binding(plate).elbow){
  constexpr int Slices=6;
  const float left=plate==4?22.f:78.f,right=plate==4?50.f:105.f;
  const auto vertex=[&](float x,float y,float angle){
   const auto v=rotate({x-elbow.x,y-elbow.y},angle);
   return Vertex{{elbow.x+v.x,elbow.y+v.y},{x,y}};
  };
  const auto joint=vertex(elbow.x,elbow.y,0);
  for(float edge:{left,right}){
   auto previous=vertex(edge,elbow.y,0);
   for(int i=1;i<=Slices;++i){
    const auto next=vertex(edge,elbow.y,bend*i/Slices);
    triangles[count++]={joint,previous,next};
    previous=next;
   }
  }
  const auto plateStrip=[&](float top,float bottom,float angle){
   const auto a=vertex(left,top,angle),b=vertex(left,bottom,angle);
   const auto c=vertex(right,bottom,angle),d=vertex(right,top,angle);
   triangles[count++]={a,b,c};
   triangles[count++]={a,c,d};
  };
  plateStrip(54.f,elbow.y,0);
  plateStrip(elbow.y,111.f,bend);
 }

 template<class Opaque> bool contains(Vector2 point,const Opaque& opaque)const{
  const auto cross=[](Vector2 a,Vector2 b,Vector2 c){return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);};
  for(int i=count-1;i>=0;--i){
   const auto& t=triangles[i];
   const auto a=t[0],b=t[1],c=t[2];
   const float area=cross(a.point,b.point,c.point);
   if(std::abs(area)<.0001f)continue;
   const float u=cross(b.point,c.point,point)/area,v=cross(c.point,a.point,point)/area,w=1-u-v;
   if(u<0||v<0||w<0)continue;
   if(opaque({u*a.source.x+v*b.source.x+w*c.source.x,u*a.source.y+v*b.source.y+w*c.source.y}))return true;
  }
  return false;
 }

 void paint(Texture2D atlas,int direction,int plate,Color tint,Vector2 pivot)const{
  rlCheckRenderBatchLimit(count*4);
  rlBegin(RL_QUADS);rlSetTexture(atlas.id);
  for(int i=0;i<count;++i){
   const auto& t=triangles[i];
   // All patches use the same winding, including the opposite elbow.
   const float area=(t[1].point.x-t[0].point.x)*(t[2].point.y-t[0].point.y)-
                    (t[1].point.y-t[0].point.y)*(t[2].point.x-t[0].point.x);
   const int b=area>0?2:1,c=3-b;
   for(int index:{0,b,c,c}){
    const auto& v=t[index];
    rlColor4ub(tint.r,tint.g,tint.b,tint.a);
    rlTexCoord2f((direction+v.source.x/128)/8,(plate+v.source.y/128)/6);
    rlVertex2f(v.point.x-pivot.x,v.point.y-pivot.y);
   }
  }
  rlEnd();rlSetTexture(0);
 }
};

inline float aimAngle(float forward,float side,int plate){
 const float across=plate==4?std::min(-.16f,forward):std::max(.16f,forward);
 const float angle=std::remainder(std::atan2(.51f+.72f*side,across)-PI*.5f,2*PI)*RAD2DEG;
 return std::clamp(angle,-105.f,105.f);
}

inline float frontView(float forward){
 return 1-enemy_death::window(std::abs(forward),.12f,.78f);
}

struct PunchPose {Joints joints;float upper,lower;};
inline PunchPose followFist(int plate,float forward,float side,float reach,float lift,float lateralRaise,
                            const Vector2* contact=nullptr){
 const auto rest=binding(plate);
 const auto delta=[](Vector2 a,Vector2 b){return Vector2{a.x-b.x,a.y-b.y};};
 const auto length=[](Vector2 a){return std::hypot(a.x,a.y);};
 const auto bearing=[](Vector2 a){return std::atan2(a.y,a.x)*RAD2DEG;};
 const auto restHand=delta(rest.fist,rest.shoulder);
 const auto restUpper=delta(rest.elbow,rest.shoulder),restLower=delta(rest.fist,rest.elbow);
 if(lateralRaise<=0)return {rest,0,0};
 const float upperLength=length(restUpper),lowerLength=length(restLower);
 const float angle=aimAngle(forward,side,plate)*DEG2RAD;
 const float extension=upperLength+lowerLength-.18f;
 const float flank=plate==4?-1.f:1.f;
 const auto extended=Arm::rotate({0,extension},angle);
 const float headsOn=frontView(forward);
 // A frontal punch projects a shorter reach. Its elbow stays outside the chest.
 const Vector2 aim=contact?*contact:Vector2{extended.x+(-flank*(side>=0?15.f:8.f)-extended.x)*headsOn,
                                           extended.y+((side>=0?26.8f:22.5f)-extended.y)*headsOn};
 // A high target can be above the shoulder. Take the fist around the outside
 // of the raised elbow before reaching it, so the two plates never fold through it.
 const float swing=contact?19.f*enemy_death::window(-aim.y,1.f,22.f)*std::sin(PI*reach):0.f;
 const Vector2 hand{restHand.x+(aim.x-restHand.x)*reach+flank*swing,
                    restHand.y+(aim.y-restHand.y)*reach-lift};
 const float folded=std::sqrt(upperLength*upperLength+lowerLength*lowerLength+
                    2*upperLength*lowerLength*std::cos(104.f*DEG2RAD));
 const float limit=std::max(1.f,folded/std::max(.01f,length(hand)));
 const auto solved=enemy_arm::solve({0,0,0},{0,flank*hand.x*limit,-hand.y*limit},
                                   upperLength,lowerLength,0,-1);
 const auto local=[&](enemy_arm::Point p){return Vector2{rest.shoulder.x+flank*p.forward,rest.shoulder.y-p.up};};
 const auto elbow=local(solved.elbow),fist=local(solved.hand);
 const float upperAim=bearing(delta(elbow,rest.shoulder))-bearing(restUpper);
 const float lowerAim=bearing(delta(fist,elbow))-bearing(restLower);
 const float upper=std::remainder(upperAim,360.f)*lateralRaise;
 const float lower=std::remainder(lowerAim-upperAim,360.f)*lateralRaise;
 const auto bicep=Arm::rotate(restUpper,upper*DEG2RAD);
 const auto forearm=Arm::rotate(restLower,(upper+lower)*DEG2RAD);
 const Vector2 raisedElbow{rest.shoulder.x+bicep.x,rest.shoulder.y+bicep.y};
 return {{rest.shoulder,raisedElbow,{raisedElbow.x+forearm.x,raisedElbow.y+forearm.y}},upper,lower};
}
}
