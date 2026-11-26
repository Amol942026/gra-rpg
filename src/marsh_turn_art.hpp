#pragma once
#include <cstdint>
#include <cstring>
#include "generated/marsh_turn_frames.hpp"

namespace marsh_art {
namespace clips=marsh_actor_frames;
namespace turn=marsh_turn_frames;
constexpr int Facings=128, PageCells=64, Poses=4*clips::Frames;
constexpr int Divisions=12, Side=Divisions+1, Vertices=Side*Side;
struct Offset { std::int16_t x,y; };
struct Marks { std::int16_t foot[2][3],light[2]; };
static_assert(sizeof(Offset)==4&&sizeof(Marks)==16);
constexpr int ViewCount=512;
struct ViewRect {
 std::uint16_t page,x,y,w,h,left,top;
};
static_assert(sizeof(ViewRect)==14);
struct PaintPage {
 Texture2D texture{};
 std::vector<unsigned char> alpha;
};
struct Painting {
 std::vector<PaintPage> pages;
 std::vector<ViewRect> views;
};
struct MotionAtlas {
 std::vector<Offset> offsets;
 std::vector<Marks> marks;
};
inline std::array<Painting,4> paintings{};
inline std::array<MotionAtlas,4> rigs{};
inline Shader bodyInk{};
inline int inkLimits=-1,inkMode=-1;
inline unsigned rigRevision=0;
static_assert(paintings.size()==BellKeeper-PeatToad+1);

inline bool ready(){
 if(!bodyInk.id)return false;
 for(int s=0;s<int(paintings.size());s++){
  if(rigs[s].offsets.empty()||rigs[s].marks.empty()||paintings[s].views.empty())return false;
  for(const auto& page:paintings[s].pages)if(!page.texture.id)return false;
 }
 return true;
}
inline void unloadActors(){
 if(bodyInk.id)UnloadShader(bodyInk);
 bodyInk={};
 for(auto& painting:paintings){
  for(auto& page:painting.pages)if(page.texture.id)UnloadTexture(page.texture);
  painting={};
 }
 for(auto& rig:rigs)rig={};
 ++rigRevision;
}
inline bool readViews(int s,Painting& painting,const char* path){
 int size=0;
 unsigned char* data=LoadFileData(path,&size);
 if(!data)return false;
 std::array<std::uint16_t,4> header{};
 bool valid=size==12+ViewCount*int(sizeof(ViewRect))&&!std::memcmp(data,"MVI1",4);
 if(valid){
  std::memcpy(header.data(),data+4,8);
  valid=header[0]==s&&header[1]==ViewCount&&header[2]>0&&header[2]<=ViewCount&&header[3]==0;
 }
 if(valid){
  painting.pages.resize(header[2]);
  painting.views.resize(ViewCount);
  std::memcpy(painting.views.data(),data+12,ViewCount*sizeof(ViewRect));
 }
 UnloadFileData(data);
 return valid;
}
inline bool loadPaintings(int s){
 char path[96];
 auto& painting=paintings[s];
 const int w=turn::CellW[s],h=turn::CellH[s];
 std::snprintf(path,sizeof(path),"assets/media/mokradla-obrot/views-%d.bin",s);
 const bool cropped=FileExists(path);
 if(cropped){
  if(!readViews(s,painting,path))return false;
 }else{
  // Earlier installations have 128 full-size, evenly spaced paintings.
  painting.pages.resize(Facings/PageCells);
  for(int b=0;b<Facings;b++)
   painting.views.push_back({std::uint16_t(b/PageCells),std::uint16_t(b%8*w),
      std::uint16_t(b%PageCells/8*h),std::uint16_t(w),std::uint16_t(h),0,0});
 }
 for(int i=0;i<int(painting.pages.size());i++){
  std::snprintf(path,sizeof(path),cropped?"assets/media/mokradla-obrot/views-%d-%d.png":
                                        "assets/media/mokradla-obrot/neutral-%d-%d.png",s,i);
  Image image=LoadImage(path);
  if(!image.data)return false;
  if(image.width<=0||image.height<=0||image.width>2048||image.height>2048||
     (!cropped&&(image.width!=w*8||image.height!=h*8))){UnloadImage(image);return false;}
  ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  auto& page=painting.pages[i];
  page.texture=LoadTextureFromImage(image);
  if(!page.texture.id){UnloadImage(image);return false;}
  SetTextureFilter(page.texture,TEXTURE_FILTER_BILINEAR);
  page.alpha.resize(image.width*image.height);
  const auto* color=static_cast<const Color*>(image.data);
  std::transform(color,color+page.alpha.size(),page.alpha.begin(),[](Color c){return c.a;});
  UnloadImage(image);
 }
 for(const auto& rect:painting.views){
  if(rect.page>=painting.pages.size()||rect.w<2||rect.h<2||rect.left+rect.w>w||rect.top+rect.h>h)return false;
  const auto& texture=painting.pages[rect.page].texture;
  if(rect.x+rect.w>texture.width||rect.y+rect.h>texture.height)return false;
 }
 return true;
}
inline bool loadSpecies(int s){
 char path[96];
 if(!loadPaintings(s))return false;
 // RGBA bytes carry two signed 16-bit vertex coordinates. This PNG is CPU
 // geometry, so its pixels are decoded without filtering, color space conversion or upload.
 std::snprintf(path,sizeof(path),"assets/media/mokradla-obrot/motion-%d.png",s);
 Image motion=LoadImage(path);
 if(!motion.data)return false;
 if(motion.width!=Facings*Side||motion.height!=Poses*Side||
    motion.format!=PIXELFORMAT_UNCOMPRESSED_R8G8B8A8){UnloadImage(motion);return false;}
 auto& rig=rigs[s];
 rig.offsets.resize(Facings*Poses*Vertices);
 const auto* bytes=static_cast<const unsigned char*>(motion.data);
 for(int b=0;b<Facings;b++)for(int k=0;k<Poses;k++)for(int y=0;y<Side;y++)
  std::memcpy(&rig.offsets[((b*Poses+k)*Side+y)*Side],
              bytes+((k*Side+y)*motion.width+b*Side)*4,Side*4);
 UnloadImage(motion);
 std::snprintf(path,sizeof(path),"assets/media/mokradla-obrot/support-%d.bin",s);
 int size=0;
 auto* file=LoadFileData(path,&size);
 if(!file)return false;
 const int yawBytes=Facings*Vertices*sizeof(Offset),markBytes=Facings*Poses*sizeof(Marks);
 bool valid=size==8+yawBytes+markBytes&&std::memcmp(file,"MRP2",4)==0&&
            file[4]==s&&file[5]==Side&&file[6]==Facings&&file[7]==Poses;
 if(valid){
  // The source rig also records a sparse angular derivative. The single-view renderer
  // only needs the sockets here.
  rig.marks.resize(Facings*Poses);
  std::memcpy(rig.marks.data(),file+8+yawBytes,markBytes);
 }
 UnloadFileData(file);return valid;
}
inline void loadActors(){
 if(ready())return;
 unloadActors();
 for(int s=0;s<int(paintings.size());s++)if(!loadSpecies(s)){unloadActors();return;}
 constexpr char pigment[]=R"GLSL(#version 330
 in vec2 fragTexCoord;
 in vec4 fragColor;
 uniform sampler2D texture0;
 uniform vec4 viewLimits;
 uniform int mode;
 out vec4 finalColor;
 void main(){
  // Each side of the cropped rectangle has a transparent gutter. Keep the
  // complete animation mesh while sampling only this one facing on the page.
  vec4 color=texture(texture0,clamp(fragTexCoord,viewLimits.xy,viewLimits.zw));
  if(mode==2){if(color.a<.62)discard;finalColor=vec4(fragColor.a);return;}
  finalColor=vec4(mode==1?fragColor.rgb:color.rgb*fragColor.rgb,fragColor.a*color.a);
 }
 )GLSL";
 bodyInk=LoadShaderFromMemory(nullptr,pigment);
 if(!bodyInk.id||bodyInk.id==rlGetShaderIdDefault()){bodyInk={};unloadActors();return;}
 inkLimits=GetShaderLocation(bodyInk,"viewLimits");
 inkMode=GetShaderLocation(bodyInk,"mode");
}
}
