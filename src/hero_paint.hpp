#pragma once
#include <array>
// Original layered Krita paint sheet; loaded at scene setup, never in draw().
namespace hero_paint {
inline Texture2D atlas{};
inline std::array<unsigned char,512*384> alpha{};
inline void load(){
 if(atlas.id)return;
 Image image=LoadImage("assets/sprites/hero-paints.png");
 if(!image.data||image.width!=512||image.height!=384){if(image.data)UnloadImage(image);return;}
 Color* pixels=LoadImageColors(image);for(size_t i=0;i<alpha.size();i++)alpha[i]=pixels[i].a;
 UnloadImageColors(pixels);atlas=LoadTextureFromImage(image);UnloadImage(image);
 if(atlas.id)SetTextureFilter(atlas,TEXTURE_FILTER_BILINEAR);
}
inline void unload(){if(atlas.id)UnloadTexture(atlas);atlas={};alpha.fill(0);}
inline bool opaque(Vector2 uv){
 int x=int(uv.x*512),y=int(uv.y*384);return x>=0&&x<512&&y>=0&&y<384&&alpha[y*512+x]>64;
}
inline Vector2 uv(int cell,float u,float v){return {(cell%4+u)/4.f,(cell/4+v)/3.f};}
}
