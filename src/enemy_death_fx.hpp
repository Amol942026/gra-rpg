#pragma once

namespace enemy_death_fx {
inline Texture2D guardianSmokeTexture{};
inline void init(){
 Image image=LoadImage("assets/stone-guardian-smoke.png");
 if(image.data&&image.width==256&&image.height==256){
  guardianSmokeTexture=LoadTextureFromImage(image);
  SetTextureFilter(guardianSmokeTexture,TEXTURE_FILTER_BILINEAR);
 }
 if(image.data)UnloadImage(image);
}
inline void unload(){
 if(guardianSmokeTexture.id)UnloadTexture(guardianSmokeTexture);
 guardianSmokeTexture={};
}
inline Rectangle guardianSmokeBounds(Vector2 anchor){
 return {anchor.x-112,anchor.y-115,224,131};
}
inline void guardianSmoke(const Enemy& e,Vector2 anchor){
 if(!guardianSmokeTexture.id||e.deathElapsed<0)return;
 const auto [opacity,spread,rise]=enemy_death::sampleTrack(
     enemy_death::data::guardianSmoke,e.deathElapsed*enemy_death::data::framesPerSecond);
 if(opacity<=0)return;
 const float side=float(e.deathScreenSide());
 const float width=88+100*spread,height=69+31*spread;
 const Vector2 center{anchor.x+side*12*spread,anchor.y-33-26*rise};
 const float size=float(guardianSmokeTexture.width);
 const Color tone=ColorLerp(rgb(249,248,246),rgb(225,228,223),spread);
 DrawTexturePro(guardianSmokeTexture,{0,0,side<0?-size:size,size},
                {center.x,center.y,width,height},{width*.5f,height*.5f},side*(1-spread)*5,Fade(tone,opacity));
}

struct Material {Color light,dark;float drift,weight;int pieces;};
inline Material material(int type){
 switch(impactMaterial(type)){
 case ImpactMaterial::Stone:return {rgb(155,147,125),rgb(74,81,73),27,1.5f,9};
 case ImpactMaterial::Bark:return {rgb(162,150,82),rgb(91,67,42),33,1.35f,8};
 case ImpactMaterial::Slime:return {rgb(148,190,101),rgb(73,113,55),21,.95f,7};
 case ImpactMaterial::Bone:return {rgb(220,203,161),rgb(133,124,102),24,1.15f,6};
 case ImpactMaterial::Sand:return {rgb(220,178,106),rgb(155,113,68),48,2.3f,12};
 case ImpactMaterial::Shell:return {rgb(194,141,92),rgb(108,85,63),25,1.3f,6};
 case ImpactMaterial::Ember:return {rgb(240,145,71),rgb(152,65,48),35,1.24f,9};
 default:return {rgb(161,141,103),rgb(100,89,70),23,1.35f,6};
 }
}
inline void draw(const Enemy& e,Vector2 anchor){
 if(e.type==VolcanoBossType){
  varkhor_death_art::rollingAsh(e,anchor.x,anchor.y,e.deathElapsed);
  return;
 }
 if(e.type==StoneGuardianType)guardianSmoke(e,anchor);
 const float contact=enemy_death::groundContact(e.type);
 if(e.deathElapsed<enemy_death::groundContactTime(e.type))return;
 const float age=std::clamp((e.deathProgress()-contact)/(1-contact),0.f,1.f);
 const auto paint=material(e.type);
 const float alpha=(.65f+.35f*age)*(1-age)*(1-age);
 const float scale=enemyBoss(e.type)?1.46f:1.f;
 const float impact=std::hypot(e.hitDirection.x,e.hitDirection.y)>.1f?std::atan2(e.hitDirection.x+e.hitDirection.y,2*(e.hitDirection.x-e.hitDirection.y)):0;
 const bool spores=e.type==SporekinType,ghost=e.type==AshWraithType;
 for(int i=0;i<paint.pieces;i++){
  const float a=impact+i*2.39996f+e.id*.16f;
  const float span=scale*paint.drift*(.22f+.15f*(i%5))*(.25f+age);
  const float rise=scale*(5+2*(i%4))*(ghost||spores?age:age*(1-age)*4);
  const Vector2 p{anchor.x+std::cos(a)*span,anchor.y+std::sin(a)*span*.44f-rise};
  const float size=paint.weight*scale*(1-.48f*age);
  if(e.type==DesertBossType||ghost||spores){
   DrawEllipse(int(p.x),int(p.y),size*2.4f,size*.86f,Fade(i%3?paint.light:paint.dark,alpha*.44f));
  }else if(i%3==0){
   DrawLineEx(p,{p.x+std::cos(a)*size*2,p.y+std::sin(a)*size},size,Fade(paint.dark,alpha*.65f));
  }else DrawCircleV(p,size,Fade(paint.light,alpha*.68f));
 }
}
}
