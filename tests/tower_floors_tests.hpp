#pragma once
void towerFloorsTests(){
 auto tower=[](){Game g;g.enterMageTower();g.dawnQuest=DawnQuest::AtTower;return g;};
 auto same=[](const auto& a,const auto& b){for(size_t i=0;i<a.size();i++)if(a[i].kind!=b[i].kind||a[i].tier!=b[i].tier||a[i].count!=b[i].count)return false;return true;};
 Game g=tower();check(g.towerFloor==0&&g.walkable(g.player),"tower return always starts on floor one at a safe arrival");
 check(!g.climbTower(-1)&&!g.climbTower(0)&&!g.climbTower(2)&&!g.climbTower(1),"stairs reject nonexistent directions and remote activation");
 auto inventory=g.bag;auto gear=g.equipment;g.skillRanks={2,3};g.hp=73;g.mana=62;g.towerRuneMask=5;int xp=g.xp;
 for(int floor=0;floor<5;floor++){
  check(g.towerFloor==floor&&g.walkable(g.player),"each stair arrival remains on the expected walkable floor");
  check(g.stairPresent(1)==(floor<4)&&g.stairPresent(-1)==(floor>0),"only existing adjacent floors have stairs");
  check((floor==0&&!g.walkable(MagePosition))||(floor>0&&(!g.mageReachable())),"Eldrin exists only in the arrival room");
  for(const auto& object:towerFurnishings(floor))check(!g.walkable(object.pos),"every themed furnishing blocks movement on its own floor");
  // A flood through actual collision geometry reaches both stairs and every interaction.
  constexpr int side=57;std::array<bool,side*side> reached{};std::vector<int> queue;int origin=28*side+36;
  auto point=[](int n){return V{5+(n%57)*.25f,5+(n/57)*.25f};};
  // Pick the nearest walkable grid point to the real arrival.
  float nearest=999;for(int n=0;n<side*side;n++)if(g.walkable(point(n))&&distance(point(n),g.player)<nearest){nearest=distance(point(n),g.player);origin=n;}
  reached[origin]=true;queue.push_back(origin);
  for(size_t i=0;i<queue.size();i++){int n=queue[i],x=n%side,y=n/side;for(int d:{-1,1,-side,side}){int next=n+d;if(next<0||next>=side*side||(d==-1&&x==0)||(d==1&&x==side-1)||(d==-side&&y==0)||(d==side&&y==side-1)||reached[next]||!g.walkable(point(next)))continue;reached[next]=true;queue.push_back(next);}}
  auto accessible=[&](V target,float range){for(int n:queue)if(distance(point(n),target)<range)return true;return false;};
  if(floor<4)check(accessible(TowerStairUp,1.4f),"upstairs can be approached through real floor collisions");
  if(floor>0)check(accessible(TowerStairDown,1.4f),"downstairs can be approached through real floor collisions");
  for(const auto& object:towerFurnishings(floor))if(towerInteractive(object.type))check(accessible(object.pos,TowerUseRange),"every interactive themed object has an accessible approach");
  if(floor<4){g.player=TowerStairUp;check(g.climbTower(1),"climbing to each next floor uses the same stairs mechanic");}
 }
 check(!g.climbTower(1)&&g.towerFloor==4,"top floor cannot leave the five-floor range");
 for(int i=4;i>0;i--){g.player=TowerStairDown;check(g.climbTower(-1)&&g.towerFloor==i-1,"all floors support a round trip back downstairs");}
 check(same(g.bag,inventory)&&same(g.equipment,gear)&&g.skillRanks==SkillRanks{2,3}&&g.hp==73&&g.mana==62&&g.xp==xp&&g.towerRuneMask==5,"stairs preserve inventory, equipment, stats, skills, resources and activated runes");
 for(int block=0;block<4;block++){g=tower();g.player=TowerStairUp;if(block==0)g.hp=0;if(block==1)g.allocationRequired=true;if(block==2)g.dawnQuest=DawnQuest::Returning;if(block==3)g.rollRemaining=.2f;check(!g.climbTower(1)&&g.towerFloor==0,"death, allocation, story and rolling block stair teleportation");}
 g=tower();g.player=TowerStairUp;g.bag[10]=item(Kind::Health);check(g.drop({Place::Bag,10},g.player),"dropping item tags it with its actual tower floor");int first=g.ground.back().id;g.climbTower(1);g.player=TowerStairUp;
 check(!g.get({Place::Ground,first})&&!g.reachable({Place::Ground,first})&&!g.transfer({Place::Ground,first},{Place::Bag,10}),"same coordinates on another floor cannot expose or collect old loot");
 g.bag[10]=item(Kind::Mana);g.drop({Place::Bag,10},g.player);int second=g.ground.back().id;g.player=TowerStairDown;g.climbTower(-1);g.player=TowerStairUp;
 check(g.get({Place::Ground,first})&&!g.get({Place::Ground,second})&&g.transfer({Place::Ground,first},{Place::Bag,10})&&g.bag[10].kind==Kind::Health,"returning recovers only loot from that floor without deleting other floor loot");
 g=tower();g.hp=4;g.mana=2;int font=7;g.player=towerFurnishings(0)[font].pos+V{1,0};check(g.restAtTower(font)&&g.hp==g.maxHp()&&g.mana==g.maxMana(),"arrival fountain fully restores HP and mana");
 g=tower();g.towerFloor=2;g.player={12,12};int health=g.bag[0].count,mana=g.bag[1].count;
 g.bag[8]=item(Kind::Mushroom,0,1);g.bag[9]=item(Kind::Mushroom,0,2);g.bag[10]=item(Kind::ForestEssence,0,3);
 check(g.brewTowerPotion(0,Kind::Health)&&g.bagCount(Kind::Mushroom)==0&&g.bag[0].count==health+1,"alchemy consumes three mushrooms across stacks for exactly one health potion");
 check(g.brewTowerPotion(0,Kind::Mana)&&g.bagCount(Kind::ForestEssence)==0&&g.bag[1].count==mana+1,"alchemy consumes three forest essences for exactly one mana potion");
 check(!g.brewTowerPotion(0,Kind::Mana)&&!g.brewTowerPotion(0,Kind::Sword),"alchemy refuses missing materials and unsupported recipes");
 g=tower();g.towerFloor=2;g.player={12,12};for(auto& value:g.bag)value=item(Kind::Sword);g.bag[0]=item(Kind::Mushroom,0,4);auto full=g.bag;
 check(!g.brewTowerPotion(0,Kind::Health)&&same(g.bag,full),"full inventory rejects brewing atomically without eating ingredients");
 g.bag[0].count=3;check(g.brewTowerPotion(0,Kind::Health)&&g.bag[0].kind==Kind::Health&&g.bag[0].count==1,"consumed last ingredients create space even in a full backpack");
 g=tower();g.towerFloor=2;g.player={12,12};g.bag[0].count=99;g.bag[8]=item(Kind::Mushroom,0,3);check(g.brewTowerPotion(0,Kind::Health)&&g.bag[0].count==99&&g.bagCount(Kind::Health)==100,"brewing respects the potion stack limit");
 g=tower();g.towerFloor=3;for(int i=0;i<3;i++){g.player=towerFurnishings(3)[i].pos+V{1,0};check(g.toggleTowerRune(i),"each rune can be activated on the seals floor");}
 check(g.towerRuneMask==7,"all three independent runes stabilize the projection");g.player=towerFurnishings(3)[2].pos+V{1,0};g.toggleTowerRune(2);check(g.towerRuneMask==3,"runes can be toggled again without resetting other stones");
 g=tower();g.towerFloor=4;g.player=TowerCenter;V enemy=g.enemies[0].pos;float enemyHp=g.enemies[0].hp;for(int i=0;i<120;i++)g.update(1.f/60,{},false);check(distance(g.enemies[0].pos,enemy)==0&&g.enemies[0].hp==enemyHp&&!g.attack()&&!g.spell()&&!g.strongCut(),"upper rooms remain safe and never simulate forest enemies");
}
