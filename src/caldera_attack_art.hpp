#pragma once
#include "generated/caldera_attack_motion.hpp"

namespace caldera_attack_art {
enum class Clip {Cleave,Breath,Eruption,Cinder};
inline Texture2D atlas{};

inline void load(){
 if(atlas.id)return;
 atlas=LoadTexture("assets/volcano/caldera-attack-fx.png");
 if(atlas.id&&(atlas.width!=2048||atlas.height!=512)){UnloadTexture(atlas);atlas={};}
 if(atlas.id)SetTextureFilter(atlas,TEXTURE_FILTER_BILINEAR);
}
inline void unload(){if(atlas.id)UnloadTexture(atlas);atlas={};}

// Bend one painted plume across the projected damage cone. Its root stays
// fixed to the jaw while waves travel toward the widening, cooling end.
inline void drawBreath(Vector2 mouth,Vector2 end,float footprintWidth,float elapsed,float duration){
 const float dx=end.x-mouth.x,dy=end.y-mouth.y,length=std::hypot(dx,dy);
 if(length<1.f)return;
 const Vector2 normal{-dy/length,dx/length};
 const float grow=std::clamp(elapsed/.12f,0.f,1.f);
 const float dissipate=std::clamp((duration-elapsed)/.10f,0.f,1.f);
 const float reach=.30f+.70f*(1-(1-grow)*(1-grow));
 const float pressure=std::cos(elapsed*(2*PI/.35f));
 const float width=std::clamp(footprintWidth,44.f,178.f)*(.93f+.055f*pressure);
 constexpr int strips=16;
 struct Edge {Vector2 top,bottom;float u;};
 auto section=[&](int index){
  const float t=float(index)/strips;
  const float bend=t*(std::sin(12*t-16*elapsed)*.054f+std::sin(5*t-9*elapsed)*.028f)*width;
  const float thickness=width*(1+t*.10f*std::sin(15*t-19*elapsed));
  // The bright ignition thread sits 64% down the source rectangle.
  const float top=bend-.64f*thickness,bottom=bend+.36f*thickness;
  const float u=t+t*(1-t)*.065f*std::sin(14*t-18*elapsed);
  const Vector2 center{mouth.x+dx*t*reach,mouth.y+dy*t*reach};
  return Edge{{center.x+normal.x*top,center.y+normal.y*top},
              {center.x+normal.x*bottom,center.y+normal.y*bottom},(513.5f+u*509.f)/atlas.width};
 };
 auto edge=section(0);
 const float topV=1.5f/atlas.height,bottomV=510.5f/atlas.height;
 rlCheckRenderBatchLimit(strips*4);rlSetTexture(atlas.id);rlBegin(RL_QUADS);
 rlColor4ub(255,255,255,static_cast<unsigned char>(255*std::sqrt(dissipate)));
 for(int i=1;i<=strips;++i){
  const auto next=section(i);
  rlTexCoord2f(edge.u,topV);rlVertex2f(edge.top.x,edge.top.y);
  rlTexCoord2f(edge.u,bottomV);rlVertex2f(edge.bottom.x,edge.bottom.y);
  rlTexCoord2f(next.u,bottomV);rlVertex2f(next.bottom.x,next.bottom.y);
  rlTexCoord2f(next.u,topV);rlVertex2f(next.top.x,next.top.y);
  edge=next;
 }
 rlEnd();rlSetTexture(0);
}

// Wrap the painted dragon-fire material round a rotating ember. The same
// core sits between the imp's palms and leaves them as a travelling sphere.
inline constexpr float heldOrbScale=.72f;
inline void fireOrb(Vector2 at,float scale,float energy,float time,float freeFlame=1){
 if(energy<=.012f)return;
 const float radius=scale*(1.4f+10.f*std::sqrt(energy));
 const float light=std::min(1.f,energy*2.4f);
 DrawCircleGradient(int(at.x),int(at.y),radius*(1.32f+.40f*freeFlame),Fade(rgb(255,130,23),(.31f+.19f*freeFlame)*light),BLANK);
 if(atlas.id){
  constexpr int Segments=24,Rings=4;
  constexpr float tau=2*PI;
  const float spin=time*2.8f;
  rlCheckRenderBatchLimit(Segments*Rings*4);rlSetTexture(atlas.id);rlBegin(RL_QUADS);
  auto vertex=[&](int segment,int ring){
   const float a=tau*segment/Segments,t=ring/float(Rings);
   const float u=std::cos(a),v=std::sin(a);
   const float flame=std::max(0.f,std::sin(5*a-1.7f*spin)+.52f*std::sin(8*a+2.2f*spin));
   const float r=radius*t*(1+(ring==Rings?.14f:0)*flame);
   const float turn=spin+.82f*t;
   const float tx=std::cos(a+turn)*t,ty=std::sin(a+turn)*t;
   const float alpha=(ring==Rings?.15f:ring==Rings-1?.98f:1.f)*light;
   rlColor4ub(255,255,255,static_cast<unsigned char>(alpha*255));
   rlTexCoord2f((512+295+tx*110+16*std::sin(spin*.8f))/atlas.width,
                (275+ty*105+13*std::cos(spin*1.1f))/atlas.height);
   rlVertex2f(at.x+u*r,at.y+v*r);
  };
  for(int ring=0;ring<Rings;++ring)for(int i=0;i<Segments;++i){
   vertex(i,ring);vertex(i+1,ring);vertex(i+1,ring+1);vertex(i,ring+1);
  }
  rlEnd();rlSetTexture(0);
 }else{
  DrawCircleGradient(int(at.x),int(at.y),radius,Fade(rgb(255,237,139),light),Fade(rgb(248,74,12),light));
 }
 // Short incandescent threads break the rim as the charge rolls around it.
 for(int j=0;j<3;++j){
  const float start=j*2*PI/3-time*(j%2?3.4f:4.6f);
  Vector2 last{};
  for(int k=0;k<=5;++k){
   const float u=k/5.f,a=start+u*1.46f,r=radius*(.63f+.34f*u);
   Vector2 here{at.x+std::cos(a)*r,at.y+std::sin(a)*r*.92f};
   if(k)DrawLineEx(last,here,scale*(1.5f-.75f*u),Fade(rgb(255,217,99),light*(1-u*.76f)));
   last=here;
  }
 }
}

inline void impFireburst(Vector2 hit,float age){
 const float t=std::clamp(age,0.f,1.f),q=1-(1-t)*(1-t)*(1-t);
 const float flames=std::max(0.f,1-t/.81f);
 // The broad roots of seven tongues touch the hot center. Their sharp ends
 // turn outward and curl upward as the pressure gives way to rising heat.
 struct Tongue {float bearing,length,width,bend;};
 static constexpr Tongue spray[]={{-105,41,18,-15},{-56,37,16,-19},{-12,32,14,-23},
                                  {39,27,14,-19},{97,22,13,17},{151,34,16,19},{-161,38,15,16}};
 if(flames>0){
  for(const auto& l:spray){
   const float a=l.bearing*DEG2RAD,span=(.22f+.78f*q)*l.length;
   const float width=l.width*(.44f+.56f*q)*std::sqrt(flames);
   const Vector2 root{hit.x+std::cos(a)*q*11,hit.y+std::sin(a)*q*8-6*t};
   if(atlas.id)DrawTexturePro(atlas,{513,1,510,510},{root.x,root.y,span,width},
               {span*.85f,width*.54f},180+l.bearing+t*l.bend,Fade(WHITE,flames*(.8f+.2f*std::sin(a))));
   else DrawLineEx(root,{root.x+std::cos(a)*span,root.y+std::sin(a)*span},width*.4f,Fade(rgb(253,117,34),flames));
  }
  fireOrb({hit.x,hit.y-3*t},1+.65f*q,flames,1+t*2.5f);
 }
 for(int i=0;i<9;++i){
  const float a=(i*2.39996f-.61f),r=q*(17+((i*13)%27)),rise=(5+(i%3)*8)*t;
  const Vector2 at{hit.x+std::cos(a)*r,hit.y+std::sin(a)*r*.72f-rise+9*t*t};
  const float shine=std::clamp((1-t)*(1.6f-.06f*i),0.f,1.f);
  const float tail=5*(1-t);
  DrawLineEx({at.x-std::cos(a)*tail,at.y-std::sin(a)*tail*.72f+tail*.3f},at,1.5f,Fade(rgb(242,99,26),shine));
  if(i%3!=1)DrawCircleV(at,1.25f-.6f*t,Fade(rgb(255,207,107),shine));
 }
}

inline bool draw(Clip clip,Vector2 at,Vector2 size,float angle,float phase,float opacity=1,Vector2 anchor={.5f,.5f}){
 if(!atlas.id)return false;
 const auto key=caldera_attack_motion::effect(int(clip),phase);
 const float width=size.x*key.width,height=size.y*key.height;
 DrawTexturePro(atlas,{float(int(clip)*512+1),1,510,510},{at.x,at.y,width,height},
                {width*anchor.x,height*anchor.y},angle+key.angle,Fade(WHITE,std::clamp(opacity*key.opacity,0.f,1.f)));
 return true;
}
}
