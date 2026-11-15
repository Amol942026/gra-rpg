#pragma once
#include "enemy_death_settle.hpp"

namespace regional_enemy_art {
constexpr int Cell=144,Pad=2,Stride=Cell+2*Pad,Columns=8,Rows=6;
constexpr int IdleFrames=8,MoveFrames=12,AttackFrames=12,HitFrames=6,FallFrames=8;
constexpr int MoveStart=IdleFrames,AttackStart=MoveStart+MoveFrames,HitStart=AttackStart+AttackFrames,FallStart=HitStart+HitFrames;
constexpr int AnchorX=72,AnchorY=121;
static_assert(FallStart+FallFrames<=Columns*Rows);

struct Atlas {
 Texture2D art{},silhouette{};
 std::vector<unsigned char> alpha;
 std::array<Rectangle,FallFrames> fallBounds{};
 std::vector<Rectangle> cellExtents;
};
inline std::array<Atlas,RegionalEnemyTypeCount> atlases{};
inline Atlas sporekinBack{},cinderhornBack{};
constexpr int DuneEyeFallFrames=16;
inline Atlas duneEyeFall{};
inline std::array<Atlas,2> sporekinFalls{};
inline std::array<std::array<Rectangle,2>,2> sporekinRestBounds{};
inline std::array<std::array<Rectangle,2>,2> sporekinGroundPatches{};
inline Atlas cinderhornRest{};
inline std::array<Rectangle,16> cinderhornRestBounds{};
inline std::array<Rectangle,16> cinderhornGroundPatches{};
inline std::array<Atlas,5> cinderhornWalk{};
inline std::array<Atlas,5> cinderhornRush{};
inline std::array<Atlas,2> cinderhornChargePose{};
inline constexpr int CinderhornWalkView[]={0,1,2,1,0,4,3,4};
inline constexpr const char* Names[]={"sporekin","dune-eye","cinderhorn"};
static_assert(cinderhorn_gait::Frames%Columns==0);

inline Rectangle cellBounds(const Atlas& atlas,int frame){
 if(!atlas.art.id)return {};
 const int sx=frame%Columns*Stride+Pad,sy=frame/Columns*Stride+Pad;
 int left=Cell,top=Cell,right=-1,bottom=-1;
 for(int y=0;y<Cell;y++)for(int x=0;x<Cell;x++)if(atlas.alpha[(sy+y)*atlas.art.width+sx+x]>=110){
  left=std::min(left,x);top=std::min(top,y);right=std::max(right,x);bottom=std::max(bottom,y);
 }
 return right>=left?Rectangle{float(left),float(top),float(right-left+1),float(bottom-top+1)}:Rectangle{};
}

inline constexpr float SporekinGroundDrop=5;
inline Rectangle sporekinGroundPatch(const Atlas& atlas,int frame,Rectangle body){
 if(body.width<=0)return {};
 const int sx=frame%Columns*Stride+Pad,sy=frame/Columns*Stride+Pad;
 const int bottom=int(body.y+body.height);
 int left=Cell,right=-1;
 // Follow the lower silhouette, where the stem and the rim of the cap meet earth.
 for(int y=std::max(0,bottom-12);y<bottom;y++)for(int x=int(body.x);x<int(body.x+body.width);x++)
  if(atlas.alpha[(sy+y)*atlas.art.width+sx+x]>=155){left=std::min(left,x);right=std::max(right,x);}
 if(right<left)return {};
 return {float(left-2),bottom-10+SporekinGroundDrop,float(right-left+5),10};
}

inline Rectangle cinderhornGroundPatch(const Atlas& atlas,int frame){
 if(!atlas.art.id)return {};
 const int sx=frame%Columns*Stride+Pad,sy=frame/Columns*Stride+Pad;
 std::array<int,Cell> coverage{};
 int total=0;
 for(int y=AnchorY-18;y<=AnchorY+4;y++)for(int x=0;x<Cell;x++){
  if(atlas.alpha[(sy+y)*atlas.art.width+sx+x]<128)continue;
  ++coverage[x];++total;
 }
 if(!total)return {};
 // Ignore the ends of the horns and isolated hoof tips when locating the body's footprint.
 int left=0,right=Cell-1;
 int leftWeight=0,rightWeight=0;
 while(left<right&&leftWeight+coverage[left]<=total/8)leftWeight+=coverage[left++];
 while(right>left&&rightWeight+coverage[right]<=total/8)rightWeight+=coverage[right--];
 const float width=float(right-left+1);
 return {left+.06f*width,float(AnchorY-6),width*.88f,12};
}

inline void loadAtlas(Atlas& atlas,const char* name,int rows=Rows,bool calibrateAll=false){
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
   if(rows==Rows)for(int i=0;i<FallFrames;i++)atlas.fallBounds[i]=cellBounds(atlas,FallStart+i);
   if(calibrateAll){
    atlas.cellExtents.resize(Columns*rows);
    for(int i=0;i<Columns*rows;i++)atlas.cellExtents[i]=cellBounds(atlas,i);
   }
   atlas.silhouette=LoadTextureFromImage(image);
   SetTextureFilter(atlas.art,TEXTURE_FILTER_BILINEAR);
   SetTextureFilter(atlas.silhouette,TEXTURE_FILTER_BILINEAR);
  }
  UnloadImage(image);
}

