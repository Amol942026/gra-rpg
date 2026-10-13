#pragma once
Game sandArena(){
 Game g;g.activateField(WorldLocation::Desert);g.props.clear();g.enemies.clear();g.forage.clear();g.player=DesertBossHome+V{4,0};
 g.attributes[int(Attribute::Vitality)]=1000;g.hp=float(g.maxHp());
 g.enemies.push_back({DesertBossId,DesertBossType,DesertBossHome,DesertBossHome,560,560});g.enemies[0].attack=1000;
 g.skillRanks={1,1,1};return g;
}
void sandPrisonTests(){
 for(int fps:{30,60,120}){
  Game g=sandArena();const float dt=1.f/fps;
  for(int i=0;i<10*fps-1;i++)g.update(dt,{},false);
  check(!g.sandTrapped()&&g.enemies[0].sandPrisonCasts==0,"boss waits ten full seconds of visible combat before casting");
  g.update(dt,{},false);check(g.sandTrapped()&&g.enemies[0].sandPrisonCasts==1&&std::abs(g.sandPrisonRemaining-3)<.0001f,"first cast imprisons for exactly three seconds at 30/60/120 Hz");
  V held=g.player;auto random=g.combatRng;float mana=g.mana;float enemyHp=g.enemies[0].hp;int ammo=g.ammoCount();
  check(!g.startRoll({1,0})&&!g.attack()&&!g.strongCut()&&!g.spell({1,0}),"sand blocks dodge, sword, strong cut and fireball");
  check(g.mana==mana&&g.enemies[0].hp==enemyHp&&g.combatRng==random,"blocked attacks spend no mana, deal no damage and consume no critical roll");
  check(g.selectWeapon(1)&&!g.preparePiercingArrow()&&!g.beginBow({1,0})&&!g.releaseBow()&&g.ammoCount()==ammo,"bow and piercing arrow stay blocked after switching weapons");
  g.move({1,1});check(distance(g.player,held)==0,"direct movement cannot bypass sand imprisonment");
  g.selectWeapon(0);
  for(int i=0;i<3*fps-1;i++)g.update(dt,{1,0},true,true);
  check(g.sandTrapped()&&distance(g.player,held)<.0001f&&g.enemies[0].hp==enemyHp,"held movement, sprint and update attack events stay blocked for the full trap duration");
  g.update(dt,{1,0},false,true);check(!g.sandTrapped()&&distance(g.player,held)<.001f,"trap expires at three seconds without applying a whole free movement frame early");
  g.update(dt,{1,0},false,true);check(distance(g.player,held)>.01f&&g.attack(),"movement and new attacks work immediately after release");
  g.player=held;g.enemies[0].attack=1000;
  for(int i=0;i<7*fps-2;i++)g.update(dt,{},false);
  check(g.enemies[0].sandPrisonCasts==1&&!g.sandTrapped(),"next prison never casts before another ten seconds from the previous cast");
  g.update(dt,{},false);check(g.enemies[0].sandPrisonCasts==2&&g.sandTrapped(),"independent boss cooldown repeats every ten seconds, rather than ten seconds after release");
 }
 Game g=sandArena();g.enemies[0].sandPrisonCd=.05f;g.startRoll({1,0});g.update(.05f,{},false);
 check(g.sandTrapped()&&!g.rolling()&&!g.animation.moving,"capture immediately interrupts an already active dodge");
 g=sandArena();g.selectWeapon(1);g.preparePiercingArrow();g.beginBow({1,0});g.enemies[0].sandPrisonCd=.05f;g.update(.05f,{},false);
 check(g.sandTrapped()&&!g.bowDrawing&&!g.piercingArrowArmed&&g.arrows.empty()&&g.ammoCount()==20&&g.mana==100,"capture cancels a drawn piercing shot without wasting ammunition or mana");
 check(!g.releaseBow()&&g.arrows.empty(),"releasing mouse after capture cannot fire the cancelled bow");
 g=sandArena();g.strongCut();g.imprisonInSand(g.enemies[0]);check(g.swing==0&&!g.strongSwingActive(),"capture ends a sword skill pose immediately");
 g=sandArena();g.imprisonInSand(g.enemies[0]);g.hp-=40;float before=g.hp;
 check(g.heal()&&g.hp==before+30&&g.sandTrapped(),"healing remains available and does not free the captive player");
 g.potionCd=0;before=g.hp;check(g.use({Place::Bag,0})&&g.hp>before&&g.sandTrapped(),"potions remain available during imprisonment");
 g=sandArena();g.spell({0,1});V shot=g.fireballs.front().pos;g.imprisonInSand(g.enemies[0]);g.update(.05f,{},false);
 check(!g.fireballs.empty()&&distance(g.fireballs.front().pos,shot)>0,"projectiles launched before the trap keep flying while the hero is captive");
 g=sandArena();g.imprisonInSand(g.enemies[0]);float trap=g.sandPrisonRemaining,cd=g.enemies[0].sandPrisonCd;g.allocationRequired=true;g.update(.05f,{1,0},true);
 check(g.sandPrisonRemaining==trap&&g.enemies[0].sandPrisonCd==cd,"level allocation pauses both imprisonment and boss cooldown");
 g=sandArena();g.enemies[0].sandPrisonCd=.05f;g.props.push_back({g.player+V{-2,0},9,1});g.enemies[0].pos=DesertBossHome;
 g.update(.05f,{},false);check(!g.sandTrapped()&&g.enemies[0].sandPrisonCasts==0,"boss cannot imprison through an obstacle that blocks sight");
 g=sandArena();g.enemies[0].sandPrisonCd=.05f;g.player={9,19};g.update(.05f,{},false);
 check(!g.sandTrapped()&&g.enemies[0].sandPrisonCd==.05f,"remote patrol cannot cast or advance the engagement cooldown");
 g=sandArena();g.enemies[0].type=8;g.enemies[0].sandPrisonCd=0;g.update(.05f,{},false);check(!g.sandTrapped(),"ordinary desert creatures never gain the boss skill");
 g=sandArena();g.location=WorldLocation::Forest;g.enemies[0].sandPrisonCd=0;g.update(.05f,{},false);check(!g.sandTrapped(),"sand imprisonment is restricted to the desert boss encounter");
 g=sandArena();g.imprisonInSand(g.enemies[0]);g.enterMageTower();check(!g.sandTrapped()&&g.inTower(),"tower return clears transient sand imprisonment");
 g=sandArena();g.imprisonInSand(g.enemies[0]);g.activateField(WorldLocation::Forest);check(!g.sandTrapped(),"changing realms clears sand imprisonment");
 g=sandArena();g.imprisonInSand(g.enemies[0]);g.hp=0;g.update(.05f,{},false);check(!g.sandTrapped(),"death removes imprisonment and its visual status");
 g=sandArena();g.imprisonInSand(g.enemies[0]);g.player=DesertPortal;check(!g.useDesertPortal(),"a captive hero cannot escape through the travel interaction");
 g=sandArena();g.player=g.enemies[0].pos+V{1.7f,0};g.enemies[0].attack=0;g.enemies[0].sandPrisonCd=.05f;before=g.hp;
 for(int i=0;i<25;i++)g.update(.05f,{},false);
 check(g.sandTrapped()&&g.hp<before,"ordinary enemy attacks continue during sand imprisonment");
 g=sandArena();g.enemies[0].sandPrisonCd=7;g.activateField(WorldLocation::Forest);g.activateField(WorldLocation::Desert);
 check(g.enemies[0].sandPrisonCd==7,"boss engagement cooldown survives leaving and revisiting its realm");
 // All visual routes and their entrances use the same actual walkable corridor.
 g=Game{};g.activateField(WorldLocation::Desert);
 for(const auto& road:desertRoads()){
  bool clear=true;
  for(V p:road.points)if(!desertWater(p)&&distance(p,DesertPortal)>1)clear&=g.walkable(p);
  check(clear,"the whole curved caravan route stays clear of actual generated cactus, rocks and palm collisions");
 }
}
