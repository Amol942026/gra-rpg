#pragma once

// Included in actor_art after the live forest and desert rigs.
namespace legacy_death {
struct Turn {
    float cosine,sine;
    explicit Turn(float degrees):cosine(std::cos(degrees*DEG2RAD)),sine(std::sin(degrees*DEG2RAD)){}
};
struct Motion {
    float progress,collapse,recoil,settle,side,accent;
    Turn pitch,roll;
    float chestLead=0;
};
#include "generated/forest_death_pose_data.hpp"

template<std::size_t N,std::size_t M>
inline std::array<float,M> fallPose(const std::array<std::array<float,M>,N>& keys,int type,float progress){
    const float t=std::clamp(progress/ enemy_death::groundContact(type),0.f,1.f)*float(N-1);
    const auto frame=std::min(std::size_t(t),N-2);
    auto result=keys[frame];
    for(std::size_t i=0;i<M;i++)result[i]+=(keys[frame+1][i]-result[i])*(t-frame);
    return result;
}

inline float between(float t,float first,float last){
    return hero_motion::ease(std::clamp((t-first)/(last-first),0.f,1.f));
}
inline float pulse(float t,float first,float peak,float last){
    return between(t,first,peak)*(1-between(t,peak,last));
}
inline P3 pitch(P3 p,Turn turn,float pivot){
    const float z=p.z-pivot;
    return {p.r,p.f*turn.cosine+z*turn.sine,pivot+z*turn.cosine-p.f*turn.sine};
}
inline P3 roll(P3 p,Turn turn,float pivot){
    const float z=p.z-pivot;
    return {p.r*turn.cosine+z*turn.sine,p.f,pivot+z*turn.cosine-p.r*turn.sine};
}
inline P3 lower(P3 p,float collapse,float bottom){
    p.z=p.z*(1-collapse)+bottom*collapse;
    return p;
}

inline P3 slime(P3 p,const Motion& m){
    const float rise=m.accent;
    const float wave=m.settle*.57f*std::sin(p.r*.13f+p.f*.10f);
    const float squeeze=1+.23f*m.collapse+.11f*m.recoil-.09f*rise;
    return {p.r*squeeze+m.side*m.recoil*p.z*.16f,
            p.f*squeeze-3*m.recoil,
            p.z*(1-.71f*m.collapse-.18f*m.recoil+.14f*rise)+wave};
}
constexpr int SkeletonFallPitch=-89,SkeletonFallRoll=6;
inline P3 skeleton(P3 p,const Motion& m){
    const float snap=m.accent;
    const float upper=between(p.z,26,66),nearSide=between(p.r*m.side,1,22);
    p.r+=m.side*(3*m.recoil*upper+2.5f*snap*nearSide);
    p.f+=5*snap*(1-upper)*between(p.z,3,27);
    p.z-=6*snap*upper;
    // Negative pitch sends the skull behind the hips, with the ribs and jaw up.
    p=pitch(p,m.pitch,31);
    p=roll(p,m.roll,31);
    p.z-=22.5f*m.collapse;
    return p;
}
inline P3 guardian(P3 p,const Motion& m){
    const float kneel=m.accent;
    const float torso=between(p.z,10,58);
    const float knee=between(p.z,3,19)*(1-between(p.z,22,52));
    p.r+=m.side*(3*m.recoil+3.2f*kneel)*torso;
    p.f+=8*kneel*knee+5*kneel*torso;
    p.z-=kneel*(10*torso+3*knee*(1+std::clamp(p.r*m.side/30.f,-1.f,1.f)));
    // The chest and shoulders turn as one mass, preserving the depth of the blocks.
    p=pitch(p,m.pitch,33);
    p=roll(p,m.roll,27);
    p.z-=17.5f*m.collapse;
    p.f+=3.8f*m.collapse;
    return p;
}
inline P3 spider(P3 p,const Motion& m){
    const float leg=between(std::abs(p.r),10,33);
    const float right=.5f+.5f*std::clamp(p.r*m.side/23.f,-1.f,1.f);
    const float fold=between(m.progress,.15f+right*.10f,.54f+right*.16f);
    const float flinch=m.accent;
    p.r*=1-.30f*leg*fold;
    p.f+=4*leg*fold*std::sin((p.f+10)*.16f);
    p.z=p.z*(1-.55f*m.collapse)+5.5f*leg*flinch*(1-right)+2.3f*leg*m.settle;
    return roll(p,m.roll,9);
}
inline constexpr int BanditFallPitch=-86,BanditFallRoll=7;
inline P3 bandit(P3 p,const Motion& m){
    constexpr float hips=35;
    const float stance=1-between(p.z,3,33);
    const float chest=between(p.z,33,62);
    p=pitch(p,Turn(-47*m.chestLead*chest),hips+9);
    p.r+=m.side*1.7f*m.chestLead*chest;
    P3 fallen=pitch(p,m.pitch,hips);
    fallen=roll(fallen,m.roll,hips);
    fallen.z-=25*m.collapse;
    // The boots briefly hold their footing as the shoulders give way.
    const float planted=stance*(1-between(m.collapse,.06f,.54f));
    p=fallen*(1-planted)+p*planted;
    return p;
}

inline void poseTreantFall(Rig& rig,int side,AnimationState state,float attack,float progress,float recoil){
    const auto [nearKnee,farKnee,tip,brace]=fallPose(treantFall,7,progress);
    const forest_detail::TreantFall fall{nearKnee,farKnee,tip,brace,float(side),recoil};
    const float live=1-between(tip,.04f,.74f);
    const float walk=creatureWalk(state)*live;
    state.idleTime*=live;
    rig.setCreatureSurfaces();rig.reserve(2400);rig.setGroundBounded();
    forest_detail::treant(rig,state,walk,attack,false,live,&fall);
}
inline void treantRest(Rig& rig,int side){poseTreantFall(rig,side,{},-1.f,1.f,0.f);}
inline P3 scorpion(P3 p,const Motion& m){
    const float tail=between(p.z,18,61)*(1-between(p.f,-28,-9));
    const float claw=between(p.f,22,49),leg=between(std::abs(p.r),13,32)*(1-claw);
    const float lash=m.accent;
    p.r+=tail*(17*lash*m.side+20*m.collapse);
    p.f+=tail*(7*m.recoil+6*m.collapse);
    p.r*=1-.17f*m.collapse*leg;
    p.r*=1-.11f*m.collapse*claw;
    p.z=p.z*(1-m.collapse*.53f)-p.z*.33f*tail*m.collapse;
    p.z+=2.3f*m.settle*claw;
    return roll(p,m.roll,6);
}
inline P3 scarab(P3 p,const Motion& m){
    const float bank=m.accent;
    const float leg=between(std::abs(p.r),13,34),head=between(p.f,9,31);
    p.r*=1-.32f*m.collapse*leg;
    p.f+=m.side*p.r*.14f*bank+3*m.collapse*head;
    p.z=p.z*(1-.55f*m.collapse)+2.8f*m.settle*head;
    p=roll(p,m.roll,13);
    return pitch(p,m.pitch,9);
}
inline P3 cobra(P3 p,const Motion& m){
    const float neck=between(p.z,12,79);
    const float hood=between(p.z,29,73);
    const float weave=m.accent;
    const float wave=std::sin(neck*PI*1.43f);
    p.r+=m.side*neck*(12*weave+3*m.recoil)-15*wave*m.collapse;
    p.r*=1-.10f*m.collapse*hood;
    p.f+=m.collapse*(39*neck-8*(1-neck))-8*weave*hood;
    p.z=p.z*(1-.89f*m.collapse)+m.settle*hood*3;
    return p;
}
inline P3 dune(P3 p,const Motion& m){
    const float crown=between(p.z,18,143);
    const float footing=1-between(p.z,8,74);
    const float cave=m.accent;
    p.r=p.r*(1+.12f*m.collapse*footing-.28f*m.collapse*crown)+crown*(18*cave*m.side+18*m.collapse);
    p.f=p.f*(1+.08f*m.collapse*footing-.23f*m.collapse*crown)+15*m.collapse*crown;
    p.z=p.z*(1-.83f*m.collapse)-21*cave*crown+1.5f*m.settle*footing;
    return p;
}

template<P3 (*Pose)(P3,const Motion&)>
inline P3 deform(P3 point,const void* context){
    return Pose(point,*static_cast<const Motion*>(context));
}

struct Beat {float first,peak,last;};
struct Swing {int collapse,recoil,accent;};
struct Profile {
    Rig::Deform deform;
    Beat beat;
    Swing pitch,roll;
};
inline constexpr Profile Profiles[]={
    // Elastic rebound, hip buckle, kneel, leg curl, stagger, crown swing,
    // tail lash, shell bank, hood weave, and collapsing crest.
    {deform<slime>,   {.06f,.16f,.30f},{},{ }},
    {},
    {deform<skeleton>,{.08f,.29f,.56f},{SkeletonFallPitch,-9,1},{SkeletonFallRoll,4,5}},
    {deform<guardian>,{.09f,.39f,.68f},{86,-8,4},{0,5,7}},
    {},
    {deform<spider>,  {.07f,.25f,.46f},{},{0,11,8}},
    {},
    {},
    {deform<scorpion>,{.02f,.26f,.58f},{},{0,7,-7}},
    {deform<scarab>,  {.08f,.35f,.68f},{0,5,-8},{0,5,21}},
    {deform<cobra>,   {.05f,.24f,.58f},{},{}},
    {deform<dune>,    {.13f,.42f,.82f},{},{}}
};
static_assert(std::size(Profiles)==LegacyEnemyTypeCount);
}

