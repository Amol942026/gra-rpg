#pragma once
// Included inside Game. Rearrange common scenery after the game has chosen
// encounter and forage positions, so landscaping never changes their layout.
void refreshForestScenery(){
 const bool active=fieldLocation==WorldLocation::Forest;
 auto& scenery=active?props:forestState.props;
 const auto& inhabitants=active?enemies:forestState.enemies;
 const auto& plants=active?forage:forestState.forage;
 if(std::none_of(inhabitants.begin(),inhabitants.end(),[](const Enemy& enemy){return enemy.id==13&&enemy.type==3;}))return;
 scenery.erase(std::remove_if(scenery.begin(),scenery.end(),[](const Prop& prop){return forestGroveProp(prop.type);}),scenery.end());

 const auto place=[&](V p,int type,float size){
  const bool tree=forestCanopy(type);
  const float bankMargin=tree?5.3f:2.8f;
  const float bankLimit=MapSize-1.f-bankMargin;
  if(p.x<bankMargin||p.y<bankMargin||p.x>bankLimit||p.y>bankLimit||
     forestPath(p,tree?2.55f:1.75f)||forestClearing(p,tree?1.15f:.12f)||
     nearFieldSite(WorldLocation::Forest,p,tree?3.7f:2.5f))return false;
  const float radius=propRadius(type);
  const float occupied=radius+(tree?1.10f:.5f);
  if(location==WorldLocation::Forest&&distance(p,player)<occupied+HeroBodyRadius)return false;
  for(const auto& c:chests)if(c.location==WorldLocation::Forest&&distance(p,c.pos)<(tree?4.5f:2.6f))return false;
  for(const auto& e:inhabitants){
   const float clearance=occupied+e.radius();
   if(distance(p,e.home)<clearance||distance(p,e.pos)<clearance||
      (e.patrolGoalActive&&distance(p,e.patrolGoal)<clearance))return false;
  }
  const float forageGap=std::max(tree?2.5f:2.2f,radius+.8f);
  for(const auto& plant:plants)if(distance(p,plant.pos)<forageGap)return false;
  for(const auto& drop:ground)if(drop.location==WorldLocation::Forest&&distance(p,drop.pos)<occupied+.55f)return false;
  for(const auto& old:scenery){
   const float gap=tree&&forestCanopy(old.type)?1.95f+.44f*(size+old.size):
       occupied+propRadius(old.type)+(old.type==3||old.type>=ForestDecorationStart?.45f:0.f);
   if(!outsideCircleBounds(p-old.pos,gap)&&distance(p,old.pos)<gap)return false;
  }
  scenery.push_back({p,type,size});
  return true;
 };

 // Jitter each grove's center and fan the trunks out at unequal radii. Dominant
 // species change by region, with an occasional different species at the edge.
 static constexpr std::array<int,3> species{{0,4,5}};
 for(int x=7;x<MapSize-4;x+=8)for(int y=7;y<MapSize-4;y+=8){
  const unsigned h=hash(x+1057,y+2191);
  const V center{float(x)+(int(h%61)-30)*.1f,float(y)+(int((h>>7)%61)-30)*.1f};
  const ForestZone zone=forestZone(center);
  const bool wooded=zone==ForestZone::Wolves||zone==ForestZone::Heart||zone==ForestZone::Birches;
  if(h%100>=(wooded?80u:65u))continue;
  const int dominant=zone==ForestZone::Wolves?0:zone==ForestZone::Heart?5:
      zone==ForestZone::Birches||zone==ForestZone::Grove?4:species[(h>>14)%species.size()];
  const float bearing=float((h>>18)%360)*AnimationPi/180.f;
  const int slots=3+int((h>>9)%3);
  int trees=0;
  for(int i=0;i<slots;i++){
   const unsigned pick=hash(x*17+i,y*19+113);
   const float angle=bearing+i*2.39996323f;
   const float spread=i==0?.55f:2.05f+float((pick>>8)%19)*.095f;
   const V p=center+V{std::cos(angle),std::sin(angle)}*spread;
   const float size=trees==0?.98f+float((pick>>17)%19)*.01f:.72f+float((pick>>17)%25)*.01f;
   const int type=trees>0&&pick%5==0?species[(pick>>4)%species.size()]:dominant;
   if(place(p,type,size))++trees;
  }
  if(trees==0)continue;
  int undergrowth=0;
  for(int i=0;i<8&&undergrowth<2;i++){
   const unsigned pick=hash(x*13+i,y*11+337);
   const float angle=bearing+.7f+i*2.39996323f;
   const V p=center+V{std::cos(angle),std::sin(angle)}*(1.8f+float(pick%44)*.065f);
   const int type=undergrowth==0&&pick%3==0?1:2;
   if(place(p,type,.67f+float((pick>>8)%38)*.01f))++undergrowth;
  }
 }
 // Keep small, irregular accents in open spaces and along the shore.
 for(int x=3;x<MapSize-3;x++)for(int y=3;y<MapSize-3;y++){
  const unsigned h=hash(x+557,y+829);
  if(h%81!=0)continue;
  const V p{float(x)+float((h>>8)%9)*.1f,float(y)+float((h>>12)%9)*.1f};
  place(p,(h>>16)%3==0?1:2,.68f+float((h>>20)%30)*.01f);
 }
}
