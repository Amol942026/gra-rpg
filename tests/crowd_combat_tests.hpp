#pragma once

bool separatedEnemies(const Game& g,float tolerance=.001f) {
    for(size_t i=0;i<g.enemies.size();i++)for(size_t j=i+1;j<g.enemies.size();j++){
        const auto& a=g.enemies[i];const auto& b=g.enemies[j];
        if(a.alive&&b.alive&&distance(a.pos,b.pos)+tolerance<a.radius()+b.radius())return false;
    }
    return true;
}
bool enemiesClearOfScenery(const Game& g) {
    for(const auto& e:g.enemies)if(e.alive){
        float r=e.radius();
        if(!std::isfinite(e.pos.x)||!std::isfinite(e.pos.y)||e.pos.x<1.6f+r-.001f||
           e.pos.y<1.6f+r-.001f||e.pos.x>MapWalkMax-r+.001f||e.pos.y>MapWalkMax-r+.001f)return false;
        for(const auto& p:g.props)if(distance(e.pos,p.pos)+.001f<r+propRadius(p.type))return false;
    }
    return true;
}
void crowdCombatTests() {
    const V directions[]={{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1}};
    Game area=emptyArena();area.animation.direction=Direction::NW;
    for(int i=0;i<8;i++){
        V pos=area.player+normalized(directions[i])*1.9f;
        area.enemies.push_back({i,i%4,pos,pos,100,100});
    }
    V far=area.player+V{MeleeRange+.02f,0},edge=area.player+V{MeleeRange,0};
    area.enemies.push_back({8,0,far,far,100,100});
    area.enemies.push_back({9,0,edge,edge,100,100});
    area.enemies.push_back({10,0,area.player,area.player,0,100});area.enemies.back().alive=false;
    check(area.attack(),"one swing can hit a group without selecting a target");
    for(int i=0;i<8;i++)check(std::abs(area.enemies[i].hp-(100-damageAfterArmor(14,area.enemies[i].defense())))<.001f,
        "AoE hits every direction once, applying each monster's armor");
    check(area.enemies[8].hp==100&&area.enemies[9].hp==86,"AoE respects the inclusive 2.1 field radius");
    check(area.enemies[10].hp==0&&area.kills==0,"AoE ignores corpses");
    check(cueCount(area,Cue::Swing)==1&&area.animation.direction==Direction::NW,"one swing sound and no automatic turn toward a target");
    check(!area.attack()&&area.enemies[0].hp==86,"cooldown prevents repeated group damage");

    Game kills=emptyArena();kills.xp=75;
    for(int i=0;i<5;i++){V p=kills.player+normalized(directions[i])*1.5f;
        kills.enemies.push_back({i,0,p,p,i==0||i==4?100.f:1.f,100});}
    kills.attack();
    check(kills.kills==3&&kills.level==2&&kills.ground.empty()&&!kills.enemies[1].loot[0].empty()&&!kills.enemies[2].loot[0].empty()&&!kills.enemies[3].loot[0].empty(),"one swing awards XP and corpse loot for every defeated enemy");
    check(kills.enemies[0].hp==86&&kills.enemies[4].hp==86,"level-up mid-swing cannot change damage for later victims");

    Game presses=emptyArena();
    for(int i=0;i<4;i++){V p=presses.player+normalized(directions[i*2])*1.5f;
        presses.enemies.push_back({i,0,p,p,500,500});}
    presses.update(1.f/60,{},true);
    for(int frame=0;frame<120;frame++){presses.hp=100;presses.update(1.f/60,{},false);}
    bool once=true;for(auto& e:presses.enemies)once&=e.hp==486;
    check(once&&cueCount(presses,Cue::Swing)==1,"group receives only one hit with no new left mouse press");
    presses.update(1.f/60,{},true);bool twice=true;for(auto& e:presses.enemies)twice&=e.hp==472;
    check(twice&&cueCount(presses,Cue::Swing)==2,"second press hits the whole group once again");

    for(int type=0;type<EnemyTypeCount;type++){
        Game overlap=emptyArena();overlap.player={2,2};
        overlap.enemies.push_back({20,type,{15,15},{15,15},100,100});
        overlap.enemies.push_back({21,0,{15.1f,15},{15.1f,15},100,100});
        overlap.update(1.f/60,{},false);
        check(separatedEnemies(overlap)&&overlap.enemies[0].pos.x<15&&overlap.enemies[1].pos.x>15.1f,
            "overlapping idle creatures both separate, including the larger boss");
    }
    Game pile=emptyArena();pile.player={2,2};
    for(int i=0;i<14;i++)pile.enemies.push_back({i,i%4,{15,15},{15,15},100,100});
    pile.update(1.f/60,{},false);
    check(separatedEnemies(pile)&&enemiesClearOfScenery(pile),"coincident spawns separate without NaNs or residual overlap");

    // Head-on impacts exercise the same physics as chasing, with explicit opposing speeds.
    for(int fps:{20,30,60,120}){
        Game headOn=emptyArena();headOn.player={2,2};
        headOn.enemies.push_back({0,1,{14,15},{14,15},100,100});
        headOn.enemies.push_back({1,1,{16,15},{16,15},100,100});
        bool clear=true,bounced=false;
        for(int frame=0;frame<fps;frame++){
            headOn.moveEnemies(1.f/fps,{{2.6f,0},{-2.6f,0}});
            clear&=separatedEnemies(headOn)&&headOn.enemies[0].pos.x<headOn.enemies[1].pos.x;
            bounced|=headOn.enemies[0].recoil.x<-2.6f&&headOn.enemies[1].recoil.x>2.6f;
        }
        check(clear&&bounced,"head-on enemies rebound and never cross at 20/30/60/120 FPS");
    }
    for(int fps:{30,60,120})for(bool obstacles:{false,true}){
        Game crowd=emptyArena();crowd.player=obstacles?V{2.2f,2.2f}:V{15,15};
        if(obstacles){crowd.props.push_back({{4,4},0,1});crowd.props.push_back({{6,3},1,1});}
        for(int i=0;i<14;i++){
            float angle=i*6.2831853f/14;
            V pos=obstacles?V{3.1f+(i%4)*1.4f,6.f+(i/4)*1.4f}:crowd.player+V{std::cos(angle)*4.5f,std::sin(angle)*4.5f};
            crowd.enemies.push_back({i,i%4,pos,pos,100,100});
        }
        bool clear=true,worldClear=true;int strikes=0;
        for(int frame=0;frame<fps*12;frame++){
            crowd.hp=100;crowd.update(1.f/fps,{},false);
            clear&=separatedEnemies(crowd);worldClear&=enemiesClearOfScenery(crowd);
            for(auto& sound:crowd.sounds)if(sound.cue==Cue::EnemyAttack)strikes++;
            crowd.sounds.clear();
        }
        check(clear,"14 chasing monsters do not overlap throughout a 12 second crowd simulation");
        if(!worldClear){
            std::cerr<<"Scenery failure at "<<fps<<" FPS, obstacles="<<obstacles<<'\n';
            for(const auto& e:crowd.enemies){std::cerr<<e.id<<": "<<e.pos.x<<", "<<e.pos.y<<" r="<<e.radius();
                for(const auto& p:crowd.props)std::cerr<<" gap="<<distance(e.pos,p.pos)-e.radius()-propRadius(p.type);
                std::cerr<<'\n';}
        }
        check(worldClear,"crowd cannot push bodies through trees, rocks or map edges");
        check(strikes>0,"collision handling still allows the crowd to reach and attack the player");
    }
    Game corpse=emptyArena();corpse.player={20,15};
    corpse.enemies.push_back({0,1,{15,15},{15,15},100,100});
    corpse.enemies.push_back({1,3,{15.1f,15},{15.1f,15},0,100});corpse.enemies[1].alive=false;
    corpse.update(.05f,{},false);
    check(std::abs(corpse.enemies[0].pos.x-15.13f)<.001f,"dead enemies no longer block living ones");
}


