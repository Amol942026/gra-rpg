#pragma once
#include <algorithm>
#include <array>
// Original layered Krita paint sheet; loaded at scene setup, never in draw().
namespace hero_paint {
inline Texture2D atlas{};
inline std::array<unsigned char,512*384> alpha{};
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
inline void load(){
 if(atlas.id)return;
 Image image=LoadImage("assets/sprites/hero-paints.png");
 if(!image.data||image.width!=512||image.height!=384){if(image.data)UnloadImage(image);return;}
 unifyHair(image);
 Color* pixels=LoadImageColors(image);for(size_t i=0;i<alpha.size();i++)alpha[i]=pixels[i].a;
 UnloadImageColors(pixels);atlas=LoadTextureFromImage(image);UnloadImage(image);
 if(atlas.id)SetTextureFilter(atlas,TEXTURE_FILTER_BILINEAR);
}
inline void unload(){if(atlas.id)UnloadTexture(atlas);atlas={};alpha.fill(0);}
inline bool opaque(Vector2 uv){
 int x=int(uv.x*512),y=int(uv.y*384);return x>=0&&x<512&&y>=0&&y<384&&alpha[y*512+x]>64;
}
inline Vector2 uv(int cell,float u,float v){return {(cell%4+u)/4.f,(cell/4+v)/3.f};}
}
