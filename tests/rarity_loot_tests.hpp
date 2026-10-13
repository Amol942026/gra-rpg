#pragma once
void rarityLootTests(){
 std::array<bool,ItemKindCount> reachable{};std::array<int,4> rarityCounts{};std::array<bool,EquipSize> slots{};
 const int expected[]={60,15,4,1};
 for(int k=1;k<ItemKindCount;k++){
  Kind kind=static_cast<Kind>(k);Item value=item(kind);
  check(!itemName(value).empty()&&std::string(rarityName(itemRarity(value))).size()>0,"every item has a readable name and one of the four rarity labels");
  check(itemDropChance(value)==(kind==Kind::DawnShard?100:expected[int(itemRarity(value))]),"item chance is determined by rarity alone with the story exception");
  check(itemRarity(item(kind,3))==itemRarity(value)&&itemDropChance(item(kind,3))==itemDropChance(value),"equipment upgrade tier cannot change rarity or loot chance");
  rarityCounts[int(itemRarity(value))]++;
 }
 for(int type=0;type<EnemyTypeCount;type++){
  const auto& rules=enemyLootTable(type);std::array<bool,ItemKindCount> seen{};
  for(const auto& rule:rules)if(rule.kind!=Kind::None){
   int k=int(rule.kind);check(!seen[k],"a species table contains each possible item only once");seen[k]=reachable[k]=true;
   check(rule.chance()==itemDropChance(rule.kind),"beast type has no independent chance override");
   check(rule.kind==Kind::DawnShard||rule.chance()<100,"coins, consumables and all gear are random rather than guaranteed");
   check(rule.minCount>0&&rule.minCount<=rule.maxCount,"all loot quantity ranges are valid");
  }
 }
 for(int k=int(Kind::WolfKnife);k<ItemKindCount;k++){
  Kind kind=static_cast<Kind>(k);Game g=emptyArena();g.bag[10]=item(kind);int slot=gearSlot(kind);slots[slot]=true;
  check(reachable[k],"every new thematic item can drop from an existing beast or boss");
  check(!gearDescription(kind).empty(),"every new item has its own thematic description");
  int before=total(g);check(g.transfer({Place::Bag,10},{Place::Equip,slot})&&g.equipment[slot].kind==kind&&total(g)==before,"new gear equips in its real slot without losing the previous item");
  if(slot==0){
   g.skillRanks={1,1,1};g.mana=100;
   check(g.bindWeapon({Place::Equip,0},6)&&g.weaponWheel[6].kind==kind,"every new weapon can be added to the weapon wheel");
   check(g.selectWeapon(0)&&g.selectWeapon(6)&&g.equipment[0].kind==kind,"weapon shortcuts select the actual named item after a swap");
   if(archetype(kind)==Kind::Bow){
    int arrows=g.ammoCount();check(g.skillWeaponMatches(Skill::PiercingArrow)&&!g.skillWeaponMatches(Skill::StrongCut),"thematic bows enable piercing arrows and block strong cuts");
    check(g.preparePiercingArrow()&&g.beginBow({1,0}),"new bows can arm the piercing skill and draw the string");
    g.bowDrawTime=BowDrawDuration;check(g.releaseBow()&&g.ammoCount()==arrows-1&&g.mana==80&&g.arrows.back().piercing,"new bow releases a real piercing arrow with exact ammo and mana costs");
    check(g.arrows.back().damage>14,"every new bow deals more damage than the starter sword");
   }else if(archetype(kind)==Kind::Sword){
    check(g.skillWeaponMatches(Skill::StrongCut)&&!g.skillWeaponMatches(Skill::PiercingArrow),"thematic swords enable strong cuts and block piercing arrows");
    check(g.strongCut()&&g.mana==90&&g.empoweredPose(),"new sword executes and animates strong cut for ten mana");
   }else check(g.attack(),"new daggers and axes still swing as real melee weapons");
  }
 }
 for(bool available:slots)check(available,"new loot covers all six equipment slots");
 for(int count:rarityCounts)check(count>0,"the item catalogue contains every rarity");
 std::mt19937 rng(90210);bool coinsMissing=false,emptyBody=false;
 for(int n=0;n<3000;n++){
  auto loot=rollEnemyLoot(0,rng);bool gold=false,any=false;for(const auto& value:loot){gold|=value.kind==Kind::Gold;any|=!value.empty();}
  coinsMissing|=!gold;emptyBody|=!any;
 }
 check(coinsMissing&&emptyBody,"random coins can fail and an ordinary corpse may legitimately contain no loot");
 for(int boss:{3,11})for(int n=0;n<200;n++){
  auto loot=rollEnemyLoot(boss,rng);int shards=0;for(const auto& value:loot)shards+=value.kind==Kind::DawnShard;
  check(shards==1&&loot[QuestLootSlot].kind==Kind::DawnShard,"each boss always leaves exactly one fragment independently of all random rewards");
 }
}
