#pragma once
Rectangle treasureBounds(Vector2 p){return treasure_art::bounds(p);}
bool treasureContains(const TreasureChest& chest,Vector2 point){return treasure_art::contains(chest,screen(chest.pos),point);}
void drawTreasureChest(const TreasureChest& chest){
 Vector2 p=screen(chest.pos);if(p.x<-60||p.x>WorldW+60||p.y<0||p.y>H+60)return;
 bool selected=hovered.kind==HoverKind::Chest&&hovered.index==chest.id;
 treasure_art::draw(chest,p,selected?Gold:BLANK);
}
