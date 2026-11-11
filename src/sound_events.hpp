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
    CinderhornAttack, CinderhornHit, CinderhornFall, SpearThrust, SpearImpact, Count
};
// Preserve the original cue numbers in older saves; new entries are appended.
inline Cue regionalEnemyCue(int type,int offset){
    return Cue(int(Cue::SporekinAttack)+(type-SporekinType)*3+offset);
}
inline Cue enemyHitCue(int type){
    if(regionalEnemy(type))return regionalEnemyCue(type,1);
    constexpr Cue material[]={Cue::SlimeHit,Cue::WolfHit,Cue::BoneHit,Cue::StoneHit,
        Cue::WolfHit,Cue::SlimeHit,Cue::WolfHit,Cue::StoneHit,
        Cue::ChitinHit,Cue::ChitinHit,Cue::WolfHit,Cue::StoneHit,
        Cue::ChitinHit,Cue::WolfHit,Cue::WolfHit,Cue::FireHit,Cue::StoneHit,Cue::WolfHit,Cue::StoneHit};
    return type>=0&&type<19?material[type]:Cue::StoneHit;
}
inline Cue enemyAttackCue(int type){
    if(regionalEnemy(type))return regionalEnemyCue(type,0);
    if(type==2)return Cue::BoneAttack;
    if(type==5||type==8||type==9||type==12)return Cue::ChitinAttack;
    if(type==6)return Cue::BladeAttack;
    if(type==7)return Cue::WoodAttack;
    if(type==10)return Cue::SerpentAttack;
    if(type==13||type==17)return Cue::ReptileAttack;
    if(type==16)return Cue::ArmoredAttack;
    if(type==14||type==15)return Cue::MagicAttack;
    if(type==3||type==11||type==18)return Cue::StoneAttack;
    if(type==1||type==4)return Cue::BeastAttack;
    return Cue::EnemyAttack;
}
inline Cue enemyFallCue(int type){
    if(regionalEnemy(type))return regionalEnemyCue(type,2);
    return enemyBoss(type)?Cue::Victory:Cue::EnemyFall;
}
struct SoundEvent { Cue cue; float x,y; };
}
