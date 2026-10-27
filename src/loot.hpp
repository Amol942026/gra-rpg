#pragma once
#include "items.hpp"
#include "entity_info.hpp"
#include <array>
#include <random>
namespace ember {
constexpr int CorpseLootSize=15;
constexpr int QuestLootSlot=4;
struct DropRule {
 Kind kind=Kind::None;int minCount=1,maxCount=1,tier=0;
 constexpr DropRule(Kind k=Kind::None,int lo=1,int hi=1,int t=0):kind(k),minCount(lo),maxCount(hi),tier(t){}
 constexpr int chance()const{return itemDropChance(kind);}
};
using LootTable=std::array<DropRule,CorpseLootSize>;
inline const LootTable& enemyLootTable(int type){
 static constexpr LootTable tables[]={
  {{{Kind::Gold,4,15,0},{Kind::SlimeGel,1,3,0},{Kind::Health},{Kind::Crystal,1,2,0},{Kind::Amulet},{Kind::GelAmulet},{Kind::SlimeBlade},{Kind::SlimeCowl},{Kind::SlimeRing}}},
  {{{Kind::Gold,4,15,0},{Kind::WolfFang,1,2,0},{Kind::WolfFur,1,2,0},{Kind::Health},{Kind::WolfKnife},{Kind::WolfCloak},{Kind::MoonBow},{Kind::WolfHood},{Kind::WolfBoots},{Kind::WolfHeartAmulet}}},
  {{{Kind::Gold,4,15,0},{Kind::BoneShard,1,3,0},{Kind::Dagger},{Kind::Mana},{Kind::BoneHelmet},{Kind::CryptSword},{Kind::BoneShield},{Kind::BoneArmor},{Kind::GraveBow}}},
  {{{Kind::Gold,25,45,0},{Kind::GuardianSword},{Kind::GuardianArmor},{Kind::GuardianHelmet},{Kind::DawnShard,1,1,2},{Kind::ForestEssence,1,2,0},{Kind::RuneStone},{Kind::EchoRing},{Kind::Health,1,2,0},{Kind::Mana,1,2,0},{Kind::Crystal},{Kind::GuardianBoots},{Kind::GuardianShield},{Kind::GuardianAxe}}},
  {{{Kind::Gold,4,15,0},{Kind::BoarTusk,1,2,0},{Kind::BoarHide},{Kind::Health},{Kind::BoarArmor},{Kind::TuskAxe},{Kind::BoarBoots},{Kind::BoarShield},{Kind::BoarHelm}}},
  {{{Kind::Gold,4,15,0},{Kind::SpiderSilk,1,3,0},{Kind::VenomSac},{Kind::Mana},{Kind::SilkBoots},{Kind::WebCowl},{Kind::VenomRing},{Kind::SpiderDagger},{Kind::WebRobe},{Kind::NightSilkCloak}}},
  {{{Kind::Gold,4,15,0},{Kind::BanditBadge},{Kind::Dagger,1,1,1},{Kind::Health,1,2,0},{Kind::BanditBow},{Kind::BanditBoots},{Kind::Arrow,5,10,0},{Kind::BanditSword},{Kind::BanditHelm},{Kind::OutlawRing}}},
  {{{Kind::Gold,4,15,0},{Kind::AncientBark,1,3,0},{Kind::ForestEssence},{Kind::RuneStone},{Kind::RootShield},{Kind::GroveAmulet},{Kind::BarkHelm},{Kind::RootBoots},{Kind::GroveBow}}},
  {{{Kind::Gold,10,24,0},{Kind::ScorpionSting,1,2,0},{Kind::VenomSac},{Kind::Health},{Kind::Mana},{Kind::StingDagger},{Kind::ScorpionAxe},{Kind::StingShield},{Kind::AmberRing}}},
  {{{Kind::Gold,8,22,0},{Kind::ScarabShell,1,3,0},{Kind::Crystal},{Kind::ScarabHelmet},{Kind::Arrow,5,10,0},{Kind::ScarabArmor},{Kind::DuneShield},{Kind::ScarabBoots},{Kind::ScarabAxe},{Kind::ScarabRing}}},
  {{{Kind::Gold,8,20,0},{Kind::CobraScale,1,3,0},{Kind::VenomSac},{Kind::Health},{Kind::CobraBoots},{Kind::VenomRing},{Kind::MirageBow},{Kind::CobraCloak},{Kind::ScaleShield},{Kind::SerpentAmulet}}},
  {{{Kind::Gold,60,110,0},{Kind::DuneBlade},{Kind::SandEssence,2,4,0},{Kind::Mana,1,3,0},{Kind::DawnShard,1,1,3},{Kind::SandRobe},{Kind::DuneShield},{Kind::SunAmulet},{Kind::MirageBow},{Kind::Health,1,3,0},{Kind::Crystal},{Kind::DuneBoots},{Kind::DuneHelmet},{Kind::DawnBow}}},
 };
 static_assert(sizeof(tables)/sizeof(tables[0])==EnemyTypeCount);
 return tables[std::clamp(type,0,EnemyTypeCount-1)];
}
inline bool rollDrop(int percent,std::mt19937& rng){return percent>=100||(percent>0&&std::uniform_int_distribution<int>(1,10000)(rng)<=percent*100);}
inline std::array<Item,CorpseLootSize> rollEnemyLoot(int type,std::mt19937& rng){
 std::array<Item,CorpseLootSize> result{};int slot=0;const auto& table=enemyLootTable(type);
 bool quest=false;for(const auto& rule:table)quest|=rule.kind==Kind::DawnShard;
 for(const auto& rule:table)if(rollDrop(rule.chance(),rng)){
  Item value=item(rule.kind,rule.tier,std::uniform_int_distribution<int>(rule.minCount,rule.maxCount)(rng));
  if(value.kind==Kind::DawnShard){result[QuestLootSlot]=value;continue;}
  if(quest&&slot==QuestLootSlot)slot++;
  result[slot++]=value;
 }
 return result;
}
inline int lootChance(int type,Item value){for(const auto& r:enemyLootTable(type))if(r.kind==value.kind&&r.tier==value.tier)return r.chance();return 0;}
inline std::string lootHint(int type){std::string text="Łup: ";int count=0;for(const auto& r:enemyLootTable(type))if(material(r.kind)&&r.chance()>0){if(count++)text+=" · ";text+=itemShortName(r.kind);text+=" "+std::to_string(r.chance())+"%";if(count==2)break;}return text;}
}
