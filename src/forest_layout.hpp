#pragma once

// Included inside ember after V. Layout is independent of both loot and plant
// randomness, so a new expedition keeps its roads and distinct encounters.
enum class ForestZone { Ruins,Wolves,Birches,Spiders,Heart,Grove,Trail };
inline ForestZone forestZone(V p){
    if(p.x<27&&p.y>=55)return ForestZone::Grove;
    if(p.x>=27&&p.x<48&&p.y>=58)return ForestZone::Trail;
    if(p.x>=48&&p.y>=35)return ForestZone::Heart;
    if(p.x>=27&&p.y>=35)return ForestZone::Spiders;
    if(p.x<27&&p.y>=29)return ForestZone::Birches;
    return p.x>=27?ForestZone::Wolves:ForestZone::Ruins;
}
inline const char* forestRegion(V p){
    switch(forestZone(p)){
        case ForestZone::Wolves:return "Wilcza knieja";
        case ForestZone::Birches:return "Brzozowa polana";
        case ForestZone::Spiders:return "Pajęcze ostępy";
        case ForestZone::Heart:return "Serce Kniei";
        case ForestZone::Grove:return "Zaciszny zagajnik";
        case ForestZone::Trail:return "Stary trakt";
        default:return "Ruiny Mchu";
    }
}
struct ForestTrail {V a,b;};
inline constexpr std::array<ForestTrail,16> ForestTrails={{
    {{8,19},{21,6}},{{21,6},{44,8}},{{44,8},{63,8}},
    {{8,19},{35,19}},{{35,19},{55,21}},{{55,21},{66,25}},
    {{8,19},{12,35}},{{12,35},{14,43}},{{14,43},{14,61}},
    {{35,19},{35,42}},{{35,42},{39,62}},{{14,43},{35,42}},
    {{35,42},{59,57}},{{55,21},{59,40}},{{59,40},{59,57}},{{14,61},{39,62}}
}};
struct ForestGlade {V center;float radius;const char* name;};
inline constexpr std::array<ForestGlade,6> ForestGlades={{
    {{8,19},3.6f,"Przesieka Wędrowca"},{{21,5},3.4f,"Ruiny Mchu"},
    {{14,43},4.4f,"Brzozowa polana"},{{14,61},5.f,"Zaciszny zagajnik"},
    {{59,57},4.8f,"Zapomniany krąg"},{{39,62},3.5f,"Stary trakt"}
}};
inline bool forestClearing(V p){for(const auto& glade:ForestGlades)if(distance(p,glade.center)<glade.radius)return true;return false;}
inline bool forestPath(V p){
    for(const auto& trail:ForestTrails){
        V edge=trail.b-trail.a,offset=p-trail.a;
        float t=std::clamp((offset.x*edge.x+offset.y*edge.y)/(edge.x*edge.x+edge.y*edge.y),0.f,1.f);
        V delta=p-(trail.a+edge*t);if(delta.x*delta.x+delta.y*delta.y<1.55f*1.55f)return true;
    }
    // The southern route continues from the crossroads into the deep forest.
    if(p.x>=39&&p.x<=66&&std::abs(p.y-(62+(p.x-39)*2/27))<1.55f)return true;
    return distance(p,{21,5})<3.4f||distance(p,{59,57})<3.3f;
}
struct ForestSpawn {int type;V pos;};
inline constexpr float EnemyHomeSpacing=8.f;
inline constexpr V ForestGuardian{21,5};
inline constexpr std::array<ForestSpawn,45> ForestSpawns={{
    {0,{19.5f,15.5f}},{0,{7.5f,5.5f}},{0,{27.5f,19}},{0,{19.5f,27}},
    {0,{7.5f,32.5f}},{0,{19.5f,35}},{0,{6,43}},{0,{19.5f,44}},
    {1,{61.5f,5}},{1,{67.5f,10.5f}},{1,{54.5f,12.5f}},{1,{63.5f,17.5f}},
    {1,{52.5f,25}},{1,{60.5f,28}},{1,{67.5f,32}},{1,{53.5f,33}},
    {1,{57.5f,40}},{1,{66.5f,41}},{1,{10,68}},
    {2,{50.5f,5.5f}},{2,{37.5f,7}},{2,{29.5f,7.5f}},{2,{44,12}},
    {2,{36,17.5f}},{2,{43,25.5f}},{2,{32.5f,27}},
    {4,{26.5f,49}},{4,{9,51}},{4,{18,53}},{4,{25.5f,57.5f}},{4,{6,58.5f}},
    {5,{44.5f,34}},{5,{29.5f,37}},{5,{49,43.5f}},{5,{37.5f,44}},
    {5,{45.5f,56}},{5,{36.5f,57}},
    {6,{65,66}},{6,{57,66.5f}},{6,{32,67.5f}},{6,{20.5f,67.5f}},{6,{46,68}},
    {7,{61,47.5f}},{7,{53.5f,50.5f}},{7,{68,54.5f}}
}};
