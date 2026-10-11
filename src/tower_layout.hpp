#pragma once
// Included inside ember after the world-coordinate helpers.
inline constexpr V TowerCenter{12,12},TowerArrival{12,14},MagePosition{12,10};
inline constexpr float TowerRadius=7.2f,MageTalkRange=2.7f;
struct TowerFurniture {V pos;float radius;int type;};
inline constexpr std::array<TowerFurniture,6> TowerFurnishings={{
    {{7.8f,8.2f},.65f,0},{{16.2f,7.8f},.65f,0},
    {{17.2f,15.4f},.65f,0},{{8.2f,17.f},.65f,0},
    {{8.1f,11.3f},1.f,1},{{16.1f,11.7f},.75f,2}
}};
inline bool towerWalkable(V p){
    if(!std::isfinite(p.x)||!std::isfinite(p.y)||distance(p,TowerCenter)>TowerRadius||distance(p,MagePosition)<.7f)return false;
    for(const auto& furniture:TowerFurnishings)if(distance(p,furniture.pos)<furniture.radius)return false;
    return true;
}
