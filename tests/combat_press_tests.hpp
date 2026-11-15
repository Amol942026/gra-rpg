#pragma once

void combatPressTests() {
    const V directions[]={{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1}};
    for(V direction:directions){
        Game reach=emptyArena();V position=reach.player+normalized(direction)*2.06f;
        reach.enemies.push_back({0,1,position,position,100,100});
        const V rear=reach.player-normalized(direction)*1.1f;
        reach.enemies.push_back({1,1,rear,rear,100,100});
        reach.aimMelee(direction*-1);
        check(reach.beginMeleeHold(direction*-1)&&reach.releaseMeleeHold(direction)&&reach.enemies[0].hp<100,
              "a sword tap reaches 2.06 map units forward in each direction, using the release aim");
        check(reach.enemies[1].hp==100&&reach.combatImpacts.size()==1,"a sword tap leaves the nearer rear enemy unharmed");
        Game far=emptyArena();position=far.player+normalized(direction)*2.2f;
        far.enemies.push_back({0,1,position,position,100,100});
        far.attack(direction);check(far.enemies[0].hp==100,"extended sword cannot hit beyond its 2.1 unit reach");
    }
    Game g=emptyArena();g.enemies.push_back({0,1,{16,15},{16,15},500,500});
    g.aimMelee({1,0});
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
        check(arena.combatImpacts.size()==2&&arena.combatImpacts.back().material==ImpactMaterial::Armor&&
              arena.combatImpacts.back().type==enemyInfo(type).attackType,
              "enemy melee contact emits one hero burst with its damage type");
        Game feedback=emptyArena();
        V point=feedback.player+V{1.95f,0};
        feedback.enemies.push_back({0,type,point,point,100,100});
        auto& target=feedback.enemies[0];
        target.attack=target.windup=10;
        check(feedback.attack({1,0})&&feedback.combatImpacts.size()==1&&length(target.hitLean())>.01f,
              "all enemy types spark and visibly lean on a normal player strike");
        check(feedback.combatImpacts[0].material==impactMaterial(type),"hit particles retain the struck enemy material");
        const V initial=target.hitLean();
        for(int tick=0;tick<4;tick++)feedback.update(.025f,{},false);
        check(length(target.hitLean())>0&&target.hitLean().x*initial.x<0&&distance(target.pos,point)<.001f,
              "hit reaction returns through a counter-sway without changing world position");
        for(int tick=0;tick<7;tick++)feedback.update(.025f,{},false);
        check(feedback.combatImpacts.empty()&&length(target.hitLean())==0,"impact particles and reaction expire");
    }
    check(impactMaterial(0)==ImpactMaterial::Slime&&impactMaterial(3)==ImpactMaterial::Stone&&
          impactMaterial(7)==ImpactMaterial::Bark,"slime, stone guardian and treant use distinct impact materials");
    Game feedback=emptyArena();V point=feedback.player+V{1,0};
    feedback.enemies.push_back({0,0,point,point,1,1});
    feedback.explode(point,20,true);
    check(feedback.combatImpacts.size()==1&&feedback.combatImpacts[0].critical&&
          feedback.combatImpacts[0].type==DamageType::Magic&&!feedback.enemies[0].alive,
          "lethal spell impacts still leave a visible burst");
    for(int type:{3,7})for(int fps:{30,60,120}){
        Game arena=emptyArena();V p=arena.player+V{1.18f,0};
        arena.enemies.push_back({13,type,p,p,500,500});
        auto& foe=arena.enemies[0];foe.boulderCd=10;
        const float dt=1.f/fps,contact=meleeWindup(type);
        arena.update(dt,{},false);
        check(foe.windup==contact&&foe.heavyMeleePhase()==0,"heavy melee opens with the complete visible windup");
        Game dodge=arena;
        int frame=0;
        while(foe.windup>0&&frame++<fps*2){
            if(dodge.enemies[0].windup<=.1f&&!dodge.rolling()&&dodge.rollCd<=0)
                dodge.startRoll({-1,0});
            dodge.update(dt,{},false);
            if(foe.windup>dt+.00001f)check(arena.hp==100,"heavy melee deals no damage before the contact pose");
            arena.update(dt,{},false);
        }
        check(std::abs(frame*dt-contact)<=dt+.00001f&&foe.strike>0&&arena.hp<100,
              "heavy melee hits on the authored contact and enters recovery");
        check(dodge.hp==100&&dodge.combatImpacts.empty(),"rolling before the heavy impact avoids damage and the hit burst");
        const float hp=arena.hp;
        for(int i=0;i<int(meleeRecovery(type)*fps);i++)arena.update(dt,{},false);
        check(arena.hp==hp,"heavy melee recovery never deals a second hit");
    }
}

