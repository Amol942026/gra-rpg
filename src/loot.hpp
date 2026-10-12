#pragma once
#include "items.hpp"
#include "entity_info.hpp"
#include <array>
#include <random>
namespace ember {
constexpr int CorpseLootSize=5;
struct DropRule {Kind kind;int percent,minCount=1,maxCount=1,tier=0;};
using LootTable=std::array<DropRule,CorpseLootSize>;
inline const LootTable& enemyLootTable(int type){
 static constexpr LootTable tables[]={
  {{{Kind::Gold,100,4,15},{Kind::SlimeGel,80,1,3},{Kind::Health,15},{Kind::Crystal,30,1,2},{Kind::Amulet,1}}},
  {{{Kind::Gold,100,4,15},{Kind::WolfFang,75,1,2},{Kind::WolfFur,55,1,2},{Kind::Health,12},{Kind::Cloak,3,1,1,1}}},
  {{{Kind::Gold,100,4,15},{Kind::BoneShard,70,1,3},{Kind::Dagger,8},{Kind::Mana,18},{Kind::Helmet,5,1,1,1}}},
  {{{Kind::Gold,100,4,15},{Kind::Sword,100,1,1,2},{Kind::Ring,100,1,1,2},{Kind::ForestEssence,100,1,2,1},{Kind::DawnShard,100,1,1,2}}},
  {{{Kind::Gold,100,4,15},{Kind::BoarTusk,65,1,2},{Kind::BoarHide,80},{Kind::Health,10},{Kind::Armor,4,1,1,1}}},
  {{{Kind::Gold,100,4,15},{Kind::SpiderSilk,85,1,3},{Kind::VenomSac,40},{Kind::Mana,15},{Kind::Amulet,2,1,1,1}}},
  {{{Kind::Gold,100,4,15},{Kind::BanditBadge,60},{Kind::Dagger,12,1,1,1},{Kind::Health,25,1,2},{Kind::Boots,8,1,1,1}}},
  {{{Kind::Gold,100,4,15},{Kind::AncientBark,90,1,3},{Kind::ForestEssence,35},{Kind::RuneStone,8},{Kind::Amulet,4,1,1,1}}},
  {{{Kind::Gold,100,10,24},{Kind::ScorpionSting,75,1,2},{Kind::VenomSac,45},{Kind::Health,20},{Kind::Mana,15}}},
  {{{Kind::Gold,100,8,22},{Kind::ScarabShell,85,1,3},{Kind::Crystal,35},{Kind::Helmet,7,1,1,2},{Kind::Arrow,35,5,10}}},
  {{{Kind::Gold,100,8,20},{Kind::CobraScale,80,1,3},{Kind::VenomSac,55},{Kind::Health,25},{Kind::Ring,4,1,1,2}}},
  {{{Kind::Gold,100,60,110},{Kind::Sword,100,1,1,3},{Kind::SandEssence,100,2,4,1},{Kind::Mana,100,3,3},{Kind::DawnShard,100,1,1,3}}}
 };
 static_assert(sizeof(tables)/sizeof(tables[0])==EnemyTypeCount);
 return tables[std::clamp(type,0,EnemyTypeCount-1)];
}
inline bool rollDrop(int percent,std::mt19937& rng){return percent>=100||(percent>0&&std::uniform_int_distribution<int>(1,10000)(rng)<=percent*100);}
inline std::array<Item,CorpseLootSize> rollEnemyLoot(int type,std::mt19937& rng){
 std::array<Item,CorpseLootSize> result{};int slot=0;
 for(const auto& rule:enemyLootTable(type))if(rollDrop(rule.percent,rng))result[slot++]=item(rule.kind,rule.tier,std::uniform_int_distribution<int>(rule.minCount,rule.maxCount)(rng));
 return result;
}
inline int lootChance(int type,Item value){for(const auto& r:enemyLootTable(type))if(r.kind==value.kind&&r.tier==value.tier)return r.percent;return 0;}
inline std::string lootHint(int type){std::string text="Łup: ";int count=0;for(const auto& r:enemyLootTable(type))if(material(r.kind)&&r.percent>0){if(count++)text+=" · ";text+=itemShortName(r.kind);text+=" "+std::to_string(r.percent)+"%";if(count==2)break;}return text;}
}
