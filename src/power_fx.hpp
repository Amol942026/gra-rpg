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
inline Vector2 spearPoint(Vector2 feet,SpearContact contact,float fraction){
 const V offset=contact.ray*fraction;
 return {feet.x+(offset.x-offset.y)*40,feet.y+(offset.x+offset.y)*20-contact.height};
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
  const auto contact=game.spearContact(true);
  const float end=std::min(1.f,game.weaponContactBlock(contact.ray,0));
  const Vector2 a=spearPoint(origin,contact,std::min(.34f,end)),b=spearPoint(origin,contact,end);
  const float extent=std::hypot(b.x-a.x,b.y-a.y);
  if(extent>1){stamp(style,{(a.x+b.x)*.5f,(a.y+b.y)*.5f},extent,28,
     std::atan2(b.y-a.y,b.x-a.x)*RAD2DEG,alpha);
   DrawLineEx(a,b,1.5f,Fade(tint,alpha*.55f));}
  break;
 }
 case PowerStyle::Bow:
  stamp(style,point(.52f+t*.5f,0,44),78+24*t,34,angle,alpha);
  break;
 case PowerStyle::Count:break;
 }
}
inline void weaponSkill(const Game& game){
 if(!game.strongSwingActive()||game.dead()||game.knockedBack()||game.rolling())return;
 const Skill skill=game.skillForSlot(0);
 const bool sword=skill==Skill::StrongCut,axe=skill==Skill::Cleave,dagger=skill==Skill::DoubleThrust;
 const float elapsed=game.meleeDuration-game.swing;
 const bool second=dagger&&elapsed>=SkillDaggerSecond;
 const float contact=sword?.07f:axe?AxeImpactFrame/60.f:dagger?(second?SkillDaggerSecond:DaggerImpactFrame/60.f):SpearImpactTime;
 const float life=dagger?.13f:.23f;
 const float t=(elapsed-contact)/life;
 if(t<0||t>=1)return;
 const V axis=normalized(game.facing),side{-axis.y,axis.x};
 const float opacity=(1-t)*(1-t);
 const Color tint=sword?rgb(255,211,122):axe?rgb(255,150,87):dagger?rgb(207,164,255):rgb(133,240,225);
 auto at=[&](V delta,float height){
  auto p=screen(game.player+delta);p.y-=height;return p;
 };
 if(sword||axe){
  const float spread=sword?PI*.5f:1.20f,radius=(sword?MeleeRange:SkillAxeReach)*(.80f+.16f*t);
  const float span=sword?1.7f:1.15f;
  const float start=-spread+(2*spread-span)*t;
  Vector2 previous{};
  for(int i=0;i<=18;i++){
   const float along=i/18.f,angle=start+span*along;
   const V ray=axis*std::cos(angle)+side*std::sin(angle);
   const Vector2 tip=at(ray*radius,(sword?48.f:37.f)+6*std::sin(angle));
   if(i>0){
    const float brightness=opacity*std::sin((.18f+.78f*along)*PI);
    DrawLineEx(previous,tip,8.f*brightness+.9f,Fade(tint,brightness*.7f));
    DrawLineEx(previous,tip,1.8f,Fade(WHITE,brightness));
   }
   if(i%4==0){
    const Vector2 ember=at(ray*(radius+.18f+t*.45f),46+22*t+(i%3)*7);
    DrawCircleV(ember,1.6f,Fade(tint,opacity*.8f));
   }
   previous=tip;
  }
  return;
 }
 if(skill==Skill::Skewer){
  const auto tip=game.spearContact(false);
  const float end=std::min(1.f,game.weaponContactBlock(tip.ray,0));
  const Vector2 feet=screen(game.player);
  const Vector2 from=spearPoint(feet,tip,std::min(.26f,end)),to=spearPoint(feet,tip,end);
  for(int i=-1;i<=1;i++){
   const float offset=i*3.5f;
   DrawLineEx({from.x,from.y+offset},{to.x,to.y+offset},i==0?5.f:2.f,Fade(tint,opacity*(i==0?.9f:.48f)));
  }
  DrawLineEx(from,to,1.5f,Fade(WHITE,opacity));
  return;
 }
 const float end=SkillDaggerReach*std::min(1.f,game.weaponContactBlock(axis*SkillDaggerReach,SpearTipRadius));
 const float start=std::min(.38f,end);
 for(int i=-1;i<=1;i++){
  const float lateral=(dagger?(second?.18f:-.18f):0.f)+i*.13f;
  const Vector2 from=at(axis*start+side*lateral,42+i*3.f);
  const Vector2 tip=at(axis*(end*(.89f+.11f*t))+side*lateral,42+i*3.f);
  DrawLineEx(from,tip,i==0?5.f:2.f,Fade(tint,opacity*(i==0?.9f:.48f)));
  if(i==0)DrawLineEx(from,tip,1.5f,Fade(WHITE,opacity));
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
