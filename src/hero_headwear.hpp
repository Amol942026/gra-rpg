#pragma once

// Included after Rig so world art and inventory previews share the same cuts.
namespace hero_headwear {
inline constexpr bool hood(Kind kind){
    return kind==Kind::WebCowl||kind==Kind::SlimeCowl||kind==Kind::WolfHood||kind==Kind::BanditHelm;
}
inline constexpr bool closed(Kind kind){
    return kind==Kind::RivetedGreathelm||kind==Kind::SunwardHelm||kind==Kind::AshenVisor;
}
inline constexpr bool open(Kind kind){
    return openHelmetKind(kind);
}
inline Color finish(unsigned pigment,bool flash){
    return flash?Text:equipment_art::color(pigment);
}

struct HoodRow {
    float height,width,front,back,offset,opening;
};
inline void drawHood(Rig& rig,float head,const GearAppearance& look,bool flash){
    const bool web=look.value.kind==Kind::WebCowl;
    const bool mist=look.value.kind==Kind::SlimeCowl;
    const bool fur=look.value.kind==Kind::WolfHood;
    const bool leather=look.value.kind==Kind::BanditHelm;
    const Color shell=finish(look.paint.primary,flash);
    const Color lining=shade(finish(look.paint.secondary,flash),.91f);
    const Color hem=ColorLerp(shade(shell,.86f),finish(look.paint.trim,flash),fur?.29f:.43f);
    const std::array<HoodRow,8> cut{{
        {fur?12.f:web?15.5f:13.6f,0,0,0,web?-4.f:-2.3f,0},
        {10.8f,7.8f,8.6f,8.4f,-.7f,0},
        {8.8f,9.7f,10.3f,9.5f,-.5f,.64f},
        {4.0f,10.25f,9.7f,10.f,-.7f,1.00f},
        {-1.8f,10.0f,9.3f,9.8f,-1.0f,.99f},
        {-6.7f,9.3f,8.9f,9.5f,-1.1f,.77f},
        {-10.5f,10.3f,9.5f,10.0f,-.9f,.69f},
        {-13.1f,mist?13.3f:12.2f,10.6f,10.5f,-.3f,.86f},
    }};
    constexpr int columns=20;
    auto angle=[&](int row,int column){
        const float a=cut[row].opening;
        return a+(2*PI-2*a)*column/columns;
    };
    auto point=[&](int row,int column){
        const auto& band=cut[row];
        const float a=angle(row,column),c=std::cos(a);
        // Folds in the back and shoulders fade out toward the face opening.
        const float pleat=(.42f*std::cos(5*a)+.24f*std::cos(8*a+row*.36f))
            *(1-.65f*std::max(0.f,c))*(row==0?0:row<5?.45f:1.2f);
        return P3{(band.width+pleat)*std::sin(a),
                  band.offset+(c>0?band.front:band.back)*c+pleat*c,
                  head+band.height+(row==7?.5f*std::cos(5*a):0)};
    };
    auto outerColor=[&](int row,int column){
        const float a=angle(row,column);
        const float slope=row<3?.48f:row>=6?-.20f:0;
        const float light=rig.surfaceLight({std::sin(a),std::cos(a),slope});
        const float fold=.97f+(fur?.055f:.036f)*std::cos(a*5+.38f*row);
        return shade(shell,light*fold*(row>=6?.91f:1));
    };
    for(int row=0;row<int(cut.size())-1;row++){
        for(int col=0;col<columns;col++){
            const P3 a=point(row,col),b=point(row,col+1),c=point(row+1,col+1),d=point(row+1,col);
            rig.surfaceQuad(a,b,c,d,outerColor(row,col),outerColor(row,col+1),
                            outerColor(row+1,col+1),outerColor(row+1,col));
            rig.face(d,c,b,a,lining,.91f);
        }
    }
    // Turn the hem in toward the temple. The fabric ends behind the face in
    // profile, so it never becomes a plate across the nose or mouth.
    for(int col:{0,columns}){
        const float side=col==0?1.f:-1.f;
        for(int row=1;row<int(cut.size())-1;row++){
            const P3 a=point(row,col),b=point(row+1,col);
            const P3 edge{-side*.30f,-.82f,-.12f};
            rig.face(a,b,b+edge,a+edge,hem);
            rig.face(a+edge,b+edge,b,a,hem);
            rig.tapered(a,b,fur?1.24f:leather?.57f:.79f,fur?1.09f:.65f,hem);
        }
    }
    if(web||mist){
        const Color thread=shade(hem,web?1.12f:.91f);
        for(int col:{1,columns-1}){
            for(int row=4;row<7;row++){
                const P3 a=point(row,col),b=point(row+1,col);
                rig.tapered(a+P3{0,0,.12f},b+P3{0,0,.12f},.38f,.38f,thread);
            }
        }
    }
}

struct HelmetRow {
    float height,width,front,back,peak=0;
};

struct OpenCut {
    float crown,width,front,back,brow,nape,bevel;
};
inline constexpr OpenCut openCut(Kind kind){
    switch(kind){
        case Kind::BoneHelmet:return {12.7f,9.1f,8.1f,9.1f,5.4f,-3.2f,.54f};
        case Kind::GuardianHelmet:return {12.2f,9.9f,8.0f,9.5f,6.0f,-2.9f,.83f};
        case Kind::ScarabHelmet:return {12.5f,9.5f,8.7f,9.2f,6.6f,-2.6f,.26f};
        case Kind::BoarHelm:return {11.6f,9.45f,8.9f,9.5f,5.7f,-3.5f,.58f};
        case Kind::BarkHelm:return {11.9f,9.2f,8.2f,8.9f,5.4f,-2.3f,.72f};
        case Kind::DuneHelmet:return {12.1f,9.6f,8.4f,9.2f,5.8f,-1.7f,.44f};
        default:return {11.7f,9.1f,8.7f,9.0f,5.8f,-2.1f,.43f};
    }
}
inline void drawOpen(Rig& rig,float head,const GearAppearance& look,bool flash=false){
    const Kind kind=look.value.kind;
    const OpenCut cut=openCut(kind);
    const Color shell=finish(look.paint.primary,flash),shadow=finish(look.paint.secondary,flash);
    const Color trim=finish(look.paint.trim,flash),accent=finish(look.paint.accent,flash);
    constexpr int segments=20;
    const auto point=[&](int band,float step,float lift=0.f){
        const float angle=step*(2*PI/segments),s=std::sin(angle),c=std::cos(angle);
        constexpr float radii[]={0,.55f,.86f,1.f};
        const float edge=cut.brow+(cut.nape-cut.brow)*(1-c)*.5f;
        const float taper=band==3?edge:band==2?edge+(cut.crown-edge)*cut.bevel:cut.crown-(band==1?1.1f:0.f);
        const float radius=radii[band];
        return P3{radius*cut.width*s,(c>=0?cut.front:cut.back)*radius*c,head+taper+lift};
    };
    const auto light=[&](int band,int step){
        const float angle=step*(2*PI/segments);
        return rig.surfaceLight({std::sin(angle),std::cos(angle),band<2?.80f:band==2?.36f:.05f});
    };
    for(int band=0;band<3;band++)for(int k=0;k<segments;k++){
        const P3 a=point(band,k),b=point(band,k+1),c=point(band+1,k+1),d=point(band+1,k);
        const bool rib=kind==Kind::BarkHelm||kind==Kind::GuardianHelmet;
        const float panel=rib&&k%3==0?.84f:1.f;
        rig.surfaceQuad(a,b,c,d,shade(shell,panel*light(band,k)),shade(shell,panel*light(band,k+1)),
                        shade(shell,panel*light(band+1,k+1)),shade(shell,panel*light(band+1,k)));
    }
    for(int k=0;k<segments;k++){
        const P3 a=point(3,k),b=point(3,k+1);
        const P3 ia{a.r*.93f,a.f*.94f,a.z+.55f},ib{b.r*.93f,b.f*.94f,b.z+.55f};
        rig.face(b,a,ia,ib,shade(shadow,.75f));
        rig.tapered(a,b,kind==Kind::DuneHelmet?1.1f:.84f,.60f,trim);
    }
    const auto raised=[&](int band,float step,float z){
        P3 p=point(band,step,z);p.r*=1.035f;p.f*=1.035f;return p;
    };
    const auto pin=[&](P3 p,float size,Color col){rig.rounded(p,{size,size*.78f,size},col);};

    // Temple hinges and side lames sit outside the skull and behind the eyes.
    for(float side:{-1.f,1.f}){
        const P3 hinge=raised(3,side*3.8f,.75f);
        pin(hinge,kind==Kind::ScarabHelmet?2.35f:1.55f,trim);
        if(kind==Kind::DuneHelmet||kind==Kind::BarkHelm)continue;
        const float cheek=kind==Kind::BoneHelmet||kind==Kind::BoarHelm?-4.9f:-3.75f;
        const P3 tip{side*(cut.width-.34f),3.7f,head+cheek};
        const P3 back{side*(cut.width-.14f),-1.1f,head+cheek-.46f};
        const P3 upper=raised(3,side*5.5f,.84f);
        rig.face(hinge,tip,back,upper,shell,.92f);
        rig.face(upper,back,tip,hinge,shade(shell,.83f));
        rig.tapered(tip,back,.68f,.48f,trim);
    }

    switch(kind){
        case Kind::BoneHelmet:
            for(float side:{-1.f,1.f}){
                const P3 rib=raised(2,side*2,.42f);
                rig.tapered(raised(3,side*.9f,.58f),rib,1.35f,.78f,accent);
                rig.tapered(rib,raised(2,side*5,.78f),.95f,.70f,accent);
            }
            break;
        case Kind::GuardianHelmet:
            for(float k:{-5.f,-2.8f,2.8f,5.f}){
                const P3 groove=raised(2,k,.28f);
                rig.tapered(groove,raised(3,k,.55f),.94f,.59f,shade(accent,.70f));
                pin(raised(3,k,.88f),1.45f,accent);
            }
            break;
        case Kind::ScarabHelmet:{
            const P3 scarab=raised(2,0,.27f);
            rig.rounded(scarab,{2.25f,1.65f,3.8f},accent);
            for(float side:{-1.f,1.f}){
                const P3 wing=raised(2,side*2.1f,.62f);
                rig.tapered(scarab,wing,1.15f,.68f,trim);
                rig.tapered(wing,raised(3,side*4,1.f),.88f,.60f,trim);
            }
            break;
        }
        case Kind::BoarHelm:
            for(float side:{-1.f,1.f}){
                const P3 root{side*(cut.width+.04f),3.36f,head-3.2f};
                const P3 curve{side*(cut.width+1.22f),5.35f,head+.45f};
                const P3 tip{side*(cut.width+1.9f),5.62f,head+4.05f};
                rig.tapered(root,curve,2.75f,1.65f,accent,.75f);
                rig.tapered(curve,tip,2.0f,1.25f,accent,.11f);
            }
            break;
        case Kind::BarkHelm:
            for(float k:{-7.f,-3.3f,0.f,3.3f,7.f}){
                const P3 upper=raised(1,k,.34f),middle=raised(2,k+.40f,.42f),lower=raised(3,k,.60f);
                rig.tapered(upper,middle,.80f,.58f,shade(shadow,.80f));
                rig.tapered(middle,lower,.74f,.53f,shade(shadow,.80f));
            }
            for(float k:{-6.f,-3.f,0.f,3.f,6.f})pin(raised(3,k,.65f),1.48f,trim);
            break;
        case Kind::DuneHelmet:
            for(float k:{-6.f,-3.f,0.f,3.f,6.f}){
                const P3 base=raised(3,k,.90f),tip=base+P3{0,0,k==0?5.4f:3.65f};
                rig.tapered(base,tip,2.65f,1.60f,trim,.43f);
                pin(tip,1.64f,accent);
            }
            rig.rounded(raised(2,0,.21f),{3.5f,2.0f,3.8f},accent);
            break;
        default:
            rig.tapered(raised(3,0,.55f),raised(2,0,.60f),1.27f,.73f,trim);
            rig.tapered(raised(2,0,.60f),raised(1,0,.35f),1.1f,.65f,trim);
            for(float k:{-3.f,3.f,8.f,-8.f})pin(raised(3,k,.58f),1.52f,accent);
            break;
    }
}
inline void drawClosed(Rig& rig,float head,const GearAppearance& look,bool flash=false){
    const Kind kind=look.value.kind;
    const bool iron=kind==Kind::RivetedGreathelm;
    const bool sun=kind==Kind::SunwardHelm;
    const Color plate=finish(look.paint.primary,flash);
    const Color dark=finish(look.paint.secondary,flash);
    const Color rim=finish(look.paint.trim,flash);
    const Color stud=finish(look.paint.accent,flash);
    const Color vent=finish(0x1b2025,flash);
    constexpr std::array<HelmetRow,8> bucket{{
        {10.95f,0,0,0},{10.7f,7.0f,6.8f,6.8f},{8.3f,8.95f,8.85f,8.45f},
        {4.1f,9.4f,9.75f,9.1f},{2.1f,9.4f,9.9f,9.1f},
        {-4.2f,8.85f,10.15f,8.6f},{-9.55f,7.55f,8.7f,7.7f},{-9.7f,0,0,0}
    }};
    constexpr std::array<HelmetRow,8> sallet{{
        {11.85f,0,0,0},{10.2f,5.9f,5.4f,5.9f},{7.3f,8.3f,8.55f,8.75f},
        {4.1f,9.05f,9.35f,9.15f,-.35f},{2.2f,9.05f,9.7f,9.6f,-.42f},
        {-3.75f,8.5f,10.55f,10.25f,-.45f},{-8.95f,7.35f,8.4f,11.35f,1.2f},{-9.1f,0,0,0}
    }};
    constexpr std::array<HelmetRow,8> visor{{
        {11.95f,0,0,0},{10.0f,4.5f,5.35f,4.9f,1.25f},{7.2f,7.55f,7.8f,7.75f,1.05f},
        {4.9f,9.0f,9.55f,8.7f,-1.75f},{3.0f,9.0f,10.35f,8.95f,-1.8f},
        {-2.45f,8.3f,11.9f,8.4f,-1.4f},{-8.85f,6.55f,8.45f,7.5f,-.9f},{-9.85f,0,0,0}
    }};
    const auto& profile=iron?bucket:sun?sallet:visor;
    constexpr int segments=24;
    const auto point=[&](int row,float step,float inset=0.f){
        const auto& cut=profile[row];
        const float a=step*2*PI/segments;
        const float x=std::sin(a),c=std::cos(a);
        const float bow=iron?std::copysign(std::sqrt(std::abs(c)),c):
            sun?c:c>0?c*c:c;
        const float width=iron?std::copysign(std::sqrt(std::abs(x)),x):x;
        const float brow=std::max(c,0.f);
        const float edge=(!iron&&!sun&&row<2)?-.9f:0.f;
        return P3{(cut.width-inset)*width,
                  (bow>=0?cut.front-inset:cut.back-inset)*bow+edge,
                  head+cut.height+cut.peak*brow*brow};
    };
    const auto metalLight=[&](int row,float step){
        const float a=step*2*PI/segments;
        const float slope=row<2?.65f:row>=5?-.22f:.07f;
        const float strength=rig.surfaceLight({std::sin(a),std::cos(a),slope});
        return strength*(row==1?.96f:row==5?.90f:1.f);
    };
    const auto slit=[&](int segment){
        // Front slits stop at the temples. The enclosed plates behind them
        // keep the opening dark from the side and retain the silhouette.
        const int half=iron?4:sun?3:4;
        return segment<half||segment>=segments-half;
    };
    for(int row=0;row<int(profile.size())-1;row++){
        for(int k=0;k<segments;k++){
            const P3 a=point(row,k),b=point(row,k+1),c=point(row+1,k+1),d=point(row+1,k);
            if(row==3&&slit(k)){
                const P3 backA=point(row,k,.92f),backB=point(row,k+1,.92f);
                const P3 backC=point(row+1,k+1,.92f),backD=point(row+1,k,.92f);
                rig.face(backA,backB,backC,backD,vent);
                rig.face(a,b,backB,backA,shade(rim,.62f));
                rig.face(backD,backC,c,d,shade(dark,.56f));
                if(!slit((k+segments-1)%segments))rig.face(d,a,backA,backD,dark);
                if(!slit((k+1)%segments))rig.face(b,c,backC,backB,dark);
                continue;
            }
            const Color metal=(!iron&&row==6)?dark:ColorLerp(plate,dark,iron&&row==4?.27f:sun?.07f:.17f);
            const float alloy=iron&&((k/3)%2)?.91f:!iron&&!sun&&(k==0||k==segments-1)?1.17f:1.f;
            rig.surfaceQuad(a,b,c,d,shade(metal,alloy*metalLight(row,k)),shade(metal,alloy*metalLight(row,k+1)),
                            shade(metal,alloy*metalLight(row+1,k+1)),shade(metal,alloy*metalLight(row+1,k)));
        }
    }
    for(int k=0;k<segments;k++){
        if(slit(k))rig.tapered(point(3,k,-.20f),point(3,k+1,-.20f),sun?.87f:.67f,.60f,rim);
        if(iron||sun||!slit(k))
            rig.tapered(point(6,k,-.16f),point(6,k+1,-.16f),.72f,.58f,shade(rim,.81f));
    }
    if(iron){
        for(float step:{0.f,3.f,6.f,9.f,12.f,15.f,18.f,21.f}){
            const P3 rivet=point(6,step,-.33f)*.55f+point(5,step,-.33f)*.45f;
            rig.rounded(rivet,{1.48f,1.2f,1.48f},stud);
        }
        for(float side:{-1.f,1.f}){
            for(float step:{1.05f,2.2f,3.25f}){
                const P3 upper=point(4,step*side,-.43f),lower=point(5,step*side,-.43f);
                const P3 first=upper*.68f+lower*.32f,second=upper*.43f+lower*.57f;
                rig.tapered(first,second,.82f,.74f,shade(vent,.85f));
            }
        }
        rig.tapered(point(2,0,-.65f),point(6,0,-.65f),1.08f,.75f,shade(rim,.88f));
        rig.tapered(point(1,-4,-.16f),point(1,4,-.16f),1.04f,.58f,shade(rim,.87f));
    }else if(sun){
        for(int side:{-1,1}){
            const float step=4.f*side;
            rig.tapered(point(4,step,-.27f),point(5,step,-.27f),1.14f,.84f,rim);
            rig.tapered(point(5,step,-.27f),point(6,3.f*side,-.27f),1.14f,.79f,rim);
            rig.rounded(point(4,5.f*side,-.42f),{2.85f,2.20f,2.85f},rim);
            rig.rounded(point(4,5.f*side,-1.0f),{1.40f,1.75f,1.40f},stud);
            for(float k:{2.f,4.f,6.f}){
                const P3 crest=point(1,(k-.60f)*side,-.20f),tip=point(2,k*side,-.32f);
                rig.tapered(crest,tip,.81f,.51f,rim,.62f);
            }
        }
        rig.rounded(point(2,0,-.52f),{4.0f,1.95f,3.75f},rim);
        rig.rounded(point(2,0,-1.21f),{2.25f,1.60f,2.2f},stud);
        for(int k=6;k<18;k++){
            const P3 a=point(6,k,-.35f)*.62f+point(5,k,-.35f)*.38f;
            const P3 b=point(6,k+1,-.35f)*.62f+point(5,k+1,-.35f)*.38f;
            rig.tapered(a,b,.70f,.50f,shade(rim,.72f));
        }
    }else{
        for(int k=1;k<3;k++)rig.tapered(point(k,0,-.54f),point(k+1,0,-.58f),1.55f,.90f,rim);
        for(int side:{-1,1}){
            const P3 jaw=point(5,0,-.58f),temple=point(4,3.f*side,-.52f);
            rig.tapered(jaw,temple,1.1f,.78f,rim);
            rig.tapered(point(5,3.f*side,-.57f),point(6,side,-.55f),.77f,.63f,stud);
            rig.tapered(point(2,2.2f*side,-.38f),point(3,5.f*side,-.42f),.83f,.64f,shade(rim,.78f));
        }
    }
}
}
