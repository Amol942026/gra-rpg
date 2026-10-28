#pragma once
// Five separate floors share local coordinates, never their dropped items.
inline constexpr int TowerFloorCount=5;
inline constexpr V TowerCenter{12,12},TowerArrival{12,14},MagePosition{12,10};
inline constexpr int MerchantFloor=2,MerchantObjectType=13;
inline constexpr V MerchantPosition{16,12.4f};
inline constexpr V TowerStairUp{12,16.5f},TowerStairDown{14.5f,14.2f};
inline constexpr float TowerRadius=7.2f,MageTalkRange=2.7f,TowerUseRange=2.3f;
inline constexpr int TowerBalconyFloor=TowerFloorCount-1;
// Radial terrace on the observatory's east side; coordinates match the Krita
// paintings. The collision inset leaves space for the stone balustrade.
inline V towerBalconyLocal(V p){return {(p.x-p.y)*.7071067812f,(p.x+p.y-24)*.7071067812f};}
inline V towerBalconyPoint(float outward,float across=0){return {12+(outward+across)*.7071067812f,12+(across-outward)*.7071067812f};}
inline bool towerBalconyDeck(V p){V q=towerBalconyLocal(p);return q.x>=6.1f&&q.x<=11.12f&&std::abs(q.y)<=2.12f&&q.x+std::abs(q.y)<=12.6f;}
inline constexpr int TowerStairSteps=14;
inline constexpr float TowerStairWidth=1.7f,TowerStairRun=2.6f,TowerStairRise=8.5f;
inline V towerStairBase(int direction){return direction==1?TowerStairUp:TowerStairDown;}
inline V towerStairApproach(int direction){return towerStairBase(direction)+V{direction==1?1.05f:-1.05f,direction==1?-1.f:0.f};}
inline constexpr float TowerStairPlayerRadius=.3f;
struct TowerStairFootprint {float left,right,halfWidth;};
inline TowerStairFootprint towerStairFootprint(int direction){
 // Includes the landing, post caps, coping and nosings of the cached artwork.
 return direction==1?TowerStairFootprint{-TowerStairRun-.3f,.18f,TowerStairWidth*.5f+.28f}:
                     TowerStairFootprint{-.18f,TowerStairRun+.18f,TowerStairWidth*.5f+.28f};
}
inline bool towerStairBlocked(V p,int direction){
 V q=p-towerStairBase(direction);auto bounds=towerStairFootprint(direction);
 float dx=std::max({bounds.left-q.x,0.f,q.x-bounds.right}),dy=std::max(0.f,std::abs(q.y)-bounds.halfWidth);
 // A swept player's footprint stays outside every solid face and the shaft.
 return dx*dx+dy*dy<=TowerStairPlayerRadius*TowerStairPlayerRadius;
}
struct TowerFloorInfo {const char* name;const char* theme;};
inline constexpr std::array<TowerFloorInfo,TowerFloorCount> TowerFloors={{{"Sala przybycia","KOMINEK / SCHRONIENIE"},{"Biblioteka","KRONIKI / WIEDZA"},{"Pracownia alchemiczna","ZIOŁA / MIKSTURY"},{"Sala pieczęci","RUNY / SERCE ŚWITU"},{"Obserwatorium","GWIAZDY / BALKON DOLINY ŚWITU"}}};
struct TowerFurniture {V pos;float radius;int type;};
inline const std::vector<TowerFurniture> TowerFurnishings={
 {{7.8f,8.2f},.65f,0},{{16.2f,7.8f},.65f,0},{{17.8f,10.2f},.65f,0},{{6.25f,11.3f},.65f,0},
 {{8.1f,11.3f},1.f,1},{{16.1f,11.7f},.75f,2},{{12,6.4f},1.f,3},{{13.5f,17},.65f,4}
};
inline const std::vector<TowerFurniture>& towerFurnishings(int floor){
 static const std::array<std::vector<TowerFurniture>,4> upper={{{
  {{9,7.2f},1.1f,5},{{14.8f,7.5f},1.1f,5},{{17.3f,11},1.1f,5},{{12,10},.7f,6},{{8.1f,11.3f},1.f,1},{{6.25f,11.3f},.65f,0}
 },{
  {{12,10},.95f,7},{{8.1f,10},1.f,1},{{16.3f,10.5f},.9f,1},{{9,7},.8f,8},{{15,7.8f},.8f,8},{{17.8f,10.2f},.65f,0},{MerchantPosition,.9f,MerchantObjectType}
 },{
  {{9,9},.6f,9},{{15,9},.6f,9},{{15.8f,12},.6f,9},{{12,11.3f},.7f,12},{{6.25f,11.3f},.65f,0},{{17.8f,10.2f},.65f,0}
 },{
  {{12,8.5f},1.f,10},{{16,11},1.f,11},{{8.5f,10},.7f,6},{{6.25f,11.3f},.65f,0},{{17.8f,10.2f},.65f,0}
 }}};
 return floor>=1&&floor<TowerFloorCount?upper[floor-1]:TowerFurnishings;
}
inline const char* towerObjectName(int type){
 static const char* names[]={"Runiczna kolumna","Stół Eldrina","Kula skupienia","Kominek","Źródło odnowy","Regał z księgami","Otwarta kronika","Kocioł alchemiczny","Ogród ziół","Kamień pieczęci","Luneta Eldrina","Astrolabium","Serce Świtu","Miron · handlarz"};
 return type>=0&&type<14?names[type]:"Wyposażenie wieży";
}
inline bool towerInteractive(int type){return type==MerchantObjectType||type==4||type==5||type==6||type==7||type==9||type==10||type==12;}
inline bool towerWalkable(V p,int floor=0){
 if(floor<0||floor>=TowerFloorCount||!std::isfinite(p.x)||!std::isfinite(p.y)||(floor==0&&distance(p,MagePosition)<.7f))return false;
 bool room=distance(p,TowerCenter)<=TowerRadius;V q=towerBalconyLocal(p);
 if(floor==TowerBalconyFloor){if(q.x>6.1f&&std::abs(q.y)>2.12f)room=false;if(!room&&!towerBalconyDeck(p))return false;}
 else if(!room)return false;
 if((floor<TowerFloorCount-1&&towerStairBlocked(p,1))||(floor>0&&towerStairBlocked(p,-1)))return false;
 for(const auto& furniture:towerFurnishings(floor))if(distance(p,furniture.pos)<furniture.radius)return false;
 return true;
}
