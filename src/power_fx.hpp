#pragma once

namespace power_fx {
inline Texture2D atlas{};
inline constexpr Color colors[]={{154,225,255,255},{248,159,65,255},{208,139,255,255},{109,248,217,255},{255,213,103,255}};
inline void load(){
 if(atlas.id||!FileExists("assets/sprites/power-attacks.png"))return;
 atlas=LoadTexture("assets/sprites/power-attacks.png");
 if(atlas.id&&(atlas.width!=640||atlas.height!=128)){UnloadTexture(atlas);atlas={};}
 if(atlas.id)SetTextureFilter(atlas,TEXTURE_FILTER_BILINEAR);
}
inline void unload(){if(atlas.id)UnloadTexture(atlas);atlas={};}
inline void stamp(PowerStyle style,Vector2 at,float width,float height,float degrees,float alpha){
 if(atlas.id&&alpha>0)
  DrawTexturePro(atlas,{float(int(style)*128),0,128,128},{at.x,at.y,width,height},{width*.5f,height*.5f},degrees,Fade(WHITE,alpha));
}
inline void arrow(Vector2 tip,Vector2 direction){
 const Color gold=colors[int(PowerStyle::Bow)];
 Vector2 center{tip.x-direction.x*37,tip.y-direction.y*37};
 stamp(PowerStyle::Bow,center,92,17,std::atan2(direction.y,direction.x)*RAD2DEG,.87f);
 DrawLineEx({tip.x-direction.x*72,tip.y-direction.y*72},tip,1.7f,Fade(gold,.68f));
 DrawCircleV(tip,3.2f,Fade(gold,.72f));
}
inline void world(const Game& game){
 const PowerStyle style=game.activePowerVisual();
 if(style==PowerStyle::Count||game.dead()||game.knockedBack()||game.rolling())return;
 const auto& profile=powerAttack(style);
 const float elapsed=game.powerVisualAge()*profile.duration;
 float age=elapsed-profile.contact;
 const bool second=style==PowerStyle::Dagger&&elapsed>=profile.second;
 if(second)age=elapsed-profile.second;
 age/=powerAttackRate(style);
 const float life=style==PowerStyle::Axe?.20f:style==PowerStyle::Sword?.17f:style==PowerStyle::Dagger?.125f:.22f;
 if(age<0||age>=life)return;
 const float t=age/life,alpha=(1-t)*.86f;
 const V direction=style==PowerStyle::Bow?game.bowAim:normalized(game.facing);
 const Vector2 origin=screen(game.player),ahead=screen(game.player+direction);
 const float angle=std::atan2(ahead.y-origin.y,ahead.x-origin.x)*RAD2DEG;
 const Color tint=colors[int(style)];
 auto point=[&](float along,float lateral,float height){
  Vector2 p=screen(game.player+direction*along+V{-direction.y,direction.x}*lateral);
  p.y-=height;return p;
 };
 switch(style){
 case PowerStyle::Sword:
  stamp(style,point(profile.reach*(.48f+t*.135f),.14f*t,49),125+26*t,70+14*t,angle+180,alpha);
  for(int i=0;i<5;i++){
   const float sweep=(i-2)*.27f+t*.46f;
   Vector2 p=point(profile.reach*(.70f+t*.14f)-std::abs(sweep)*.35f,sweep,52-t*7);
   DrawCircleV(p,1.65f,Fade(tint,alpha));
  }
  break;
 case PowerStyle::Axe:{
  const float along=1.42f*std::min(1.f,game.weaponContactBlock(direction*1.42f,.10f));
  Vector2 p=point(along,0,3);
  stamp(style,{p.x,p.y-13},120+75*t,66+29*t,0,alpha*.92f);
  DrawEllipseLines(int(p.x),int(p.y),26+54*t,11+26*t,Fade(tint,alpha*.82f));
  for(int i=0;i<8;i++){
   float a=i*PI/4;
   Vector2 chip{p.x+std::cos(a)*(19+54*t),p.y+std::sin(a)*(7+20*t)-42*t*(1-t)};
   DrawLineEx(chip,{chip.x+std::cos(a)*6,chip.y-3},2.3f,Fade(tint,alpha));
  }
  break;
 }
 case PowerStyle::Dagger:
  stamp(style,point(1.02f+t*.40f,second?.21f:-.21f,43),86+18*t,44,angle+(second?-14:14),alpha);
  break;
 case PowerStyle::Spear:{
  const float end=profile.reach*std::min(1.f,game.weaponContactBlock(direction*profile.reach,SpearTipRadius));
  const Vector2 a=point(std::min(.55f,end),0,43),b=point(end,0,43);
  const float extent=std::hypot(b.x-a.x,b.y-a.y);
  if(extent>1){stamp(style,{(a.x+b.x)*.5f,(a.y+b.y)*.5f},extent+16,28,angle,alpha);
   DrawLineEx(a,b,1.5f,Fade(tint,alpha*.55f));}
  break;
 }
 case PowerStyle::Bow:
  stamp(style,point(.52f+t*.5f,0,44),78+24*t,34,angle,alpha);
  break;
 case PowerStyle::Count:break;
 }
}
inline Rectangle chargeBarRect(PowerStyle style,Vector2 feet){
 const float height=style==PowerStyle::Sword||style==PowerStyle::Axe?132.f:107.f;
 return {std::round(feet.x-27),std::round(feet.y-height),54,7};
}
inline void chargeBar(const Game& game,Vector2 feet){
 const float charge=game.powerWindupProgress();
 const PowerStyle style=powerStyle(archetype(game.equipment[0].kind));
 if(charge<0||style==PowerStyle::Count)return;
 const Color tint=colors[int(style)];
 const Rectangle frame=chargeBarRect(style,feet);
 DrawRectangleRounded({frame.x-2,frame.y-2,frame.width+4,frame.height+4},.42f,5,Fade(Ink,.87f));
 DrawRectangleRec(frame,rgb(21,34,37));
 DrawRectangleLinesEx(frame,1,Fade(charge>=1?tint:Gold,charge>=1?1.f:.63f));
 const int fill=int((frame.width-4)*charge);
 if(fill>0)DrawRectangleGradientV(int(frame.x)+2,int(frame.y)+2,fill,int(frame.height)-4,
                                 charge>=1?WHITE:tint,shade(tint,.61f));
}
}
