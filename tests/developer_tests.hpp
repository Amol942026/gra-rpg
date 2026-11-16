#pragma once
#include "../src/developer_mode.hpp"
void developerTests(){
 auto same=[](const auto& a,const auto& b){if(a.size()!=b.size())return false;for(size_t i=0;i<a.size();i++)if(a[i].kind!=b[i].kind||a[i].tier!=b[i].tier||a[i].count!=b[i].count)return false;return true;};
 auto catalogueComplete=[](const Game& g){
  for(int id=1;id<ItemKindCount;id++){
   const Kind kind=Kind(id);const int slot=gearSlot(kind);
   if(kind==Kind::DawnShard)continue;
   if((slot<0||g.equipment[slot].kind!=kind)&&g.bagCount(kind)==0)return false;
  }
  return true;
 };
 auto unlocked=[](const Game& g){
  for(int rank:g.skillRanks)if(rank<1)return false;
  for(int i=1;i<int(Spell::Count);i++)if(!g.knowsSpell(Spell(i)))return false;
  return g.developerSession&&g.level==100&&!g.allocationRequired&&g.knowsSpell(g.selectedSpell);
 };
 for(unsigned seed:{1u,12593u,67841u})for(int destination=0;destination<DeveloperDestinationCount;destination++){
  Game g(seed);g.hp=0;g.mana=0;check(developerStart(g,destination,true,seed),"every developer destination accepts a fresh session across seeds");
  const auto& target=DeveloperDestinations[destination];check(g.location==target.location&&g.towerFloor==target.floor&&developerSpawnSafe(g,g.player),"developer spawn is in selected room and respects scenery and enemy bodies");
  check(distance(g.player,target.spawn)<=4.001f&&g.hp==g.maxHp()&&g.mana==g.maxMana()&&!g.allocationRequired&&!g.storyBlocked()&&!g.towerReturnPending,"direct start restores resources and never leaves a story or progression input lock");
  check(g.maxHp()==920&&g.maxMana()==510,"new level 100 developer gets balanced health and mana pools");
  check(unlocked(g)&&g.xp==0&&g.ammoCount()==100&&g.bagCount(Kind::Health)==50&&g.bagCount(Kind::Mana)==50,"each fresh developer start has all unlocks and exact supplies");
 }
 {
  Game arena;
  check(!summonDeveloperEnemy(arena,0),"ordinary mode rejects summoning");
  check(developerStart(arena,8,true)&&!summonDeveloperEnemy(arena,0),"tower remains a sanctuary in DEV");
  for(int destination:{0,13,16}){
   check(developerStart(arena,destination,true),"start each realm to test the full summon catalogue");
   const auto base=arena.enemies.size();
   check(!summonDeveloperEnemy(arena,-1)&&!summonDeveloperEnemy(arena,EnemyTypeCount)&&arena.enemies.size()==base,
         "unknown types do not modify the encounters");
   for(int type=0;type<EnemyTypeCount;type++){
    if(arena.enemies.size()>base)arena.enemies.pop_back();
    const bool created=summonDeveloperEnemy(arena,type);
    check(created,"every enemy is available beside the real arrival in every combat realm");
    if(!created)continue;
    const auto& e=arena.enemies.back();
    check(e.type==type&&e.alive&&e.hp==enemyInfo(type).health&&e.maxHp==e.hp&&
          arena.testSummon(e.id)&&e.behavior==EnemyBehavior::Chase&&arena.canSeePlayer(e)&&
          arena.enemyPositionClear(e.pos,e.radius())&&distance(e.pos,arena.player)<=6.001f&&
          distance(e.pos,arena.player)>e.radius()+HeroBodyRadius&&distance(e.pos,e.home)<.001f,
          "summon uses real health, walkable terrain and a local home");
   }
  }
  check(developerStart(arena,0,true),"start a fresh forest for simultaneous summons");
  arena.props.clear();arena.player={20,20};arena.enemies.clear();arena.merchantArrows=0;
  for(int type:{StoneGuardianType,DesertBossType,VolcanoBossType,StoneGuardianType}){
   const bool created=summonDeveloperEnemy(arena,type);
   check(created,"repeated calls summon multiple bosses");
   if(!created)continue;
   const auto& e=arena.enemies.back();
   for(auto i=arena.enemies.begin();i!=arena.enemies.end()-1;i++)
    check(i->id!=e.id&&distance(i->pos,e.pos)>i->radius()+e.radius(),"simultaneous summons have separate IDs and bodies");
  }
  for(auto& e:arena.enemies){
   arena.hit(e,10000);
   check(!e.alive&&std::none_of(e.loot.begin(),e.loot.end(),[](Item drop){return drop.kind==Kind::DawnShard;}),
         "a summoned boss drops ordinary loot without a quest fragment");
  }
  check(!arena.bossDead&&!arena.desertBossDead&&!arena.volcanoBossDead&&arena.merchantArrows==0,
        "summoned boss deaths preserve story and merchant stock");
  const auto count=arena.enemies.size();
  arena.props.push_back({arena.player,0,1});
  check(!summonDeveloperEnemy(arena,0)&&arena.enemies.size()==count,"a blocked location does not create an enemy through scenery");
 }
 {
  Game arena;
  check(developerStart(arena,0,true),"start the developer combat arena");
  arena.enemies.clear();arena.props.clear();arena.player={20,20};arena.facing={1,0};
  for(int type=0;type<EnemyTypeCount;type++){
   Game trial=arena;
   check(summonDeveloperEnemy(trial,type),"summon one enemy from the catalogue for a live attack");
   const float hp=trial.hp;
   for(int frame=0;frame<480&&trial.combatImpacts.empty();frame++)trial.update(1.f/60,{},false);
   check(!trial.combatImpacts.empty()&&trial.combatImpacts.back().material==ImpactMaterial::Armor&&
         trial.hp==hp&&!trial.dead(),
         (std::string(enemyInfo(type).name)+" lands a real hostile attack while the DEV hero stays protected").c_str());
  }
 }
 for(int fps:{30,60,120})for(bool walk:{false,true}){
  Game duel;
  check(developerStart(duel,0,true),"start a DEV encounter with an existing wolf");
  duel.props.clear();duel.enemies.clear();duel.player={20,20};duel.facing={1,0};
  const V original=duel.player+V{2.f,.075f};
  duel.enemies.push_back({23,1,original,original,54,54});
  const float dt=1.f/fps;
  for(int frame=0;frame<2*fps;frame++)duel.update(dt,{},false);
  check(summonDeveloperEnemy(duel,1),"place a summoned wolf behind the attacking one");
  const float hp=duel.hp;
  std::array<bool,2> struck{};
  float largestTurn=0,largestStep=0,smallestGap=100;
  bool clear=true;
  for(int frame=0;frame<6*fps;frame++){
   const auto old=duel.enemies;
   duel.update(dt,walk&&frame<fps?V{.55f,.25f}:V{},false);
   clear&=separatedEnemies(duel);
   for(int i=0;i<2;i++){
    const auto& a=old[i];const auto& b=duel.enemies[i];
    largestStep=std::max(largestStep,distance(a.pos,b.pos));
    smallestGap=std::min(smallestGap,distance(b.pos,duel.player)-b.radius());
    struck[i]|=a.windup>0&&b.strike>0;
    if(a.plantedAttack()){
     const float turn=std::abs(std::remainder(b.animation.facingAngle-a.animation.facingAngle,2*AnimationPi));
     largestTurn=std::max(largestTurn,turn);
    }
   }
  }
  check(clear&&struck[0]&&struck[1]&&duel.hp==hp,"both wolves attack, keep separate bodies and preserve DEV protection");
  check(largestStep<3.f*dt&&largestTurn<(walk?18.f*dt:.12f)&&
        (walk||smallestGap>0),"a new summon does not spin, teleport or push the current attacker through the hero");
 }
 for(bool dev:{false,true}){
  Game hit;hit.developerSession=dev;hit.player={15,15};hit.enemies.clear();hit.props.clear();
  hit.enemies.push_back({20,0,{16,15},{16,15},100,100});hit.enemies.back().windup=.01f;hit.hp=1;
  hit.update(.02f,{},false);
  check(hit.hp==(dev?1.f:0.f)&&hit.dead()!=dev,"a real lethal monster strike only kills in ordinary play");
  hit.hp=70;hit.sounds.clear();hit.effects.clear();hit.hurt=0;
  hit.enemies[0].windup=.01f;
  hit.update(.02f,{},false);
  check(dev?hit.hp==70&&hit.effects.size()==1&&hit.effects[0].text=="TRAFIENIE"&&hit.hitReactionProgress()==0&&hit.hurt==0:
            hit.hp<70&&!hit.effects.empty()&&hit.hurt>0,
        "DEV shows contact and the regular hit reaction without reducing HP");
  hit.hp=70;hit.effects.clear();hit.sounds.clear();
  hit.enemyAbilityDamage(200,DamageType::Magic,hit.player,false,0,0);
  check(dev?hit.hp==70&&!hit.dead()&&hit.effects.size()==1&&hit.effects[0].text=="TRAFIENIE"&&
            hit.sounds.size()==1&&hit.sounds[0].cue==Cue::Hurt:hit.dead(),
        "lethal magic gives contact feedback in DEV and retains normal damage in player mode");
 }
 {
  Game rock;rock.developerSession=true;rock.enemies.clear();rock.props.clear();rock.player={20,20};
  rock.boulders.push_back({{17,20},{20,20},{19.7f,20},.45f,.5f,25,70,BoulderDamage,13});
  rock.update(.05f,{},false);
  check(rock.hp==rock.maxHp()&&rock.knockedBack()&&rock.heroKnockdownRemaining>0&&
        rock.effects.size()==1&&rock.effects[0].text=="TRAFIENIE",
        "rock impact keeps its knockdown for testing without reducing developer HP");
 }
 Game g;check(!g.developerSession&&g.bag.size()==BagSize&&g.level==1&&g.skillRanks==SkillRanks{}&&g.learnedSpells==0&&g.ammoCount()==20,"normal character starts with ordinary progression and one page");
 check(developerStart(g,10,true)&&g.inTower()&&g.towerFloor==2&&unlocked(g),"developer tower start grants the standard test character");
 check(catalogueComplete(g)&&g.bag.size()>BagSize&&g.bag.size()%BagSize==0,"fresh DEV session contains every ordinary item across full pages");
 Game earlierCatalogue=g;
 for(auto& value:earlierCatalogue.bag)if(protectiveGear(value.kind))value={};
 check(!catalogueComplete(earlierCatalogue)&&developerStart(earlierCatalogue,11,false)&&catalogueComplete(earlierCatalogue),
       "moving an existing DEV character replenishes all six newly introduced items");
 Game arena;
 for(Skill skill:{Skill::Cleave,Skill::DoubleThrust,Skill::Skewer}){
  check(developerStart(arena,0,true),"fresh combat test session starts");
  const Kind weapon=skillProfile(skill).weapon;
  const auto binding=std::find_if(arena.weaponWheel.begin(),arena.weaponWheel.end(),[&](auto value){return archetype(value.kind)==weapon;});
  check(binding!=arena.weaponWheel.end()&&arena.selectWeapon(int(binding-arena.weaponWheel.begin())),
        "each new weapon skill has its weapon on the developer wheel");
  check(arena.skillForSlot(0)==skill&&arena.useMeleeSkill(skill,{1,0}),"developer can use the corresponding skill before gaining any experience");
 }
 auto later=std::find_if(g.bag.begin()+BagSize,g.bag.end(),[](Item value){return gearSlot(value.kind)==0;});
 const int laterIndex=int(later-g.bag.begin());
 check(later!=g.bag.end()&&g.bindWeapon({Place::Bag,laterIndex},7)&&g.weaponSource(7).index==laterIndex&&g.selectWeapon(7),
       "a weapon on a later page can be assigned and equipped through the wheel");
 g.skillRanks={3,2};g.level=7;g.xp=56;g.attributes={8,7,6,5,4,3};auto attrs=g.attributes;g.towerRuneMask=7;developerBagSlot(g)=item(Kind::DawnShard);developerBagSlot(g)=item(Kind::Sword,2);g.addGround(item(Kind::Gold),g.player);int dropped=g.ground.back().id;auto supplies=g.bag;
 g.bag[0].count-=5;g.bag[1].count-=4;g.bag[5].count-=7;
 check(developerStart(g,12,false)&&same(g.bag,supplies)&&unlocked(g)&&g.xp==0&&g.attributes==attrs&&g.skillRanks==SkillRanks{3,2,1,1,1,1}&&g.towerRuneMask==7,"moving upgrades the character and preserves gear, attributes and higher ranks");
 check(!g.get({Place::Ground,dropped})&&developerStart(g,10,false)&&g.get({Place::Ground,dropped}),"developer transitions retain floor-local ground items for return");
 g.towerReturnPending=true;g.dawnQuest=DawnQuest::Returning;g.openedCorpse=13;g.rollRemaining=.3f;g.swing=.2f;g.bowDrawing=true;g.piercingArrowArmed=true;g.fireballs.push_back({g.player,{1,0}});g.arrows.push_back({g.player,{1,0},4,10});g.effects.push_back({g.player,"test",1,2});
 check(developerStart(g,0,false)&&!g.inTower()&&!g.storyBlocked()&&!g.towerReturnPending&&!g.rolling()&&!g.bowDrawing&&!g.piercingArrowArmed&&g.openedCorpse==-1&&g.fireballs.empty()&&g.arrows.empty()&&g.effects.empty(),"jump during active story or combat clears old transient actions without leaving blocked control");
 g.attributePoints=5;g.skillPoints=1;g.allocationRequired=true;check(developerStart(g,11,false)&&g.attributePoints==5&&g.skillPoints==1&&!g.allocationRequired,"developer transfer keeps earned points and opens play immediately");
 check(developerStart(g,1,true)&&unlocked(g)&&g.xp==0&&g.dawnQuest==DawnQuest::Seeking&&!g.bossDead&&g.enemies.back().alive,"new forest session resets boss and quest with the developer loadout");
 if(auto* guardian=g.findEnemy(13))g.hit(*guardian,10000);
 check(g.bossDead&&g.level==100&&g.xp==0&&!g.allocationRequired,"developer combat preserves level 100 without triggering mandatory progression");
 auto before=g.bag;V pos=g.player;check(!developerStart(g,-1,true)&&!developerStart(g,DeveloperDestinationCount,true)&&same(before,g.bag)&&distance(pos,g.player)==0,"invalid developer destination is rejected without modifying game state");
 g.props.push_back({DeveloperDestinations[0].spawn,0,1});g.enemies.clear();check(developerStart(g,0,false)&&distance(g.player,DeveloperDestinations[0].spawn)>.1f&&g.walkable(g.player),"blocked preset position falls back to the nearest safe point");
 g=Game{};for(auto& value:g.bag)value=item(Kind::Sword,2);g.bag.back()=item(Kind::DawnShard);
 check(developerStart(g,10,false)&&g.ammoCount()==100&&g.bagCount(Kind::Health)==50&&g.bagCount(Kind::Mana)==50&&g.bagCount(Kind::DawnShard)==1,"full pack receives all supplies and keeps its quest item");
 check(g.ground.empty()&&g.bagCount(Kind::Sword)==BagSize-1&&catalogueComplete(g),"full backpack adds pages while preserving the existing gear");
 g=Game{};std::fill(g.bag.begin(),g.bag.end(),item(Kind::Health,0,1));
 check(developerStart(g,10,false)&&g.ammoCount()==100&&g.bagCount(Kind::Health)==50&&g.bagCount(Kind::Mana)==50&&g.ground.empty(),"fragmented potion stacks consolidate before reserving space for supplies");
 auto parse=[](std::vector<std::string> arguments){std::vector<char*> argv;for(auto& s:arguments)argv.push_back(s.data());DeveloperLaunch launch;for(int i=1;i<int(argv.size());i++)parseDeveloperArgument(argv[i],i,int(argv.size()),argv.data(),launch);return launch;};
 auto launch=parse({"game","--dev"});check(launch.enabled&&launch.destination==-1&&launch.error.empty(),"dev flag opens picker rather than requiring a destination");
 launch=parse({"game","--start","tower-5","--no-audio"});check(launch.enabled&&launch.destination==12&&launch.error.empty(),"direct command-line tower start parses separate argument");
 launch=parse({"game","--start=ruins","--no-dev-kit"});check(launch.destination==1&&launch.error.empty(),"old command lines still select their location");
 launch=parse({"game","--start","--no-audio"});check(launch.enabled&&launch.destination==-1&&!launch.error.empty(),"missing location does not consume a following option or silently pick a wrong room");
 launch=parse({"game","--start","tower-6"});check(launch.enabled&&launch.destination==-1&&!launch.error.empty(),"invalid floor requests return to picker with an explicit error");
 check(developerDestination("TOWER-3")==10&&developerDestination("tower")==8&&developerDestination("boss")==1&&developerDestination("")==-1,"location aliases, case normalization and unknown input are predictable");
 check(developerDestination("volcano")==16&&developerDestination("volcano-forges")==17&&developerDestination("volcano-boss")==18&&developerDestination("KALDERA")==16&&developerDestination("varkhor")==18,"third realm and boss have stable developer destinations and aliases");
 launch=parse({"game","--start=volcano-boss","--no-audio"});check(launch.enabled&&launch.destination==18&&launch.error.empty(),"CLI supports direct start at Varkhor");
 check(developerStart(g,16,true)&&g.inVolcano()&&g.dawnFragments()==2&&g.desertBossDead&&!g.volcanoBossDead&&g.findEnemy(VolcanoBossId)&&g.bagCount(Kind::DawnShard)==0,"fresh volcanic session grants gear but leaves Varkhor and the quest item in the world");
 auto equipment=g.equipment;auto inventory=g.bag;g.level=9;g.xp=35;g.attributes={3,2,4,1,5,6};g.skillRanks={2,1,0};g.hp=float(g.maxHp());g.mana=float(g.maxMana());
 auto* boss=g.findEnemy(VolcanoBossId);boss->hp-=23;float bossHp=boss->hp;boss->eruptionWindup=.8f;
 g.cinders.push_back({g.player,{7.5f,0},7,20,2002});g.fissures.push_back({g.player,0,.95f,48,VolcanoBossId,false});g.heroKnockdownRemaining=.5f;g.heroKnockbackRemaining=.2f;g.sandPrisonRemaining=1;g.openedChest=4;
 check(developerStart(g,18,false)&&g.inVolcano()&&developerSpawnSafe(g,g.player)&&distance(g.player,VolcanoBossHome)<6,"current character can move directly to the volcanic boss safely");
 check(same(g.bag,inventory)&&same(g.equipment,equipment)&&unlocked(g)&&g.xp==0&&g.attributes==Attributes{3,2,4,1,5,6}&&g.skillRanks==SkillRanks{2,1,1,1,1,1}&&g.findEnemy(VolcanoBossId)->hp==bossHp,"same-realm warp keeps gear and enemy progress with all developer unlocks");
 check(g.cinders.empty()&&g.fissures.empty()&&g.findEnemy(VolcanoBossId)->eruptionWindup==0&&!g.knockedBack()&&!g.sandTrapped()&&g.openedChest==-1,"same-realm warp cancels eruptions, projectiles, old knockdown and open loot");
 g.player=VolcanoPortal;check(g.useVolcanoPortal()&&g.inTower(),"volcanic developer session can return through the ordinary portal");g.player=MagePosition;check(g.canTravelVolcano(),"developer story setup permits returning to the third realm through Eldrin");
}
