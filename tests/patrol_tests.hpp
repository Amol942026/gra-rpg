#pragma once

void patrolTests(){
    for(int type=0;type<EnemyTypeCount;type++){
        Game g=emptyArena();g.player={2,2};g.enemies.push_back({71,type,{15,15},{15,15},100,100});
        int moving=0,resting=0;bool bounded=true,clear=true;
        for(int frame=0;frame<3600;frame++){
            g.update(1.f/60,{},false);const Enemy& e=g.enemies[0];
            moving+=e.animation.moving;resting+=!e.animation.moving;
            bounded&=distance(e.pos,e.home)<=PatrolRadius+.02f;
            clear&=enemiesClearOfScenery(g)&&e.behavior!=EnemyBehavior::Chase;
        }
        check(moving>600&&resting>120,"every unseen monster species alternates walking and resting for a full minute");
        check(bounded&&clear,"unseen monsters stay around their own spawn and within walkable terrain");
    }
    for(int type=0;type<EnemyTypeCount;type++)for(int fps:{30,60,120}){
        Game g=emptyArena();g.player={2,2};g.enemies.push_back({7,type,{15,15},{15,15},100,100});
        auto& e=g.enemies[0];e.behavior=EnemyBehavior::Patrol;e.patrolGoal={16.8f,15};e.patrolGoalActive=true;
        for(int i=0;i<fps;i++)g.update(1.f/fps,{},false);
        check(std::abs(e.pos.x-15-enemySpeed(type)*PatrolSpeedFactor)<.002f,"patrol speed uses 60% of chase speed independently of FPS for every species");
    }
    for(int type=0;type<PropTypeCount;type++){
        Game g=emptyArena();g.player={18,15};g.enemies.push_back({4,0,{15,15},{15,15},100,100});
        check(g.canSeePlayer(g.enemies[0]),"unobstructed nearby hero is visible");
        g.props.push_back({{16.5f,15},type,1});
        check(!g.canSeePlayer(g.enemies[0]),"trees, rocks, bushes and columns block monster sight");
        g.update(.05f,{},false);
        check(g.enemies[0].behavior!=EnemyBehavior::Chase&&g.enemies[0].windup==0,"hidden hero does not start pursuit or an attack");
        g.props.clear();g.update(.05f,{},false);
        check(g.enemies[0].behavior==EnemyBehavior::Chase&&g.enemies[0].animation.moving,"clearing the view interrupts patrol and immediately starts pursuit");
    }
    Game range=emptyArena();range.player={22,15};range.enemies.push_back({3,0,{15,15},{15,15},100,100});
    check(!range.canSeePlayer(range.enemies[0]),"hero outside the 6.5 field sight radius is not detected");
    Game hiddenStrike=emptyArena();hiddenStrike.player={16.1f,15};
    hiddenStrike.enemies.push_back({5,0,{14.6f,15},{14.6f,15},100,100});hiddenStrike.props.push_back({{15.5f,15},1,1});
    hiddenStrike.enemies[0].windup=.01f;hiddenStrike.update(.02f,{},false);
    check(hiddenStrike.hp==100,"a prepared melee attack cannot hit through a sight-blocking rock");
    for(int type=0;type<EnemyTypeCount;type++){
        Game g=emptyArena();g.player={2,2};g.enemies.push_back({11,type,{20,15},{15,15},100,100});
        g.enemies[0].behavior=EnemyBehavior::Chase;g.props.push_back({{17.5f,15},1,1});
        bool visitedHome=false,clear=true;
        for(int i=0;i<1200;i++){
            g.update(1.f/60,{},false);visitedHome|=distance(g.enemies[0].pos,g.enemies[0].home)<.3f;
            clear&=enemiesClearOfScenery(g);
        }
        check(visitedHome&&clear,"losing the hero returns every species home around an obstacle before resuming patrol");
    }
    Game pause=emptyArena();pause.player={2,2};pause.enemies.push_back({3,0,{15,15},{15,15},100,100});pause.update(.05f,{},false);
    Enemy before=pause.enemies[0];pause.allocationRequired=true;pause.update(.05f,{1,0},true);
    const Enemy& after=pause.enemies[0];
    check(distance(before.pos,after.pos)==0&&before.patrolSequence==after.patrolSequence&&before.patrolWait==after.patrolWait,"level pause freezes patrol position, wait timer and waypoint sequence");
    pause.allocationRequired=false;pause.update(0,{},false);
    check(before.patrolSequence==pause.enemies[0].patrolSequence&&distance(before.pos,pause.enemies[0].pos)==0,"zero elapsed time cannot advance patrol");
    Game moving=emptyArena(),still=emptyArena();moving.player=still.player={2,2};
    moving.enemies.push_back({91,0,{15,15},{15,15},100,100});still.enemies=moving.enemies;
    for(int i=0;i<1800;i++)moving.update(1.f/60,{},false);
    moving.hit(moving.enemies[0],200);still.hit(still.enemies[0],200);
    for(int i=0;i<2;i++){
        const Item& a=moving.enemies[0].loot[i];const Item& b=still.enemies[0].loot[i];
        check(a.kind==b.kind&&a.tier==b.tier&&a.count==b.count,"patrol randomness never changes the random loot stream");
    }
    Enemy body=moving.enemies[0];for(int i=0;i<600;i++)moving.update(1.f/60,{},false);
    check(distance(body.pos,moving.enemies[0].pos)==0&&!moving.enemies[0].patrolGoalActive&&body.patrolSequence==moving.enemies[0].patrolSequence,"corpses never patrol or select new goals");
    Game real;real.player={2,2};bool roaming=false,valid=true;
    for(int frame=0;frame<1800;frame++){
        real.hp=100;real.update(1.f/60,{},false);roaming|=std::any_of(real.enemies.begin(),real.enemies.end(),[](const Enemy& e){return e.behavior==EnemyBehavior::Patrol&&e.animation.moving;});
        valid&=separatedEnemies(real)&&enemiesClearOfScenery(real)&&real.ground.empty();
    }
    check(roaming&&valid,"actual populated map patrols without starting ground loot, overlap or scenery penetration");
}

