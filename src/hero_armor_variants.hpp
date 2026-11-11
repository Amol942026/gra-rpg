#pragma once

// Worn surfaces share the hero's torso and arm transforms.
namespace hero_armor_variants {
struct Finish {Color shell,cloth,trim,accent;int cell;};
struct Cut {float width,depth,shoulder;};
inline constexpr std::array<Cut,AdditionalArmorCount> cuts{{
 {24,18,9}, {25,20,12}, {29,22,13}, {26,20,11}, {27,21,13},
 {28,22,13}, {26,20,11}, {29,22,14}, {28,22,14}, {29,23,14}
}};

inline void seam(Rig& rig,P3 a,P3 b,float width,Color color){
    rig.tapered(a,b,width,width,color);
}
inline void facet(Rig& rig,P3 a,P3 b,P3 c,Color color){
    rig.face(a,b,c,c,color);
    rig.face(c,b,a,a,color);
}
inline void layeredPlates(Rig& rig,int count,float sign,float depth,float width,const Finish& f){
    for(int row=0;row<count;row++){
        const float z=36.f+row*3.35f,y=(depth*.36f+row*.58f)*sign;
        const float w=width+row*.83f;
        rig.rounded({0,y,z},{w,4.2f,4.6f},shade(f.shell,.80f+row*.08f));
        for(float side:{-1.f,1.f})
            seam(rig,{0,y+2.f*sign,z-1.f},{side*w*.44f,y+.65f*sign,z+.3f},.85f,f.trim);
    }
}
inline void torso(Rig& rig,Kind kind,const Finish& f){
    const auto cut=cuts[int(kind)-FirstAdditionalArmor];
    rig.rounded({0,0,46.7f},{cut.width,cut.depth,28},f.shell,f.cell,false,f.cell,false,true);
    for(float facing:{1.f,-1.f}){
        const float front=facing*cut.depth*.5f;
        const P3 heart{0,front+facing*.8f,49};
        switch(kind){
        case Kind::BanditJack:
            seam(rig,{-8,front*.68f,56},{-3,front*.99f,51},3.6f,f.trim);
            seam(rig,{-3,front*.99f,51},{7,front*.77f,39},3.6f,f.trim);
            rig.rounded(heart+P3{-3,0,1.1f},{3.5f,1.8f,3.2f},f.accent);
            break;
        case Kind::SporeHarness:
            for(float side:{-1.f,1.f})for(int row=0;row<3;row++){
                float z=42+row*4.3f;
                rig.rounded({side*(4.f+row*.9f),front*.86f,z},{6.3f,3.1f,4.6f},shade(f.trim,.72f+row*.12f));
                rig.rounded({side*(4.f+row*.9f),front+facing*.1f,z+1},{1.9f,1.0f,1.7f},f.accent);
            }
            break;
        case Kind::GroveBarkArmor:
            for(float side:{-1.f,1.f}){
                P3 a{side*2,front,40},b{side*10.5f,front*.68f,47},c{side*6.3f,front*.73f,56};
                facet(rig,a,b,c,shade(f.shell,side>0?1.23f:.84f));
                seam(rig,b,c,1.55f,f.trim);
            }
            rig.rounded(heart,{4.7f,2.9f,6.6f},f.accent);
            break;
        case Kind::DuneLamellar:
            layeredPlates(rig,5,facing,cut.depth,16,f);
            for(float side:{-1.f,1.f})
                seam(rig,{side*4,front*.76f,55},{side*8,front*.86f,43},1.55f,f.accent);
            break;
        case Kind::ScorpionCarapace:
            layeredPlates(rig,4,facing,cut.depth,19,f);
            for(float side:{-1.f,1.f}){
                seam(rig,heart+P3{0,0,-2},{side*7,front*.91f,53},2.35f,f.trim);
                seam(rig,{side*7,front*.91f,53},{side*10,front*.61f,57},1.75f,f.trim);
            }
            break;
        case Kind::SunwardCuirass:
            for(int ray=0;ray<8;ray++){
                const float angle=ray*PI/4;
                const P3 radial{std::cos(angle),0,std::sin(angle)};
                seam(rig,heart+radial*2.2f,heart+radial*(ray%2?5.f:6.1f)+P3{0,-1.3f*facing,0},1.35f,f.trim);
            }
            rig.rounded(heart,{5.7f,2.2f,5.7f},f.trim);
            rig.rounded(heart+P3{0,1.05f*facing,0},{3.7f,1.9f,3.7f},f.accent);
            break;
        case Kind::AshBrigandine:
            for(float side:{-1.f,1.f})for(int row=0;row<4;row++){
                const float z=39+row*4.f,y=front*(.86f+std::min(row,2)*.045f);
                rig.rounded({side*4.5f,y,z},{1.8f,1.8f,1.8f},f.accent);
                rig.rounded({side*8.4f,y*.85f,z+1.1f},{1.65f,1.5f,1.65f},f.trim);
            }
            seam(rig,heart+P3{0,0,-9},heart+P3{0,-facing,7},1.28f,f.cloth);
            break;
        case Kind::CinderhornMail:
            layeredPlates(rig,4,facing,cut.depth,20,f);
            for(float side:{-1.f,1.f})
                seam(rig,{side*2,front,49},{side*7,front*.79f,55},2.1f,f.accent);
            break;
        case Kind::ObsidianPlate:
            for(float side:{-1.f,1.f}){
                const P3 a{side*11,front*.70f,50},b{side*6,front*.79f,57},c{0,front+facing,50},d{side*4.5f,front*.92f,40};
                facet(rig,a,b,c,shade(f.shell,side>0?1.38f:1.16f));
                facet(rig,a,c,d,shade(f.shell,side>0?.92f:1.14f));
                seam(rig,c,d,.8f,f.trim);
            }
            break;
        case Kind::RiftCarapace:
            for(float side:{-1.f,1.f})for(int row=0;row<3;row++){
                P3 rib=heart+P3{side*2,0,(row-1)*3.9f};
                P3 bend{side*6,front+.3f*facing,rib.z+1.8f},tip{side*(10+row*.4f),front*.69f,rib.z+4.f};
                seam(rig,rib,bend,2.f,f.trim);seam(rig,bend,tip,1.65f,shade(f.trim,.84f));
            }
            rig.form(heart+P3{0,.55f*facing,0},{4.7f,2.6f,7.5f},f.accent);
            break;
        default:break;
        }
    }
}
inline void shoulder(Rig& rig,Kind kind,P3 pivot,float side,const Finish& f){
    const float width=cuts[int(kind)-FirstAdditionalArmor].shoulder;
    const P3 p=pivot+P3{side*.95f,.25f,1.9f};
    const Color shell=kind==Kind::SporeHarness?f.trim:kind==Kind::AshBrigandine?f.cloth:f.shell;
    if(kind==Kind::ObsidianPlate){
        rig.form(p,{width,13.5f,8.4f},shell);
        seam(rig,p+P3{side*5,4.4f,0},p+P3{side,5.9f,3.8f},1.12f,f.trim);
    }else{
        rig.rounded(p,{width,kind==Kind::BanditJack?12.f:15.f,kind==Kind::BanditJack?5.f:8.f},
                    shell,f.cell,false,f.cell,false,true);
    }
    if(kind!=Kind::BanditJack)
        rig.rounded(p+P3{side*.8f,0,-3.1f},{width-.7f,13.5f,3.4f},shade(f.trim,.88f));
    if(kind==Kind::DuneLamellar||kind==Kind::ScorpionCarapace||kind==Kind::CinderhornMail)
        rig.rounded(p+P3{side*1.25f,0,-5.7f},{width-1.7f,11.1f,4.1f},f.shell);
    if(kind==Kind::CinderhornMail||kind==Kind::ScorpionCarapace||kind==Kind::RiftCarapace){
        const P3 base=p+P3{side*3,-.5f,2.5f},bend=p+P3{side*5.3f,-2,5.4f};
        const Color horn=kind==Kind::CinderhornMail?f.accent:f.trim;
        rig.tapered(base,bend,3.2f,2.9f,horn,.66f);
        rig.spike(bend,p+P3{side*5.1f,-4,8.3f},2.1f,1.9f,horn);
    }
}
}
