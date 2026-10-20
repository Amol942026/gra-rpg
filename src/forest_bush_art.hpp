#pragma once
namespace forest_bush_art {
// The root shares the (90,208) anchor of picking, sorting and shadows.
// The 2x Krita source is baked once into propArt[2], then released.
inline Texture2D source{};
inline void load(){source=LoadTexture("assets/sprites/forest-bush.png");if(source.id)SetTextureFilter(source,TEXTURE_FILTER_BILINEAR);}
inline bool draw(float x,float y,float size){
 if(!source.id)return false;
 DrawTexturePro(source,{0,0,float(source.width),float(source.height)},{x-90*size,y-208*size,180*size,230*size},{0,0},0,WHITE);return true;
}
inline void unload(){if(source.id)UnloadTexture(source);source={};}
}
