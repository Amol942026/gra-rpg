#pragma once
// Five separate floors share local coordinates, never their dropped items.
inline constexpr int TowerFloorCount=5;
inline constexpr V TowerCenter{12,12},TowerArrival{12,14},MagePosition{12,10};
inline constexpr V TowerStairUp{8.5f,14.5f},TowerStairDown{15.8f,14.5f};
inline constexpr float TowerRadius=7.2f,MageTalkRange=2.7f,TowerUseRange=2.3f;
inline constexpr int TowerStairSteps=14;
inline constexpr float TowerStairWidth=1.7f,TowerStairRun=2.6f,TowerStairRise=8.5f;
inline V towerStairBase(int direction){return direction==1?TowerStairUp:TowerStairDown;}
inline bool towerStairBlocked(V p,int direction){
 V offset=p-towerStairBase(direction);float along=offset.x*(direction==1?-1.f:1.f),across=std::abs(offset.y);
 // The entrance remains walkable; the raised flight and open shaft are solid boundaries.
 return (along>.18f&&along<TowerStairRun+.25f&&across<TowerStairWidth*.5f+.2f)||
        (along>-.25f&&along<TowerStairRun+.25f&&across>TowerStairWidth*.5f-.1f&&across<TowerStairWidth*.5f+.24f);
}
struct TowerFloorInfo {const char* name;const char* theme;};
inline constexpr std::array<TowerFloorInfo,TowerFloorCount> TowerFloors={{{"Sala przybycia","KOMINEK / SCHRONIENIE"},{"Biblioteka","KRONIKI / WIEDZA"},{"Pracownia alchemiczna","ZIOŁA / MIKSTURY"},{"Sala pieczęci","RUNY / SERCE ŚWITU"},{"Obserwatorium","GWIAZDY / ASTROLABIUM"}}};
struct TowerFurniture {V pos;float radius;int type;};
inline const std::vector<TowerFurniture> TowerFurnishings={
 {{7.8f,8.2f},.65f,0},{{16.2f,7.8f},.65f,0},{{17.8f,10.2f},.65f,0},{{6.25f,11.3f},.65f,0},
 {{8.1f,11.3f},1.f,1},{{16.1f,11.7f},.75f,2},{{12,6.4f},1.f,3},{{13.5f,17},.65f,4}
};
inline const std::vector<TowerFurniture>& towerFurnishings(int floor){
 static const std::array<std::vector<TowerFurniture>,4> upper={{{
  {{9,7.2f},1.1f,5},{{14.8f,7.5f},1.1f,5},{{17.3f,11},1.1f,5},{{12,10},.7f,6},{{8.1f,11.3f},1.f,1},{{6.25f,11.3f},.65f,0}
 },{
  {{12,10},.95f,7},{{8.1f,10},1.f,1},{{16.3f,10.5f},.9f,1},{{9,7},.8f,8},{{15,7.8f},.8f,8},{{17.8f,10.2f},.65f,0}
 },{
  {{9,9},.6f,9},{{15,9},.6f,9},{{15.8f,12},.6f,9},{{12,11.3f},.7f,12},{{6.25f,11.3f},.65f,0},{{17.8f,10.2f},.65f,0}
 },{
  {{12,8.5f},1.f,10},{{16,11},1.f,11},{{8.5f,10},.7f,6},{{6.25f,11.3f},.65f,0},{{17.8f,10.2f},.65f,0}
 }}};
 return floor>=1&&floor<TowerFloorCount?upper[floor-1]:TowerFurnishings;
}
inline const char* towerObjectName(int type){
 static const char* names[]={"Runiczna kolumna","Stół Eldrina","Kula skupienia","Kominek","Źródło odnowy","Regał z księgami","Otwarta kronika","Kocioł alchemiczny","Ogród ziół","Kamień pieczęci","Luneta Eldrina","Astrolabium","Serce Świtu"};
 return type>=0&&type<13?names[type]:"Wyposażenie wieży";
}
inline bool towerInteractive(int type){return type==4||type==5||type==6||type==7||type==9||type==10||type==12;}
inline bool towerWalkable(V p,int floor=0){
 if(floor<0||floor>=TowerFloorCount||!std::isfinite(p.x)||!std::isfinite(p.y)||distance(p,TowerCenter)>TowerRadius||(floor==0&&distance(p,MagePosition)<.7f))return false;
 if((floor<TowerFloorCount-1&&towerStairBlocked(p,1))||(floor>0&&towerStairBlocked(p,-1)))return false;
 for(const auto& furniture:towerFurnishings(floor))if(distance(p,furniture.pos)<furniture.radius)return false;
 return true;
}
