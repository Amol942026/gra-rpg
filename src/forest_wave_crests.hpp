#pragma once

// A breaking wave is a set of projected 2D ribbons. The crest can move up
// independently of the footprint, and the folded lip covers the rear face.
namespace forest_wave_crests {
inline Shader material{};
inline int timeLocation=-1;
inline constexpr int CellWidth=326,CellHeight=158,Margin=270,MaxSegments=30;
inline constexpr int MaxVisible=((WorldW+2*Margin)/CellWidth+2)*((H+2*Margin)/CellHeight+2);

struct Crest {
    V center{},tangent{},normal{};
    float span=0,height=0,depth=0,opacity=0,phase=0,roll=0;
};
struct Visible {
    std::array<Crest,MaxVisible> crests{};
    int count=0;
};
struct Profile {float forward,height;Color color;};
inline constexpr std::array<Profile,11> Face{{
    {-.93f,0,{15,49,76,0}},
    {-.58f,.19f,{24,78,111,169}},
    {-.26f,.64f,{43,112,149,232}},
    { .02f,.96f,{81,150,176,248}},
    { .18f,1,{164,208,217,254}},
    { .29f,.97f,{188,221,224,254}},
    { .36f,.91f,{105,159,177,251}},
    { .39f,.84f,{17,48,78,247}},
    { .46f,.55f,{27,79,117,239}},
    { .72f,.18f,{30,81,121,187}},
    { 1.06f,0,{17,61,92,0}}
}};
inline constexpr std::array<Profile,4> Shadow{{
    {-.57f,0,{5,22,40,0}}, {-.17f,0,{5,22,40,82}},
    { .41f,0,{5,22,40,94}}, {1.06f,0,{5,22,40,0}}
}};

inline void init(){
    material=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;
uniform sampler2D texture0;
uniform float age;

float filament(float phase){
    float edge=max(.06,.63*fwidth(phase));
    return 1.-smoothstep(.08,.08+edge,abs(sin(phase)));
}

void main(){
    vec2 p=fragTexCoord;
    vec4 broad=texture(texture0,(p*vec2(15.,8.)+vec2(.11,-.36)*age)/64.);
    vec2 churn=vec2(87.*p.x+2.4*sin(14.*p.y+4.1*broad.g),24.*p.y);
    vec4 grain=texture(texture0,(churn+vec2(-.23,-.91)*age)/64.);
    float lace=.54*broad.r+.46*grain.g;
    float lip=smoothstep(.34,.45,p.y)*(1.-smoothstep(.54,.65,p.y));
    float rim=1.-smoothstep(.011,.038,abs(p.y-(.453+.044*(broad.b-.5))));
    float web=filament(84.*p.x+20.*p.y-1.18*age+3.6*broad.g)
             +.59*filament(47.*p.x-37.*p.y+.63*age+2.9*grain.r);
    float foam=lip*(.51*smoothstep(.51,.79,lace)*web
             +.35*smoothstep(.58,.86,grain.a)+.42*rim*smoothstep(.28,.70,lace));

    // Long wet grooves on the face stream down from the curling lip.
    float front=smoothstep(.68,.77,p.y)*(1.-smoothstep(.92,.99,p.y));
    float folds=smoothstep(.59,.84,.73*grain.b+.27*broad.r);
    vec3 color=fragColor.rgb*(.87+.26*lace);
    color=mix(color,vec3(.30,.61,.73),folds*front*.23);
    color=mix(color,vec3(.89,.96,.97),min(foam,.87));
    float opacity=fragColor.a*(.92+.08*smoothstep(.22,.74,lace));
    finalColor=vec4(color,opacity);
}
)GLSL");
    if(material.id!=rlGetShaderIdDefault())timeLocation=GetShaderLocation(material,"age");
}
inline void unload(){
    if(material.id&&material.id!=rlGetShaderIdDefault())UnloadShader(material);
    material={};timeLocation=-1;
}

