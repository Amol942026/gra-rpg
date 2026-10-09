#pragma once
void forestLootTests(){
 Game forest;
 check(MapSize==48&&forest.enemies.size()==46,"expanded forest contains 46 monsters on a 48x48 map");
 std::array<int,EnemyTypeCount> species{};std::array<bool,PropTypeCount> props{};
 for(const auto& e:forest.enemies){species[e.type]++;check(forest.enemyPositionClear(e.home,e.radius()),"every initial monster has clearance at its spawn");}
 for(const auto& p:forest.props)props[p.type]=true;
 for(int type=0;type<EnemyTypeCount;type++)check(species[type]>0,"all eight species occur in the actual map");
 check(props[0]&&props[4]&&props[5]&&!forest.forage.empty(),"forest includes pine, birch, oak and mushrooms");
 check(separatedEnemies(forest)&&enemiesClearOfScenery(forest)&&forest.ground.empty(),"expanded initial world keeps body separation and empty starter ground");
 Game route=emptyArena();route.player={8,19};
 for(int i=0;i<420;i++)route.update(1.f/60,{1,0},false,true);
 check(route.player.x>40&&route.walkable(route.player),"player can run into the added eastern forest beyond the former border");
 Game edge=emptyArena();edge.player={MapWalkMax-.1f,MapWalkMax-.1f};edge.startRoll({1,1});
 for(int i=0;i<12;i++)edge.update(1.f/60,{},false);
 check(edge.player.x<=MapWalkMax&&edge.player.y<=MapWalkMax,"expanded border still stops rolling");
 std::mt19937 endpoints(24);for(int i=0;i<100;i++)check(!rollDrop(0,endpoints)&&rollDrop(100,endpoints),"0 percent never drops and 100 percent always drops");
 constexpr int trials=30000;
 for(int type=0;type<EnemyTypeCount;type++){
  std::mt19937 rng(4567+type);std::array<int,CorpseLootSize> occurrences{};int wolfBoth=0;bool valid=true;
  const auto& table=enemyLootTable(type);
  for(int n=0;n<trials;n++){
   auto loot=rollEnemyLoot(type,rng);bool fang=false,fur=false;
   for(const auto& value:loot)if(!value.empty()){
    int match=-1;for(int j=0;j<CorpseLootSize;j++)if(value.kind==table[j].kind&&value.tier==table[j].tier)match=j;
    if(match<0){valid=false;continue;}occurrences[match]++;const auto& rule=table[match];valid&=value.count>=rule.minCount&&value.count<=rule.maxCount;
    fang|=value.kind==Kind::WolfFang;fur|=value.kind==Kind::WolfFur;
   }
   wolfBoth+=fang&&fur;
  }
  check(valid,"all rolled loot belongs to its own species table with valid tier and quantity");
  for(int j=0;j<CorpseLootSize;j++){
   double chance=table[j].percent/100.0,observed=occurrences[j]/double(trials);
   double tolerance=std::max(.004,5*std::sqrt(chance*(1-chance)/trials));
   check(std::abs(observed-chance)<tolerance,"30000 deterministic rolls reproduce each configured drop probability");
  }
  if(type==1)check(std::abs(wolfBoth/double(trials)-.75*.55)<.02,"wolf fangs and fur roll independently and can drop together");
 }
 for(int k=int(Kind::WolfFang);k<=int(Kind::RuneStone);k++){
  Kind kind=static_cast<Kind>(k);Game g=emptyArena();g.bag[6]=item(kind,0,98);
  V pos=g.player+V{1,0};g.enemies.push_back({901,1,pos,pos,0,54});g.enemies.back().alive=false;g.enemies.back().loot[0]=item(kind,0,3);g.openCorpse(901);
  check(g.transfer({Place::Corpse,0,901},{Place::Bag,6})&&g.bag[6].count==99&&g.enemies.back().loot[0].count==2,"every material stacks from a body with excess left safely in the corpse");
  check(!g.transfer({Place::Bag,6},{Place::Equip,0})&&!g.use({Place::Bag,6})&&power(g.bag[6])==0,"materials cannot be equipped, consumed or counted as combat gear");
 }
 for(Kind kind:{Kind::Dagger,Kind::Cloak,Kind::Amulet}){Game g=emptyArena();g.bag[8]=item(kind,1);int slot=gearSlot(kind),before=slot==3?g.defense():g.damage();
  check(g.transfer({Place::Bag,8},{Place::Equip,slot})&&(slot==3?g.defense():g.damage())>before,"new weapon, cloak and amulet equip into the correct slot and improve stats");}
 Game moving=emptyArena(),still=emptyArena();V home{35,35};moving.player=still.player={2,2};moving.enemies.push_back({77,5,home,home,1,48});still.enemies=moving.enemies;
 for(int i=0;i<600;i++)moving.update(1.f/60,{},false);
 moving.hit(moving.enemies[0],100);still.hit(still.enemies[0],100);bool identical=true;
 for(int i=0;i<CorpseLootSize;i++){const auto&a=moving.enemies[0].loot[i];const auto&b=still.enemies[0].loot[i];identical&=a.kind==b.kind&&a.count==b.count&&a.tier==b.tier;}
 check(identical,"new spider patrols do not consume the separate loot RNG stream");
}
