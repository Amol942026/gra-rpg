#pragma once
// Map labels follow world positions and snap to the final framebuffer grid.
struct MapNameplate {
 std::string name,hint;Vector2 anchor;float size;Color color,hintColor;
 bool title=false,bold=true;
};
template<class Emit> void forEachMapNameplate(Emit emit){
 if(!game.inTower())return;
 if(game.towerFloor==0){
  Vector2 p=screen(MagePosition);
  emit(MapNameplate{"ELDRIN",game.mageReachable()?"E / PPM · ROZMAWIAJ":"MAG · MENTOR",
   {p.x,p.y-144},15,rgb(205,172,248),game.mageReachable()?Gold:Muted,true,false});
 }
 if(game.towerFloor==MerchantFloor){
  Vector2 p=screen(MerchantPosition);
  emit(MapNameplate{"MIRON",game.merchantReachable()?"E / PPM · HANDEL":"HANDLARZ",
   {p.x,p.y-138},20,Gold,game.merchantReachable()?Text:Muted});
 }
}
Rectangle mapNameplateBounds(const MapNameplate& label){
 float width=std::max(typography.width(label.name,label.size,label.title,label.bold),
                      typography.width(label.hint,14,false,true))+16;
 width=std::ceil(width*renderScale)/renderScale;
 float height=std::ceil((label.size+8+(label.hint.empty()?0:20))*renderScale)/renderScale;
 return {std::round((label.anchor.x-width/2)*renderScale)/renderScale,
         std::round((label.anchor.y-4)*renderScale)/renderScale,width,height};
}
void drawMapNameplate(const MapNameplate& label){
 Rectangle box=mapNameplateBounds(label);
 // Snap both text lines together. A glyph shadow keeps them legible over
 // bright scenery while leaving the space around the letters transparent.
 float x=box.x+box.width/2,y=box.y+4;
 auto drawLine=[&](const std::string& text,float top,float size,Color color,bool title,bool bold){
  float left=x-typography.width(text,size,title,bold)/2;
  typography.draw(text,left+1/renderScale,top+1/renderScale,size,rgb(12,16,25,225),title,bold);
  typography.draw(text,left,top,size,color,title,bold);
 };
 drawLine(label.name,y,label.size,label.color,label.title,label.bold);
 if(!label.hint.empty())drawLine(label.hint,y+label.size+6,14,label.hintColor,false,true);
}
void drawWorldLabels(){
 clip(0,0,WorldW,H);
 forEachMapNameplate([](const MapNameplate& label){drawMapNameplate(label);});
 drawCombatEffects();
 EndScissorMode();
}
