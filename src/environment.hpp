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
// Hit testing uses the exact piecewise-linear deformation of the drawn mesh.
float treeShift(const Prop& prop,float sourceY,float time){
    float y=std::clamp(sourceY,0.f,230.f),a=std::floor(y/5)*5,b=std::min(a+5,230.f);
    float fraction=b>a?(y-a)/(b-a):0;
    return treeBend(prop,a,time)*(1-fraction)+treeBend(prop,b,time)*fraction;
}
void drawPropTexture(const Prop& prop,Texture2D texture,bool upright,Color tint){
    Vector2 base=screen(prop.pos);
    if(!treeProp(prop.type)){
        DrawTexturePro(texture,{0,0,float(texture.width),upright?float(texture.height):-float(texture.height)},
            {base.x-90*prop.size,base.y-208*prop.size,180*prop.size,230*prop.size},{0,0},0,tint);
        return;
    }
    // Filtered, connected quads preserve fractional-pixel movement of the crown.
    rlSetTexture(texture.id);rlBegin(RL_QUADS);rlColor4ub(tint.r,tint.g,tint.b,tint.a);
    for(int y=0;y<230;y+=5){
        float top=base.y+(y-208)*prop.size,bottom=top+5*prop.size;
        float left0=base.x+(-90+treeBend(prop,float(y),game.time))*prop.size;
        float left1=base.x+(-90+treeBend(prop,float(y+5),game.time))*prop.size;
        float v0=y/230.f,v1=(y+5)/230.f;if(!upright){v0=1-v0;v1=1-v1;}
        rlTexCoord2f(0,v0);rlVertex2f(left0,top);
        rlTexCoord2f(0,v1);rlVertex2f(left1,bottom);
        rlTexCoord2f(1,v1);rlVertex2f(left1+180*prop.size,bottom);
        rlTexCoord2f(1,v0);rlVertex2f(left0+180*prop.size,top);
    }
    rlEnd();rlSetTexture(0);
}
bool waterAt(V p){return p.x<.5f||p.x>(MapSize-1.5f)||p.y<.5f||p.y>(MapSize-1.5f);}
void softWave(Vector2 position,float width,float phase,Color color){
    // A ribbon with transparent edges gives antialiasing inside the offscreen
    // canvas, where the window's MSAA setting does not apply. No pixel rounding.
    constexpr int segments=14;
    float feather=1.1f/renderScale;
    // Adjacent segments share the same edge. Evaluate each sine once and use
    // sin(a + phase) = sin(a) cos(phase) + cos(a) sin(phase).
    static const auto curve=[](){std::array<Vector2,segments+1> values{};
        for(int i=0;i<=segments;i++){float a=-1+2.f*i/segments;values[i]={std::sin(a*PI),std::cos(a*PI)};}return values;}();
    float sinPhase=std::sin(phase),cosPhase=std::cos(phase);
    std::array<float,segments+1> heights{};
    for(int i=0;i<=segments;i++){
        float a=-1+2.f*i/segments;
        heights[i]=position.y+a*width*.12f+(curve[i].x*cosPhase+curve[i].y*sinPhase)*1.4f;
    }
    auto vertex=[&](float x,float y,float opacity){
        rlColor4ub(color.r,color.g,color.b,(unsigned char)(color.a*opacity));
        rlTexCoord2f(.5f,.5f);rlVertex2f(x,y);
    };
    rlSetTexture(rlGetTextureIdDefault());rlBegin(RL_QUADS);
    for(int i=0;i<segments;i++){
        float a=-1+2.f*i/segments,b=-1+2.f*(i+1)/segments;
        float x0=position.x+a*width,x1=position.x+b*width;
        float y0=heights[i],y1=heights[i+1];
        float fade0=1-a*a,fade1=1-b*b;
        float edges[]={-.38f-feather,-.38f,.38f,.38f+feather};
        float coverage[]={0,1,1,0};
        for(int strip=0;strip<3;strip++){
            vertex(x0,y0+edges[strip],fade0*coverage[strip]);
            vertex(x0,y0+edges[strip+1],fade0*coverage[strip+1]);
            vertex(x1,y1+edges[strip+1],fade1*coverage[strip+1]);
            vertex(x1,y1+edges[strip],fade1*coverage[strip]);
        }
    }
    rlEnd();rlSetTexture(0);
}
void drawWindWater(){
    // World-anchored wavelets extend past the map; the opaque terrain covers the
    // shore. Resetting their travel occurs only while fully faded out.
    V a=toWorld({-120,-80}),b=toWorld({WorldW+120.f,-80}),c=toWorld({-120,H+80.f}),d=toWorld({WorldW+120.f,H+80.f});
    int x0=int(std::floor(std::min({a.x,b.x,c.x,d.x}))),x1=int(std::ceil(std::max({a.x,b.x,c.x,d.x})));
    int y0=int(std::floor(std::min({a.y,b.y,c.y,d.y}))),y1=int(std::ceil(std::max({a.y,b.y,c.y,d.y})));
    for(int x=x0;x<=x1;x++)for(int y=y0;y<=y1;y++){
        if(x>=1&&x<=MapSize-2&&y>=1&&y<=MapSize-2)continue;
        // A conservative bound includes drift, ribbon width and its second
        // crest. Reject invisible cells before hashing and trigonometry.
        Vector2 cell=screen({float(x),float(y)});
        if(cell.x<-120||cell.x>WorldW+120||cell.y<-80||cell.y>H+80)continue;
        unsigned hash=Game::hash(x,y);
        V origin{float(x)+float(hash%89)/100.f-.44f,float(y)+float((hash>>8)%89)/100.f-.44f};
        float cycle=game.time*.20f+float(hash%997)/997.f;
        float travel=cycle-std::floor(cycle),envelope=std::sin(travel*PI);
        V position=origin+V{.8f,-.24f}*(travel-.5f);
        Vector2 p=screen(position);
        if(p.x<-40||p.x>WorldW+40||p.y<-30||p.y>H+30)continue;
        float gust=windGust(origin,game.time),width=12+float(hash%14)+gust*4;
        float wave=game.time*1.3f+origin.x*.8f+origin.y*.6f;
        Color crest=Fade(rgb(100,156,151),envelope*envelope*(.20f+gust*.22f));
        softWave(p,width,wave,crest);
        softWave({p.x-5,p.y+8},width*.45f,wave+.8f,
                 Fade(rgb(57,107,108),envelope*envelope*.35f));
    }
}

