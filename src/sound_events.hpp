#pragma once
#include "entity_info.hpp"

namespace ember {
enum class Cue {
    Step, Swing, SlimeHit, WolfHit, BoneHit, StoneHit, EnemyFall, Hurt, Death,
    Roll, Dodge, Spell, Potion, Pickup, Equip, Drop, LevelUp, Victory,
    EnemyAttack, Ui, Reject, BowShot,
    ChestOpen, WeaponEquip, ChitinHit, FireHit, BeastAttack, StoneAttack,
    MagicAttack, VenomCast, CopperWave, ObsidianCleave, FireBreath,
    BoneAttack, ChitinAttack, SerpentAttack, ReptileAttack, WoodAttack, ArmoredAttack, BladeAttack, HeavySwing,
    FireballCast, FireballImpact, TeleportArrival, BowDraw,
    IceCast, IceImpact, LightningCast, LightningImpact, SpellLearn,
    SporekinAttack, SporekinHit, SporekinFall,
    DuneEyeAttack, DuneEyeHit, DuneEyeFall,
    CinderhornAttack, CinderhornHit, CinderhornFall, SpearThrust, SpearImpact,
    BriarSpearThrust, BriarSpearImpact, OasisSpearThrust, OasisSpearImpact,
    BasaltSpearThrust, BasaltSpearImpact, DawnSpearThrust, DawnSpearImpact,
    PowerSword, PowerAxe, PowerDagger, PowerSpear, PowerBow, CinderhornCharge,
    LandSlime, LandBeast, LandBone, LandStone, LandWood, LandShell, LandSand, LandEmber,
    SlimeVoiceAttack, SlimeVoiceHit, SlimeVoiceFall,
    WolfVoiceAttack, WolfVoiceHit, WolfVoiceFall,
    SkeletonVoiceAttack, SkeletonVoiceHit, SkeletonVoiceFall,
    StoneGuardianVoiceAttack, StoneGuardianVoiceHit, StoneGuardianVoiceFall,
    BoarVoiceAttack, BoarVoiceHit, BoarVoiceFall,
    ForestSpiderVoiceAttack, ForestSpiderVoiceHit, ForestSpiderVoiceFall,
    ForestBanditVoiceAttack, ForestBanditVoiceHit, ForestBanditVoiceFall,
    TreantVoiceAttack, TreantVoiceHit, TreantVoiceFall,
    AmberScorpionVoiceAttack, AmberScorpionVoiceHit, AmberScorpionVoiceFall,
    CopperScarabVoiceAttack, CopperScarabVoiceHit, CopperScarabVoiceFall,
    SandCobraVoiceAttack, SandCobraVoiceHit, SandCobraVoiceFall,
    LivingDuneVoiceAttack, LivingDuneVoiceHit, LivingDuneVoiceFall,
    SlagCrawlerVoiceAttack, SlagCrawlerVoiceHit, SlagCrawlerVoiceFall,
    LavaSalamanderVoiceAttack, LavaSalamanderVoiceHit, LavaSalamanderVoiceFall,
    FireImpVoiceAttack, FireImpVoiceHit, FireImpVoiceFall,
    AshWraithVoiceAttack, AshWraithVoiceHit, AshWraithVoiceFall,
    ObsidianSentinelVoiceAttack, ObsidianSentinelVoiceHit, ObsidianSentinelVoiceFall,
    EmberDrakeVoiceAttack, EmberDrakeVoiceHit, EmberDrakeVoiceFall,
    VarkhorVoiceAttack, VarkhorVoiceHit, VarkhorVoiceFall,
    SlimeDying,WolfDying,SkeletonDying,StoneGuardianDying,
    BoarDying,ForestSpiderDying,ForestBanditDying,TreantDying,
    AmberScorpionDying,CopperScarabDying,SandCobraDying,LivingDuneDying,
    SlagCrawlerDying,LavaSalamanderDying,FireImpDying,AshWraithDying,
    ObsidianSentinelDying,EmberDrakeDying,VarkhorDying,SporekinDying,
    DuneEyeDying,CinderhornDying,
    Count
};
// Preserve the original cue numbers in older saves; new entries are appended.
static_assert(int(Cue::SlimeDying)-int(Cue::SlimeVoiceAttack)==3*SporekinType);
static_assert(int(Cue::Count)-int(Cue::SlimeDying)==CinderhornType+1);
static_assert(int(Cue::CinderhornFall)-int(Cue::SporekinAttack)+1==3*RegionalEnemyTypeCount);
inline constexpr int enemyVoiceType(int type){
    return marshEnemy(type)?marshMotionFamily(type):type==DuneJackalType?1:type==GlassReaperType?ObsidianSentinelType:type;
}
inline constexpr Cue enemyGestureCue(int type,int offset,Cue fallback){
    if(type<0||type>=EnemyTypeCount)return fallback;
    type=enemyVoiceType(type);
    if(regionalEnemy(type))return Cue(int(Cue::SporekinAttack)+(type-SporekinType)*3+offset);
    return Cue(int(Cue::SlimeVoiceAttack)+type*3+offset);
}
inline constexpr Cue enemyAttackCue(int type){return enemyGestureCue(type,0,Cue::EnemyAttack);}
inline constexpr Cue enemyHitCue(int type){return enemyGestureCue(type,1,Cue::StoneHit);}
inline constexpr Cue enemyFallCue(int type){return enemyGestureCue(type,2,Cue::EnemyFall);}
inline constexpr Cue enemyDyingCue(int type){return Cue(int(Cue::SlimeDying)+enemyVoiceType(type));}
inline constexpr bool enemyDyingCue(Cue cue){return cue>=Cue::SlimeDying&&cue<Cue::Count;}
inline constexpr bool enemyAttackSound(Cue cue){
    return (cue>=Cue::SlimeVoiceAttack&&cue<Cue::SlimeDying&&(int(cue)-int(Cue::SlimeVoiceAttack))%3==0)||
           (cue>=Cue::SporekinAttack&&cue<=Cue::CinderhornFall&&(int(cue)-int(Cue::SporekinAttack))%3==0);
}
inline constexpr bool enemyLandingCue(Cue cue){
    return cue==Cue::EnemyFall||cue==Cue::SporekinFall||cue==Cue::DuneEyeFall||cue==Cue::CinderhornFall||
           (cue>=Cue::LandSlime&&cue<=Cue::LandEmber)||
           (cue>=Cue::SlimeVoiceAttack&&cue<Cue::SlimeDying&&(int(cue)-int(Cue::SlimeVoiceAttack))%3==2);
}
struct SoundEvent { Cue cue; float x,y; };
}
