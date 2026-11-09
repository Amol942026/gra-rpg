#pragma once
#include "../src/developer_mode.hpp"
void developerTests(){
 auto same=[](const auto& a,const auto& b){for(size_t i=0;i<a.size();i++)if(a[i].kind!=b[i].kind||a[i].tier!=b[i].tier||a[i].count!=b[i].count)return false;return true;};
 auto unlocked=[](const Game& g){
  for(int rank:g.skillRanks)if(rank<1)return false;
  for(int i=1;i<int(Spell::Count);i++)if(!g.knowsSpell(Spell(i)))return false;
  return g.developerSession&&g.level==100&&!g.allocationRequired&&g.knowsSpell(g.selectedSpell);
 };
 for(unsigned seed:{1u,12593u,67841u})for(int destination=0;destination<DeveloperDestinationCount;destination++){
  Game g(seed);g.hp=0;g.mana=0;check(developerStart(g,destination,true,seed),"every developer destination accepts a fresh session across seeds");
  const auto& target=DeveloperDestinations[destination];check(g.location==target.location&&g.towerFloor==target.floor&&developerSpawnSafe(g,g.player),"developer spawn is in selected room and respects scenery and enemy bodies");
  check(distance(g.player,target.spawn)<=4.001f&&g.hp==g.maxHp()&&g.mana==g.maxMana()&&!g.allocationRequired&&!g.storyBlocked()&&!g.towerReturnPending,"direct start restores resources and never leaves a story or progression input lock");
  check(unlocked(g)&&g.xp==0&&g.ammoCount()==100&&g.bagCount(Kind::Health)==50&&g.bagCount(Kind::Mana)==50,"each fresh developer start has all unlocks and exact supplies");
 }
 Game g;check(!g.developerSession&&g.level==1&&g.skillRanks==SkillRanks{}&&g.learnedSpells==0&&g.ammoCount()==20,"normal character starts with ordinary progression");
 check(developerStart(g,10,true)&&g.inTower()&&g.towerFloor==2&&unlocked(g),"developer tower start grants the standard test character");
 g.skillRanks={3,2};g.level=7;g.xp=56;g.attributes={8,7,6,5,4,3};auto attrs=g.attributes;g.towerRuneMask=7;g.bag[12]=item(Kind::DawnShard);g.bag[13]=item(Kind::Sword,2);g.addGround(item(Kind::Gold),g.player);int dropped=g.ground.back().id;auto supplies=g.bag;
 g.bag[0].count-=5;g.bag[1].count-=4;g.bag[5].count-=7;
 check(developerStart(g,12,false)&&same(g.bag,supplies)&&unlocked(g)&&g.xp==0&&g.attributes==attrs&&g.skillRanks==SkillRanks{3,2,1}&&g.towerRuneMask==7,"moving upgrades the character and preserves gear, attributes and higher ranks");
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
 bool stash=g.ground.size()==4&&g.bagCount(Kind::Sword)==20;
 for(const auto& drop:g.ground)stash&=drop.value.kind==Kind::Sword&&drop.value.tier==2&&g.groundVisible(drop)&&distance(drop.pos,g.player)==0;
 check(stash,"gear displaced by supplies remains recoverable at the destination");
 g=Game{};g.bag.fill(item(Kind::Health,0,1));
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
 check(developerStart(g,16,true)&&g.inVolcano()&&g.dawnFragments()==2&&g.desertBossDead&&!g.volcanoBossDead&&g.findEnemy(VolcanoBossId)&&g.bagCount(Kind::DawnShard)==0,"fresh volcanic session bypasses earlier story gates without granting gear or defeating Varkhor");
 auto equipment=g.equipment;auto inventory=g.bag;g.level=9;g.xp=35;g.attributes={3,2,4,1,5,6};g.skillRanks={2,1,0};g.hp=float(g.maxHp());g.mana=float(g.maxMana());
 auto* boss=g.findEnemy(VolcanoBossId);boss->hp-=23;float bossHp=boss->hp;boss->eruptionWindup=.8f;
 g.cinders.push_back({g.player,{7.5f,0},7,20,2002});g.fissures.push_back({g.player,0,.95f,48,VolcanoBossId,false});g.heroKnockdownRemaining=.5f;g.heroKnockbackRemaining=.2f;g.sandPrisonRemaining=1;g.openedChest=4;
 check(developerStart(g,18,false)&&g.inVolcano()&&developerSpawnSafe(g,g.player)&&distance(g.player,VolcanoBossHome)<6,"current character can move directly to the volcanic boss safely");
 check(same(g.bag,inventory)&&same(g.equipment,equipment)&&unlocked(g)&&g.xp==0&&g.attributes==Attributes{3,2,4,1,5,6}&&g.skillRanks==SkillRanks{2,1,1}&&g.findEnemy(VolcanoBossId)->hp==bossHp,"same-realm warp keeps gear and enemy progress with all developer unlocks");
 check(g.cinders.empty()&&g.fissures.empty()&&g.findEnemy(VolcanoBossId)->eruptionWindup==0&&!g.knockedBack()&&!g.sandTrapped()&&g.openedChest==-1,"same-realm warp cancels eruptions, projectiles, old knockdown and open loot");
 g.player=VolcanoPortal;check(g.useVolcanoPortal()&&g.inTower(),"volcanic developer session can return through the ordinary portal");g.player=MagePosition;check(g.canTravelVolcano(),"developer story setup permits returning to the third realm through Eldrin");
}
