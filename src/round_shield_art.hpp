#pragma once

// Included inside actor_art after Rig. The same face and fittings are drawn
// on the carried shield and in the inventory icon.
namespace round_shield_art {
inline constexpr bool has(Kind kind){
    return kind==Kind::IronwoodRoundshield||kind==Kind::SunwheelShield||kind==Kind::ObsidianRoundshield;
}
inline constexpr int Segments=24;
struct Band {float radius,depth;Color paint;};

inline void draw(Rig& rig,const GearAppearance& look,bool flash=false){
    const Kind kind=look.value.kind;
    const bool timber=kind==Kind::IronwoodRoundshield,sun=kind==Kind::SunwheelShield;
    const auto paint=[flash](unsigned code){return flash?Text:equipment_art::color(code);};
    const Color base=paint(look.paint.primary),back=paint(look.paint.secondary),
                rim=paint(look.paint.trim),inlay=paint(look.paint.accent);
    const float radius=timber?11.9f:sun?12.6f:11.7f;
    static const auto edge=[]{
        std::array<P3,Segments> points{};
        for(int i=0;i<Segments;i++){
            const float angle=2*PI*i/Segments;
            points[i]={std::cos(angle),0,std::sin(angle)};
        }
        return points;
    }();
    const std::array<Band,8> bands{{
        {0,-1.75f,shade(back,.86f)}, {radius-.75f,-1.75f,back},
        {radius,-.45f,shade(rim,.66f)}, {radius-.05f,1.f,rim},
        {radius-.62f,1.95f,shade(rim,1.14f)}, {radius-1.52f,2.05f,sun?shade(inlay,.76f):shade(rim,.87f)},
        {radius-2.25f,1.72f,base}, {0,3.1f,shade(base,1.16f)}
    }};
    for(size_t b=1;b<bands.size();b++){
        const auto& outer=bands[b-1];
        const auto& inner=bands[b];
        for(int i=0;i<Segments;i++){
            const int j=(i+1)%Segments;
            const float lightA=.99f-.13f*edge[i].r+.12f*edge[i].z;
            const float lightB=.99f-.13f*edge[j].r+.12f*edge[j].z;
            const float panel=b==bands.size()-1&&!sun&&!timber&&i%8<4?.79f:1.f;
            const P3 a=edge[i]*outer.radius+P3{0,outer.depth,0};
            const P3 bPoint=edge[i]*inner.radius+P3{0,inner.depth,0};
            const P3 c=edge[j]*inner.radius+P3{0,inner.depth,0};
            const P3 d=edge[j]*outer.radius+P3{0,outer.depth,0};
            rig.surfaceQuad(a,bPoint,c,d,shade(outer.paint,lightA*panel),shade(inner.paint,lightA*panel),
                            shade(inner.paint,lightB*panel),shade(outer.paint,lightB*panel));
        }
    }
    const float field=radius-2.25f;
    const auto onFace=[field](float x,float z){
        return P3{x,3.25f-1.38f*std::min(1.f,std::hypot(x,z)/field),z};
    };
    const auto diamond=[&](P3 center,P3 across,P3 up,float width,float height,Color color){
        const P3 a=center-across*width,b=center+up*height,c=center+across*width,d=center-up*height;
        rig.face(a,b,c,d,color);
    };
    if(timber){
        // Split wood planks, grain and a forged boss with a counterchange pin.
        for(float x:{-5.45f,-2.4f,2.4f,5.45f}){
            const float z=std::sqrt(field*field-x*x)-.18f;
            for(float sign:{-1.f,1.f}){
                const P3 top=onFace(x,sign>0?z:0),bottom=onFace(x,sign<0?-z:0);
                const P3 width{.15f,0,0};
                rig.face(top-width,top+width,bottom+width,bottom-width,shade(back,.53f));
            }
        }
        for(int i=0;i<10;i++){
            const float x=(i%5-2)*2.25f+.65f;
            const float z=i<5?-6.75f:6.75f;
            const P3 a=onFace(x,z),b=onFace(x+.45f,z+(i<5?2.f:-2.f));
            const Color ink=shade(base,i%2?.78f:1.22f);
            const P3 stroke{.18f,.11f,0};
            if(i<5)rig.face(a,a-stroke,b,b,ink);
            else rig.face(a,b,a-stroke,a-stroke,ink);
        }
        rig.rounded({0,3.6f,0},{7.7f,3.25f,7.7f},shade(rim,.72f));
        rig.rounded({0,4.7f,0},{5.25f,3.55f,5.25f},rim);
        diamond({-.63f,6.65f,.72f},{1,0,0},{0,0,1},.78f,.8f,inlay);
    }else if(sun){
        // Eight inlaid bronze sun rays around a turquoise cabochon.
        for(int i=0;i<8;i++){
            const P3 dir=edge[i*3],across{dir.z,0,-dir.r};
            const float outer=i%2?7.25f:9.1f;
            const P3 a=onFace(dir.r*3.8f,dir.z*3.8f),b=onFace(dir.r*outer,dir.z*outer);
            const P3 root=onFace(dir.r*5.4f,dir.z*5.4f);
            rig.face(a,b,root+across*1.25f,root+across*1.25f,shade(rim,.82f));
            rig.face(a,root-across*1.25f,b,b,shade(rim,1.14f));
        }
        rig.rounded({0,3.75f,0},{7.45f,3.25f,7.45f},rim);
        rig.rounded({0,5.1f,0},{5.35f,3.2f,5.35f},inlay);
    }else{
        // Copper links on segmented black iron, around a cut ember stone.
        for(int i=0;i<6;i++){
            const P3 dir=edge[i*4],across{dir.z,0,-dir.r};
            const P3 center=onFace(dir.r*7.f,dir.z*7.f);
            diamond(center+P3{0,.23f,0},across,dir,.82f,2.4f,rim);
        }
        rig.rounded({0,3.7f,0},{7.6f,3.25f,7.6f},shade(rim,.82f));
        const P3 top{0,5.65f,3.75f},left{-2.55f,5.53f,0},right{2.55f,5.53f,0},
                 bottom{0,5.65f,-3.75f},crest{-.55f,7.5f,.66f};
        rig.face(top,right,crest,crest,shade(inlay,.88f));
        rig.face(right,bottom,crest,crest,shade(inlay,.64f));
        rig.face(bottom,left,crest,crest,inlay);
        rig.face(left,top,crest,crest,shade(inlay,1.28f));
    }
    for(int i=0;i<Segments;i+=timber?3:4){
        const P3 dir=edge[i],across{dir.z,0,-dir.r};
        const P3 station=dir*(radius-1.05f)+P3{0,2.22f,0};
        diamond(station,across,dir,.61f,.72f,timber?inlay:shade(rim,1.24f));
    }
}
}
