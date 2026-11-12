#pragma once
#include "hero_weapon_art.hpp"
#include "hero_armor_variants.hpp"

// Included inside actor_art after Rig. Every view and pointer query uses this
// same articulated model, including equipment, bow poses and roll transforms.
template<std::size_t N>
inline void gearPlate(Rig& rig,const std::array<equipment_shape::Point,N>& outline,
                      const equipment_shape::Triangles<N>& triangles,P3 center,P3 u,P3 v,P3 depth,Color color){
 auto cross=[](P3 a,P3 b){return P3{a.f*b.z-a.z*b.f,a.z*b.r-a.r*b.z,a.r*b.f-a.f*b.r};};
 P3 normal=cross(u,v);bool reverse=normal.r*depth.r+normal.f*depth.f+normal.z*depth.z<0;
 auto point=[&](int i,float side){return center+u*outline[i].x+v*outline[i].y+depth*(side*.5f);};
 for(int i=0;i<triangles.count;i++){
  auto t=triangles.indices[i];if(reverse)std::swap(t[1],t[2]);
  rig.face(point(t[0],1),point(t[1],1),point(t[2],1),point(t[2],1),color,1.06f);
  rig.face(point(t[2],-1),point(t[1],-1),point(t[0],-1),point(t[0],-1),color,.78f);
 }
 for(std::size_t i=0;i<N;i++){
  int j=int((i+1)%N);P3 a=point(int(i),-1),b=point(j,-1),c=point(j,1),d=point(int(i),1);
  rig.face(a,b,c,d,color,.85f);rig.face(d,c,b,a,color,.85f);
 }
}
inline void gearInlay(Rig& rig,const GearAppearance& look,P3 center,float width,float height,bool back=false){
    if(!look.present())return;
    for(auto s:equipmentInlay(look.value.kind)){
        float left=(s.x-8)*width/16,right=(s.x+s.width-8)*width/16,top=(8-s.y)*height/16,bottom=(7-s.y)*height/16;
        P3 a=center+P3{left,0,top},b=center+P3{right,0,top},c=center+P3{right,0,bottom},d=center+P3{left,0,bottom};
        if(back)rig.face(d,c,b,a,equipment_art::color(look.paint.accent));
        else rig.face(a,b,c,d,equipment_art::color(look.paint.accent));
    }
}
inline int armorMaterial(Kind kind){
    switch(kind){
        case Kind::Armor:return hero_paint::Steel;
        case Kind::BoarArmor:return hero_paint::Hide;
        case Kind::GuardianArmor:return hero_paint::Stone;
        case Kind::ScarabArmor:case Kind::CobraCloak:return hero_paint::Scales;
        case Kind::BoneArmor:return hero_paint::Bone;
        default:return hero_paint::Cloth;
    }
}
inline void buildHero(Rig& rig,float lift,float stride,float cycle,float tuck,
                      bool helmet,bool shield,Kind weapon,bool flash,bool rolling,bool striking,
                      float strikeProgress,float bowDraw,int quiverArrows,float run,bool empowered,const AnimationState& state,float rollProgress,const HeroAppearance& appearance,float knockdown=-1,float fallR=0,float fallF=-1){
    rig.reserve(1800);
    rig.setSmoothSurfaces();
    const auto& held=appearance[0];const auto& headwear=appearance[1];const auto& ward=appearance[2];
    const auto& armor=appearance[3];const auto& boots=appearance[4];const auto& jewel=appearance[5];
    auto paint=[&](unsigned color){return flash?Text:equipment_art::color(color);};
    const bool travelCoat=!armor.present()||(!appearance.explicitGear&&armor.value.kind==Kind::Armor);
    const bool robe=armor.value.kind==Kind::SandRobe||armor.value.kind==Kind::WebRobe;
    const bool plated=armor.shape==Kind::Armor&&!travelCoat&&!robe;
    const bool realmArmor=additionalArmor(armor.value.kind);
    const bool cape=travelCoat||armor.shape==Kind::Cloak;
    const int material=armorMaterial(armor.value.kind);
    const bool hide=material==hero_paint::Hide,bone=material==hero_paint::Bone,scales=material==hero_paint::Scales;
    const Color skin=flash?Text:rgb(202,158,121),skinLight=flash?Text:rgb(227,186,148);
    const Color metal=plated?paint(armor.paint.primary):paint(0x657f79),edge=plated?shade(metal,1.3f):paint(0x9bb5aa);
    const Color cloth=travelCoat?paint(0x354c62):armor.present()?paint(armor.paint.secondary):paint(0x454c55);
    const Color mantle=travelCoat?paint(0x326e70):armor.present()?paint(armor.paint.primary):cloth,clothLight=shade(cloth,1.25f);
    const Color trim=armor.present()?paint(armor.paint.trim):paint(0xa49370);
    const hero_armor_variants::Finish realmFinish{metal,cloth,trim,paint(armor.paint.accent),
        !realmArmor||flash?-1:hero_paint::NewArmorFirstCell+int(armor.value.kind)-FirstAdditionalArmor};
    const Color weaponMetal=held.present()?paint(held.paint.primary):edge,weaponTrim=held.present()?paint(held.paint.trim):Gold;
    const Color weaponGrip=held.present()?paint(held.paint.secondary):paint(0x60452e);
    const hero_weapon_art::Finish weaponFinish{weaponMetal,weaponGrip,weaponTrim,paint(held.paint.accent),flash};
    const Color leather=paint(0x744b38),pants=paint(0x5d5746);
    bool archer=weapon==Kind::Bow;float drawCharge=std::max(0.f,bowDraw);
    const bool spearing=weapon==Kind::Spear;
    const bool axeOrDagger=weapon==Kind::Axe||weapon==Kind::Dagger;
    const bool swordStrike=striking&&weapon==Kind::Sword;
    float stowPhase=weapon==Kind::Sword&&!striking?(rolling?1.f:state.motionInitialized?state.swordStow:std::clamp(run,0.f,1.f)):0;
    float moving=state.motionInitialized?state.moveBlend:(state.moving?1.f:0.f);
    auto pose=hero_motion::evaluate(state.phase,moving,bowDraw>=0?0:state.runBlend,state.idleTime,swordStrike?strikeProgress:-1,empowered,rollProgress);
    if(!rolling){
        if(archer&&bowDraw<0)hero_carry::bow(pose);
        if(axeOrDagger){
            if(striking)hero_weapon_motion::apply(pose,weapon,strikeProgress);
            else hero_weapon_motion::carry(pose,weapon);
        }
    }
    if(swordStrike)hero_carry::melee(pose,1-hero_motion::ease(strikeProgress/.12f)*hero_motion::ease((1-strikeProgress)/.20f));
    const float spearSlide=spearing?spear_motion::apply(pose,held.value.kind,striking?strikeProgress:-1,moving,stride):0;
    auto stow=weapon==Kind::Sword&&!striking?hero_sheath::apply(pose,stowPhase):sword_stow::sample(0);
    if(knockdown>=0){
        pose=hero_knockdown::evaluate(pose,knockdown,{fallR,fallF,0});
        if(weapon==Kind::Sword&&stowPhase>=.999f)stow=hero_sheath::apply(pose,1);
        if(knockdown>0&&knockdown<1)rig.setGroundBounded();
    }
    float exposedBlade=weapon==Kind::Sword?sword_stow::visibleFraction(stow):1;
    if(archer&&bowDraw>=0&&!rolling){
        for(int side=0;side<2;side++){
            int upper=side?hero_motion::data::UpperArmR:hero_motion::data::UpperArmL;
            int fore=side?hero_motion::data::ForearmR:hero_motion::data::ForearmL;
            int handBone=side?hero_motion::data::HandR:hero_motion::data::HandL;
            auto sh=pose.world[upper].p;
            hero_motion::Vec target=side?hero_motion::Vec{4-12*drawCharge,16-23*drawCharge,47}:hero_motion::Vec{-10,27,47};
            auto arm=sword_pose::solveArm({sh.x,sh.y,sh.z},{target.x,target.y,target.z},side?1:-1);
            hero_motion::Vec elbow{arm.elbow.r,arm.elbow.f,arm.elbow.z},hand{arm.hand.r,arm.hand.f,arm.hand.z};
            hero_motion::attach(pose,upper,sh,elbow,hero_motion::restHead(fore));
            hero_motion::attach(pose,fore,elbow,hand,hero_motion::restHead(handBone));
            pose.world[handBone]={hand,pose.world[hero_motion::data::Spine].q};
        }
        pose.bind();
    }
    // Keep the left side free until the sword hand finishes crossing the hip.
    const bool shieldOnBack=archer||spearing||run>.1f||rolling||stowPhase>0;
    if(shield&&!shieldOnBack&&knockdown<0)hero_carry::shield(pose);
    auto rest=[](int bone){auto p=hero_motion::restHead(bone);return P3{p.x,p.y,p.z};};
    auto world=[](hero_motion::Vec p){return P3{p.x,p.y,p.z};};
    lift=0;stride*=moving;
    auto clothMotion=hero_secondary::evaluate(state.phase,moving,state.runBlend,state.idleTime,swordStrike?strikeProgress:-1,empowered,rollProgress);
    rig.setSkeleton(&pose);rig.setBone(-2);
    // Krita-painted panels conform to the existing joints. Item-specific
    // silhouettes, inlays and materials remain independent of this paint.
    auto panel=[&](P3 center,float width,float height,int cell,float radiusR,float radiusZ,Color tint=WHITE,bool back=false){
        if(flash)return;
        constexpr int cols=8,rows=6;
        auto point=[&](float u,float v){
            float r=(u-.5f)*width,z=(.5f-v)*height;
            float front=center.f*std::sqrt(std::max(.035f,1-r*r/(radiusR*radiusR)-z*z/(radiusZ*radiusZ)));
            return P3{center.r+r,front+(back?-.48f:.48f),center.z+z};
        };
        for(int y=0;y<rows;y++)for(int x=0;x<cols;x++){
            float u=x/float(cols),v=y/float(rows),uu=(x+1)/float(cols),vv=(y+1)/float(rows);
            rig.decal(point(u,v),point(uu,v),point(uu,vv),point(u,vv),cell,u,v,uu,vv,flash?Text:tint,back);
        }
    };
    // Cloth hangs in a curved sheet; its scalloped edge and folds move with
    // the walking cycle, while the roll rotates the entire sheet with the body.
    auto capePoint=[&](float u,float down){
        float edge=std::abs(u*2-1);
        float r=(u*2-1)*(travelCoat?12+down*4:7+down*11);
        float tip=down*down,fold=std::cos(u*5*PI)*(.55f+tip*1.8f);
        float sway=std::sin(cycle-down*2+u*3)*std::abs(stride)*tip*(1.3f+run);
        float length=travelCoat?34.f:45.f;
        float hem=travelCoat?std::sin(u*4*PI)*1.5f:
            edge<=.45f?-1.6f+edge*(3.7f/.45f):2.1f*(1-(edge-.45f)/.55f);
        return P3{r+sway+clothMotion.capeSway*.44f*tip,
                  -10-down*(3+run*9)-fold-clothMotion.capeLift*.50f*tip,
                  58+lift-down*(length-run*7-tuck*13)+hem*tip+clothMotion.capeLift*.25f*tip};
    };
    if(cape){
        for(int row=0;row<7;row++)for(int col=0;col<12;col++){
            float a=col/12.f,b=(col+1)/12.f,top=row/7.f,bottom=(row+1)/7.f;
            P3 p=capePoint(a,top),q=capePoint(b,top),r=capePoint(b,bottom),s=capePoint(a,bottom);
            float light=.86f+.16f*std::cos((a+b)*2.5f*PI);
            rig.face(p,q,r,s,mantle,light);rig.face(s,r,q,p,mantle,light*.85f);
            if(!flash)rig.decal(p,q,r,s,travelCoat?7:material,a,top,b,bottom,shade(mantle,light),true);
        }
        for(int i=0;i<18;i++)rig.tapered(capePoint(i/18.f,1),capePoint((i+1)/18.f,1),.85f,.85f,trim);
        for(float side:{0.f,1.f})for(int i=0;i<8;i++)rig.tapered(capePoint(side,i/8.f),capePoint(side,(i+1)/8.f),.85f,.85f,shade(mantle,1.3f));
    }
    for(int side=0;side<2;side++){
        int thigh=side?hero_motion::data::ThighR:hero_motion::data::ThighL;
        int shin=side?hero_motion::data::ShinR:hero_motion::data::ShinL;
        int ankle=side?hero_motion::data::FootR:hero_motion::data::FootL;
        P3 hip=rest(thigh),knee=rest(shin),foot=rest(ankle);
        rig.setBone(thigh);
        rig.tapered(knee,hip,8,9,pants,1.15f);rig.rounded(knee,{8,8,9},pants);
        if(travelCoat||robe){
            // The two coat panels follow their thighs, clearing the knees and
            // scabbard as the feet trade places.
            float sign=side?1.f:-1.f;
            auto hem=[&](float u,float down){
                float x=hip.r+(u-.5f)*(10+2*down)+sign*down*1.1f;
                return P3{x,7.8f+down*1.4f-std::abs(u-.5f)*2,34-down*(14+sign*(u-.5f)*3)};
            };
            for(int row=0;row<3;row++)for(int col=0;col<4;col++){
                float u=col/4.f,v=row/3.f,uu=(col+1)/4.f,vv=(row+1)/3.f;
                P3 a=hem(u,v),b=hem(uu,v),c=hem(uu,vv),d=hem(u,vv);
                rig.face(a,b,c,d,cloth,.93f);rig.face(d,c,b,a,cloth,.65f);
                if(!flash)rig.decal(a,b,c,d,robe?material:8,u,v,uu,vv,robe?mantle:clothLight);
            }
            for(int i=0;i<4;i++)rig.tapered(hem(i/4.f,1),hem((i+1)/4.f,1),.7f,.7f,trim);
        }
        Color bootColor=boots.present()?paint(boots.paint.secondary):pants;
        Color bootTrim=boots.present()?paint(boots.paint.trim):leather;
        rig.setBone(shin);rig.tapered(foot,knee,7.5f,8,boots.present()?bootColor:pants,.92f);
        if(boots.present())rig.rounded(knee+P3{0,0,-1},{9,9,3},bootTrim);
        if(boots.present()){
            panel({foot.r,4.6f,foot.z+11},7.5f,15,5,5,11,shade(bootColor,1.25f));
            gearInlay(rig,boots,foot+P3{0,4,12},6,9);
            for(int i=0;i<4;i++)rig.tapered(foot+P3{-2.4f,4.8f,7+i*3.f},foot+P3{2.4f,4.8f,9+i*3.f},.55f,.55f,bootTrim);
        }
        rig.setBone(ankle);
        rig.rounded(foot+P3{0,3,-1},{10,16,8},boots.present()?bootColor:leather);
        if(boots.present())rig.rounded(foot+P3{0,8,0},{9,6,5},paint(boots.paint.primary));
        rig.rounded(foot+P3{0,1,4},{9,10,9},boots.present()?bootColor:leather);
        rig.rounded(foot+P3{0,5,-3},{10,14,2},rgb(37,32,28));
        if(!boots.present()){
            rig.rounded(foot+P3{0,0,6},{8.5f,10,4},shade(leather,.75f));
            if(travelCoat){
                rig.setBone(shin);
                rig.tapered(foot+P3{0,0,1},foot+P3{0,0,9},8,7.5f,leather,1.1f);
                panel({foot.r,4.6f,foot.z+5},6,7,5,5,6.5f,shade(leather,1.3f));
            }
        }
        if(boots.present())rig.tapered(foot+P3{-4,2,5},foot+P3{4,2,5},1.1f,1.1f,bootTrim);
    }
    rig.setBone(hero_motion::data::Pelvis);
    rig.rounded({0,0,31+lift},{24,17,15},cloth);
    rig.setBone(hero_motion::data::Spine);
    if(realmArmor){
        hero_armor_variants::torso(rig,armor.value.kind,realmFinish);
    }else if(travelCoat){
        rig.rounded({0,0,46},{25,17,27},cloth);
        panel({0,8.6f,46},23,25,3,12.5f,14,clothLight);
        panel({0,-8.6f,46},23,25,3,12.5f,14,clothLight,true);
    }else{
        const int cell=flash?-1:material;
        const Color outer=plated?metal:mantle;
        rig.rounded({0,0,47},{plated?27.f:25.f,plated?21.f:18.f,28},outer,cell,false,cell,false,true);
        // Follow the shell curvature so neck binding and shoulder seams sit
        // against the material as the torso twists.
        for(float side:{-1.f,1.f})for(float facing:{-1.f,1.f}){
            const float f=facing*(plated?1.f:.86f);
            rig.tapered({0,8.2f*f,54},{side*5,7.5f*f,56.5f},1.2f,1.2f,trim);
            rig.tapered({side*5,7.5f*f,56.5f},{side*8,6.2f*f,56},1.2f,1.2f,trim);
            rig.tapered({side*8,6.2f*f,56},{side*11,6.5f*f,51},1.1f,1.1f,trim);
            if(plated){
                rig.rounded({side*9.8f,7.5f*f,51},{1.8f,1.8f,1.8f},trim);
                rig.rounded({side*10.3f,7.3f*f,43},{1.5f,1.7f,1.5f},trim);
            }
        }
    }
    if(plated&&!realmArmor){
        const int rows=scales||bone?4:3;
        for(float facing:{-1.f,1.f})for(int row=0;row<rows;row++){
            float z=35+row*3.1f,halfWidth=8.f+row*.63f,front=(6.7f+row*.8f)*facing;
            Color finish=shade(metal,.86f+.07f*row);
            rig.rounded({0,front,z},{halfWidth*2+2,5,4.8f},finish);
            for(float side:{-1.f,1.f}){
                rig.tapered({0,front+2.4f*facing,z-.9f},
                            {side*halfWidth,front-.4f*facing,z+.6f},.72f,.72f,shade(trim,.78f+.055f*row));
            }
        }
        gearInlay(rig,armor,{0,10.7f,50.1f},7,7);
        gearInlay(rig,armor,{0,-10.7f,50.1f},7,7,true);
    }
    rig.setBone(hero_motion::data::Pelvis);
    rig.rounded({0,0,32+lift},{25,18,4},leather);
    rig.rounded({0,9,32+lift},{5,2,4},Gold);rig.rounded({0,10,32+lift},{2.8f,1,2},leather);
    rig.setBone(hero_motion::data::Spine);
    if(travelCoat)rig.tapered({-9,8,54},{9,8,36},3.3f,2,leather);
    P3 clasp=travelCoat?P3{-2,10.3f,50}:P3{0,plated?11.f:9.4f,54};
    if(!realmArmor){
        rig.rounded(clasp,{travelCoat?7.f:5.f,2,travelCoat?7.f:6.f},trim);
        rig.rounded(clasp+P3{0,1.2f,0},{3.2f,1,3.2f},travelCoat?paint(0x263b45):cloth);
    }
    if(travelCoat)for(int i=0;i<4;i++){
        P3 ray=i%2?P3{0,0,i==1?1.f:-1.f}:P3{i==0?1.f:-1.f,0,0};
        rig.tapered(clasp+P3{0,1.8f,0}+ray*1.25f,clasp+P3{0,1.5f,0}+ray*2.9f,.8f,.65f,trim);
    }
    for(float side:{-1.f,1.f}){
        rig.rounded({side*12,-1,29+lift},{7,9,9},leather);
        rig.rounded({side*12,3.5f,31+lift},{6,2,3},shade(leather,1.25f));
        rig.rounded({side*12,4.5f,30+lift},{1.6f,1,1.6f},Gold);
    }
    rig.setBone(hero_motion::data::Head);
    rig.rounded({0,0,61+lift},{10,10,10},skin);
    // A softer collar and two shoulder plates attach to rounded upper arms.
    rig.setBone(hero_motion::data::Spine);
    rig.rounded({0,-1,57+lift},{26,18,7},mantle);
    if(travelCoat||cape||robe)for(float side:{-1.f,1.f}){
        rig.tapered({side*5,-1,60},{side*8,7,54},4.2f,3,shade(mantle,1.2f));
        rig.tapered({side*8,7,54},{side*2,9,51},3.2f,2.4f,mantle);
    }
    if(jewel.shape==Kind::Amulet){
        Color chain=paint(jewel.paint.trim);
        rig.tapered({-6,7,58},{0,10.5f,48},.7f,.7f,chain);rig.tapered({6,7,58},{0,10.5f,48},.7f,.7f,chain);
        gearPlate(rig,equipment_shape::Pendant,equipment_shape::PendantTriangles,{0,11,51},{6,0,0},{0,0,6},{0,2,0},chain);
        gearPlate(rig,equipment_shape::Pendant,equipment_shape::PendantTriangles,{0,12.3f,50.2f},{4.2f,0,0},{0,0,4.2f},{0,1.2f,0},paint(jewel.paint.accent));
    }
    if(archer){
        rig.rounded({-7,-17,44+lift},{12,12,31},leather);
        rig.rounded({-7,-17,59+lift},{14,13,4},Gold);
        rig.tapered({8,9,54+lift},{-9,-17,32+lift},2.8f,2,leather);
        for(int i=0;i<std::clamp(quiverArrows,0,3);i++){
            float r=-10+i*3.f,top=73+i%2*5.f+lift;
            rig.tapered({r,-17,45+lift},{r,-17,top},1.1f,1.1f,rgb(182,134,75));
            rig.form({r,-17,top-3},{5,2,6},Text);rig.form({r,-17,top-4},{2,5,6},Gold);
        }
    }
    if(weapon==Kind::Sword){
        // The scabbard hangs from the belt at the left hip. Its body follows
        // the torso, never the pumping hand. A drawn blade leaves it empty.
        rig.setBone(hero_motion::data::Pelvis);
        P3 mouth=swordPoint(sword_stow::Mouth),axis=swordPoint(sword_stow::Axis),tip=mouth+axis*37;
        Color darkLeather=shade(weaponGrip,.62f);
        rig.tapered({-12,7,33+lift},mouth+P3{0,0,-4},2.1f,2.1f,leather);
        rig.tapered({-12,3,32+lift},mouth+axis*7,2,2,leather);
        rig.tapered(mouth+axis*1.2f,tip,5.7f,3.8f,darkLeather);
        rig.tapered(mouth, mouth+axis*2,6.6f,6.2f,weaponTrim);
        rig.tapered(mouth+axis*34.5f,tip,4.3f,3.8f,metal);
        rig.tapered(mouth+axis*9,mouth+axis*10,5.9f,5.9f,leather);
        rig.tapered(mouth+axis*23,mouth+axis*24,5.2f,5.2f,leather);
        rig.setBone(hero_motion::data::Spine);
    }
    for(float side:{-1.f,1.f}){
        int upper=side>0?hero_motion::data::UpperArmR:hero_motion::data::UpperArmL;
        int fore=side>0?hero_motion::data::ForearmR:hero_motion::data::ForearmL;
        int handBone=side>0?hero_motion::data::HandR:hero_motion::data::HandL;
        P3 shoulder=rest(upper),elbow=rest(fore),hand=rest(handBone);
        rig.setBone(upper);
        rig.tapered(elbow,shoulder,8,9,cloth);
        rig.rounded(shoulder,{11,14,8},cloth);
        if(travelCoat){
            P3 pauldron=shoulder+P3{side*.7f,.4f,1.6f};
            rig.rounded(pauldron,{12.2f,16,7.2f},paint(0x995d36),flash?-1:6);
            rig.rounded(pauldron+P3{side*.25f,0,-3.1f},{11.8f,14.7f,2.2f},trim);
        }
        if(realmArmor){
            hero_armor_variants::shoulder(rig,armor.value.kind,shoulder,side,realmFinish);
        }else if(plated){
            const float width=hide?11.f:13.f;
            P3 shell=shoulder+P3{side*.9f,.4f,1.7f};
            const int cell=flash?-1:material;
            rig.rounded(shell,{width,15.4f,8},metal,cell,false,cell,false,true);
            rig.rounded(shell+P3{side*.6f,0,-3.5f},{width-.6f,14.2f,4.1f},shade(trim,.87f));
            if(scales||bone)rig.rounded(shell+P3{side*.9f,0,-6},{width-1.8f,11.8f,4.7f},metal);
            for(float f:{-1.f,1.f})rig.rounded(shell+P3{side*2.2f,f*6.9f,-.3f},{2,2,2},trim);
        }
        rig.setBone(fore);
        rig.tapered(hand,elbow,6,7,cloth,1.15f);
        rig.rounded(elbow,{8,9,8},plated?metal:cloth);
        P3 cuff=hand+(elbow-hand)*.42f;
        rig.rounded(cuff,{7.2f,8.2f,9},leather);
        panel({cuff.r,cuff.f+4.2f,cuff.z},7.4f,9.5f,4,5.8f,8.5f);
        rig.setBone(handBone);
        rig.rounded(!archer&&!rolling?hand+(elbow-hand)*.17f:hand+P3{0,-1,5},{7,8,7},leather);
        rig.rounded(hand,{7,7,8},skin);
        rig.rounded(hand+P3{0,3.2f,1},{5,1.5f,5},skinLight);
        for(int i=0;i<3;i++)rig.tapered(hand+P3{-2+i*2.f,3.8f,-1},hand+P3{-2+i*2.f,4,2},.35f,.35f,shade(skin,.73f));
        if(side>0&&jewel.shape==Kind::Ring){
            rig.rounded(hand+P3{1,2,1},{3.5f,5.5f,3},paint(jewel.paint.trim));
            rig.rounded(hand+P3{1,4.6f,1.5f},{3.5f,2.2f,3.5f},paint(jewel.paint.accent));
        }
        if(archer&&side<0){
            rig.setBone(-1);
            P3 grip=world(pose.world[hero_motion::data::HandL].p);
            P3 axis=world(pose.vector(hero_motion::data::HandL,{0,0,1}));
            P3 forward=world(pose.vector(hero_motion::data::HandL,{0,1,0}));
            const bool nocked=bowDraw>=0&&!rolling;
            const float curve=equipment_shape::BowCurve*(1-.15f*drawCharge);
            P3 string=nocked?world(pose.world[hero_motion::data::HandR].p):grip-forward*curve;
            hero_weapon_art::bow(rig,grip,axis,forward,string,weaponFinish,held.value.kind,drawCharge,nocked);
        }
        if(side>0&&spearing){
            rig.setBone(-1);
            hero_weapon_art::spear(rig,world(pose.world[hero_motion::data::HandR].p),
                 world(pose.world[hero_motion::data::HandL].p),spearSlide,weaponFinish,held.value.kind);
        }else if(side>0&&held.present()&&!archer){
            rig.setBone(hero_motion::data::Weapon);
            hero_weapon_art::melee(rig,hand,held.value.kind,exposedBlade,weaponFinish);
        }
        if(side<0&&shield){
            bool stored=shieldOnBack;float front=stored?-1.f:1.f;
            rig.setBone(stored?hero_motion::data::Spine:handBone);
            P3 p=stored?P3{archer?7.f:0.f,-15,archer?42.f:45.f}:hand+P3{-2,hero_carry::ShieldOffset,1};
            if(!stored){
                // The fist closes around a leather handle mounted on the inner face.
                rig.tapered(hand+P3{-2,2,-2.3f},p+P3{0,-1.5f,-3.3f},1.7f,1.6f,leather);
                rig.tapered(hand+P3{-2,2,2.3f},p+P3{0,-1.5f,3.3f},1.7f,1.6f,leather);
            }
            gearPlate(rig,equipment_shape::Shield,equipment_shape::ShieldTriangles,p,{8.7f,0,0},{0,0,8.7f},{0,front*hero_carry::ShieldRimDepth,0},paint(ward.paint.trim));
            gearPlate(rig,equipment_shape::Shield,equipment_shape::ShieldTriangles,p+P3{0,front*2,0},{7.45f,0,0},{0,0,7.45f},{0,front*1.4f,0},paint(ward.paint.primary));
            for(float x:{-.6f,0.f,.6f}){rig.tapered(p+P3{x*8.7f,front*3,8},p+P3{x*8.7f,front*3,0},.65f,.65f,paint(ward.paint.secondary));rig.tapered(p+P3{x*8.7f,front*3,0},p+P3{x*3.9f,front*3,-10},.65f,.65f,paint(ward.paint.secondary));}
            rig.rounded(p+P3{0,front*3.6f,1.3f},{5.5f,2.5f,5.5f},paint(ward.paint.trim));
            rig.rounded(p+P3{0,front*5,1.3f},{2.4f,1.3f,2.4f},paint(ward.paint.accent));
        }
    }
    rig.setBone(hero_motion::data::Head);
    float head=70+lift;
    rig.rounded({0,0,head},{17,16,20},skin,flash?-1:0);
    for(float side:{-1.f,1.f}){
        rig.rounded({side*8.2f,-.1f,head-.8f},{2.8f,3.5f,5.3f},skin);
        if(!hero_paint::atlas.id){
        rig.rounded({side*4,7.7f,head+1.6f},{3.2f,1.6f,1.7f},rgb(235,229,208));
        rig.rounded({side*4,8.6f,head+1.6f},{1.4f,1,1.5f},Ink);
        rig.tapered({side*2.3f,8,head+4},{side*5.8f,7,head+4.4f},.7f,.7f,shade(leather,.7f));
        rig.rounded({side*4.8f,7,head-2},{4,2.5f,3},skinLight);
        }
    }
    if(!hero_paint::atlas.id){
    rig.rounded({0,8.1f,head-1},{3.4f,4,4.5f},skinLight);
    rig.tapered({-2.3f,8.3f,head-5},{2.3f,8.3f,head-5},.65f,.65f,rgb(134,88,78));
    rig.rounded({0,6.5f,head-8},{9,3,2.5f},shade(skin,.82f));
    }
    Color hood=helmet?paint(headwear.paint.primary):flash?Text:hero_paint::HairColor;
    if(helmet){
        rig.rounded({0,-3,head+2},{20,17,24},hood);
        gearPlate(rig,equipment_shape::Helmet,equipment_shape::HelmetTriangles,{0,6,head},{8.5f,0,0},{0,0,8.5f},{0,4,0},hood);
        gearPlate(rig,equipment_shape::Visor,equipment_shape::VisorTriangles,{0,8.2f,head},{8.5f,0,0},{0,0,8.5f},{0,.6f,0},paint(0x17212a));
    }else{
        rig.rounded({-.85f,-3.45f,head+4.2f},{19.5f,18,17.6f},hood,flash?-1:1,false,flash?-1:2);
        rig.rounded({-5.4f,3.6f,head+6.4f},{6.8f,8.1f,5.4f},hood,flash?-1:1);
    }
    for(float side:{-1.f,1.f}){
        P3 a{side*8,6,head+8},b{side*10,4,head-5};
        if(helmet)rig.tapered(a,b,.8f,.8f,paint(headwear.paint.trim));
    }
    if(helmet){
        Color helmetTrim=paint(headwear.paint.trim);
        rig.tapered({0,9,head+4},{0,9,head-10},1.6f,1.6f,helmetTrim);
        rig.tapered({-9,8.8f,head+3.6f},{0,9.2f,head+5.3f},1.1f,1.1f,helmetTrim);
        rig.tapered({0,9.2f,head+5.3f},{9,8.8f,head+3.6f},1.1f,1.1f,helmetTrim);
        rig.rounded({0,8.7f,head+8},{2.8f,1.6f,2.8f},paint(headwear.paint.accent));
    }
    rig.setSkeleton(nullptr);
}
