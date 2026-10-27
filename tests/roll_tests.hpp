#pragma once

Game emptyArena() {
    Game g;g.props.clear();g.enemies.clear();g.ground.clear();g.player={15,15};
    return g;
}
void rollTests() {
    for(bool boots:{false,true}){
        Game g=emptyArena();if(boots)g.equipment[4]=item(Kind::Boots);
        for(int i=0;i<20;i++)g.update(.05f,{1,0},false);
        check(std::abs(g.player.x-15-(boots?4.3f:3.8f))<.001f,"faster walking, including equipment bonus");
    }
    for(bool boots:{false,true})for(int fps:{30,60,120})for(V direction:{V{1,0},V{1,-1}}){
        Game runner=emptyArena();if(boots)runner.equipment[4]=item(Kind::Boots);
        for(int i=0;i<fps;i++)runner.update(1.f/fps,direction,false,true);
        const float expected=(boots?BootsSpeed:WalkSpeed)*RunMultiplier;
        check(distance(runner.player,V{15,15}+normalized(direction)*expected)<.002f,"held run adds 50% speed, normalizes diagonals and is FPS independent with boots");
        check(!runner.rolling()&&runner.swing==0&&runner.mana==100,"running does not roll, attack or spend mana");
        V before=runner.player;runner.update(.05f,direction,false,false);
        check(std::abs(distance(before,runner.player)-(boots?BootsSpeed:WalkSpeed)*.05f)<.001f,"releasing run immediately restores walking speed");
    }
    Game idleRun=emptyArena();idleRun.update(.05f,{},false,true);
    check(distance(idleRun.player,V{15,15})==0&&!idleRun.animation.moving,"holding run while idle does not move the hero");
    Game runRoll=emptyArena();runRoll.startRoll({1,-1});
    for(int i=0;i<8;i++)runRoll.update(.05f,{},false,true);
    check(distance(runRoll.player,V{15,15}+normalized(V{1,-1})*2.8f)<.002f,"run does not multiply the existing roll distance");
    Game runWall=emptyArena();runWall.props.push_back({{15.7f,15},0,1});
    for(int i=0;i<20;i++)runWall.update(.05f,{1,0},false,true);
    check(runWall.walkable(runWall.player)&&runWall.player.x<15.7f,"running respects terrain collisions");
    const float speeds[]={1.85f,2.6f,1.85f,1.35f,2.25f,2.8f,2.2f,1.45f,2.35f,2.1f,2.7f,1.45f};
    static_assert(sizeof(speeds)/sizeof(speeds[0])==EnemyTypeCount);
    for(int type=0;type<EnemyTypeCount;type++){
        Game g=emptyArena();g.player={20,15};
        g.enemies.push_back({0,type,{15,15},{15,15},100,100});
        g.enemies.back().boulderCd=BoulderCooldown; // Isolate chase speed from the boss's stationary throw windup.
        for(int i=0;i<20;i++)g.update(.05f,{},false);
        check(std::abs(g.enemies[0].pos.x-15-speeds[type])<.001f,"all enemy types have faster movement");
    }
    const V directions[]={{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1}};
    for(int d=0;d<8;d++){
        Game g=emptyArena();V start=g.player;
        check(g.startRoll(directions[d]),"roll starts in every movement direction");
        check(!g.startRoll(directions[(d+4)%8]),"cannot restart or redirect an active roll");
        for(int i=0;i<8;i++)g.update(.05f,{},false);
        check(distance(g.player,start+normalized(directions[d])*2.8f)<.002f&&!g.rolling(),"roll covers equal distance in all eight directions and ends");
        check(int(g.animation.direction)==d,"roll keeps its visual facing");
        Game idle=emptyArena();idle.animation.direction=static_cast<Direction>(d);
        check(idle.startRoll({})&&distance(idle.rollDirection,normalized(directions[d]))<.001f,"stationary roll follows last visual facing");
    }
    Game control=emptyArena();control.startRoll({1,0});
    control.update(.2f,{-1,0},true); // Long frame is deliberately clamped to 50 ms.
    check(control.player.x>15&&control.player.y==15&&control.swing==0,"roll locks direction and suppresses held attack");
    check(!control.attack()&&!control.spell()&&control.mana==100,"no attacks or magic during roll, no mana cost");
    V frozen=control.player;float remaining=control.rollRemaining;
    control.update(0,{},false);
    check(distance(frozen,control.player)==0&&remaining==control.rollRemaining,"zero elapsed time freezes roll");
    for(int i=0;i<7;i++)control.update(.05f,{},false);
    check(!control.rolling()&&!control.startRoll({1,0}),"cooldown persists after the roll ends");
    check(control.attack()&&control.spell(),"combat resumes after rolling");
    for(int i=0;i<17;i++)control.update(.05f,{},false);
    check(control.startRoll({-1,0}),"roll is reusable when cooldown expires");
    control.hp=0;control.update(.05f,{},false);
    check(!control.rolling()&&!control.startRoll({1,0}),"death stops roll and prevents another");

    for(int type=0;type<PropTypeCount;type++){
        Game wall=emptyArena();wall.player={14,15};wall.props.push_back({{15,15},type,1});
        wall.startRoll({1,0});bool valid=true;
        for(int i=0;i<8;i++){wall.update(.05f,{},false);valid=valid&&wall.walkable(wall.player);}
        check(valid&&wall.player.x<15,"roll cannot tunnel through any scenery type");
    }
    Game edge=emptyArena();edge.player={MapWalkMax-.1f,MapWalkMax-.1f};edge.startRoll({1,1});
    for(int i=0;i<8;i++)edge.update(.05f,{},false);
    check(edge.walkable(edge.player)&&edge.player.x<=MapWalkMax&&edge.player.y<=MapWalkMax,"roll respects map borders");

    for(int fps:{30,60,120}){
        Game g=emptyArena();g.startRoll({1,-1});
        for(int i=0;i<fps;i++)g.update(1.f/fps,{},false);
        check(distance(g.player,V{15,15}+normalized(V{1,-1})*2.8f)<.002f&&!g.rolling(),"roll distance and duration are independent of FPS");
    }
    // Hold the hero against a tree so range cannot mask an immunity failure.
    Game protection=emptyArena();protection.props.push_back({{15.6f,15},0,1});
    protection.enemies.push_back({0,3,{14.5f,15},{14.5f,15},100,100});
    protection.startRoll({1,0});protection.enemies[0].windup=.01f;
    protection.update(.05f,{},false);
    check(protection.hp==100&&!protection.effects.empty()&&protection.effects.back().text=="UNIK","early roll prevents a real enemy strike in range");
    for(int i=0;i<4;i++)protection.update(.05f,{},false);
    protection.enemies[0].windup=.02f;protection.update(.05f,{},false);
    check(protection.hp==100,"impact before protection boundary is dodged even if frame ends afterward");
    protection.enemies[0].windup=.01f;protection.update(.05f,{},false);
    check(protection.hp<100&&protection.rolling(),"late recovery can be hit before roll animation ends");
    protection.update(.05f,{},false);float hp=protection.hp;
    protection.enemies[0].windup=.01f;protection.update(.05f,{},false);
    check(protection.hp<hp&&!protection.rolling(),"protection cannot persist after rolling");
}



