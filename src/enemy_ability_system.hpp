#pragma once
// Included within Game. No random draws: loot and combat-critical RNG streams
// remain independent of ability timing, particles and terrain projection.
static void faceEnemyAbility(Enemy& e){
 V d=e.abilityDirection;e.animation.face(d.x,d.y);e.animation.targetAngle=std::atan2((d.x+d.y)*.5f,d.x-d.y);
}
void cancelEnemyAbilities(){
 venomGlobs.clear();for(auto& e:enemies){if(e.abilityAge>=0)e.abilityCd=std::max(e.abilityCd,1.2f);e.abilityAge=-1;e.abilityMask=0;e.abilityDirection={};}
}
bool prepareEnemyAbility(Enemy& e,float dt,bool visible,float range){
 if(!enemyAbilityRealm(e.type,location))return false;
 if(visible||e.abilityAge>=0)e.abilityCd=std::max(0.f,e.abilityCd-dt);
 if(e.abilityAge>=0)return true;
 const auto& spec=enemyAbilityProfile(e.type);
 if(dt<=0||!visible||range<spec.minRange||range>spec.maxRange||e.abilityCd>.00001f||e.windup>0||e.strike>0||e.attack>0)return false;
 e.abilityDirection=normalized(player-e.pos);if(length(e.abilityDirection)<.5f)return false;
 e.abilityAge=0;e.abilityCd=spec.cooldown;e.abilityMask=0;e.abilityCasts++;e.behavior=EnemyBehavior::Chase;e.patrolGoalActive=false;e.patrolBlocked=0;e.recoil={};
 faceEnemyAbility(e);sound(enemyAttackCue(e.type),e.pos);return true;
}
void enemyAbilityDamage(float amount,DamageType type,V at,bool wasRolling,float rollAge,float impactTime){
 if(dead())return;
 if(wasRolling&&!sandTrapped()&&rollAge+impactTime<RollProtection){sound(Cue::Dodge,at);effects.push_back({at,"UNIK",.65f,3});return;}
 float damage=incomingDamage(amount,type);hp=std::max(0.f,hp-damage);hurt=.25f;sound(dead()?Cue::Death:Cue::Hurt,at);effects.push_back({at,"-"+std::to_string(int(damage)),.8f,4});
}
void advanceEnemyAbility(Enemy& e,float dt,V casterBefore,V heroBefore,bool wasRolling,float rollAge){
 if(e.abilityAge<0||!e.alive||dt<=0)return;
 if(!enemyAbilityRealm(e.type,location)){e.abilityAge=-1;e.abilityMask=0;e.abilityDirection={};return;}
 const auto& spec=enemyAbilityProfile(e.type);EnemyAbility kind=enemyAbility(e.type);float before=e.abilityAge,after=before+dt;
 auto point=[&](float event){float t=std::clamp((event-before)/dt,0.f,1.f);return std::pair<V,V>{casterBefore+(e.pos-casterBefore)*t,heroBefore+(player-heroBefore)*t};};
 if(kind==EnemyAbility::VenomFan&&!(e.abilityMask&1)&&after+.000001f>=spec.windup){
  e.abilityMask|=1;V origin=point(spec.windup).first+e.abilityDirection*.5f;
  if(venomGlobs.size()<=21)for(float angle:{-.28f,0.f,.28f})venomGlobs.push_back({origin,rotateAbilityDirection(e.abilityDirection,angle)*6.f,7.f,e.damage()*spec.damageScale,e.id});
  sound(Cue::VenomCast,e.pos);
 }
 if(kind==EnemyAbility::ObsidianCleave&&!(e.abilityMask&1)&&after+.000001f>=spec.windup){
  e.abilityMask|=1;auto [origin,hero]=point(spec.windup);
  if(insideEnemyCleave(hero-origin,e.abilityDirection)&&clearEnemySegment(origin,hero,.04f))enemyAbilityDamage(e.damage()*spec.damageScale,spec.damageType,hero,wasRolling,rollAge,std::clamp(spec.windup-before,0.f,dt));
  sound(Cue::ObsidianCleave,e.pos);
 }
 if(kind==EnemyAbility::EmberBreath)for(int pulse=0;pulse<3;pulse++){
  float event=spec.windup+pulse*.35f;if((e.abilityMask&(1<<pulse))||after+.000001f<event)continue;e.abilityMask|=1<<pulse;auto [origin,hero]=point(event);
  if(insideEmberBreath(hero-origin,e.abilityDirection)&&clearEnemySegment(origin,hero,.04f))enemyAbilityDamage(e.damage()*spec.damageScale/3,spec.damageType,hero,wasRolling,rollAge,std::clamp(event-before,0.f,dt));
  if(pulse==0)sound(Cue::FireBreath,origin);
 }
 if(kind==EnemyAbility::CopperWave&&!(e.abilityMask&1)&&after>=spec.windup&&before<spec.windup+spec.active){
  float a=std::max(before,spec.windup),b=std::min(after,spec.windup+spec.active),from=(a-before)/dt,to=(b-before)/dt;
  V centerA=casterBefore+(e.pos-casterBefore)*from,centerB=casterBefore+(e.pos-casterBefore)*to,heroA=heroBefore+(player-heroBefore)*from,heroB=heroBefore+(player-heroBefore)*to;
  float radius=.65f+2.75f*(a-spec.windup)/spec.active,growth=2.75f*(b-a)/spec.active;
  float t=expandingWaveContact(heroA-centerA,(heroB-heroA)-(centerB-centerA),radius,growth,.16f+HeroBodyRadius);
  if(t<=1){V at=heroA+(heroB-heroA)*t,origin=centerA+(centerB-centerA)*t;if(clearEnemySegment(origin,at,.04f)){e.abilityMask|=1;enemyAbilityDamage(e.damage()*spec.damageScale,spec.damageType,at,wasRolling,rollAge,(a-before)+(b-a)*t);}}
  if(before<spec.windup)sound(Cue::CopperWave,e.pos);
 }
 e.abilityAge=after;
 if(e.abilityAge>=spec.duration()){e.abilityAge=-1;e.abilityMask=0;e.abilityDirection={};}
}
void updateVenomGlobs(float dt,V heroBefore,bool wasRolling,float rollAge){
 if(!inDesert()||dt<=0)return;
 for(size_t i=0;i<venomGlobs.size();){auto& glob=venomGlobs[i];V delta=glob.velocity*dt;float obstacle=2;
  for(const auto& p:props)obstacle=std::min(obstacle,segmentCircle(glob.pos,delta,p.pos,propRadius(p.type)+.12f));
  for(const auto& chest:chests)if(chest.location==location)obstacle=std::min(obstacle,segmentCircle(glob.pos,delta,chest.pos,ChestRadius+.12f));
  for(V fountain:fountains())obstacle=std::min(obstacle,segmentCircle(glob.pos,delta,fountain,FountainRadius+.12f));
  float contact=segmentCircle(glob.pos-heroBefore,delta-(player-heroBefore),{},.42f);bool hit=contact<=1&&contact<=obstacle;
  if(hit){
   enemyAbilityDamage(glob.damage,DamageType::Magic,heroBefore+(player-heroBefore)*contact,wasRolling,rollAge,dt*contact);
   // A fan is one attack: overlapping bolts at melee range cannot triple-hit.
   int owner=glob.owner;size_t next=0;for(size_t j=0;j<i;j++)next+=venomGlobs[j].owner!=owner;
   venomGlobs.erase(std::remove_if(venomGlobs.begin(),venomGlobs.end(),[&](const auto& shot){return shot.owner==owner;}),venomGlobs.end());i=next;continue;
  }
  glob.pos=glob.pos+delta;glob.remaining-=length(delta);
  if(obstacle<=1||glob.remaining<=0||glob.pos.x<MapWalkMin||glob.pos.y<MapWalkMin||glob.pos.x>MapWalkMax||glob.pos.y>MapWalkMax)venomGlobs.erase(venomGlobs.begin()+i);else i++;
 }
}
