#pragma once

namespace forest_detail {
inline constexpr Color Gel{84,143,83,255}, Fur{112,121,116,255}, Ruff{176,178,155,255};
inline constexpr Color Bone{197,181,141,255}, Bark{84,72,52,255}, Leaf{68,99,54,255};
inline constexpr Color Amber{236,178,77,255};
inline constexpr int BarkMaterial=7;

inline forest_motion::Key sample(const forest_motion::Key* keys,int section,float phase){
    float f=std::clamp(phase,0.f,1.f)*forest_motion::Frames;
    int i=std::min(int(f),forest_motion::Frames-1);
    const auto& a=keys[section*(forest_motion::Frames+1)+i];
    const auto& b=keys[section*(forest_motion::Frames+1)+i+1];
    auto mix=[t=f-i](float x,float y){return x+(y-x)*t;};
    return {mix(a.squash,b.squash),mix(a.lean,b.lean),mix(a.reach,b.reach),
            mix(a.rise,b.rise),mix(a.drift,b.drift),mix(a.twist,b.twist)};
}

struct Motion {forest_motion::Key gait,action;};
inline Motion motion(const forest_motion::Key* keys,const AnimationState& state,float walk,float attack,float breathRate){
    float breath=state.idleTime*breathRate;
    auto idle=sample(keys,0,breath-std::floor(breath)),step=sample(keys,1,state.phase);
    auto mix=[walk](float a,float b){return a+(b-a)*walk;};
    auto action=sample(keys,2,attack<0?.55f*std::sin(-std::clamp(attack,-1.f,0.f)*PI):attack);
    if(attack<0){action.squash=-action.squash;action.lean=-action.lean;action.reach=-action.reach;}
    return {{mix(idle.squash,step.squash),mix(idle.lean,step.lean),mix(idle.reach,step.reach),
             mix(idle.rise,step.rise),mix(idle.drift,step.drift),mix(idle.twist,step.twist)},action};
}

inline void surface(Rig& r,P3 p,P3 size,Color base,int cell,bool flash){
    r.rounded(p,size,flash?Text:base,flash?-1:100+cell,false,-1,true);
}

inline void slime(Rig& r,const AnimationState& s,float walk,float hit,bool flash){
    auto m=motion(forest_motion::slime,s,walk,hit,.63f);
    float stretch=1+m.gait.squash+m.action.squash;
    float height=31*stretch,width=1/std::sqrt(stretch);
    float forward=m.gait.lean+m.action.lean+m.action.reach*.32f;
    Color gel=flash?Text:Gel,rim=shade(gel,.70f);
    surface(r,{0,forward,height*.50f},{44*width,41*width+m.action.reach,height},gel,0,flash);
    for(int i=0;i<5;i++){
        float angle=(i+.35f)*2*PI/5;
        float ripple=std::sin(angle+m.gait.drift)*1.7f;
        r.rounded({std::cos(angle)*(15*width+ripple),forward+std::sin(angle)*16,3.5f},
                  {15,16,7},shade(rim,1+i*.045f));
    }
    for(float side:{-1.f,1.f}){
        float eyeZ=height*(.57f+side*.022f*m.gait.twist);
        creatureEye(r,{side*6.5f,forward+width*17.8f+m.action.reach*.32f,eyeZ},4.25f,Amber);
    }
    r.tube({{-4,forward+width*19.5f,8},{0,forward+width*21,7.5f},{4,forward+width*19.5f,8}},
           1.15f,.8f,shade(rim,.72f));
    r.rounded({-7,forward+4,height*.87f},{9,5,2},flash?Text:rgb(168,203,117));
    r.rounded({-10,forward+6,height*.93f},{4,2.6f,1.5f},flash?Text:rgb(215,231,159));
}

#include "wolf_art.hpp"

inline void skeleton(Rig& r,const AnimationState& s,float walk,float hit,bool flash,float vitality=1){
    auto m=motion(forest_motion::skeleton,s,walk,hit,.37f);
    auto gait=grounded_gait::fromStanding(s,walk,forest_gait::Skeleton,-std::abs(m.action.squash)*16);
    float z=gait.hip+m.gait.rise,lean=m.gait.lean+m.action.lean;
    const float weather=1-vitality;
    Color bone=flash?Text:ColorLerp(Bone,rgb(182,165,130),weather);
    Color dark=shade(bone,.62f),iron=flash?Text:rgb(128,140,128);
    Color rust=flash?Text:rgb(131,89,54);
    const Color cavity=rgb(63,62,48),patina=shade(bone,1.19f);
    for(float side:{-1.f,1.f}){
        const auto& leg=gait.legs[side>0];
        P3 knee{side*6.8f,leg.joint.forward,leg.joint.height},foot{side*6.8f,leg.forward,leg.height};
        creatureJoint(r,{side*6.8f,0,gait.hip},knee,4.6f,bone,.80f);
        creatureJoint(r,knee,foot,4.1f,bone,.75f);
        r.tapered(knee+P3{side*1.5f,-.5f,-1},foot+P3{side*1.3f,0,.8f},1.35f,1.35f,dark);
        r.rounded(foot+P3{0,2.4f,-1.6f},{5.6f,9.2f,4.8f},bone);
        for(int i=0;i<3;i++){
            P3 toe=foot+P3{i*1.35f-1.35f,4,-1.8f};
            r.tapered(toe,toe+P3{0,2.8f,-.5f},1.2f,1.15f,bone);
        }
        if(weather>0){
            r.rounded(knee+P3{0,1.75f,-.6f},{4.6f,2.7f,3.9f},ColorLerp(bone,patina,weather));
            r.tapered(knee+P3{side*.9f,1.1f,-2},foot+P3{side*.85f,1.2f,1.5f},
                1.1f*weather,1.0f*weather,patina,.56f);
        }
    }
    surface(r,{0,0,gait.hip},{19.2f,12,8},bone,2,flash);
    r.rounded({0,6.1f,gait.hip+.4f},{7,2.6f,4.1f},dark);
    if(weather>0){
        for(float side:{-1.f,1.f}){
            const P3 socket{side*5.15f,5.1f,gait.hip-.9f};
            r.rounded(socket,{4.9f,2.2f,2.9f},ColorLerp(bone,cavity,weather));
            r.tapered({side*4.5f,4.8f,gait.hip+1.1f},{side*8.f,1.5f,gait.hip+3},
                1.25f*weather,1.45f*weather,patina);
        }
    }
    for(int i=0;i<7;i++){
        float shift=i*lean/7;
        r.rounded({0,-3+shift,z+2+i*3.5f},{4.4f,5.4f,3.65f},shade(bone,.86f+i*.021f));
    }
    for(int j=0;j<5;j++)for(float side:{-1.f,1.f}){
        float rib=z+6+j*3.55f,depth=lean*(j+2)*.16f,w=9.7f-j*.62f;
        r.tube({{side*1.9f,-3+depth,rib},{side*w,1+depth,rib+1.35f},
                {side*(w+.8f),5+depth,rib},{side*5.8f,8+depth,rib-1.75f},{side*1.8f,8+depth,rib-2.1f}},
               1.7f,1.9f,shade(bone,.83f+j*.065f));
        if(weather>0){
            r.tapered({side*(w+.85f),4.7f+depth,rib+.35f},{side*5.8f,8.65f+depth,rib-1.45f},
                .8f*weather,.8f*weather,patina,.66f);
        }
    }
    r.tapered({0,8+lean*.18f,z+6},{0,8+lean,z+21},2.4f,1.9f,bone);
    if(weather>0){
        for(int i=0;i<5;i++)
            r.rounded({0,9+lean*.55f,z+8+i*2.65f},{3.1f,1.4f,1.75f},ColorLerp(bone,patina,weather*.87f));
    }
    for(float side:{-1.f,1.f}){
        P3 shoulder{side*13.3f,lean*.73f,z+22};
        float reach=(side>0?1:.43f)*m.action.reach;
        auto arm=sword_pose::solveArm({shoulder.r,shoulder.f,shoulder.z},
            {side*17,3+side*m.gait.reach+reach,z+2+m.action.rise},side);
        P3 elbow=swordPoint(arm.elbow),hand=swordPoint(arm.hand);
        r.tapered({0,-2+lean*.7f,z+22},shoulder,2.9f,2.6f,bone);
        creatureJoint(r,shoulder,elbow,4.2f,bone,.72f);
        creatureJoint(r,elbow,hand,3.4f,bone,.70f);
        r.rounded(hand,{4.3f,5.2f,4.9f},bone);
        if(side<0){
            r.rounded(shoulder+P3{0,.3f,1.7f},{7.7f,9.2f,6.7f},rust);
            r.rounded(shoulder+P3{0,4.5f,2.1f},{2.2f,1.3f,2.2f},shade(rust,1.52f));
        }else{
            P3 blade=swordPoint(sword_pose::blade(-m.action.reach*.032f,1.30f-m.action.reach*.074f));
            P3 guard=hand+blade*.15f;
            r.tapered(guard,hand+blade,4.0f,1.7f,iron,.24f);
            r.tapered(guard+P3{-5.3f,0,0},guard+P3{5.3f,0,0},1.9f,1.8f,rust);
            r.tapered(hand-blade*.10f,guard,2.6f,2.7f,dark);
            if(weather>0){
                const P3 edge{.68f,.47f,0};
                r.tapered(hand+blade*.26f+edge,hand+blade*.91f+edge*.26f,
                    .7f*weather,.7f*weather,shade(iron,1.28f),.35f);
            }
        }
        if(weather>0){
            r.tapered({side*5.1f,2.7f+lean*.56f,z+22.75f},shoulder+P3{0,1.8f,.9f},
                1.8f*weather,1.3f*weather,patina,.71f);
            for(int i=0;i<4;i++){
                const P3 knuckle=hand+P3{(i-1.5f)*1.05f,1.2f,-1.25f};
                r.tapered(knuckle,knuckle+P3{side*.45f,.8f,-1.8f},1.18f*weather,1.05f*weather,patina,.66f);
            }
            if(side<0){
                for(int i=0;i<3;i++)
                    r.rounded(shoulder+P3{(i-1)*1.7f,4.5f-i*.16f,2.25f+i*.9f},
                        {1.25f,1.0f,1.35f},ColorLerp(rust,rgb(185,149,94),weather));
                r.tapered(shoulder+P3{-2,3.7f,4.15f},shoulder+P3{2.35f,3.7f,4.3f},
                    .8f*weather,.75f*weather,shade(rust,1.55f));
            }
        }
    }
    P3 skull{m.gait.twist*.50f,lean,z+35.6f+m.action.rise*.12f};
    surface(r,skull,{17.7f,16.7f,17.2f},bone,2,flash);
    float jaw=std::max(0.f,m.action.rise)*.22f;
    r.rounded(skull+P3{0,3,-6.8f-jaw},{13.5f,10.2f,4.6f},bone);
    r.rounded(skull+P3{0,8,-5.6f-jaw*.6f},{10.4f,1.5f,2.3f},shade(dark,.57f));
    for(float side:{-1.f,1.f}){
        r.rounded(skull+P3{side*4.5f,7.1f,1.2f},{6.3f,4.2f,4.8f},rgb(48,46,35));
        r.rounded(skull+P3{side*4.5f,9.1f,.9f},{2.15f,1.4f,1.7f},ColorLerp(rgb(48,46,35),Amber,vitality));
    }
    for(int i=0;i<5;i++)r.rounded(skull+P3{i*1.9f-3.8f,8.8f,-5.1f},{1.28f,1.5f,2.5f},shade(bone,1.15f));
    r.rounded(skull+P3{0,8.5f,-2.2f},{2.6f,1.6f,3.4f},shade(dark,.72f));
    if(weather>0){
        for(float side:{-1.f,1.f}){
            r.tube({skull+P3{side*2.1f,8.5f,3.65f},skull+P3{side*4.7f,8.2f,4.25f},
                    skull+P3{side*7.25f,5.7f,2.8f}},1.55f*weather,1.30f*weather,patina);
            r.tube({skull+P3{side*7.2f,5.3f,-1.2f},skull+P3{side*5.8f,7.05f,-2.65f},
                    skull+P3{side*3.65f,7.8f,-3.5f}},1.8f*weather,1.35f*weather,bone);
        }
        const P3 fracture=skull+P3{-3.6f,4.4f,6.4f};
        r.tube({fracture,fracture+P3{1.85f,1.35f,.13f},fracture+P3{2.8f,1.6f,-.75f},
                fracture+P3{3.65f,1.85f,-2.1f}},.72f*weather,.52f*weather,shade(bone,.47f));
        for(int i=0;i<5;i++)
            r.rounded(skull+P3{i*1.78f-3.6f,8.1f,-6.65f-jaw*.55f},{1.18f,1.4f,1.85f},
                ColorLerp(bone,patina,weather));
    }
}

struct TreantFall {
    float nearKnee,farKnee,tip,brace,flank,recoil;
};
struct TreantFallTrunk {
    P3 liveHip,posedHip;
    float pitchCos=1,pitchSin=0,rollCos=1,rollSin=0,crownLag=0,flank=0;
};
inline P3 fallenTreantTrunk(P3 point,const void* context){
    const auto& pose=*static_cast<const TreantFallTrunk*>(context);
    P3 local=point-pose.liveHip;
    // The crown lags behind the trunk just as the front branches catch it.
    const float bow=pose.crownLag*enemy_death::window(local.z,12.f,75.f);
    local.f-=12*bow;
    local.z+=15*bow;
    const float forward=local.f*pose.pitchCos+local.z*pose.pitchSin;
    const float height=local.z*pose.pitchCos-local.f*pose.pitchSin;
    return pose.posedHip+P3{local.r*pose.rollCos+height*pose.rollSin+3.5f*pose.flank*bow,
                            forward,height*pose.rollCos-local.r*pose.rollSin};
}
inline void treant(Rig& r,const AnimationState& s,float walk,float phase,bool flash,float glow=1,const TreantFall* fall=nullptr){
    auto m=motion(forest_motion::treant,s,walk,0,.32f);
    const bool striking=phase>=0;
    const auto idle=forest_melee::treantSwing[0];
    auto action=striking?forest_melee::sample(forest_melee::treantSwing,phase):idle;
    if(fall){
        const auto release=[glow](float a,float b){return a+(b-a)*glow;};
        action={release(idle.side,action.side),release(idle.forward,action.forward),release(idle.up,action.up),
                release(idle.lean,action.lean),release(idle.drop,action.drop),release(idle.twist,action.twist),
                release(idle.follow,action.follow)};
    }
    const auto gait=grounded_gait::pose(s,walk,treant_gait::Contact);
    float z=gait.hip-40+action.drop,lean=m.gait.lean+action.lean;
    Color bark=flash?Text:Bark,leaf=flash?Text:Leaf,dark=shade(bark,.53f);
    const int material=flash?-1:BarkMaterial;
    const P3 liveHip{0,action.lean*.35f,gait.hip+action.drop};
    TreantFallTrunk trunk{liveHip,{}};
    for(float side:{-1.f,1.f}){
        const auto& leg=gait.legs[side>0];
        P3 foot{side*12,leg.forward,leg.height},knee{side*10,leg.joint.forward,leg.joint.height};
        P3 hip=liveHip+P3{side*8,0,0};
        float rootTip=0;
        if(fall){
            const float flex=side==fall->flank?fall->nearKnee:fall->farKnee;
            const float upperLength=std::hypot(hip.f-knee.f,hip.z-knee.z);
            const float lowerLength=std::hypot(knee.f-foot.f,knee.z-foot.z);
            const float upper=std::atan2(knee.f-hip.f,hip.z-knee.z);
            const float lower=std::atan2(foot.f-knee.f,knee.z-foot.z);
            const float foldedUpper=(57.f-131.f*fall->tip)*DEG2RAD;
            const float foldedLower=(-82.f-16.f*fall->tip)*DEG2RAD;
            const float upperAngle=upper+(foldedUpper-upper)*flex;
            const float lowerAngle=lower+(foldedLower-lower)*flex;
            // Once the knee is down, pivot forward over that same foothold.
            const float kneeContact=side==fall->flank?5.f:2.f;
            hip.f+=(kneeContact-upperLength*std::sin(foldedUpper)-hip.f)*flex;
            hip.z+=(5.4f+upperLength*std::cos(foldedUpper)-hip.z)*flex;
            knee={side*(10+1.5f*flex),hip.f+upperLength*std::sin(upperAngle),hip.z-upperLength*std::cos(upperAngle)};
            foot={side*(12+2.f*flex),knee.f+lowerLength*std::sin(lowerAngle),
                  std::max(3.2f,knee.z-lowerLength*std::cos(lowerAngle))};
            // Finish with the roots trailing, so the prone legs keep their
            // full length and the feet do not fold underneath the trunk.
            rootTip=-2.52f*flex*enemy_death::window(fall->tip,.23f,1);
            trunk.posedHip=trunk.posedHip+P3{0,hip.f,hip.z}*.5f;
        }
        creatureJoint(r,hip,knee,13,bark,.86f,material);
        creatureJoint(r,knee,foot,12,bark,.80f,material);
        for(int root=0;root<3;root++){
            const float length=10+root*2.f;
            P3 tip=foot+P3{side*(root-1)*7,length*std::cos(rootTip),-3-length*std::sin(rootTip)};
            r.tapered(foot,tip,5.1f,3.9f,shade(bark,1-root*.08f),.18f,material);
        }
    }
    if(fall){
        const float knees=(fall->nearKnee+fall->farKnee)*.5f;
        const float pitch=(9*knees+81*fall->tip-5*fall->recoil)*DEG2RAD;
        const float roll=fall->flank*(9*fall->tip+7*fall->recoil)*DEG2RAD;
        trunk.pitchSin=std::sin(pitch);trunk.pitchCos=std::cos(pitch);
        trunk.rollSin=std::sin(roll);trunk.rollCos=std::cos(roll);
        trunk.crownLag=fall->tip*(1-fall->tip);
        trunk.flank=fall->flank;
        r.setDeformation(fallenTreantTrunk,&trunk);
    }
    for(float side:{-1.f,1.f}){
        const auto key=side>0?action:forest_melee::treantSwing[0];
        P3 shoulder{side*16,lean+side*action.twist*.6f,70+z+m.gait.rise};
        const float brace=fall?fall->brace*(side==fall->flank?1.f:.70f):0;
        const auto arm=enemy_arm::solve({shoulder.r,shoulder.f,shoulder.z},
            {shoulder.r+side*(key.side+brace*13),shoulder.f+key.forward+side*m.gait.reach+brace*29,
             shoulder.z+key.up+brace*15},21,20);
        P3 elbow{arm.elbow.side,arm.elbow.forward,arm.elbow.up};
        P3 hand{arm.hand.side,arm.hand.forward,arm.hand.up};
        creatureJoint(r,shoulder,elbow,10,bark,.72f,material);
        creatureJoint(r,elbow,hand,8,bark,.72f,material);
        const P3 axis=(hand-elbow)*(1.f/20);
        for(int n=0;n<3;n++){
            P3 root=hand+P3{side*(n-1)*2.8f,0,0};
            r.tapered(root,root+axis*(9.f+n)+P3{side*(n-1)*3,2,0},2.7f,2.8f,bark,.26f,material);
        }
        surface(r,shoulder+P3{side*4.1f,-2.5f,4},{23,21,16},shade(leaf,.84f),16,flash);
        P3 bough{side*9,lean-3,95+z};
        P3 fork=bough+P3{side*8+m.gait.drift*.55f,-3,18};
        r.tapered(bough,fork,7,6,bark,.35f);
        r.spike(fork+P3{0,0,-7},fork+P3{side*7,0,6},4.3f,3.2f,bark);
    }
    surface(r,{0,lean*.37f,57+z},{31,26,46},bark,BarkMaterial,flash);
    P3 head{m.gait.twist*.48f+action.follow*.35f,lean+1,87+z+m.gait.rise};
    surface(r,head,{24.8f,23,27},bark,BarkMaterial,flash);
    for(float side:{-1.f,1.f}){
        P3 eye=head+P3{side*5.1f,10.8f,3};
        r.rounded(eye,{7.2f,2.9f,4.7f},dark);
        r.rounded(eye+P3{0,1.5f,0},{4.8f,1.4f,1.5f},ColorLerp(dark,Amber,glow));
        r.tapered(eye+P3{-side*2.4f,1.5f,1.6f},eye+P3{side*3.7f,-.9f,3.3f},1.5f,1.6f,shade(bark,.83f));
    }
    r.tube({head+P3{-5.7f,10.6f,-7.8f},head+P3{-.5f,11.5f,-8.9f},head+P3{5.7f,10.6f,-6.7f}},
           2,1.7f,dark);
    for(int i=0;i<7;i++){
        float wave=m.gait.drift+action.follow;
        float x=-21+i*7.f+wave*(.42f+i*.08f),f=-7+i%3*5.f+lean*.72f;
        float h=111+(i%2)*5.f+z+m.gait.rise*.9f+std::cos(i*2.f)*wave*.34f;
        surface(r,{x,f,h},{18+i%3*2.f,21+i%2*3.f,14+i%3*2.f},shade(leaf,.74f+i*.06f),16,flash);
        if(i%2==0)surface(r,{x-4,f-3,h+5},{12,15,9},shade(leaf,1.17f),16,flash);
    }
    for(int i=0;i<5;i++){
        float x=-8+i*4.f,f=8+i%2*1.5f;
        r.tube({{x,f,42+z},{x-2,f+3+lean*.28f,55+z},{x+1,f+lean*.38f,72+z}},
               .85f,.85f,shade(bark,.64f));
    }
    if(fall)r.setDeformation(nullptr,nullptr);
}
}
