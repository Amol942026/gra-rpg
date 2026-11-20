#pragma once

// This pose shares the directional atlas renderer with the other Caldera beasts.
namespace cinderhorn_turn {
inline bool ready(){return turn_art::cinderhorn.ready();}

inline constexpr Vector2 Joints[8][6]={
 {{55,91},{44,94},{107,78},{80,91},{117,91},{94,98}},
 {{71,96},{52,98},{112,76},{80,91},{120,93},{86,98}},
 {{87,106},{72,106},{80,76},{80,93},{98,102},{63,102}},
 {{108,98},{89,96},{47,76},{79,91},{73,98},{39,93}},
 {{115,94},{104,91},{52,78},{79,91},{65,98},{42,91}},
 {{92,102},{109,98},{50,68},{79,89},{55,93},{78,97}},
 {{60,101},{100,101},{80,60},{80,92},{69,105},{91,105}},
 {{50,98},{67,102},{109,68},{80,89},{81,97},{104,93}}
};

template<size_t N>
inline Vector2 track(const cinderhorn_synfig::Key (&keys)[N],float time){
 const auto* next=std::upper_bound(keys,keys+N,time,[](float t,const auto& key){return t<key.time;});
 if(next==keys)return {keys[0].along,keys[0].lift};
 if(next==keys+N)return {keys[N-1].along,keys[N-1].lift};
 const auto& previous=*(next-1);
 const float between=(time-previous.time)/(next->time-previous.time);
 return {turn_art::mix(previous.along,next->along,between),turn_art::mix(previous.lift,next->lift,between)};
}

inline std::array<Part,6> pose(const Enemy& e,const turn_art::View& view){
 const auto& state=e.animation;
 const float heading=turn_art::mix(facingAngles[view.base],view.base==7?2*PI:facingAngles[view.nextBase()],view.t);
 const float forward=std::cos(heading),depth=std::sin(heading);
 const bool rushing=e.boarRush>0;
 const float action=e.windup>0?.5f*(1-e.windup/meleeWindup(e.type)):
                    e.strike>0?.5f+.5f*(1-e.strike/meleeRecovery(e.type)):0;
 Vector2 head=track(cinderhorn_synfig::lower,action);
 if(e.boarWindup>0)head=track(cinderhorn_synfig::lower,.5f*(1-e.boarWindup/CinderhornCharge.windup));
 if(rushing)head=track(cinderhorn_synfig::lower,.5f);
 if(e.boarRecovery>0)head=track(cinderhorn_synfig::lower,1-.5f*e.boarRecovery/CinderhornCharge.recovery);
 const float movement=state.motionInitialized?state.moveBlend:state.moving?1.f:0.f;
 const float moving=(e.boarWindup>0||e.boarRecovery>0||e.windup>0||e.strike>0)?0.f:movement;
 const float error=std::abs(std::remainder(state.targetAngle-state.facingAngle,2*PI));
 const float pivot=(1-moving)*std::min(error*.75f,.7f)*(1-std::clamp(head.x/12.f,0.f,1.f));
 const float bob=.55f*std::sin(4*state.idleTime)*(1-moving)+
                 (rushing?2.f:1.25f)*std::cos(4*PI*state.phase)*moving;
 constexpr float touchdown[]={.5f,0,0,0,.75f,.25f};
 std::array<Part,6> result{};
 for(int i=0;i<6;i++){
  const auto a=Joints[view.base][i],b=Joints[view.nextBase()][i];
  auto& part=result[i];
  part.pivot={.8f*turn_art::mix(a.x,b.x,view.t),.8f*turn_art::mix(a.y,b.y,view.t)};
  if(i==2){
   part.shift={forward*head.x*.72f,depth*head.x*.54f-head.y-bob};
   part.angle=forward*(head.x*1.55f+1.7f*std::sin(4*PI*state.phase)*moving);
  }else if(i==3){
   part.shift={forward*head.x*.23f,depth*head.x*.18f-head.y*.22f-bob};
   part.angle=forward*head.x*.32f;
  }else{
   const auto phase=[](float t){return t-std::floor(t);};
   const float stridePhase=phase(state.phase-touchdown[i]);
   const auto hoof=rushing?track(cinderhorn_synfig::rush,stridePhase):track(cinderhorn_synfig::walk,stridePhase);
   const auto swivel=track(cinderhorn_synfig::walk,phase(2*state.idleTime-touchdown[i]));
   const float x=hoof.x*moving+.27f*swivel.x*pivot;
   const float y=hoof.y*moving+.31f*swivel.y*pivot;
   // Keep the hip attached while translating the hoof along the ground.
   // Stretch and rotate the cutout about that joint to its target.
   const auto& tileA=cinderhorn_turn_atlas::tiles[view.sample][i];
   const auto& tileB=cinderhorn_turn_atlas::tiles[view.nextSample()][i];
   const float footX=.8f*turn_art::mix(tileA.ox+tileA.w*.5f,tileB.ox+tileB.w*.5f,view.mix);
   const float footY=.8f*turn_art::mix(tileA.oy+tileA.h-1,tileB.oy+tileB.h-1,view.mix);
   const Vector2 base{footX-part.pivot.x,footY-part.pivot.y};
   const Vector2 anchor{forward*head.x*.20f,depth*head.x*.16f-head.y*.20f-bob};
   const Vector2 wanted{base.x+forward*x-anchor.x,base.y+.52f*depth*x-y-anchor.y};
   const float scale=std::clamp(std::hypot(wanted.x,wanted.y)/std::max(1.f,std::hypot(base.x,base.y)),.59f,1.32f);
   part.shift=anchor;
   part.scaleX=part.scaleY=scale;
   part.angle=std::remainder(std::atan2(wanted.y,wanted.x)-std::atan2(base.y,base.x),2*PI)*RAD2DEG;
  }
 }
 return result;
}

inline Rectangle bounds(const Enemy& e){
 const auto view=turn_art::view(e.animation,turn_art::cinderhorn);
 const auto parts=pose(e,view);
 const turn_art::Rig rig(turn_art::cinderhorn,view,0,parts,WHITE,1);
 constexpr float size=cinderhorn_gait::Scale;
 return {rig.box.x*size,rig.box.y*size,rig.box.width*size,rig.box.height*size};
}

inline bool draw(const Enemy& e,float x,float y,Color outline,const Vector2* probe,bool shadow,float scale,float opacity){
 const auto view=turn_art::view(e.animation,turn_art::cinderhorn);
 const auto parts=pose(e,view);
 turn_art::Rig rig(turn_art::cinderhorn,view,0,parts,WHITE,1);
 const float s=scale*cinderhorn_gait::Scale;
 if(probe)return opacity>=.43f&&rig.hit({(probe->x-x)/s,(probe->y-y)/s});
 rlPushMatrix();
 rlTranslatef(x,y,0);rlScalef(s,s,1);
 rig.begin({},0);
 if(outline.a&&!shadow){
  outline.a=static_cast<unsigned char>(outline.a*opacity);
  for(const auto d:{Vector2{1.5f,0},Vector2{-1.5f,0},Vector2{0,1.5f},Vector2{0,-1.5f}})
   rig.paint(true,outline,d.x/s,d.y/s);
 }
 rig.paint(shadow,Fade(WHITE,opacity),0,0,shadow);
 if(turn_art::filtered())EndShaderMode();
 rlPopMatrix();
 return false;
}
}
