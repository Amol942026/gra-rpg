#pragma once

// Included inside actor_art::forest_detail after the shared material layer helper.
namespace wolf_art {
inline constexpr Color Coat{108,114,106,255}, Silver{174,174,153,255};
inline constexpr Color Charcoal{64,66,60,255}, Umber{124,111,86,255}, Lining{112,81,66,255};

inline float mix(float a,float b,float t){return a+(b-a)*t;}

template<size_t N>
inline wolf_motion::Key sample(const wolf_motion::Key (&track)[N],float phase){
    const float frame=std::clamp(phase,0.f,1.f)*(N-1);
    const auto index=std::min(size_t(frame),N-2);
    const float t=frame-index;
    const auto a=track[index],b=track[index+1];
    return {mix(a.chest,b.chest,t),mix(a.hip,b.hip,t),mix(a.reach,b.reach,t),
            mix(a.lift,b.lift,t),mix(a.pitch,b.pitch,t),mix(a.jaw,b.jaw,t),
            mix(a.tail,b.tail,t),mix(a.ears,b.ears,t),mix(a.stride,b.stride,t),
            mix(a.clearance,b.clearance,t),mix(a.weight,b.weight,t),mix(a.sway,b.sway,t)};
}

struct Rib {float forward,height,width,depth;};
inline void coat(Rig& rig,float hip,float chest,float weight,float sway,bool flash){
    constexpr int Edges=16;
    static const auto ring=[]{
        std::array<P3,Edges> circle{};
        for(int i=0;i<Edges;i++){
            const float angle=i*2*PI/Edges;
            circle[i]={std::sin(angle),0,std::cos(angle)};
        }
        return circle;
    }();
    const std::array<Rib,9> silhouette{{
        {-27,hip+.3f,0,0},{-22,hip+.6f,7.8f,7.9f},{-13,hip+.65f,10.2f,10.3f},
        {-5,(hip+chest)*.5f+2.2f,8.7f,8.1f},{4,chest-.65f,10.8f,11.3f},
        {13,chest+.2f,11.7f,12.6f},{20,chest+3.8f,10.1f,11.9f},
        {25,chest+7.1f,7.8f,9.1f},{30,chest+7.8f,0,0}
    }};
    const auto point=[&](const Rib& rib,int index){
        const P3 a=ring[index];
        return P3{a.r*rib.width+sway,rib.forward+weight,rib.height+a.z*rib.depth};
    };
    const auto tint=[&](int section,int index){
        if(flash)return Text;
        const P3 n=ring[index];
        const float side=1-std::abs(n.z);
        const Color base=ColorLerp(Coat,Umber,.25f*side);
        const float flank=std::clamp((.12f-n.z)*.91f,0.f,1.f);
        const Color pelage=ColorLerp(base,section>=6?Silver:shade(Silver,.86f),flank);
        const Color back=ColorLerp(pelage,Charcoal,std::max(0.f,n.z)*.38f);
        return shade(back,rig.surfaceLight(n));
    };
    for(int row=0;row<int(silhouette.size())-1;row++)for(int edge=0;edge<Edges;edge++){
        const int next=(edge+1)%Edges;
        const auto& tail=silhouette[row];
        const auto& head=silhouette[row+1];
        const P3 a=point(tail,edge),b=point(tail,next),c=point(head,next),d=point(head,edge);
        rig.surfaceQuad(a,b,c,d,tint(row,edge),tint(row,next),tint(row+1,next),tint(row+1,edge));
        if(!flash){
            const int tile=ring[edge].z+ring[next].z<-.30f?13:1;
            const float light=.63f*rig.surfaceLight((ring[edge]+ring[next])*.5f);
            rig.materialQuad(a,b,c,d,tile,edge/float(Edges),row/8.f,(edge+1)/float(Edges),(row+1)/8.f,shade(WHITE,light));
        }
    }
    // Long guard hairs run back from the withers and melt into the ruff.
    for(float side:{-1.f,1.f})for(int i=0;i<4;i++){
        const P3 base{side*(6.8f+i*.72f)+sway,22.f+weight-i*2.6f,chest+12.9f-i*1.9f};
        const P3 point=base+P3{side*(1.05f+i*.35f),-5.3f-i*.45f,-2.9f};
        rig.spike(base,point,3.6f,3.4f,flash?Text:shade(i<2?Coat:Umber,.91f+i*.04f));
    }
}

struct Leg {P3 root,joint,wrist,paw;float pitch;};
inline Leg legPose(const AnimationState& state,float walk,float hip,float chest,float weight,
                   const wolf_motion::Key& action,bool front,bool right){
    const float side=right?1.f:-1.f,base=front?14.f:-16.5f;
    const float lateral=side*(front?7.5f:7.1f);
    auto step=forest_gait::wolfFoot(state.phase,walk,right,front);
    if(front){
        step.forward+=action.stride*(right?.88f:1.f);
        step.height+=action.clearance*(right?.70f:1.f);
    }
    const auto leg=forest_gait::solveWolfLeg(step,front?chest:hip,weight,front);
    const auto point=[&](grounded_gait::Knee p){return P3{lateral,base+p.forward,p.height};};
    return {point(leg.root),point(leg.joint),point(leg.wrist),{lateral,base+step.forward,step.height},step.pitch};
}

inline void limbs(Rig& rig,const AnimationState& state,float walk,float hip,float chest,
                  float weight,const wolf_motion::Key& action,bool flash){
    const Color muscle=flash?Text:shade(Coat,.90f);
    const Color paws=flash?Text:shade(Charcoal,.96f);
    const int material=flash?-1:1;
    for(bool front:{false,true})for(bool right:{false,true}){
        const auto [root,joint,wrist,paw,pitch]=legPose(state,walk,hip,chest,weight,action,front,right);
        // Keep the elbow and wrist in the paw's fore-aft plane. The long
        // forearm reads as one continuous silhouette below the coat.
        rig.tapered(root,joint,front?7.7f:10.7f,front?8.5f:11.8f,muscle,front?.66f:.52f,material);
        rig.rounded(joint,{front?4.9f:5.45f,5.3f,5.6f},muscle,flash?-1:101,false,-1,true);
        rig.tapered(joint,wrist,front?4.8f:5.1f,front?5.2f:5.5f,shade(muscle,.88f),.69f,material);
        rig.rounded(wrist,{3.45f,3.7f,3.5f},shade(muscle,.85f));
        rig.tapered(wrist,paw,3.4f,3.8f,shade(muscle,.85f),1.06f,material);

        const hero_motion::Transform toePose{{paw.r,paw.f,paw.z},hero_motion::axisAngle({1,0,0},pitch)};
        rig.setAttachment(&toePose);
        rig.rounded({0,1.15f,-.5f},{5.85f,7.65f,4.15f},paws);
        for(int toe=-1;toe<=1;toe++){
            const P3 tip{1.48f*toe,3.65f-.30f*std::abs(toe),-1.1f};
            rig.rounded(tip,{1.95f,2.9f,2.0f},shade(paws,1.18f));
        }
        rig.setAttachment(nullptr);
    }
}

inline void face(Rig& rig,P3 pivot,float pitch,float mouth,float ears,bool flash,bool eyesClosed=false){
    const float c=std::cos(pitch*DEG2RAD),s=std::sin(pitch*DEG2RAD);
    const auto p=[&](float x,float y,float z){return pivot+P3{x,c*y-s*z,s*y+c*z};};
    const auto tone=[flash](Color color){return flash?Text:color;};
    const Color tip=tone(Charcoal),under=tone(Silver),fur=tone(Coat);
    surface(rig,p(0,-.8f,0),{15.6f,16.5f,15.8f},Coat,1,flash);
    surface(rig,p(0,2.8f,-3.9f),{15.8f,13.6f,9.3f},Silver,13,flash);
    rig.tapered(p(0,4.2f,-1.2f),p(0,16.3f,-4.7f),11.8f,10.0f,under,.57f,flash?-1:13);
    rig.tapered(p(0,5.3f,1.9f),p(0,14.8f,-2.8f),7.1f,4.f,fur,.68f,flash?-1:1);
    rig.rounded(p(0,16.95f,-3.0f),{6.2f,4.0f,4.2f},shade(tip,.52f));
    rig.rounded(p(-.95f,18.1f,-2.25f),{1.9f,1.45f,1.3f},shade(tip,1.22f));

    const float open=mouth*DEG2RAD;
    const auto jaw=[&](float x,float y,float z){
        const float f=y-4.f,h=z+5.2f;
        return p(x,4.f+std::cos(open)*f+std::sin(open)*h,-5.2f-std::sin(open)*f+std::cos(open)*h);
    };
    rig.tapered(jaw(0,4,-5.5f),jaw(0,15.7f,-6.3f),8.3f,4.1f,under,.68f,flash?-1:13);
    rig.tapered(jaw(0,6,-4.55f),jaw(0,15.3f,-5.25f),7.6f,1.25f,shade(tip,.53f),.66f);
    if(mouth>8.f)rig.rounded(jaw(0,11,-4.1f),{5.f,6.9f,1.55f},tone(rgb(125,69,63)));

    for(float side:{-1.f,1.f}){
        surface(rig,p(side*5.9f,-1.5f,-.9f),{6.7f,13.9f,8.5f},Silver,13,flash);
        const P3 ear=p(side*5.5f,-4.6f,4.2f);
        const P3 end=p(side*(6.2f-ears*.055f),-4.5f+ears*.26f,15.5f+ears*.18f);
        rig.spike(ear,end,7.1f,5.8f,shade(tip,1.16f));
        rig.spike(ear+P3{0,2.1f,1.0f},end+P3{0,.6f,-2.7f},4.45f,2.7f,tone(Lining));
        rig.tapered(p(side*4.7f,-1.5f,5.1f),p(side*7,3.8f,2.7f),3.5f,2.8f,shade(fur,.75f),.67f);
        if(eyesClosed){
            rig.tapered(p(side*7.3f,4.6f,2.36f),p(side*7.45f,6.48f,1.9f),.92f,.73f,shade(tip,.52f),.74f);
        }else{
            const P3 socket=p(side*6.4f,5.15f,2.0f);
            rig.rounded(socket,{2.7f,3.7f,2.6f},shade(tip,.56f));
            rig.rounded(p(side*7.05f,5.95f,2.13f),{1.6f,2.25f,1.38f},tone(rgb(184,137,71)));
            rig.rounded(p(side*7.4f,6.32f,2.13f),{.8f,1.22f,.85f},shade(tip,.46f));
            rig.rounded(p(side*7.42f,6.54f,2.49f),{.8f,.64f,.70f},tone(rgb(228,199,136)));
        }
        rig.tapered(p(side*5.22f,6.1f,-5.05f),p(side*3.1f,15.25f,-6.0f),.8f,.85f,shade(tip,.69f),.65f);
        if(mouth>5.f){
            const float tooth=std::clamp((mouth-5)/9,0.f,1.f);
            rig.spike(p(side*3.9f,11.65f,-5.9f),p(side*3.45f,11.4f,-5.9f-tooth*2.4f),1.65f,1.35f,under);
        }
        for(int i=0;i<3;i++){
            const P3 lock=p(side*(6.7f-i*.16f),-.3f-i*1.9f,-3.2f-i*.9f);
            rig.spike(lock,lock+P3{side*(2.2f-i*.25f),-3.6f-i*.75f,-3.7f},3.9f,3.85f,shade(under,.75f+i*.055f));
        }
    }
}

struct Stance {
    wolf_motion::Key action;
    float hip,chest,weight,sway,pitch,jaw,ears;
    P3 head,root,curve,brush,end,tailTip;
};
inline Stance stance(const AnimationState& state,float walk,float phase){
    const auto gait=sample(wolf_motion::walk,state.phase);
    const auto action=phase>=0?sample(wolf_motion::bite,phase):wolf_motion::Key{};
    const float idle=std::sin(state.idleTime*2.05f)*(1-walk);
    const float hip=25.7f+gait.hip*walk+action.hip+idle*.20f;
    const float chest=28.1f+gait.chest*walk+action.chest+idle*.50f;
    const float weight=action.weight,sway=gait.sway*walk+action.sway;
    const P3 head{sway*.79f,25.4f+weight*.7f+gait.reach*walk+action.reach,
                  chest+10.9f+idle*.23f+gait.lift*walk+action.lift};

    const float tail=idle*1.6f+gait.tail*walk+action.tail;
    const P3 root{sway,-24+weight,hip+3};
    const P3 curve{tail*.38f,-31+weight,hip-.8f+action.tail*.40f};
    const P3 brush{tail,-39+weight,hip-5.8f+action.tail*.63f};
    const P3 end{tail*1.30f,-48+weight,hip-11.3f+action.tail*.8f};
    return {action,hip,chest,weight,sway,gait.pitch*walk+action.pitch,action.jaw,action.ears+gait.ears*walk,
            head,root,curve,brush,end,end+P3{tail*.17f,-3.7f,-1.6f}};
}

inline void draw(Rig& rig,const AnimationState& state,float walk,float phase,bool flash){
    const auto look=stance(state,walk,phase);
    limbs(rig,state,walk,look.hip,look.chest,look.weight,look.action,flash);
    coat(rig,look.hip,look.chest,look.weight,look.sway,flash);
    face(rig,look.head,look.pitch,look.jaw,look.ears,flash);
    const Color fur=flash?Text:Charcoal;
    const int paint=flash?-1:1;
    rig.tapered(look.root,look.curve,6.65f,7.55f,shade(fur,1.18f),1.11f,paint);
    rig.tapered(look.curve,look.brush,7.38f,8.4f,shade(fur,1.12f),.88f,paint);
    rig.rounded(look.brush,{6.5f,7.8f,6.4f},shade(fur,1.06f),flash?-1:101,false,-1,true);
    rig.tapered(look.brush,look.end,6.5f,7.35f,fur,.29f,paint);
    rig.spike(look.end,look.tailTip,2.5f,2.5f,fur);
}
}
