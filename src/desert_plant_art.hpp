#pragma once
namespace desert_plant_art {
inline constexpr const char* files[]={
 "assets/media/desert-plants/desert-saguaro.png",
 "assets/media/desert-plants/desert-opuntia.png",
 "assets/media/desert-plants/desert-palm.png"
};
inline std::array<Texture2D,3> sources{};
inline std::array<bool,3> baked{};
inline int slot(int type){return type==7?0:type==8?1:type==12?2:-1;}
inline float scale(int type){return type==7?.58f:type==8?.62f:type==12?2.f:1.f;}
inline bool available(int type){
 const int index=slot(type);
 return index>=0&&sources[index].id;
}
inline void load(){
 baked={};
 for(int i=0;i<3;i++){
  if(FileExists(files[i]))sources[i]=LoadTexture(files[i]);
  if(sources[i].id)SetTextureFilter(sources[i],TEXTURE_FILTER_BILINEAR);
 }
}
inline void draw(int type,int width,int height){
 const int index=slot(type);
 const auto texture=sources[index];
 rlSetBlendFactors(RL_ONE,RL_ZERO,RL_FUNC_ADD);
 BeginBlendMode(BLEND_CUSTOM);
 DrawTexturePro(texture,{0,0,float(texture.width),float(texture.height)},
                {0,0,float(width),float(height)},{},0,WHITE);
 EndBlendMode();
 baked[index]=true;
}
inline void unload(){
 for(auto& source:sources){if(source.id)UnloadTexture(source);source={};}
}
}
