#pragma once

namespace regional_enemy_art {
constexpr int Cell=144,Pad=2,Stride=Cell+2*Pad,Columns=8,Rows=6;
constexpr int IdleFrames=8,MoveFrames=12,AttackFrames=12,HitFrames=6,FallFrames=8;
constexpr int MoveStart=IdleFrames,AttackStart=MoveStart+MoveFrames,HitStart=AttackStart+AttackFrames,FallStart=HitStart+HitFrames;
constexpr int AnchorX=72,AnchorY=121;
static_assert(FallStart+FallFrames<=Columns*Rows);

struct Atlas {
 Texture2D art{},silhouette{};
 std::vector<unsigned char> alpha;
};
inline std::array<Atlas,RegionalEnemyTypeCount> atlases{};
inline Atlas sporekinBack{},cinderhornBack{};
inline std::array<Atlas,5> cinderhornWalk{};
inline std::array<Atlas,5> cinderhornRush{};
inline std::array<Atlas,2> cinderhornChargePose{};
inline constexpr int CinderhornWalkView[]={0,1,2,1,0,4,3,4};
inline constexpr const char* Names[]={"sporekin","dune-eye","cinderhorn"};
static_assert(cinderhorn_gait::Frames%Columns==0);

inline void loadAtlas(Atlas& atlas,const char* name,int rows=Rows){
  if(atlas.art.id)return;
  std::string path=std::string("assets/sprites/region-enemies/")+name+".png";
  Image image=LoadImage(path.c_str());
  if(!image.data)return;
  if(image.width==Columns*Stride&&image.height==rows*Stride){
   ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
   atlas.art=LoadTextureFromImage(image);
   auto* pixels=static_cast<Color*>(image.data);
   atlas.alpha.resize(image.width*image.height);
   for(size_t p=0;p<atlas.alpha.size();p++){
    atlas.alpha[p]=pixels[p].a;
    pixels[p]={255,255,255,pixels[p].a};
   }
   atlas.silhouette=LoadTextureFromImage(image);
   SetTextureFilter(atlas.art,TEXTURE_FILTER_BILINEAR);
   SetTextureFilter(atlas.silhouette,TEXTURE_FILTER_BILINEAR);
  }
  UnloadImage(image);
}

inline void load(){
 for(int i=0;i<RegionalEnemyTypeCount;i++)loadAtlas(atlases[i],Names[i]);
 loadAtlas(sporekinBack,"sporekin-back");
 loadAtlas(cinderhornBack,"cinderhorn-back");
 for(int i=0;i<int(cinderhornWalk.size());i++)
  loadAtlas(cinderhornWalk[i],TextFormat("cinderhorn-walk-%d",i),cinderhorn_gait::Frames/Columns);
 for(int i=0;i<int(cinderhornRush.size());i++)
  loadAtlas(cinderhornRush[i],TextFormat("cinderhorn-charge-%d",i),cinderhorn_gait::ChargeFrames/Columns);
 loadAtlas(cinderhornChargePose[0],"cinderhorn-charge-front",4);
 loadAtlas(cinderhornChargePose[1],"cinderhorn-charge-back",4);
}
inline void unloadAtlas(Atlas& atlas){
 if(atlas.art.id)UnloadTexture(atlas.art);
 if(atlas.silhouette.id)UnloadTexture(atlas.silhouette);
 atlas={};
}
inline void unload(){
 for(auto& atlas:atlases)unloadAtlas(atlas);
 unloadAtlas(sporekinBack);
 unloadAtlas(cinderhornBack);
 for(auto& atlas:cinderhornWalk)unloadAtlas(atlas);
 for(auto& atlas:cinderhornRush)unloadAtlas(atlas);
 for(auto& atlas:cinderhornChargePose)unloadAtlas(atlas);
}

inline bool northFacing(Direction direction){
 return direction==Direction::NW||direction==Direction::N||direction==Direction::NE;
}
inline bool mirrored(int type,Direction direction){
 if(type==SporekinType)return direction==Direction::E||direction==Direction::SE||direction==Direction::NE;
 return direction==Direction::SW||direction==Direction::W||direction==Direction::NW;
}
inline int frame(const Enemy& e){
 auto index=[](float progress,int count){return std::clamp(int(progress*count),0,count-1);};
 if(!e.alive)return FallStart+index(1-e.strike/RegionalEnemyFallTime,FallFrames);
 if(e.flash>0)return HitStart+index(1-e.flash/.18f,HitFrames);
 if(e.windup>0)return AttackStart+index(.66f*(1-e.windup/.42f),AttackFrames);
 if(e.strike>0)return AttackStart+index(.66f+.34f*(1-e.strike/.22f),AttackFrames);
 if(e.animation.moving)return MoveStart+index(e.animation.phase,MoveFrames);
 return index(std::fmod(e.animation.idleTime*.84f,1.f),IdleFrames);
}

struct SpriteFrame {const Atlas& atlas;int index;};
inline SpriteFrame spriteFrame(const Enemy& e){
 const auto direction=e.animation.direction;
 const int view=CinderhornWalkView[int(direction)];
 auto sample=[](float t,int frames){return std::clamp(int(t*frames),0,frames-1);};
 if(e.type==CinderhornType&&e.alive&&e.chargeBusy()){
  const auto& poses=cinderhornChargePose[northFacing(direction)];
  if(e.boarWindup>0&&poses.art.id)return {poses,sample(1-e.boarWindup/CinderhornCharge.windup,12)};
  const auto& rush=cinderhornRush[view];
  if(e.boarRush>0&&rush.art.id)return {rush,sample(e.animation.phase,cinderhorn_gait::ChargeFrames)};
  if(e.boarRecovery>0&&poses.art.id){
   const float t=1-e.boarRecovery/CinderhornCharge.recovery;
   return {poses,e.boarHit?20+sample(t,12):12+sample(t,8)};
  }
 }
 const Atlas* alternate=e.type==SporekinType?&sporekinBack:e.type==CinderhornType?&cinderhornBack:nullptr;
 const int pose=frame(e);
 const auto& walk=cinderhornWalk[view];
 const bool walking=e.type==CinderhornType&&pose>=MoveStart&&pose<AttackStart&&walk.art.id;
 if(walking)return {walk,sample(e.animation.phase,cinderhorn_gait::Frames)};
 return {alternate&&alternate->art.id&&northFacing(direction)?*alternate:atlases[e.type-SporekinType],pose};
}
inline bool draw(const Enemy& e,float x,float y,Color outline=BLANK,
                 const Vector2* probe=nullptr,bool shadow=false,float scale=1){
 const auto [atlas,n]=spriteFrame(e);
 if(!atlas.art.id)return false;
 const int sx=(n%Columns)*Stride+Pad,sy=(n/Columns)*Stride+Pad;
 const bool mirror=mirrored(e.type,e.animation.direction);
 if(e.type==CinderhornType)scale*=cinderhorn_gait::Scale;
 const float left=x-AnchorX*scale,top=y-AnchorY*scale;
 if(probe){
  int px=int(std::floor((probe->x-left)/scale)),py=int(std::floor((probe->y-top)/scale));
  if(px<0||px>=Cell||py<0||py>=Cell)return false;
  if(mirror)px=Cell-1-px;
  return atlas.alpha[(sy+py)*atlas.art.width+sx+px]>=110;
 }
 Rectangle source{float(sx),float(sy),mirror?-float(Cell):float(Cell),float(Cell)};
 Rectangle dest{left,top,Cell*scale,Cell*scale};
 if(!shadow&&outline.a&&atlas.silhouette.id){
  for(Vector2 shift:{Vector2{-1.5f,0},Vector2{1.5f,0},Vector2{0,-1.5f},Vector2{0,1.5f}}){
   DrawTexturePro(atlas.silhouette,source,{dest.x+shift.x,dest.y+shift.y,dest.width,dest.height},{},0,outline);
  }
 }
 DrawTexturePro(atlas.art,source,dest,{},0,WHITE);
 return false;
}
}
