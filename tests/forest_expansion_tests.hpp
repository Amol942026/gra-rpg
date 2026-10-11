#pragma once

void forestExpansionTests(){
    Game forest;float nearest=1e9f,minSpacing=1e9f;int guardians=0;std::array<bool,9> populated{};
    for(size_t i=0;i<forest.enemies.size();i++){
        const Enemy& e=forest.enemies[i];nearest=std::min(nearest,distance(e.home,forest.player));guardians+=e.type==3;
        populated[std::min(2,int(e.home.y)/24)*3+std::min(2,int(e.home.x)/24)]=true;
        for(size_t j=0;j<i;j++)minSpacing=std::min(minSpacing,distance(e.home,forest.enemies[j].home));
    }
    check(minSpacing>=8,"encounters have at least eight map units between their home positions");
    check(nearest>EnemySightRange+PatrolRadius+1,"forest arrival is beyond the entire patrol and sight range of nearby enemies");
    check(guardians==1&&distance(forest.enemies.back().home,ForestGuardian)<.001f,"the original northern guardian stays at the ritual ruins");
    check(std::all_of(populated.begin(),populated.end(),[](bool v){return v;}),"all nine large map sectors contain dispersed encounters");
    // Flood through actual scenery, checking each segment rather than assuming
    // that an unblocked road endpoint guarantees an open path between tiles.
    std::array<int,MapSize*MapSize> parent;parent.fill(-1);std::vector<int> queue;
    int start=19*MapSize+8;parent[start]=start;queue.push_back(start);
    auto point=[](int cell){return V{float(cell%MapSize),float(cell/MapSize)};};
    for(size_t head=0;head<queue.size();head++){
        int cell=queue[head];V p=point(cell);
        for(V delta:{V{1,0},V{-1,0},V{0,1},V{0,-1}}){
            V q=p+delta;if(!forest.walkable(q))continue;int next=int(q.y)*MapSize+int(q.x);
            if(parent[next]>=0||!forest.clearEnemySegment(p,q,.05f))continue;
            parent[next]=cell;queue.push_back(next);
        }
    }
    for(const auto& glade:ForestGlades){
        int goal=int(glade.center.y)*MapSize+int(glade.center.x);bool reachable=parent[goal]>=0;
        check(reachable,"every landmark clearing connects to the arrival through traversable forest");
        if(!reachable)continue;
        Game walking=forest;std::vector<int> route;for(int cell=goal;cell!=start;cell=parent[cell])route.push_back(cell);
        bool walked=true;for(auto it=route.rbegin();it!=route.rend();++it){
            V target=point(*it);for(int step=0;step<20&&distance(walking.player,target)>.001f;step++)walking.move(normalized(target-walking.player)*std::min(.1f,distance(walking.player,target)));
            walked&=distance(walking.player,target)<.001f;
        }
        check(walked,"real player movement can follow a route to each clearing and the guardian");
    }
    bool accessible=true;for(const auto& e:forest.enemies){bool found=false;
        for(int x=int(e.home.x)-2;x<=int(e.home.x)+2;x++)for(int y=int(e.home.y)-2;y<=int(e.home.y)+2;y++){
            if(x<2||y<2||x>=MapSize-2||y>=MapSize-2)continue;
            V p{float(x),float(y)};found|=parent[y*MapSize+x]>=0&&forest.clearEnemySegment(p,e.home,.05f);
        }accessible&=found;
    }
    check(accessible,"every dispersed encounter can be reached from the start without crossing scenery");
    bool quiet=true,bounded=true;
    for(int frame=0;frame<600;frame++){
        forest.update(1.f/60,{},false);
        quiet&=forest.hp==forest.maxHp()&&std::none_of(forest.enemies.begin(),forest.enemies.end(),[&](const Enemy& e){return forest.canSeePlayer(e);});
        for(const auto& e:forest.enemies)bounded&=distance(e.pos,e.home)<PatrolRadius+.15f&&forest.enemyPositionClear(e.pos,e.radius());
    }
    check(quiet,"standing at the forest arrival does not trigger an automatic fight");
    check(bounded,"dispersed enemies patrol their own clear territories without drifting into crowds");
}
