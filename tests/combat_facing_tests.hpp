#pragma once

void combatFacingTests(){
 const V aims[]={{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1}};
 auto angleError=[](const Game& g){return std::abs(std::remainder(g.animation.targetAngle-g.animation.facingAngle,2*AnimationPi));};
 for(int fps:{30,60,120})for(V aim:aims)for(bool running:{false,true}){
  const float dt=1.f/fps;const V movement=aim*-1;
  for(bool strong:{false,true}){
   Game g=emptyArena();g.facing=normalized(movement);g.animation.face(movement.x,movement.y);g.skillRanks[int(Skill::StrongCut)]=1;
   check(strong?g.strongCut(aim):g.attack(aim),"directed sword attack starts while moving in the opposite direction");
   const V before=g.player;float error=angleError(g),elapsed=0;
   while(g.swing>dt){
    g.update(dt,movement,false,running);elapsed+=dt;
    check(g.animation.direction==directionFor(aim.x,aim.y,Direction::SE)&&distance(g.facing,normalized(aim))<.0001f,"sword target facing survives walking and sprinting in all eight directions");
    check(angleError(g)<=error+.00001f,"rendered sword angle turns toward the attack target, never movement");error=angleError(g);
   }
   check(elapsed>.2f&&error<.05f&&g.animation.moving&&distance(g.player,before)>.5f&&g.animation.phase>0,"sword aiming preserves real displacement, gait and smooth turning at 30/60/120 Hz");
   for(int i=0;i<fps/2;i++)g.update(dt,movement,false,running);
   check(g.animation.direction==directionFor(movement.x,movement.y,Direction::SE)&&angleError(g)<.001f,"walking facing resumes after the sword animation ends");
  }
  Game g=emptyArena();g.selectWeapon(1);g.facing=normalized(movement);g.animation.face(movement.x,movement.y);
  check(g.beginBow(aim),"bow starts aiming opposite movement");const V before=g.player;
  for(int i=0;i<fps/2;i++)g.update(dt,movement,false,running);
  check(g.animation.direction==directionFor(aim.x,aim.y,Direction::SE)&&distance(g.facing,normalized(aim))<.0001f&&angleError(g)<.001f,"moving bow draw keeps both logical and rendered facing on the cursor");
  check(g.animation.moving&&distance(g.player,before)>.5f&&g.animation.phase>0,"bow aim does not stop locomotion animation");
  V changedAim={-aim.y,aim.x};g.aimBow(changedAim);
  for(int i=0;i<fps/2;i++)g.update(dt,movement,false,running);
  check(g.animation.direction==directionFor(changedAim.x,changedAim.y,Direction::SE)&&angleError(g)<.001f,"moving bow follows a changed cursor target");
  check(g.releaseBow()&&distance(normalized(g.arrows.back().velocity),normalized(changedAim))<.0001f,"arrow flies along the same direction as the facing pose");
  while(g.bowReleaseTime>dt){g.update(dt,movement,false,running);check(distance(g.facing,normalized(changedAim))<.0001f&&angleError(g)<.001f,"shot release retains aim while moving");}
  for(int i=0;i<fps/2;i++)g.update(dt,movement,false,running);
  check(g.animation.direction==directionFor(movement.x,movement.y,Direction::SE)&&angleError(g)<.001f,"normal movement facing resumes after bow release");
 }
 for(bool bow:{false,true}){
  Game g=emptyArena();if(bow){g.selectWeapon(1);g.beginBow({-1,0});}else g.attack({-1,0});
  check(g.startRoll({1,0}),"roll can interrupt aimed combat");g.update(.05f,{0,1},false);
  check(!g.combatFacingActive()&&g.animation.direction==directionFor(1,0,Direction::SE)&&distance(g.facing,V{1,0})<.0001f,"roll takes facing priority and clears the combat pose");
 }
 Game g=emptyArena();g.selectWeapon(1);g.player={MapWalkMax,15};g.beginBow({-1,0});
 for(int i=0;i<30;i++)g.update(1.f/60,{1,0},false);
 check(!g.animation.moving&&distance(g.facing,V{-1,0})<.0001f&&angleError(g)<.001f,"blocked movement cannot overwrite bow aim");
 g=emptyArena();g.facing={1,0};check(g.attack({})&&distance(g.facing,V{1,0})<.0001f,"zero-length melee aim safely retains the previous facing");
 const V facing=g.facing;check(!g.attack({-1,0})&&distance(g.facing,facing)==0,"rejected attack cannot change facing during cooldown");
}
