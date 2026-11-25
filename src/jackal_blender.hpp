#pragma once

namespace jackal_blender {
// Matches the Blender export in tools/media/jackal_blender.py.
inline constexpr int Width=104,Height=120,Pad=2,AnchorX=52,AnchorY=98;
inline constexpr int Facings=48,Walk=24,Attack=15,Fall=12,Frames=Walk+1+Attack+2*Fall;
inline constexpr int Columns=16,Rows=16,PerPage=Columns*Rows/Frames;
inline constexpr int SheetWidth=Columns*(Width+Pad),SheetHeight=Rows*(Height+Pad);
static_assert(Facings%PerPage==0);
struct Page {
 Texture2D texture{};
 std::vector<unsigned char> alpha;
};
inline std::array<Page,Facings/PerPage> pages{};
inline Shader silhouette{};
namespace turn { inline void load();inline void unload(); }

inline bool ready(){return pages.back().texture.id!=0;}

inline void unload(){
 turn::unload();
 for(auto& page:pages){
  if(page.texture.id)UnloadTexture(page.texture);
  page={};
 }
 if(silhouette.id&&silhouette.id!=rlGetShaderIdDefault())UnloadShader(silhouette);
 silhouette={};
}

inline void load(){
 if(ready())return;
 for(int i=0;i<int(pages.size());++i){
  char path[96];
  std::snprintf(path,sizeof(path),"assets/desert/jackal-blender/jackal-%02d.png",i);
  if(!FileExists(path)){unload();return;}
  Image image=LoadImage(path);
  if(!image.data||image.width!=SheetWidth||image.height!=SheetHeight){
   if(image.data)UnloadImage(image);
   unload();return;
  }
  ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  auto& page=pages[i];
  page.texture=LoadTextureFromImage(image);
  const auto* pixels=static_cast<const Color*>(image.data);
  page.alpha.resize(SheetWidth*SheetHeight);
  std::transform(pixels,pixels+page.alpha.size(),page.alpha.begin(),[](Color pixel){return pixel.a;});
  UnloadImage(image);
  if(!page.texture.id){unload();return;}
  SetTextureFilter(page.texture,TEXTURE_FILTER_BILINEAR);
 }
 silhouette=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
out vec4 finalColor;
void main(){finalColor=vec4(fragColor.rgb,fragColor.a*texture(texture0,fragTexCoord).a);}
)GLSL");
 turn::load();
}

struct Frame {int facing,index;};
inline float facingCoordinate(const AnimationState& a){
 const float heading=a.facingInitialized?a.facingAngle:int(a.direction)*PI/4;
 const auto direction=actorFacing(heading);
 return std::atan2(direction.y,direction.x)*Facings/(2*PI);
}
inline Frame select(const Enemy& enemy,float fall){
 const auto& a=fall>=0&&enemy.deathMotion.facingInitialized?enemy.deathMotion:enemy.animation;
 const int facing=(int(std::lround(facingCoordinate(a)))+Facings)%Facings;
 const auto scrub=[](float t,int length){
  return int(std::lround(std::clamp(t,0.f,1.f)*(length-1)));
 };
 if(fall>=0)return {facing,Walk+1+Attack+(enemy.deathScreenSide()>=0?Fall:0)+scrub(fall,Fall)};
 if(const float attack=enemy.volcanoAction();attack>=0)return {facing,Walk+1+scrub(attack,Attack)};
 if((a.motionInitialized?a.moveBlend:float(a.moving))<.24f)return {facing,Walk};
 const float cycle=a.phase-std::floor(a.phase);
 return {facing,int(std::lround(cycle*Walk))%Walk};
}

inline Rectangle tile(Frame f){
 const int cell=(f.facing%PerPage)*Frames+f.index;
 return {float((cell%Columns)*(Width+Pad)+1),float((cell/Columns)*(Height+Pad)+1),Width,Height};
}

inline float coverage(const Page& page,Rectangle source,float x,float y){
 if(x<0||x>=Width||y<0||y>=Height)return 0;
 x+=source.x-.5f;y+=source.y-.5f;
 const int left=int(x),top=int(y);
 const auto i=top*SheetWidth+left;
 const float u=x-left,v=y-top;
 const auto& a=page.alpha;
 const float high=a[i]+(a[i+1]-a[i])*u;
 const float low=a[i+SheetWidth]+(a[i+SheetWidth+1]-a[i+SheetWidth])*u;
 return high+(low-high)*v;
}

#include "jackal_blender_turn.hpp"

inline bool draw(const Enemy& enemy,float x,float y,Color outline,const Vector2* probe,
                 bool shadow,float scale,float opacity,float fall){
 if(opacity<=0)return false;
 const auto frame=select(enemy,fall);
 const auto& page=pages[frame.facing/PerPage];
 const auto source=tile(frame);
 const Rectangle dest{x-AnchorX*scale,y-AnchorY*scale,Width*scale,Height*scale};
 if(!shadow&&frame.index<Walk&&turn::ready()){
  const auto pair=turn::between(enemy.animation,frame.index);
  if(pair.amount>.001f&&pair.amount<.999f){
   if(probe)return opacity>=.43f&&turn::cover(pair,{(probe->x-dest.x)/scale,(probe->y-dest.y)/scale})>=105;
   turn::draw(pair,dest,outline,opacity);
   return false;
  }
 }
 if(probe)return opacity>=.43f&&coverage(page,source,(probe->x-dest.x)/scale,(probe->y-dest.y)/scale)>=105;
 if(outline.a&&!shadow&&silhouette.id&&silhouette.id!=rlGetShaderIdDefault()){
  Color color=outline;color.a=static_cast<unsigned char>(outline.a*opacity);
  BeginShaderMode(silhouette);
  for(Vector2 offset:{Vector2{-1.3f,0},Vector2{1.3f,0},Vector2{0,-1.3f},Vector2{0,1.3f}})
   DrawTexturePro(page.texture,source,{dest.x+offset.x,dest.y+offset.y,dest.width,dest.height},{},0,color);
  EndShaderMode();
 }
 DrawTexturePro(page.texture,source,dest,{},0,Fade(WHITE,opacity));
 return false;
}
}
