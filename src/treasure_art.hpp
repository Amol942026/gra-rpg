#pragma once
// Original layered Krita artwork. One atlas upload; no per-frame file I/O.
namespace treasure_art {
constexpr int Cell=256,States=3,Realms=3,Width=Cell*States,Height=Cell*Realms;
// World position is the center of the base footprint, not its frontmost corner.
constexpr float Scale=.42f,AnchorX=128,AnchorY=183;
inline Texture2D atlas{};
inline std::array<unsigned char,Width*Height> alpha{};
inline int realm(const TreasureChest& chest){return chest.location==WorldLocation::Volcano?2:chest.location==WorldLocation::Desert?1:0;}
inline int frame(const TreasureChest& chest){
 if(!chest.opened)return 0;
 for(const auto& value:chest.loot)if(!value.empty())return 1;
 return 2;
}
inline Rectangle bounds(Vector2 p){return {p.x-AnchorX*Scale,p.y-AnchorY*Scale,Cell*Scale,Cell*Scale};}
inline void make(){
 if(atlas.id)return;
 Image image=LoadImage("assets/sprites/treasure-chests.png");
 if(!image.data||image.width!=Width||image.height!=Height){
  if(image.data)UnloadImage(image);
  TraceLog(LOG_WARNING,"TREASURE: Missing or invalid chest atlas");return;
 }
 Color* pixels=LoadImageColors(image);
 for(int i=0;i<Width*Height;i++)alpha[i]=pixels[i].a;
 UnloadImageColors(pixels);atlas=LoadTextureFromImage(image);UnloadImage(image);
 SetTextureFilter(atlas,TEXTURE_FILTER_BILINEAR);
}
inline bool contains(const TreasureChest& chest,Vector2 p,Vector2 point){
 if(!atlas.id)return false;
 Rectangle r=bounds(p);if(!CheckCollisionPointRec(point,r))return false;
 int x=int((point.x-r.x)/Scale),y=int((point.y-r.y)/Scale);
 return x>=0&&x<Cell&&y>=0&&y<Cell&&alpha[(realm(chest)*Cell+y)*Width+frame(chest)*Cell+x]>96;
}
inline void draw(const TreasureChest& chest,Vector2 p){
 if(atlas.id)DrawTexturePro(atlas,{float(frame(chest)*Cell),float(realm(chest)*Cell),float(Cell),float(Cell)},bounds(p),{0,0},0,WHITE);
}
inline void unload(){if(atlas.id)UnloadTexture(atlas);atlas={};alpha.fill(0);}
}
