#pragma once
Game boarArena(){
 Game g;g.props.clear();g.enemies.clear();g.forage.clear();g.player={20,20};
 g.enemies.push_back({91,BoarType,{16,20},{16,20},88,88});g.enemies[0].boarChargeCd=0;
 return g;
}
void launchBoar(Game& g){
 for(int frame=0;frame<50&&g.enemies[0].boarCharges==0;frame++)g.update(.02f,{},false);
 check(g.enemies[0].boarRush>0&&g.enemies[0].boarCharges==1,"boar launches after its real AI preparation");
}
void boarChargeTests(){
 for(int fps:{30,60,120}){
  auto g=boarArena();auto rng=g.rng,combat=g.combatRng;float startHp=g.hp;
  float expected=g.incomingDamage(float(g.enemies[0].damage()),DamageType::Physical)*2;
  V start=g.enemies[0].pos;g.update(1.f/fps,{},false);
  check(g.enemies[0].boarWindup>0&&distance(g.enemies[0].pos,start)<.0001f,"charge begins with stationary warning");
  for(int i=1;i<int(fps*1.5f);i++)g.update(1.f/fps,{},false);
  check(std::abs(startHp-g.hp-expected)<.001f,"charge inflicts exactly one double-damage hit at 30/60/120 Hz");
  check(std::abs(g.enemies[0].pos.x-start.x-BoarChargeSpeed*BoarChargeDuration)<.002f,"whole committed rush covers the same distance at every tested tick rate");
  check(g.enemies[0].boarCharges==1&&g.enemies[0].boarRush==0&&g.enemies[0].boarRecovery>0,"spent rush recovers without repeated hits");
  check(g.rng==rng&&g.combatRng==combat,"charge does not alter either loot or combat random stream");
 }
 auto g=boarArena();g.update(.05f,{},false);g.player={20,22};
 for(int i=0;i<9;i++)g.update(.05f,{},false);
 check(g.enemies[0].boarWindup>0&&distance(g.enemies[0].boarAim,g.player)<.001f,"preparation tracks the current target");
 while(!g.enemies[0].boarRush)g.update(.01f,{},false);
 V heading=g.enemies[0].boarDirection,start=g.enemies[0].pos;
 check(distance(heading,normalized(g.player-start))<.001f,"release aims at current position instead of an old target");
 g.player={20,25};g.update(.05f,{},false);
 check(distance(heading,g.enemies[0].boarDirection)<.0001f&&std::abs(distance(start,g.enemies[0].pos)-BoarChargeSpeed*.05f)<.001f,"rush keeps its direction and moves much faster than walking");
 for(int i=0;i<15;i++)g.update(.05f,{},false);
 check(g.hp==g.maxHp(),"sidestepping after release avoids the charge");
 g=boarArena();launchBoar(g);g.player=g.enemies[0].pos+g.enemies[0].boarDirection*1.12f;
 check(g.startRoll({0,1}),"dodge starts immediately before contact");g.update(.02f,{},false);
 check(g.hp==g.maxHp()&&g.enemies[0].boarHit,"protected roll consumes the single charge contact without damage");
 g=boarArena();launchBoar(g);g.player=g.enemies[0].pos+g.enemies[0].boarDirection*1.12f;
 g.startRoll({0,1});g.rollRemaining=.08f;g.update(.02f,{},false);
 check(g.hp<g.maxHp(),"late roll outside protection does not negate charge damage");
 // A moving hero crosses the boar segment between endpoints; relative sweep must hit.
 g=boarArena();auto& crossing=g.enemies[0];crossing.boarRush=.5f;crossing.boarDirection={1,0};crossing.boarAim={20,20};
 g.advanceBoarCharge(crossing,.2f,{17,18},{17,22},false,0);
 check(g.hp<g.maxHp()&&crossing.boarHit,"relative sweep detects a hero crossing the rush between frames");
 for(int obstacle=0;obstacle<4;obstacle++){
  g=boarArena();launchBoar(g);float stop=18;
  if(obstacle==0)g.props.push_back({{stop,20},6,1});
  if(obstacle==1)g.chests[0].pos={stop,20};
  if(obstacle==2){g.enemies.push_back({92,1,{stop,20},{stop,20},100,100});g.enemies[1].attack=100;g.enemies[1].windup=100;}
  if(obstacle==3){g.enemies[0].pos={MapWalkMax-1,20};g.player={MapWalkMax,20};stop=MapWalkMax;}
  V at=g.enemies[0].pos;
  for(int i=0;i<15;i++)g.update(.05f,{},false);
  check(g.enemies[0].boarRush==0,"scenery, chest, living enemy and map boundary stop an active rush");
  check(g.enemies[0].pos.x<stop&&distance(at,g.enemies[0].pos)<3,"charge cannot tunnel through blocking objects");
  if(obstacle<3)check(g.hp==g.maxHp(),"hero behind the blocker is not trampled");
 }
 for(int obstacle=0;obstacle<3;obstacle++){
  g=boarArena();launchBoar(g);float gap=g.enemies[0].radius()+.002f+(obstacle==0?propRadius(6):obstacle==1?ChestRadius:enemyInfo(1).radius);
  V behind=g.enemies[0].pos-V{gap-.00002f,0},start=g.enemies[0].pos;
  if(obstacle==0)g.props.push_back({behind,6,1});
  if(obstacle==1)g.chests[0].pos=behind;
  if(obstacle==2){g.enemies.push_back({92,1,behind,behind,100,100});g.enemies[1].attack=100;g.enemies[1].windup=100;}
  g.update(.05f,{},false);
  check(std::abs(distance(start,g.enemies[0].pos)-BoarChargeSpeed*.05f)<.001f&&g.enemies[0].boarRush>0,"touching scenery, chest or creature behind the boar allows rushing away");
 }
 g=boarArena();launchBoar(g);g.enemies[0].boarRush=.01f;g.update(.02f,{},false);
 check(g.enemies[0].boarRecovery>0&&g.enemies[0].boarRush==0,"a spent charge enters a short recovery");
 g=boarArena();launchBoar(g);float elapsed=0;
 while(elapsed<BoarChargeCooldown-.1f){g.player=g.enemies[0].pos+V{4,0};g.update(.02f,{},false);elapsed+=.02f;}
 check(g.enemies[0].boarCharges==1,"boar cannot repeat its charge before cooldown expires");
 for(int i=0;i<40;i++){g.player=g.enemies[0].pos+V{4,0};g.update(.02f,{},false);}
 check(g.enemies[0].boarCharges==2,"charge becomes available again after the cooldown and next preparation");
 g=boarArena();g.enemies[0].boarChargeCd=BoarChargeFirstDelay;g.update(.05f,{},false);
 check(!g.enemies[0].boarBusy(),"initial encounter gives the player time to react");
 g=boarArena();g.player={16.8f,20};g.update(.05f,{},false);
 check(g.enemies[0].windup>0&&!g.enemies[0].boarBusy(),"boar keeps its ordinary melee attack at close range");
 g=boarArena();g.props.push_back({{18,20},0,1});g.update(.05f,{},false);
 check(!g.enemies[0].boarBusy(),"hidden target behind a tree does not trigger charge");
 g=boarArena();g.player={25,20};g.update(.05f,{},false);
 check(!g.enemies[0].boarBusy(),"target outside sight range does not trigger charge");
 for(int type=0;type<EnemyTypeCount;type++)if(type!=BoarType){g=boarArena();g.enemies[0].type=type;g.update(.05f,{},false);check(!g.enemies[0].boarBusy()&&g.enemies[0].boarCharges==0,"other species retain their own AI");}
 g=boarArena();g.location=WorldLocation::Desert;g.update(.05f,{},false);
 check(!g.enemies[0].boarBusy(),"forest charge does not activate in another region");
 g=boarArena();g.update(0,{},false);check(!g.enemies[0].boarBusy(),"zero-time update cannot start charge");
 g=boarArena();launchBoar(g);V at=g.enemies[0].pos;float clock=g.enemies[0].boarRush;g.allocationRequired=true;g.update(.05f,{},false);
 check(distance(g.enemies[0].pos,at)==0&&g.enemies[0].boarRush==clock,"allocation pause freezes charge without advancing clocks");
 g.allocationRequired=false;g.hit(g.enemies[0],10000);check(!g.enemies[0].boarBusy(),"killing a charging boar clears the attack");
 g=boarArena();launchBoar(g);g.hp=0;g.update(.05f,{},false);check(!g.enemies[0].boarBusy(),"player death cancels active charge");
 g=boarArena();launchBoar(g);g.enterMageTower();check(!g.enemies[0].boarBusy(),"tower travel cancels active charge");
 g=boarArena();launchBoar(g);g.activateField(WorldLocation::Desert);
 for(const auto& e:g.forestState.enemies)check(!e.boarBusy(),"stored forest does not resume a suspended charge after travel");
 g=boarArena();g.attributes[int(Attribute::Vitality)]=10;g.hp=float(g.maxHp());g.equipment[2]=item(Kind::GuardianArmor,3);
 float normal=g.incomingDamage(float(g.enemies[0].damage()),DamageType::Physical),hp=g.hp;launchBoar(g);
 for(int i=0;i<35;i++)g.update(.02f,{},false);
 check(std::abs(hp-g.hp-normal*2)<.001f&&hp-g.hp>normal,"charge remains stronger than melee even against high defense");
}
