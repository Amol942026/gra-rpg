#pragma once
void towerStairsTests(){
 Game g;g.enterMageTower();g.dawnQuest=DawnQuest::AtTower;
 for(int floor=0;floor<TowerFloorCount;floor++){
  g.towerFloor=floor;
  for(int direction:{1,-1}){
   V base=towerStairBase(direction);float sign=direction==1?-1.f:1.f;
   check(g.walkable(base)&&g.walkable(base+V{-sign,0}),"wide stair entrance and approach stay walkable on each floor");
   for(float t:{.5f,1.2f,2.2f})check(g.walkable(base+V{sign*t,0})==!g.stairPresent(direction),"raised flight and open stairwell block walking only when the staircase exists");
   if(g.stairPresent(direction)){
    g.player=base+V{-sign,0};check(g.stairReachable(direction),"real stair entrance remains within shared floor-change interaction range");
    for(int i=0;i<20;i++)g.move({sign*.1f,0});check((g.player.x-base.x)*sign<=.19f,"movement cannot pass through the raised staircase or into the open shaft");
    g.player=base+V{sign,-TowerStairWidth*.5f-.1f};check(!g.walkable(g.player),"stone balustrade has a matching physical collision boundary");
   }
  }
 }
 g.towerFloor=0;g.player=TowerArrival;check(g.walkable(g.player),"tower arrival remains safely clear of full-size stairs");
}
