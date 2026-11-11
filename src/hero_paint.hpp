#pragma once
#include <algorithm>
#include <array>
// Original layered Krita paint sheet; loaded at scene setup, never in draw().
namespace hero_paint {
inline Texture2D atlas{};
inline constexpr int Width=512,BaseHeight=384,ArmorHeight=256,WeaponHeight=256;
inline constexpr int NewArmorTop=BaseHeight+ArmorHeight+WeaponHeight,NewArmorHeight=384;
inline constexpr int NewArmorFirstCell=NewArmorTop/128*4;
inline constexpr int RealmWeaponTop=NewArmorTop+NewArmorHeight,RealmWeaponHeight=384;
inline constexpr int RealmWeaponFirstCell=RealmWeaponTop/128*4;
inline constexpr int SpearTop=RealmWeaponTop+RealmWeaponHeight,SpearHeight=256;
inline constexpr int Height=SpearTop+SpearHeight,SpearFirstCell=SpearTop/128*4;
inline constexpr int Steel=12,Hide=13,Stone=14,Scales=15,Bone=16,Cloth=17;
inline constexpr int Forged=20,Bronze=21,Wood=22,Binding=23,Obsidian=24,Ivory=25;
inline bool weaponsLoaded=false,realmWeaponsLoaded=false,spearsLoaded=false;
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
inline void load(){
 if(atlas.id)return;
 Image image=LoadImage("assets/sprites/hero-paints.png");
 if(!image.data||image.width!=Width||image.height!=BaseHeight){if(image.data)UnloadImage(image);return;}
 unifyHair(image);
 ImageResizeCanvas(&image,Width,Height,0,0,BLANK);
 addSheet(image,"assets/sprites/hero-armor-paints.png",BaseHeight,ArmorHeight);
 weaponsLoaded=addSheet(image,"assets/sprites/hero-weapon-paints.png",BaseHeight+ArmorHeight,WeaponHeight);
 addSheet(image,"assets/sprites/hero-realm-armor.png",NewArmorTop,NewArmorHeight);
 realmWeaponsLoaded=addSheet(image,"assets/sprites/hero-realm-weapons.png",RealmWeaponTop,RealmWeaponHeight);
 spearsLoaded=addSheet(image,"assets/sprites/hero-spear-paints.png",SpearTop,SpearHeight);
 Color* pixels=LoadImageColors(image);for(size_t i=0;i<alpha.size();i++)alpha[i]=pixels[i].a;
 UnloadImageColors(pixels);atlas=LoadTextureFromImage(image);UnloadImage(image);
 if(atlas.id)SetTextureFilter(atlas,TEXTURE_FILTER_BILINEAR);
}
inline void unload(){if(atlas.id)UnloadTexture(atlas);atlas={};weaponsLoaded=realmWeaponsLoaded=spearsLoaded=false;alpha.fill(0);}
inline bool opaque(Vector2 uv){
 int x=int(uv.x*Width),y=int(uv.y*Height);return x>=0&&x<Width&&y>=0&&y<Height&&alpha[y*Width+x]>64;
}
inline Vector2 uv(int cell,float u,float v){
 constexpr float inset=.5f/128;
 return {(cell%4+std::clamp(u,inset,1-inset))/4.f,
         (cell/4+std::clamp(v,inset,1-inset))/(Height/128.f)};
}
}
