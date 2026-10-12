#pragma once
// The second realm has its own populations and landmark routes, on a 72x72 map.
inline constexpr V DesertArrival{9,19},DesertPortal{8,17},DesertOasis{17,43},DesertBossHome{57,55};
inline constexpr int DesertBossType=11,DesertBossId=1099;
inline bool desertWater(V p,float padding=0){return distance(p,DesertOasis)<2.45f+padding;}
inline float desertSegmentDistance(V p,V a,V b){V d=b-a;float n=d.x*d.x+d.y*d.y;float t=n>0?std::clamp(((p.x-a.x)*d.x+(p.y-a.y)*d.y)/n,0.f,1.f):0;return distance(p,a+d*t);}
inline bool desertPath(V p){
 constexpr V route[]={{9,19},{21,20},{31,30},{44,33},{54,45},{57,55}};
 for(int i=0;i<5;i++)if(desertSegmentDistance(p,route[i],route[i+1])<1.35f)return true;
 return desertSegmentDistance(p,{21,20},{17,43})<1.15f||desertSegmentDistance(p,{31,30},{43,15})<1.1f;
}
inline const char* desertRegion(V p){return distance(p,DesertArrival)<8?"Brama Piasków":distance(p,DesertOasis)<9?"Oaza Bursztynu":distance(p,{43,15})<10?"Ruiny Słońca":distance(p,DesertBossHome)<12?"Tron Żywej Wydmy":"Morze Piasków";}
struct DesertSpawn {V pos;int type;};
inline constexpr std::array<DesertSpawn,27> DesertSpawns={{{{20,11},8},{{30,10},9},{{43,12},10},{{55,10},8},{{63,21},9},{{52,23},10},{{42,24},8},{{32,22},9},{{22,28},10},{{10,30},8},{{8,42},9},{{12,54},10},{{24,43},8},{{24,55},9},{{34,43},10},{{36,56},8},{{45,46},9},{{46,59},10},{{60,43},8},{{64,63},9},{{8,63},10},{{42,66},8},{{57,33},9},{{34,65},10},{{53,65},8},{{34,34},9},{{23,65},10}}};
