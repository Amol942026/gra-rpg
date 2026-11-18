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

inline bool draw(Clip clip,Vector2 at,Vector2 size,float angle,float phase,float opacity=1,Vector2 anchor={.5f,.5f}){
 if(!atlas.id)return false;
 const auto key=caldera_attack_motion::effect(int(clip),phase);
 const float width=size.x*key.width,height=size.y*key.height;
 DrawTexturePro(atlas,{float(int(clip)*512+1),1,510,510},{at.x,at.y,width,height},
                {width*anchor.x,height*anchor.y},angle+key.angle,Fade(WHITE,std::clamp(opacity*key.opacity,0.f,1.f)));
 return true;
}
}
