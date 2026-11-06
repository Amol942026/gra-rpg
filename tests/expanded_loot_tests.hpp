#pragma once
void expandedLootTests(){
 static_assert(int(Kind::DawnShard)==30&&int(Kind::DuneShield)==62&&int(Kind::SlimeBlade)==63,"old save item IDs must never move");
 static_assert(int(Kind::DawnBow)==98&&int(Kind::TrailFalchion)==99&&int(Kind::AshBow)==108);
 check(ItemKindCount==109&&EquipmentKindCount==85,"catalogue includes ten additional weapons alongside all original IDs");
 for(int type=0;type<LegacyEnemyTypeCount;type++){
  int added=0,count=0;for(const auto& rule:enemyLootTable(type)){count+=rule.kind!=Kind::None;added+=rule.kind>=Kind::SlimeBlade&&rule.kind<Kind::TrailFalchion;}
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
 int common=0,rare=0;
 for(int id=int(Kind::TrailFalchion);id<=int(Kind::AshBow);id++){
  Kind kind=Kind(id);
  check(gearSlot(kind)==0&&!stackable(kind),"all ten additions are single weapons for the weapon slot");
  common+=itemRarity(kind)==Rarity::Common;
  rare+=itemRarity(kind)==Rarity::Rare;
  check(std::count_if(MerchantOffers.begin(),MerchantOffers.end(),[&](const auto& offer){return offer.kind==kind;})==1,
        "each new weapon is available exactly once in Miron's catalogue");
 }
 check(common==4&&rare==6,"the new weapons preserve the chest rarity ceiling");
}