inline void load(){
 loadAtlas(duneEyeFall,"dune-eye-fall",2*DuneEyeFallFrames/Columns);
 for(int i=0;i<RegionalEnemyTypeCount;i++)loadAtlas(atlases[i],Names[i],Rows,i==CinderhornType-SporekinType);
 loadAtlas(sporekinBack,"sporekin-back");
 loadAtlas(sporekinFalls[0],"sporekin-fall-front",2);
 loadAtlas(sporekinFalls[1],"sporekin-fall-back",2);
 for(int view=0;view<2;view++)for(int flank=0;flank<2;flank++){
  const int frame=flank*Columns+FallFrames-1;
  const auto bounds=cellBounds(sporekinFalls[view],frame);
  sporekinRestBounds[view][flank]=bounds;
  sporekinGroundPatches[view][flank]=sporekinGroundPatch(sporekinFalls[view],frame,bounds);
 }
 loadAtlas(cinderhornBack,"cinderhorn-back",Rows,true);
 loadAtlas(cinderhornRest,"cinderhorn-rest",2);
 for(int i=0;i<int(cinderhornRestBounds.size());i++){
  cinderhornRestBounds[i]=cellBounds(cinderhornRest,i);
  cinderhornGroundPatches[i]=cinderhornGroundPatch(cinderhornRest,i);
 }
 for(int i=0;i<int(cinderhornWalk.size());i++)
  loadAtlas(cinderhornWalk[i],TextFormat("cinderhorn-walk-%d",i),cinderhorn_gait::Frames/Columns,true);
 for(int i=0;i<int(cinderhornRush.size());i++)
  loadAtlas(cinderhornRush[i],TextFormat("cinderhorn-charge-%d",i),cinderhorn_gait::ChargeFrames/Columns,true);
 loadAtlas(cinderhornChargePose[0],"cinderhorn-charge-front",4,true);
 loadAtlas(cinderhornChargePose[1],"cinderhorn-charge-back",4,true);
}
inline void unloadAtlas(Atlas& atlas){
 if(atlas.art.id)UnloadTexture(atlas.art);
 if(atlas.silhouette.id)UnloadTexture(atlas.silhouette);
 atlas={};
}
inline void unload(){
 unloadAtlas(duneEyeFall);
 for(auto& atlas:atlases)unloadAtlas(atlas);
 unloadAtlas(sporekinBack);
 for(auto& atlas:sporekinFalls)unloadAtlas(atlas);
 sporekinRestBounds={};
 sporekinGroundPatches={};
 unloadAtlas(cinderhornBack);
 unloadAtlas(cinderhornRest);cinderhornRestBounds={};cinderhornGroundPatches={};
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
 if(!e.alive)return FallStart+FallFrames-1;
 if(e.flash>0&&!e.abilityBusy()&&e.windup<=0&&e.strike<=0)return HitStart+index(1-e.flash/EnemyHitRecovery,HitFrames);
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
inline int frameAlpha(SpriteFrame chosen,int type,Direction direction,Vector2 point,float scale,bool useFacingMirror=true){
 const auto [atlas,n]=chosen;
 if(!atlas.art.id)return 0;
 if(type==CinderhornType)scale*=cinderhorn_gait::Scale;
 int px=int(std::floor(point.x/scale+AnchorX)),py=int(std::floor(point.y/scale+AnchorY));
 if(px<0||px>=Cell||py<0||py>=Cell)return 0;
 if(useFacingMirror&&mirrored(type,direction))px=Cell-1-px;
 const int sx=(n%Columns)*Stride+Pad,sy=(n/Columns)*Stride+Pad;
 return atlas.alpha[(sy+py)*atlas.art.width+sx+px];
}
inline bool drawFrame(SpriteFrame chosen,int type,Direction direction,float x,float y,Color outline,
                      const Vector2* probe,bool shadow,float scale,float opacity=1,bool useFacingMirror=true){
 if(probe)return frameAlpha(chosen,type,direction,{probe->x-x,probe->y-y},scale,useFacingMirror)*opacity>=110;
 const auto [atlas,n]=chosen;
 if(!atlas.art.id)return false;
 const int sx=(n%Columns)*Stride+Pad,sy=(n/Columns)*Stride+Pad;
 const bool mirror=useFacingMirror&&mirrored(type,direction);
 if(type==CinderhornType)scale*=cinderhorn_gait::Scale;
 const float left=x-AnchorX*scale,top=y-AnchorY*scale;
 Rectangle source{float(sx),float(sy),mirror?-float(Cell):float(Cell),float(Cell)};
 Rectangle dest{left,top,Cell*scale,Cell*scale};
 if(!shadow&&outline.a&&atlas.silhouette.id){
  outline.a=static_cast<unsigned char>(outline.a*opacity);
  for(Vector2 shift:{Vector2{-1.5f,0},Vector2{1.5f,0},Vector2{0,-1.5f},Vector2{0,1.5f}}){
   DrawTexturePro(atlas.silhouette,source,{dest.x+shift.x,dest.y+shift.y,dest.width,dest.height},{},0,outline);
  }
 }
 DrawTexturePro(atlas.art,source,dest,{},0,Fade(WHITE,opacity));
 return false;
}
inline bool draw(const Enemy& e,float x,float y,Color outline=BLANK,
                 const Vector2* probe=nullptr,bool shadow=false,float scale=1){
 return drawFrame(spriteFrame(e),e.type,e.animation.direction,x,y,outline,probe,shadow,scale);
}

inline int paintedRestCell(const Enemy& e){return int(e.animation.direction)+(e.deathSide()<0?Columns:0);}
inline int duneEyeFallStart(const Enemy& e){
 const int side=e.deathScreenSide();
 return (side!=0&&((side<0)!=mirrored(e.type,e.animation.direction)))?DuneEyeFallFrames:0;
}
inline Rectangle worldBounds(Rectangle bounds,int type,Direction direction,bool useFacingMirror=true){
 if(useFacingMirror&&mirrored(type,direction))bounds.x=Cell-bounds.x-bounds.width;
 const float scale=type==CinderhornType?cinderhorn_gait::Scale:1.f;
 return {(bounds.x-AnchorX)*scale,(bounds.y-AnchorY)*scale,bounds.width*scale,bounds.height*scale};
}
inline int sporekinFallRow(const Enemy& e){
 // Choose the screen side of the impact before applying the normal facing mirror.
 const int side=e.deathScreenSide();
 return side!=0&&((side<0)!=mirrored(e.type,e.animation.direction))?1:0;
}
inline Rectangle paintedRestBounds(const Enemy& e){
 if(e.type==SporekinType){
  const int view=northFacing(e.animation.direction);
  auto bounds=sporekinRestBounds[view][sporekinFallRow(e)];
  bounds.y+=SporekinGroundDrop;
  return worldBounds(bounds,e.type,e.animation.direction);
 }
 if(e.type!=CinderhornType||!cinderhornRest.art.id)return {};
 return worldBounds(cinderhornRestBounds[paintedRestCell(e)],e.type,e.animation.direction,false);
}
inline Rectangle sporekinFootprint(const Enemy& e){
 return worldBounds(sporekinGroundPatches[northFacing(e.animation.direction)][sporekinFallRow(e)],
                    e.type,e.animation.direction);
}
inline Rectangle cinderhornFootprint(const Enemy& e){
 return worldBounds(cinderhornGroundPatches[paintedRestCell(e)],e.type,e.animation.direction,false);
}
inline bool rest(const Enemy& e,float x,float y,Color outline=BLANK,const Vector2* probe=nullptr,bool shadow=false,float opacity=1){
 if(e.type==DuneEyeType&&duneEyeFall.art.id)
  return drawFrame({duneEyeFall,duneEyeFallStart(e)+DuneEyeFallFrames-1},e.type,e.animation.direction,x,y,outline,probe,shadow,1,opacity);
 if(e.type==SporekinType){
  const auto& atlas=sporekinFalls[northFacing(e.animation.direction)];
  if(atlas.art.id)
   return drawFrame({atlas,sporekinFallRow(e)*Columns+FallFrames-1},e.type,e.animation.direction,
                    x,y+SporekinGroundDrop,outline,probe,shadow,1,opacity);
 }
 if(e.type==CinderhornType&&cinderhornRest.art.id)
  return drawFrame({cinderhornRest,paintedRestCell(e)},e.type,e.animation.direction,x,y,outline,probe,shadow,1,opacity,false);
 return drawFrame(spriteFrame(e),e.type,e.animation.direction,x,y,outline,probe,shadow,1,opacity);
}

inline SpriteFrame finalHitFrame(const Enemy& e){
 Enemy pose{};
 pose.type=e.type;
 pose.animation=e.deathMotion;
 pose.windup=e.deathWindup;
 pose.strike=e.deathStrike;
 pose.flash=e.deathAction<0?e.deathFlash:0;
 if(e.type==CinderhornType&&e.deathCharge>=0){
  using forest_boar_motion::Action;
  const auto state=forest_boar_motion::phase(e.deathCharge);
  const float remaining=std::max(.00001f,1-state.progress);
  if(state.action==Action::Windup)pose.boarWindup=CinderhornCharge.windup*remaining;
  if(state.action==Action::Rush)pose.boarRush=CinderhornCharge.duration*remaining;
  if(state.action==Action::Recovery)pose.boarRecovery=CinderhornCharge.recovery*remaining;
  pose.boarHit=e.deathChargeHit;
 }
 return spriteFrame(pose);
}

inline bool duneEyeDeath(const Enemy& e,float x,float y,float time,Color outline,
                         const Vector2* probe,bool shadow,float opacity){
 const float sample=std::clamp(time/.94f,0.f,1.f)*(DuneEyeFallFrames-1);
 const int i=int(sample),start=duneEyeFallStart(e);
 const float transition=enemy_death::window(time,0,.24f),next=sample-i;
 const SpriteFrame layers[]={finalHitFrame(e),{duneEyeFall,start+i},
                            {duneEyeFall,start+std::min(i+1,DuneEyeFallFrames-1)}};
 const float weights[]={1-transition,transition*(1-next),transition*next};
 float transmission=1;
 for(int lane=0;lane<3;lane++){
  const float weight=weights[lane];
  if(weight<=0)continue;
  if(probe)transmission*=1-weight*frameAlpha(layers[lane],e.type,e.animation.direction,{probe->x-x,probe->y-y},1)/255.f;
  else drawFrame(layers[lane],e.type,e.animation.direction,x,y,outline,nullptr,shadow,1,
                 opacity*(shadow?enemy_death::ease(2*weight):weight*(2-weight)));
 }
 return probe&&(1-transmission)*opacity>=110.f/255;
}

inline float cinderhornFallPhase(const Enemy& e,float progress){
 return std::clamp(progress/enemy_death::groundContact(e.type),0.f,1.f);
}
inline float cinderhornFallBlend(const Enemy& e,float progress){
 return enemy_death::window(cinderhornFallPhase(e,progress),.34f,.88f);
}
inline bool cinderhornFallLayer(const Enemy& e,float x,float y,float progress,Color outline,
                               const Vector2* probe,bool shadow,float opacity,bool settled){
 const float t=cinderhornFallPhase(e,progress);
 if(t>=1)return rest(e,x,y,outline,probe,shadow,opacity);
 const auto direction=e.animation.direction;
 const auto living=finalHitFrame(e);
 if(living.index<0||living.index>=int(living.atlas.cellExtents.size()))
  return rest(e,x,y,outline,probe,shadow,opacity);
 if(t<=0&&!settled)return drawFrame(living,e.type,direction,x,y,outline,probe,shadow,1,opacity);
 const auto from=worldBounds(living.atlas.cellExtents[living.index],e.type,direction);
 const auto to=paintedRestBounds(e);
 if(from.width<=0||to.width<=0)return rest(e,x,y,outline,probe,shadow,opacity);
 const auto resting=SpriteFrame{cinderhornRest,paintedRestCell(e)};
 const auto frame=settled?resting:living;
 const auto bounds=settled?to:from;
 const auto join=[](float a,float b,float weight){return a+(b-a)*weight;};
 const float roll=enemy_death::window(t,.21f,1);
 const float knees=enemy_death::window(t,.02f,.29f)*(1-roll);
 const int side=e.deathScreenSide();
 const float impact=side?float(side):(mirrored(e.type,direction)?-1.f:1.f);
 const float angle=impact*(settled?-28*(1-roll):46*roll-9*knees);
 const float radians=angle*DEG2RAD,c=std::cos(radians),s=std::sin(radians);
 const float w=join(from.width,to.width,roll)+5*knees;
 const float h=std::max(1.f,join(from.height,to.height,roll)-12*knees);
 const float sx=w/(bounds.width*c+bounds.height*std::abs(s));
 const float sy=h/(bounds.height*c+bounds.width*std::abs(s));
 const Vector2 origin{bounds.x+bounds.width*.5f,bounds.y+bounds.height*.5f};
 const float midX=x+join(from.x+from.width*.5f,to.x+to.width*.5f,roll)+impact*(2*knees+6*roll*(1-roll));
 // The near foreleg gives way while the lowest part of both layers follows
 // the same ground line; the horns swing down around the loaded shoulder.
 const float midY=y+join(from.y+from.height,to.y+to.height,roll)-h*.5f;
 if(probe){
  const float dx=(probe->x-midX)/sx,dy=(probe->y-midY)/sy;
  const Vector2 local{c*dx+s*dy+origin.x,c*dy-s*dx+origin.y};
  return frameAlpha(frame,e.type,direction,local,1,!settled)*opacity>=110;
 }
 rlPushMatrix();rlTranslatef(midX,midY,0);rlScalef(sx,sy,1);rlRotatef(angle,0,0,1);
 drawFrame(frame,e.type,direction,-origin.x,-origin.y,outline,nullptr,shadow,1,opacity,!settled);
 rlPopMatrix();
 return false;
}
inline bool cinderhornFallingRest(const Enemy& e,float x,float y,float progress,Color outline,
                                  const Vector2* probe,bool shadow,float opacity){
 return cinderhornFallLayer(e,x,y,progress,outline,probe,shadow,opacity,true);
}

struct FallStyle {float turn,rise,slide,squash,rebound,handoff;};
inline constexpr FallStyle Falls[]={
 // The cap and stem begin to buckle before rolling over onto the chosen flank.
 {-16,-4,3,.10f,4,.24f},
 // The eye pulls up and spins before the stone pieces lose their levitation.
 {28,17,-3,-.04f,4,.31f},
 // The forelegs buckle. A separate sideways tilt carries the shoulders into the fall.
 {-14,-4,5,.10f,0,.33f},
};
static_assert(std::size(Falls)==RegionalEnemyTypeCount);
struct FrameFit {float x=0,y=0,sx=1,sy=1;};
inline FrameFit fitFallFrame(SpriteFrame frame,int type,Direction direction,float weight){
 if(weight<=0||frame.index<FallStart||frame.index>=FallStart+FallFrames)return {};
 auto from=frame.atlas.fallBounds[frame.index-FallStart],to=frame.atlas.fallBounds.back();
 if(from.width<=0||from.height<=0||to.width<=0||to.height<=0)return {};
 if(mirrored(type,direction)){from.x=Cell-from.x-from.width;to.x=Cell-to.x-to.width;}
 const float scale=type==CinderhornType?cinderhorn_gait::Scale:1.f;
 const float sx=1+weight*(std::clamp(to.width/from.width,.54f,1.8f)-1);
 const float sy=1+weight*(std::clamp(to.height/from.height,.54f,1.8f)-1);
 const float oldX=from.x+from.width*.5f-AnchorX,oldY=from.y+from.height*.5f-AnchorY;
 const float dx=to.x+to.width*.5f-AnchorX-oldX,dy=to.y+to.height*.5f-AnchorY-oldY;
 return {scale*(weight*dx+(1-sx)*oldX),scale*(weight*dy+(1-sy)*oldY),sx,sy};
}
inline bool death(const Enemy& e,float x,float y,float progress,Color outline=BLANK,
                  const Vector2* probe=nullptr,bool shadow=false,float opacity=1,const Rectangle* target=nullptr){
 if(e.type==DuneEyeType&&duneEyeFall.art.id)
  return duneEyeDeath(e,x,y,progress/enemy_death::groundContact(e.type),outline,probe,shadow,opacity);
 if(e.type==CinderhornType&&cinderhornRest.art.id)
  return cinderhornFallLayer(e,x,y,progress,outline,probe,shadow,opacity,false);
 const auto originalRest=spriteFrame(e);
 const auto direction=e.animation.direction;
 if(progress>=enemy_death::groundContact(e.type))return rest(e,x,y,outline,probe,shadow,opacity);
 const auto& sporekinFall=sporekinFalls[northFacing(direction)];
 const bool layeredFall=e.type==SporekinType&&sporekinFall.art.id;
 const auto key=enemy_death::sample(e.type,progress);
 const auto& style=Falls[e.type-SporekinType];
 const float raw=std::min(1.f,progress/style.handoff);
 const float handoff=raw*raw*(3-2*raw);
 const int impactSide=e.deathScreenSide();
 const float sign=impactSide?float(impactSide):(mirrored(e.type,direction)?-1.f:1.f);
 const auto heading=actorFacing(int(direction)*AnimationPi/4);
 const float trip=e.type==CinderhornType?enemy_death::window(progress,.09f,.47f)*(1-enemy_death::landingFit(e.type,progress)):0;
 const float angle=sign*(std::abs(style.turn)*key.recoil-style.rebound*key.settle+19*trip);
 const float grounding=layeredFall?SporekinGroundDrop*enemy_death::ease(key.collapse):0;
 const float drop=-style.rise*key.recoil+style.rebound*key.settle+13*trip+grounding;
 const float scaleY=1-style.squash*key.recoil+key.settle*.045f;
 const float dx=heading.x*style.slide*key.recoil+sign*8*trip,dy=heading.y*style.slide*.5f*key.recoil+drop;
 const float offset=key.collapse*(FallFrames-1);
 const int clipFrame=std::min(FallFrames-1,int(offset));
 const float split=offset-clipFrame;
 const auto start=handoff<1?finalHitFrame(e):originalRest;
 const auto& fallAtlas=layeredFall?sporekinFall:originalRest.atlas;
 const int fallStart=layeredFall?sporekinFallRow(e)*Columns:FallStart;
 const SpriteFrame frames[]={start,{fallAtlas,fallStart+clipFrame},{fallAtlas,fallStart+std::min(FallFrames-1,clipFrame+1)}};
 const float rawWeights[]={1-handoff,handoff*(1-split),handoff*split};
 const float weights[]={opacity*(shadow?enemy_death::ease(2*rawWeights[0]):rawWeights[0]),
                        opacity*(shadow?enemy_death::ease(2*rawWeights[1]):rawWeights[1]),
                        opacity*(shadow?enemy_death::ease(2*rawWeights[2]):rawWeights[2]*(2-rawWeights[2]))};
 // The new fall is painted on the resting sprite's own anchor in each frame.
 const float fit=layeredFall?0:enemy_death::landingFit(e.type,progress);
 const FrameFit fits[]={{},fitFallFrame(frames[1],e.type,direction,fit),fitFallFrame(frames[2],e.type,direction,fit)};
 FrameFit align{};
 if(target&&fit>0){
  const auto from=worldBounds(originalRest.atlas.fallBounds.back(),e.type,direction);
  if(from.width>0&&from.height>0){
   align.sx=1+fit*(std::clamp(target->width/from.width,.50f,2.4f)-1);
   align.sy=1+fit*(std::clamp(target->height/from.height,.50f,2.4f)-1);
   align.x=(from.x+from.width*.5f)*(1-align.sx)+fit*(target->x+target->width*.5f-from.x-from.width*.5f);
   align.y=(from.y+from.height*.5f)*(1-align.sy)+fit*(target->y+target->height*.5f-from.y-from.height*.5f);
  }
 }
 if(probe){
  const float u=(probe->x-x-align.x)/align.sx-dx,v=((probe->y-y-align.y)/align.sy-dy)/scaleY;
  const float rot=angle*DEG2RAD,c=std::cos(rot),s=std::sin(rot);
  const Vector2 local{u*c+v*s,v*c-u*s};
  float transmission=1;
  for(int i=0;i<3;i++)if(weights[i]>0){
   const auto& match=fits[i];
   const Vector2 point{(local.x-match.x)/match.sx,(local.y-match.y)/match.sy};
   transmission*=1-weights[i]*frameAlpha(frames[i],e.type,direction,point,1)/255.f;
  }
  return transmission<=1-110.f/255;
 }
 rlPushMatrix();
 rlTranslatef(x+align.x,y+align.y,0);rlScalef(align.sx,align.sy,1);
 rlTranslatef(dx,dy,0);rlScalef(1,scaleY,1);rlRotatef(angle,0,0,1);
 for(int i=0;i<3;i++)if(weights[i]>0){
  const auto& match=fits[i];
  rlPushMatrix();rlTranslatef(match.x,match.y,0);rlScalef(match.sx,match.sy,1);
  drawFrame(frames[i],e.type,direction,0,0,outline,nullptr,shadow,1,weights[i]);
  rlPopMatrix();
 }
 rlPopMatrix();
 return false;
}
}
