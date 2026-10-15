#pragma once
#include "fallen_forest_art.hpp"
namespace fallen_art {
constexpr int Width=192,Height=128,Density=2,OriginX=96,OriginY=80;
struct Sprite {Texture2D body{},edge{};std::vector<unsigned char> mask;shadow_geometry::Sprite ground;};
inline std::array<std::array<Sprite,8>,EnemyTypeCount> sprites{};
inline int prepared=0;
inline void storeSprite(Sprite& sprite,const Image& image){
 sprite.body=LoadTextureFromImage(image);SetTextureFilter(sprite.body,TEXTURE_FILTER_BILINEAR);
 Color* pixels=LoadImageColors(image);sprite.mask.resize(image.width*image.height);
 Image rim=GenImageColor(image.width,image.height,BLANK);
 for(int y=0;y<image.height;y++)for(int x=0;x<image.width;x++){
  int n=y*image.width+x;sprite.mask[n]=pixels[n].a>=128;
  if(sprite.mask[n])continue;bool nearby=false;
  for(int dy=-3;dy<=3&&!nearby;dy++)for(int dx=-3;dx<=3&&!nearby;dx++)if(dx*dx+dy*dy<=9&&x+dx>=0&&x+dx<image.width&&y+dy>=0&&y+dy<image.height)nearby=pixels[(y+dy)*image.width+x+dx].a>=160;
  if(nearby)ImageDrawPixel(&rim,x,y,{255,255,255,220});
 }
 sprite.edge=LoadTextureFromImage(rim);SetTextureFilter(sprite.edge,TEXTURE_FILTER_BILINEAR);UnloadImage(rim);UnloadImageColors(pixels);prepared++;
}
inline void init(){
 if(prepared)return;
 const char* path="assets/generated/corpses-v39.png";
 Image atlas=FileExists(path)?LoadImage(path):Image{};
 bool baked=atlas.data&&atlas.width==Width*Density*8&&atlas.height==Height*Density*EnemyTypeCount;
 if(!baked){if(atlas.data)UnloadImage(atlas);atlas=GenImageColor(Width*Density*8,Height*Density*EnemyTypeCount,BLANK);}
 for(int type=0;type<EnemyTypeCount;type++)for(int direction=0;direction<8;direction++){
  Rectangle cell{float(direction*Width*Density),float(type*Height*Density),float(Width*Density),float(Height*Density)};
  Image body{};
  if(baked)body=ImageFromImage(atlas,cell);
  else{
   RenderTexture2D target=LoadRenderTexture(Width*Density,Height*Density);BeginTextureMode(target);ClearBackground(BLANK);
   actor_art::Rig rig(OriginX*Density,OriginY*Density,Density,Direction(direction));
   if(type>=8)actor_art::buildDesertCorpse(rig,type);else actor_art::buildFallenForest(rig,type);rig.draw();EndTextureMode();
   body=LoadImageFromTexture(target.texture);ImageFlipVertical(&body);UnloadRenderTexture(target);
   ImageDraw(&atlas,body,{0,0,float(body.width),float(body.height)},cell,WHITE);
  }
  storeSprite(sprites[type][direction],body);UnloadImage(body);
  actor_art::Rig mesh(0,0,1,Direction(direction),true);
  if(type>=8)actor_art::buildDesertCorpse(mesh,type);else actor_art::buildFallenForest(mesh,type);
  sprites[type][direction].ground=shadow_geometry::bake(mesh.shadows());
 }
 if(!baked){fs::create_directories("assets/generated");ExportImage(atlas,path);}
 UnloadImage(atlas);
}
inline void unload(){for(auto& species:sprites)for(auto& sprite:species){if(sprite.body.id)UnloadTexture(sprite.body);if(sprite.edge.id)UnloadTexture(sprite.edge);shadow_geometry::unload(sprite.ground);sprite={};}prepared=0;}
}
bool corpseArt(const Enemy& e,float x,float y,Color outline=BLANK,const Vector2* probe=nullptr){
 int type=std::clamp(e.type,0,EnemyTypeCount-1),direction=std::clamp(int(e.animation.direction),0,7);
 const auto& sprite=fallen_art::sprites[type][direction];if(!sprite.body.id)return false;
 if(probe){
  int px=int(std::floor((probe->x-x+fallen_art::OriginX)*fallen_art::Density));
  int py=int(std::floor((probe->y-y+fallen_art::OriginY)*fallen_art::Density));
  return px>=0&&py>=0&&px<sprite.body.width&&py<sprite.body.height&&sprite.mask[py*sprite.body.width+px];
 }
 Rectangle source{0,0,float(sprite.body.width),float(sprite.body.height)},dest{x-fallen_art::OriginX,y-fallen_art::OriginY,float(fallen_art::Width),float(fallen_art::Height)};
 if(outline.a)DrawTexturePro(sprite.edge,source,dest,{0,0},0,outline);
 BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);DrawTexturePro(sprite.body,source,dest,{0,0},0,WHITE);native_world::endBlendMode();return false;
}
