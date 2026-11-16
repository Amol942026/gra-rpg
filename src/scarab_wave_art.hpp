#pragma once

namespace scarab_wave_art {
inline Texture2D crest{};

inline void load(){
 const char* file="assets/sprites/copper-wave-crest.png";
 if(FileExists(file))crest=LoadTexture(file);
 if(crest.id)SetTextureFilter(crest,TEXTURE_FILTER_BILINEAR);
}
inline void unload(){if(crest.id)UnloadTexture(crest);crest={};}

inline const auto Directions=[]{
 std::array<V,48> points{};
 for(int i=0;i<int(points.size());i++){
  const float angle=i*2*PI/points.size();
  points[i]={std::cos(angle),std::sin(angle)};
 }
 return points;
}();
inline void band(V center,float inner,float outer,Color inside,Color edge){
 for(int i=0;i<int(Directions.size());i++){
  const V a=Directions[i],b=Directions[(i+1)%Directions.size()];
  const Vector2 p=screen(center+a*inner),q=screen(center+a*outer);
  const Vector2 r=screen(center+b*outer),s=screen(center+b*inner);
  rlBegin(RL_TRIANGLES);
  rlColor4ub(inside.r,inside.g,inside.b,inside.a);rlVertex2f(p.x,p.y);
  rlColor4ub(edge.r,edge.g,edge.b,edge.a);rlVertex2f(r.x,r.y);rlVertex2f(q.x,q.y);
  rlVertex2f(r.x,r.y);rlColor4ub(inside.r,inside.g,inside.b,inside.a);rlVertex2f(p.x,p.y);rlVertex2f(s.x,s.y);
  rlEnd();
 }
}
inline void draw(const Enemy& e,const EnemyAbilityProfile& profile){
 const float elapsed=e.abilityAge-profile.windup;
 const Color gold=rgb(255,211,135),verdigris=rgb(111,195,160),copper=rgb(194,112,62);
 if(elapsed<0){
  const float charge=enemy_death::ease(e.abilityAge/profile.windup);
  band(e.pos,CopperWaveOuter-.08f,CopperWaveOuter,Fade(gold,.18f),Fade(gold,.62f));
  const float pull=CopperWaveOuter-(CopperWaveOuter-CopperWaveInner)*charge;
  band(e.pos,std::max(.12f,pull-.36f),pull,Fade(copper,0),Fade(copper,.34f+.24f*charge));
  for(int n=0;n<12;n++){
   const V radial=Directions[n*4],tangent{-radial.y,radial.x};
   const Vector2 a=screen(e.pos+radial*(CopperWaveOuter-.20f)-tangent*.10f);
   const Vector2 b=screen(e.pos+radial*(CopperWaveOuter-.20f)+tangent*.10f);
   const Vector2 c=screen(e.pos+radial*(CopperWaveOuter-.40f));
   DrawLineEx(a,c,1.5f,Fade(n%3?gold:verdigris,.42f+.36f*charge));
   DrawLineEx(b,c,1.5f,Fade(n%3?gold:verdigris,.42f+.36f*charge));
  }
  return;
 }
 const float phase=std::min(1.f,elapsed/profile.active);
 const float fade=1-enemy_death::window(elapsed,profile.active,profile.active+.38f);
 if(fade<=0)return;
 const float front=copperWaveRadius(phase),wake=std::max(.05f,front-.52f);
 band(e.pos,wake,front-.11f,Fade(copper,0),Fade(copper,.39f*fade));
 band(e.pos,front-.11f,front,Fade(copper,.39f*fade),Fade(gold,.94f*fade));
 band(e.pos,front,front+.13f,Fade(gold,.94f*fade),Fade(gold,0));
 const float echo=std::max(.10f,front-.40f);
 if(echo>.15f)band(e.pos,echo-.05f,echo,Fade(verdigris,.04f),Fade(verdigris,.41f*fade));
 for(int i=0;i<16;i++){
  const int direction=(i*3+e.id%3)%int(Directions.size());
  const V radial=Directions[direction],tangent{-radial.y,radial.x};
  const float ripple=.5f+.5f*std::sin(phase*7.8f+i*2.39996f);
  const Vector2 foot=screen(e.pos+radial*(front-.05f));
  if(crest.id){
   const Vector2 to=screen(e.pos+radial*(front-.05f)+tangent*.4f);
   float angle=std::atan2(to.y-foot.y,to.x-foot.x)*RAD2DEG;
   if(angle>90)angle-=180;
   if(angle<-90)angle+=180;
   const float width=std::clamp(front*13.5f,13.f,49.f),height=13+12*ripple;
   const Rectangle cell{0,0,float(crest.width),float(crest.height)};
   DrawTexturePro(crest,cell,{foot.x,foot.y,width,height},{width*.52f,height*.76f},angle*.47f,
                  Fade(WHITE,fade*(.57f+.31f*ripple)));
  }
  const Vector2 fleck=screen(e.pos+radial*(front-.20f-.18f*ripple)+tangent*.14f);
  const float rise=(5+17*ripple)*std::sin(phase*PI*.85f+.30f);
  DrawCircleV({fleck.x,fleck.y-rise},1.25f+ripple,Fade(i%4?gold:verdigris,fade*(.25f+.52f*ripple)));
 }
}
}
