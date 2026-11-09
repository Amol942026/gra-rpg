#pragma once

namespace forest_detail {
inline constexpr Color Gel{84,143,83,255}, Fur{112,121,116,255}, Ruff{176,178,155,255};
inline constexpr Color Bone{197,181,141,255}, Bark{98,74,48,255}, Leaf{80,114,61,255};
inline constexpr Color Amber{236,178,77,255};

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

inline void wolf(Rig& r,const AnimationState& s,float walk,float hit,bool flash){
    auto m=motion(forest_motion::wolf,s,walk,hit,.48f);
    float z=26.3f+m.gait.rise+m.action.rise*.25f;
    float lean=m.gait.lean+m.action.lean;
    Color fur=flash?Text:Fur,ruff=flash?Text:Ruff,dark=shade(fur,.60f);
    for(bool front:{false,true})for(bool right:{false,true}){
        float side=right?1.f:-1.f,base=front?14.f:-15.f,hip=z+(front?1.f:-1.f);
        const auto leg=forest_gait::wolfLeg(s.phase,walk,right,front,hip);
        P3 knee{side*7.8f,base+leg.joint.forward,leg.joint.height};
        P3 foot{side*7.8f,base+leg.forward,leg.height};
        creatureJoint(r,{side*7.8f,base,hip},knee,front?7.3f:9.f,shade(fur,.88f),.70f);
        creatureJoint(r,knee,foot,front?4.8f:4.3f,dark,.71f);
        r.rounded(foot+P3{0,2.5f,-.6f},{6.8f,10,4},dark);
        for(float toe:{-1.45f,1.45f})r.tapered(foot+P3{toe,5,0},foot+P3{toe,6.3f,-.65f},.65f,.6f,ruff);
    }
    surface(r,{0,-4,z},{22,43,21+12*m.gait.squash},fur,1,flash);
    surface(r,{0,12+lean*.23f,z+4},{25,25,27},fur,1,flash);
    surface(r,{0,19+lean*.35f,z+1},{21,19,19},ruff,13,flash);
    P3 head{m.gait.twist*.60f,25+lean+m.action.reach*.40f,z+16+m.action.rise};
    surface(r,head,{17.5f,20.5f,18},fur,13,flash);
    P3 muzzle=head+P3{0,12,-4};
    surface(r,muzzle,{11.5f,18,9},ruff,13,flash);
    r.rounded(muzzle+P3{0,8.4f,1.7f},{8.2f,4,5},shade(dark,.62f));
    float jaw=std::max(0.f,m.action.reach)*.59f;
    r.rounded(muzzle+P3{0,1.4f,-3-jaw*.45f},{9.6f,15.8f,5.5f},dark);
    r.rounded(muzzle+P3{0,4,-4.5f-jaw},{8.5f,13,4},ruff);
    for(float side:{-1.f,1.f}){
        creatureEye(r,head+P3{side*5.7f,8.1f,2.1f},2.4f,Amber);
        P3 ear=head+P3{side*6.6f,-4.5f,6.2f};
        P3 tip=head+P3{side*8.5f,-5.1f+lean*.25f,20+m.gait.twist*.5f};
        r.spike(ear,tip,8.5f,5.8f,dark);
        r.spike(ear+P3{0,2,1},tip+P3{0,.4f,-3},4.1f,2,flash?Text:rgb(142,121,110));
        r.spike(muzzle+P3{side*4.3f,4.6f,-2},muzzle+P3{side*4,5,-4-jaw*.75f},1.4f,1.4f,ruff);
        for(int i=0;i<3;i++){
            P3 root=head+P3{side*7.2f,-2-i*3.f,-3-i*.45f};
            r.spike(root,root+P3{side*2.4f,-3.6f,-5.2f},4.2f,4.1f,shade(ruff,.90f+i*.025f));
        }
    }
    float wag=m.gait.drift+m.action.drift;
    r.tube({{0,-23,z+1},{wag*.55f,-31,z-2},{wag,-40,z-7},{wag*1.5f,-46,z-10}},6.4f,7.8f,dark);
    r.spike({wag*1.5f,-45,z-9},{wag*1.7f,-50,z-9},5.1f,5.8f,dark);
}

inline void skeleton(Rig& r,const AnimationState& s,float walk,float hit,bool flash){
    auto m=motion(forest_motion::skeleton,s,walk,hit,.37f);
    auto gait=grounded_gait::fromStanding(s,walk,forest_gait::Skeleton,-std::abs(m.action.squash)*16);
    float z=gait.hip+m.gait.rise,lean=m.gait.lean+m.action.lean;
    Color bone=flash?Text:Bone,dark=shade(bone,.62f),iron=flash?Text:rgb(128,140,128);
    Color rust=flash?Text:rgb(131,89,54);
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
    }
    surface(r,{0,0,gait.hip},{19.2f,12,8},bone,2,flash);
    r.rounded({0,6.1f,gait.hip+.4f},{7,2.6f,4.1f},dark);
    for(int i=0;i<7;i++){
        float shift=i*lean/7;
        r.rounded({0,-3+shift,z+2+i*3.5f},{4.4f,5.4f,3.65f},shade(bone,.86f+i*.021f));
    }
    for(int j=0;j<5;j++)for(float side:{-1.f,1.f}){
        float rib=z+6+j*3.55f,depth=lean*(j+2)*.16f,w=9.7f-j*.62f;
        r.tube({{side*1.9f,-3+depth,rib},{side*w,1+depth,rib+1.35f},
                {side*(w+.8f),5+depth,rib},{side*5.8f,8+depth,rib-1.75f},{side*1.8f,8+depth,rib-2.1f}},
               1.7f,1.9f,shade(bone,.83f+j*.065f));
    }
    r.tapered({0,8+lean*.18f,z+6},{0,8+lean,z+21},2.4f,1.9f,bone);
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
        }
    }
    P3 skull{m.gait.twist*.50f,lean,z+35.6f+m.action.rise*.12f};
    surface(r,skull,{17.7f,16.7f,17.2f},bone,2,flash);
    float jaw=std::max(0.f,m.action.rise)*.22f;
    r.rounded(skull+P3{0,3,-6.8f-jaw},{13.5f,10.2f,4.6f},bone);
    r.rounded(skull+P3{0,8,-5.6f-jaw*.6f},{10.4f,1.5f,2.3f},shade(dark,.57f));
    for(float side:{-1.f,1.f}){
        r.rounded(skull+P3{side*4.5f,7.1f,1.2f},{6.3f,4.2f,4.8f},rgb(48,46,35));
        r.rounded(skull+P3{side*4.5f,9.1f,.9f},{2.15f,1.4f,1.7f},Amber);
    }
    for(int i=0;i<5;i++)r.rounded(skull+P3{i*1.9f-3.8f,8.8f,-5.1f},{1.28f,1.5f,2.5f},shade(bone,1.15f));
    r.rounded(skull+P3{0,8.5f,-2.2f},{2.6f,1.6f,3.4f},shade(dark,.72f));
}

