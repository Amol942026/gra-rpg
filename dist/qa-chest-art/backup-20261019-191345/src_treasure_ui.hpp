#pragma once
// Reuse the existing HUD and coin icon while Figma artwork is unavailable.
// This marker is intentionally text, not an asset generated outside Figma.
Rectangle treasureBounds(Vector2 p){return {p.x-46,p.y-58,92,56};}
void drawTreasureChest(const TreasureChest& chest){
 Vector2 p=screen(chest.pos);if(p.x<-60||p.x>WorldW+60||p.y<0||p.y>H+60)return;
 bool selected=hovered.kind==HoverKind::Chest&&hovered.index==chest.id;
 Color color=chest.opened?Muted:Gold;
 Rectangle box=treasureBounds(p);panel(box,Panel);if(selected)DrawRectangleLinesEx(box,2,Gold);
 int count=0;for(const auto& value:chest.loot)count+=!value.empty();
 if(!chest.opened||count)icon(item(Kind::Gold),p.x,p.y-37,.48f);
 center(chest.opened?(count?"OTWARTA":"PUSTA"):"SKRZYNIA",p.x,p.y-21,14,color);
}
