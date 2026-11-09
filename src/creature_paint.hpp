#pragma once
// Krita material marks, preloaded with world art. Accept the previous atlas
// dimensions for older asset folders and skip any unavailable new cells.
namespace creature_paint {
inline constexpr int Width=512,Tile=128,MaxHeight=640;
inline Texture2D atlas{};
inline std::array<unsigned char,Width*MaxHeight> alpha{};
inline void load(){
 if(atlas.id)return;Image im=LoadImage("assets/sprites/creature-paints.png");
 if(!im.data||im.width!=Width||(im.height!=512&&im.height!=MaxHeight)){if(im.data)UnloadImage(im);return;}
 Color* p=LoadImageColors(im);for(int i=0;i<im.width*im.height;i++)alpha[i]=p[i].a;
 UnloadImageColors(p);atlas=LoadTextureFromImage(im);UnloadImage(im);if(atlas.id)SetTextureFilter(atlas,TEXTURE_FILTER_BILINEAR);
}
inline void unload(){if(atlas.id)UnloadTexture(atlas);atlas={};alpha.fill(0);}
inline bool available(int cell){return atlas.id&&cell>=0&&cell/4<atlas.height/Tile;}
inline Vector2 uv(int cell,float u,float v){return {(cell%4+u)/4.f,(cell/4+v)/(atlas.height/float(Tile))};}
inline bool opaque(Vector2 uv){
 int x=int(uv.x*Width),y=int(uv.y*atlas.height);
 return x>=0&&x<Width&&y>=0&&y<atlas.height&&alpha[y*Width+x]>64;
}
}
