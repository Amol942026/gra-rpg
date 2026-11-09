#pragma once

// Included inside ember after V. Layout is independent of both loot and plant
// randomness, so a new expedition keeps its roads and distinct encounters.
inline constexpr V ForestArrival{MapWalkMin,MapWalkMax};
inline constexpr V PreviousForestArrival{MapWalkMin,PreviousMapSize-2.6f};
enum class ForestZone { Ruins,Wolves,Birches,Spiders,Heart,Grove,Trail };
inline ForestZone forestZone(V p){
    if(p.x>=PreviousMapSize)return p.y<48?ForestZone::Ruins:p.y<96?ForestZone::Wolves:ForestZone::Heart;
    if(p.y>=PreviousMapSize)return p.x<51?ForestZone::Grove:ForestZone::Trail;
    if(p.x<27&&p.y>=55)return ForestZone::Grove;
    if(p.x>=27&&p.x<48&&p.y>=58)return ForestZone::Trail;
    if(p.x>=48&&p.y>=35)return ForestZone::Heart;
    if(p.x>=27&&p.y>=35)return ForestZone::Spiders;
    if(p.x<27&&p.y>=29)return ForestZone::Birches;
    return p.x>=27?ForestZone::Wolves:ForestZone::Ruins;
}
inline const char* forestRegion(V p){
    if(distance(p,ForestArrival)<4.2f)return "Przesieka Wędrowca";
    if(p.x>=PreviousMapSize)return p.y<48?"Ruiny Kamiennego Brodu":p.y<96?"Sosnowe Grzbiety":"Prastara Knieja";
    if(p.y>=PreviousMapSize)return p.x<51?"Wąwóz Paproci":"Wzgórza Starego Traktu";
    if(p.x>=72)return p.y<30?"Wschodnie Ruiny":p.y<72?"Głęboka Knieja":"Korzenne Wzgórza";
    if(p.y>=72)return p.x<35?"Południowy Zagajnik":"Korzenne Rozdroża";
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
inline constexpr std::array<ForestTrail,56> ForestTrails={{
    {PreviousForestArrival,{10,92}},
    {{8,19},{21,6}},{{21,6},{44,8}},{{44,8},{63,8}},
    {{8,19},{35,19}},{{35,19},{55,21}},{{55,21},{66,25}},
    {{8,19},{12,35}},{{12,35},{14,43}},{{14,43},{14,61}},
    {{35,19},{35,42}},{{35,42},{39,62}},{{14,43},{35,42}},
    {{35,42},{59,57}},{{55,21},{59,40}},{{59,40},{59,57}},{{14,61},{39,62}},
    {{10,92},{18,82}},{{18,82},{14,61}},{{18,82},{39,84}},{{39,84},{59,84}},
    {{59,84},{80,84}},{{80,84},{94,92}},{{39,62},{39,84}},{{59,57},{80,55}},
    {{80,55},{94,55}},{{94,55},{94,32}},{{94,32},{83,14}},{{63,8},{83,14}},
    {{83,14},{79,34}},{{79,34},{80,55}},{{80,55},{80,84}},{{59,84},{66,64}},
    {{94,55},{94,76}},{{94,76},{94,92}},
    {ForestArrival,{11,133}},{{11,133},{16,117}},{{16,117},PreviousForestArrival},
    {{11,133},{37,129}},{{37,129},{62,135}},{{62,135},{88,121}},{{88,121},{112,129}},{{112,129},{133,135}},
    {{39,84},{43,106}},{{43,106},{37,129}},{{94,92},{88,121}},{{94,92},{115,103}},{{115,103},{112,129}},
    {{83,14},{107,15}},{{107,15},{131,11}},{{131,11},{132,39}},{{132,39},{115,60}},{{115,60},{94,55}},
    {{115,60},{128,79}},{{128,79},{115,103}},{{128,79},{137,108}}
}};
struct ForestGlade {V center;float radius;const char* name;};
inline constexpr std::array<ForestGlade,19> ForestGlades={{
    {ForestArrival,3.6f,"Przesieka Wędrowca"},
    {{8,19},3.6f,"Stara przesieka"},{{21,5},3.4f,"Ruiny Mchu"},
    {{14,43},4.4f,"Brzozowa polana"},{{14,61},5.f,"Zaciszny zagajnik"},
    {{59,57},4.8f,"Zapomniany krąg"},{{39,62},3.5f,"Stary trakt"},
    {{18,82},4.2f,"Południowy Zagajnik"},{{39,84},4.f,"Korzenne Rozdroża"},
    {{83,14},4.f,"Wschodnie Ruiny"},{{80,55},4.5f,"Głęboka Knieja"},{{94,92},4.f,"Korzenne Wzgórza"},
    {PreviousForestArrival,3.6f,"Stara przesieka"},{{107,15},4.f,"Ruiny Kamiennego Brodu"},
    {{115,60},4.6f,"Sosnowe Grzbiety"},{{128,79},4.f,"Sosnowa Polana"},{{115,103},4.8f,"Prastara Knieja"},
    {{37,129},4.6f,"Wąwóz Paproci"},{{88,121},4.2f,"Wzgórza Starego Traktu"}
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
inline constexpr std::array<ForestSpawn,180> ForestSpawns={{
    {0,{19.5f,15.5f}},{0,{7.5f,5.5f}},{0,{27.5f,19}},{0,{19.5f,27}},
    {0,{7.5f,32.5f}},{0,{19.5f,35}},{0,{6,43}},{0,{19.5f,44}},
    {1,{61.5f,5}},{1,{67.5f,10.5f}},{1,{54.5f,12.5f}},{1,{63.5f,17.5f}},
    {1,{52.5f,25}},{1,{60.5f,28}},{1,{67.5f,32}},{1,{53.5f,33}},
    {1,{57.5f,40}},{1,{66.5f,41}},{1,{12,68}},
    {2,{50.5f,5.5f}},{2,{37.5f,7}},{2,{29.5f,7.5f}},{2,{44,12}},
    {2,{36,17.5f}},{2,{43,25.5f}},{2,{32.5f,27}},
    {4,{26.5f,49}},{4,{9,51}},{4,{18,53}},{4,{25.5f,57.5f}},{4,{6,58.5f}},
    {5,{44.5f,34}},{5,{29.5f,37}},{5,{49,43.5f}},{5,{37.5f,44}},
    {5,{45.5f,56}},{5,{36.5f,57}},
    {6,{65,66}},{6,{57,66.5f}},{6,{32,67.5f}},{6,{20.5f,67.5f}},{6,{46,68}},
    {7,{61,47.5f}},{7,{53.5f,50.5f}},{7,{68,54.5f}},
    {0,{78,8}},{2,{78,18}},{2,{78,28}},{7,{78,38}},{7,{78,48}},
    {7,{78,58}},{5,{78,68}},{1,{78,78}},{6,{78,88}},{0,{78,97}},
    {2,{88,8}},{2,{88,18}},{2,{88,28}},{7,{88,38}},{1,{88,48}},
    {7,{88,58}},{5,{88,68}},{5,{88,78}},{0,{88,88}},{6,{88,97}},
    {2,{97,18}},{1,{97,28}},{7,{97,38}},{7,{97,58}},{5,{97,68}},
    {5,{97,78}},{6,{97,88}},{0,{8,78}},{1,{18,78}},{4,{28,78}},
    {5,{38,78}},{5,{48,78}},{5,{58,78}},{5,{68,78}},{4,{8,88}},
    {1,{18,88}},{0,{28,88}},{6,{38,88}},{6,{48,88}},{6,{58,88}},
    {6,{68,88}},{4,{18,97}},{1,{38,97}},{6,{48,97}},{6,{68,97}},
    {2,{107.2f,9.3f}},{2,{115.2f,10.7f}},{2,{125.5f,8.7f}},{7,{135.4f,10.4f}},{1,{108.3f,20.7f}},
    {2,{116.6f,20.5f}},{5,{125.7f,20}},{7,{133.8f,20}},{2,{107.7f,29.5f}},{6,{117.5f,29}},
    {0,{126.3f,31.2f}},{7,{135.8f,29.5f}},{4,{106.9f,41.8f}},{5,{117.7f,42}},{1,{126.1f,40.6f}},
    {0,{136.1f,39.2f}},{0,{106.3f,49.9f}},{0,{115.2f,50}},{5,{127,49.5f}},{2,{136,50.2f}},
    {1,{106.6f,59.9f}},{0,{117.1f,61}},{6,{126.2f,63}},{0,{134.1f,61.1f}},{4,{105.9f,70.6f}},
    {6,{117.1f,73.7f}},{5,{125,71.9f}},{6,{134.7f,71.9f}},{4,{107.9f,83.6f}},{4,{116.3f,82}},
    {6,{124.4f,81.1f}},{1,{136.7f,82.2f}},{5,{106.2f,92.8f}},{5,{118,94.4f}},{1,{125.5f,91.4f}},
    {0,{136,91.6f}},{1,{107.4f,105.1f}},{4,{115.7f,102.2f}},{7,{126.6f,104.6f}},{4,{134.9f,103.9f}},
    {1,{108.6f,115}},{7,{116.5f,113.1f}},{0,{124.6f,113.1f}},{7,{136.4f,112.1f}},{2,{106.1f,124}},
    {0,{116.8f,124.3f}},{1,{124.9f,125.1f}},{2,{135.3f,125.2f}},{4,{112.1f,133.9f}},{6,{130.2f,133.8f}},
    {1,{7.4f,106.9f}},{0,{18.7f,107.3f}},{7,{29.3f,106.3f}},{5,{38.2f,107.2f}},{0,{47.1f,108.9f}},
    {6,{57.2f,107.4f}},{6,{65.3f,107.2f}},{6,{77.5f,105.6f}},{1,{85.8f,108.2f}},{0,{95.9f,106.2f}},
    {1,{9.7f,116.9f}},{2,{17.8f,116}},{1,{27.8f,116.9f}},{7,{37.1f,115.2f}},{7,{48.8f,117.7f}},
    {4,{57.8f,116.2f}},{4,{66.5f,115.2f}},{6,{77.2f,115.6f}},{4,{85.3f,116.3f}},{6,{94.8f,117.6f}},
    {5,{9.2f,126.5f}},{2,{19,126.5f}},{7,{28.9f,124.9f}},{2,{38.8f,126.1f}},{4,{48,126.7f}},
    {4,{58.2f,125.7f}},{5,{68.2f,124.8f}},{7,{77.5f,126.2f}},{2,{87.2f,124.2f}},{6,{97.3f,125.3f}},
    {5,{9.4f,134.7f}},{5,{19.4f,136.5f}},{0,{28.4f,136.8f}},{7,{37.8f,134.6f}},{2,{46.9f,135.2f}},
    {5,{58.2f,133.8f}},{4,{66.3f,136.7f}},{5,{76.4f,135.1f}},{6,{88,135.1f}},{1,{97.4f,134.5f}}
}};
