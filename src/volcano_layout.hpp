#pragma once
// Third realm; coordinates and IDs are independent of existing saves.
inline constexpr V VolcanoArrival{8,62},VolcanoPortal{8,65},VolcanoBossHome{60,15};
inline constexpr int VolcanoFirstType=12,VolcanoBossType=18,VolcanoBossId=2099;
inline constexpr int VolcanoSceneryRevision=5;
// Visual channels continue beyond the playable field, with no exposed end cap.
inline constexpr float VolcanoRiverStart=-16,VolcanoRiverEnd=MapSize+16,VolcanoRiverDepth=38;
inline constexpr int VolcanoRiverSegments=int((VolcanoRiverEnd-VolcanoRiverStart)*2);
inline constexpr std::array<float,7> VolcanoStrides={{34,40,28,35,32,36,58}};
inline constexpr std::array<V,2> VolcanoFountains={{{17,41},{55,28}}};
inline constexpr std::array<V,8> VolcanoBridges={{{27,48},{27,22},{50,34},{50,58},{27,80},{50,80},{27,94},{50,94}}};
inline constexpr std::array<int,8> VolcanoBridgeRivers={{0,0,1,1,0,1,0,1}};
inline bool nearVolcanoFountain(V p,float padding=0){for(V f:VolcanoFountains)if(distance(p,f)<FountainRadius+padding)return true;return false;}
inline float lavaCenter(int river,float y){return river==0?27+std::sin(y*.12f)*3.2f:50+std::sin(y*.14f+1)*3.f;}
inline bool volcanoBridge(V p,float padding=0){
 for(size_t i=0;i<VolcanoBridges.size();i++){const auto bridge=VolcanoBridges[i];int river=VolcanoBridgeRivers[i];
  if(std::abs(p.y-bridge.y)<=2.8f-padding&&std::abs(p.x-lavaCenter(river,p.y))<=5.3f)return true;
 }return false;
}
inline bool volcanoLava(V p,float padding=0){
 if(p.y<1-padding||p.y>MapSize-1+padding||distance(p,VolcanoBossHome)<7.5f-padding||volcanoBridge(p,padding))return false;
 for(int river=0;river<2;river++)if(std::abs(p.x-lavaCenter(river,p.y))<1.6f+padding)return true;
 return false;
}
inline const std::array<std::array<V,6>,9> VolcanoRoads={{
 std::array<V,6>{{VolcanoArrival,{16,54},{lavaCenter(0,48),48},{38,43},{lavaCenter(1,34),34},VolcanoBossHome}},
 std::array<V,6>{{{16,54},{17,41},{17,23},{lavaCenter(0,22),22},{37,26},{43,38}}},
 std::array<V,6>{{{38,43},{39,58},{55.5f,58},{55,52},{61,42},{60,27}}},
 std::array<V,6>{{{60,27},{70,30},{80,31},{92,26},{94,42},{91,56}}},
 std::array<V,6>{{{61,61},{75,64},{91,56},{91,76},{92,91},{78,94}}},
 std::array<V,6>{{{16,54},{14,72},{18,80},{lavaCenter(0,80),80},{39,80},{lavaCenter(1,80),80}}},
 std::array<V,6>{{{39,58},{39,80},{39,94},{lavaCenter(1,94),94},{66,94},{78,94}}},
 std::array<V,6>{{{14,72},{12,90},{lavaCenter(0,94),94},{39,94},{39,80},{lavaCenter(1,80),80}}},
 std::array<V,6>{{{39,80},{lavaCenter(1,80),80},{62,80},{76,80},{91,76},{92,91}}}
}};
inline bool volcanoPath(V p){for(const auto& road:VolcanoRoads)for(int i=1;i<6;i++)if(desertSegmentDistance(p,road[i-1],road[i])<1.4f)return true;return false;}
inline const char* volcanoRegion(V p){
 if(p.x>=72)return p.y<54?"Wschodnie Kuźnie":"Bazaltowe Wzgórza";
 if(p.y>=72)return p.x<25?"Południowe Pogorzelisko":"Przełęcz Żaru";
 return distance(p,VolcanoArrival)<8?"Brama Popiołu":distance(p,VolcanoBossHome)<11?"Tron Serca Kaldery":p.x<24?"Popielne Pogorzelisko":p.x>51?"Obsydianowe Kuźnie":"Rozłam Żaru";
}
struct VolcanoSpawn {V pos;int type;};
inline constexpr std::array<VolcanoSpawn,60> VolcanoSpawns={{{{16,57},12},{{8,50},13},{{17,48},14},{{11,37},15},{{18,30},16},{{10,22},17},
 {{17,16},12},{{35,20},13},{{37,31},14},{{35,39},15},{{40,48},16},{{35,57},17},
 {{18,66},13},{{38,65},12},{{61,61},14},{{62,50},15},{{56,46},16},{{63,36},17},
 {{57,27},12},{{41,14},13},{{63,24},14},{{46,22},15},{{56,9},16},{{65,9},17},
 {{10,12},14},{{36,9},15},{{44,63},16},{{20,9},17},{{63,68},12},{{57,64},13},
    {{78,9},12},{{78,24},13},{{78,39},14},{{78,54},15},{{78,69},16},
    {{78,84},17},{{78,97},12},{{88,9},13},{{88,24},14},{{88,39},15},
    {{88,54},16},{{88,69},17},{{88,84},12},{{88,97},13},{{97,24},14},
    {{97,39},15},{{97,54},16},{{97,69},17},{{97,84},12},{{97,97},13},
    {{10,81},14},{{20,81},15},{{37,81},16},{{62,81},17},{{70,81},12},
    {{10,95},13},{{20,95},14},{{37,95},15},{{62,95},16},{{70,95},17}
}};
