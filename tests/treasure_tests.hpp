#pragma once
void treasureTests(){
 int common=0,rare=0;std::array<bool,EquipSize> slots{};bool varied=false;
 auto original=makeTreasure(0);
 for(unsigned seed=0;seed<1000;seed++){
  auto chests=makeTreasure(seed),same=makeTreasure(seed);int forest=0,desert=0,volcano=0;
  for(int c=0;c<TreasureCount;c++){
   const auto& chest=chests[c];forest+=chest.location==WorldLocation::Forest;desert+=chest.location==WorldLocation::Desert;volcano+=chest.location==WorldLocation::Volcano;
   check(chest.id==c&&!chest.opened,"six stable chest IDs start closed");
   for(int i=0;i<ChestLootSize;i++){
    Item value=chest.loot[i];auto rarity=itemRarity(value);
    check(!value.empty()&&value.count==1&&gearSlot(value.kind)>=0,"every chest guarantees three pieces of equipment");
    check(rarity==Rarity::Common||rarity==Rarity::Rare,"chests never drop very rare gear, materials or quest items");
    check(value.kind==same[c].loot[i].kind,"seed reproduces chest contents");
    for(int j=0;j<i;j++)check(value.kind!=chest.loot[j].kind,"a chest contains distinct kinds of gear");
    common+=rarity==Rarity::Common;rare+=rarity==Rarity::Rare;slots[gearSlot(value.kind)]=true;
    varied|=value.kind!=original[c].loot[i].kind;
   }
  }
  check(forest==2&&desert==2&&volcano==2,"exactly two treasure chests per field");
 }
 check(varied&&rare>4050&&rare<4950&&common+rare==18000,"new seeds vary treasure and maintain the 75/25 rarity roll");
 for(bool seen:slots)check(seen,"treasure can supply every equipment slot");
 for(WorldLocation location:{WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano}){
  Game g(9876),control(9876);g.activateField(location);
  int active=0;
  for(const auto& layout:TreasureLayout){
   if(layout.location!=location){check(!g.findChest(layout.id)&&!g.get({Place::Chest,0,layout.id}),"remote-realm treasure cannot be accessed");continue;}
   active++;auto* c=g.findChest(layout.id);Slot source{Place::Chest,0,c->id};
   check(!g.get(source),"closed chest contents cannot be accessed");
   check(!g.walkable(c->pos),"chest is a solid collision obstacle");
   int approach=0;for(V d:{V{1.5f,0},V{-1.5f,0},V{0,1.5f},V{0,-1.5f}})approach+=g.walkable(c->pos+d);
   check(approach>=3,"chest has at least three clear approach directions");
   g.player=c->pos+V{3,0};check(!g.openChest(c->id),"distant chest refuses opening");
   g.player=c->pos+V{1.5f,0};
   g.hp=0;check(!g.openChest(c->id),"dead hero cannot open a chest");g.hp=100;
   g.allocationRequired=true;check(!g.openChest(c->id),"point allocation blocks opening");g.allocationRequired=false;
   g.dawnQuest=DawnQuest::Returning;check(!g.openChest(c->id),"story transition blocks opening");g.dawnQuest=DawnQuest::Seeking;
   g.rollRemaining=.2f;check(!g.openChest(c->id),"rolling blocks opening");g.rollRemaining=0;
   g.sandPrisonRemaining=1;check(!g.openChest(c->id),"sand prison blocks opening");g.sandPrisonRemaining=0;
   check(g.openNearestChest()&&g.openedChest==c->id&&c->opened,"E interaction selects the nearest reachable chest");
   auto loot=c->loot;g.closeCorpse();check(g.openChest(c->id)&&c->loot[0].kind==loot[0].kind,"reopening does not reroll treasure");
   check(g.get(source)&&g.reachable(source),"opened chest exposes reachable loot");
   for(auto& value:g.bag)value=item(Kind::Sword);
   check(!g.transfer(source,{Place::Bag,10})&&c->loot[0].kind==loot[0].kind,"full backpack preserves treasure without swapping gear back");
   check(!g.transfer(source,{Place::Equip,(gearSlot(loot[0].kind)+1)%EquipSize})&&!g.drop(source,g.player)&&!g.use(source),"chest gear rejects a mismatched slot, ground drop and consumption");
   g.bag[10]={};check(g.transfer(source,{Place::Bag,10})&&c->loot[0].empty()&&g.bag[10].kind==loot[0].kind,"collecting transfers the exact piece once");
   check(!g.transfer({Place::Bag,10},source),"items cannot be deposited into treasure slots");
   check(!g.get({Place::Chest,-1,c->id})&&!g.get({Place::Chest,ChestLootSize,c->id}),"invalid chest slots are rejected");
   g.player=c->pos+V{5,0};g.enemies.clear();g.update(.01f,{},false);
   check(g.openedChest==-1&&!g.transfer({Place::Chest,1,c->id},{Place::Bag,11}),"leaving range closes the panel and denies remote transfers");
   g.enterMageTower();check(!g.findChest(c->id)&&!g.get(source),"tower hides field treasure");g.activateField(location);
   c=g.findChest(layout.id);check(c&&c->opened&&c->loot[0].empty()&&c->loot[1].kind==loot[1].kind,"travel preserves opened state and remaining treasure");
   g.player=c->pos+V{1.5f,0};g.openChest(c->id);g.bag[11]={};g.bag[12]={};
   check(g.transfer({Place::Chest,1,c->id},{Place::Bag,11})&&g.transfer({Place::Chest,2,c->id},{Place::Bag,12}),"all remaining treasure can be collected");
   g.closeCorpse();g.openChest(c->id);bool empty=true;for(auto value:c->loot)empty&=value.empty();
   check(empty,"empty chest stays empty after reopening");
  }
  check(active==2&&g.rng()==control.rng()&&g.combatRng()==control.combatRng(),"treasure is independent of combat and enemy-loot random streams");
 }
}
