#pragma once
// The second realm keeps its original landmarks and extends over 102x102 cells.
inline constexpr V DesertArrival{9,19},DesertPortal{8,17},DesertOasis{17,43},DesertBossHome{57,55};
inline constexpr int DesertBossType=11,DesertBossId=1099;
inline constexpr int FountainCount=2;
inline constexpr std::array<V,FountainCount> DesertFountains={{{25,22},{50,37}}};
inline constexpr float FountainRadius=.9f,FountainUseRange=2.5f,FountainDrainDuration=1.f;
inline bool nearDesertFountain(V p,float padding=0){for(V f:DesertFountains)if(distance(p,f)<FountainRadius+padding)return true;return false;}
inline const char* fountainName(int id){return id==0?"Źródło Błękitnej Pieczęci":"Źródło Szkarłatnego Świtu";}
inline bool desertWater(V p,float padding=0){return distance(p,DesertOasis)<2.45f+padding;}
inline float desertSegmentDistance(V p,V a,V b){V d=b-a;float n=d.x*d.x+d.y*d.y;float t=n>0?std::clamp(((p.x-a.x)*d.x+(p.y-a.y)*d.y)/n,0.f,1.f):0;return distance(p,a+d*t);}
inline constexpr float SandPrisonInterval=10.f,SandPrisonDuration=3.f,SandCastDuration=.7f;
struct DesertRoad {std::vector<V> points;float width;};
inline const std::array<DesertRoad,6>& desertRoads(){
 static const auto roads=[](){
  std::array<DesertRoad,6> result;
  const std::array<std::vector<V>,6> nodes={{{DesertArrival,{21,20},{31,30},{44,33},{54,45},DesertBossHome},
   {{21,20},{20,29},{17,37},DesertOasis},{{31,30},{35,23},{43,15}},
   {{43,15},{43,19},{54,20},{64,16},{79,16},{94,28},{88,49},{92,68},{93,91}},
   {{17,37},{10,42},{11,54},{21,63},{22,81},{43,91},{65,85},{80,73},{93,91}},
   {DesertBossHome,{69,65},{80,73},{88,49}}}};
  for(int route=0;route<int(result.size());route++){
   auto& road=result[route];road.width=route==0?1.f:.84f;const auto& n=nodes[route];
   for(size_t i=0;i+1<n.size();i++)for(int step=0;step<32;step++){
    V a=n[i?i-1:0],b=n[i],c=n[i+1],d=n[std::min(i+2,n.size()-1)];float t=step/32.f;
    auto value=[&](float a,float b,float c,float d){return .5f*(2*b+(-a+c)*t+(2*a-5*b+4*c-d)*t*t+(-a+3*b-3*c+d)*t*t*t);};
    road.points.push_back({value(a.x,b.x,c.x,d.x),value(a.y,b.y,c.y,d.y)});
   }
   road.points.push_back(n.back());
  }return result;
 }();return roads;
}
inline bool desertPath(V p){
 for(const auto& road:desertRoads())for(size_t i=1;i<road.points.size();i++){
  V a=road.points[i-1],b=road.points[i];float width=1.95f*road.width;
  if(p.x<std::min(a.x,b.x)-width||p.x>std::max(a.x,b.x)+width||p.y<std::min(a.y,b.y)-width||p.y>std::max(a.y,b.y)+width)continue;
  V delta=b-a;float norm=delta.x*delta.x+delta.y*delta.y;
  float t=norm>0?std::clamp(((p.x-a.x)*delta.x+(p.y-a.y)*delta.y)/norm,0.f,1.f):0;
  V offset=p-a-delta*t;if(offset.x*offset.x+offset.y*offset.y<width*width)return true;
 }return false;
}
inline const char* desertRegion(V p){return p.x>=72?(p.y<42?"Wschodnie Wydmy":"Wzgórza Bursztynu"):p.y>=72?"Południowe Pustkowia":distance(p,DesertArrival)<8?"Brama Piasków":distance(p,DesertOasis)<9?"Oaza Bursztynu":distance(p,{43,15})<10?"Ruiny Słońca":distance(p,DesertBossHome)<12?"Tron Żywej Wydmy":"Morze Piasków";}
struct DesertSpawn {V pos;int type;};
inline constexpr std::array<DesertSpawn,54> DesertSpawns={{{{20,11},8},{{30,10},9},{{43,12},10},{{55,10},8},{{63,21},9},{{52,23},10},{{42,24},8},{{32,22},9},{{22,28},10},{{10,30},8},{{8,42},9},{{12,54},10},{{24,43},8},{{24,55},9},{{34,43},10},{{36,56},8},{{45,46},9},{{46,59},10},{{60,43},8},{{64,63},9},{{8,63},10},{{42,66},8},{{57,33},9},{{34,65},10},{{53,65},8},{{34,34},9},{{23,65},10},
    {{79,8},8},{{79,23},9},{{79,38},10},{{79,53},8},{{79,68},9},
    {{79,83},10},{{79,97},8},{{89,8},9},{{89,23},10},{{89,38},8},
    {{89,53},9},{{89,68},10},{{89,83},8},{{89,97},9},{{97,23},10},
    {{97,38},8},{{97,53},9},{{97,68},10},{{97,83},8},{{97,97},9},
    {{12,80},10},{{28,80},8},{{44,80},9},{{60,80},10},{{28,96},8},
    {{44,96},9},{{60,96},10}
}};
