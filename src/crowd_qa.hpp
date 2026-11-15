#pragma once

bool runCrowdQA() {
    fs::create_directories("qa-v13");std::ofstream report("qa-v13/crowd-tests.txt");
    int checks=0,failures=0;
    auto check=[&](bool ok,const char* label){checks++;if(!ok)failures++;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';};
    auto reset=[](){game=Game{};game.props.clear();game.enemies.clear();game.ground.clear();game.player={15,15};
        welcome=help=paused=audioOpen=false;dragging={};mouse={0,0};};
    reset();prepareCanvas(1.2f);
    const V positions[]={{-1.6f,-.2f},{-.8f,-1.6f},{.8f,-1.6f},{1.8f,-.1f},{.5f,1.7f},{-1.1f,1.5f},{4,0}};
    for(int i=0;i<7;i++){int type=i==6?3:i%3;V p=game.player+positions[i];float hp=enemyInfo(type).health;
        game.enemies.push_back({i,type,p,p,hp,hp});}
    game.animation.direction=Direction::E;
    game.say("LPM — jeden zamach trafia przeciwników z przodu w zasięgu 2,1 pola.");
    renderScene();saveTexture(canvas,"qa-v13/area-before.png");
    for(int i=0;i<2;i++){
        const Vector2 p=screen(game.enemies[i].pos);
        bool found=false;
        for(int y=8;y<=42&&!found;y+=2)for(int x=-16;x<=16&&!found;x+=2){
            const Vector2 point{p.x+x,p.y-y};
            const auto hit=pickWorld(point);
            if(hit.kind==HoverKind::Enemy&&hit.index==game.enemies[i].id){mouse=point;found=true;}
        }
        check(found,"test pointer is over the intended monster");
    }
    clickWorldOrInventory();releaseWorldAttack();mouse={0,0};
    check(game.swing>0&&game.attackCd>0,"left tap releases one sword swing");
    check(inspectWorld(pickWorld(mouse)).source==HoverKind::None,"moving the pointer away leaves no selected enemy in HUD");
    for(int i=0;i<6;i++)check((game.enemies[i].hp<game.enemies[i].maxHp)==(i<3),"one left click hits nearby monsters in the aimed half of the ring");
    check(game.enemies[6].hp==game.enemies[6].maxHp,"distant boss is outside the attack area");
    game.swing=SwingDuration*.5f;for(auto& e:game.enemies)e.flash=0;
    renderScene();saveTexture(canvas,"qa-v13/area-swing.png");

    reset();
    for(int i=0;i<14;i++){
        float angle=i*2*PI/14;V p=game.player+V{std::cos(angle)*4.5f,std::sin(angle)*4.5f};
        int type=i==13?3:i%3;float hp=enemyInfo(type).health;
        game.enemies.push_back({i,type,p,p,hp,hp});
    }
    bool separated=true;
    for(int frame=0;frame<360;frame++){
        game.hp=100;game.update(1.f/60,{},false);game.sounds.clear();
        for(size_t i=0;i<game.enemies.size();i++)for(size_t j=i+1;j<game.enemies.size();j++){
            const auto& a=game.enemies[i];const auto& b=game.enemies[j];
            separated&=distance(a.pos,b.pos)+.001f>=a.radius()+b.radius();
        }
    }
    check(separated,"all 14 chasing enemies remain separated during 360 frames");
    game.effects.clear();game.hp=100;game.hurt=0;game.say("Potwory odpychają się wzajemnie i nie zajmują tego samego miejsca.");
    renderScene();saveTexture(canvas,"qa-v13/crowd.png");
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
