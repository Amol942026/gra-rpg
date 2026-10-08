#pragma once

void combatPressTests() {
    const V directions[]={{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1}};
    for(V direction:directions){
        Game reach=emptyArena();V position=reach.player+normalized(direction)*2.06f;
        reach.enemies.push_back({0,1,position,position,100,100});
        reach.attack();check(reach.enemies[0].hp<100,"extended sword reaches a target at 2.06 map units in every direction");
        Game far=emptyArena();position=far.player+normalized(direction)*2.2f;
        far.enemies.push_back({0,1,position,position,100,100});
        far.attack();check(far.enemies[0].hp==100,"extended sword cannot hit beyond its 2.1 unit reach");
    }
    Game g=emptyArena();g.enemies.push_back({0,1,{16,15},{16,15},500,500});
    g.enemies[0].attack=99;
    for(int i=0;i<60;i++)g.update(1.f/60,{},false);
    check(g.enemies[0].hp==500&&cueCount(g,Cue::Swing)==0,"nearby monster never starts an automatic player attack");
    g.update(1.f/60,{},true);float afterPress=g.enemies[0].hp;
    check(afterPress<500&&cueCount(g,Cue::Swing)==1,"one left mouse press causes one sword swing");
    for(int i=0;i<120;i++)g.update(1.f/60,{},false);
    check(g.enemies[0].hp==afterPress&&cueCount(g,Cue::Swing)==1,"no new press means no repeated attacks after cooldown");
    g.update(1.f/60,{},true);
    check(g.enemies[0].hp<afterPress&&cueCount(g,Cue::Swing)==2,"a second press produces exactly one second swing");
    for(int type=0;type<EnemyTypeCount;type++){
        Game arena=emptyArena();Enemy foe{0,type,{15,15},{15,15},100,100};
        float expected=damageAfterArmor(20,enemyInfo(type).defense);arena.hit(foe,20);
        check(std::abs(foe.hp-(100-expected))<.001f,"displayed enemy defense is used by received damage");
        arena.enemies.push_back({0,type,{15.2f,15},{15.2f,15},100,100});arena.enemies[0].windup=.01f;
        float received=arena.incomingDamage(float(enemyInfo(type).attack),enemyInfo(type).attackType);arena.update(.02f,{},false);
        check(std::abs(arena.hp-(100-received))<.001f,"displayed enemy attack is used by outgoing damage");
    }
}

