#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

namespace hero_paint::turn {
inline constexpr int Columns=8,Cell=128,Size=Columns*Cell,Views=Columns*Columns;
inline Texture2D atlas{};
inline Shader shader{};
inline std::vector<unsigned char> coverage;

inline void unload(){
    if(atlas.id)UnloadTexture(atlas);
    if(shader.id&&shader.id!=rlGetShaderIdDefault())UnloadShader(shader);
    atlas={};shader={};coverage.clear();
}

inline void load(Color flashColor){
    const char* path="assets/sprites/hero-head-turn.png";
    if(atlas.id||!FileExists(path))return;
    Image image=LoadImage(path);
    if(!image.data)return;
    if(image.width!=Size||image.height!=Size){UnloadImage(image);return;}
    // Each square contains an already warped painting. Blend the two adjacent
    // poses once per fragment, including the wrap from the last tile to front.
    shader=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 flashColor;
out vec4 finalColor;
void main(){
    ivec2 cell=ivec2(floor(fragTexCoord*8.0));
    int index=(cell.y*8+cell.x+1)%64;
    vec2 offset=vec2(ivec2(index%8,index/8)-cell)/8.0;
    vec4 paint=mix(texture(texture0,fragTexCoord),texture(texture0,fragTexCoord+offset),fragColor.r);
    if(paint.a<=0.0039)discard;
    vec3 color=mix(paint.rgb/paint.a,flashColor,fragColor.g);
    finalColor=vec4(color,paint.a*colDiffuse.a*fragColor.a);
})GLSL");
    if(!IsShaderValid(shader)||shader.id==rlGetShaderIdDefault()){
        UnloadImage(image);unload();return;
    }
    const float flash[]={flashColor.r/255.f,flashColor.g/255.f,flashColor.b/255.f};
    SetShaderValue(shader,GetShaderLocation(shader,"flashColor"),flash,SHADER_UNIFORM_VEC3);
    ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageAlphaPremultiply(&image);
    atlas=LoadTextureFromImage(image);
    if(atlas.id){
        SetTextureFilter(atlas,TEXTURE_FILTER_BILINEAR);
        coverage.resize(Size*Size);
        const Color* pixels=static_cast<const Color*>(image.data);
        for(size_t i=0;i<coverage.size();i++)coverage[i]=pixels[i].a;
    }
    UnloadImage(image);
    if(!atlas.id)unload();
}

struct Frame {
    int index;
    unsigned char blend;
};
inline Frame sample(float yaw){
    const float revolutions=yaw/(2*PI);
    const float position=(revolutions-std::floor(revolutions))*Views;
    const int first=int(position);
    return {first%Views,static_cast<unsigned char>(std::round((position-first)*255.f))};
}
inline Vector2 uv(int index,float u,float v){
    return {((index%Columns)*Cell+.5f+(Cell-1)*u)/Size,
            ((index/Columns)*Cell+.5f+(Cell-1)*v)/Size};
}
inline bool opaque(Vector2 texel,unsigned char blend){
    // Match bilinear filtering in the same pair of poses for world picking
    // and the foliage-occlusion mask. The half-texel inset keeps both samples
    // inside their own cell at an outside edge or a row wrap.
    const int col=std::clamp(int(texel.x*Columns),0,Columns-1);
    const int row=std::clamp(int(texel.y*Columns),0,Columns-1);
    const int first=row*Columns+col,next=(first+1)%Views;
    const float x=std::clamp(texel.x*Size-col*Cell-.5f,0.f,float(Cell-1));
    const float y=std::clamp(texel.y*Size-row*Cell-.5f,0.f,float(Cell-1));
    const int x0=int(x),y0=int(y),x1=std::min(x0+1,Cell-1),y1=std::min(y0+1,Cell-1);
    const auto filtered=[&](int view){
        const auto* p=coverage.data()+(view/Columns)*Cell*Size+(view%Columns)*Cell;
        const float top=p[y0*Size+x0]+(p[y0*Size+x1]-p[y0*Size+x0])*(x-x0);
        const float bottom=p[y1*Size+x0]+(p[y1*Size+x1]-p[y1*Size+x0])*(x-x0);
        return top+(bottom-top)*(y-y0);
    };
    const float a=filtered(first);
    return a+(filtered(next)-a)*(blend/255.f)>64.f;
}
}
