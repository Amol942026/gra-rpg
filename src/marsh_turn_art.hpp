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
constexpr int ViewCount=512,WalkFrames=12,WalkMargin=12;
inline constexpr bool meshGait(int species){return species==0||species==3;}
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
inline std::array<Painting,4> walkingPaintings{};
inline std::array<std::vector<Marks>,4> walkingMarks{};
inline std::array<MotionAtlas,4> rigs{};
inline Shader bodyInk{};
inline int inkLimits=-1,inkMode=-1;
inline Texture2D bellInk{};
inline int inkBell=-1,inkBellPose=-1,inkOrigin=-1,inkCloakX=-1;
inline unsigned rigRevision=0;
static_assert(paintings.size()==BellKeeper-PeatToad+1);

inline bool ready(){
 if(!bodyInk.id)return false;
 for(int s=0;s<int(paintings.size());s++){
  if(rigs[s].offsets.empty()||rigs[s].marks.empty()||paintings[s].views.empty())return false;
  if(!meshGait(s)&&(walkingPaintings[s].views.size()!=Facings*WalkFrames||
                  walkingMarks[s].size()!=Facings*WalkFrames))return false;
  for(const auto& page:paintings[s].pages)if(!page.texture.id)return false;
  for(const auto& page:walkingPaintings[s].pages)if(!page.texture.id)return false;
 }
 return true;
}
inline void unloadActors(){
 if(bodyInk.id)UnloadShader(bodyInk);
 if(bellInk.id)UnloadTexture(bellInk);
 bodyInk={};
 bellInk={};
 for(auto* group:{&paintings,&walkingPaintings})for(auto& painting:*group){
  for(auto& page:painting.pages)if(page.texture.id)UnloadTexture(page.texture);
  painting={};
 }
 for(auto& marks:walkingMarks)marks.clear();
 for(auto& rig:rigs)rig={};
 ++rigRevision;
}
inline bool readViews(int s,Painting& painting,const char* path,int count){
 int size=0;
 unsigned char* data=LoadFileData(path,&size);
 if(!data)return false;
 std::array<std::uint16_t,4> header{};
 bool valid=size==12+count*int(sizeof(ViewRect))&&!std::memcmp(data,"MVI1",4);
 if(valid){
  std::memcpy(header.data(),data+4,8);
  valid=header[0]==s&&header[1]==count&&header[2]>0&&header[2]<=count&&header[3]==0;
 }
 if(valid){
  painting.pages.resize(header[2]);
  painting.views.resize(count);
  std::memcpy(painting.views.data(),data+12,count*sizeof(ViewRect));
 }
 UnloadFileData(data);
 return valid;
}
inline bool loadPaintings(int s,Painting& painting,bool walk){
 char path[96];
 const char* prefix=walk?"walk":"views";
 const int pad=walk?2*WalkMargin:0;
 const int w=turn::CellW[s]+pad,h=turn::CellH[s]+pad;
 std::snprintf(path,sizeof(path),"assets/media/mokradla-obrot/%s-%d.bin",prefix,s);
 const bool cropped=FileExists(path);
 if(cropped){
  if(!readViews(s,painting,path,walk?Facings*WalkFrames:ViewCount))return false;
 }else{
  if(walk)return false;
  // Earlier installations have 128 full-size, evenly spaced paintings.
  painting.pages.resize(Facings/PageCells);
  for(int b=0;b<Facings;b++)
   painting.views.push_back({std::uint16_t(b/PageCells),std::uint16_t(b%8*w),
      std::uint16_t(b%PageCells/8*h),std::uint16_t(w),std::uint16_t(h),0,0});
 }
 for(int i=0;i<int(painting.pages.size());i++){
  std::snprintf(path,sizeof(path),"assets/media/mokradla-obrot/%s-%d-%d.png",cropped?prefix:"neutral",s,i);
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
 if(!loadPaintings(s,paintings[s],false))return false;
 if(!meshGait(s)&&!loadPaintings(s,walkingPaintings[s],true))return false;
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
 UnloadFileData(file);
 if(meshGait(s))return valid;
 if(!valid)return false;
 std::snprintf(path,sizeof(path),"assets/media/mokradla-obrot/walk-support-%d.bin",s);
 file=LoadFileData(path,&size);
 if(!file)return false;
 std::uint16_t header[2]{};
 valid=size==8+Facings*WalkFrames*int(sizeof(Marks))&&!std::memcmp(file,"MWS1",4);
 if(valid){std::memcpy(header,file+4,4);valid=header[0]==s&&header[1]==Facings*WalkFrames;}
 if(valid){
  walkingMarks[s].resize(Facings*WalkFrames);
  std::memcpy(walkingMarks[s].data(),file+8,walkingMarks[s].size()*sizeof(Marks));
 }
 UnloadFileData(file);return valid;
}
inline void loadActors(){
 if(ready())return;
 unloadActors();
 for(int s=0;s<int(paintings.size());s++)if(!loadSpecies(s)){unloadActors();return;}
 const char* bellPath="assets/media/mokradla-obrot/keeper-bell.png";
 if(FileExists(bellPath)){
  bellInk=LoadTexture(bellPath);
  if(bellInk.id)SetTextureFilter(bellInk,TEXTURE_FILTER_BILINEAR);
 }
 constexpr char pigment[]=R"GLSL(#version 330
 in vec2 fragTexCoord;
 in vec4 fragColor;
 uniform sampler2D texture0, bellTexture;
 uniform vec4 viewLimits;
 uniform vec4 bellPose;
 uniform vec2 viewOrigin;
 uniform float cloakX;
 uniform int mode;
 out vec4 finalColor;
 void main(){
  // Each side of the cropped rectangle has a transparent gutter. Keep the
  // complete animation mesh while sampling only this one facing on the page.
  vec4 color=texture(texture0,clamp(fragTexCoord,viewLimits.xy,viewLimits.zw));
  if(mode==2){if(color.a<.62)discard;finalColor=vec4(fragColor.a);return;}
  if(mode==0&&abs(bellPose.w)>0.&&color.a>0.){
   vec2 cell=fragTexCoord*vec2(textureSize(texture0,0))-viewOrigin;
   vec2 p=cell-bellPose.xy;
   float c=cos(bellPose.z),s=sin(bellPose.z);
   // Inverse rigid rotation about the top hanger. The dress and its alpha
   // occlude the metal in this same draw, including when the turn wraps.
   p=mat2(c,-s,s,c)*p+vec2(18.,6.);
   vec2 size=vec2(textureSize(bellTexture,0));
   if(all(greaterThanEqual(p,vec2(0.)))&&all(lessThan(p,size))){
    float visible=abs(bellPose.w);
    // The cloak is in the actor's coordinates. Its edge does not spin or
    // swing with the coordinates used to sample the painted bell.
    float clipPlane=sign(bellPose.w)*(cell.x-cloakX)+27.*visible-13.;
    float exposed=mix(clamp(clipPlane+.5,0.,1.),1.,smoothstep(.72,1.,visible))*min(1.,visible*12.);
    vec4 metal=texture(bellTexture,p/size)*exposed;
    color.rgb=color.rgb*(1.-metal.a)+metal.rgb;
   }
  }
  finalColor=vec4(mode==1?fragColor.rgb:color.rgb*fragColor.rgb,fragColor.a*color.a);
 }
 )GLSL";
 bodyInk=LoadShaderFromMemory(nullptr,pigment);
 if(!bodyInk.id||bodyInk.id==rlGetShaderIdDefault()){bodyInk={};unloadActors();return;}
 inkLimits=GetShaderLocation(bodyInk,"viewLimits");
 inkMode=GetShaderLocation(bodyInk,"mode");
 inkBell=GetShaderLocation(bodyInk,"bellTexture");
 inkBellPose=GetShaderLocation(bodyInk,"bellPose");
 inkOrigin=GetShaderLocation(bodyInk,"viewOrigin");
 inkCloakX=GetShaderLocation(bodyInk,"cloakX");
}
}