inline void buildFallenSkeleton(Rig& rig,int flank){
    using namespace legacy_death;
    const Motion pose{1,1,0,0,float(flank),0,Turn(SkeletonFallPitch),Turn(SkeletonFallRoll*flank)};
    rig.setGroundBounded();
    rig.setDeformation(deform<skeleton>,&pose);
    forest_detail::skeleton(rig,{},0,0,false,0);
    rig.setDeformation(nullptr,nullptr);
}

inline void poseBanditFall(Rig& rig,int flank,AnimationState stride,float attack,float progress,float recoil){
    using namespace legacy_death;
    const auto [buckle,hips,chest,freeArm,swordArm]=fallPose(banditFall,6,progress);
    const float lead=chest-hips;
    const Motion pose{progress,hips,recoil,0,float(flank),buckle,Turn(BanditFallPitch*hips-4*recoil),
                      Turn(flank*(BanditFallRoll*hips+18*lead)),lead};
    const bandit_art::Fall limbs{buckle,freeArm,swordArm,float(flank)};
    const float relaxed=between(chest,.09f,.94f);
    stride.moveBlend=creatureWalk(stride)*(1-relaxed);
    stride.motionInitialized=true;
    stride.idleTime*=1-relaxed;
    rig.setCreatureSurfaces();rig.reserve(2400);rig.setGroundBounded();
    rig.setDeformation(deform<bandit>,&pose);
    buildBandit(rig,stride,attack*(1-relaxed),false,relaxed,&limbs);
    rig.setDeformation(nullptr,nullptr);
}
inline void buildFallenBandit(Rig& rig,int flank){poseBanditFall(rig,flank,{},0,1,0);}

