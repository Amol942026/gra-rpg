#pragma once

bool runPatrolQA(){
    fs::create_directories("qa-v13");std::ofstream report("qa-v13/patrol-tests.txt");int checks=0,failures=0;
    auto check=[&](bool ok,const char* text){checks++;if(!ok)failures++;report<<(ok?"PASS: ":"FAIL: ")<<text<<'\n';report.flush();};
    auto reset=[](){game=Game{};welcome=help=paused=audioOpen=statsOpen=spellBookOpen=false;dragging={};mouse={0,0};nearbyPage=0;};
    reset();prepareCanvas(1.f);
    check(game.ground.empty()&&nearby().empty(),"new game has no world items or nearby loot tray entries");
    check(game.equipment[0].kind==Kind::Sword&&game.equipment[3].kind==Kind::Armor&&game.bag[0].count==5&&game.bag[1].count==3,"starting equipment and backpack supplies are preserved");
    renderScene();saveTexture(canvas,"qa-v13/startup-no-loot.png");
    bool patrolling=false;
    for(int i=0;i<300;i++){
        game.hp=100;game.update(1.f/60,{},false);
        for(const auto& e:game.enemies)patrolling|=e.behavior==EnemyBehavior::Patrol&&e.animation.moving;
    }
    check(game.ground.empty()&&patrolling,"ordinary gameplay begins patrols without creating starter loot");
    renderScene();saveTexture(canvas,"qa-v13/map-patrols.png");
    check(game.drop({Place::Bag,2},game.player)&&game.ground.size()==1,"player can still intentionally drop an inventory item");
    reset();check(game.ground.empty(),"a new expedition clears dropped items and starts empty again");
    for(int type=0;type<4;type++){
        reset();game.props.clear();game.enemies.clear();game.player={15,15};
        V home=game.player+V{1,7.5f};float hp=enemyInfo(type).health;
        game.enemies.push_back({31,type,home,home,hp,hp});Enemy& e=game.enemies[0];
        // A known valid patrol leg keeps all four rendered walk poses visible.
        e.behavior=EnemyBehavior::Patrol;e.patrolGoal=home+V{1.5f,-.3f};e.patrolGoalActive=true;
        for(int i=0;i<24;i++)game.update(1.f/60,{},false);
        check(!game.canSeePlayer(e)&&e.behavior==EnemyBehavior::Patrol&&e.animation.moving&&distance(e.pos,home)>.2f,"every species animates a real patrol leg while the hero is outside sight range");
        Vector2 p=screen(e.pos);bool picked=false;
        // Squashing and facing move the visible slime surface. Find an opaque
        // part of the current pose rather than assuming a fixed idle pixel.
        for(int y=-122;y<=10&&!picked;y+=2)for(int x=-54;x<=54&&!picked;x+=2){
            Vector2 point{p.x+x,p.y+y};WorldHover hit=pickWorld(point);
            if(hit.kind==HoverKind::Enemy&&hit.index==e.id){mouse=point;picked=true;}
        }
        check(picked&&inspectWorld(pickWorld(mouse)).source==HoverKind::Enemy,"patrolling monster retains mouse hover and HUD inspection");
        game.say("Potwór patroluje okolicę respawnu, gdy nie widzi bohatera.");renderScene();saveTexture(canvas,TextFormat("qa-v13/patrol-species-%d.png",type));
    }
    reset();
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
