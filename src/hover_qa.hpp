#pragma once
void runHoverQA() {
    fs::create_directories("qa-v04");
    std::ofstream report("qa-v04/hover-tests.txt");int checks=0;
    auto check=[&](bool result,const char* label){
        report<<(result?"PASS: ":"FAIL: ")<<label<<"\n";report.flush();checks++;
        if(!result)std::exit(2);
    };
    game=Game{};game.player={15,15};game.props.clear();game.enemies.clear();game.ground.clear();game.messageTime=0;
    welcome=help=paused=false;dragging={};prepareCanvas(1.2f);
    auto at=[&](V world){return pickWorld(screen(world));};
    auto tile=at({12,16});
    check(tile.tileValid&&tile.tile.x==12&&tile.tile.y==16&&tile.kind==HoverKind::None,"exact isometric tile under cursor");
    tile=at({12.49f,16.49f});check(tile.tile.x==12&&tile.tile.y==16,"tile inside edge");
    tile=at({12.51f,16});check(tile.tile.x==13&&tile.tile.y==16,"adjacent tile across edge");
    for(Vector2 p:std::initializer_list<Vector2>{{-1,400},{1100,400},{900,200},{500,50},{370,825},{150,730}})
        check(!pickWorld(p).tileValid,"HUD, minimap and outside viewport do not select terrain");
    paused=true;check(!at({12,16}).tileValid,"pause disables world hover");paused=false;
    welcome=true;check(!at({12,16}).tileValid,"welcome disables world hover");welcome=false;
    mouse=screen({12,16});renderScene();saveTexture(canvas,"qa-v04/hover-terrain.png");

    game.props.push_back({{14,16},0,1});Vector2 base=screen({14,16});mouse={base.x,base.y-100};
    auto hit=pickWorld(mouse);check(hit.kind==HoverKind::Prop&&hit.index==0,"tree silhouette is selected");
    check(!propContains(game.props[0],{base.x-85,base.y-190}),"transparent sprite corner is not selected");
    check(!propContains(game.props[0],{base.x+30,base.y+3}),"tree shadow is not treated as solid object");
    renderScene();saveTexture(canvas,"qa-v04/hover-tree.png");
    for(int type=1;type<4;type++){
        game.props[0].type=type;
        check(propContains(game.props[0],{base.x,base.y-18}),"rock, bush and column use their opaque masks");
    }
    game.props[0].type=0;
    game.enemies.push_back({73,2,{13.5f,15.5f},{13.5f,15.5f},54,54});
    mouse={base.x,base.y-70};
    check(pickWorld(mouse).kind==HoverKind::Prop,"foreground tree blocks selection of enemy behind it");
    game.props[0].pos={13.5f,15.5f};game.enemies[0].pos={14,16};mouse={base.x,base.y-66};
    hit=pickWorld(mouse);check(hit.kind==HoverKind::Enemy&&hit.index==73,"foreground enemy wins over background tree and keeps stable ID");
    game.props.clear();renderScene();saveTexture(canvas,"qa-v04/hover-enemy.png");
    game.enemies[0].alive=false;check(pickWorld(mouse).kind==HoverKind::None,"dead enemy cannot be hovered");
    game.enemies.clear();game.addGround(item(Kind::Sword,1),{14,16});mouse={base.x,base.y-14};
    hit=pickWorld(mouse);check(hit.kind==HoverKind::Ground&&hit.index==game.ground[0].id,"ground loot is highlighted");
    check(hoverSlot().place==Place::Ground,"loot drag uses same target as hover");
    renderScene();saveTexture(canvas,"qa-v04/hover-loot.png");
    game.props.push_back({{14,16},0,1});
    check(pickWorld(mouse).kind==HoverKind::Prop&&hoverSlot().place==Place::None,"occluded loot is not selected through tree");
    game.props.clear();game.enemies.push_back({74,2,{14.5f,14.5f},{14.5f,14.5f},54,54});
    Vector2 player=screen(game.player);mouse={player.x,player.y-62};
    check(pickWorld(mouse).kind!=HoverKind::Enemy,"player blocks enemy silhouette behind them");
    report<<"PASS: "<<checks<<" hover checks\n";
}
