#pragma once
// Procedural 2D combat indicators over the existing painted terrain. They use
// the same height projection as feet and paths; existing Synfig actor clips
// are reused, without a new sprite, texture, model or animation asset.
namespace enemy_ability_art {
inline Color color(EnemyAbility ability){return ability==EnemyAbility::VenomFan?rgb(144,231,103):ability==EnemyAbility::CopperWave?rgb(242,177,103):ability==EnemyAbility::ObsidianCleave?rgb(196,175,253):rgb(255,154,69);}
inline void ring(V center,float radius,Color tint,float width=2){
 for(int i=0;i<48;i++){float a=i*2*PI/48,b=(i+1)*2*PI/48;DrawLineEx(screen(center+V{std::cos(a),std::sin(a)}*radius),screen(center+V{std::cos(b),std::sin(b)}*radius),width,tint);}
}
inline void cone(V center,V direction,float radius,float angle,Color fill,Color edge){
 Vector2 root=screen(center);for(int i=0;i<20;i++){
  V a=center+rotateAbilityDirection(direction,-angle+i*2*angle/20)*radius,b=center+rotateAbilityDirection(direction,-angle+(i+1)*2*angle/20)*radius;
  poly({root,screen(a),screen(b)},fill);DrawLineEx(screen(a),screen(b),1.8f,edge);
 }
 DrawLineEx(root,screen(center+rotateAbilityDirection(direction,-angle)*radius),1.7f,edge);DrawLineEx(root,screen(center+rotateAbilityDirection(direction,angle)*radius),1.7f,edge);
}
inline void cleave(const Enemy& e,Color fill,Color edge){
 V side{-e.abilityDirection.y*.72f,e.abilityDirection.x*.72f},a=e.pos+e.abilityDirection*.3f,b=e.pos+e.abilityDirection*5.2f;
 Vector2 p[]={screen(a-side),screen(b-side),screen(b+side),screen(a+side)};poly({p[0],p[1],p[2],p[3]},fill);for(int i=0;i<4;i++)DrawLineEx(p[i],p[(i+1)%4],2,edge);
}
inline void ground(){
 for(const auto& e:game.enemies){if(!e.alive||!e.abilityBusy()||!enemyAbilityRealm(e.type,game.location))continue;
  Vector2 at=screen(e.pos);if(at.x<-340||at.x>WorldW+340||at.y<-180||at.y>H+300)continue;
  const auto& spec=enemyAbilityProfile(e.type);EnemyAbility ability=enemyAbility(e.type);Color tint=color(ability);float progress=std::min(1.f,e.abilityAge/spec.windup);bool warning=e.abilityAge<spec.windup,active=e.abilityAge<spec.windup+spec.active;
  float alpha=warning?.1f+.12f*progress:active?.3f:.08f;
  if(ability==EnemyAbility::VenomFan){
   if(warning){cone(e.pos,e.abilityDirection,6,.34f,Fade(tint,alpha*.65f),Fade(tint,.7f));for(float angle:{-.28f,0.f,.28f})DrawLineEx(screen(e.pos+e.abilityDirection*.5f),screen(e.pos+rotateAbilityDirection(e.abilityDirection,angle)*6),1.3f,Fade(tint,.55f));}
  }else if(ability==EnemyAbility::CopperWave){
   if(warning){ring(e.pos,3.4f,Fade(tint,.6f));ring(e.pos,.65f+2.75f*progress,Fade(tint,.35f),1.4f);}
   else if(active){float radius=.65f+2.75f*(e.abilityAge-spec.windup)/spec.active;ring(e.pos,radius,Fade(tint,.95f),4);ring(e.pos,std::max(.2f,radius-.22f),Fade(tint,.42f),2);}
  }else if(ability==EnemyAbility::ObsidianCleave){
   cleave(e,Fade(tint,alpha),Fade(tint,warning?.8f:active?1.f:.18f));
   if(!warning&&active){Vector2 a=screen(e.pos+e.abilityDirection*.4f),b=screen(e.pos+e.abilityDirection*5.2f);DrawLineEx(a,b,5,Fade(WHITE,.85f));}
  }else{
   if(warning||active)cone(e.pos,e.abilityDirection,4.8f,.610865f,Fade(tint,alpha),Fade(tint,warning?.8f:.45f));
  }
  if(warning){std::string name=spec.name;float width=textWidth(name,15);rect(at.x-width/2-7,at.y-enemyInfo(e.type).visualHeight-35,width+14,23,rgb(15,20,28,220));txt(name,at.x-width/2,at.y-enemyInfo(e.type).visualHeight-32,15,tint);}
 }
}
inline void glob(const VenomGlob& value){
 Vector2 p=screen(value.pos),d{(value.velocity.x-value.velocity.y)*40,(value.velocity.x+value.velocity.y)*20};float n=std::hypot(d.x,d.y);if(n<.001f)return;d.x/=n;d.y/=n;p.y-=35;
 for(int i=4;i>=1;i--)ellipse(p.x-d.x*i*4,p.y-d.y*i*4,5-i*.65f,4-i*.5f,Fade(rgb(110,199,76),.6f-i*.09f));
 ellipse(p.x,p.y,5.5f,5,rgb(118,204,69));ellipse(p.x-1,p.y-1.5f,2.7f,2.4f,rgb(225,255,159));
}
inline void breath(){
 for(const auto& e:game.enemies){if(!e.alive||enemyAbility(e.type)!=EnemyAbility::EmberBreath||!e.abilityBusy())continue;
  const auto& spec=enemyAbilityProfile(e.type);if(e.abilityAge<spec.windup||e.abilityAge>=spec.windup+spec.active)continue;
  for(int ray=-2;ray<=2;ray++)for(int part=0;part<8;part++){
   float angle=ray*.23f,travel=.65f+part*.53f;V world=e.pos+rotateAbilityDirection(e.abilityDirection,angle)*travel;Vector2 p=screen(world);float wave=std::sin(game.time*21+part*1.7f+ray);p.y-=36-part*2+wave*4;
   float size=5+part*.8f;Color tint=part%2?rgb(255,124,47):rgb(255,182,70);ellipse(p.x,p.y,size,size*.7f,Fade(tint,.6f));
   poly({{p.x-size*.7f,p.y},{p.x+wave*3,p.y-size*(1.7f+.2f*wave)},{p.x+size*.7f,p.y}},Fade(tint,.65f));
   if(part<5)ellipse(p.x,p.y-1,size*.4f,size*.4f,Fade(rgb(255,236,165),.65f));
  }
 }
}
}
