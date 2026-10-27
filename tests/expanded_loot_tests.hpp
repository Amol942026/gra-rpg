#pragma once
void expandedLootTests(){
 static_assert(int(Kind::DawnShard)==30&&int(Kind::DuneShield)==62&&int(Kind::SlimeBlade)==63,"old save item IDs must never move");
 check(ItemKindCount==99&&EquipmentKindCount==75,"catalogue includes 36 new equippable items alongside all original IDs");
 for(int type=0;type<EnemyTypeCount;type++){
  int added=0,count=0;for(const auto& rule:enemyLootTable(type)){count+=rule.kind!=Kind::None;added+=rule.kind>=Kind::SlimeBlade;}
  check(added==3&&count<=CorpseLootSize,"each beast gains three loot entries without overflowing its corpse");
 }
 for(int k=int(Kind::SlimeBlade);k<ItemKindCount;k++){
  Kind kind=Kind(k);int owner=-1;for(int type=0;type<EnemyTypeCount;type++)for(const auto& rule:enemyLootTable(type))if(rule.kind==kind)owner=type;
  check(owner>=0,"new item has a real source beast");unsigned seed=0;bool found=false;
  for(;seed<20000&&!found;seed++){std::mt19937 rng(seed);auto loot=rollEnemyLoot(owner,rng);for(const auto& value:loot)found|=value.kind==kind;}
  check(found,"new item is attainable through the unchanged rarity roll");
  Game g=emptyArena();g.level=20;g.rng.seed(seed-1);V p=g.player+V{.5f,0};g.enemies.push_back({910,owner,p,p,1,enemyInfo(owner).health});g.hit(g.enemies.back(),10000);
  auto& corpse=g.enemies.back();int slot=-1;for(int n=0;n<CorpseLootSize;n++)if(corpse.loot[n].kind==kind)slot=n;
  check(!corpse.alive&&slot>=0,"actual enemy death generates the selected new gear");auto before=corpse.loot;
  check(g.openCorpse(910),"new gear is reachable through real corpse looting");g.closeCorpse();g.openCorpse(910);
  bool same=true;for(int n=0;n<CorpseLootSize;n++)same&=before[n].kind==corpse.loot[n].kind&&before[n].count==corpse.loot[n].count;
  check(same,"reopening a corpse cannot reroll new equipment");
  check(g.transfer({Place::Corpse,slot,910},{Place::Bag,10})&&g.bag[10].kind==kind,"new drop transfers from the corpse into the backpack");
  check(g.transfer({Place::Bag,10},{Place::Equip,gearSlot(kind)})&&g.equipment[gearSlot(kind)].kind==kind,"looted item equips into its actual slot");
  check(itemSellValue(item(kind))>0,"new gear can be valued and sold using the existing economy");
 }
}
