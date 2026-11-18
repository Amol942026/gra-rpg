#pragma once
#include <algorithm>
#include <array>
#include "hero_head_turn.hpp"
// Original layered Krita paint sheet; loaded at scene setup, never in draw().
namespace hero_paint {
inline Texture2D atlas{};
inline constexpr int Width=512,BaseHeight=384,ArmorHeight=256,WeaponHeight=256;
inline constexpr int NewArmorTop=BaseHeight+ArmorHeight+WeaponHeight,NewArmorHeight=384;
inline constexpr int NewArmorFirstCell=NewArmorTop/128*4;
inline constexpr int RealmWeaponTop=NewArmorTop+NewArmorHeight,RealmWeaponHeight=384;
inline constexpr int RealmWeaponFirstCell=RealmWeaponTop/128*4;
inline constexpr int SpearTop=RealmWeaponTop+RealmWeaponHeight,SpearHeight=256;
inline constexpr int SpearFirstCell=SpearTop/128*4;
inline constexpr int HeadTop=SpearTop+SpearHeight,HeadViews=5,HeadCellWidth=Width/HeadViews;
inline constexpr int Height=HeadTop+128;
inline std::array<int,HeadViews> headMaskCells{};
inline constexpr int Steel=12,Hide=13,Stone=14,Scales=15,Bone=16,Cloth=17;
inline constexpr int Forged=20,Bronze=21,Wood=22,Binding=23,Obsidian=24,Ivory=25;
inline bool weaponsLoaded=false,realmWeaponsLoaded=false,spearsLoaded=false,headsLoaded=false,cuirassLoaded=false;
inline bool weaponCellAvailable(int cell){
 return cell>=0&&(cell>=SpearFirstCell?spearsLoaded:cell>=RealmWeaponFirstCell?realmWeaponsLoaded:weaponsLoaded);
}
inline std::array<unsigned char,Width*Height> alpha{};
inline constexpr Color HairColor{70,59,50,255};
// The imported strands contain a broad, baked highlight. Applying it over an
// already lit crown made a pale patch above the forehead. Keep fine strand
// contrast, but use the crown's brown and let the actor supply its lighting.
// This material conversion runs once when preloading, never during drawing.
inline void unifyHair(Image& image){
 ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
 auto* pixels=static_cast<Color*>(image.data);
 for(int cell:{1,2}){
  std::array<float,128*128> luminance{};
  int left=cell*128;
  for(int y=0;y<128;y++)for(int x=0;x<128;x++){
   Color c=pixels[y*512+left+x];
   luminance[y*128+x]=(54.f*c.r+183.f*c.g+19.f*c.b)/256.f;
  }
  for(int y=0;y<128;y++)for(int x=0;x<128;x++){
   Color& c=pixels[y*512+left+x];if(!c.a)continue;
   float sum=0,weight=0;
   for(int dy=-6;dy<=6;dy+=2)for(int dx=-6;dx<=6;dx+=2){
    int sx=std::clamp(x+dx,0,127),sy=std::clamp(y+dy,0,127);
    float a=pixels[sy*512+left+sx].a;
    sum+=luminance[sy*128+sx]*a;weight+=a;
   }
   float value=luminance[y*128+x],local=sum/std::max(1.f,weight);
   float tone=1.f+std::clamp((value-local)/80.f,-.14f,.10f)
                  +std::clamp((value-60.f)/1000.f,-.02f,.02f);
   c={static_cast<unsigned char>(HairColor.r*tone+.5f),
      static_cast<unsigned char>(HairColor.g*tone+.5f),
      static_cast<unsigned char>(HairColor.b*tone+.5f),c.a};
  }
 }
}
inline bool addSheet(Image& atlasImage,const char* path,int top,int height){
 if(!FileExists(path))return false;
 Image sheet=LoadImage(path);
 const bool valid=sheet.data&&sheet.width==Width&&sheet.height==height;
 if(valid)ImageDraw(&atlasImage,sheet,{0,0,Width,float(height)},{0,float(top),Width,float(height)},WHITE);
 if(sheet.data)UnloadImage(sheet);
 return valid;
}
inline bool addHeads(Image& target){
 const char* path="assets/sprites/hero-heads.png";
 if(!FileExists(path))return false;
 Image sheet=LoadImage(path);
 const bool valid=sheet.data&&sheet.width==HeadViews*128&&sheet.height==128;
 if(valid){
  // Reuse vacant equipment cells for the impact masks. Each mask uses
  // exactly the same filtered alpha as its painted view.
  auto* pixels=static_cast<Color*>(target.data);
  int masks=0;
  for(int cell=HeadTop/128*4-1;cell>=BaseHeight/128*4&&masks<HeadViews;cell--){
   const int x0=cell%4*128,y0=cell/4*128;
   bool empty=true;
   for(int y=0;y<128&&empty;y++)for(int x=0;x<128;x++)
    if(pixels[(y0+y)*Width+x0+x].a){empty=false;break;}
   if(empty)headMaskCells[masks++]=cell;
  }
  for(int i=masks;i<HeadViews;i++)headMaskCells[i]=-1;
  for(int i=0;i<HeadViews;i++){
   // Square display coordinates restore the authored aspect of each view.
   ImageDraw(&target,sheet,{float(i*128),0,128,128},
             {float(i*HeadCellWidth),float(HeadTop),float(HeadCellWidth),128},WHITE);
   const int cell=headMaskCells[i];
   if(cell<0)continue;
   for(int y=0;y<128;y++)for(int x=0;x<HeadCellWidth;x++){
    const auto a=pixels[(HeadTop+y)*Width+i*HeadCellWidth+x].a;
    pixels[(cell/4*128+y)*Width+cell%4*128+x]={255,255,255,a};
   }
  }
 }
 if(sheet.data)UnloadImage(sheet);
 return valid;
}
inline void load(){
 if(atlas.id)return;
 Image image=LoadImage("assets/sprites/hero-paints.png");
 if(!image.data||image.width!=Width||image.height!=BaseHeight){if(image.data)UnloadImage(image);return;}
 unifyHair(image);
 ImageResizeCanvas(&image,Width,Height,0,0,BLANK);
 addSheet(image,"assets/sprites/hero-armor-paints.png",BaseHeight,ArmorHeight);
 cuirassLoaded=GetImageColor(image,(18%4)*128+64,(18/4)*128+64).a>64;
 weaponsLoaded=addSheet(image,"assets/sprites/hero-weapon-paints.png",BaseHeight+ArmorHeight,WeaponHeight);
 addSheet(image,"assets/sprites/hero-realm-armor.png",NewArmorTop,NewArmorHeight);
 realmWeaponsLoaded=addSheet(image,"assets/sprites/hero-realm-weapons.png",RealmWeaponTop,RealmWeaponHeight);
 spearsLoaded=addSheet(image,"assets/sprites/hero-spear-paints.png",SpearTop,SpearHeight);
 headsLoaded=addHeads(image);
 Color* pixels=LoadImageColors(image);for(size_t i=0;i<alpha.size();i++)alpha[i]=pixels[i].a;
 UnloadImageColors(pixels);atlas=LoadTextureFromImage(image);UnloadImage(image);
 if(atlas.id){SetTextureFilter(atlas,TEXTURE_FILTER_BILINEAR);turn::load(Text);}
}
inline void unload(){turn::unload();if(atlas.id)UnloadTexture(atlas);atlas={};weaponsLoaded=realmWeaponsLoaded=spearsLoaded=headsLoaded=cuirassLoaded=false;alpha.fill(0);}
inline bool opaque(Vector2 uv){
 int x=int(uv.x*Width),y=int(uv.y*Height);return x>=0&&x<Width&&y>=0&&y<Height&&alpha[y*Width+x]>64;
}
inline Vector2 uv(int cell,float u,float v){
 constexpr float inset=.5f/128;
 return {(cell%4+std::clamp(u,inset,1-inset))/4.f,
         (cell/4+std::clamp(v,inset,1-inset))/(Height/128.f)};
}
inline Vector2 headUV(int facing,float u,float v,bool flash=false){
 const int mask=flash?headMaskCells[facing]:-1;
 const int left=mask>=0?mask%4*128:facing*HeadCellWidth,top=mask>=0?mask/4*128:HeadTop;
 return {(left+.5f+u*(HeadCellWidth-1))/Width,(top+.5f+v*127.f)/Height};
}
}
