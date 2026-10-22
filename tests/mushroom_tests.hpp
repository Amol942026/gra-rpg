#pragma once
void mushroomTests(){
 {Game legacy;legacy.forage[3].value={};legacy.bag[20]=item(Kind::Mushroom,0,17);auto first=legacy.forage[0].pos;
  for(int id=49;id<=64;id++)legacy.forage.push_back({id,item(Kind::Mushroom),{30,30}});
  legacy.limitForestForage();check(legacy.forage.size()==48&&legacy.forage[3].value.empty()&&legacy.bag[20].count==17&&distance(legacy.forage[0].pos,first)<.00001f,"older saves lose only surplus plants without respawn or inventory loss");
  for(int id=49;id<=64;id++)legacy.forage.push_back({id,item(Kind::Mushroom),{30,30}});
  legacy.activateField(WorldLocation::Desert);legacy.limitForestForage();check(legacy.forestState.forage.size()==48&&legacy.forage.empty(),"legacy forest is thinned even when saved on another map");}
 for(unsigned seed=0;seed<16;seed++){
  Game g(seed);check(g.forage.size()==48&&MushroomCount==48,"each expedition generates 48 mushrooms, 25% fewer than before");
  bool valid=true;for(size_t i=0;i<g.forage.size();i++){
   const auto& m=g.forage[i];valid&=m.id==int(i)+1&&m.value.kind==Kind::Mushroom&&m.value.count==1&&g.walkable(m.pos)&&distance(m.pos,g.player)>=3.5f;
   valid&=!Game::onPath(int(std::round(m.pos.x)),int(std::round(m.pos.y)));
   for(const auto& p:g.props)valid&=distance(m.pos,p.pos)>=std::max(2.2f,propRadius(p.type)+.8f);
   for(size_t j=0;j<i;j++)valid&=distance(m.pos,g.forage[j].pos)>=2.2f;
  }
  check(valid,"mushrooms grow on accessible forest ground, away from obstacles and each other");
  check(g.rng==std::mt19937(seed)&&g.ground.empty(),"foraging leaves monster loot RNG and initial loose items unchanged");
 }
 Game same(42),again(42),different(43);bool matching=true,changed=false;
 for(size_t i=0;i<same.forage.size();i++){matching&=distance(same.forage[i].pos,again.forage[i].pos)<.0001f;changed|=distance(same.forage[i].pos,different.forage[i].pos)>.1f;}
 check(matching&&changed,"seeded expeditions reproduce plants while new seeds change their positions");
 Game g;g.props.clear();g.enemies.clear();g.player=g.forage[0].pos;g.hp=40;Slot source{Place::Forage,g.forage[0].id};
 check(!g.use(source)&&g.hp==40&&!g.get(source)->empty(),"growing mushroom must be collected before eating");
 check(!g.transfer(source,{Place::Equip,0})&&!g.drop(source,g.player)&&!g.transfer({Place::Bag,0},source),"plants cannot be equipped, swapped or duplicated by dropping");
 g.player=g.player+V{4,0};check(!g.transfer(source,{Place::Bag,24})&&g.bag[24].empty(),"distant mushroom cannot be collected");g.player=g.forage[0].pos;
 for(auto& value:g.bag)value=item(Kind::Sword);check(!g.transfer(source,{Place::Bag,24})&&!g.get(source)->empty(),"full backpack preserves the mushroom");
 g.bag[24]=item(Kind::Mushroom,0,99);check(!g.transfer(source,{Place::Bag,24})&&!g.get(source)->empty(),"full stack preserves uncollected mushroom");
 g.bag[24].count=98;check(g.transfer(source,{Place::Bag,24})&&g.bag[24].count==99&&g.get(source)->empty()&&g.hp==40,"collecting stacks once, removes plant and does not heal yet");
 check(!g.transfer(source,{Place::Bag,23})&&g.bag[24].count==99,"collected plant cannot be picked twice");
 check(g.use({Place::Bag,24})&&g.hp==50&&g.bag[24].count==98,"one eaten mushroom restores exactly ten HP and consumes one item");
 check(!g.use({Place::Bag,24})&&g.hp==50&&g.bag[24].count==98,"shared use cooldown prevents accidental repeated consumption");
 g.potionCd=0;g.hp=g.maxHp();check(!g.use({Place::Bag,24})&&g.bag[24].count==98,"full health does not consume a mushroom");
 g.hp=96;g.bag[24].count=1;check(g.use({Place::Bag,24})&&g.hp==100&&g.bag[24].empty(),"healing caps at maximum HP and clears exhausted stack");
 g.attributes[int(Attribute::Vitality)]=3;g.hp=123;g.potionCd=0;g.bag[24]=item(Kind::Mushroom);
 check(g.use({Place::Bag,24})&&g.hp==130,"mushroom respects increased maximum health");
 check(stackable(Kind::Mushroom)&&gearSlot(Kind::Mushroom)<0&&power(item(Kind::Mushroom))==0,"mushroom is stackable food without equipment stats");
}
