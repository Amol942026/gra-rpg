#pragma once
#include "items.hpp"
#include "raylib.h"

// Preloaded 2D item illustrations. Each atlas follows stable Kind ordering;
// the equipment cells stay fixed as the remaining item art is introduced.
namespace inventory_icons {
inline Texture2D atlas{},lootAtlas{};
inline constexpr int Cell=128,Columns=12,Width=Columns*Cell;
inline constexpr int Height=(ember::EquipmentKindCount+Columns-1)/Columns*Cell;
static_assert(ember::EquipmentKindCount<=Columns*(Height/Cell));
inline constexpr int LootColumns=8,LootWidth=1024,LootHeight=384;
inline const auto cells=[](){
    std::array<int,ember::ItemKindCount> result{};result.fill(-1);int cell=0;
    for(int kind=1;kind<ember::ItemKindCount;kind++)if(ember::gearSlot(ember::Kind(kind))>=0)result[kind]=cell++;
    return result;
}();
inline int cell(ember::Kind kind){int id=int(kind);return id>=0&&id<ember::ItemKindCount?cells[id]:-1;}
inline const auto lootCells=[](){
    std::array<int,ember::ItemKindCount> result{};result.fill(-1);int cell=0;
    for(int kind=1;kind<ember::ItemKindCount;kind++)if(ember::gearSlot(ember::Kind(kind))<0)result[kind]=cell++;
    return result;
}();
inline int lootCell(ember::Kind kind){int id=int(kind);return id>=0&&id<ember::ItemKindCount?lootCells[id]:-1;}
inline void loadAtlas(Texture2D& texture,const char* path,int width,int height){
    if(texture.id||!FileExists(path))return;
    texture=LoadTexture(path);
    if(texture.id&&(texture.width!=width||texture.height!=height)){UnloadTexture(texture);texture={};return;}
    if(texture.id)SetTextureFilter(texture,TEXTURE_FILTER_BILINEAR);
}
inline void load(){
    loadAtlas(atlas,"assets/sprites/equipment-icons.png",Width,Height);
    loadAtlas(lootAtlas,"assets/sprites/loot-icons.png",LootWidth,LootHeight);
}
inline bool draw(ember::Item value,float x,float y,float scale,float alpha){
    if(value.empty())return false;
    int index=cell(value.kind),columns=Columns;Texture2D texture=atlas;
    if(index<0){index=lootCell(value.kind);columns=LootColumns;texture=lootAtlas;}
    if(!texture.id||index<0)return false;
    Rectangle source{float(index%columns*Cell),float(index/columns*Cell),float(Cell),float(Cell)};
    DrawTexturePro(texture,source,{x-24*scale,y-24*scale,48*scale,48*scale},{0,0},0,Fade(WHITE,alpha));
    return true;
}
inline void unload(){if(atlas.id)UnloadTexture(atlas);if(lootAtlas.id)UnloadTexture(lootAtlas);atlas={};lootAtlas={};}
}
