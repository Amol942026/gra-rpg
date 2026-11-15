#pragma once

namespace forest_boar_art {
inline constexpr Color Flank{115,86,65,255}, Shoulder{98,73,58,255}, Belly{119,93,73,255}, Bristle{61,49,41,255};
inline constexpr Color Muzzle{139,96,79,255}, Nose{146,99,89,255}, HoofColor{48,40,33,255}, Ivory{205,192,155,255};
inline Color finish(Color color,bool flash){return flash?Text:color;}

inline void trunk(Rig& rig,float cycle,float rush,float wag,float flick,bool flash){
    const Color flank=finish(Flank,flash), shoulder=finish(Shoulder,flash);
    const Color belly=finish(Belly,flash),bristle=finish(Bristle,flash);
    forest_detail::surface(rig,{0,-5,-.4f},{27.6f,35.5f,23.2f},flank,4,flash);
    forest_detail::surface(rig,{0,-16,-1.f},{22.4f,22,21},flank,4,flash);
    forest_detail::surface(rig,{0,9,3.7f},{31,28,30.4f},shoulder,4,flash);
    forest_detail::surface(rig,{0,17,-2},{23.4f,16.5f,21.5f},shoulder,4,flash);
    forest_detail::surface(rig,{0,0,-8},{22,28,10.7f},belly,4,flash);
    // Short overlapping guard hairs are tallest above the forequarters.
    for(int i=0;i<14;i++){
        const float f=-24.f+i*3.25f;
        const float ridge=7.5f+10.5f*std::exp(-(f-7.f)*(f-7.f)*.006f);
        const float height=2.3f+2.9f*std::exp(-(f-6.f)*(f-6.f)*.004f);
        const Color tuft=shade(bristle,1.12f+(i%4)*.062f);
        const float sway=std::sin(cycle+i*.28f)*(.24f+rush*.5f);
        rig.spike({sway*.3f,f,ridge},{sway,f-2.7f,ridge+height},3.3f,4.8f,tuft);
        for(float side:{-1.f,1.f}){
            const float x=side*(3.3f+(i%3)*.42f);
            rig.spike({x,f+.6f,ridge-.9f},{x+sway+side*.8f,f-3.0f,ridge+height*.52f},3.2f,3.5f,shade(tuft,1.16f));
        }
    }
    rig.tapered({0,-26,1.7f},{wag,-29,2.8f+flick},2.9f,2.9f,flank,.72f);
    rig.tapered({wag,-29,2.8f+flick},{wag+.8f,-34,1.0f+flick},2.18f,2.23f,flank,.66f);
    rig.spike({wag+.8f,-33.2f,1.3f+flick},{wag+1.7f,-38,.2f+flick},3.5f,2.8f,bristle);

}

inline void head(Rig& rig,float earLag,bool flash,bool eyesClosed=false){
    const Color shoulder=finish(Shoulder,flash),bristle=finish(Bristle,flash);
    const Color muzzle=finish(Muzzle,flash),nose=finish(Nose,flash);
    const Color hoofColor=finish(HoofColor,flash), ivory=finish(Ivory,flash);
    const Color cheek=shade(shoulder,1.13f), gum=shade(muzzle,.44f);
    forest_detail::surface(rig,{0,.6f,0.5f},{21,20.8f,19.8f},shoulder,4,flash);
    forest_detail::surface(rig,{0,7.4f,-2.7f},{17,17.1f,12.4f},cheek,4,flash);
    forest_detail::surface(rig,{0,13.7f,-5.f},{14.6f,13.7f,9.3f},muzzle,14,flash);
    rig.rounded({0,12.65f,-8.3f},{13.6f,11,4.8f},shade(bristle,1.23f));
    rig.rounded({0,19.1f,-5.4f},{14.6f,4.3f,8.5f},nose);
    for(float side:{-1.f,1.f}){
        const P3 eye{side*7.9f,6.0f,2.6f};
        if(eyesClosed){
            const P3 lid=eye+P3{side*1.45f,.42f,-.32f};
            rig.tapered(lid+P3{0,-1.8f,.4f},lid+P3{side*.3f,1.55f,-.28f},.80f,.79f,hoofColor,.76f);
        }else{
            rig.rounded(eye,{4.5f,4.2f,3.2f},bristle);
            const P3 iris=eye+P3{side*1.6f,.7f,-.13f};
            rig.rounded(iris,{1.1f,1.9f,1.55f},flash?Text:rgb(162,114,60));
            rig.rounded(iris+P3{side*.53f,.22f,-.2f},{.55f,.75f,.9f},hoofColor);
        }
        rig.tapered(eye+P3{side*.7f,-2.8f,1.6f},eye+P3{side*.3f,2.4f,.7f},1.8f,1.5f,shade(shoulder,.83f),.7f);

        const P3 ear{side*6.7f,-4.3f,5.5f}, point{side*9.9f,-5.55f+earLag,15.0f+earLag*.7f};
        rig.spike(ear,point,8.7f,5.0f,bristle);
        rig.spike(ear+P3{side*.48f,1.25f,.75f},point+P3{-side*.55f,1.1f,-2.8f},4.55f,2.5f,shade(nose,.86f));
        rig.rounded({side*3.4f,20.95f,-5.0f},{4.01f,1.3f,2.4f},gum);
        rig.tapered({side*4.9f,13.8f,-8.6f},{side*6.8f,8.6f,-7.4f},.88f,.82f,gum);

        const P3 root{side*5.6f,9.5f,-8.52f};
        const P3 curve{side*8.8f,12.25f,-7.0f}, bend{side*10.25f,14.72f,-4.12f};
        const P3 tip{side*10.6f,16.62f,.75f};
        rig.tapered(root,curve,4.1f,3.8f,shade(ivory,.85f),.83f);
        rig.tapered(curve,bend,3.4f,3.16f,ivory,.61f);
        rig.spike(bend,tip,2.4f,2.15f,shade(ivory,1.09f));
    }
}

struct Stance {
    forest_boar_motion::LegPose limbs;
    hero_motion::Transform body,head;
    float cycle,rush,wag,flick,earLag;
};
inline Stance stance(const AnimationState& state,float walk,float attack,float action){
    using namespace forest_boar_motion;
    // Standalone poses use the same contact and recovery ranges as normal melee.
    const float contact=meleeWindup(BoarType)/(meleeWindup(BoarType)+meleeRecovery(BoarType));
    const auto pose=phase(action>=0?action:attack<0?-attack*contact:attack>0?contact+(1-attack)*(1-contact):0);
    const auto key=sample(pose);
    const float rush=gallopBlend(pose);
    if(pose.action==Action::Windup)walk*=1-blend(pose.progress/.25f);
    if(pose.action==Action::Recovery)walk*=1-blend(pose.progress/.22f);
    const float cycle=state.phase*2*PI;
    const auto limbPose=legs(state,pose,walk,rush,key);
    const float spine=limbPose.pitch;
    const auto bodyRotation=hero_motion::axisAngle({1,0,0},spine);
    const hero_motion::Transform bodyMount{{0,key.lean,limbPose.center},bodyRotation};
    auto bridge=bodyMount.p+hero_motion::rotate(bodyRotation,{0,21.f+key.reach,4.6f});
    const hero_motion::Transform headMount{bridge,
        hero_motion::axisAngle({1,0,0},spine+key.headPitch+walk*std::sin(cycle-.6f)*1.4f)};
    const float wag=std::sin(state.idleTime*4.5f+cycle)*(.65f+walk*1.2f);
    const float earLag=std::sin(cycle-.9f)*(.31f+rush*.8f)-key.headPitch*.034f;
    return {limbPose,bodyMount,headMount,cycle,rush,wag,key.tail*.25f,earLag};
}

inline void draw(Rig& rig,const AnimationState& state,float walk,float attack,float action,bool flash){
    using namespace forest_boar_motion;
    const auto look=stance(state,walk,attack,action);
    const Color flank=finish(Flank,flash),bristle=finish(Bristle,flash),hoofColor=finish(HoofColor,flash);

    // Elbows fold behind the forelegs. Hind stifles fold forward while
    // the low hocks bear the weight through the short hind cannon bones.
    for(const auto& limb:look.limbs.legs){
        const bool front=limb.front;
        const auto point=[side=limb.side](Joint offset){return P3{side,offset.forward,offset.height};};
        const P3 hip=point(limb.hip), hinge=point(limb.knee), hock=point(limb.hock), foot=point(limb.foot);
        const float width=front?7.4f:9.65f;
        const auto upper=shade(flank,front?.92f:1.f);
        const auto shin=shade(bristle,1.3f);
        const auto lower=shade(bristle,1.15f);
        rig.tapered(hip,hinge,width,width*1.06f,upper,.66f,flash?-1:4);
        rig.rounded(hinge,{width*.63f,width*.69f,width*.64f},upper);
        rig.tapered(hinge,hock,front?4.6f:5.25f,front?4.7f:5.2f,shin,.71f);
        rig.rounded(hock,{3.65f,4.05f,3.85f},shin);
        rig.tapered(hock,foot,3.65f,4.05f,lower,.97f);
        const hero_motion::Transform hoofMount{{foot.r,foot.f,foot.z},
            hero_motion::axisAngle({1,0,0},limb.rock)};
        rig.setAttachment(&hoofMount);
        rig.rounded({0,.45f,.6f},{4.8f,4.1f,3.3f},lower);
        for(float split:{-1.f,1.f}){
            const P3 toe{split*1.2f,1.18f,-.49f};
            rig.rounded(toe,{2.3f,5.55f,3.9f},hoofColor);
            rig.tapered(toe+P3{split*.60f,1,1.f},toe+P3{split*.60f,3.1f,.37f},.59f,.48f,shade(hoofColor,1.36f));
        }
        rig.setAttachment(nullptr);
    }

    rig.setAttachment(&look.body);
    trunk(rig,look.cycle,look.rush,look.wag,look.flick,flash);

    rig.setAttachment(&look.head);
    head(rig,look.earLag,flash);
    rig.setAttachment(nullptr);
}
}
