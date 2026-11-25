#pragma once
// Original layered Krita artwork. One atlas upload; no per-frame file I/O.
namespace treasure_art {
constexpr int Cell=256,States=3,Realms=4,Width=Cell*States,Height=Cell*Realms;
// World position is the center of the base footprint, not its frontmost corner.
constexpr float Scale=.42f,AnchorX=128,AnchorY=183;
inline Texture2D atlas{},outline{};
inline std::array<unsigned char,Width*Height> alpha{};
inline int realm(const TreasureChest& chest){return chest.location==WorldLocation::Swamp?3:chest.location==WorldLocation::Volcano?2:chest.location==WorldLocation::Desert?1:0;}
inline int frame(const TreasureChest& chest){
 if(!chest.opened)return 0;
 for(const auto& value:chest.loot)if(!value.empty())return 1;
 return 2;
}
inline Rectangle bounds(Vector2 p){return {p.x-AnchorX*Scale,p.y-AnchorY*Scale,Cell*Scale,Cell*Scale};}
inline void make(){
 if(atlas.id)return;
 Image image=LoadImage("assets/sprites/treasure-chests.png");
 if(!image.data||image.width!=Width||image.height!=3*Cell){
  if(image.data)UnloadImage(image);
  TraceLog(LOG_WARNING,"TREASURE: Missing or invalid chest atlas");return;
 }
 Image marshRow=LoadImage(marsh_art::SceneryPath);
 ImageResizeCanvas(&image,Width,Height,0,0,BLANK);
 if(marshRow.data){ImageDraw(&image,marshRow,marsh_art::ChestRow,{0,float(3*Cell),float(Width),float(Cell)},WHITE);UnloadImage(marshRow);}
 Color* pixels=LoadImageColors(image);
 for(int i=0;i<Width*Height;i++)alpha[i]=pixels[i].a;
 UnloadImageColors(pixels);atlas=LoadTextureFromImage(image);UnloadImage(image);
 SetTextureFilter(atlas,TEXTURE_FILTER_BILINEAR);
 Image border=GenImageColor(Width,Height,BLANK);
 auto* edge=static_cast<Color*>(border.data);
 constexpr int Radius=5,Opaque=96;
 for(int row=0;row<Realms;row++)for(int col=0;col<States;col++){
  const int start=row*Cell*Width+col*Cell;
  for(int y=0;y<Cell;y++)for(int x=0;x<Cell;x++){
   const int i=start+y*Width+x;
   if(alpha[i]>Opaque)continue;
   bool near=false;
   for(int dy=-Radius;dy<=Radius&&!near;dy++)for(int dx=-Radius;dx<=Radius;dx++){
    const int u=x+dx,v=y+dy;
    if(dx*dx+dy*dy<=Radius*Radius&&u>=0&&u<Cell&&v>=0&&v<Cell&&alpha[start+v*Width+u]>Opaque){near=true;break;}
   }
   if(near)edge[i]=WHITE;
  }
 }
 outline=LoadTextureFromImage(border);UnloadImage(border);
 SetTextureFilter(outline,TEXTURE_FILTER_BILINEAR);
}
inline bool contains(const TreasureChest& chest,Vector2 p,Vector2 point){
 if(!atlas.id)return false;
 Rectangle r=bounds(p);if(!CheckCollisionPointRec(point,r))return false;
 int x=int((point.x-r.x)/Scale),y=int((point.y-r.y)/Scale);
 return x>=0&&x<Cell&&y>=0&&y<Cell&&alpha[(realm(chest)*Cell+y)*Width+frame(chest)*Cell+x]>96;
}
inline void drawScaled(const TreasureChest& chest,Vector2 p,float scale){
 const float s=Scale*scale;
 if(atlas.id)DrawTexturePro(atlas,{float(frame(chest)*Cell),float(realm(chest)*Cell),float(Cell),float(Cell)},
  {p.x-AnchorX*s,p.y-AnchorY*s,Cell*s,Cell*s},{0,0},0,WHITE);
}
inline void draw(const TreasureChest& chest,Vector2 p,Color highlight=BLANK){
 if(highlight.a&&outline.id)DrawTexturePro(outline,{float(frame(chest)*Cell),float(realm(chest)*Cell),float(Cell),float(Cell)},
  bounds(p),{},0,highlight);
 drawScaled(chest,p,1);
}
inline void unload(){if(atlas.id)UnloadTexture(atlas);if(outline.id)UnloadTexture(outline);atlas=outline={};alpha.fill(0);}
}
