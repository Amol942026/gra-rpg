#include "../src/game.hpp"
#include <cstdlib>
#include <iostream>
using namespace ember;
int checks=0;
void check(bool b,const char* label){checks++;if(!b){std::cerr<<"FAIL: "<<label<<"\n";std::exit(1);}}
int total(const Game&g){int n=0;for(auto&i:g.bag)if(!i.empty())n+=i.count;for(auto&i:g.equipment)if(!i.empty())n+=i.count;for(auto&i:g.ground)if(!i.value.empty())n+=i.value.count;for(auto&e:g.enemies)for(auto&i:e.loot)if(!i.empty())n+=i.count;return n;}
#include "roll_tests.hpp"
#include "audio_events_tests.hpp"
#include "combat_press_tests.hpp"
#include "crowd_combat_tests.hpp"
#include "progression_tests.hpp"
#include "corpse_tests.hpp"
#include "patrol_tests.hpp"
#include "forest_loot_tests.hpp"
#include "mushroom_tests.hpp"
int main(){
 corpseTests();
 patrolTests();
 forestLootTests();
 mushroomTests();
 Game g;check(g.ground.empty(),"new game starts without any ground items");
 g.addGround(item(Kind::Sword,1),{7.2f,18.2f});g.addGround(item(Kind::Health,0,2),{8.2f,18.1f});g.addGround(item(Kind::Helmet),{9,18.7f});
 int n=total(g);int id=g.ground[0].id;
 check(g.transfer({Place::Ground,id},{Place::Bag,5}),"ground to backpack");check(total(g)==n,"item conservation");
 int oldDamage=g.damage();check(g.transfer({Place::Bag,5},{Place::Equip,0}),"equip weapon swap");check(g.damage()>oldDamage,"weapon changes damage");check(total(g)==n,"swap conservation");
 check(!g.transfer({Place::Bag,0},{Place::Equip,0}),"reject potion in weapon slot");check(total(g)==n,"rejection preserves inventory");
 g.addGround(item(Kind::Health),{25,25});check(!g.transfer({Place::Ground,g.ground.back().id},{Place::Bag,6}),"reject distant loot");
 check(g.transfer({Place::Ground,g.ground[1].id},{Place::Bag,0}),"stack health potions");check(g.bag[0].count==7,"stack count");
 g.hp=20;check(g.use({Place::Bag,0}),"use potion");check(g.hp==70&&g.bag[0].count==6,"heal and consume");check(!g.use({Place::Bag,0}),"potion cooldown");
 g.potionCd=0;g.hp=g.maxHp();check(!g.use({Place::Bag,0}),"no waste at full health");
 int oldTotal=total(g);check(g.drop({Place::Bag,2},g.player),"drop inventory item");check(total(g)==oldTotal,"drop conservation");
 check(!g.drop({Place::Bag,3},{0,0}),"reject invalid drop");
 g.player=g.enemies[0].pos;g.enemies[0].hp=1;size_t previousGround=g.ground.size();check(g.attack(),"attack triggers");check(!g.enemies[0].alive&&g.kills==1,"combat kill");check(g.ground.size()==previousGround&&!g.enemies[0].loot[0].empty()&&!g.enemies[0].loot[1].empty(),"kill keeps loot in the body without scattering ground items");
 check(!g.attack(),"attack cooldown");g.mana=100;check(g.spell(),"spell cast");check(g.mana==90,"spell mana cost");check(!g.spell(),"spell cooldown");
 g.hp=0;check(!g.use({Place::Bag,0}),"dead cannot consume");check(!g.attack(),"dead cannot attack");
 Game a,b;for(int i=0;i<60;i++)a.update(1.f/60,{1,0},false);for(int i=0;i<120;i++)b.update(1.f/120,{1,0},false);check(distance(a.player,b.player)<.002f,"movement independent of frame rate");
 Game c;c.bag[10]=item(Kind::Health,0,98);c.bag[11]=item(Kind::Health,0,8);check(c.transfer({Place::Bag,11},{Place::Bag,10}),"partial stack");check(c.bag[10].count==99&&c.bag[11].count==7,"stack limit preserves remainder");
 c.player=c.enemies.back().pos;c.enemies.back().hp=1;c.spell();c.update(.01f,{},false);check(c.bossDead,"boss objective");
 const V directions[8]={{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1}};
 for(int d=0;d<8;d++){
  Game walk;walk.props.clear();walk.enemies.clear();walk.player={15,15};
  walk.update(.05f,directions[d],false);
  check(walk.animation.moving&&int(walk.animation.direction)==d,"player faces actual movement in eight screen directions");
  walk.update(.05f,{},false);
  check(!walk.animation.moving&&int(walk.animation.direction)==d,"player retains facing and stops walk cycle when idle");
  for(int type=0;type<EnemyTypeCount;type++){
   Game chase;chase.props.clear();chase.enemies.clear();
   chase.enemies.push_back({0,type,{15,15},{15,15},100,100});
   chase.player=V{15,15}+normalized(directions[d])*3;
   chase.update(.05f,{},false);
   check(chase.enemies[0].animation.moving&&int(chase.enemies[0].animation.direction)==d,"every enemy type faces its actual chase direction");
   chase.player={1.6f,1.6f};chase.update(.05f,{},false);
   check(!chase.enemies[0].animation.moving&&int(chase.enemies[0].animation.direction)==d,"enemy retains direction while idle");
   chase.player=chase.enemies[0].pos+normalized(directions[d])*.8f;
   chase.update(.05f,{},false);
   check(chase.enemies[0].windup>0&&!chase.enemies[0].animation.moving&&int(chase.enemies[0].animation.direction)==d,"enemy turns toward attack without walking");
  }
 }
 Game blocked;blocked.props.clear();blocked.enemies.clear();blocked.player={1.6f,15};
 blocked.update(.05f,{-1,0},false);check(!blocked.animation.moving,"blocked character does not walk in place");
 AnimationState slow,fast;
 for(int i=0;i<60;i++)slow.advance(.03f,.01f,1.f/60);
 for(int i=0;i<120;i++)fast.advance(.015f,.005f,1.f/120);
 check(std::abs(slow.phase-fast.phase)<.0001f&&slow.direction==fast.direction,"animation phase is independent of FPS");
 for(int i=0;i<8;i++){AnimationState pose;pose.moving=true;pose.phase=i/8.f;check(pose.walkFrame()==i,"all eight walk keyframes are selectable");}
 rollTests();
 audioEventTests();
 combatPressTests();
 crowdCombatTests();
 progressionTests();
 std::cout<<"PASS: "<<checks<<" gameplay and animation checks\n";
}

