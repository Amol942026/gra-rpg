#pragma once
// Preloaded restoration fountains with separate sandstone and basalt bodies.
namespace fountain_art {
inline std::array<Texture2D,FountainCount> atlas{},dry{},volcanoAtlas{},volcanoDry{};
inline std::array<std::array<std::vector<unsigned char>,FountainCount>,2> masks;
inline void load(){
 for(int realm=0;realm<2;realm++)for(int i=0;i<FountainCount;i++){
  const std::string name=std::string(realm==0?"assets/sprites/fountain-":"assets/sprites/fountain-volcano-")+(i==0?"mana":"health");
  auto& wet=(realm==0?atlas:volcanoAtlas)[i];auto& empty=(realm==0?dry:volcanoDry)[i];
  wet=LoadTexture((name+".png").c_str());empty=LoadTexture((name+"-empty.png").c_str());
  if(wet.id)SetTextureFilter(wet,TEXTURE_FILTER_BILINEAR);
  if(empty.id){
   SetTextureFilter(empty,TEXTURE_FILTER_BILINEAR);Image im=LoadImageFromTexture(empty);Color* pixels=LoadImageColors(im);
   auto& mask=masks[realm][i];mask.resize(im.width*im.height);for(int p=0;p<im.width*im.height;p++)mask[p]=pixels[p].a>=80;
   UnloadImageColors(pixels);UnloadImage(im);
  }
 }
}
inline int frame(int id){
 if(!game.fountainSpent(id))return int(game.time*24)%24;
 return 24+std::clamp(int((1-game.fountainDraining(id)/FountainDrainDuration)*24),0,23);
}
inline bool contains(int id,Vector2 point){
 Vector2 p=screen(game.fountains()[id]);int x=int(std::floor(point.x-p.x+64)),y=int(std::floor(point.y-p.y+148));
 const auto& mask=masks[game.inVolcano()][id];
 return x>=0&&x<128&&y>=0&&y<160&&!mask.empty()&&mask[y*128+x];
}
inline void draw(int id,bool active=true){
 Vector2 p=screen(game.fountains()[id]);if(p.x<-90||p.x>WorldW+90||p.y<-30||p.y>H+165)return;
 bool full=active&&(!game.fountainSpent(id)||game.fountainDraining(id)>0);
 if(active&&hovered.kind==HoverKind::Fountain&&hovered.index==id)ellipseOutline(p.x,p.y,58,23,game.fountainSpent(id)?Muted:id==0?Blue:rgb(238,123,123));
 int f=frame(id);
 const bool volcano=game.inVolcano();
 Texture2D tex=full?(volcano?volcanoAtlas:atlas)[id]:(volcano?volcanoDry:dry)[id];if(!tex.id)return;
 Rectangle source=full?Rectangle{float(f%8*128),float(f/8*160),128,160}:Rectangle{0,0,128,160};
 DrawTexturePro(tex,source,{p.x-64,p.y-148,128,160},{0,0},0,WHITE);
}
inline void unload(){for(auto* group:{&atlas,&dry,&volcanoAtlas,&volcanoDry})for(auto& t:*group){if(t.id)UnloadTexture(t);t={};}}
}
