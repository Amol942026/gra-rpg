#pragma once

// Included after Rig. Weapon coordinates stay attached to the existing grip
// bones, including when a sword is handed off to the scabbard.
namespace hero_weapon_art {
struct Finish {
    Color body,grip,trim,accent;
    bool flash;
    int texture(int cell) const {return flash?-1:cell;}
};
inline Finish finish(const GearAppearance& look,bool flash=false){
    auto paint=[flash](unsigned value){return flash?Text:equipment_art::color(value);};
    return {paint(look.paint.primary),paint(look.paint.secondary),paint(look.paint.trim),paint(look.paint.accent),flash};
}

inline P3 cross(P3 a,P3 b){
    return {a.f*b.z-a.z*b.f,a.z*b.r-a.r*b.z,a.r*b.f-a.f*b.r};
}
inline int bladeMaterial(Kind kind){
    if(realmWeapon(kind))return hero_paint::RealmWeaponFirstCell+int(kind)-FirstRealmWeapon;
    switch(kind){
        case Kind::BasaltCleaver:case Kind::GlassDagger:case Kind::FlintDagger:return hero_paint::Obsidian;
        case Kind::WolfKnife:case Kind::TuskAxe:case Kind::StingDagger:case Kind::ScorpionAxe:return hero_paint::Ivory;
        case Kind::CopperHatchet:case Kind::ScarabAxe:case Kind::SunTrailSword:case Kind::DuneBlade:return hero_paint::Bronze;
        default:return hero_paint::Forged;
    }
}

struct Basis {P3 axis,across,normal;};
inline Basis basis(P3 axis,P3 across){
    const float norm=std::hypot(std::hypot(axis.r,axis.f),axis.z);
    axis=axis*(1/std::max(.001f,norm));
    P3 normal=cross(across,axis);
    const float side=std::hypot(std::hypot(normal.r,normal.f),normal.z);
    normal=normal*(1/std::max(.001f,side));
    return {axis,cross(axis,normal),normal};
}
inline Basis heldBasis(){
    const P3 axis=swordPoint(sword_pose::idleBlade());
    return basis(axis,{1,0,0});
}
inline const auto axeFaces=[](){
    std::array<equipment_shape::Triangles<10>,ItemKindCount> result{};
    for(int i=1;i<ItemKindCount;i++)if(archetype(Kind(i))==Kind::Axe)
        result[i]=equipment_shape::triangulate(weapon_profiles::axe(Kind(i)).bit);
    return result;
}();
inline const auto spearFaces=[](){
    std::array<equipment_shape::Triangles<7>,SpearCount> result{};
    for(int i=0;i<SpearCount;i++)
        result[i]=equipment_shape::triangulate(weapon_profiles::spear(Kind(FirstSpear+i)).point);
    return result;
}();
inline std::array<equipment_shape::Point,9> bladeContour(Kind kind,const weapon_profiles::Blade& p,float bend){
    const float w=p.width;
    const float bevelStart=kind==Kind::SporeDagger?.23f:kind==Kind::BriarSword?.30f:.39f;
    return {{
        {-w*.52f,0},{bend*.13f-w*bevelStart,.29f},{bend*.53f-w*.95f,p.shoulder},
        {bend*.86f-w*.46f,(1+p.shoulder)*.5f},{bend,1},
        {bend*.74f+w*.51f,(1+p.shoulder)*.5f},{bend*.49f+w*.91f,p.shoulder},
        {bend*.10f+w*.46f,.28f},{w*.49f,0}
    }};
}
inline const auto bladeFaces=[](){
    std::array<equipment_shape::Triangles<9>,ItemKindCount> result{};
    for(int i=1;i<ItemKindCount;i++){
        const Kind kind=Kind(i),shape=archetype(kind);
        if(shape!=Kind::Sword&&shape!=Kind::Dagger)continue;
        const auto p=weapon_profiles::blade(kind);
        result[i]=equipment_shape::triangulate(bladeContour(kind,p,p.bend));
    }
    return result;
}();

template<std::size_t N>
inline void bevel(Rig& rig,const std::array<equipment_shape::Point,N>& outline,
                  const equipment_shape::Triangles<N>& triangles,P3 center,P3 across,P3 length,
                  P3 depth,int cell,const Finish& finish,bool axe){
    float left=outline[0].x,right=left,bottom=outline[0].y,top=bottom,winding=0;
    for(std::size_t i=0;i<N;i++){
        const auto p=outline[i],q=outline[(i+1)%N];
        left=std::min(left,p.x);right=std::max(right,p.x);
        bottom=std::min(bottom,p.y);top=std::max(top,p.y);
        winding+=p.x*q.y-p.y*q.x;
    }
    auto point=[&](std::size_t i,float side,bool inset){
        const auto p=outline[i];
        const float x=inset?p.x*.72f:p.x;
        const float y=inset?p.y*.85f+(axe?1.18f:.28f)*.15f:p.y;
        return center+across*x+length*y+depth*side;
    };
    auto uv=[&](int i){
        return Vector2{(outline[i].x-left)/(right-left),(top-outline[i].y)/(top-bottom)};
    };
    for(float side:{1.f,-1.f}){
        const Color core=shade(finish.body,side>0?1.04f:.83f);
        for(int i=0;i<triangles.count;i++){
            auto [a,b,c]=triangles.indices[i];
            if(side<0)std::swap(b,c);
            rig.paintedWeaponFace({point(a,side,true),point(b,side,true),point(c,side,true),point(c,side,true)},
                                  {uv(a),uv(b),uv(c),uv(c)},finish.texture(cell),core);
        }
        for(std::size_t i=0;i<N;i++){
            const std::size_t j=(i+1)%N;
            P3 a=point(i,0,false),b=point(j,0,false),c=point(j,side,true),d=point(i,side,true);
            if((winding>0)!=(side>0)){std::swap(a,b);std::swap(c,d);}
            const float edge=(outline[i].x+outline[j].x)*side;
            const float light=std::clamp(1.18f+edge*.23f,.78f,1.42f);
            rig.face(a,b,c,d,finish.body,light);
        }
    }
}

// Six small faces define an oval cross-section. The front and back receive
// the same grain coordinates as the strip bends with the bowstring.
inline void strip(Rig& rig,P3 a,P3 b,P3 across,P3 normal,float widthA,float widthB,
                  float v0,float v1,Color color,int cell){
    constexpr float edge=.60f;
    const std::array<P3,6> ring{across,across*edge+normal,across*-edge+normal,
                               across*-1,across*-edge-normal,across*edge-normal};
    for(std::size_t i=0;i<ring.size();i++){
        const std::size_t j=(i+1)%ring.size();
        const float light=i<3?1.06f:.82f;
        const float u0=float(i)/ring.size(),u1=float(i+1)/ring.size();
        rig.paintedWeaponFace({a+ring[j]*widthA,a+ring[i]*widthA,b+ring[i]*widthB,b+ring[j]*widthB},
                              {{{u1,v0},{u0,v0},{u0,v1},{u1,v1}}},cell,shade(color,light));
    }
}

inline void spear(Rig& rig,P3 rear,Basis frame,float slide,const Finish& finish,Kind kind){
    const auto [axis,across,normal]=frame;
    const auto profile=weapon_profiles::spear(kind);
    const P3 shaft=rear+axis*slide;
    // Extend the ends without moving the binding or either hand's contact.
    const P3 heel=shaft+axis*weapon_profiles::SpearHeel;
    const P3 socket=shaft+axis*weapon_profiles::SpearSocket;
    strip(rig,heel,socket,across,normal*.80f,profile.shaft,profile.shaft*1.1f,0,1,finish.grip,finish.texture(hero_paint::Wood));
    strip(rig,shaft-axis*3.5f,shaft+axis*(16.7f*profile.wrap),across,normal*.83f,
          profile.shaft*1.21f,profile.shaft*1.21f,0,1,shade(finish.grip,.83f),finish.texture(hero_paint::Binding));
    for(float at:{weapon_profiles::SpearHeel+1.f,-6.f,16.7f*profile.wrap+1.1f,
                  weapon_profiles::SpearSocket-1.7f,weapon_profiles::SpearSocket+.3f}){
        const P3 band=shaft+axis*at;
        strip(rig,band-axis*.7f,band+axis*.7f,across,normal*.82f,profile.shaft*1.31f,profile.shaft*1.31f,0,1,finish.trim,-1);
    }
    const int cell=hero_paint::SpearFirstCell+int(kind)-FirstSpear;
    const P3 width=across*profile.halfWidth,length=axis*profile.bladeLength();
    bevel(rig,profile.point,spearFaces[int(kind)-FirstSpear],socket,width,length,normal*.68f,cell,finish,false);
    if(kind!=Kind::Spear)for(float side:{-1.f,1.f}){
        const P3 center=socket+normal*(side*.77f);
        const P3 inlay=length*(kind==Kind::BasaltSpear?.72f:.62f)+across*(kind==Kind::BriarSpear?.59f:.28f);
        rig.beam(center+length*.17f,center+inlay,.46f,.46f,finish.accent);
        if(kind==Kind::OasisSpear||kind==Kind::DawnSpear)
            rig.rounded(center+length*.15f,{1.35f,1.35f,1.35f},finish.accent);
    }
}
inline void spear(Rig& rig,P3 rear,P3 front,float slide,const Finish& finish,Kind kind){
    const P3 axis=front-rear;
    const P3 across=std::hypot(axis.r,axis.f)>.001f?P3{axis.f,-axis.r,0}:P3{1,0,0};
    spear(rig,rear,basis(axis,across),slide,finish,kind);
}

inline void bow(Rig& rig,P3 grip,P3 axis,P3 forward,P3 string,const Finish& finish,Kind kind,float charge,bool arrow){
    const P3 across=cross(axis,forward);
    const auto profile=weapon_profiles::bow(kind);
    const float curve=profile.curve*(1-.15f*std::max(0.f,charge));
    auto limb=[&](float t){
        const float s=2*t-1,outer=std::abs(s);
        const float sweep=std::pow(std::max(0.f,std::sin(t*PI)),profile.shoulder);
        const float recurve=profile.recurve*(std::pow(outer,9.f)-std::pow(outer,4.f)*.71f);
        return grip+axis*(profile.half*s)+forward*(curve*(sweep-1)+recurve);
    };
    auto width=[&](float t){
        float section=std::abs(2*t-1);
        const float taper=profile.width+(profile.tip-profile.width)*std::pow(section,1.35f);
        if(kind==Kind::RiftBow)return taper+.15f+std::sin(section*22)*section*.38f;
        if(kind==Kind::ReedBow)return taper+.29f;
        return taper;
    };
    const P3 bottom=limb(0),top=limb(1);
    P3 previous=bottom;
    const int limbCell=realmWeapon(kind)?bladeMaterial(kind):kind==Kind::GraveBow?hero_paint::Ivory:hero_paint::Wood;
    for(int part=1;part<=16;part++){
        const float t=part/16.f,from=(part-1)/16.f;
        P3 next=limb(t);
        const bool wrapped=part>=7&&part<=10;
        const bool horn=(kind==Kind::CinderBow||kind==Kind::GraveBow)&&(part<=2||part>=15);
        strip(rig,previous,next,across,forward*.64f,width(from),width(t),
              from,t,wrapped?finish.grip:horn?finish.accent:finish.body,
              finish.texture(wrapped?hero_paint::Binding:horn?hero_paint::Ivory:limbCell));
        previous=next;
    }
    for(float t:{.035f,.16f,.37f,.63f,.84f,.965f}){
        const float w=std::min(1.60f,width(t)+.19f);
        strip(rig,limb(t-.014f),limb(t+.014f),across,forward*.72f,w,w,0,1,
              t==.16f||t==.84f?finish.accent:finish.trim,-1);
    }
    if(kind==Kind::RiftBow){
        for(float t:{.23f,.30f,.70f,.77f}){
            const float w=std::min(1.58f,width(t)+.24f);
            strip(rig,limb(t-.017f),limb(t+.017f),across,forward*.70f,w,w,0,1,finish.trim,-1);
        }
        rig.form(limb(.61f)+forward*.46f,{2.7f,2.3f,3.4f},finish.accent);
    }
    const Color cord=shade(Text,.88f);
    if(charge<0)rig.beam(bottom,top,.68f,.68f,cord);
    else{
        rig.beam(bottom,string,.57f,.57f,cord);
        rig.beam(string,top,.57f,.57f,cord);
    }
    if(arrow){
        P3 tip=grip+P3{0,20,0},nock=string-P3{0,3,0};
        P3 shaft=tip-nock;
        rig.beam(nock,tip,1.15f,1.15f,rgb(169,126,77));
        rig.form(tip,{3.6f,6.f,2.7f},finish.trim);
        for(P3 vane:{P3{0,0,2.1f},across*1.75f,across*-1.75f}){
            P3 a=nock+shaft*.06f,b=nock+shaft*.25f,c=nock+shaft*.11f+vane;
            rig.face(a,b,c,c,Text,.94f);rig.face(c,b,a,a,Text,.78f);
        }
    }
}

inline void melee(Rig& rig,P3 hand,Basis frame,Kind kind,float visible,const Finish& finish){
    const Kind shape=archetype(kind);
    const bool dagger=shape==Kind::Dagger;
    const auto [axis,across,normal]=frame;
    const int cell=bladeMaterial(kind);
    if(shape==Kind::Axe){
        const auto profile=weapon_profiles::axe(kind);
        const P3 heel=hand-axis*profile.butt,tip=hand+axis*profile.haftEnd;
        const float width=profile.width;
        strip(rig,heel,tip,across,normal*.83f,width*.95f,width,0,1,finish.grip,finish.texture(hero_paint::Wood));
        strip(rig,heel,hand+axis*5.2f,across,normal*.87f,width*1.18f,width*1.18f,
              0,1,finish.grip,finish.texture(hero_paint::Binding));
        for(float along:{-profile.butt+1.f,-5.f,4.f,profile.haftEnd-4.f}){
            const P3 band=hand+axis*along;
            strip(rig,band-axis*.65f,band+axis*.65f,across,normal*.85f,width*1.26f,width*1.26f,0,1,finish.trim,-1);
        }
        const P3 head=hand+axis*(profile.haftEnd-17.5f);
        const P3 u=across*9.25f,v=axis*9.9f,depth=normal*1.12f;
        bevel(rig,profile.bit,axeFaces[int(kind)],head,u,v,depth,cell,finish,true);
        const P3 socket=head+v*1.14f;
        rig.beam(socket-normal*1.6f,socket+normal*1.6f,3.1f,3.1f,finish.trim);
        rig.beam(socket-normal*1.75f,socket+normal*1.75f,1.45f,1.45f,finish.accent);
        for(float side:{-1.f,1.f}){
            const P3 inlay=head-u*.64f+v*1.19f+normal*(side*1.18f);
            if(kind==Kind::OathAxe||kind==Kind::GuardianAxe){
                rig.beam(inlay-v*.23f,inlay+v*.23f,.69f,.69f,finish.accent);
                rig.beam(inlay-u*.16f,inlay+v*.12f,.60f,.60f,finish.accent);
                rig.beam(inlay+u*.16f,inlay+v*.12f,.60f,.60f,finish.accent);
            }else if(kind==Kind::ScarabAxe||kind==Kind::ScorpionAxe)
                for(float z:{-.17f,.03f,.23f})rig.beam(inlay+v*z-u*.16f,inlay+v*(z+.08f)+u*.08f,.62f,.62f,finish.trim);
            else if(kind==Kind::BasaltCleaver||kind==Kind::AshHookAxe)
                rig.beam(inlay-v*.24f-u*.12f,inlay+v*.26f+u*.18f,.62f,.62f,finish.accent);
        }
        return;
    }
    const auto p=weapon_profiles::blade(kind);
    const float guardDistance=dagger?2.45f:3.7f;
    const P3 guard=hand+axis*guardDistance;
    const float exposed=std::max(0.f,(dagger?24.f:37.f)*visible-guardDistance);
    const float length=std::min(exposed,p.length-guardDistance);
    if(length>.45f){
        const float bend=p.bend*(length/(p.length-guardDistance));
        const auto contour=bladeContour(kind,p,bend);
        const P3 blade=axis*length;
        const auto faces=std::abs(bend-p.bend)<.01f?bladeFaces[int(kind)]:equipment_shape::triangulate(contour);
        bevel(rig,contour,faces,guard,across,blade,normal*.68f,cell,finish,false);
        if(visible>.62f)for(float side:{-1.f,1.f}){
            const P3 root=guard+normal*(side*.75f);
            const Color fuller=kind==Kind::Sword||kind==Kind::TrailFalchion?shade(finish.body,.72f):finish.accent;
            P3 a=root+blade*.12f;
            const P3 b=root+blade*.43f+across*(bend*.21f),c=root+blade*p.point+across*(bend*.57f);
            rig.beam(a,b,.58f,.54f,fuller);rig.beam(b,c,.54f,.29f,fuller);
            if(kind==Kind::CryptSword||kind==Kind::GuardianSword||kind==Kind::SporeDagger)
                for(float mark:{.22f,.38f,.54f}){
                    const P3 rune=root+blade*mark+across*(bend*mark*.55f);
                    rig.beam(rune-across*.9f,rune+across*.9f+blade*.035f,.63f,.44f,finish.trim);
                }
        }
    }
    const P3 pommel=hand-axis*(dagger?5.3f:8.15f);
    strip(rig,pommel,hand+axis*(dagger?1.44f:2.22f),across,normal*.87f,1.48f,1.48f,0,1,finish.grip,finish.texture(hero_paint::Binding));
    for(float side:{-1.f,1.f}){
        const float span=p.guard*(kind==Kind::StingDagger&&side<0?.68f:1.f);
        const P3 quillon=guard+across*(side*span)+axis*(span*p.hook);
        const P3 elbow=guard+across*(side*span*.52f)-axis*.72f;
        rig.tapered(guard,elbow,2.10f,1.90f,finish.trim,.81f);
        rig.tapered(elbow,quillon,1.68f,1.60f,finish.trim,.72f);
    }
    rig.rounded(guard,{3.3f,2.4f,2.5f},finish.trim);
    rig.rounded(pommel,{2*p.pommel,2*p.pommel,1.8f*p.pommel},finish.trim);
    if(beastGear(kind))rig.rounded(pommel+normal*1.38f,{2.15f,2.15f,2.15f},finish.accent);
}
inline void melee(Rig& rig,P3 hand,Kind kind,float visible,const Finish& finish){
    melee(rig,hand,heldBasis(),kind,visible,finish);
}
}
