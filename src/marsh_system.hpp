#pragma once
void spaceMarshHuts(std::vector<Prop>& scenery,const std::vector<Enemy>& residents)const{
 for(V village:{marsh::Ferry,marsh::Village,marsh::Mill}){
  const V oldPlot=village+V{1,-5};
  auto hut=std::find_if(scenery.begin(),scenery.end(),[&](const Prop& p){return p.type==MarshFishingHut&&distance(p.pos,oldPlot)<.01f;});
  if(hut==scenery.end())continue;
  for(V offset:{V{2,-2.5f},V{2,-3.5f},V{1,-4},V{3,-3}}){
   const V plot=oldPlot+offset;
   if(marsh::shore(plot)<-3||marsh::road(plot)<1.85f||nearTreasure(plot,WorldLocation::Swamp,2))continue;
   const auto occupied=[&](const Prop& p){return &p!=&*hut&&distance(plot,p.pos)<propRadius(MarshFishingHut)+propRadius(p.type)+.3f;};
   if(std::any_of(scenery.begin(),scenery.end(),occupied))continue;
   if(std::any_of(residents.begin(),residents.end(),[&](const Enemy& e){return distance(e.pos,plot)<propRadius(MarshFishingHut)+e.radius();}))continue;
   hut->pos=plot;break;
  }
 }
}
void addMarshWorksites(std::vector<Prop>& scenery,const std::vector<Enemy>& residents,V visitor)const{
 static constexpr Prop plots[]={
  {{41,105.5f},MarshFishTrap,.94f},{{78.5f,82.5f},MarshFishTrap,1.02f},{{34.5f,52},MarshFishTrap,.89f},
  {{66.5f,38.5f},MarshFishTrap,1.06f},{{114,89},MarshFishTrap,.95f},{{18,122},MarshFishTrap,.91f},
  {{70.5f,123},MarshPeatCart,1.04f},{{106,105.5f},MarshPeatCart,.96f},{{67,74},MarshPeatCart,1},
  {{73.5f,27.5f},MarshPeatCart,.93f},{{35.5f,98.5f},MarshPeatCart,.98f}
 };
 for(auto plot:plots){
  const float radius=propRadius(plot.type);
  const auto clear=[&](V pos){
   const float shore=marsh::shore(pos);
   if(plot.type==MarshFishTrap?(shore<-.8f||shore>1.5f):shore<2.6f)return false;
   if(marsh::road(pos)<2.7f||nearFieldSite(WorldLocation::Swamp,pos,2.8f)||
      nearTreasure(pos,WorldLocation::Swamp,1.7f)||distance(pos,visitor)<radius+.6f)return false;
   for(const auto& p:scenery)if(distance(pos,p.pos)<radius+propRadius(p.type)+.45f)return false;
   for(const auto& e:residents)if(distance(pos,e.pos)<radius+e.radius()+.35f)return false;
   return true;
  };
  // A returning traveller or patrolling enemy may occupy the original plot in an older save.
  const V anchor=plot.pos;
  for(int ring=0;ring<=5;ring++){
   bool placed=false;
   for(int i=0;i<(ring?12:1);i++){
    const float angle=i*.52359878f;
    plot.pos=anchor+V{std::cos(angle),std::sin(angle)}*(ring*.8f);
    if(clear(plot.pos)){scenery.push_back(plot);placed=true;break;}
   }
   if(placed)break;
  }
 }
}
void buildMarsh(){
 props.clear();enemies.clear();forage.clear();
 const auto encounter=[&](V p,int type){
  const auto& species=enemyInfo(type);
  const auto clear=[&](V point){
   if(point.x<MapWalkMin+species.radius||point.y<MapWalkMin+species.radius||point.x>MapWalkMax-species.radius||point.y>MapWalkMax-species.radius||marsh::deep(point,species.radius))return false;
   if(nearTreasure(point,WorldLocation::Swamp,species.radius+ChestRadius+.1f))return false;
   for(const auto& prop:props)if(distance(point,prop.pos)<species.radius+propRadius(prop.type)+.1f)return false;
   for(const auto& e:enemies)if(distance(point,e.pos)<species.radius+e.radius()+.2f)return false;
   return true;
  };
  if(!clear(p)){
   const V origin=p;bool found=false;
   for(int ring=1;ring<=4&&!found;ring++)for(int i=0;i<8;i++){
    float angle=i*.78539816f;V candidate=origin+V{std::cos(angle),std::sin(angle)}*float(ring);
    if(clear(candidate)){p=candidate;found=true;break;}
   }
   if(!found)return;
  }
  const float hp=species.health;
  enemies.push_back({4000+int(enemies.size()),type,p,p,hp,hp});
  enemies.back().patrolWait=.65f+float(enemies.size()%7)*.18f;
 };
 props.push_back({marsh::Portal,13,1});
 const auto place=[&](V p,int design,float size){
  const float bank=marsh::shore(p);
  const bool waterside=design==2||design==4||design==6;
  if(bank<(waterside?-3.f:-.55f)||nearTreasure(p,WorldLocation::Swamp)||distance(p,marsh::Arrival)<4||distance(p,marsh::Guardian)<3)return;
  for(const auto& prop:props)if(distance(p,prop.pos)<2.7f)return;
  props.push_back({p,MarshPropStart+design,size});
 };
 place(marsh::Guardian+V{.5f,-4},3,1.30f);
 place(marsh::Isle+V{3,1},7,1.17f);
 place(marsh::Ferry+V{-1,3},4,1.10f);place(marsh::Ferry+V{3,1},5,1);
 place(marsh::Mill+V{2,-2},6,1.34f);place(marsh::Mill+V{-5,3},4,.95f);
 place(marsh::Village+V{2,5},5,1.08f);place(marsh::Village+V{-5,3},4,1.02f);
 place(marsh::Peat+V{2,-3},7,.96f);place(marsh::Reedbed+V{3,-3},5,.94f);
 place(marsh::Arrival+V{5,1},7,.85f);
 place(marsh::Arrival+V{1,-5},0,1.28f);place(marsh::Arrival+V{-2,-4},1,1.02f);
 for(V center:{marsh::Village,marsh::Ferry,marsh::Mill}){
  place(center+V{-3,-2},2,1.16f);place(center+V{4,2},2,.98f);place(center+V{1,-5},2,1.03f);
 }
 for(int y=4;y<MapSize-3;y+=4)for(int x=4;x<MapSize-3;x+=4){
  unsigned seed=hash(x+347,y+491);V p{x+.02f*(seed%139),y+.02f*((seed/139)%137)};
  float bank=marsh::shore(p),road=marsh::road(p);
  if(road<2.1f||nearFieldSite(WorldLocation::Swamp,p,2.8f))continue;
  if(bank<2.5f||seed%7==0)place(p,1,.61f+.07f*(seed%6));
  else if(seed%5==0)place(p,0,1.04f+.08f*(seed%5));
 }
 spaceMarshHuts(props,enemies);
 for(int y=9;y<MapSize-5;y+=13)for(int x=8;x<MapSize-5;x+=13){
  unsigned seed=hash(x,y+835);V p{x+float(seed%5),y+float((seed/7)%5)};
  if(distance(p,marsh::Arrival)<9||distance(p,marsh::Guardian)<7||marsh::deep(p,1))continue;
  if(nearTreasure(p,WorldLocation::Swamp,2)||std::any_of(props.begin(),props.end(),[&](const Prop& prop){return distance(prop.pos,p)<2.1f;}))continue;
  encounter(p,PeatToad+int(seed%3));
 }
 encounter(marsh::Ferry+V{3,-2},PeatToad);encounter(marsh::Village+V{2,-1},MarshLeech);
 encounter(marsh::Isle+V{-2,1},MarshLight);encounter(marsh::Guardian,BellKeeper);
 addMarshWorksites(props,enemies,marsh::Arrival);
 marshCreated=true;
}
bool canTravelMarsh()const{return mageReachable()&&recoveredDawnPart(2);}
bool travelToMarsh(){
 if(!canTravelMarsh()||!activateField(WorldLocation::Swamp))return false;
 resetFieldArrival(marsh::Arrival);sound(Cue::TeleportArrival,player);
 say("Mokradła Cichych Dzwonów. Grobla prowadzi przez zatopioną osadę do dzwonnicy.");return true;
}
bool useMarshPortal(){
 if(!inMarsh()||dead()||allocationRequired||storyBlocked()||rolling()||knockedBack()||distance(player,marsh::Portal)>2.8f)return false;
 enterMageTower();sound(Cue::TeleportArrival,player);dawnQuest=DawnQuest::AtTower;return true;
}
