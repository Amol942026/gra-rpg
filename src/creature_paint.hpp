#pragma once
// Krita material marks, preloaded with world art. Accept the previous atlas
// dimensions for older asset folders and skip any unavailable new cells.
namespace creature_paint {
inline constexpr int Width=512,Tile=128,MaxHeight=640;
inline Texture2D atlas{};
inline Texture2D cobraSkin{};
inline Texture2D cobraParts{};
inline std::array<unsigned char,Width*MaxHeight> alpha{};
inline constexpr int CobraCell=10;
inline constexpr int CobraHoodFront=20,CobraHoodBack=21,CobraHead=22,CobraJaw=23;
inline bool solidCobraSkin=false;
inline Texture2D loadCobraPaint(const char* path){
 if(!FileExists(path))return {};
 auto paint=LoadTexture(path);
 if(paint.id){
  GenTextureMipmaps(&paint);
  SetTextureFilter(paint,TEXTURE_FILTER_TRILINEAR);
  SetTextureFilter(paint,TEXTURE_FILTER_ANISOTROPIC_4X);
  rlTextureParameters(paint.id,RL_TEXTURE_MIPMAP_BIAS_RATIO,-20);
  SetTextureWrap(paint,TEXTURE_WRAP_CLAMP);
 }
 return paint;
}
inline void load(){
 if(atlas.id)return;
 if(!cobraSkin.id)cobraSkin=loadCobraPaint("assets/sprites/cobra-skin.png");
 if(!cobraParts.id)cobraParts=loadCobraPaint("assets/sprites/cobra-parts.png");
 Image im=LoadImage("assets/sprites/creature-paints.png");
 if(!im.data||im.width!=Width||(im.height!=512&&im.height!=MaxHeight)){if(im.data)UnloadImage(im);return;}
 Color* p=LoadImageColors(im);for(int i=0;i<im.width*im.height;i++)alpha[i]=p[i].a;
 solidCobraSkin=true;
 for(int y=0;y<Tile;y++)for(int x=0;x<Tile;x++)
  if(alpha[(CobraCell/4*Tile+y)*Width+CobraCell%4*Tile+x]!=255)solidCobraSkin=false;
 UnloadImageColors(p);atlas=LoadTextureFromImage(im);UnloadImage(im);if(atlas.id)SetTextureFilter(atlas,TEXTURE_FILTER_BILINEAR);
}
inline void unload(){
 if(atlas.id)UnloadTexture(atlas);
 if(cobraSkin.id)UnloadTexture(cobraSkin);
 if(cobraParts.id)UnloadTexture(cobraParts);
 atlas={};cobraSkin={};cobraParts={};alpha.fill(0);solidCobraSkin=false;
}
inline bool opaqueCobraTexture(unsigned id){return id&&(id==cobraSkin.id||id==cobraParts.id);}
inline unsigned texture(int cell){
 if(cell==CobraCell&&cobraSkin.id)return cobraSkin.id;
 if(cell>=CobraHoodFront&&cell<=CobraJaw)return cobraParts.id;
 return cell>=0&&cell/4<atlas.height/Tile?atlas.id:0;
}
inline bool available(int cell){return texture(cell)!=0;}
inline Vector2 uv(int cell,float u,float v){
 if(cell==CobraCell&&cobraSkin.id)return {u,v};
 if(cell>=CobraHoodFront&&cell<=CobraJaw){
  const int patch=cell-CobraHoodFront;
  return {(256*(patch%2)+8.5f+239*u)/512.f,(256*(patch/2)+8.5f+239*v)/512.f};
 }
 return {(cell%4+u)/4.f,(cell/4+v)/(atlas.height/float(Tile))};
}
inline bool opaque(Vector2 uv){
 int x=int(uv.x*Width),y=int(uv.y*atlas.height);
 return x>=0&&x<Width&&y>=0&&y<atlas.height&&alpha[y*Width+x]>64;
}
}
