#pragma once

namespace bandit_art {
inline constexpr Color Coat{137,93,59,255}, Hood{126,57,47,255}, Leather{77,54,39,255};
inline constexpr Color Trousers{76,71,55,255}, Mask{57,58,50,255}, Skin{199,156,112,255};
inline constexpr int CoatCell=6,HoodCell=15;
}

inline void buildBandit(Rig& r,const AnimationState& state,float attack,bool flash){
    using namespace bandit_art;
    using bandit_gait::sample;
    float walk=creatureWalk(state);
    float breath=state.idleTime*.56f;
    auto idle=sample(bandit_motion::idle,breath-std::floor(breath));
    auto step=sample(bandit_motion::walk,state.phase);
    auto motion=bandit_gait::blend(idle,step,walk);
    auto action=attack<0?sample(bandit_motion::windup,-attack):sample(bandit_motion::strike,attack);
    auto gait=bandit_gait::pose(state,walk,action.hip);
    float hip=gait.hip,lean=motion.lean+action.lean,z=hip+motion.bob+action.bob;
    auto color=[flash](Color value){return flash?Text:value;};
    auto paint=[&](P3 point,P3 size,Color tint,int cell){
        r.rounded(point,size,color(tint),flash?-1:100+cell);
    };
    Color leather=color(Leather),skin=color(Skin),hood=color(Hood),coat=color(Coat);
    Color seam=color(rgb(173,124,83)),steel=color(rgb(156,163,152));
    for(float side:{-1.f,1.f}){
        const auto& leg=gait.legs[side>0];
        P3 foot{side*7,leg.forward,leg.height},knee{side*7,leg.joint.forward,leg.joint.height};
        creatureJoint(r,{side*7,0,hip},knee,9,color(Trousers),.75f);
        creatureJoint(r,knee,foot,7,color(shade(Trousers,.83f)),.75f);
        P3 bootTop=foot+(knee-foot)*.49f;
        r.tapered(foot,bootTop,8.4f,8.4f,leather,1.08f);
        r.rounded(bootTop,{9.2f,9.2f,2.5f},shade(leather,1.2f));
        r.rounded(foot+P3{0,3,-1},{9,15,6},leather);
        r.rounded(foot+P3{0,3,-2.55f},{9,14,2.9f},shade(leather,.63f));
    }

    paint({0,.25f*lean,hip-1},{22,14,12},Coat,CoatCell);
    paint({0,.5f*lean,z+12},{24,17,25},Coat,CoatCell);
    r.tapered({-8,8+lean*.7f,z+20},{8,8+lean*.2f,hip+1},3,2,leather);
    r.rounded({-1,9+lean*.45f,z+10},{5,2.2f,4.7f},seam);
    r.rounded({-1,10.2f+lean*.45f,z+10},{2.8f,1.4f,2.7f},leather);
    r.rounded({0,0,hip+1},{25,18,4.5f},leather);
    r.rounded({0,9.2f,hip+1},{5.5f,2,4},steel);
    r.rounded({-11,4,hip-1.6f},{7.5f,8,10},color(rgb(101,92,59)));
    r.rounded({-11,7.5f,hip+.5f},{6.5f,2.5f,4.5f},seam);

    // The hood's loose end follows the upper-body tracks, while foot contact
    // and the weapon hand come from this same pose for picking and shadows.
    float cloth=motion.cloth+action.cloth;
    r.tapered({-4,lean-7,z+21},{-6+cloth,lean-10-cloth*.5f,z+10},6,3,hood,.55f);
    paint({0,lean-.5f,z+22},{22,16,7},Hood,HoodCell);
    for(float side:{-1.f,1.f}){
        float reach=action.arm*(side>0?1:.4f),swing=side*motion.arm;
        P3 shoulder{side*13.8f,lean*.7f,z+20};
        P3 elbow{side*16,lean*.45f+swing*.46f+reach*.45f,z+10+action.raise*.4f};
        P3 hand{side*17,lean*.35f+3+swing+reach,z-1+motion.raise+action.raise};
        creatureJoint(r,shoulder,elbow,8.8f,coat,.76f);
        creatureJoint(r,elbow,hand,7,coat,.68f);
        paint(shoulder+P3{0,.5f,1.2f},{11,13,8},Coat,CoatCell);
        P3 cuff=hand+(elbow-hand)*.25f;
        r.rounded(cuff,{6.8f,7.8f,5.5f},leather);
        r.rounded(hand,{5.5f,6,6.8f},skin);
        if(side>0){
            float angle=.16f+std::clamp(attack,0.f,1.f)*1.38f+std::min(0.f,action.arm)*.038f;
            P3 blade{-3-8*std::max(0.f,attack),std::sin(angle)*32,std::cos(angle)*32};
            P3 guard=hand+blade*.15f;
            r.tapered(guard,hand+blade,4.1f,1.6f,steel,.28f);
            r.tapered(hand-blade*.12f,guard,3.1f,3.1f,leather);
            r.tapered(guard+P3{-5.8f,0,0},guard+P3{5.8f,0,0},1.7f,1.9f,seam);
        }
    }
    float head=z+32,forward=lean*.84f;
    r.rounded({0,forward,head},{15.6f,15,19},skin);
    paint({0,forward-2.2f,head+6.4f},{19.4f,18.8f,13.8f},Hood,HoodCell);
    paint({0,forward-5.8f,head-1},{19,9,21},Hood,HoodCell);
    for(float side:{-1.f,1.f}){
        paint({side*7.1f,forward+.4f,head-.9f},{5.8f,13.8f,19},Hood,HoodCell);
        creatureEye(r,{side*3.3f,forward+6.9f,head+2.3f},2.05f,color(rgb(118,100,71)));
    }
    r.rounded({0,forward+5.7f,head-3.2f},{13.5f,6.3f,9},color(Mask));
    r.tapered({-5.2f,forward+7.3f,head-2},{5.2f,forward+7.3f,head-2},.85f,.85f,color(shade(Mask,1.35f)));
}
