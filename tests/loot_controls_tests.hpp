#pragma once
#include "../src/inventory_click.hpp"
void lootControlsTests(){
 auto arena=[](bool chest){
  Game g=emptyArena();g.bag={};
  if(chest){g.player=g.chests[0].pos+V{1.5f,0};g.openChest(0);g.chests[0].loot={};}
  else{V p=g.player+V{.5f,0};g.enemies.push_back({77,1,p,p,0,100});g.enemies.back().alive=false;g.enemies.back().loot={};g.openCorpse(77);}
  return g;
 };
 auto source=[](bool chest){return Slot{chest?Place::Chest:Place::Corpse,0,chest?0:77};};
 auto count=[](const Game& g){int n=total(g);for(const auto& c:g.chests)for(Item v:c.loot)if(!v.empty())n+=v.count;return n;};
 for(bool chest:{false,true})for(int k=1;k<ItemKindCount;k++){
  Kind kind=Kind(k);int slot=gearSlot(kind);if(slot<0)continue;
  Game g=arena(chest);Slot from=source(chest);*g.get(from)=item(kind,2);g.equipment[slot]=item(archetype(kind));int before=count(g);
  check(g.transfer(from,{Place::Equip,slot})&&g.equipment[slot].kind==kind&&g.equipment[slot].tier==2,"all gear kinds can be equipped directly from either loot container");
  check(g.get(from)->empty()&&g.bag[0].kind==archetype(kind)&&count(g)==before,"direct equip saves replaced gear in the backpack and conserves every item");
  *g.get(from)=item(kind,3);for(auto& v:g.bag)v=item(Kind::Sword);before=count(g);
  check(!g.transfer(from,{Place::Equip,slot})&&g.equipment[slot].tier==2&&g.get(from)->tier==3&&count(g)==before,"full backpack rejects a gear replacement without changing either item");
  g.equipment[slot]={};before=count(g);
  check(g.transfer(from,{Place::Equip,slot})&&g.equipment[slot].tier==3&&g.get(from)->empty()&&count(g)==before,"an empty gear slot accepts loot even when the backpack is full");
 }
 for(bool chest:{false,true}){
  Game g=arena(chest);Slot from=source(chest);*g.get(from)=item(Kind::Helmet,2);int before=count(g);
  check(g.quickTransfer(from)&&g.get(from)->empty()&&g.bag[0].kind==Kind::Helmet&&count(g)==before,"quick loot goes to the first free backpack slot without equipping itself");
  check(g.quickTransfer({Place::Bag,0})&&g.equipment[1].tier==2&&g.bag[0].empty()&&count(g)==before,"backpack quick action equips gear to its matching slot");
  g.bag[0]=item(Kind::Helmet,3);before=count(g);check(g.quickTransfer({Place::Bag,0})&&g.bag[0].tier==2&&g.equipment[1].tier==3&&count(g)==before,"backpack quick equip swaps previous gear back into the same backpack slot");
  *g.get(from)=item(Kind::Health,0,12);for(auto& v:g.bag)v=item(Kind::Sword);g.bag[0]=item(Kind::Health,0,98);g.bag[1]=item(Kind::Health,0,90);before=count(g);
  check(g.quickTransfer(from)&&g.bag[0].count==99&&g.bag[1].count==99&&g.get(from)->count==2&&count(g)==before,"quick loot fills compatible stacks and leaves overflow safely in the source");
  check(!g.quickTransfer(from)&&g.get(from)->count==2&&count(g)==before,"a full backpack never loses or duplicates loot");
  g.bag[24]={};before=count(g);check(g.quickTransfer(from)&&g.bag[24].count==2&&g.get(from)->empty()&&count(g)==before,"quick loot places remaining items in a free slot");
  *g.get(from)=item(Kind::Helmet);g.player={30,30};before=count(g);
  check(!g.quickTransfer(from)&&count(g)==before,"quick loot cannot take an out-of-range item");
  g=arena(chest);*g.get(from)=item(Kind::Bow,2);g.equipment[0]=item(Kind::Bow);g.bag[5]=item(Kind::Arrow,0,20);g.beginBow({1,0});
  check(g.bowDrawing&&g.transfer(from,{Place::Equip,0})&&!g.bowDrawing,"direct weapon equip cancels a drawn bow without firing");
  *g.get(from)=item(Kind::Helmet);g.hp=0;check(!g.quickTransfer(from),"dead heroes cannot quick-loot");
  g.hp=100;g.allocationRequired=true;check(!g.quickTransfer(from),"pending allocation blocks quick loot");
 }
 Game g=arena(false);Slot from=source(false);g.enemies[0].type=3;*g.get(from)=item(Kind::DawnShard,2);
 check(g.quickTransfer(from)&&g.dawnFragments()==1&&g.towerReturnPending&&g.storyBlocked(),"quick collecting the boss artifact preserves the story transition");
 g=arena(false);*g.get(from)=item(Kind::Sword);g.closeCorpse();check(!g.quickTransfer(from)&&!g.enemies[0].loot[0].empty(),"a closed corpse cannot be looted by a stale click");
 g=arena(true);g.bag[0]=item(Kind::Health,0,3);g.hp=10;check(!g.quickTransfer({Place::Bag,0})&&g.hp==10&&g.bag[0].count==3,"backpack non-gear is not accidentally consumed by the quick equip action");
 InventoryClicks clicks;Slot bag{Place::Bag,10};Item sword=item(Kind::Sword);
 check(!clicks.press(bag,sword,1,20,30)&&clicks.press(bag,sword,1.2,20,30),"two presses on the same item form one double click");
 check(!clicks.press(bag,sword,1.3,20,30),"a third press begins a fresh pair instead of repeating the quick action");
 clicks.clear();check(!clicks.press(bag,sword,2,20,30)&&!clicks.press(bag,sword,2.401,20,30),"slow repeated clicks remain ordinary single clicks");
 clicks.clear();clicks.press(bag,sword,3,20,30);clicks.moved(40,30);check(!clicks.press(bag,sword,3.1,20,30),"drag movement cancels a pending double click even after returning to the slot");
 for(Slot other:{Slot{Place::Bag,11},Slot{Place::Chest,10,0},Slot{Place::Corpse,10,0}}){clicks.clear();clicks.press(bag,sword,4,20,30);check(!clicks.press(other,sword,4.1,20,30),"different slots and loot containers cannot combine into a double click");}
 clicks.clear();clicks.press({Place::Corpse,0,77},sword,5,20,30);check(!clicks.press({Place::Corpse,0,78},sword,5.1,20,30),"different corpse owners cannot share a double-click pair");
 for(Item changed:{item(Kind::Axe),item(Kind::Sword,1),item(Kind::Sword,0,2)}){clicks.clear();clicks.press(bag,sword,6,20,30);check(!clicks.press(bag,changed,6.1,20,30),"a changed kind, tier or count invalidates the previous click");}
}
