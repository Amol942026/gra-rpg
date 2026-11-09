#pragma once
#include "raylib.h"
#include "spell_definitions.hpp"
#include "generated/spell_motion.hpp"
namespace spell_art {
inline Texture2D atlas{};
inline constexpr int Cell=256;
inline Color tint(ember::Spell spell){return spell==ember::Spell::IceShard?Color{139,227,249,255}:spell==ember::Spell::LightningOrb?Color{163,176,255,255}:Color{255,174,70,255};}
inline void load(){
 if(atlas.id||!FileExists("assets/sprites/spells.png"))return;
 atlas=LoadTexture("assets/sprites/spells.png");
 if(atlas.id&&(atlas.width!=768||atlas.height!=768)){UnloadTexture(atlas);atlas={};return;}
 if(atlas.id)SetTextureFilter(atlas,TEXTURE_FILTER_BILINEAR);
}
inline bool draw(ember::Spell spell,int row,Vector2 center,float width,float height,float rotation,float alpha=1){
 if(!atlas.id||!ember::validSpell(spell))return false;
 DrawTexturePro(atlas,{float((int(spell)-1)*Cell),float(row*Cell),float(Cell),float(Cell)},
  {center.x,center.y,width,height},{width*.5f,height*.5f},rotation,Fade(WHITE,alpha));return true;
}
inline bool icon(ember::Spell spell,float x,float y,float scale=1,float alpha=1){
 if(!ember::validSpell(spell)){DrawCircleLines(int(x),int(y),14*scale,Fade(Color{116,143,146,255},alpha));DrawLineEx({x-7*scale,y+7*scale},{x+7*scale,y-7*scale},2*scale,Fade(Color{116,143,146,255},alpha));return true;}
 return draw(spell,0,{x,y},64*scale,64*scale,0,alpha);
}
inline void unload(){if(atlas.id)UnloadTexture(atlas);atlas={};}
}
