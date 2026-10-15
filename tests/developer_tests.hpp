#pragma once
#include "../src/developer_mode.hpp"
void developerTests(){
 auto same=[](const auto& a,const auto& b){for(size_t i=0;i<a.size();i++)if(a[i].kind!=b[i].kind||a[i].tier!=b[i].tier||a[i].count!=b[i].count)return false;return true;};
 for(unsigned seed:{1u,12593u,67841u})for(int destination=0;destination<DeveloperDestinationCount;destination++){
  Game g(seed);g.hp=0;g.mana=0;check(developerStart(g,destination,true,false,seed),"every developer destination accepts a fresh session across seeds");
  const auto& target=DeveloperDestinations[destination];check(g.location==target.location&&g.towerFloor==target.floor&&developerSpawnSafe(g,g.player),"developer spawn is in selected room and respects scenery and enemy bodies");
  check(distance(g.player,target.spawn)<=4.001f&&g.hp==g.maxHp()&&g.mana==g.maxMana()&&!g.allocationRequired&&!g.storyBlocked()&&!g.towerReturnPending,"direct start restores resources and never leaves a story or progression input lock");
  check(g.level==1&&g.xp==0&&g.skillRanks==SkillRanks{}&&g.ammoCount()==20&&g.bagCount(Kind::Mushroom)==0,"starting without test kit preserves normal starting character balance");
 }
 Game g;check(developerStart(g,10,true,true)&&g.inTower()&&g.towerFloor==2&&g.skillRanks==SkillRanks{}&&g.ammoCount()==100&&g.bagCount(Kind::Mushroom)==15&&g.bagCount(Kind::ForestEssence)==15&&g.bagCount(Kind::Health)==10&&g.bagCount(Kind::Mana)==10,"optional kit supplies items without granting unearned skills");
 auto kit=g.bag;g.skillRanks={3,2};g.level=7;g.xp=56;g.attributes={8,7,6,5,4,3};auto attrs=g.attributes;g.towerRuneMask=7;g.bag[12]=item(Kind::DawnShard);g.bag[13]=item(Kind::Sword,2);g.addGround(item(Kind::Gold),g.player);int dropped=g.ground.back().id;kit=g.bag;
 check(developerStart(g,12,false,true)&&same(g.bag,kit)&&g.level==7&&g.xp==56&&g.attributes==attrs&&g.skillRanks==SkillRanks{3,2,0}&&g.towerRuneMask==7,"moving current character preserves allocated ranks and leaves unlearned Healing locked");
 check(!g.get({Place::Ground,dropped})&&developerStart(g,10,false,false)&&g.get({Place::Ground,dropped}),"developer transitions retain floor-local ground items for return");
 g.towerReturnPending=true;g.dawnQuest=DawnQuest::Returning;g.openedCorpse=13;g.rollRemaining=.3f;g.swing=.2f;g.bowDrawing=true;g.piercingArrowArmed=true;g.fireballs.push_back({g.player,{1,0}});g.arrows.push_back({g.player,{1,0},4,10});g.effects.push_back({g.player,"test",1,2});
 check(developerStart(g,0,false,false)&&!g.inTower()&&!g.storyBlocked()&&!g.towerReturnPending&&!g.rolling()&&!g.bowDrawing&&!g.piercingArrowArmed&&g.openedCorpse==-1&&g.fireballs.empty()&&g.arrows.empty()&&g.effects.empty(),"jump during active story or combat clears old transient actions without leaving blocked control");
 g.attributePoints=5;g.skillPoints=1;g.allocationRequired=true;check(developerStart(g,11,false,false)&&g.attributePoints==5&&g.skillPoints==1&&g.allocationRequired,"moving current character does not discard earned unallocated points");
 check(developerStart(g,1,true,false)&&g.level==1&&g.xp==0&&!g.allocationRequired&&g.dawnQuest==DawnQuest::Seeking&&!g.bossDead&&g.enemies.back().alive,"new forest session resets boss and quest so the whole encounter can be tested again");
 auto before=g.bag;V pos=g.player;check(!developerStart(g,-1,true,true)&&!developerStart(g,DeveloperDestinationCount,true,true)&&same(before,g.bag)&&distance(pos,g.player)==0,"invalid developer destination is rejected without modifying game state");
 g.props.push_back({DeveloperDestinations[0].spawn,0,1});g.enemies.clear();check(developerStart(g,0,false,false)&&distance(g.player,DeveloperDestinations[0].spawn)>.1f&&g.walkable(g.player),"blocked preset position falls back to the nearest safe point");
 g=Game{};for(auto& value:g.bag)value=item(Kind::Sword,2);before=g.bag;check(developerStart(g,10,false,true)&&same(g.bag,before)&&g.message.find("pełny plecak")!=std::string::npos,"full pack never loses player items when optional kit cannot fit");
 auto parse=[](std::vector<std::string> arguments){std::vector<char*> argv;for(auto& s:arguments)argv.push_back(s.data());DeveloperLaunch launch;for(int i=1;i<int(argv.size());i++)parseDeveloperArgument(argv[i],i,int(argv.size()),argv.data(),launch);return launch;};
 auto launch=parse({"game","--dev"});check(launch.enabled&&launch.destination==-1&&launch.error.empty(),"dev flag opens picker rather than requiring a destination");
 launch=parse({"game","--start","tower-5","--no-audio"});check(launch.enabled&&launch.destination==12&&launch.error.empty(),"direct command-line tower start parses separate argument");
 launch=parse({"game","--start=ruins","--no-dev-kit"});check(launch.destination==1&&!launch.kit,"inline direct location and no-kit flag parse correctly");
 launch=parse({"game","--start","--no-audio"});check(launch.enabled&&launch.destination==-1&&!launch.error.empty(),"missing location does not consume a following option or silently pick a wrong room");
 launch=parse({"game","--start","tower-6"});check(launch.enabled&&launch.destination==-1&&!launch.error.empty(),"invalid floor requests return to picker with an explicit error");
 check(developerDestination("TOWER-3")==10&&developerDestination("tower")==8&&developerDestination("boss")==1&&developerDestination("")==-1,"location aliases, case normalization and unknown input are predictable");
}
