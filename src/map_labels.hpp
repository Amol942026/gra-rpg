#pragma once
// Map labels follow world positions, but their glyphs and plates share the
// final framebuffer pixel grid, shared with the directly rendered world.
struct MapNameplate {
 std::string name,hint;Vector2 anchor;float size;Color color,hintColor;
 bool title=false,bold=true;
};
template<class Emit> void forEachMapNameplate(Emit emit){
 if(!game.inTower())return;
 for(int direction:{1,-1})if(game.stairPresent(direction)){
  Vector2 p=screen(towerStairBase(direction));
  bool hot=hovered.kind==HoverKind::TowerStair&&hovered.index==direction;
  emit(MapNameplate{direction==1?"SCHODY W GÓRĘ":"SCHODY W DÓŁ",
   game.stairReachable(direction)?"E / LPM / PPM":"",
   {p.x+(direction==1?-20:TowerStairRun*20),p.y+(direction==1?29:92)},
   14,hot?Text:direction==1?Gold:Blue,Text});
 }
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
 DrawRectangleRec(box,rgb(12,18,26));
 DrawRectangleLinesEx(box,1/renderScale,rgb(66,69,67));
 // Both lines are placed relative to the snapped plate, so their raster and
 // padding stay identical at every fractional phase of camera movement.
 float x=box.x+box.width/2,y=box.y+4;
 float width=typography.width(label.name,label.size,label.title,label.bold);
 typography.draw(label.name,x-width/2,y,label.size,label.color,label.title,label.bold);
 if(!label.hint.empty()){
  float hintWidth=typography.width(label.hint,14,false,true);
  typography.draw(label.hint,x-hintWidth/2,y+label.size+6,14,label.hintColor,false,true);
 }
}
void drawWorldLabels(){
 clip(0,0,WorldW,H);
 forEachMapNameplate([](const MapNameplate& label){drawMapNameplate(label);});
 drawCombatEffects();
 EndScissorMode();
}
