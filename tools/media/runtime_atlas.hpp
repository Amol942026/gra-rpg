#pragma once
#include <algorithm>
#include <cmath>
#include "raylib.h"

// Small optional adapter for exported fixed-cell 2D clips. Load textures once
// during scene setup, retain the clip, draw once per actor, unload on teardown.
// Source PNG rows have positive height (unlike a raylib RenderTexture).
namespace ember_media {
struct SpriteClip {
    Texture2D texture{};
    int width=128,height=128,columns=8,count=60,padding=2;
    float fps=60,pivotX=.5f,pivotY=.9f;
    int frameAt(double seconds) const {
        if(count<=0 || fps<=0 || !std::isfinite(seconds))return 0;
        double phase=std::fmod(std::max(0.,seconds),double(count)/fps);
        return std::min(count-1,int(phase*fps));
    }
    Rectangle source(int frame) const {
        frame=std::clamp(frame,0,std::max(0,count-1));
        int cols=std::max(1,columns);
        return {float((frame%cols)*(width+2*padding)+padding),
                float((frame/cols)*(height+2*padding)+padding),float(width),float(height)};
    }
    void draw(Vector2 anchor,double seconds,float scale=1,Color tint=WHITE) const {
        if(!texture.id || count<=0)return;
        DrawTexturePro(texture,source(frameAt(seconds)),
                       {anchor.x,anchor.y,width*scale,height*scale},
                       {width*scale*pivotX,height*scale*pivotY},0,tint);
    }
};
}
