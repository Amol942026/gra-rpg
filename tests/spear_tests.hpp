#pragma once

void spearTests(){
 static_assert(FirstSpear==129&&SpearCount==5);
 auto arena=[]{
  Game g=emptyArena();g.enemies.clear();g.props.clear();g.equipment[0]=item(Kind::Spear);return g;
 };
 auto enemy=[](Game& g,int id,V offset){
  V p=g.player+offset;
  g.enemies.push_back({id,2,p,p,150,150});g.enemies.back().windup=3;
 };
 auto advance=[](Game& g,int frames){for(int i=0;i<frames;i++)g.update(1.f/60,{},false);};
 auto cue=[](const Game& g,Cue sound){return std::count_if(g.sounds.begin(),g.sounds.end(),[&](auto e){return e.cue==sound;});};
 Game g=arena();enemy(g,1,{2.6f,0});enemy(g,2,{3.7f,0});enemy(g,3,{2.4f,1.2f});enemy(g,4,{-1.4f,0});
 check(g.attack({1,0})&&g.spearActive()&&!g.attack({1,0}),"spear starts one timed thrust and respects cooldown");
 check(cue(g,Cue::SpearThrust)==1&&cue(g,Cue::Swing)==0,"spear uses a dedicated thrust sound");
 advance(g,10);
 check(g.enemies[0].hp==150&&cue(g,Cue::SpearImpact)==0,"spear windup does not hit early");
 advance(g,3);
 check(g.enemies[0].hp<150&&g.enemies[1].hp==150&&g.enemies[2].hp==150&&g.enemies[3].hp==150,
       "thrust hits the first enemy in its narrow forward line, outside ordinary melee reach");
 float hp=g.enemies[0].hp;advance(g,24);
 check(g.enemies[0].hp==hp&&cue(g,Cue::SpearImpact)==1&&!g.spearActive(),"recovery never repeats damage or the impact sound");
 g=arena();enemy(g,1,{2.6f,0});g.props.push_back({g.player+V{1.2f,0},3});
 check(g.attack({1,0}),"spear begins even when a prop obstructs the strike");advance(g,13);
 check(g.enemies[0].hp==150&&cue(g,Cue::SpearImpact)==1,"solid prop stops the thrust and has its own contact event");
 g=arena();enemy(g,1,{2.6f,1.25f});
 check(g.attack({1,0}),"spear can miss a target off the aim line");advance(g,13);
 check(g.enemies[0].hp==150&&cue(g,Cue::SpearImpact)==0,"a miss plays air movement without an impact");
 g=arena();enemy(g,1,{2.6f,0});g.bag[10]=item(Kind::Sword);
 check(g.attack({1,0})&&g.transfer({Place::Bag,10},{Place::Equip,0}),"weapon change during spear windup succeeds");
 advance(g,15);check(g.enemies[0].hp==150&&!g.spearActive(),"unequipping cancels an uncommitted strike");
 g=arena();enemy(g,1,{2.6f,0});check(g.attack({1,0})&&g.startRoll({0,-1}),"rolling cancels the spear windup");
 advance(g,14);check(cue(g,Cue::SpearImpact)==0,"roll does not trigger a delayed hit");
 constexpr int sources[]={6,7,8,16,18},chances[]={60,15,15,4,1};
 for(int i=0;i<SpearCount;i++){
  Kind kind=Kind(FirstSpear+i);
  check(archetype(kind)==Kind::Spear&&gearSlot(kind)==0&&!stackable(kind)&&
        lootChance(sources[i],item(kind))==chances[i],"each spear is a single weapon with its own documented source");
 }
}
