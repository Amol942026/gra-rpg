#pragma once
#include "rlgl.h"

// A common wind field drives the crowns and the surface of the water.
float windGust(V position,float time){
    return .65f+.25f*std::sin(time*.43f-(position.x-position.y)*.13f)
                +.10f*std::sin(time*.79f+position.y*.19f);
}
float treeBend(const Prop& prop,float sourceY,float time){
    float height=std::clamp((200-sourceY)/165.f,0.f,1.21f);
    float phase=prop.pos.x*.71f+prop.pos.y*.43f;
    float sway=3.f+5.f*std::sin(time*1.05f+phase)+1.6f*std::sin(time*1.73f+phase*.7f);
    return height*height*windGust(prop.pos,time)*sway
        +height*height*std::sin(time*2.6f+phase+height*6.f)*.45f;
}
namespace tree_wind {
inline bool cached=true,shared=true;
struct Samples {
 std::array<float,47> bends{};
 Samples()=default;
 Samples(const Prop& prop,float time){
  if(!cached)return;
  const float phase=prop.pos.x*.71f+prop.pos.y*.43f;
  const float sway=3.f+5.f*std::sin(time*1.05f+phase)+1.6f*std::sin(time*1.73f+phase*.7f);
  const float gust=windGust(prop.pos,time);
  for(int i=0;i<47;i++){
   const float height=std::clamp((200-i*5.f)/165.f,0.f,1.21f);
   bends[i]=height*height*gust*sway+height*height*std::sin(time*2.6f+phase+height*6.f)*.45f;
  }
 }
};
struct Entry {V position;float time;Samples samples;};
inline std::vector<Entry> frameSamples;
inline void init(){frameSamples.reserve(256);}
inline void beginFrame(){frameSamples.clear();}
inline const Samples& samples(const Prop& prop,float time){
 if(cached&&shared){
  for(const auto& entry:frameSamples)if(entry.position.x==prop.pos.x&&entry.position.y==prop.pos.y&&entry.time==time)return entry.samples;
  if(frameSamples.size()<256){
   if(frameSamples.capacity()<256)init();
   frameSamples.push_back({prop.pos,time,Samples(prop,time)});return frameSamples.back().samples;
  }
 }
 // Reference rendering and unusually crowded views keep identical behaviour
 // without letting the frame cache grow beyond its fixed budget.
 static Samples fallback;fallback=Samples(prop,time);return fallback;
}
}
// Hit testing uses the exact piecewise-linear deformation of the drawn mesh.
float treeShift(const Prop& prop,float sourceY,float time){
    float y=std::clamp(sourceY,0.f,230.f),a=std::floor(y/5)*5,b=std::min(a+5,230.f);
    float fraction=b>a?(y-a)/(b-a):0;
    return treeBend(prop,a,time)*(1-fraction)+treeBend(prop,b,time)*fraction;
}
void drawPropTexture(const Prop& prop,Texture2D texture,bool upright,Color tint){
    if(prop.type==13&&game.inVolcano()){volcano_art::drawPortal(prop,upright,tint);return;}
    if(prop.type>=14){volcano_art::drawProp(prop,upright,tint);return;}
    Vector2 base=screen(prop.pos);
    if(!treeProp(prop.type)){
        DrawTexturePro(texture,{0,0,float(texture.width),upright?float(texture.height):-float(texture.height)},
            {base.x-90*prop.size,base.y-208*prop.size,180*prop.size,230*prop.size},{0,0},0,tint);
        return;
    }
    // Filtered, connected quads preserve fractional-pixel movement of the crown.
    const auto& wind=tree_wind::samples(prop,game.time);
    rlSetTexture(texture.id);rlBegin(RL_QUADS);rlColor4ub(tint.r,tint.g,tint.b,tint.a);
    for(int y=0;y<230;y+=5){
        float top=base.y+(y-208)*prop.size,bottom=top+5*prop.size;
        float left0=base.x+(-90+(tree_wind::cached?wind.bends[y/5]:treeBend(prop,float(y),game.time)))*prop.size;
        float left1=base.x+(-90+(tree_wind::cached?wind.bends[y/5+1]:treeBend(prop,float(y+5),game.time)))*prop.size;
        float v0=y/230.f,v1=(y+5)/230.f;if(!upright){v0=1-v0;v1=1-v1;}
        rlTexCoord2f(0,v0);rlVertex2f(left0,top);
        rlTexCoord2f(0,v1);rlVertex2f(left1,bottom);
        rlTexCoord2f(1,v1);rlVertex2f(left1+180*prop.size,bottom);
        rlTexCoord2f(1,v0);rlVertex2f(left0+180*prop.size,top);
    }
    rlEnd();rlSetTexture(0);
}
bool waterAt(V p){return p.x<.5f||p.x>(MapSize-1.5f)||p.y<.5f||p.y>(MapSize-1.5f);}
#include "forest_wave_crests.hpp"

