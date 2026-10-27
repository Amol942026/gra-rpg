#pragma once
// Original Krita sandstone / gem cutouts, water baked by native Synfig.
// Two preloaded atlases, fixed foot anchors. No production work in frame loops.
namespace fountain_art {
inline std::array<Texture2D,FountainCount> atlas{},dry{};
inline std::array<std::vector<unsigned char>,FountainCount> masks;
inline void load(){
 for(int i=0;i<FountainCount;i++){
  std::string name=std::string("assets/sprites/fountain-")+(i==0?"mana":"health");
  atlas[i]=LoadTexture((name+".png").c_str());dry[i]=LoadTexture((name+"-empty.png").c_str());
  if(atlas[i].id)SetTextureFilter(atlas[i],TEXTURE_FILTER_BILINEAR);
  if(dry[i].id){
   SetTextureFilter(dry[i],TEXTURE_FILTER_BILINEAR);Image im=LoadImageFromTexture(dry[i]);Color* pixels=LoadImageColors(im);
   masks[i].resize(im.width*im.height);for(int p=0;p<im.width*im.height;p++)masks[i][p]=pixels[p].a>=80;
   UnloadImageColors(pixels);UnloadImage(im);
  }
 }
}
inline int frame(int id){
 if(!game.fountainUsed[id])return int(game.time*24)%24;
 return 24+std::clamp(int((1-game.fountainDrain[id]/FountainDrainDuration)*24),0,23);
}
inline bool contains(int id,Vector2 point){
 Vector2 p=screen(DesertFountains[id]);int x=int(std::floor(point.x-p.x+64)),y=int(std::floor(point.y-p.y+148));
 return x>=0&&x<128&&y>=0&&y<160&&!masks[id].empty()&&masks[id][y*128+x];
}
inline void draw(int id,bool active=true){
 Vector2 p=screen(DesertFountains[id]);if(p.x<-90||p.x>WorldW+90||p.y<-30||p.y>H+165)return;
 bool full=active&&(!game.fountainUsed[id]||game.fountainDrain[id]>0);
 if(active&&hovered.kind==HoverKind::Fountain&&hovered.index==id)ellipseOutline(p.x,p.y,58,23,game.fountainUsed[id]?Muted:id==0?Blue:rgb(238,123,123));
 int f=frame(id);Texture2D tex=full?atlas[id]:dry[id];if(!tex.id)return;
 Rectangle source=full?Rectangle{float(f%8*128),float(f/8*160),128,160}:Rectangle{0,0,128,160};
 DrawTexturePro(tex,source,{p.x-64,p.y-148,128,160},{0,0},0,WHITE);
}
inline void unload(){for(auto& t:atlas)if(t.id)UnloadTexture(t);for(auto& t:dry)if(t.id)UnloadTexture(t);}
}
