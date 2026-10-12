#pragma once
#include "../src/tower_story.hpp"

void towerTests(){
    for(unsigned seed=1;seed<=40;seed++){
        Game g(seed);Enemy& boss=g.enemies.back();g.hit(boss,1000);
        int fragments=0;for(const auto& value:boss.loot)if(value.kind==Kind::DawnShard)fragments+=value.count;
        check(fragments==1&&lootChance(3,boss.loot[4])==100,"every boss death generates exactly one guaranteed Dawn shard");
        check(g.dawnFragments()==0&&!g.towerReturnPending&&!g.inTower(),"boss death alone does not collect the fragment or leave the forest");
        int count=total(g);g.hit(boss,1000);check(total(g)==count,"repeated hits on the guardian cannot regenerate a fragment");
    }
    for(int type=0;type<EnemyTypeCount;type++)if(!enemyBoss(type)){std::mt19937 rng(8);bool clean=true;for(int n=0;n<100;n++)for(const auto& value:rollEnemyLoot(type,rng))clean&=value.kind!=Kind::DawnShard;check(clean,"ordinary monsters never drop the story artifact");}
    Game g;Enemy& boss=g.enemies.back();g.player=boss.pos;g.hit(boss,1000);
    Attributes points{};points[0]=g.attributePoints;g.distribute(points);SkillRanks learned{};learned[0]=g.skillPoints;g.trainSkills(learned);
    check(!g.transfer({Place::Corpse,4,boss.id},{Place::Bag,8})&&!g.towerReturnPending,"unopened guardian loot cannot trigger a return");
    g.openCorpse(boss.id);
    check(!g.transfer({Place::Corpse,4,boss.id},{Place::Equip,0})&&!g.towerReturnPending,"attempting to equip the fragment does not collect it");
    g.bag[8]=item(Kind::Sword);check(!g.transfer({Place::Corpse,4,boss.id},{Place::Bag,8})&&!boss.loot[4].empty()&&!g.towerReturnPending,"full destination preserves the artifact in the corpse without starting a cutscene");
    g.bag[8]={};V home=g.player;g.player={8,19};
    check(!g.transfer({Place::Corpse,4,boss.id},{Place::Bag,8})&&!g.towerReturnPending,"distant looting cannot trigger the quest");g.player=home;
    check(g.transfer({Place::Corpse,0,boss.id},{Place::Bag,9})&&!g.towerReturnPending,"collecting ordinary boss loot does not start the return scene");
    int before=total(g);check(g.transfer({Place::Corpse,4,boss.id},{Place::Bag,8})&&g.bag[8].kind==Kind::DawnShard&&total(g)==before&&g.dawnFragments()==1,"successful artifact transfer preserves the item and records one of five pieces");
    check(g.storyBlocked()&&g.takeTowerReturn()&&!g.takeTowerReturn(),"artifact pickup emits one return event and immediately freezes gameplay");
    float time=g.time;V position=g.player;g.update(.05f,{1,0},true);
    check(g.time==time&&distance(position,g.player)==0&&!g.startRoll({1,0})&&!g.attack()&&!g.spell(),"movement and combat stop during the entire story return");
    int level=g.level,xp=g.xp;Attributes stats=g.attributes;float hp=g.hp,mana=g.mana;
    auto equipment=g.equipment;auto bag=g.bag;V enemy=g.enemies[0].pos;
    g.addGround(item(Kind::Health),TowerArrival);int forestDrop=g.ground.back().id;
    g.fireballs.push_back({g.player,{1,0}});g.arrows.push_back({g.player,{1,0},5,10});g.enterMageTower();
    check(g.inTower()&&distance(g.player,TowerArrival)==0&&g.walkable(g.player)&&g.storyBlocked(),"return arrives inside the playable tower while cinematic input remains locked");
    check(g.level==level&&g.xp==xp&&g.attributes==stats&&g.hp==hp&&g.mana==mana,"location transition retains progression, health and mana");
    bool preserved=true;for(int i=0;i<BagSize;i++)preserved&=g.bag[i].kind==bag[i].kind&&g.bag[i].count==bag[i].count&&g.bag[i].tier==bag[i].tier;
    for(int i=0;i<EquipSize;i++)preserved&=g.equipment[i].kind==equipment[i].kind&&g.equipment[i].tier==equipment[i].tier;
    check(preserved&&g.arrows.empty()&&g.fireballs.empty()&&g.openedCorpse==-1,"teleport keeps inventory and equipment while clearing projectiles and the corpse menu");
    check(!g.get({Place::Ground,forestDrop})&&!g.reachable({Place::Ground,forestDrop})&&!g.get({Place::Corpse,0,boss.id})&&!g.get({Place::Forage,1}),"forest loot and mushrooms never leak into tower coordinates");
    g.finishTowerReturn();check(!g.storyBlocked()&&g.dawnQuest==DawnQuest::AtTower,"finishing the cinematic unlocks the tower instead of teleporting to the forest");
    check(!g.drop({Place::Bag,8},g.player)&&!g.use({Place::Bag,8})&&!g.transfer({Place::Bag,8},{Place::Equip,5}),"quest artifact cannot be discarded, consumed or equipped");
    g.addGround(item(Kind::Health),g.player);int towerDrop=g.ground.back().id;
    check(!g.transfer({Place::Bag,8},{Place::Ground,towerDrop})&&!g.transfer({Place::Ground,towerDrop},{Place::Bag,8})&&g.bag[8].kind==Kind::DawnShard,"both swap directions protect the artifact from ending on the ground");
    check(g.transfer({Place::Bag,8},{Place::Bag,10})&&!g.towerReturnPending&&g.dawnFragments()==1,"reordering the artifact does not replay the cinematic or add progress");
    check(g.transfer({Place::Ground,towerDrop},{Place::Bag,8}),"normal tower ground loot remains collectible");
    check(!g.mageReachable(),"Eldrin conversation requires actually approaching the NPC");
    for(int i=0;i<95;i++)g.update(1.f/60,{0,-1},false);
    check(g.mageReachable()&&distance(g.player,TowerArrival)>1,"tower supports walking from the arrival point into conversation range");
    check(distance(g.enemies[0].pos,enemy)==0&&g.hp==hp&&!g.attack()&&!g.spell(),"forest enemies remain frozen and combat cannot target them from the tower");
    check(!g.walkable(MagePosition)&&!g.walkable(TowerCenter+V{TowerRadius+.1f,0}),"mage body and tower boundary block movement");
    for(const auto& object:TowerFurnishings)check(!g.walkable(object.pos),"tower furnishings have real collisions");
    g.player=TowerCenter+V{TowerRadius-.03f,0};g.startRoll({1,0});for(int i=0;i<30;i++)g.update(1.f/60,{},false);
    check(distance(g.player,TowerCenter)<=TowerRadius+.0001f,"roll cannot escape the tower floor");
    for(float fps:{30.f,60.f,144.f}){
        TowerSequence scene;scene.beginReturn();std::array<bool,TowerReturnDialogue.size()> seen{};int arrivals=0,completions=0;
        for(int frame=0;frame<int(fps*100)&&scene.active();frame++){
            if(scene.phase==TowerPhase::Dialogue)seen[scene.line]=true;
            scene.update(1/fps);arrivals+=scene.takeArrival();completions+=scene.takeCompletion();
        }
        check(!scene.active()&&arrivals==1&&completions==1,"automatic return scene emits a single arrival and completion at varied FPS");
        check(std::all_of(seen.begin(),seen.end(),[](bool value){return value;}),"automatic return dialogue shows every hero and mage line");
    }
    TowerSequence scene;scene.beginTalk();scene.update(.1f);scene.update(.1f);scene.advance();
    check(scene.line==0&&scene.textComplete(),"NPC dialogue first click reveals rather than skips the current line");scene.advance();
    check(scene.line==1&&!scene.revealed,"NPC dialogue advances to the next speaker on the second click");
    scene.skip();check(!scene.active()&&!scene.takeArrival()&&!scene.takeCompletion(),"closing NPC dialogue cannot teleport the hero or replay quest completion");
    for(int repeat=0;repeat<3;repeat++){scene.beginTalk();for(int frame=0;frame<3600&&scene.active();frame++)scene.update(1.f/60);check(!scene.active()&&!scene.takeArrival(),"Eldrin supports repeat conversations that return to tower gameplay");}
    scene.beginReturn();int arrivals=0,completions=0;
    for(int i=0;i<300;i++){scene.skip();scene.advance();scene.update(1.f/60);arrivals+=scene.takeArrival();completions+=scene.takeCompletion();}
    check(!scene.active()&&arrivals==1&&completions==1,"repeated skip or advance cannot lose or duplicate a tower transition");
}
