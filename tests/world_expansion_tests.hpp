#pragma once
void worldExpansionTests(){
 check(std::abs(float(MapSize*MapSize)/(PreviousMapSize*PreviousMapSize)-2)<.01f,"each field has double its old area within one percent on the tile grid");
 check(terrain_height::Count==150&&VolcanoBridges.size()==12,"expansion adds 62 hills and knolls and four new lava bridges");
 for(auto realm:{WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano}){
  Game g;g.activateField(realm);int expected=realm==WorldLocation::Forest?181:realm==WorldLocation::Desert?109:121;
  check(int(g.enemies.size())==expected+RegionalEncountersPerRealm,"each realm retains its expanded population and receives its new species");
  int guardian=realm==WorldLocation::Forest?3:realm==WorldLocation::Desert?11:18,guardians=0,addedEnemies=0,addedProps=0;
  std::array<int,10> creatures{},objects{};
  auto sector=[](V p){
   return p.x>=PreviousMapSize?std::min(3,int(p.y/36))*2+std::min(1,int((p.x-PreviousMapSize)/21)):
          p.y>=PreviousMapSize?8+std::min(1,int(p.x/51)):-1;
  };
  for(const auto& e:g.enemies){guardians+=e.type==guardian;int cell=sector(e.home);if(cell>=0&&!regionalEnemy(e.type)){creatures[cell]++;addedEnemies++;}}
  for(const auto& prop:g.props){int cell=sector(prop.pos);if(cell>=0){objects[cell]++;addedProps++;}}
  check(guardians==1&&addedEnemies==(realm==WorldLocation::Forest?90:realm==WorldLocation::Desert?54:60),"all added enemies occupy the new territory, without copying bosses");
  check(std::all_of(creatures.begin(),creatures.end(),[](int n){return n>0;})&&std::all_of(objects.begin(),objects.end(),[](int n){return n>0;}),"all ten extension sectors contain both scenery and encounters");
  check(addedProps>(realm==WorldLocation::Volcano?90:180),"new territory contains substantial authored scenery coverage");
  std::array<bool,MapSize*MapSize> reached{};std::vector<V> queue;
  V arrival=realm==WorldLocation::Forest?ForestArrival:realm==WorldLocation::Desert?DesertArrival:VolcanoArrival;
  V start{std::round(arrival.x),std::round(arrival.y)};queue.push_back(start);reached[int(start.y)*MapSize+int(start.x)]=true;
  for(size_t i=0;i<queue.size();i++)for(V delta:{V{1,0},V{-1,0},V{0,1},V{0,-1}}){
   V p=queue[i]+delta;if(p.x<2||p.y<2||p.x>=MapSize-2||p.y>=MapSize-2||!g.walkable(p))continue;
   int cell=int(p.y)*MapSize+int(p.x);if(reached[cell]||!g.clearEnemySegment(queue[i],p,.05f))continue;reached[cell]=true;queue.push_back(p);
  }
  for(const auto& e:g.enemies){
   check(std::count_if(g.enemies.begin(),g.enemies.end(),[&](const Enemy& other){return other.id==e.id;})==1,"authored enemy IDs remain unique for persistent corpse and loot data");
   bool accessible=false;
   for(int x=int(e.home.x)-2;x<=int(e.home.x)+2;x++)for(int y=int(e.home.y)-2;y<=int(e.home.y)+2;y++){
    if(x<2||y<2||x>=MapSize-2||y>=MapSize-2)continue;V p{float(x),float(y)};
    accessible|=reached[y*MapSize+x]&&g.clearEnemySegment(p,e.home,.05f);
   }
   check(accessible,"every original and new encounter is connected to the realm arrival through real terrain collisions");
  }
  if(realm==WorldLocation::Volcano)for(const auto& p:g.props)if(p.type>=14){
   check(!Game::retiredVolcanoBridgeProp(p),"the two original bridge props are absent from fresh games");
   check(!Game::volcanoPropCoversBridge(p),"original and extended scenery silhouettes leave bridge decks clear");
  }
  bool extended=false;for(V p:queue)extended|=p.x>135&&p.y>135;check(extended,"the far corner of the expanded field is reachable without crossing invisible borders or lava");
  std::array<int,4> quadrants{};
  std::array<int,DecorationsPerRealm> decorations{};
  const int first=realm==WorldLocation::Forest?ForestDecorationStart:realm==WorldLocation::Desert?DesertDecorationStart:VolcanoDecorationStart;
  const int limit=realm==WorldLocation::Volcano?PropTypeCount:first+DecorationsPerRealm;
  for(const auto& prop:g.props)if(prop.type>=ForestDecorationStart){
   check(prop.type>=first&&prop.type<limit,"new scenery belongs to its realm");
   if(prop.type>=first&&prop.type<first+DecorationsPerRealm)decorations[prop.type-first]++;
  }
  check(std::all_of(decorations.begin(),decorations.end(),[](int n){return n>=2;}),"all five new designs are distributed across the realm");
  for(const auto& site:FieldSites)if(site.realm==realm){
   const V p=site.position;
   const int x=int(std::round(p.x)),y=int(std::round(p.y));
   quadrants[(p.x>=72?1:0)+(p.y>=72?2:0)]++;
   bool accessible=false;
   for(int dy=-3;dy<=3;dy++)for(int dx=-3;dx<=3;dx++){
    const int nx=x+dx,ny=y+dy;
    if(nx<2||ny<2||nx>=MapSize-2||ny>=MapSize-2)continue;
    accessible|=reached[ny*MapSize+nx]&& &fieldRegion(realm,{float(nx),float(ny)})==&site;
   }
   check(accessible,"named area has reachable terrain and retains its original label");
  }
  check(std::all_of(quadrants.begin(),quadrants.end(),[](int count){return count>=2;}),"all four quadrants have multiple named sites");
 }
 for(size_t bridge=0;bridge<VolcanoBridges.size();bridge++){
  float y=VolcanoBridges[bridge].y;int river=VolcanoBridgeRivers[bridge];bool clear=true;
  for(float x=-4.8f;x<=4.8f;x+=.2f)clear&=!volcanoLava({lavaCenter(river,y)+x,y},HeroBodyRadius);
  check(clear,"each new and original lava bridge admits the whole hero body");
 }
}
