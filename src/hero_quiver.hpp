#pragma once
#include "hero_motion.hpp"

namespace ember::hero_quiver {
// Rig +X points to screen right in a front view. The archer's anatomical
// right hand is therefore HandL in the rig. Synfig's front guides use +X
// for the drawing side, so convert only the archery tracks into rig space.
inline constexpr auto DrawHand=hero_motion::data::HandL, BowHand=hero_motion::data::HandR;
inline constexpr hero_motion::Vec mirror(hero_motion::Vec v){return {-v.x,v.y,v.z};}
inline constexpr hero_motion::Vec Nock=mirror({14,-11,65});
inline constexpr hero_motion::Vec Shaft=mirror({-.60f,0,-.80f}); // from feathers to point
inline constexpr float ArrowLength=36.f;
inline constexpr float MouthDepth=12.f;
inline constexpr float Pickup=.30f,Clear=.56f;

struct ArrowStock {
    int count;
    bool inHand;
};

inline ArrowStock stock(int ammo,bool lifted,bool released){
    const int count=std::clamp(ammo,0,3);
    const bool inHand=count>0&&lifted&&!released;
    return {count-int(inHand),inHand};
}

inline hero_motion::Vec slot(int index){
    // Other shafts fan gently behind the accessible arrow. The last shaft in
    // a full quiver is concealed below the lip until the next space opens.
    if(index==0)return Nock;
    if(index==1)return Nock+mirror({-2.0f,-1.6f,-1.1f});
    if(index==2)return Nock+mirror({-3.4f,1.55f,-2.2f});
    return slot(2)+Shaft*(MouthDepth+1.f);
}

inline hero_motion::Vec storedNock(int slotIndex,bool lifted,float release){
    if(lifted&&release<0)return slot(slotIndex+1);
    if(release<0)return slot(slotIndex);
    const float settle=hero_motion::ease(release);
    return slot(slotIndex+1)*(1-settle)+slot(slotIndex)*settle;
}

inline float exposedLength(hero_motion::Vec nock,hero_motion::Vec mouth,hero_motion::Vec inward){
    const auto difference=mouth-nock;
    return std::clamp(difference.x*inward.x+difference.y*inward.y+difference.z*inward.z,0.f,ArrowLength);
}

inline hero_motion::Vec featherAcross(hero_motion::Vec axis){
    return hero_motion::rotate(hero_motion::fromTo({0,1,0},axis),{1,0,0});
}

struct CarriedArrow {
    hero_motion::Vec nock,axis,across;
    float exposed;
};

inline CarriedArrow carried(const hero_motion::Pose& pose,hero_motion::Vec localAxis,float gripOffset,float preparation){
    using namespace hero_motion;
    const Vec fingers=pose.world[DrawHand].p;
    const float seating=ease((preparation-.93f)/.07f);
    if(seating>0){
        const Vec towardsRest=pose.world[BowHand].p-fingers;
        const Vec localRest=rotate(conjugate(pose.skin[BowHand].q),towardsRest);
        localAxis=localAxis*(1-seating)+localRest*(seating/std::max(.001f,length(localRest)));
    }
    localAxis=localAxis*(1/std::max(.001f,length(localAxis)));
    const Vec axis=pose.vector(BowHand,localAxis);
    const Vec nock=fingers-axis*gripOffset;
    const float exposed=preparation>Clear?ArrowLength:
        exposedLength(nock,pose.point(data::Spine,Nock+Shaft*MouthDepth),pose.vector(data::Spine,Shaft));
    return {nock,axis,pose.vector(BowHand,featherAcross(localAxis)),exposed};
}
}
