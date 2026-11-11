#pragma once

// Included after Rig. Weapon coordinates stay attached to the existing grip
// bones, including when a sword is handed off to the scabbard.
namespace hero_weapon_art {
struct Finish {
    Color body,grip,trim,accent;
    bool flash;
    int texture(int cell) const {return flash?-1:cell;}
};

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

inline constexpr std::array<equipment_shape::Point,7> BriarBlade{{
    {-.11f,-.65f},{-.18f,.42f},{-.27f,1.4f},{0,1.9f},{.27f,1.4f},{.18f,.42f},{.11f,-.65f}
}};
inline constexpr std::array<equipment_shape::Point,7> MirageBlade{{
    {-.12f,-.65f},{-.15f,.30f},{-.27f,1.4f},{0,1.9f},{.16f,1.4f},{.17f,.50f},{.14f,-.65f}
}};
inline constexpr std::array<equipment_shape::Point,8> SporeBlade{{
    {-.15f,-.65f},{-.20f,-.10f},{-.13f,.18f},{-.235f,.67f},{0,1.25f},{.235f,.67f},{.15f,-.10f},{.15f,-.65f}
}};
inline constexpr std::array<equipment_shape::Point,11> HookHead{{
    {-.1f,.66f},{-.45f,.82f},{-1.1f,.5f},{-1.35f,.9f},{-1.04f,1.12f},{-1.35f,1.6f},
    {-.7f,1.85f},{-.15f,1.55f},{.48f,1.62f},{.7f,1.25f},{.44f,.86f}
}};
inline constexpr auto BriarTriangles=equipment_shape::triangulate(BriarBlade);
inline constexpr auto MirageTriangles=equipment_shape::triangulate(MirageBlade);
inline constexpr auto SporeTriangles=equipment_shape::triangulate(SporeBlade);
inline constexpr auto HookTriangles=equipment_shape::triangulate(HookHead);
static_assert(BriarTriangles.count==5&&MirageTriangles.count==5&&SporeTriangles.count==6&&HookTriangles.count==9);

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

inline constexpr std::array<equipment_shape::Point,6> SpearLeaf{{
    {-.24f,-.12f},{-.52f,.28f},{-.34f,.65f},{0,1.16f},{.38f,.60f},{.44f,.21f}
}};
inline constexpr std::array<equipment_shape::Point,7> SpearWing{{
    {-.24f,-.12f},{-.66f,.26f},{-.41f,.32f},{0,1.19f},{.40f,.34f},{.65f,.23f},{.21f,-.12f}
}};
inline constexpr auto SpearLeafFaces=equipment_shape::triangulate(SpearLeaf);
inline constexpr auto SpearWingFaces=equipment_shape::triangulate(SpearWing);
static_assert(SpearLeafFaces.count==4&&SpearWingFaces.count==5);

inline void spear(Rig& rig,P3 rear,P3 front,const Finish& finish,Kind kind){
    P3 axis=front-rear;
    axis=axis*(1/std::max(.001f,std::hypot(std::hypot(axis.r,axis.f),axis.z)));
    const float flat=std::hypot(axis.r,axis.f);
    const P3 across=flat>.001f?P3{axis.f/flat,-axis.r/flat,0}:P3{1,0,0};
    const P3 normal=cross(across,axis);
    const P3 heel=rear-axis*21.f,socket=rear+axis*31.5f;
    strip(rig,heel,socket,across,normal*.80f,1.11f,1.22f,0,1,finish.grip,finish.texture(hero_paint::Wood));
    strip(rig,rear-axis*3.5f,front+axis*3.3f,across,normal*.83f,1.35f,1.35f,
          0,1,shade(finish.grip,.83f),finish.texture(hero_paint::Binding));
    for(float at:{-20.f,-6.f,17.8f,29.8f,31.8f}){
        const P3 band=rear+axis*at;
        strip(rig,band-axis*.7f,band+axis*.7f,across,normal*.82f,1.48f,1.48f,0,1,finish.trim,-1);
    }
    const int cell=hero_paint::SpearFirstCell+int(kind)-FirstSpear;
    const bool winged=kind==Kind::OasisSpear||kind==Kind::DawnSpear;
    const P3 width=across*(kind==Kind::BriarSpear?4.5f:kind==Kind::BasaltSpear?6.25f:5.35f);
    const P3 length=axis*(kind==Kind::DawnSpear?12.f:10.9f);
    if(winged)bevel(rig,SpearWing,SpearWingFaces,socket,width,length,normal*.68f,cell,finish,false);
    else bevel(rig,SpearLeaf,SpearLeafFaces,socket,width,length,normal*.68f,cell,finish,false);
    if(kind==Kind::BasaltSpear||kind==Kind::DawnSpear)for(float side:{-1.f,1.f}){
        const P3 center=socket+normal*(side*.76f);
        rig.beam(center+length*.17f,center+length*.67f+across*.38f,.48f,.48f,finish.accent);
    }
}

inline void bow(Rig& rig,P3 grip,P3 axis,P3 forward,P3 string,const Finish& finish,Kind kind,float charge,bool arrow){
    const P3 across=cross(axis,forward);
    const float half=equipment_shape::BowHalfLength,curve=equipment_shape::BowCurve*(1-.15f*charge);
    auto limb=[&](float t){return grip+axis*(half*(2*t-1))+forward*(curve*(std::sin(t*PI)-1));};
    const float widthScale=kind==Kind::ReedBow?.85f:1.f;
    auto width=[&](float t){return widthScale*(1.56f-.94f*std::pow(std::abs(2*t-1),1.35f));};
    const P3 bottom=limb(0),top=limb(1);
    P3 previous=bottom;
    const int limbCell=realmWeapon(kind)?bladeMaterial(kind):kind==Kind::GraveBow?hero_paint::Ivory:hero_paint::Wood;
    for(int part=1;part<=16;part++){
        const float t=part/16.f,from=(part-1)/16.f;
        P3 next=limb(t);
        const bool wrapped=part>=7&&part<=10;
        const bool horn=kind==Kind::CinderBow&&(part<=2||part>=15);
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
    rig.beam(bottom,string,.67f,.67f,cord);
    rig.beam(string,top,.67f,.67f,cord);
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

inline void melee(Rig& rig,P3 hand,Kind kind,float visible,const Finish& finish){
    const Kind shape=archetype(kind);
    const bool dagger=shape==Kind::Dagger,axe=shape==Kind::Axe;
    const P3 blade=swordPoint(sword_pose::idleBlade(0))*(dagger?.65f:1.f);
    const float flat=std::max(.001f,std::hypot(blade.r,blade.f));
    const float length=std::hypot(flat,blade.z);
    const P3 across{blade.f/flat,-blade.r/flat,0},normal=cross(across,blade)*(1/length);
    const P3 tip=hand+blade*visible,guard=hand+blade*.1f;
    const int cell=bladeMaterial(kind);
    if(axe){
        const P3 heel=hand-blade*.28f,head=hand+blade*.45f;
        strip(rig,heel,tip,across,normal*.83f,1.22f,1.1f,0,1,finish.grip,finish.texture(hero_paint::Wood));
        strip(rig,heel,hand+blade*.08f,across,normal*.87f,1.30f,1.31f,0,1,finish.grip,finish.texture(hero_paint::Binding));
        rig.rounded(heel,{2.65f,2.5f,2.5f},finish.trim);
        const P3 u=across*(length*.30f),v=blade*.30f,depth=normal*1.17f;
        if(kind==Kind::AshHookAxe)bevel(rig,HookHead,HookTriangles,head,u,v,depth,cell,finish,true);
        else bevel(rig,equipment_shape::Axe,equipment_shape::AxeTriangles,head,u,v,depth,cell,finish,true);
        P3 socket=head+blade*.335f;
        rig.beam(socket-normal*1.6f,socket+normal*1.6f,3.1f,3.1f,finish.trim);
        rig.beam(socket-normal*1.75f,socket+normal*1.75f,1.45f,1.45f,finish.accent);
        if(kind==Kind::OathAxe)for(float side:{-1.f,1.f}){
            const P3 p=head+u*-.61f+v*1.22f+depth*(side*1.03f);
            rig.beam(p-v*.19f,p+v*.19f,.65f,.65f,finish.accent);
            rig.beam(p-u*.13f,p+v*.11f,.65f,.65f,finish.accent);
            rig.beam(p+u*.13f,p+v*.11f,.65f,.65f,finish.accent);
        }
        return;
    }
    if(visible>.115f){
        const P3 base=dagger?hand-blade*.12f:guard;
        const P3 v=(tip-base)*(1/(dagger?1.9f:2.55f));
        const P3 origin=base+v*.65f;
        const float width=std::hypot(std::hypot(v.r,v.f),v.z);
        const P3 u=across*width,depth=normal*.7f;
        if(kind==Kind::BriarSword)bevel(rig,BriarBlade,BriarTriangles,origin,u,v,depth,cell,finish,false);
        else if(kind==Kind::MirageSword)bevel(rig,MirageBlade,MirageTriangles,origin,u,v,depth,cell,finish,false);
        else if(kind==Kind::SporeDagger)bevel(rig,SporeBlade,SporeTriangles,origin,u,v,depth,cell,finish,false);
        else if(dagger)bevel(rig,equipment_shape::Dagger,equipment_shape::DaggerTriangles,origin,u,v,depth,cell,finish,false);
        else bevel(rig,equipment_shape::Sword,equipment_shape::SwordTriangles,origin,u,v,depth,cell,finish,false);
        if(beastGear(kind)){
            for(float side:{-1.f,1.f})for(float mark:{.23f,.39f,.55f}){
                P3 p=base+(tip-base)*mark+normal*(side*.75f);
                P3 x=across*(dagger?.43f:.59f),y=blade*(dagger?.030f:.027f);
                if(side>0)rig.face(p-x,p-y,p+x,p+y,finish.accent,.94f);
                else rig.face(p+y,p+x,p-y,p-x,finish.accent,.85f);
            }
        }
        if(visible>.62f&&(kind==Kind::EmberSword||kind==Kind::MirageSword)){
            for(float side:{-1.f,1.f}){
                const P3 a=base+(tip-base)*.20f+depth*(side*1.08f);
                const P3 b=base+(tip-base)*.52f+depth*(side*1.08f)+across*.5f;
                const P3 c=base+(tip-base)*.79f+depth*(side*1.08f)-across*.22f;
                rig.beam(a,b,.48f,.48f,finish.accent);
                rig.beam(b,c,.40f,.40f,finish.accent);
            }
        }
    }
    const P3 pommel=hand-blade*.22f;
    strip(rig,pommel,hand+blade*.06f,across,normal*.87f,1.48f,1.48f,0,1,finish.grip,finish.texture(hero_paint::Binding));
    for(float side:{-1.f,1.f}){
        const float span=dagger?5.f:7.f;
        const P3 quillon=guard+across*(side*span);
        const P3 elbow=guard+across*(side*span*.52f)-blade*.034f;
        rig.tapered(guard,elbow,2.10f,1.90f,finish.trim,.81f);
        rig.tapered(elbow,quillon,1.68f,1.60f,finish.trim,.72f);
    }
    rig.rounded(guard,{3.3f,2.4f,2.5f},finish.trim);
    rig.rounded(pommel,{4.1f,4.1f,4.1f},finish.trim);
    if(beastGear(kind))rig.rounded(pommel+normal*1.38f,{2.15f,2.15f,2.15f},finish.accent);
}
}