inline void treant(Rig& r,const AnimationState& s,float walk,float hit,bool flash){
    auto m=motion(forest_motion::treant,s,walk,hit,.32f);
    const auto gait=grounded_gait::pose(s,walk,treant_gait::Contact);
    float z=gait.hip-40,lean=m.gait.lean+m.action.lean;
    Color bark=flash?Text:Bark,leaf=flash?Text:Leaf,dark=shade(bark,.53f);
    for(float side:{-1.f,1.f}){
        const auto& leg=gait.legs[side>0];
        P3 foot{side*12,leg.forward,leg.height},knee{side*10,leg.joint.forward,leg.joint.height};
        creatureJoint(r,{side*8,0,gait.hip},knee,13,bark,.86f);
        creatureJoint(r,knee,foot,12,bark,.80f);
        for(int root=0;root<3;root++){
            P3 tip=foot+P3{side*(root-1)*7,10+root*2.f,-3};
            r.tapered(foot,tip,5.1f,3.9f,shade(bark,1-root*.08f),.18f);
        }
        float reach=m.action.reach*(side>0?1:.55f)+side*m.gait.reach;
        P3 shoulder{side*16,lean*.45f,70+z+m.gait.rise};
        P3 elbow{side*26,3+reach*.44f,56+z+m.action.rise*.44f};
        P3 hand{side*31,5+reach,41+z+m.action.rise};
        creatureJoint(r,shoulder,elbow,10,bark);
        creatureJoint(r,elbow,hand,8,bark);
        for(int n=0;n<3;n++){
            P3 root=hand+P3{side*(n-1)*2.8f,0,0};
            r.tapered(root,root+P3{side*(n-1)*3,6,-7.f+n*.9f},2.7f,2.8f,bark,.26f);
        }
        surface(r,shoulder+P3{side*4.1f,-2.5f,4},{23,21,16},shade(leaf,.84f),16,flash);
        P3 bough{side*9,lean-3,95+z};
        P3 fork=bough+P3{side*8+m.gait.drift*.55f,-3,18};
        r.tapered(bough,fork,7,6,bark,.35f);
        r.spike(fork+P3{0,0,-7},fork+P3{side*7,0,6},4.3f,3.2f,bark);
    }
    surface(r,{0,lean*.37f,57+z},{31,26,46},bark,7,flash);
    P3 head{m.gait.twist*.48f,lean+1,87+z+m.gait.rise};
    surface(r,head,{24.8f,23,27},bark,7,flash);
    for(float side:{-1.f,1.f}){
        P3 eye=head+P3{side*5.1f,10.8f,3};
        r.rounded(eye,{7.2f,2.9f,4.7f},dark);
        r.rounded(eye+P3{0,1.5f,0},{4.8f,1.4f,1.5f},Amber);
        r.tapered(eye+P3{-side*2.4f,1.5f,1.6f},eye+P3{side*3.7f,-.9f,3.3f},1.5f,1.6f,shade(bark,.83f));
    }
    r.tube({head+P3{-5.7f,10.6f,-7.8f},head+P3{-.5f,11.5f,-8.9f},head+P3{5.7f,10.6f,-6.7f}},
           2,1.7f,dark);
    for(int i=0;i<7;i++){
        float wave=m.gait.drift+m.action.drift*.8f;
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
}
}
