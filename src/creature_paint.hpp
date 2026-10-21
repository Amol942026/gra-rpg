#pragma once
// Original Krita material marks; one 1 MiB sheet, preloaded with world art.
namespace creature_paint {
inline Texture2D atlas{};
inline std::array<unsigned char,512*512> alpha{};
inline void load(){
 if(atlas.id)return;Image im=LoadImage("assets/sprites/creature-paints.png");
 if(!im.data||im.width!=512||im.height!=512){if(im.data)UnloadImage(im);return;}
 Color* p=LoadImageColors(im);for(size_t i=0;i<alpha.size();i++)alpha[i]=p[i].a;
 UnloadImageColors(p);atlas=LoadTextureFromImage(im);UnloadImage(im);if(atlas.id)SetTextureFilter(atlas,TEXTURE_FILTER_BILINEAR);
}
inline void unload(){if(atlas.id)UnloadTexture(atlas);atlas={};alpha.fill(0);}
inline Vector2 uv(int cell,float u,float v){return {(cell%4+u)/4.f,(cell/4+v)/4.f};}
inline bool opaque(Vector2 uv){int x=int(uv.x*512),y=int(uv.y*512);return x>=0&&x<512&&y>=0&&y<512&&alpha[y*512+x]>64;}
}