inline float smooth(float lo,float hi,float x){
    float t=std::clamp((x-lo)/(hi-lo),0.f,1.f);return t*t*(3-2*t);
}
inline V coastOffset(V p){
    V world{p.y/40+p.x/80,p.y/40-p.x/80};
    return V{std::clamp(world.x,.5f,MapSize-1.5f),std::clamp(world.y,.5f,MapSize-1.5f)}-world;
}
inline float shoreFade(V p){
    return smooth(.62f,3.5f,length(coastOffset(p)));
}
inline Visible collect(Vector2 origin,float time){
    Visible result;
    const int x0=int(std::floor((-origin.x-Margin)/CellWidth)),x1=int(std::floor((WorldW-origin.x+Margin)/CellWidth));
    const int y0=int(std::floor((-origin.y-Margin)/CellHeight)),y1=int(std::floor((H-origin.y+Margin)/CellHeight));
    for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++){
        const unsigned seed=Game::hash(x+43,y-107);
        auto unit=[&](int shift){return float((seed>>shift)&255)/255.f;};
        float cycle=time*(.114f+.040f*unit(21))+unit(0);
        float phase=cycle-std::floor(cycle);
        float envelope=smooth(.04f,.36f,phase)*(1-smooth(.68f,.98f,phase));
        if(envelope<=.012f)continue;
        V anchor{(x+.26f+.48f*unit(4))*CellWidth,(y+.23f+.54f*unit(15))*CellHeight};
        V toward=coastOffset(anchor);
        if(length(toward)<.01f)continue;
        V bankDirection=normalized(V{(toward.x-toward.y)*40,(toward.x+toward.y)*20});
        const float angle=.34f+(unit(10)-.5f)*.58f;
        V tangent{std::cos(angle),std::sin(angle)},normal{-tangent.y,tangent.x};
        const float forward=(normal.x*bankDirection.x+normal.y*bankDirection.y)>=0?1.f:-1.f;
        V drift=normalized(normal*(forward*.48f)+bankDirection*.74f);
        anchor=anchor+drift*((phase-.42f)*(165+51*unit(18)));
        float shoal=shoreFade(anchor);
        if(envelope*shoal<=.025f)continue;

        const float grade=unit(13);
        const float height=grade<.25f?14+17*unit(8):grade<.65f?34+28*unit(8):67+43*unit(8);
        const float span=70+height*.75f+30*unit(24);
        const float depth=(22+height*.28f)*shoal;
        const V center=anchor+V{origin.x,origin.y};
        const float reachX=std::abs(tangent.x)*span+std::abs(normal.x)*depth+28;
        const float reachY=std::abs(tangent.y)*span+std::abs(normal.y)*depth+28;
        if(center.x<-reachX||center.x>WorldW+reachX||center.y<-reachY||center.y>H+reachY+height)continue;
        Crest& crest=result.crests[result.count++];
        crest={center,tangent,normal,span,height*envelope*shoal,depth,
               envelope*shoal,float(seed%255)*.87f,smooth(.29f,.85f,phase)};
    }
    std::sort(result.crests.begin(),result.crests.begin()+result.count,
              [](const Crest& a,const Crest& b){return a.center.y<b.center.y;});
    return result;
}
struct Vertex {V pos;Vector2 uv;Color color;};
inline void quad(const Vertex& a,const Vertex& b,const Vertex& c,const Vertex& d){
    V ab=b.pos-a.pos,ac=c.pos-a.pos;
    const bool reverse=ab.x*ac.y-ab.y*ac.x>0;
    for(const Vertex* v:{&a,reverse?&d:&b,&c,reverse?&b:&d}){
        rlColor4ub(v->color.r,v->color.g,v->color.b,v->color.a);
        rlTexCoord2f(v->uv.x,v->uv.y);rlVertex2f(v->pos.x,v->pos.y);
    }
}
template<size_t Rows>
inline void drawProfile(const Crest& crest,Vector2 origin,Texture2D variation,float time,
                        const std::array<Profile,Rows>& profile,bool shadow){
    const int segments=std::clamp(int(std::ceil(crest.span/(shadow?10.f:5.9f))),14,MaxSegments);
    std::array<std::array<Vertex,Rows>,MaxSegments+1> columns;
    for(int i=0;i<=segments;i++){
        float t=float(i)/segments,u=2*t-1;
        float taper=std::max(0.f,1-u*u);
        V foot=crest.center+crest.tangent*(u*crest.span)
            +crest.normal*(13*u*u+8*std::sin(3.9f*u+crest.phase));
        float nearShore=shoreFade(foot-V{origin.x,origin.y});
        float rise=shadow?0:crest.height*taper*(.86f+.14f*std::sin(6.8f*u+crest.phase+time*.67f))*nearShore;
        float opacity=crest.opacity*smooth(0,.73f,taper)*nearShore;
        for(size_t row=0;row<Rows;row++){
            const Profile& band=profile[row];
            float distance=band.forward*crest.depth*(.56f+.44f*taper);
            float lift=rise*band.height;
            if(shadow)distance+=crest.depth*.46f;
            else if(row>=4&&row<=7){
                distance+=crest.roll*crest.depth*.27f*taper;
                lift*=1-crest.roll*.11f;
            }
            V pos=foot+crest.normal*distance+V{0,shadow?7.f:-lift};
            Color color=band.color;
            color.a=static_cast<unsigned char>(color.a*opacity);
            columns[i][row]={pos,{crest.phase+t,shadow?0:float(row)/(Rows-1)},color};
        }
    }
    rlCheckRenderBatchLimit(segments*int(Rows-1)*4);
    rlSetTexture(variation.id);rlBegin(RL_QUADS);
    for(size_t row=1;row<Rows;row++)for(int i=1;i<=segments;i++)
        quad(columns[i-1][row-1],columns[i-1][row],columns[i][row],columns[i][row-1]);
    rlEnd();rlSetTexture(0);
}

inline void draw(Vector2 origin,Texture2D variation,float time){
    if(timeLocation<0)return;
    const auto visible=collect(origin,time);
    if(!visible.count)return;
    Texture2D white{rlGetTextureIdDefault(),1,1,1,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    for(int i=0;i<visible.count;i++)drawProfile(visible.crests[i],origin,white,time,Shadow,true);
    BeginShaderMode(material);
    SetShaderValue(material,timeLocation,&time,SHADER_UNIFORM_FLOAT);
    for(int i=0;i<visible.count;i++)drawProfile(visible.crests[i],origin,variation,time,Face,false);
    EndShaderMode();
}
}