namespace forest_water {
inline Shader material{};
inline Texture2D variation{};
inline int cameraLocation=-1,timeLocation=-1;

inline void init(){
    material=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
out vec4 finalColor;
uniform sampler2D texture0;
uniform vec2 origin;
uniform float time;
uniform float farBank;

vec4 surfaceVariation(vec2 p){return texture(texture0,p/64.0);}

float reflectedLine(float phase,float width){
    // Broaden subpixel highlights just enough to remain stable in motion.
    float antialias=max(.47,.66*fwidth(phase));
    float offset=abs(fract(phase/6.2831853+.5)-.5)*6.2831853;
    return 1.0-smoothstep(width,width+.095+antialias,offset);
}

void main(){
    vec2 screen=fragTexCoord-origin;
    vec2 d=screen/vec2(40.0,20.0);
    vec2 world=vec2(d.y+d.x,d.y-d.x)*.5;
    vec2 outside=max(vec2(.5)-world,world-vec2(farBank));
    if(max(outside.x,outside.y)<-3.5) discard;
    float shore=length(max(outside,0.0));

    float age=.58*time;
    // Wind aligns the long highlights, while crossing waves and drifting
    // variations bend and break them into clusters of smaller reflections.
    vec2 p=mat2(.94,-.342,.342,.94)*(screen/40.0);
    vec4 swellNoise=surfaceVariation(p*vec2(.35,.51)+vec2(.11,-.20)*age);
    vec2 warp=vec2(swellNoise.r-.5,swellNoise.g-.5);
    vec4 chop=surfaceVariation((p+warp*.78)*vec2(1.86,3.25)+vec2(.26,-.46)*age);
    vec4 grain=surfaceVariation(p*vec2(9.1,14.4)+vec2(-.38,.81)*age);

    // Each scale has its own amplitude. A tall swell bends the smaller
    // ripples on its face, and calmer pockets keep only short crests.
    float swellHeight=mix(.26,1.52,smoothstep(.18,.82,swellNoise.a));
    float chopHeight=mix(.36,1.19,smoothstep(.20,.82,chop.b));
    float fineHeight=mix(.38,.94,grain.b);
    float swell=1.72*p.y+.21*p.x-1.62*age+2.6*warp.x+.33*sin(1.23*p.x+.26*p.y);
    float chopPhase=4.90*p.y-.55*p.x-2.45*age+3.10*warp.y
        +(.30+.62*swellHeight)*sin(swell+.55)+.64*(chop.g-.5);
    float finePhase=12.6*p.y+1.58*p.x-3.18*age+2.6*warp.x
        +(.27+.32*chopHeight)*sin(chopPhase+.45)+.43*(chop.b-.5)+.19*(grain.r-.5);

    float swellLight=sin(swell);
    float relief=.73*swellHeight*swellLight+.21*chopHeight*sin(chopPhase)+.06*fineHeight*sin(finePhase);
    float slope=.60*swellHeight*cos(swell)+.29*chopHeight*cos(chopPhase)+.11*fineHeight*cos(finePhase);
    float highCrest=smoothstep(.55,.95,swellLight)*smoothstep(.56,1.40,swellHeight);
    float shallow=1.0-smoothstep(.7,5.8,shore);
    vec3 water=mix(vec3(18,63,93),vec3(25,76,97),shallow);
    water+=vec3(22,28,30)*(swellNoise.b-.37)
        +vec3(17,28,31)*(.65*relief+.35*slope)
        +vec3(14,25,30)*highCrest
        +vec3(15,20,22)*(.62*(chop.r-.5)+.62*(grain.g-.5))
        +vec3(5,9,12)*sin(finePhase+.7);

    float pockets=smoothstep(.25,.67,.62*chop.b+.23*swellNoise.a+.16*slope);
    float mainGlint=reflectedLine(chopPhase+.38,.05+.051*chopHeight)*pockets
        *mix(.65,1.10,highCrest);
    float fineGlint=reflectedLine(finePhase+.12,.035+.031*fineHeight)
        *smoothstep(.37,.77,.67*chop.a+.33*grain.b+.12*swellLight);
    float trailingGlint=reflectedLine(chopPhase-.44+.35*cos(finePhase),.041)
        *smoothstep(.50,.78,chop.r)*pockets;
    float sheen=mainGlint*.78+fineGlint*.45+trailingGlint*.24;
    water=mix(water,vec3(157,203,217),clamp(sheen,0.0,.84));

    // Little broken crests run inward, then dissolve at the foot of the bank.
    float wash=7.6*shore+2.56*age+2.8*warp.x+.35*chop.g;
    float foam=reflectedLine(wash,.20)*smoothstep(.43,.82,.75*chop.b+.25*grain.r)
        *smoothstep(.23,.74,shore)*(1.0-smoothstep(1.18,2.38,shore));
    water=mix(water,vec3(150,192,199),foam*.46);
    finalColor=vec4(water/255.0,1.0);
}
)GLSL");
    if(material.id==rlGetShaderIdDefault())return;
    // One small repeatable noise field supplies irregularity, not wave frames.
    Image noise=GenImageColor(64,64,WHITE);
    auto* pixels=static_cast<Color*>(noise.data);
    for(int y=0;y<64;y++)for(int x=0;x<64;x++){
        unsigned seed=Game::hash(x+59,y+1207);
        pixels[y*64+x]=rgb(seed&255,(seed>>8)&255,(seed>>16)&255,(seed>>24)&255);
    }
    variation=LoadTextureFromImage(noise);UnloadImage(noise);
    SetTextureFilter(variation,TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(variation,TEXTURE_WRAP_REPEAT);
    forest_wave_crests::init();
    cameraLocation=GetShaderLocation(material,"origin");
    timeLocation=GetShaderLocation(material,"time");
    const float farBank=MapSize-1.5f;
    SetShaderValue(material,GetShaderLocation(material,"farBank"),&farBank,SHADER_UNIFORM_FLOAT);
}

inline void draw(){
    if(cameraLocation<0||!variation.id)return;
    Vector2 origin=screenFlat({0,0});
    const float margin=terrain_height::MaxPixels/40+1;
    const float minX=-origin.x/80-origin.y/40,maxX=minX+WorldW/80.f+H/40.f;
    const float minY=(origin.x-WorldW)/80-origin.y/40,maxY=minY+WorldW/80.f+H/40.f;
    if(minX>.5f+margin&&minY>.5f+margin&&maxX<MapSize-1.5f-margin&&maxY<MapSize-1.5f-margin)return;
    BeginShaderMode(material);
    SetShaderValue(material,cameraLocation,&origin.x,SHADER_UNIFORM_VEC2);
    SetShaderValue(material,timeLocation,&game.time,SHADER_UNIFORM_FLOAT);
    rlSetTexture(variation.id);rlBegin(RL_QUADS);rlColor4ub(255,255,255,255);
    for(Vector2 corner:{Vector2{0,0},Vector2{0,H},Vector2{WorldW,H},Vector2{WorldW,0}}){
        rlTexCoord2f(corner.x,corner.y);rlVertex2f(corner.x,corner.y);
    }
    rlEnd();rlSetTexture(0);EndShaderMode();
}
inline void drawRaisedCrests(){
    if(variation.id)forest_wave_crests::draw(screenFlat({0,0}),variation,game.time);
}
inline void unload(){
    forest_wave_crests::unload();
    if(variation.id)UnloadTexture(variation);
    if(material.id&&material.id!=rlGetShaderIdDefault())UnloadShader(material);
    variation={};material={};cameraLocation=timeLocation=-1;
}
}
