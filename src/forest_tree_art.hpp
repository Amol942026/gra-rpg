#pragma once
namespace forest_tree_art {
// Three native Krita paintings share the legacy root anchor and wind mesh.
// Sources only live during makeArt; gameplay uses the ordinary propArt cache.
inline std::array<Texture2D,3> sources{};
inline std::array<bool,3> baked{};
inline int slot(int type){return type==0?0:type==4?1:type==5?2:-1;}
inline void load(){
 const char* files[]={"assets/sprites/forest-spruce.png","assets/sprites/forest-birch.png","assets/sprites/forest-oak.png"};
 baked={};
 for(int i=0;i<3;i++){sources[i]=LoadTexture(files[i]);if(sources[i].id)SetTextureFilter(sources[i],TEXTURE_FILTER_BILINEAR);}
}
inline bool draw(int type,float x,float y,float size){
 int i=slot(type);if(i<0||!sources[i].id)return false;const auto& source=sources[i];
 // Copy straight RGBA into the transparent cache, including edge coverage.
 // Standard alpha-over here would multiply the antialiased leaf edges twice.
 rlSetBlendFactors(RL_ONE,RL_ZERO,RL_FUNC_ADD);BeginBlendMode(BLEND_CUSTOM);
 DrawTexturePro(source,{0,0,float(source.width),float(source.height)},
  {x-90*size,y-208*size,180*size,230*size},{0,0},0,WHITE);
 EndBlendMode();baked[i]=true;return true;
}
inline void unload(){for(auto& source:sources){if(source.id)UnloadTexture(source);source={};}}
}
