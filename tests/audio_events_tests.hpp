#pragma once

int cueCount(const Game& g,Cue cue){return int(std::count_if(g.sounds.begin(),g.sounds.end(),[cue](auto e){return e.cue==cue;}));}
void audioEventTests() {
    Game g=emptyArena();check(g.sounds.empty(),"new expedition does not play phantom loot sounds");
    g.attack();check(cueCount(g,Cue::Swing)==1,"swing emits even when attack misses");
    g.attack();check(cueCount(g,Cue::Swing)==1,"cooldown does not repeat swing audio");
    g.sounds.clear();g.startRoll({1,0});
    for(int i=0;i<8;i++)g.update(.05f,{},false);
    check(cueCount(g,Cue::Roll)==1&&cueCount(g,Cue::Step)==0,"roll emits once without walking footsteps");
    g.sounds.clear();for(int i=0;i<20;i++)g.update(.05f,{1,0},false);
    check(cueCount(g,Cue::Step)>=4&&cueCount(g,Cue::Step)<=5,"footsteps follow travelled distance");
    g.sounds.clear();for(int i=0;i<20;i++)g.update(.05f,{},false);
    check(g.sounds.empty(),"idle does not create footsteps");
    g.player={MapWalkMax,15};g.sounds.clear();for(int i=0;i<20;i++)g.update(.05f,{1,0},false);
    check(g.sounds.empty(),"blocked movement is silent");
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
    const Cue materials[]={Cue::SlimeHit,Cue::WolfHit,Cue::BoneHit,Cue::StoneHit};
    for(int type=0;type<4;type++){
        Game e=emptyArena();Enemy enemy{0,type,{15,15},{15,15},1,1};
        e.hit(enemy,10);
        check(cueCount(e,materials[type])==1,"each enemy material emits its own hit sound");
        check(cueCount(e,type==3?Cue::Victory:Cue::EnemyFall)==1,"death or victory cue emitted once");
        auto count=e.sounds.size();e.hit(enemy,10);check(e.sounds.size()==count,"dead enemies cannot emit repeated deaths");
    }
    Game level=emptyArena();level.xp=experienceForLevel(1)-1;Enemy enemy{0,0,{15,15},{15,15},1,1};level.hit(enemy,5);
    check(cueCount(level,Cue::LevelUp)==1,"level advancement emits fanfare");
    Game spell=emptyArena();spell.spell();check(cueCount(spell,Cue::Spell)==1,"magic emits its own effect");
    spell.sounds.clear();spell.spell();check(spell.sounds.empty(),"magic cooldown does not repeat sound");
    Game hurt=emptyArena();hurt.enemies.push_back({0,0,{15.2f,15},{15.2f,15},100,100});
    hurt.enemies[0].windup=.01f;hurt.update(.02f,{},false);
    check(cueCount(hurt,Cue::Hurt)==1,"enemy impact emits player hurt");
    hurt.sounds.clear();hurt.hp=1;hurt.enemies[0].windup=.01f;hurt.update(.02f,{},false);
    check(cueCount(hurt,Cue::Death)==1&&cueCount(hurt,Cue::Hurt)==0,"lethal impact emits death instead of duplicate hurt");
    hurt.update(.02f,{},false);check(cueCount(hurt,Cue::Death)==1,"death audio does not loop");
    Game dodge=emptyArena();dodge.enemies.push_back({0,0,{15.2f,15},{15.2f,15},100,100});
    dodge.startRoll({1,0});dodge.enemies[0].windup=.01f;dodge.update(.02f,{},false);
    check(cueCount(dodge,Cue::Dodge)==1&&cueCount(dodge,Cue::Hurt)==0,"protected roll emits dodge without hurt");
    Game bounded=emptyArena();for(int i=0;i<1000;i++)bounded.sound(Cue::Step,bounded.player);
    check(bounded.sounds.size()==64,"unconsumed sound queue is bounded");
}

