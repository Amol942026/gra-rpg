#pragma once
#include "hero_weapon_art.hpp"
#include "hero_armor_variants.hpp"
#include "hero_headwear.hpp"
#include "round_shield_art.hpp"

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
// A fitted fabric panel follows one limb, allowing the sleeve to taper into
// the cuff and each trouser leg to narrow at the knee independently.
inline void fittedGarment(Rig& rig,P3 bottom,P3 top,const std::array<Vector2,4>& diameters,
                          Color cloth,int cell,bool closedEnds=false,int frontCell=-1){
    const auto delta=top-bottom;
    const float length=std::sqrt(delta.r*delta.r+delta.f*delta.f+delta.z*delta.z);
    if(length<.001f)return;
    const P3 axis=delta*(1/length);
    const float flat=std::hypot(axis.r,axis.f);
    const P3 right=flat>.001f?P3{axis.f/flat,-axis.r/flat,0}:P3{1,0,0};
    const P3 front{axis.f*right.z-axis.z*right.f,axis.z*right.r-axis.r*right.z,
                   axis.r*right.f-axis.f*right.r};
    const bool paint=cell>=0&&hero_paint::atlas.id&&hero_paint::opaque(hero_paint::uv(cell,.5f,.5f));
    constexpr int sides=12;
    constexpr float levels[]={0,.26f,.69f,1};
    std::array<std::array<P3,sides>,4> points;
    std::array<float,sides> lights;
    const auto& ring=Rig::unitSphere<sides,2>()[1];
    for(int i=0;i<sides;i++){
        const P3 normal=right*ring[i].r+front*ring[i].f;
        lights[i]=rig.surfaceLight(normal);
        for(int row=0;row<4;row++){
            const auto d=diameters[row];
            points[row][i]=bottom+delta*levels[row]+right*(.5f*d.x*ring[i].r)+front*(.5f*d.y*ring[i].f);
        }
    }
    for(int i=0;i<sides;i++){
        const int next=(i+1)%sides;
        const float light=(lights[i]+lights[next])*.5f;
        const float column=float((i+sides*3/4)%sides);
        for(int row=0;row<3;row++){
            const P3 a=points[row][i],b=points[row][next],c=points[row+1][next],d=points[row+1][i];
            const Color left=shade(cloth,lights[i]),rightColor=shade(cloth,lights[next]);
            rig.surfaceQuad(a,b,c,d,left,rightColor,rightColor,left);
            if(frontCell>=0&&(ring[i].f+ring[next].f)>0){
                rig.decal(a,b,c,d,frontCell,.5f+.5f*ring[i].r,1-levels[row],
                          .5f+.5f*ring[next].r,1-levels[row+1],shade(WHITE,light));
            }else if(paint){
                rig.decal(a,b,c,d,cell,column/sides,1-levels[row],(column+1)/sides,
                          1-levels[row+1],shade(cloth,light));
            }
        }
        if(closedEnds){
            rig.face(bottom,points[0][next],points[0][i],bottom,cloth,.8f);
            rig.face(top,points[3][i],points[3][next],top,cloth);
        }
    }
}
inline void travelPauldron(Rig& rig,P3 shoulder,float side,Color shell,Color lining,
                           Color bronze,int shellCell,bool flash){
    // Three overlapping lames slope from the shoulder joint toward the
    // upper arm. Their cut edges remain readable even in a side view.
    constexpr int columns=12;
    const auto& circle=Rig::unitSphere<columns,2>()[1];
    const auto point=[&](int band,int column){
        constexpr float radiusR[]={5.9f,6.5f,6.8f,5.8f,3.2f};
        constexpr float radiusF[]={6.4f,7.3f,7.8f,7.2f,4.6f};
        constexpr float levels[]={-3.7f,-2.f,-.6f,1.3f,3.6f};
        const P3 n=circle[column];
        const float outer=.5f+.5f*side*n.r;
        return shoulder+P3{side*.45f+n.r*radiusR[band],n.f*radiusF[band],
                           levels[band]+1.1f-outer*(band<2?3.1f:1.6f)};
    };
    const P3 crown=shoulder+P3{-side*.75f,-.4f,5.3f};
    for(int i=0;i<columns;i++){
        const int j=(i+1)%columns;
        for(int band=0;band<4;band++){
            const P3 a=point(band,i),b=point(band,j),c=point(band+1,j),d=point(band+1,i);
            const float light=rig.surfaceLight({circle[i].r,circle[i].f,band>1?.45f:0.f});
            const Color color=band==0?lining:band==1?bronze:shell;
            rig.face(a,b,c,d,color,light);
            if(!flash&&band>=2)rig.decal(a,b,c,d,shellCell,float(i)/columns,
                (4-band)*.32f,float(i+1)/columns,(3-band)*.32f,shade(shellCell==6?WHITE:shell,light));
        }
        const auto a=point(4,i),b=point(4,j);
        rig.face(a,b,crown,crown,shell,1.04f);
    }
    const P3 rivet=shoulder+P3{side*4.4f,5.5f,-1.0f};
    rig.rounded(rivet,{1.65f,1.3f,1.6f},bronze);
}
inline void buildHero(Rig& rig,float lift,float stride,float cycle,float tuck,
                      bool helmet,bool shield,Kind weapon,bool flash,bool rolling,bool striking,
                      float strikeProgress,float bowDraw,int quiverArrows,float run,bool empowered,const AnimationState& state,float rollProgress,const HeroAppearance& appearance,float knockdown=-1,float fallR=0,float fallF=-1,
                      PowerStyle power=PowerStyle::Count,float powerAge=0,float windup=-1,HeroReaction reaction={},float bowRelease=-1,float bowPreparation=1.f){
    rig.reserve(1800);
    rig.setSmoothSurfaces();
    const auto& held=appearance[0];const auto& headwear=appearance[1];const auto& ward=appearance[2];
    const auto& armor=appearance[3];const auto& boots=appearance[4];const auto& jewel=appearance[5];
    auto paint=[&](unsigned color){return flash?Text:equipment_art::color(color);};
    const bool travelCoat=!armor.present()||(!appearance.explicitGear&&armor.value.kind==Kind::Armor);
    const bool robe=armor.value.kind==Kind::SandRobe||armor.value.kind==Kind::WebRobe;
    const bool plated=armor.shape==Kind::Armor&&!travelCoat&&!robe;
    const bool realmArmor=additionalArmor(armor.value.kind);
    const bool trailMantle=travelCoat||armor.value.kind==Kind::Armor;
    const bool cape=trailMantle||armor.shape==Kind::Cloak;
    const int material=armorMaterial(armor.value.kind);
    const bool hide=material==hero_paint::Hide,bone=material==hero_paint::Bone,scales=material==hero_paint::Scales;
    const Color skin=flash?Text:rgb(202,158,121),skinLight=flash?Text:rgb(227,186,148);
    const Color metal=plated?paint(armor.paint.primary):paint(0x657f79);
    const Color cloth=travelCoat?paint(0x354c62):armor.present()?paint(armor.paint.secondary):paint(0x454c55);
    const Color mantle=trailMantle?paint(0x387172):armor.present()?paint(armor.paint.primary):cloth,clothLight=shade(cloth,1.25f);
    const Color trim=armor.present()?paint(armor.paint.trim):paint(0xa49370);
    const hero_armor_variants::Finish realmFinish{metal,cloth,trim,paint(armor.paint.accent),
        !realmArmor||flash?-1:hero_paint::NewArmorFirstCell+int(armor.value.kind)-FirstAdditionalArmor};
    const Color weaponTrim=held.present()?paint(held.paint.trim):Gold;
    const Color weaponGrip=held.present()?paint(held.paint.secondary):paint(0x60452e);
    const auto weaponFinish=hero_weapon_art::finish(held,flash);
    const Color leather=paint(0x744b38),pants=paint(0x5d5746);
    if(rolling||knockdown>=0){power=PowerStyle::Count;windup=-1;}
    const bool extraStow=weapon_stow::supported(weapon);
    const float stowPhase=(weapon==Kind::Sword||extraStow)&&!striking?(rolling?1.f:state.motionInitialized?state.swordStow:std::clamp(run,0.f,1.f)):0;
    const bool charging=windup>=0;
    if(charging)power=powerStyle(weapon);
    const bool overhead=power_motion::usesOverheadGrip(power);
    const float powerProgress=charging?windup:powerAge;
    const auto powerKey=power_motion::sample(power,powerProgress,charging);
    if(power!=PowerStyle::Count)strikeProgress=powerKey.phase;
    if(power==PowerStyle::Sword)empowered=true;
    if(weapon==Kind::Dagger&&striking&&empowered&&power==PowerStyle::Count){
        const float first=float(DaggerImpactFrame)/DaggerAttackFrames;
        const float second=SkillDaggerSecond/DaggerAttackDuration;
        const float pullback=(first+second)*.5f;
        if(strikeProgress>first&&strikeProgress<pullback)
            strikeProgress=first-(strikeProgress-first)/(pullback-first)*.28f;
        else if(strikeProgress>=pullback&&strikeProgress<second)
            strikeProgress=first-.28f+(strikeProgress-pullback)/(second-pullback)*.28f;
        else if(strikeProgress>=second)
            strikeProgress=first+(1-first)*(strikeProgress-second)/(1-second);
    }
    const bool archer=weapon==Kind::Bow;
    const auto bowShot=archer?hero_bow::sampleShot(bowDraw,bowRelease,bowPreparation):hero_bow::Shot{};
    const auto bowProfile=weapon_profiles::bow(held.value.kind);
    const float brace=bowProfile.curve-bowProfile.recurve*.29f;
    const auto arrows=hero_quiver::stock(quiverArrows,archer&&bowDraw>=0&&bowPreparation>=hero_quiver::Pickup&&
                                         !rolling&&knockdown<0,bowRelease>=0);
    const bool spearing=weapon==Kind::Spear;
    const bool axeOrDagger=weapon==Kind::Axe||weapon==Kind::Dagger;
    const bool attackPose=striking||charging;
    const bool swordStrike=striking&&weapon==Kind::Sword&&!overhead;
    float moving=state.motionInitialized?state.moveBlend:(state.moving?1.f:0.f);
    const float sprintBlend=archer?state.runBlend*(1-bowShot.lift):attackPose?0.f:state.runBlend;
    if(archer&&!rolling)run=sprintBlend;
    auto pose=hero_motion::evaluate(state.phase,moving,sprintBlend,state.idleTime,swordStrike?strikeProgress:-1,empowered,rollProgress);
    if(power!=PowerStyle::Count&&!archer)power_motion::body(pose,powerKey);
    auto natural=pose;
    if(!rolling){
        if(archer&&bowShot.lift<1)hero_carry::bow(pose);
        if(axeOrDagger){
            if(attackPose&&!overhead&&stowPhase<=0)hero_weapon_motion::apply(pose,weapon,strikeProgress);
            else hero_weapon_motion::carry(pose,weapon);
        }
    }
    if(swordStrike)hero_carry::melee(pose,1-hero_motion::ease(strikeProgress/.12f)*hero_motion::ease((1-strikeProgress)/.20f));
    const float spearSlide=spearing?spear_motion::apply(pose,held.value.kind,attackPose&&stowPhase<=0?strikeProgress:-1,moving,stride):0;
    auto stow=weapon==Kind::Sword&&(!swordStrike||stowPhase>0)?hero_sheath::apply(pose,stowPhase):sword_stow::sample(0);
    if(overhead)power_motion::overheadGrip(pose,power,powerProgress,charging,1-hero_motion::ease(stowPhase/(extraStow?.08f:.26f)));
    const bool reacting=!rolling&&knockdown<0&&reaction.progress>=0;
    if(knockdown>=0){
        pose=hero_knockdown::evaluate(pose,knockdown,{fallR,fallF,0});
        if(extraStow&&stowPhase>0)natural=hero_knockdown::evaluate(natural,knockdown,{fallR,fallF,0});
        if(weapon==Kind::Sword&&stowPhase>=.999f)stow=hero_sheath::apply(pose,1);
        if(knockdown>0&&knockdown<1)rig.setGroundBounded();
    }else if(reacting){
        hero_hit_reaction::apply(pose,reaction.progress,reaction.away);
        if((extraStow&&stowPhase>0)||(shield&&weapon==Kind::Sword))
            hero_hit_reaction::apply(natural,reaction.progress,reaction.away);
        if(weapon==Kind::Sword&&stowPhase>=.999f)stow=hero_sheath::apply(pose,1);
    }
    weapon_stow::Mount extra{};
    const float backDepth=cape?12.3f:realmArmor?
        hero_armor_variants::cuts[int(armor.value.kind)-FirstAdditionalArmor].depth*.5f+1.4f:plated?11.85f:10.5f;
    if(extraStow&&stowPhase>0)extra=weapon_stow::apply(pose,natural,weapon,stowPhase,spearSlide,backDepth);
    float exposedBlade=weapon==Kind::Sword?sword_stow::visibleFraction(stow):extra.daggerVisible;
    if(archer&&!rolling&&knockdown<0)hero_bow::apply(pose,bowShot,brace);
    const bool shieldFixed=archer||spearing||rolling;
    const float shieldPhase=shieldFixed?1.f:(state.motionInitialized?state.swordStow:std::clamp(run,0.f,1.f));
    hero_motion::Transform shieldMount{};
    if(shield)shieldMount=shield_stow::apply(pose,weapon,shieldPhase,backDepth,!shieldFixed&&knockdown<0,
                                           weapon==Kind::Sword&&knockdown<0?&natural:nullptr);
    auto rest=[](int bone){auto p=hero_motion::restHead(bone);return P3{p.x,p.y,p.z};};
    auto world=[](hero_motion::Vec p){return P3{p.x,p.y,p.z};};
    lift=0;stride*=moving;
    auto clothMotion=hero_secondary::evaluate(state.phase,moving,state.runBlend,state.idleTime,swordStrike?strikeProgress:-1,empowered,rollProgress);
    if(cape)hero_motion::mantleTension(clothMotion,pose);
    if(!rolling){
        const auto finishing=traveler_gesture::action(weapon,striking?strikeProgress:-1,power,powerAge,windup,bowDraw,
                                                      bowRelease,stowPhase,state.sprinting,reacting?reaction.progress:-1,knockdown);
        traveler_gesture::apply(pose,clothMotion,finishing,std::clamp(bowDraw,0.f,1.f));
    }
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
        float r=(u*2-1)*(trailMantle?11.7f+down*3.9f:7+down*11);
        float tip=down*down,fold=std::cos(u*5*PI)*(.55f+tip*1.8f);
        float sway=std::sin(cycle-down*2+u*3)*std::abs(stride)*tip*(1.3f+run);
        float length=trailMantle?32.f:45.f;
        float hem=trailMantle?std::sin(u*2*PI)*2.4f+std::abs(2*u-1)*4.f:
            edge<=.45f?-1.6f+edge*(3.7f/.45f):2.1f*(1-(edge-.45f)/.55f);
        float back=-10-down*(3+run*9)-fold-clothMotion.capeLift*.50f*tip;
        if(weapon==Kind::Axe||spearing){
            const float contact=hero_motion::ease((stowPhase-.52f)/.2f);
            const auto mount=weapon_stow::socket(weapon,backDepth);
            const auto shaft=weapon_stow::storedAxis(weapon);
            const float restingLine=mount.x+(58-down*length-mount.z)*shaft.x/shaft.z;
            const float flatten=(1-hero_motion::ease((std::abs(r-restingLine)-5)/13))*contact;
            back=back+(1.1f-backDepth-back)*flatten;
        }
        return P3{r+sway+clothMotion.capeSway*.44f*tip,back,
                  58+lift-down*(length-run*7-tuck*13)+hem*tip+clothMotion.capeLift*.25f*tip};
    };
    if(cape){
        for(int row=0;row<7;row++)for(int col=0;col<12;col++){
            float a=col/12.f,b=(col+1)/12.f,top=row/7.f,bottom=(row+1)/7.f;
            P3 p=capePoint(a,top),q=capePoint(b,top),r=capePoint(b,bottom),s=capePoint(a,bottom);
            float light=.86f+.16f*std::cos((a+b)*2.5f*PI);
            rig.face(p,q,r,s,mantle,light);rig.face(s,r,q,p,mantle,light*.85f);
            if(!flash)rig.decal(p,q,r,s,trailMantle?7:material,a,top,b,bottom,shade(mantle,light),true);
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
        fittedGarment(rig,knee,hip,{{{7.3f,7.4f},{9,9.6f},{10.7f,11.5f},{9.9f,10.5f}}},pants,flash?-1:9);
        rig.rounded(knee,{7.5f,7.8f,7.8f},pants);
        if(trailMantle||robe){
            // The two coat panels follow their thighs, clearing the knees and
            // scabbard as the feet trade places.
            float sign=side?1.f:-1.f;
            auto hem=[&](float u,float down){
                float x=hip.r+(u-.5f)*(10+2.8f*down)+sign*down*1.3f;
                return P3{x,7.9f+down*1.4f-std::abs(u-.5f)*2,34-down*(15.5f+sign*(u-.5f)*4.8f)};
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
        rig.setBone(shin);
        fittedGarment(rig,foot,knee,{{{6.5f,7.2f},{7.2f,8},{8.6f,9},{7.7f,7.6f}}},
                      boots.present()?bootColor:pants,flash?-1:boots.present()?5:9);
        if(boots.present())rig.rounded(knee+P3{0,0,-1},{9,9,3},bootTrim);
        if(boots.present()){
            panel({foot.r,4.6f,foot.z+11},7.5f,15,5,5,11,shade(bootColor,1.25f));
            gearInlay(rig,boots,foot+P3{0,4,12},6,9);
            for(int i=0;i<4;i++)rig.tapered(foot+P3{-2.4f,4.8f,7+i*3.f},foot+P3{2.4f,4.8f,9+i*3.f},.55f,.55f,bootTrim);
        }
        const Color hide=boots.present()?bootColor:leather;
        rig.setBone(shin);
        fittedGarment(rig,foot+P3{0,0,.2f},foot+P3{0,1.6f,9.0f},
                      {{{6.4f,7.1f},{7.2f,7.7f},{7.7f,8.0f},{8.2f,8.5f}}},
                      hide,flash?-1:5);
        for(int row=0;row<2;row++){
            const float z=4.6f+2.7f*row;
            rig.tapered(foot+P3{-3.2f,3.2f,z-.8f},foot+P3{2.9f,4.6f,z+1.3f},
                        .85f,1.10f,boots.present()?bootTrim:shade(leather,1.24f));
        }
        rig.setBone(ankle);
        fittedGarment(rig,foot+P3{0,-3.8f,0},foot+P3{0,10.7f,-1.1f},
                      {{{6.2f,6.5f},{7.4f,8.2f},{8.2f,5.4f},{3.8f,2.7f}}},
                      boots.present()?paint(boots.paint.primary):hide,flash?-1:5,true);
        fittedGarment(rig,foot+P3{0,-3.9f,-2.9f},foot+P3{0,10.8f,-2.8f},
                      {{{6.1f,1.3f},{7.6f,1.7f},{8.1f,1.65f},{3.6f,1.1f}}},
                      shade(leather,.49f),-1,true);
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
        if(armor.value.kind==Kind::Armor){
            // The starter cuirass narrows at the waist and neck; the widest
            // part is over the ribs, leaving the hinged pauldrons free.
            fittedGarment(rig,{0,0,32},{0,0,59},
                          {{{19.6f,14.8f},{22.5f,18.2f},{28.1f,21.1f},{17.4f,13.9f}}},metal,cell,
                          false,hero_paint::cuirassLoaded&&!flash?18:-1);
            if(!hero_paint::cuirassLoaded)for(float side:{-1.f,1.f}){
                const P3 throat{side*3.0f,8.8f,55.6f},ridge{side*6.9f,9.1f,50.4f};
                const P3 waist{side*3.1f,9.0f,40.7f};
                rig.tapered(throat,ridge,1.2f,.80f,shade(metal,1.22f));
                rig.tapered(ridge,waist,1.15f,.77f,shade(metal,1.22f),.60f);
            }
        }else{
            rig.rounded({0,0,47},{plated?27.f:25.f,plated?21.f:18.f,28},outer,cell,false,cell,false,true);
        }
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
        const int rows=scales||bone?4:armor.value.kind==Kind::Armor?2:3;
        for(float facing:{-1.f,1.f})for(int row=0;row<rows;row++){
            float z=35+row*3.1f,halfWidth=8.f+row*.63f,front=(6.7f+row*.8f)*facing;
            Color finish=shade(metal,.86f+.07f*row);
            rig.rounded({0,front,z},{halfWidth*2+2,5,4.8f},finish);
            for(float side:{-1.f,1.f}){
                rig.tapered({0,front+2.4f*facing,z-.9f},
                            {side*halfWidth,front-.4f*facing,z+.6f},.72f,.72f,shade(trim,.78f+.055f*row));
            }
        }
        if(armor.value.kind!=Kind::Armor||!hero_paint::cuirassLoaded)
            gearInlay(rig,armor,{0,10.7f,50.1f},7,7);
        gearInlay(rig,armor,{0,-10.7f,50.1f},7,7,true);
    }
    rig.setBone(hero_motion::data::Pelvis);
    rig.rounded({0,0,32+lift},{25,18,4},leather);
    rig.rounded({0,9,32+lift},{5,2,4},Gold);rig.rounded({0,10,32+lift},{2.8f,1,2},leather);
    rig.setBone(hero_motion::data::Spine);
    if(trailMantle){
        const float chest=plated?10.9f:8.8f;
        rig.tapered({-9,6.8f,55},{-1,chest,47},3.2f,1.35f,leather);
        rig.tapered({-1,chest,47},{9,6.1f,35.2f},3.1f,1.38f,leather);
        rig.tapered({1.1f,chest+.23f,44.6f},{3.6f,chest-.5f,46.6f},.96f,1.0f,trim);
    }
    P3 clasp=travelCoat?P3{-2,10.3f,50}:P3{0,plated?11.f:9.4f,54};
    if(!realmArmor&&(travelCoat||armor.value.kind!=Kind::Armor||!hero_paint::cuirassLoaded)){
        rig.rounded(clasp,{travelCoat?7.f:5.f,2,travelCoat?7.f:6.f},trim);
        rig.rounded(clasp+P3{0,1.2f,0},{3.2f,1,3.2f},travelCoat?paint(0x263b45):cloth);
    }
    if(travelCoat)for(int i=0;i<4;i++){
        P3 ray=i%2?P3{0,0,i==1?1.f:-1.f}:P3{i==0?1.f:-1.f,0,0};
        rig.tapered(clasp+P3{0,1.8f,0}+ray*1.25f,clasp+P3{0,1.5f,0}+ray*2.9f,.8f,.65f,trim);
    }
    rig.setBone(hero_motion::data::Pelvis);
    const P3 bag{10.5f,4,28};
    rig.rounded(bag,{8.8f,8.1f,10.5f},shade(leather,.84f));
    rig.rounded(bag+P3{0,1.4f,2.0f},{9.3f,7.6f,6.6f},leather,flash?-1:11);
    rig.tapered(bag+P3{0,5.1f,2.2f},bag+P3{0,4.7f,-2.0f},2.3f,1.f,shade(leather,.78f));
    rig.rounded(bag+P3{0,5.4f,-.7f},{1.9f,1.3f,1.8f},trim);
    rig.setBone(hero_motion::data::Head);
    fittedGarment(rig,{0,-.8f,58.8f},{0,-1.1f,63.1f},
                  {{{7.0f,6.1f},{7.1f,6.3f},{6.7f,6.2f},{6.2f,5.8f}}},shade(skin,.81f),-1);
    // A softer collar and two shoulder plates attach to rounded upper arms.
    rig.setBone(hero_motion::data::Spine);
    fittedGarment(rig,{0,-1,55.0f},{0,-1.2f,61.5f},
                  {{{19.2f,13.5f},{19.3f,13.7f},{14.0f,11.8f},{11.8f,10.3f}}},
                  mantle,flash?-1:7,true);
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
        hero_weapon_art::quiver(rig,leather,weaponTrim);
        const auto shaft=hero_quiver::Shaft,mouth=hero_quiver::Nock+shaft*hero_quiver::MouthDepth;
        for(int i=0;i<arrows.count;i++){
            const auto nock=hero_quiver::storedNock(i,arrows.inHand,bowRelease);
            hero_weapon_art::arrow(rig,world(nock),world(shaft),world(hero_quiver::featherAcross(shaft)),paint(0xaebbb8),
                                   hero_quiver::exposedLength(nock,mouth,shaft));
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
    }else if(weapon==Kind::Dagger){
        rig.setBone(hero_motion::data::Pelvis);
        const P3 mouth=world(weapon_stow::DaggerMouth),axis=world(weapon_stow::storedAxis(weapon));
        rig.tapered({13,5,33},mouth+axis*6,2.2f,1.9f,leather);
        rig.tapered(mouth, mouth+axis*22,4.4f,2.5f,shade(weaponGrip,.63f));
        rig.tapered(mouth, mouth+axis*2,5.f,4.7f,weaponTrim);
        rig.tapered(mouth+axis*19,mouth+axis*22,3.15f,2.5f,metal);
    }
    for(float side:{-1.f,1.f}){
        int upper=side>0?hero_motion::data::UpperArmR:hero_motion::data::UpperArmL;
        int fore=side>0?hero_motion::data::ForearmR:hero_motion::data::ForearmL;
        int handBone=side>0?hero_motion::data::HandR:hero_motion::data::HandL;
        P3 shoulder=rest(upper),elbow=rest(fore),hand=rest(handBone);
        rig.setBone(upper);
        fittedGarment(rig,elbow,shoulder,{{{6.4f,7},{7.7f,8.5f},{9.5f,10.4f},{9,9.2f}}},
                      cloth,flash?-1:10);
        rig.rounded(shoulder,{9.2f,10.3f,6.7f},cloth);
        if(travelCoat){
            travelPauldron(rig,shoulder,side,paint(0x995d36),leather,trim,6,flash);
        }
        if(realmArmor){
            hero_armor_variants::shoulder(rig,armor.value.kind,shoulder,side,realmFinish);
        }else if(plated&&armor.value.kind==Kind::Armor){
            travelPauldron(rig,shoulder,side,metal,shade(metal,.76f),paint(0xbd9460),material,flash);
            if(armor.value.tier>=2)hero_armor_variants::legacyCrest(rig,armor.value.kind,shoulder+P3{side*.7f,0,1.8f},side,realmFinish);
        }else if(plated){
            const float width=hide?11.f:13.f;
            P3 shell=shoulder+P3{side*.9f,.4f,1.7f};
            const int cell=flash?-1:material;
            rig.rounded(shell,{width,15.4f,8},metal,cell,false,cell,false,true);
            rig.rounded(shell+P3{side*.6f,0,-3.5f},{width-.6f,14.2f,4.1f},shade(trim,.87f));
            if(scales||bone)rig.rounded(shell+P3{side*.9f,0,-6},{width-1.8f,11.8f,4.7f},metal);
            for(float f:{-1.f,1.f})rig.rounded(shell+P3{side*2.2f,f*6.9f,-.3f},{2,2,2},trim);
            if(itemRarity(armor.value)>=Rarity::Rare||(armor.value.kind==Kind::Armor&&armor.value.tier>=2))
                hero_armor_variants::legacyCrest(rig,armor.value.kind,shell,side,realmFinish);
        }
        rig.setBone(fore);
        rig.tapered(hand,elbow,6,7,cloth,1.15f);
        rig.rounded(elbow,{8,9,8},plated?metal:cloth);
        P3 cuff=hand+(elbow-hand)*.42f;
        rig.rounded(cuff,{7.4f,8.2f,9.6f},leather,flash?-1:4,false,flash?-1:4,true);
        rig.setBone(handBone);
        const auto wrist=!archer&&!rolling?hand+(elbow-hand)*.18f:hand+P3{0,-.6f,3.8f};
        rig.rounded(wrist,{5.25f,5.7f,4.8f},shade(leather,.77f));
        rig.rounded(hand+P3{0,-.35f,.7f},{5.5f,4.7f,6.3f},leather,flash?-1:4,false,flash?-1:4,true);
        for(int i=0;i<4;i++){
            const float x=(i-1.5f)*1.33f*side,z=-std::abs(i-1.f)*.35f;
            const P3 knuckle=hand+P3{x,1.15f,.25f+z},joint=hand+P3{x,2.0f,-1.35f+z};
            rig.tapered(knuckle,joint,1.40f,1.37f,shade(leather,1.08f),.91f);
            rig.tapered(joint,hand+P3{x-side*.16f,1.36f,-2.82f+z},1.24f,1.25f,skinLight,.82f);
        }
        rig.tapered(hand+P3{-side*2.35f,.15f,1.95f},hand+P3{-side*2.7f,2.3f,-.45f},
                    1.77f,1.64f,skin,.81f);
        if(side>0&&jewel.shape==Kind::Ring){
            rig.rounded(hand+P3{1,2,1},{3.5f,5.5f,3},paint(jewel.paint.trim));
            rig.rounded(hand+P3{1,4.6f,1.5f},{3.5f,2.2f,3.5f},paint(jewel.paint.accent));
        }
        if(archer&&handBone==hero_quiver::BowHand){
            rig.setBone(-1);
            P3 grip=world(pose.world[hero_quiver::BowHand].p);
            P3 axis=world(pose.vector(hero_quiver::BowHand,{0,0,1}));
            P3 forward=world(pose.vector(hero_quiver::BowHand,{0,1,0}));
            const float tension=bowShot.tension*bowShot.stringContact;
            const float curve=bowProfile.curve*(1-.15f*tension)-bowProfile.recurve*.29f;
            P3 string=grip-forward*curve;
            if(bowDraw>=0&&!rolling)
                string=string*(1-bowShot.stringContact)+world(pose.world[hero_quiver::DrawHand].p)*bowShot.stringContact;
            hero_weapon_art::bow(rig,grip,axis,forward,string,weaponFinish,held.value.kind,bowDraw<0?-1.f:tension);
            if(arrows.inHand){
                const auto arrow=hero_quiver::carried(pose,bowShot.arrowAxis,bowShot.arrowGripOffset,bowPreparation);
                hero_weapon_art::arrow(rig,world(arrow.nock),world(arrow.axis),world(arrow.across),paint(0xaebbb8),arrow.exposed);
            }
        }
        if(side>0&&spearing){
            rig.setBone(-1);
            if(stowPhase>0)hero_weapon_art::spear(rig,world(extra.grip),hero_weapon_art::basis(world(extra.axis),world(extra.across)),extra.spearSlide,weaponFinish,held.value.kind);
            else hero_weapon_art::spear(rig,world(pose.world[hero_motion::data::HandR].p),
                 world(pose.world[hero_motion::data::HandL].p),spearSlide,weaponFinish,held.value.kind);
        }else if(side>0&&held.present()&&!archer){
            rig.setBone(hero_motion::data::Weapon);
            hero_weapon_art::melee(rig,hand,held.value.kind,exposedBlade,weaponFinish);
        }
        if(side<0&&shield){
            rig.setAttachment(&shieldMount);
            for(float z:{-3.3f,1.3f}){
                rig.tapered({0,-5,z},{0,-1.5f,z},1.7f,1.6f,leather);
            }
            if(round_shield_art::has(ward.value.kind))round_shield_art::draw(rig,ward,flash);
            else{
                gearPlate(rig,equipment_shape::Shield,equipment_shape::ShieldTriangles,{},{8.7f,0,0},{0,0,8.7f},{0,hero_carry::ShieldRimDepth,0},paint(ward.paint.trim));
                gearPlate(rig,equipment_shape::Shield,equipment_shape::ShieldTriangles,{0,2,0},{7.45f,0,0},{0,0,7.45f},{0,1.4f,0},paint(ward.paint.primary));
                for(float x:{-.6f,0.f,.6f}){rig.tapered({x*8.7f,3,8},{x*8.7f,3,0},.65f,.65f,paint(ward.paint.secondary));rig.tapered({x*8.7f,3,0},{x*3.9f,3,-10},.65f,.65f,paint(ward.paint.secondary));}
                rig.rounded({0,3.6f,1.3f},{5.5f,2.5f,5.5f},paint(ward.paint.trim));
                rig.rounded({0,5,1.3f},{2.4f,1.3f,2.4f},paint(ward.paint.accent));
            }
            rig.setAttachment(nullptr);
        }
    }
    rig.setBone(hero_motion::data::Head);
    float head=70+lift;
    const Kind headgear=headwear.value.kind;
    if(helmet&&hero_headwear::closed(headgear)){
        hero_headwear::drawClosed(rig,head,headwear,flash);
        rig.setSkeleton(nullptr);
        return;
    }
    if(hero_paint::headsLoaded&&hero_paint::atlas.id){
        rig.paintedHead({0,-.4f,head+1.3f},flash,helmet);
        if(helmet){
            if(hero_headwear::hood(headgear))hero_headwear::drawHood(rig,head,headwear,flash);
            else hero_headwear::drawOpen(rig,head,headwear,flash);
        }
        rig.setSkeleton(nullptr);
        return;
    }
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
    if(helmet&&hero_headwear::hood(headgear)){
        hero_headwear::drawHood(rig,head,headwear,flash);
        rig.setSkeleton(nullptr);
        return;
    }
    if(helmet){
        hero_headwear::drawOpen(rig,head,headwear,flash);
    }else{
        const Color hair=flash?Text:hero_paint::HairColor;
        rig.rounded({-.85f,-3.45f,head+4.2f},{19.5f,18,17.6f},hair,flash?-1:1,false,flash?-1:2);
        rig.rounded({-5.4f,3.6f,head+6.4f},{6.8f,8.1f,5.4f},hair,flash?-1:1);
    }
    rig.setSkeleton(nullptr);
}