inline void buildLegacyDeath(Rig& rig,const Enemy& e,float progress){
    using namespace legacy_death;
    const auto& profile=Profiles[e.type];
    const auto& state=e.deathMotion;
    const auto key=enemy_death::sample(e.type,progress);
    const float angle=state.facingInitialized?state.facingAngle:int(state.direction)*PI/4;
    const float finish=int(e.animation.direction)*PI/4;
    rig.setFacing(angle+std::remainder(finish-angle,2*PI)*between(progress,.18f,enemy_death::groundContact(e.type)));
    const float side=float(e.deathSide());
    if(e.type==6){poseBanditFall(rig,e.deathSide(),state,e.deathAttack,progress,key.recoil);return;}
    if(e.type==7){
        const float attack=heavyMeleePreview(e.deathAction,e.deathAttack,TreantSwingWindup/(TreantSwingWindup+TreantSwingRecovery));
        poseTreantFall(rig,e.deathSide(),state,attack,progress,key.recoil);
        return;
    }
    const float accent=profile.deform?pulse(progress,profile.beat.first,profile.beat.peak,profile.beat.last):0;
    const auto swing=[&](Swing amount){
        return amount.collapse*key.collapse+amount.recoil*key.recoil+amount.accent*accent;
    };
    const Motion motion{progress,key.collapse,key.recoil,key.settle,side,accent,
                        Turn(swing(profile.pitch)),Turn(side*swing(profile.roll))};
    if(progress>0&&profile.deform){
        rig.setDeformation(profile.deform,&motion);
        rig.setGroundBounded();
    }
    if(e.type==2){
        rig.setCreatureSurfaces();rig.reserve(1700);
        forest_detail::skeleton(rig,state,creatureWalk(state),e.deathAttack,false,1-key.collapse);
    }else if(e.type<ForestEnemyTypeCount)
        buildForestCreature(rig,e.type,state,e.deathAttack,false,e.deathSpecial,e.deathAction);
    else
        buildDesert(rig,e.type,state,e.deathAttack,false);
    rig.setDeformation(nullptr,nullptr);
}
