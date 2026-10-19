#pragma once
#include "equipment_appearance.hpp"
namespace equipment_art {
inline Color color(unsigned value){return {static_cast<unsigned char>(value>>16),static_cast<unsigned char>(value>>8),static_cast<unsigned char>(value),255};}
inline void iconInlay(ember::Item value,float x,float y,float size,float alpha){
 auto look=ember::gearAppearance(value);if(!look.present())return;
 auto span=equipment_paint_data::spans[int(value.kind)];Color ink=Fade(color(look.paint.accent),alpha);
 float pixel=size/16;
 for(int i=0;i<span.count;i++){
  auto stroke=equipment_paint_data::strokes[span.start+i];
  DrawRectangleRec({x+(stroke.x-8)*pixel,y+(stroke.y-8)*pixel,stroke.width*pixel,pixel},ink);
 }
}
}
