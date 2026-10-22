#pragma once
#include "generated/menu_motion_data.hpp"

namespace menu_scene {
inline std::array<Texture2D,4> layers{};
inline bool ready(){for(auto t:layers)if(!t.id)return false;return true;}
inline void load(){
 const char* names[]={"background","foreground","mist","glow"};
 for(int i=0;i<4;i++)if(!layers[i].id){
  Image image=LoadImage(TextFormat("assets/menu/%s.png",names[i]));
  if(!image.data)continue;
  if(image.width==1440&&image.height==900){layers[i]=LoadTextureFromImage(image);SetTextureFilter(layers[i],TEXTURE_FILTER_BILINEAR);}
  UnloadImage(image);
 }
}
inline void unload(){for(auto& t:layers){if(t.id)UnloadTexture(t);t={};}}
inline void draw(float time){
 if(!ready()){intro_art::observatory(time,false);intro_art::observatoryObjects(time);return;}
 auto motion=menu_motion::sample(time);
 auto paint=[&](int layer,float x,float y,float alpha=1,float zoom=1){
  DrawTexturePro(layers[layer],{0,0,1440,900},{720-740*zoom+x,450-462.5f*zoom+y,1480*zoom,925*zoom},{0,0},0,Fade(WHITE,alpha));
 };
 paint(0,motion.cameraX,motion.cameraY);
 paint(3,0,0,motion.glow,motion.scale);
 paint(2,motion.mistX,motion.mistY,.8f);
 // Quiet motes and fireflies, deterministic and bounded. The rise fades at
 // both endpoints; the complete background returns to its first pose at 12s.
 for(int i=0;i<48;i++){
  float age=std::fmod(time/menu_motion::Duration+i*.618033989f,1.f);
  float fade=std::sin(PI*age),x=175+(i*113)%680+motion.mistX*(.3f+i%3*.2f),y=810-age*615;
  Color color=i%4?rgb(174,215,181):rgb(238,194,121);
  DrawCircleV({x,y},i%5==0?2.1f:1.05f,Fade(color,fade*(i%4?.43f:.8f)));
  if(i%8==0)DrawCircleV({x,y},5,Fade(color,fade*.07f));
 }
 paint(1,motion.foregroundX,motion.foregroundY);
}
}
