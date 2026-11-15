#pragma once

namespace inventory_icons {
inline void bakeWeapons(){
    if(weaponAtlas.id)return;
    const int count=*std::max_element(weaponCells.begin(),weaponCells.end())+1;
    const int rows=(count+WeaponColumns-1)/WeaponColumns;
    weaponAtlas=LoadRenderTexture(WeaponCell*WeaponColumns,WeaponCell*rows);
    if(!weaponAtlas.id)return;
    auto sample=LoadRenderTexture(WeaponCell,WeaponCell);
    if(!sample.id){UnloadRenderTexture(weaponAtlas);weaponAtlas={};return;}
    BeginTextureMode(weaponAtlas);
    ClearBackground(BLANK);
    EndTextureMode();
    for(int id=1;id<ItemKindCount;id++){
        const Kind kind=Kind(id);
        const int cell=weaponCell(kind);
        if(cell<0)continue;
        const auto look=gearAppearance(item(kind));
        const auto finish=actor_art::hero_weapon_art::finish(look);
        using namespace actor_art;
        using namespace hero_weapon_art;
        const float tilt=look.shape==Kind::Bow?.32f:.55f;
        const P3 axis{std::sin(tilt),0,std::cos(tilt)},right{axis.z,0,-axis.r};
        const auto frame=basis(axis,right);
        const bool spearItem=look.shape==Kind::Spear;
        const bool shortBlade=look.shape==Kind::Dagger;
        const float tip=spearItem?weapon_profiles::spear(kind).tipOffset():0.f;
        const float scale=spearItem?81.f/((tip-weapon_profiles::SpearHeel)*axis.z):shortBlade?2.45f:look.shape==Kind::Sword?1.76f:
            look.shape==Kind::Bow?43.f/(weapon_profiles::bow(kind).half*std::cos(tilt)+2.4f):1.52f;
        const float center=spearItem?(tip+weapon_profiles::SpearHeel)*.5f:
            look.shape==Kind::Sword?13.f:look.shape==Kind::Axe?11.f:shortBlade?8.f:0.f;
        Rig rig(WeaponCell*.5f,WeaponCell*.5f,scale,Direction::S);
        rig.setSmoothSurfaces();
        const P3 grip=axis*-center;
        if(spearItem)spear(rig,grip,frame,0,finish,kind);
        else if(look.shape==Kind::Bow)bow(rig,grip,axis,right,{},finish,kind,-1,false);
        else melee(rig,grip,frame,kind,1,finish);
        BeginTextureMode(sample);
        ClearBackground(BLANK);
        rig.draw();
        EndTextureMode();
        BeginTextureMode(weaponAtlas);
        rlSetBlendFactors(RL_ONE,RL_ZERO,RL_FUNC_ADD);
        BeginBlendMode(BLEND_CUSTOM);
        DrawTextureRec(sample.texture,{0,0,float(WeaponCell),-float(WeaponCell)},
                       {float(cell%WeaponColumns*WeaponCell),float(cell/WeaponColumns*WeaponCell)},WHITE);
        EndBlendMode();
        EndTextureMode();
    }
    UnloadRenderTexture(sample);
    GenTextureMipmaps(&weaponAtlas.texture);
    SetTextureFilter(weaponAtlas.texture,TEXTURE_FILTER_TRILINEAR);
}
}
