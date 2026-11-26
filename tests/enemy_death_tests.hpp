#pragma once

void enemyDeathTests(){
 for(int type:{1,BoarType,DuneEyeType,VolcanoBossType,CinderhornType})for(int direction=0;direction<8;direction++){
  for(int side:{1,-1}){
   Game arena;arena.developerSession=true;
   Enemy e{50,type,{20,20},{20,20},10,100};
   e.animation.direction=Direction(direction);e.animation.facingAngle=direction*AnimationPi/4+.099f;e.animation.facingInitialized=true;
   const V impulse=flankImpulse(e.animation,side);
   arena.hit(e,2,DamageType::Physical,false,impulse*-1);
   check(e.alive&&length(e.deathDirection)==0,"a nonlethal hit never sets the persistent corpse side");
   arena.hit(e,10000,DamageType::Physical,false,impulse);
   check(e.dying()&&e.deathSide()==side&&distance(e.deathDirection,impulse)<.0001f,"the lethal hit determines the opposite flank independently of the previous hit");
   arena.hit(e,10000,DamageType::Physical,false,impulse*-1);
   check(e.deathSide()==side&&distance(e.deathDirection,impulse)<.0001f,"an already fallen corpse retains its impact side");
  }
 }
 for(int type=0;type<EnemyTypeCount;type++){
  auto first=enemy_death::sample(type,0),last=enemy_death::sample(type,1);
  check(first.collapse==0&&first.recoil==0&&last.collapse==1&&last.settle==0,"death clip starts live and ends motionless");
  const float impactPhase=enemy_death::groundContact(type);
  bool held=true;
  for(float phase:{impactPhase,(1+impactPhase)*.5f,1.f}){
   const auto pose=enemy_death::sample(type,phase);
   held&=std::abs(pose.collapse-1)<.00001f&&std::abs(pose.recoil)<.00001f&&pose.settle==0;
  }
  check(held,"the authored body pose is held from ground impact onward");
  const float contact=enemy_death::groundContactTime(type);
  const float restingTime=enemy_death::duration(type)-contact;
  constexpr int marshContactFrames[]={26,25,30,42};
  const int transformationFrames=marshEnemy(type)?marshContactFrames[type-PeatToad]:
                                 type==StoneGuardianType?16:type==AshWraithType?24:
                                 type==FireImpType?23:type==9||type==ObsidianSentinelType?25:
                                 type==VolcanoBossType?39:type==DuneJackalType?23:type==GlassReaperType?27:0;
  const float expectedScale=type==DuneJackalType?.70f:1.f;
  const bool contactTiming=transformationFrames
       ? std::abs(contact-transformationFrames*expectedScale/52.f)<.0001f
       : contact>=.15f&&contact<=.33f;
  const bool restingTiming=type==FireImpType?std::abs(restingTime-7.f/52)<.0001f:
                           type==9?std::abs(restingTime-13.f/52)<.0001f:
                           type==StoneGuardianType?std::abs(restingTime-10.f/52)<.0001f:
                           type==DuneJackalType?std::abs(restingTime-9.f*expectedScale/52)<.0001f:
                           type==GlassReaperType?std::abs(restingTime-11.f/52)<.0001f:restingTime>=.11f&&restingTime<=.12f;
  check(contactTiming&&restingTiming,
        "the fall reaches contact at the authored time and holds the pose for ground particles");
  for(float rate:{30.f,60.f,120.f}){
   Game arena(104);
   arena.player={20,20};arena.props.clear();arena.enemies.clear();arena.developerSession=true;
   const V anchor{21.25f,20};
   arena.enemies.push_back({42,type,anchor,anchor,100,100});
   Enemy& e=arena.enemies.front();e.animation.phase=.25f;e.animation.moving=true;e.animation.direction=Direction::NW;e.windup=.2f;
   const auto cry=enemyDyingCue(type);
   arena.hit(e,4);
   check(e.alive&&!e.dying()&&cueCount(arena,cry)==0,"ordinary damage never starts the death clip or voice");
   arena.hit(e,10000);
   check(!e.alive&&e.dying()&&!e.corpseGrounded()&&e.deathProgress()==0&&e.deathMotion.phase==.25f&&e.deathMotion.moving&&e.deathWindup==.2f,
         "lethal damage captures the live pose and immediately marks the enemy dead");
   check(arena.findEnemy(e.id)==nullptr&&arena.findCorpse(e.id)==&e&&arena.openCorpse(e.id),"corpse and loot are available throughout the visual clip");
   const auto landing=enemyFallCue(type);
   check(enemyDyingCue(cry)&&cueCount(arena,cry)==1&&enemyLandingCue(landing)&&cueCount(arena,landing)==0,
         "the death voice starts with the lethal pose, before a separate ground contact cue");
   const auto rng=arena.rng;arena.hit(e,10000);
   check(arena.kills==1&&arena.rng==rng&&e.deathProgress()==0,"dead enemy cannot restart death or reroll loot");
   const float dt=1/rate;
   float previous=0,elapsed=0,landed=-1;
   bool advancing=true,groundedOnCue=true;
   // The death timeline also advances while the player is allocating a level.
   arena.allocationRequired=rate==30.f;
   while(e.dying()&&elapsed<2){
    arena.update(dt,{},false);elapsed+=dt;
    if(landed<0&&cueCount(arena,landing)>0)landed=elapsed;
    groundedOnCue&=e.corpseGrounded()==(landed>=0);
    const float progress=e.deathProgress();
    advancing&=progress>=previous&&distance(e.pos,anchor)==0;
    previous=progress;
   }
   check(advancing&&!e.dying()&&std::abs(elapsed-enemy_death::endTime(type))<dt+.001f,
         "the body and its particles reach the same endpoint at 30, 60 and 120 Hz");
   arena.update(.05f,{},false);
   check(landed>=contact-.0001f&&landed<=contact+dt+.0001f&&cueCount(arena,landing)==1&&cueCount(arena,cry)==1,
         "death voice and subsequent material contact each play once at 30/60/120 Hz");
   check(groundedOnCue&&e.corpseGrounded(),"the body locks on exactly the world update that emits the ground cue");
  }
 }
 const auto contact=enemy_death::groundContactTime;
 for(const auto& pair:std::array<std::array<int,2>,7>{{
     {7,6},{BoarType,1},{DesertBossType,8},{8,9},
     {CinderhornType,13},{17,FireImpType},{DesertBossType,10}}})
  check(contact(pair[0])<contact(pair[1]),"a heavier creature reaches the ground sooner than a lighter creature");
 Game travel;
 const int id=travel.enemies.front().id;
 travel.hit(travel.enemies.front(),10000);
 check(travel.enemies.front().dying(),"realm travel fixture has an active death clip");
 travel.activateField(WorldLocation::Desert);travel.activateField(WorldLocation::Forest);
 check(travel.findCorpse(id)&&!travel.findCorpse(id)->dying(),"returning to a realm shows its settled corpses");
 travel.sounds.clear();travel.advanceEnemyDeaths(.05f);
 check(travel.sounds.empty(),"changing fields clears the abandoned ground contact event");
}
