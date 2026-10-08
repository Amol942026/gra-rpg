#pragma once

bool runInspectionQA() {
    fs::create_directories("qa-v13");std::ofstream report("qa-v13/inspection-tests.txt");int checks=0,failures=0;
    auto check=[&](bool ok,const char* text){checks++;if(!ok)failures++;report<<(ok?"PASS: ":"FAIL: ")<<text<<'\n';report.flush();};
    auto reset=[](){game=Game{};game.props.clear();game.enemies.clear();welcome=paused=help=audioOpen=false;dragging={};mouse={0,0};};
    reset();prepareCanvas(1.2f);
    for(int type=0;type<4;type++){
        reset();const auto& stats=enemyInfo(type);V position=game.player+V{1,-2};
        game.enemies.push_back({73,type,position,position,stats.health,stats.health});
        auto& enemy=game.enemies[0];enemy.attack=99;
        Vector2 p=screen(position);mouse={p.x,p.y-(type==0?18.f:type==1?36.f:type==2?66.f:92.f)};
        WorldHover hit=pickWorld(mouse);Inspection info=inspectWorld(hit);
        check(hit.kind==HoverKind::Enemy&&hit.index==73,"pointer finds stable enemy ID for inspection");
        check(info.name==stats.name&&info.kind==stats.kind&&info.attack==enemy.damage()&&info.defense==enemy.defense(),"HUD uses actual enemy identity and combat stats");
        game.hit(enemy,10);info=inspectWorld(hit);
        check(info.hp==enemy.hp&&info.maxHp==enemy.maxHp,"HUD health updates immediately after damage");
        const Direction direction=game.animation.direction;
        check(clickWorldOrInventory(),"left click on a monster starts one area swing");
        check(game.animation.direction==direction&&game.swing>0&&game.attackCd>0&&dragging.place==Place::None,"monster click swings without selecting a target or starting item drag");
        game.swing=game.attackCd=0;
        activateMouseAction(0,{});
        check(game.swing==0&&game.attackCd==0,"clicking sword indicator cannot attack");
        renderScene();saveTexture(canvas,std::string("qa-v13/enemy-")+std::to_string(type)+".png");
        enemy.alive=false;check(inspectWorld(hit).source==HoverKind::None,"dead enemy cannot leave stale details");
    }
    for(int type=0;type<4;type++){
        reset();V position=game.player+V{1,-2};game.props.push_back({position,type,1});
        Vector2 p=screen(position);mouse={p.x,p.y-(type==0?85.f:type==1?17.f:type==2?15.f:55.f)};
        WorldHover hit=pickWorld(mouse);Inspection info=inspectWorld(hit);
        check(hit.kind==HoverKind::Prop&&info.name==propInfo(type).name&&info.kind==propInfo(type).kind,"terrain object name and type shown from its actual hovered shape");
        check(info.maxHp==0,"scenery has no invented combat stats");
        if(type==0){renderScene();saveTexture(canvas,"qa-v13/tree.png");}
    }
    reset();game.ground.clear();game.addGround(item(Kind::Health,0,3),game.player+V{1,-2});
    Vector2 p=screen(game.ground[0].pos);mouse={p.x,p.y-14};
    check(inspectWorld(pickWorld(mouse)).source==HoverKind::Ground,"ground loot also has an inspection card");
    mouse={1130,747};check(inspectWorld(pickWorld(mouse)).source==HoverKind::None,"moving into HUD clears world inspection");
    paused=true;mouse={p.x,p.y-14};check(inspectWorld(pickWorld(mouse)).source==HoverKind::None,"pause clears inspection");paused=false;
    for(int i=0;i<BagSize;i++){
        Rectangle r=bagRect(i);mouse={r.x+r.width/2,r.y+r.height/2};Slot slot=hoverSlot();
        check(slot.place==Place::Bag&&slot.index==i,"all 25 compact backpack slots remain clickable");
    }
    mouse={0,0};renderScene();saveTexture(canvas,"qa-v13/idle-hud.png");
    welcome=true;renderScene();saveTexture(canvas,"qa-v13/help.png");
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
