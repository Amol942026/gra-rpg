#pragma once

// Included inside Game. The existing saved swing clock carries the windup
// across saves. Crossing the impact time resolves the thrust exactly once.
bool spearActive()const{return meleeDuration==SpearDuration&&swing>0;}
void cancelSpear(){if(spearActive())swing=0;}
bool startSpear(){
 if(length(facing)<.001f)facing={0,-1};
 animation.face(facing.x,facing.y);
 animation.targetAngle=std::atan2((facing.x+facing.y)*.5f,facing.x-facing.y);
 animation.stopSprint();animation.swordStow=0;
 attackCd=SpearCooldown;meleeDuration=swing=SpearDuration;swingStrong=false;
 sound(Cue::SpearThrust,player);
 return true;
}
void updateSpear(float previousSwing){
 constexpr float impactRemaining=SpearDuration-SpearImpactTime;
 if(meleeDuration!=SpearDuration||previousSwing<=impactRemaining||swing>impactRemaining)return;
 if(archetype(equipment[0].kind)!=Kind::Spear||sandTrapped()||knockedBack()||rolling()||inTower())return;
 const V delta=normalized(facing)*SpearReach;
 float block=2.f;
 auto stopAt=[&](V center,float radius){block=std::min(block,segmentCircle(player,delta,center,radius+SpearTipRadius));};
 for(const auto& prop:props)stopAt(prop.pos,propRadius(prop.type));
 if(hasFountains())for(V center:fountains())stopAt(center,FountainRadius);
 const V end=player+delta;
 if(end.x<MapWalkMin||end.x>MapWalkMax)block=std::min(block,(std::clamp(end.x,MapWalkMin,MapWalkMax)-player.x)/delta.x);
 if(end.y<MapWalkMin||end.y>MapWalkMax)block=std::min(block,(std::clamp(end.y,MapWalkMin,MapWalkMax)-player.y)/delta.y);
 Enemy* first=nullptr;float contact=block;
 for(auto& enemy:enemies)if(enemy.alive){
  const float t=segmentCircle(player,delta,enemy.pos,enemy.radius()+SpearTipRadius);
  if(t<contact){contact=t;first=&enemy;}
 }
 if(contact>1.f)return;
 sound(Cue::SpearImpact,player+delta*std::clamp(contact,0.f,1.f));
 if(first)hit(*first,float(damage()),DamageType::Physical,rollCritical());
}
