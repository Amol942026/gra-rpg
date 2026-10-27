#pragma once
#include "../src/hero_knockdown.hpp"
Game guardianArena(){
 Game g;g.player={20,20};g.props.clear();g.enemies.clear();g.forage.clear();
 g.enemies.push_back({13,3,{16,20},{16,20},260,260});g.enemies[0].attack=1000;g.enemies[0].boulderCd=0;
 return g;
}
void guardianKnockbackTests(){
 auto landing=[](V hero){Game g=guardianArena();g.enemies.clear();g.player=hero;g.boulders.push_back({{17,20},{20,20},{19.7f,20},.45f,.5f,25,70,BoulderDamage,13});return g;};
 check(std::abs(HeroKnockdownDuration-hero_motion::knockdown_synfig::Duration)<.00001f,"game clock matches the authored Synfig fall/get-up clip");
 auto base=hero_motion::evaluate(0,0,0,0);
 for(int side=0;side<8;side++){
  hero_motion::Vec away{std::cos(side*3.14159265f/4),std::sin(side*3.14159265f/4),0};
  auto down=hero_knockdown::evaluate(base,.36f,away),rising=hero_knockdown::evaluate(base,.8f,away),end=hero_knockdown::evaluate(base,1,away);
  auto head=down.point(hero_motion::data::Head,{0,0,70});
  check(head.z<45&&head.z>5&&rising.world[hero_motion::data::Head].p.z>down.world[hero_motion::data::Head].p.z,"head and torso really fall to the ground and rise in every impulse direction");
  for(int b=0;b<hero_motion::data::BoneCount;b++)check(hero_motion::length(end.world[b].p-base.world[b].p)<.00001f,"get-up ends exactly in the original idle pose without a pop");
  for(int f=0;f<=hero_motion::knockdown_synfig::Frames;f++){
   auto p=hero_knockdown::evaluate(base,f/float(hero_motion::knockdown_synfig::Frames),away);
   for(const auto& joint:p.world)check(std::isfinite(joint.p.z)&&joint.p.z>=0,"all fall/get-up joints stay finite and above the ground plane");
  }
 }
 auto air=hero_knockdown::evaluate(base,.24f/HeroKnockdownDuration,{0,-1,0});
 check(air.world[hero_motion::data::FootL].p.z>15&&air.world[hero_motion::data::FootR].p.z>15,"both feet leave the ground during the backward flight");
 auto supine=hero_knockdown::evaluate(base,.8f/HeroKnockdownDuration,{0,-1,0});
 check(supine.vector(hero_motion::data::Spine,{0,1,0}).z>.98f&&supine.world[hero_motion::data::Head].p.z<15,"grounded rest is on the back with chest and face upwards");
 for(float age:{.7f,1.1f,1.65f}){
  auto at=landing({20,20});at.updateBoulders(.05f,at.player);for(int i=0;i<int(age*60+.1f);i++)at.update(1.f/60,{},false);
  check(at.knockedBack()&&!at.attack()&&!at.startRoll({1,0}),"flight, supine rest and slow recovery retain the control lock");
 }
 for(int fps:{30,60,120})for(int side=0;side<8;side++)for(float radius:{.3f,1.2f}){
  V away{std::cos(side*3.14159265f/4),std::sin(side*3.14159265f/4)},start=V{20,20}+away*radius;
  auto g=landing(start);auto rng=g.rng,combat=g.combatRng;
  g.updateBoulders(.05f,start);check(g.knockedBack(),"nearby ground impact starts a radial impulse, including outside the damage radius");
  float expected=BoulderKnockbackDistance*(1-.6f*radius/BoulderKnockbackRadius);
  float hp=radius<=BoulderImpactRadius?100-g.incomingDamage(BoulderDamage,DamageType::Physical):100;
  for(int i=0;i<int(std::ceil(fps*HeroKnockdownDuration))+1;i++)g.update(1.f/fps,{},false);
  check(distance(g.player,start+away*expected)<.0001f&&!g.knockedBack(),"knockback distance and direction agree at 30, 60 and 120 Hz in eight directions");
  check(std::abs(g.hp-hp)<.001f&&g.rng==rng&&g.combatRng==combat,"shockwave adds no damage or random rolls outside the original hit");
  check(distance(g.facing,away*-1)<.0001f&&!g.animation.moving,"backfall faces the impact and does not play walking or footstep sounds");
 }
 auto g=landing({20,20});g.update(.05f,{},false);check(g.knockedBack()&&g.heroKnockback.x>0&&std::abs(g.heroKnockback.y)<.001f,"direct centre hit falls back to incoming rock direction without NaNs");
 V start=g.player;check(!g.startRoll({-1,0})&&!g.attack({-1,0})&&!g.spell({-1,0}),"new roll and attacks cannot cancel the short physical recoil");
 g.update(.05f,{-1,0},true,true);check(g.player.x>start.x&&!g.animation.sprinting,"held movement cannot reverse the impulse or turn it into sprinting");
 start=g.player;float remaining=g.heroKnockbackRemaining;g.allocationRequired=true;g.update(.05f,{},false);
 check(distance(g.player,start)==0&&g.heroKnockbackRemaining==remaining,"paused simulation preserves the remaining impulse exactly");g.allocationRequired=false;
 g.enterMageTower();check(!g.knockedBack()&&length(g.heroKnockback)==0,"travel clears the impulse before moving to the tower");
 g=landing({20,20});g.update(.05f,{},false);g.activateField(WorldLocation::Desert);check(!g.knockedBack(),"changing field clears forest knockback");
 g=landing({20,20});g.update(.05f,{},false);g.hp=0;start=g.player;g.update(.05f,{},false);check(!g.knockedBack()&&distance(start,g.player)==0,"death stops the impulse");
 g=landing({21.61f,20});start=g.player;g.update(.05f,{},false);check(!g.knockedBack()&&distance(start,g.player)==0&&g.hp==100,"hero beyond the blast radius is unaffected");
 g=landing({20,20});g.startRoll({1,0});g.update(.05f,{},false);check(!g.knockedBack()&&g.hp==100&&g.rolling(),"protected dodge negates both damage and knockback");
 g=landing({20,20});g.startRoll({1,0});g.rollRemaining=.08f;g.update(.05f,{},false);check(g.knockedBack()&&!g.rolling()&&g.hp<100,"late unprotected dodge is interrupted by the impact");
 g=landing({20.3f,20});g.equipment[0]=item(Kind::Bow);g.beginBow({1,0});g.update(.05f,{},false);check(g.knockedBack()&&!g.bowDrawing&&!g.beginBow({1,0}),"impact interrupts drawing the bow and blocks a new draw until recoil ends");
 g=landing({20.3f,20});g.props.push_back({{21.1f,20},6,1});g.update(.05f,{},false);for(int i=0;i<20;i++)g.update(.02f,{},false);
 check(g.player.x<21.1f-propRadius(6)&&g.walkable(g.player),"small collision steps stop knockback on narrow scenery");
 g=landing({20.3f,20});g.chests[0].pos={21.2f,20};g.update(.05f,{},false);for(int i=0;i<20;i++)g.update(.02f,{},false);
 check(g.player.x<21.2f-ChestRadius&&g.walkable(g.player),"knockback respects chest collision");
 g=landing({MapWalkMax-.15f,20});V target{MapWalkMax-.4f,20};g.boulders={{{MapWalkMax-3,20},target,target-V{.3f,0},.45f,.5f,25,70,BoulderDamage,13}};
 g.update(.05f,{},false);for(int i=0;i<20;i++)g.update(.02f,{},false);check(g.player.x<=MapWalkMax&&g.walkable(g.player),"knockback cannot leave map bounds");
 g=landing({20,21.3f});g.props.push_back({{20,20.65f},6,1});start=g.player;g.update(.05f,{},false);
 check(!g.knockedBack()&&distance(g.player,start)==0,"terrain between the ground impact and hero shields the shockwave");
 g=landing({20,20});g.props.push_back({{19.8f,20},6,1});g.update(.05f,{},false);check(!g.knockedBack()&&g.hp==100,"boulder intercepted by cover does not send a shockwave through it");
 g=landing({20,20});g.animation.motionInitialized=true;g.animation.swordStow=.7f;g.update(.05f,{},false);float frozenStow=g.animation.swordStow;
 for(int i=0;i<10;i++)g.update(.05f,{},false);
 check(g.knockedBack()&&g.heroKnockbackRemaining==0&&g.knockdownProgress()>.15f&&!g.startRoll({1,0}),"lying phase keeps control locked after the physical slide ends");
 check(std::abs(g.animation.swordStow-frozenStow)<.00001f,"sword stow timeline is frozen while the hero lies on the ground");
 V at=g.player;float phase=g.heroKnockdownRemaining;g.allocationRequired=true;g.update(.05f,{},false);
 check(distance(g.player,at)==0&&g.heroKnockdownRemaining==phase,"pause freezes the lying/get-up phase too");g.allocationRequired=false;
 for(int i=0;i<55;i++)g.update(.05f,{},false);
 check(!g.knockedBack()&&g.knockdownProgress()<0&&g.startRoll({1,0}),"control and dodge return after standing up");
}
void guardianBoulderTests(){
 guardianKnockbackTests();
 for(int piece=0;piece<guardian_break::Pieces;piece++){
  auto first=guardian_break::sample(piece,0),burst=guardian_break::sample(piece,.25f),last=guardian_break::sample(piece,1);auto pivot=guardian_break::pivots[piece];
  check(std::abs(first.x-pivot.x*BoulderSpriteSize/96+BoulderSpriteSize/2)<.001f&&std::abs(first.y-first.height-pivot.y*BoulderSpriteSize/96+guardian_break::RestHeight+BoulderSpriteSize/2)<.001f,"six cutouts initially assemble the same boulder at ground contact");
  check(std::hypot(burst.x-first.x,burst.y-first.y)>15&&std::abs(burst.angle)>20,"large pieces visibly separate and spin after impact");
  check(first.alpha==1&&last.alpha==0&&last.height>0,"fragments start solid, rest on the ground and fade out");
 }
 for(int fps:{30,60,120}){
  auto dust=guardianArena();dust.enemies.clear();dust.boulderImpacts.push_back({dust.player});auto rng=dust.rng,combat=dust.combatRng;
  for(int frame=0;frame<fps*3/5;frame++)dust.update(1.f/fps,{},false);
  check(dust.boulderImpacts.size()==1&&std::abs(dust.boulderImpacts[0].progress()-.5f)<.001f,"fracture phase progresses for 1.2 seconds independent of frame rate");
  float life=dust.boulderImpacts[0].life;dust.allocationRequired=true;dust.update(.05f,{},false);check(dust.boulderImpacts[0].life==life,"point allocation freezes airborne fragments");dust.allocationRequired=false;
  for(int frame=0;frame<fps;frame++)dust.update(1.f/fps,{},false);
  check(dust.boulderImpacts.empty()&&dust.hp==100&&dust.rng==rng&&dust.combatRng==combat,"cosmetic rubble expires without extra damage or consuming combat/loot RNG");
 }
 // The idle arm bends toward the palm/front. Keep that same hinge side while
 // raising the shoulder; choosing the other IK solution hyperextends the arm.
 for(int step=0;step<=guardian_motion::Frames*4;step++){
  auto p=guardian_motion::sample(float(step)/(guardian_motion::Frames*4));
  float upperF=p.elbowForward-p.bodyForward,upperZ=p.elbowUp-(53+p.bodyUp),lowerF=p.handForward-p.elbowForward,lowerZ=p.handUp-p.elbowUp;
  check(upperF*lowerZ-upperZ*lowerF>1.f,"both elbow hinges keep the natural idle bend side throughout the throw and return");
  float upperSide=p.elbowSide-23,lowerSide=p.handSide-p.elbowSide;
  float lengthA=std::sqrt(upperF*upperF+upperZ*upperZ+upperSide*upperSide),lengthB=std::sqrt(lowerF*lowerF+lowerZ*lowerZ+lowerSide*lowerSide);
  float cosine=(upperF*lowerF+upperZ*lowerZ+upperSide*lowerSide)/(lengthA*lengthB);
  check(cosine<std::cos(8*3.14159265f/180)&&cosine>std::cos(145*3.14159265f/180),"elbows stay flexed within joint limits instead of locking or folding back");
 }
 check(BoulderSpriteSize>=70&&BoulderWindup==1.5f&&BoulderRecovery==.4f,"large boulder has a faster authored two-handed windup and follow-through");
 auto crouch=guardian_motion::poses[16],overhead=guardian_motion::poses[74];
 check(crouch.bodyUp<=-15&&crouch.handUp<=13&&crouch.rockUp<=15,"both mirrored hands reach the embedded stone while the guardian crouches");
 check(overhead.rockUp-(67+overhead.bodyUp)>36&&overhead.handUp>85,"stone and both hands are above the head before the overhead throw");
 for(int frame=16;frame<=guardian_motion::ReleaseFrame;frame++){
  auto p=guardian_motion::poses[frame];float df=std::max(0.f,std::abs(p.rockForward-p.bodyForward*.8f)-11.5f),dz=std::max(0.f,std::abs(p.rockUp-(67+p.bodyUp))-11.5f);
  check(std::hypot(df,dz)>20,"lifted boulder clears the head throughout the lift and overhead swing");
  check(p.handSide>=19&&p.elbowSide>=20,"both mirrored arms pass outside the head silhouette");
  check(std::hypot(p.handForward-p.rockForward,p.handUp-p.rockUp)<18,"both grips stay against the lower sides of the same large stone");
 }
 for(int direction=0;direction<8;direction++){
  auto arena=guardianArena();auto& boss=arena.enemies[0];boss.animation.face(std::cos(direction*3.14159265f/4),std::sin(direction*3.14159265f/4));auto p=guardian_motion::Release;
  auto rock=Game::boulderHand(boss,BoulderWindup/(BoulderWindup+BoulderRecovery)),left=Game::guardianSocket(boss,-p.handSide,p.handForward,p.handUp),right=Game::guardianSocket(boss,p.handSide,p.handForward,p.handUp);
  check(distance((left.ground+right.ground)*.5f,rock.ground)<.00001f&&std::abs(left.height-right.height)<.00001f,"two-hand grip stays centered on release in every facing direction");
 }
 Game start;check(distance(start.player,ForestArrival)<.00001f&&start.walkable(start.player),"hero starts at walkable westernmost corner");
 check(start.player.x==MapWalkMin&&start.player.y==MapWalkMax&&distance(start.player,ForestGuardian)>65,"isometric x-y is minimal and guardian is over 65 tiles away");
 check(start.skillRanks==SkillRanks{}&&start.attributePoints==0,"new arrival grants no skills or developer equipment");
 Game natural=guardianArena();natural.player={22.3f,20};natural.enemies[0].boulderCd=BoulderFirstDelay;
 for(int i=0;i<240&&natural.enemies[0].boulderThrows==0;i++)natural.update(1.f/60,{},false);
 check(natural.enemies[0].boulderThrows==1,"ordinary approach triggers the first throw without forcing the cooldown");
 for(int fps:{30,60,120}){
  float dt=1.f/fps;Game g=guardianArena();auto rng=g.rng,combat=g.combatRng;
  g.update(dt,{},false);check(g.enemies[0].boulderWindup>0&&g.boulders.empty()&&g.hp==100,"guardian visibly prepares before releasing a stone");
  V target=g.enemies[0].boulderTarget;int frames=0;
  while(g.boulders.empty()&&frames++<fps*4)g.update(dt,{},false);
  check(g.enemies[0].boulderThrows==1&&g.boulders.size()==1&&g.hp==100,"one stone is released after one full windup");
  check(distance(g.boulders[0].target,target)<.00001f&&g.boulders[0].height>50,"locked target and elevated hand launch are retained");
  float damage=g.incomingDamage(BoulderDamage,DamageType::Physical);
  while(g.boulderImpacts.empty()&&frames++<fps*5)g.update(dt,{},false);
  check(g.boulders.empty()&&std::abs(g.hp-(100-damage))<.001f,"landing hits once with physical armor at all tested FPS");
  check(g.boulderImpacts.size()==1&&g.boulderImpacts[0].progress()<.001f,"the intact projectile is replaced by exactly one fresh fracture at terrain contact");
  float hp=g.hp;for(int i=0;i<fps/5;i++)g.update(dt,{},false);
  check(g.hp==hp&&g.enemies[0].boulderThrows==1,"landed stone cannot damage repeatedly and cooldown prevents spam");
  check(g.rng==rng&&g.combatRng==combat,"boss throws do not alter loot or player critical RNG");
 }
 Game g=guardianArena();g.update(.05f,{},false);g.player={20,22};
 while(g.boulders.empty())g.update(.05f,{},false);
 V locked=g.boulders[0].target;
 check(distance(locked,{20,22})<.00001f,"windup follows the current hero position until actual hand release");
 g.player={23,23};for(int i=0;i<25;i++)g.update(.05f,{},false);
 check(g.hp==100&&g.enemies[0].boulderThrows==1,"moving after release dodges the committed airborne stone without homing");
 for(int fps:{30,60,120}){
  auto aim=guardianArena();float dt=1.f/fps;aim.update(dt,{},false);int steps=0;
  while(aim.boulders.empty()&&steps++<fps*3){aim.player={20,20+std::sin(steps*.04f)};V live=aim.player;aim.update(dt,{},false);check(distance(aim.enemies[0].boulderTarget,live)<.00001f,"tracking remains current on every windup tick");}
  check(!aim.boulders.empty()&&steps<=int(fps*BoulderWindup)+1,"quicker lift releases within the new authored windup at all simulation rates");
  check(aim.boulders[0].duration<.6f,"quicker projectile crosses the arena in under 0.6 seconds");
 }

 g=guardianArena();g.update(1.f/60,{},false);V beforeHit=g.player;
 g.heroKnockback={BoulderKnockbackDistance,0};g.heroKnockbackRemaining=BoulderKnockbackDuration;g.heroKnockdownRemaining=HeroKnockdownDuration;
 while(g.boulders.empty())g.update(1.f/60,{},false);
 check(distance(g.player,beforeHit)>1.3f&&distance(g.boulders[0].target,g.player)<.00001f,"throw after a previous knockback aims at the displaced hero, not the pre-hit position");
 g=guardianArena();g.props.push_back({{18,20},0,1});g.update(.05f,{},false);
 check(g.enemies[0].boulderWindup==0&&g.enemies[0].boulderThrows==0,"guardian cannot begin a throw through scenery");
 g=guardianArena();g.update(.05f,{},false);while(g.boulders.empty())g.update(.05f,{},false);
 V mid=(g.boulders[0].origin+g.boulders[0].target)*.5f;g.props.push_back({mid,3,1});
 for(int i=0;i<25;i++)g.update(.05f,{},false);
 check(g.hp==100&&g.boulders.empty(),"swept rock collision hits cover before the hero");
 auto landing=[](){Game a=guardianArena();a.enemies.clear();a.boulders.push_back({{17,20},{20,20},{19.7f,20},.45f,.5f,25,70,BoulderDamage,13});return a;};
 g=landing();g.startRoll({1,0});g.update(.05f,{},false);
 check(g.hp==100&&std::any_of(g.sounds.begin(),g.sounds.end(),[](const SoundEvent& s){return s.cue==Cue::Dodge;}),"protected roll negates impact even while inside the landing radius");
 g=landing();g.startRoll({1,0});g.rollRemaining=.08f;g.update(.05f,{},false);
 check(g.hp<100,"late unprotected roll does not provide permanent immunity");
 g=landing();g.hp=1;g.update(.05f,{},false);check(g.dead(),"stone can defeat the hero and triggers ordinary health rules");
 g=guardianArena();g.update(.05f,{},false);g.hit(g.enemies[0],10000);
 check(g.enemies[0].boulderWindup==0&&g.boulders.empty(),"killing the guardian cancels a pending throw");
 g=guardianArena();g.update(.05f,{},false);while(g.boulders.empty())g.update(.05f,{},false);g.hit(g.enemies[0],10000);
 check(g.boulders.empty(),"victory removes the dead boss's airborne stones");
 g=guardianArena();g.update(.05f,{},false);float windup=g.enemies[0].boulderWindup;g.allocationRequired=true;g.update(.05f,{},false);
 check(g.enemies[0].boulderWindup==windup&&g.boulders.empty(),"mandatory level allocation freezes preparation and projectiles");
 g=guardianArena();g.update(.05f,{},false);g.enterMageTower();
 check(g.boulders.empty()&&g.enemies[0].boulderWindup==0,"tower travel cancels telegraphs and projectiles");
 g=guardianArena();g.update(.05f,{},false);g.activateField(WorldLocation::Desert);
 check(g.boulders.empty()&&g.forestState.enemies[0].boulderWindup==0,"world switch cancels pending forest attacks before storing enemies");
 g=guardianArena();g.enemies[0].type=0;g.update(.05f,{},false);check(g.enemies[0].boulderWindup==0,"ordinary enemies do not inherit the boss ability");
 g=guardianArena();g.location=g.fieldLocation=WorldLocation::Desert;g.update(.05f,{},false);check(g.enemies[0].boulderWindup==0,"stone ability is confined to the first forest boss");
 g=guardianArena();g.player={17,20};g.enemies[0].attack=0;g.update(.05f,{},false);
 check(g.enemies[0].windup>0&&g.enemies[0].boulderWindup==0,"close range retains the guardian's original melee attack");
 g=guardianArena();g.player={26,20};g.update(.05f,{},false);check(g.enemies[0].boulderWindup==0,"unengaged distant boss cannot snipe the whole map");
 g=guardianArena();for(size_t i=0;i<MaxBoulders;i++)g.boulders.push_back({{17,20},{20,20},{17,20},0,2,70,70,BoulderDamage,13});g.update(.01f,{},false);
 check(g.boulders.size()==MaxBoulders&&g.enemies[0].boulderWindup==0,"projectile count remains bounded");
}
