#pragma once
#include "items.hpp"
#include "raylib.h"

// Preloaded 2D item illustrations. Each atlas follows stable Kind ordering;
// the equipment cells stay fixed as the remaining item art is introduced.
namespace inventory_icons {
inline Texture2D atlas{},lootAtlas{};
inline RenderTexture2D weaponAtlas{},gearAtlas{};
inline constexpr int WeaponCell=96,WeaponColumns=8;
inline const auto weaponCells=[](){
    std::array<int,ember::ItemKindCount> result{};result.fill(-1);
    int cell=0;
    for(int kind=1;kind<ember::ItemKindCount;kind++)if(ember::gearSlot(ember::Kind(kind))==0)result[kind]=cell++;
    return result;
}();
inline int weaponCell(ember::Kind kind){
    const int id=int(kind);
    return id>=0&&id<ember::ItemKindCount?weaponCells[id]:-1;
}
inline constexpr int Cell=128,Columns=12,Width=Columns*Cell;
inline constexpr int LegacyEquipmentCount=ember::EquipmentKindCount-ember::ProtectiveGearCount;
inline constexpr int Height=(LegacyEquipmentCount+Columns-1)/Columns*Cell;
inline constexpr int GearWidth=Cell*(ember::ProtectiveGearCount+ember::OpenHelmetKinds.size());
static_assert(GearWidth<=2048);
inline constexpr int gearCell(ember::Kind kind){
    if(ember::protectiveGear(kind))return int(kind)-ember::FirstProtectiveGear;
    for(size_t i=0;i<ember::OpenHelmetKinds.size();i++)
        if(ember::OpenHelmetKinds[i]==kind)return ember::ProtectiveGearCount+int(i);
    return -1;
}
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
    const int gear=gearCell(value.kind);
    if(gear>=0){
        if(!gearAtlas.id)return false;
        const Rectangle source{float(gear*Cell),0,float(Cell),-float(Cell)};
        DrawTexturePro(gearAtlas.texture,source,{x-24*scale,y-24*scale,48*scale,48*scale},{},0,Fade(WHITE,alpha));
        return true;
    }
    const int weapon=weaponCell(value.kind);
    if(weapon>=0&&weaponAtlas.id){
        const auto texture=weaponAtlas.texture;
        const Rectangle source{float(weapon%WeaponColumns*WeaponCell),
            float(texture.height-(weapon/WeaponColumns+1)*WeaponCell),float(WeaponCell),-float(WeaponCell)};
        DrawTexturePro(texture,source,{x-24*scale,y-24*scale,48*scale,48*scale},{},0,Fade(WHITE,alpha));
        return true;
    }
    int index=cell(value.kind),columns=Columns;Texture2D texture=atlas;
    if(index<0){index=lootCell(value.kind);columns=LootColumns;texture=lootAtlas;}
    if(!texture.id||index<0)return false;
    Rectangle source{float(index%columns*Cell),float(index/columns*Cell),float(Cell),float(Cell)};
    DrawTexturePro(texture,source,{x-24*scale,y-24*scale,48*scale,48*scale},{0,0},0,Fade(WHITE,alpha));
    return true;
}
inline void unload(){
    if(atlas.id)UnloadTexture(atlas);
    if(lootAtlas.id)UnloadTexture(lootAtlas);
    if(weaponAtlas.id)UnloadRenderTexture(weaponAtlas);
    if(gearAtlas.id)UnloadRenderTexture(gearAtlas);
    atlas={};lootAtlas={};weaponAtlas={};gearAtlas={};
}
}
