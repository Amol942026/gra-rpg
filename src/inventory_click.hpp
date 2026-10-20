#pragma once
#include "game.hpp"
namespace ember {
// A click pair belongs to one unchanged item, not just one screen rectangle.
struct InventoryClicks {
 static constexpr double Interval=.4;
 static constexpr float Slop=8;
 Slot slot{};Item value{};double time=-1;float x=0,y=0;
 void clear(){slot={};value={};time=-1;}
 void moved(float px,float py){if(time>=0&&std::hypot(px-x,py-y)>Slop)clear();}
 bool press(Slot next,Item item,double now,float px,float py){
  bool same=slot.place==next.place&&slot.index==next.index&&slot.owner==next.owner&&
   value.kind==item.kind&&value.tier==item.tier&&value.count==item.count;
  if(time>=0&&same&&now>=time&&now-time<=Interval&&std::hypot(px-x,py-y)<=Slop){clear();return true;}
  slot=next;value=item;time=now;x=px;y=py;return false;
 }
};
}
