#pragma once
#include "hero_carry.hpp"
#include "hero_quiver.hpp"
#include "generated/hero_bow_data.hpp"
#include "generated/hero_bow_nock_data.hpp"

namespace ember::hero_bow {
using hero_motion::Vec;

template<class Key,size_t N>
inline Vec sample(const Key (&keys)[N],Vec Key::* channel,float phase){
    size_t end=1;
    while(end<N-1&&phase>keys[end].t)++end;
    const auto &from=keys[end-1],&to=keys[end];
    const float t=std::clamp((phase-from.t)/(to.t-from.t),0.f,1.f);
    return from.*channel*(1-t)+to.*channel*t;
}

struct Shot {
    float tension,lift,stringContact;
    Vec grip,nock;
    float preparation=-1;
    Vec arrowAxis{};
    float arrowGripOffset=0;
};

inline Shot sampleShot(float visibleCharge,float release,float preparation=1.f){
    using hero_motion::ease;
    using hero_quiver::mirror;
    if(visibleCharge<0&&!(preparation>=0&&preparation<1))return {};
    if(preparation>=0&&preparation<1&&release<0){
        constexpr int last=int(std::size(data::prepare))-1;
        const float frame=preparation*last;
        const int index=std::min(int(frame),last-1);
        const float t=frame-index;
        const auto &a=data::prepare[index],&b=data::prepare[index+1];
        Vec axis=a.axis*(1-t)+b.axis*t;
        axis=axis*(1/hero_motion::length(axis));
        return {0,ease(preparation/.24f),0,mirror(a.grip*(1-t)+b.grip*t),mirror(a.hand*(1-t)+b.hand*t),
                preparation,mirror(axis),a.gripOffset*(1-t)+b.gripOffset*t};
    }
    // The game keeps remaining draw for the HUD. The arms follow the tension
    // actually released, then the fingers travel back separately from the cord.
    const float t=std::clamp(visibleCharge/(release<0?1.f:std::max(.0001f,1-release)),0.f,1.f);
    const float lift=1-ease((release-.22f)/.78f);
    auto grip=sample(data::draw,&data::DrawKey::grip,t);
    auto nock=sample(data::draw,&data::DrawKey::nock,t);
    if(release>=0)nock=nock+sample(data::loose,&data::LooseKey::delta,release)*t;
    const float fingers=ease((lift-.18f)/.46f)*(1-ease(release/.17f));
    return {t,lift,fingers,mirror(grip),mirror(nock),1,{0,1,0},0};
}

inline void apply(hero_motion::Pose& pose,const Shot& shot,float brace=11.f){
    using namespace hero_motion;
    namespace bone=hero_motion::data;
    if(shot.lift<=0)return;
    // Skin coordinates before opening the shoulders preserve a moving or
    // recoiling actor's original aim plane. The neck continues facing the shot.
    const bool preparing=shot.preparation>=0&&shot.preparation<1;
    const Vec grip=pose.point(bone::Spine,shot.grip);
    const Vec nock=pose.point(bone::Spine,shot.nock);
    const Quat aim=pose.skin[bone::Spine].q;
    const Vec up=rotate(aim,{0,0,1});
    const float turn=preparing?ease((shot.preparation-.59f)/.41f):1.f;
    const float twist=(27.f+18.f*shot.tension)*shot.lift*turn;
    bend(pose,bone::Spine,axisAngle(up,twist));
    bend(pose,bone::Head,axisAngle(up,-twist));
    pose.bind();
    for(int hand:{hero_quiver::BowHand,hero_quiver::DrawHand}){
        const bool drawing=hand==hero_quiver::DrawHand,left=hand==bone::HandL;
        const int upper=left?bone::UpperArmL:bone::UpperArmR;
        const int fore=left?bone::ForearmL:bone::ForearmR;
        const Vec current=pose.world[hand].p;
        const Vec restPole=(pose.world[fore].p-pose.world[upper].p)*(1.f/sword_pose::UpperArm);
        const Vec aimingPole=rotate(aim,drawing?Vec{-1.f,-.65f-.33f*shot.tension,.34f}:Vec{.60f,.20f,-.8f});
        Vec target=drawing?nock:grip;
        if(drawing){
            // The first draw key was painted at one brace height. Seat the
            // nock against the actual relaxed string of the currently held bow.
            const float seating=preparing?ease((shot.preparation-.79f)/.21f):1-ease(shot.tension/.40f);
            const Vec nominal=preparing?Vec{0,-8,0}:hero_quiver::mirror(data::draw[0].nock-data::draw[0].grip);
            const Vec string=pose.world[hero_quiver::BowHand].p-rotate(aim,{0,brace,0});
            target=target+(string-grip-rotate(aim,nominal))*seating;
            if(preparing||seating>0){
                // A small clavicle lift lets the wrist finish withdrawing the
                // rigid shaft before it turns. Both arm bones stay their length.
                const Vec delta=target-pose.world[upper].p;
                const float span=length(delta);
                if(span>sword_pose::UpperArm+sword_pose::Forearm-.2f){
                    const float reach=std::min(span-(sword_pose::UpperArm+sword_pose::Forearm-.2f),4.f);
                    const float follow=preparing?1.f:seating;
                    pose.world[upper].p=pose.world[upper].p+delta*(reach*shot.lift*follow/span);
                }
            }
        }
        hero_carry::placeGrip(pose,left,current*(1-shot.lift)+target*shot.lift,
                              restPole*(1-shot.lift)+aimingPole*shot.lift,
                              mix(pose.skin[hand].q,aim,shot.lift));
    }
}
}
