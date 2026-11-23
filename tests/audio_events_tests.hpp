#pragma once
#include "../src/music_blend.hpp"

int cueCount(const Game& g,Cue cue){return int(std::count_if(g.sounds.begin(),g.sounds.end(),[cue](auto e){return e.cue==cue;}));}
void audioEventTests() {
    Game g=emptyArena();check(g.sounds.empty(),"new expedition does not play phantom loot sounds");
    g.attack();check(cueCount(g,Cue::Swing)==1,"swing emits even when attack misses");
    g.attack();check(cueCount(g,Cue::Swing)==1,"cooldown does not repeat swing audio");
    g.sounds.clear();g.startRoll({1,0});
    for(int i=0;i<8;i++)g.update(.05f,{},false);
    check(cueCount(g,Cue::Roll)==1&&cueCount(g,Cue::Step)==0,"roll emits once without walking footsteps");
    g.sounds.clear();for(int i=0;i<20;i++)g.update(.05f,{1,0},false);
    check(cueCount(g,Cue::Step)>=2&&cueCount(g,Cue::Step)<=3,"footsteps follow the two visible foot contacts per gait cycle");
    g.sounds.clear();for(int i=0;i<20;i++)g.update(.05f,{},false);
    check(g.sounds.empty(),"idle does not create footsteps");
    g.player={MapWalkMax,15};g.sounds.clear();for(int i=0;i<20;i++)g.update(.05f,{1,0},false);
    check(g.sounds.empty(),"blocked movement is silent");
    for(int fps:{30,60,120})for(bool run:{false,true}){
        Game steps=emptyArena();steps.player={30,30};
        for(int frame=0;frame<fps*2;frame++){
            float before=steps.animation.phase;steps.sounds.clear();steps.update(1.f/fps,{1,0},false,run);
            bool contact=steps.animation.phase<before||int(steps.animation.phase*2)>int(before*2);
            check(cueCount(steps,Cue::Step)==int(contact),"walk and run emit one footstep exactly at each animated contact at 30/60/120 Hz");
        }
    }
    Game p=emptyArena();p.hp=25;p.use({Place::Bag,0});
    check(cueCount(p,Cue::Potion)==1,"successful potion emits bubbling sound");
    p.use({Place::Bag,0});check(cueCount(p,Cue::Potion)==1,"failed potion use is silent");
    p.sounds.clear();p.transfer({Place::Bag,2},{Place::Equip,2});
    check(cueCount(p,Cue::Equip)==1,"equipping emits gear sound");
    p.sounds.clear();p.addGround(item(Kind::Gold),p.player);p.transfer({Place::Ground,p.ground.back().id},{Place::Bag,10});
    check(cueCount(p,Cue::Pickup)==1,"ground to backpack emits pickup");
    p.sounds.clear();p.drop({Place::Bag,10},p.player);
    check(cueCount(p,Cue::Drop)==1,"dropping emits impact");
    p.sounds.clear();p.transfer({Place::Bag,0},{Place::Equip,0});
    check(p.sounds.empty(),"rejected inventory transfer emits no success sound");
    for(int type=0;type<4;type++){
        Game e=emptyArena();Enemy enemy{0,type,{15,15},{15,15},1,1};
        e.hit(enemy,10);
        check(cueCount(e,enemyHitCue(type))==1,"a lethal hit still plays the enemy's impact sound");
        check(cueCount(e,Cue::Victory)==int(type==3)&&cueCount(e,enemyFallCue(type))==0,
              "the boss reward still cues on defeat while impacts wait for the terrain");
        auto count=e.sounds.size();e.hit(enemy,10);check(e.sounds.size()==count,"dead enemies cannot emit repeated deaths");
    }
    Game level=emptyArena();level.xp=experienceForLevel(1)-1;Enemy enemy{0,0,{15,15},{15,15},1,1};level.hit(enemy,5);
    check(cueCount(level,Cue::LevelUp)==1,"level advancement emits fanfare");
    Game spell=emptyArena();spell.spell();check(cueCount(spell,Cue::FireballCast)==1&&cueCount(spell,Cue::Spell)==0,"fireball launch uses its own flame gesture");
    spell.sounds.clear();spell.spell();check(spell.sounds.empty(),"magic cooldown does not repeat sound");
    spell.explode({18,15},20);check(cueCount(spell,Cue::FireballImpact)==1,"fireball impact emits an explosion even when it hits terrain or expires");
    Game bow=emptyArena();bow.equipment[0]=item(Kind::Bow);
    check(bow.beginBow({1,0})&&!bow.beginBow({1,0})&&cueCount(bow,Cue::BowDraw)==0&&cueCount(bow,Cue::BowShot)==0,"taking the arrow from the quiver makes no string or release sound");
    const int nockFrames=int(std::lround(BowNockDuration*60));
    for(int i=1;i<nockFrames;i++)bow.update(1.f/60,{},false);
    check(cueCount(bow,Cue::BowDraw)==0,"the string stays silent until the nock arrives");
    bow.update(1.f/60,{},false);
    check(cueCount(bow,Cue::BowDraw)==1&&cueCount(bow,Cue::BowShot)==0,"tension starts once on the transition from nocking to pulling");
    for(int i=0;i<40;i++)bow.update(.05f,{},false);
    check(cueCount(bow,Cue::BowDraw)==1,"fully drawn bow does not restart a looping tension sound");
    bow.sounds.clear();bow.releaseBow();check(cueCount(bow,Cue::BowShot)==1&&cueCount(bow,Cue::BowDraw)==0,"release emits only the separate arrow shot");
    bow.attackCd=0;bow.beginBow({1,0});bow.sounds.clear();bow.cancelBow();check(bow.sounds.empty()&&!bow.bowDrawing,"cancelling a bow never invents a release sound");
    for(auto& value:bow.bag)if(value.kind==Kind::Arrow)value={};bow.sounds.clear();
    check(!bow.beginBow({1,0})&&cueCount(bow,Cue::BowDraw)==0,"empty quiver cannot play string tension");
    Game portal=emptyArena();portal.enterMageTower();portal.dawnQuest=DawnQuest::AtTower;portal.player=MagePosition;portal.collectedFragments=1;
    check(portal.travelToDesert()&&cueCount(portal,Cue::TeleportArrival)==1,"desert arrival emits a portal burst at the destination");
    portal.sounds.clear();check(!portal.travelToDesert()&&portal.sounds.empty(),"rejected or repeated travel cannot replay the portal burst");
    portal.player=DesertPortal;check(portal.useDesertPortal()&&cueCount(portal,Cue::TeleportArrival)==1,"return to the tower has its own arrival burst");
    portal.sounds.clear();portal.player=MagePosition;portal.desertBossDead=true;
    check(portal.travelToVolcano()&&cueCount(portal,Cue::TeleportArrival)==1,"volcanic arrival uses the dedicated portal sound");
    portal.sounds.clear();portal.player=VolcanoPortal;
    check(portal.useVolcanoPortal()&&cueCount(portal,Cue::TeleportArrival)==1,"return from the volcano plays arrival exactly once");
    Game hurt=emptyArena();hurt.enemies.push_back({0,0,{15.2f,15},{15.2f,15},100,100});
    hurt.enemies[0].windup=.01f;hurt.update(.02f,{},false);
    check(cueCount(hurt,Cue::Hurt)==1&&hurt.hitReactionProgress()==0&&hurt.heroHitDirection.x<-.99f,
          "enemy impact emits player hurt and starts a lean away from the attacker");
    hurt.sounds.clear();hurt.hp=1;hurt.enemies[0].windup=.01f;hurt.update(.02f,{},false);
    check(cueCount(hurt,Cue::Death)==1&&cueCount(hurt,Cue::Hurt)==0&&hurt.hitReactionProgress()<0,
          "lethal impact clears the hit reaction and emits death instead of duplicate hurt");
    hurt.update(.02f,{},false);check(cueCount(hurt,Cue::Death)==1,"death audio does not loop");
    Game dodge=emptyArena();dodge.enemies.push_back({0,0,{15.2f,15},{15.2f,15},100,100});
    dodge.startRoll({1,0});dodge.enemies[0].windup=.01f;dodge.update(.02f,{},false);
    check(cueCount(dodge,Cue::Dodge)==1&&cueCount(dodge,Cue::Hurt)==0&&dodge.hitReactionProgress()<0,
          "protected roll emits dodge without hurt or recoil");

    const V hitPosition{15,15};
    auto meleeHit=[hitPosition](V away){
        Game arena=emptyArena();
        arena.player=hitPosition;
        const V foe=arena.player-away;
        arena.enemies.push_back({0,0,foe,foe,100,100});
        arena.enemies.front().windup=.01f;
        arena.enemies.front().attack=2;
        arena.update(.02f,{},false);
        return arena;
    };
    const V away=normalized(V{1,1});
    for(float rate:{30.f,60.f,120.f}){
        Game reacting=meleeHit(away);
        const V facingBefore=reacting.facing;
        check(reacting.hitReactionProgress()==0&&distance(reacting.heroHitDirection,away)<.0001f&&!reacting.knockedBack(),
              "ordinary melee starts a directed reaction without the boulder control lock");
        Game walking=reacting,walkControl=reacting;
        walkControl.clearHeroHitReaction();
        walking.update(1/rate,{1,0},false);walkControl.update(1/rate,{1,0},false);
        check(walking.hitReactionProgress()>0&&walking.player.x>hitPosition.x&&distance(walking.player,walkControl.player)==0,
              "the lean leaves the first walking step unchanged");
        bool planted=distance(reacting.player,hitPosition)==0;
        for(int frame=0;frame<int(rate*HeroHitReactionDuration)+1;frame++){
            reacting.update(1/rate,{},false);
            planted&=distance(reacting.player,hitPosition)==0;
        }
        check(planted&&distance(reacting.facing,facingBefore)==0&&
              reacting.hitReactionProgress()<0&&cueCount(reacting,Cue::Step)==0,
              "idle hit keeps the hero planted throughout the lean at 30, 60 and 120 Hz");
    }
    Game travel=meleeHit({1,0});travel.activateField(WorldLocation::Desert);
    check(travel.hitReactionProgress()<0&&length(travel.heroHitDirection)==0,
          "travelling to another realm clears the temporary hit reaction");
    Game bounded=emptyArena();for(int i=0;i<1000;i++)bounded.sound(Cue::Step,bounded.player);
    check(bounded.sounds.size()==64,"unconsumed sound queue is bounded");
    Game chest;chest.player=chest.chests[0].pos+V{1,0};
    check(chest.openChest(chest.chests[0].id)&&cueCount(chest,Cue::ChestOpen)==1&&cueCount(chest,Cue::Ui)==0,"opening a real treasure chest emits latch and hinge instead of a button click");
    chest.sounds.clear();chest.player={1,1};check(!chest.openChest(chest.chests[0].id)&&chest.sounds.empty(),"out-of-range chest interaction cannot emit a success sound");
    Game weapon=emptyArena();weapon.bag[12]=item(Kind::Bow);weapon.transfer({Place::Bag,12},{Place::Equip,0});
    check(cueCount(weapon,Cue::WeaponEquip)==1&&cueCount(weapon,Cue::Equip)==0,"changing a weapon emits a distinct draw and fastening sound");
    std::array<bool,int(Cue::Count)> enemyCues{};
    for(int type=0;type<EnemyTypeCount;type++){
        for(Cue cue:{enemyAttackCue(type),enemyHitCue(type),enemyFallCue(type),enemyDyingCue(type)}){
            const bool borrowed=type==DuneJackalType||type==GlassReaperType;
            check(enemyCues[int(cue)]==borrowed,"desert variants share existing voice banks and original enemies keep their cues");
            if(!borrowed)enemyCues[int(cue)]=true;
        }
        check(enemyAttackSound(enemyAttackCue(type))&&enemyLandingCue(enemyFallCue(type))&&
              !enemyLandingCue(enemyHitCue(type)),"attack and landing retain their mixer categories");
        Game hit=emptyArena();Enemy target{0,type,{15,15},{15,15},10000,10000};hit.hit(target,10);
        check(cueCount(hit,enemyHitCue(type))==1,"all enemy species emit their assigned material impact");
        Game attack=emptyArena();attack.enemies.push_back({0,type,{15.2f,15},{15.2f,15},100,100});attack.update(.01f,{},false);
        check(cueCount(attack,enemyAttackCue(type))==0&&attack.enemies[0].windup>0,"ordinary attack sound waits for the actual strike instead of playing during preparation");
        for(int frame=0;frame<2+int(meleeWindup(type)/.02f);frame++)attack.update(.02f,{},false);
        check(cueCount(attack,enemyAttackCue(type))==1,"all enemy species emit one physical attack sound at their strike");
    }
    for(int fps:{30,60,120}){
        MusicBlend blend;
        for(int target:{1,2,3,0}){
            bool headroom=true;for(int frame=0;frame<fps*3;frame++){blend.update(1.f/fps,target);float sum=0;for(float weight:blend.weights)sum+=weight;headroom&=std::abs(sum-1)<.0001f;}
            check(headroom,"crossfade preserves music headroom through every realm transition");
            check(blend.selected==target&&blend.weights[target]>.999f,"each location smoothly becomes the dominant score at 30/60/120 Hz");
        }
        auto weights=blend.weights;blend.update(0,2);check(blend.weights==weights,"a zero-time update cannot jump music volumes");
    }
    MusicBlend rapid;bool boundedGains=true;for(int i=0;i<500;i++){rapid.update(.01f,i%4);for(float weight:rapid.weights)boundedGains&=weight>=0&&weight<=1;}check(boundedGains,"rapid travel keeps every music gain bounded");
}

