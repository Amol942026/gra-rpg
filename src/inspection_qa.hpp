#pragma once

bool runInspectionQA() {
    fs::create_directories("qa-v13");std::ofstream report("qa-v13/inspection-tests.txt");int checks=0,failures=0;
    auto check=[&](bool ok,const char* text){checks++;if(!ok)failures++;report<<(ok?"PASS: ":"FAIL: ")<<text<<'\n';report.flush();};
    auto reset=[](){game=Game{};game.props.clear();game.enemies.clear();game.ground.clear();game.forage.clear();weaponWheelOpen=false;welcome=paused=help=audioOpen=false;dragging={};mouse={0,0};};
    reset();prepareCanvas(1.2f);
    for(int type=0;type<4;type++){
        reset();const auto& stats=enemyInfo(type);V position=game.player+V{1,-2};
        game.enemies.push_back({73,type,position,position,stats.health,stats.health});
        auto& enemy=game.enemies[0];enemy.attack=99;
        Vector2 p=screen(position);mouse={p.x,p.y-(type==0?18.f:type==1?36.f:type==2?66.f:92.f)};
        WorldHover hit=pickWorld(mouse);Inspection info=inspectWorld(hit);
        check(hit.kind==HoverKind::Enemy&&hit.index==73,"pointer finds stable enemy ID for inspection");
        check(info.source==HoverKind::None,"living enemies have no sidebar inspection text");
        game.hit(enemy,10);info=inspectWorld(hit);
        check(enemy.hp<enemy.maxHp&&inspectWorld(hit).source==HoverKind::None,"damage leaves health in the overhead bar only");
        // A hit leans the rig away from the blow. Find a point on the current
        // silhouette before exercising the click-to-aim path.
        const Vector2 beforeHit=mouse;
        auto pointsAtEnemy=[](Vector2 point){
            const WorldHover picked=pickWorld(point);
            return picked.kind==HoverKind::Enemy&&picked.index==73;
        };
        bool currentTarget=pointsAtEnemy(mouse);
        for(int y=-12;y<=12&&!currentTarget;y+=2)for(int x=-12;x<=12&&!currentTarget;x+=2){
            const Vector2 point{beforeHit.x+x,beforeHit.y+y};
            if(pointsAtEnemy(point)){mouse=point;currentTarget=true;}
        }
        check(currentTarget,"the damaged monster remains selectable in its current hit pose");
        const Direction direction=directionFor(position.x-game.player.x,position.y-game.player.y,game.animation.direction);
        check(clickWorldOrInventory()&&releaseWorldAttack(),"left tap on a monster starts one area swing");
        check(game.animation.direction==direction&&game.swing>0&&game.attackCd>0&&dragging.place==Place::None,"monster click aims the swing without starting item drag");
        game.swing=game.attackCd=0;
        activateMouseAction(0,{});
        check(game.swing==0&&game.attackCd==0,"clicking sword indicator cannot attack");
        renderScene();saveTexture(canvas,std::string("qa-v13/enemy-")+std::to_string(type)+".png");
        enemy.alive=false;check(inspectWorld(hit).source==HoverKind::None,"dead enemy cannot leave stale details");
    }
    for(int type=0;type<4;type++){
        reset();V position=game.player+V{1,-2};game.props.push_back({position,type,1});
        Vector2 p=screen(position);bool found=false;
        // Detailed bushes have gaps between their leaves. Inspect an actually
        // rendered part rather than assuming that the geometric center is opaque.
        for(int y=-190;y<=5&&!found;y+=2)for(int x=-65;x<=65&&!found;x+=2){
            Vector2 point{p.x+x,p.y+y};WorldHover candidate=pickWorld(point);
            if(candidate.kind==HoverKind::Prop&&candidate.index==0){mouse=point;found=true;}
        }
        WorldHover hit=pickWorld(mouse);Inspection info=inspectWorld(hit);
        check(hit.kind==HoverKind::Prop&&info.name==propInfo(type).name&&info.kind==propInfo(type).kind,"terrain object name and type shown from its actual hovered shape");
        if(type==0){renderScene();saveTexture(canvas,"qa-v13/tree.png");}
    }
    reset();game.ground.clear();game.addGround(item(Kind::Health,0,3),game.player+V{1,-2});
    Vector2 p=screen(game.ground[0].pos);mouse={p.x,p.y-14};
    check(inspectWorld(pickWorld(mouse)).source==HoverKind::Ground,"ground loot also has an inspection card");
    mouse={1130,747};check(inspectWorld(pickWorld(mouse)).source==HoverKind::None,"moving into HUD clears world inspection");
    paused=true;mouse={p.x,p.y-14};check(inspectWorld(pickWorld(mouse)).source==HoverKind::None,"pause clears inspection");paused=false;
    reset();nearbyPage=0;
    game.chests[0].location=game.location;game.chests[0].pos=game.player+V{0,1.5f};
    V body=game.player+V{1.2f,0};
    game.enemies.push_back({95,0,body,body,0,50});game.enemies.back().alive=false;
    game.enemies.back().loot[0]=item(Kind::Gold,0,8);
    const auto entries=nearbyEntries();
    check(entries.size()==2&&entries[0].target.kind==HoverKind::Chest&&entries[1].target.kind==HoverKind::Corpse,
          "nearby panel finds chests and fallen enemies alongside normal pickups");
    mouse={lootRect(1).x+26,lootRect(1).y+18};renderScene();saveTexture(canvas,"qa-v13/nearby-containers.png");
    leftClickGameplay({});
    check(game.openedCorpse==95&&hoverSlot().place==Place::Corpse,"left click opens the corpse loot in the nearby panel");
    game.closeCorpse();mouse={lootRect(0).x+26,lootRect(0).y+18};
    check(rightClickWorldOrInventory()&&game.openedChest==game.chests[0].id,"right click opens a nearby chest");
    game.closeCorpse();game.player=game.player+V{6,0};
    check(nearbyEntries().empty(),"out-of-reach bodies and chests leave the tray");
    {
        const auto& atlas=fallen_art::thumbnails.texture;
        Image image=LoadImageFromTexture(atlas);
        if(image.data)ImageFlipVertical(&image);
        const Rectangle card=lootRect(0);
        check(fallen_art::ThumbnailWidth==fallen_art::ThumbnailDensity*(card.width-2)&&
              fallen_art::ThumbnailHeight==fallen_art::ThumbnailDensity*(card.height-2),
              "the corpse atlas fills the interior of its nearby card at double resolution");
        bool complete=image.data!=nullptr;
        std::vector<std::vector<unsigned char>> silhouettes;
        if(image.data){
            Color* pixels=LoadImageColors(image);
            for(int type=0;type<EnemyTypeCount;type++)for(int direction=0;direction<fallen_art::Directions;direction++){
                const Rectangle r=fallen_art::thumbnailCell(type,direction);
                int visible=0,clipped=0;
                std::vector<unsigned char> shape;
                for(int y=0;y<r.height;y++)for(int x=0;x<r.width;x++){
                    const auto p=pixels[(int(r.y)+y)*image.width+int(r.x)+x];
                    const bool opaque=p.a>96;
                    shape.push_back(opaque);
                    visible+=opaque;
                    clipped+=opaque&&(x==0||y==0||x==r.width-1||y==r.height-1);
                }
                complete&=fallen_art::thumbnailReady[type][direction]&&visible>140&&clipped==0;
                if(direction!=int(Direction::SE))continue;
                complete&=std::find(silhouettes.begin(),silhouettes.end(),shape)==silhouettes.end();
                silhouettes.push_back(std::move(shape));
            }
            UnloadImageColors(pixels);UnloadImage(image);
        }
        check(complete&&silhouettes.size()==EnemyTypeCount,
              "all monster species have distinct, fitted corpse thumbnails across all eight directions");
        reset();
        const int species[]={0,2,DesertBossType,VolcanoBossType,CinderhornType};
        for(int i=0;i<int(std::size(species));i++){
            const V at=game.player+V{float(i-2)*.57f,1.05f};
            game.enemies.push_back({201+i,species[i],at,at,0,100});
            auto& e=game.enemies.back();
            e.alive=false;e.animation.direction=Direction(i);
        }
        check(nearbyEntries().size()==std::size(species),"nearby tray accepts visible bodies from every renderer");
        renderScene();saveTexture(canvas,"qa-v13/nearby-corpse-thumbnails.png");
        mouse={card.x+card.width*.5f,card.y+card.height-5};
        const bool pointsAtBody=nearbyUnderPointer().target.index==201;
        leftClickGameplay({});
        check(pointsAtBody&&game.openedCorpse==201,
              "the bottom of the enlarged thumbnail opens the same corpse loot");
        game.closeCorpse();
    }
    for(int i=0;i<BagSize;i++){
        Rectangle r=bagRect(i);mouse={r.x+r.width/2,r.y+r.height/2};Slot slot=hoverSlot();
        check(slot.place==Place::Bag&&slot.index==i,"all 25 compact backpack slots remain clickable");
    }
    mouse={0,0};renderScene();saveTexture(canvas,"qa-v13/idle-hud.png");
    welcome=true;renderScene();saveTexture(canvas,"qa-v13/help.png");
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
