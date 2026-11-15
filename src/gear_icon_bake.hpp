#pragma once

namespace inventory_icons {
inline void bakeGear(){
    if(gearAtlas.id)return;
    gearAtlas=LoadRenderTexture(GearWidth,Cell);
    if(!gearAtlas.id)return;
    const auto sample=LoadRenderTexture(Cell,Cell);
    if(!sample.id){UnloadRenderTexture(gearAtlas);gearAtlas={};return;}
    BeginTextureMode(gearAtlas);
    ClearBackground(BLANK);
    EndTextureMode();
    for(int id=1;id<ItemKindCount;id++){
        const Kind kind=Kind(id);
        const int cell=gearCell(kind);
        if(cell<0)continue;
        const auto look=gearAppearance(item(kind));
        const bool shield=actor_art::round_shield_art::has(kind);
        const bool openHelmet=actor_art::hero_headwear::open(kind);
        const float y=Cell*.5f+(openHelmet?11.f:0.f);
        actor_art::Rig rig(Cell*.5f,y,shield?4.05f:openHelmet?3.55f:3.65f,Direction::SE);
        rig.setSmoothSurfaces();
        if(shield){
            rig.setFacing(PI*.5f-.38f);
            actor_art::round_shield_art::draw(rig,look);
        }else if(openHelmet)actor_art::hero_headwear::drawOpen(rig,0,look);
        else actor_art::hero_headwear::drawClosed(rig,0,look);
        BeginTextureMode(sample);
        ClearBackground(BLANK);
        rig.draw();
        EndTextureMode();
        BeginTextureMode(gearAtlas);
        // Copy coverage verbatim. Rendering an alpha-blended copy would
        // attenuate the contour once before the HUD blends it again.
        rlSetBlendFactors(RL_ONE,RL_ZERO,RL_FUNC_ADD);
        BeginBlendMode(BLEND_CUSTOM);
        DrawTextureRec(sample.texture,{0,0,float(Cell),-float(Cell)},{float(cell*Cell),0},WHITE);
        EndBlendMode();
        EndTextureMode();
    }
    UnloadRenderTexture(sample);
    GenTextureMipmaps(&gearAtlas.texture);
    SetTextureFilter(gearAtlas.texture,TEXTURE_FILTER_TRILINEAR);
}
}
