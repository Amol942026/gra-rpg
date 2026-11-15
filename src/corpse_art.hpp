#pragma once
#include "fallen_forest_art.hpp"
#include "enemy_death_settle.hpp"
#include "corpse_pose_mix.hpp"
namespace fallen_art {
constexpr int Width=192,Height=128,Density=2,OriginX=96,OriginY=80;
constexpr int RestWidth=256,RestHeight=176;
inline constexpr Vector2 RestOrigin{128,112};
constexpr int Directions=8,ThumbnailDensity=2,ThumbnailColumns=16;
constexpr int ThumbnailWidth=55*ThumbnailDensity,ThumbnailHeight=42*ThumbnailDensity;
inline RenderTexture2D thumbnails{};
inline void initThumbnails();
struct Sprite {Texture2D body{},edge{};std::vector<unsigned char> mask;shadow_geometry::Sprite ground;Rectangle footprint{};Vector2 origin{};};
inline std::array<std::array<Sprite,8>,LegacyEnemyTypeCount> sprites{};
inline constexpr std::array SidedTypes{1,2,3,BoarType,6,7,8,9,10,11};
inline constexpr int CachedSpriteCount=(LegacyEnemyTypeCount+SidedTypes.size())*Directions;
inline constexpr int ThumbnailCount=EnemyTypeCount*Directions*2;
inline std::array<std::array<bool,Directions>,EnemyTypeCount*2> thumbnailReady{};
inline std::array<std::array<Sprite,Directions>,SidedTypes.size()> otherFlank{};
inline int flankIndex(int type){
 const auto index=std::find(SidedTypes.begin(),SidedTypes.end(),type);
 return index==SidedTypes.end()?-1:int(index-SidedTypes.begin());
}
inline bool cachedBody(int type){return type<LegacyEnemyTypeCount;}
inline Sprite& spriteFor(int type,int direction,int side){
 const int index=side<0?flankIndex(type):-1;
 if(index>=0)return otherFlank[index][direction];
 return sprites[type][direction];
}
inline const Sprite& spriteFor(const Enemy& e){return spriteFor(e.type,int(e.animation.direction),e.deathSide());}
inline int prepared=0;
inline void storeSprite(Sprite& sprite,const Image& source,Vector2 anchor){
 if(!source.data)return;
 const auto opaque=GetImageAlphaBorder(source,0);
 if(opaque.width<=0||opaque.height<=0)return;
 constexpr float Margin=5;
 const float x0=std::max(0.f,opaque.x-Margin),y0=std::max(0.f,opaque.y-Margin);
 const float x1=std::min(float(source.width),opaque.x+opaque.width+Margin),y1=std::min(float(source.height),opaque.y+opaque.height+Margin);
 Image image=ImageFromImage(source,{x0,y0,x1-x0,y1-y0});
 sprite.origin={anchor.x-x0/Density,anchor.y-y0/Density};
 sprite.body=LoadTextureFromImage(image);SetTextureFilter(sprite.body,TEXTURE_FILTER_BILINEAR);
 Color* pixels=LoadImageColors(image);sprite.mask.resize(image.width*image.height);
 Image rim=GenImageColor(image.width,image.height,BLANK);
 int left=image.width,right=-1,top=image.height,bottom=-1;
 for(int y=0;y<image.height;y++)for(int x=0;x<image.width;x++){
  int n=y*image.width+x;sprite.mask[n]=pixels[n].a>=128;
  if(sprite.mask[n]){
   left=std::min(left,x);right=std::max(right,x);top=std::min(top,y);bottom=std::max(bottom,y);continue;
  }
  bool nearby=false;
  for(int dy=-3;dy<=3&&!nearby;dy++)for(int dx=-3;dx<=3&&!nearby;dx++)if(dx*dx+dy*dy<=9&&x+dx>=0&&x+dx<image.width&&y+dy>=0&&y+dy<image.height)nearby=pixels[(y+dy)*image.width+x+dx].a>=160;
  if(nearby)ImageDrawPixel(&rim,x,y,{255,255,255,220});
 }
 if(right>=left)sprite.footprint={float(left)/Density-sprite.origin.x,float(top)/Density-sprite.origin.y,
                                 float(right-left+1)/Density,float(bottom-top+1)/Density};
 sprite.edge=LoadTextureFromImage(rim);SetTextureFilter(sprite.edge,TEXTURE_FILTER_BILINEAR);UnloadImage(rim);UnloadImageColors(pixels);UnloadImage(image);prepared++;
}
inline void buildRestRig(actor_art::Rig& rig,int type,int side){
 if(type>=8)actor_art::buildDesertCorpse(rig,type,side);
 else actor_art::buildFallenForest(rig,type,side);
}
inline float restScale(int type){
 return type==3?GuardianBodyScale:type==7?treant_gait::Scale:1.f;
}
inline Image renderLegacyBody(RenderTexture2D target,int type,int direction,int side=1){
 BeginTextureMode(target);ClearBackground(BLANK);
 actor_art::Rig rig(RestOrigin.x*Density,RestOrigin.y*Density,restScale(type)*Density,Direction(direction));
 buildRestRig(rig,type,side);
 native_world::beginOpaqueFrame();
 rig.draw();
 native_world::finishOpaqueBackground();
 EndTextureMode();
 Image body=LoadImageFromTexture(target.texture);ImageFlipVertical(&body);return body;
}
inline void init(){
 if(prepared)return;
 const char* path="assets/generated/corpses-creatures-v48.png";
 Image atlas=FileExists(path)?LoadImage(path):Image{};
 const bool baked=atlas.data&&atlas.width==Width*Density*8&&atlas.height==Height*Density*LegacyEnemyTypeCount;
 auto target=LoadRenderTexture(RestWidth*Density,RestHeight*Density);
 auto prepare=[&](int type,int direction,int side,bool useBaked){
  const Rectangle cell{float(direction*Width*Density),float(type*Height*Density),float(Width*Density),float(Height*Density)};
  Image body=useBaked?ImageFromImage(atlas,cell):renderLegacyBody(target,type,direction,side);
  auto& sprite=spriteFor(type,direction,side);
  storeSprite(sprite,body,useBaked?Vector2{OriginX,OriginY}:RestOrigin);UnloadImage(body);
  actor_art::Rig mesh(0,0,restScale(type),Direction(direction),true);
  buildRestRig(mesh,type,side);
  sprite.ground=shadow_geometry::bake(mesh.shadows());
 };
 for(int type=0;type<LegacyEnemyTypeCount;type++)for(int direction=0;direction<Directions;direction++)
  prepare(type,direction,1,baked&&(type==0||type==1||type==BoarType||type==5));
 for(int type:SidedTypes)for(int direction=0;direction<Directions;direction++)prepare(type,direction,-1,false);
 UnloadRenderTexture(target);
 if(atlas.data)UnloadImage(atlas);
 initThumbnails();
 corpse_pose_mix::init();
}
inline void unload(){
 auto clear=[](Sprite& sprite){if(sprite.body.id)UnloadTexture(sprite.body);if(sprite.edge.id)UnloadTexture(sprite.edge);shadow_geometry::unload(sprite.ground);sprite={};};
 for(auto& species:sprites)for(auto& sprite:species)clear(sprite);
 for(auto& species:otherFlank)for(auto& sprite:species)clear(sprite);
 if(thumbnails.id)UnloadRenderTexture(thumbnails);
 corpse_pose_mix::unload();
 thumbnails={};thumbnailReady={};prepared=0;
}
}
inline bool restingCorpseArt(const Enemy& e,float x,float y,Color outline=BLANK,const Vector2* probe=nullptr,float opacity=1,bool shadow=false){
 if(!fallen_art::cachedBody(e.type)){
  if(regionalEnemy(e.type))return regional_enemy_art::rest(e,x,y,outline,probe,shadow,opacity);
  return volcano_actors::rest(e,x,y,outline,probe);
 }
 const auto& sprite=fallen_art::spriteFor(e);if(!sprite.body.id)return false;
 if(probe){
  int px=int(std::floor((probe->x-x+sprite.origin.x)*fallen_art::Density));
  int py=int(std::floor((probe->y-y+sprite.origin.y)*fallen_art::Density));
  return px>=0&&py>=0&&px<sprite.body.width&&py<sprite.body.height&&sprite.mask[py*sprite.body.width+px];
 }
 Rectangle source{0,0,float(sprite.body.width),float(sprite.body.height)},dest{x-sprite.origin.x,y-sprite.origin.y,source.width/fallen_art::Density,source.height/fallen_art::Density};
 if(shadow){DrawTexturePro(sprite.body,source,dest,{0,0},0,Fade(WHITE,opacity));return false;}
 if(outline.a)DrawTexturePro(sprite.edge,source,dest,{0,0},0,outline);
 const auto a=static_cast<unsigned char>(opacity*255+.5f);
 BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);DrawTexturePro(sprite.body,source,dest,{0,0},0,{a,a,a,a});native_world::endBlendMode();return false;
}

inline bool sharedFallenRig(int type){return type==1||type==BoarType;}
inline bool articulatedFallenRig(int type){return type>=6&&type<LegacyEnemyTypeCount;}
inline void buildDeathRig(actor_art::Rig& rig,const Enemy& e,float progress){
 if(sharedFallenRig(e.type))actor_art::fallen_quadrupeds::death(rig,e,progress);
 else if(e.type>=8)actor_art::buildDesertDeath(rig,e,progress);
 else actor_art::buildLegacyDeath(rig,e,progress);
}
inline bool landingCorpseArt(const Enemy& e,Vector2 p,Color outline,const Vector2* probe=nullptr,float opacity=1){
 const float spread=e.type==3?actor_art::guardianSpread(e.deathProgress()):1.f;
 if(spread>=1)return restingCorpseArt(e,p.x,p.y,outline,probe,opacity);
 actor_art::Rig fragments(p.x,p.y,GuardianBodyScale,e.animation.direction);
 actor_art::buildFallenGuardian(fragments,e.deathSide(),spread);
 if(probe)return fragments.contains(*probe);
 fragments.setVisibility(opacity);fragments.draw(outline);
 return false;
}
inline void drawLegacyHandoff(const Enemy& e,actor_art::Rig& rig,Vector2 anchor,float weight,Color outline){
 const Vector2 previous=rig.anchor();
 auto moving=[&](Vector2 p){rig.moveAnchor(p.x,p.y);rig.draw(outline);rig.moveAnchor(previous.x,previous.y);};
 if(weight<=0){moving(anchor);return;}
 auto settled=[&](Vector2 p){landingCorpseArt(e,p,outline);};
 if(corpse_pose_mix::draw(anchor,weight,moving,settled))return;
 // Keep a direct rendering path on systems without the scratch targets.
 landingCorpseArt(e,anchor,Fade(outline,weight),nullptr,weight*(2-weight));
 rig.setVisibility(1-weight);rig.moveAnchor(anchor.x,anchor.y);rig.draw(Fade(outline,1-weight));
 rig.moveAnchor(previous.x,previous.y);rig.setVisibility(1);
}

// One pose drives the color, the silhouette hit test and the ground shadow.
inline bool animatedLegacyCorpse(const Enemy& e,float x,float y,float progress,Color outline,const Vector2* probe,bool shadow=false,float shadowOpacity=1){
 const float corpse=enemy_death::restingBlend(e.type,progress);
 if(corpse>=1.f)return shadow?false:restingCorpseArt(e,x,y,outline,probe);
 if(probe&&corpse>=.45f&&landingCorpseArt(e,{x,y},BLANK,probe))return true;
 if(probe&&corpse>.55f)return false;
 using namespace actor_art;
 const int cacheKey=100+e.id;
 if(!shadow&&!probe)for(auto& entry:frameRigs)if(entry.first==cacheKey){
  drawLegacyHandoff(e,entry.second,{x,y},corpse,outline);return false;
 }
 Rig rig(x,y,fallen_art::restScale(e.type),e.animation.direction,shadow);
 if(shadow&&!recycledRigs.empty()){rig.reuseStorage(recycledRigs.back());recycledRigs.pop_back();}
 buildDeathRig(rig,e,progress);
 const auto& rest=fallen_art::spriteFor(e);
 Rectangle body=rest.footprint;
 body.x+=x;body.y+=y;
 Rectangle floor{};
 if(shadow){
  if(rest.ground.art.id){
   // Ground sprites have two logical pixels of guard padding on each side.
   floor={x-rest.ground.origin.x+2,y-rest.ground.origin.y+2,
          rest.ground.art.texture.width*.5f-4,rest.ground.art.texture.height*.5f-4};
  }
 }
 if(!articulatedFallenRig(e.type))rig.fitProjectedBounds(body,floor,enemy_death::landingFit(e.type,progress));
 if(shadow){rig.drawShadow(shadowOpacity);frameRigs.emplace_back(cacheKey,std::move(rig));return false;}
 if(probe)return rig.contains(*probe);
 drawLegacyHandoff(e,rig,{x,y},corpse,outline);return false;
}

inline Vector2 corpseDrawPoint(const Enemy& e,Vector2 anchor){
 const V impulse=normalized({(e.deathDirection.x-e.deathDirection.y)*2,e.deathDirection.x+e.deathDirection.y});
 const float travel=e.corpseGrounded()?1.f:enemy_death::window(e.deathProgress(),.03f,enemy_death::groundContact(e.type));
 const float slide=(enemyBoss(e.type)?9.f:7.f)*travel;
 return {anchor.x+impulse.x*slide,anchor.y+impulse.y*slide};
}

inline Rectangle cachedCorpseBounds(const Enemy& e,Vector2 anchor){
 auto bounds=fallen_art::spriteFor(e).footprint;
 const auto point=corpseDrawPoint(e,anchor);
 bounds.x+=point.x;bounds.y+=point.y;
 return bounds;
}

inline bool regionalCorpse(const Enemy& e,float x,float y,Color outline,const Vector2* probe,bool shadow=false,float opacity=1){
 if(e.corpseGrounded())return restingCorpseArt(e,x,y,outline,probe,opacity,shadow);
 const float progress=e.deathProgress();
 if(e.type!=CinderhornType||!regional_enemy_art::cinderhornRest.art.id)
  return regional_enemy_art::death(e,x,y,progress,outline,probe,shadow,opacity);
 const float weight=regional_enemy_art::cinderhornFallBlend(e,progress);
 if(weight>=1)return regional_enemy_art::cinderhornFallingRest(e,x,y,progress,outline,probe,shadow,opacity);
 if(weight<=0)return regional_enemy_art::death(e,x,y,progress,outline,probe,shadow,opacity);
 if(shadow){
  regional_enemy_art::cinderhornFallingRest(e,x,y,progress,BLANK,nullptr,true,opacity*enemy_death::ease(2*weight));
  return regional_enemy_art::death(e,x,y,progress,BLANK,nullptr,true,opacity*enemy_death::ease(2*(1-weight)));
 }
 if(probe&&weight>=.45f&&regional_enemy_art::cinderhornFallingRest(e,x,y,progress,BLANK,probe,false,1))return true;
 if(probe&&weight>.55f)return false;
 if(probe)return regional_enemy_art::death(e,x,y,progress,BLANK,probe);
 auto moving=[&](Vector2 p){regional_enemy_art::death(e,p.x,p.y,progress,outline);};
 auto settled=[&](Vector2 p){regional_enemy_art::cinderhornFallingRest(e,p.x,p.y,progress,outline,nullptr,false,1);};
 if(corpse_pose_mix::draw({x,y},weight,moving,settled))return false;
 regional_enemy_art::cinderhornFallingRest(e,x,y,progress,outline,nullptr,false,weight*(2-weight));
 return regional_enemy_art::death(e,x,y,progress,outline,nullptr,false,1-weight);
}

inline bool corpseArt(const Enemy& e,float x,float y,Color outline=BLANK,const Vector2* probe=nullptr){
 const auto p=corpseDrawPoint(e,{x,y});x=p.x;y=p.y;
 if(e.corpseGrounded())return restingCorpseArt(e,x,y,outline,probe);
 const float progress=e.deathProgress();
 if(regionalEnemy(e.type))return regionalCorpse(e,x,y,outline,probe);
 if(e.type>=VolcanoFirstType)return volcano_actors::death(e,x,y,progress,outline,probe);
 return animatedLegacyCorpse(e,x,y,progress,outline,probe);
}

namespace fallen_art {
inline int thumbnailType(int type,int side){return type+(side<0?EnemyTypeCount:0);}
inline Rectangle thumbnailCell(int type,int direction,int side=1){
 const int cell=thumbnailType(type,side)*Directions+direction;
 return {float(cell%ThumbnailColumns*ThumbnailWidth),float(cell/ThumbnailColumns*ThumbnailHeight),
         float(ThumbnailWidth),float(ThumbnailHeight)};
}
inline Rectangle maskBounds(const Sprite& sprite){
 const auto& area=sprite.footprint;
 if(area.width<=0||area.height<=0)return {};
 const float x=(area.x+sprite.origin.x)*Density,y=(area.y+sprite.origin.y)*Density;
 const float left=std::max(0.f,x-1),top=std::max(0.f,y-1);
 return {left,top,std::min(float(sprite.body.width),x+area.width*Density+1)-left,
                  std::min(float(sprite.body.height),y+area.height*Density+1)-top};
}
inline void initThumbnails(){
 constexpr int Rows=(ThumbnailCount+ThumbnailColumns-1)/ThumbnailColumns;
 constexpr int W=ThumbnailColumns*ThumbnailWidth,H=Rows*ThumbnailHeight;
 static_assert(W<=2048&&H<=2048);
 thumbnails=render_targets::colorOnly(W,H);
 if(!thumbnails.id)return;
 auto sample=render_targets::colorOnly(448,212);
 if(!sample.id){UnloadRenderTexture(thumbnails);thumbnails={};return;}
 SetTextureFilter(sample.texture,TEXTURE_FILTER_BILINEAR);
 SetTextureFilter(thumbnails.texture,TEXTURE_FILTER_BILINEAR);
 BeginTextureMode(thumbnails);ClearBackground(BLANK);EndTextureMode();
 for(int type=0;type<EnemyTypeCount;type++)for(int side:{1,-1})for(int direction=0;direction<Directions;direction++){
  Texture2D texture{};
  Rectangle source{};
  if(cachedBody(type)){
   const auto& sprite=spriteFor(type,direction,side);
   texture=sprite.body;
   if(texture.id)source=maskBounds(sprite);
  }else{
   Enemy body{};body.type=type;body.alive=false;body.animation.direction=Direction(direction);
   body.deathDirection=regionalEnemy(type)?flankImpulse(body.animation,side):normalized(V{1,-1})*float(side);
   BeginTextureMode(sample);ClearBackground(BLANK);
   native_world::beginOpaqueFrame();restingCorpseArt(body,224,139);native_world::finishOpaqueBackground();
   EndTextureMode();
   Image im=LoadImageFromTexture(sample.texture);
   if(im.data){
    ImageFlipVertical(&im);
    source=GetImageAlphaBorder(im,.11f);
    UnloadImage(im);
   }
   texture=sample.texture;
   source.y=texture.height-source.y-source.height;
   source.height=-source.height;
  }
  const float height=std::abs(source.height);
  if(!texture.id||source.width<=0||height<=0)continue;
  const float scale=std::min((ThumbnailWidth-4.f)/source.width,(ThumbnailHeight-4.f)/height);
  const auto cell=thumbnailCell(type,direction,side);
  const Rectangle dest{cell.x+(cell.width-source.width*scale)/2,cell.y+(cell.height-height*scale)/2,
                       source.width*scale,height*scale};
  BeginTextureMode(thumbnails);
  rlSetBlendFactors(RL_ONE,RL_ZERO,RL_FUNC_ADD);BeginBlendMode(BLEND_CUSTOM);
  DrawTexturePro(texture,source,dest,{},0,WHITE);
  EndBlendMode();EndTextureMode();
  thumbnailReady[thumbnailType(type,side)][direction]=true;
 }
 UnloadRenderTexture(sample);
}
}

inline void drawCorpseThumbnail(const Enemy& e,Rectangle card){
 const int type=e.type,direction=std::clamp(int(e.animation.direction),0,fallen_art::Directions-1);
 if(type<0||type>=EnemyTypeCount)return;
 int side=e.deathSide();
 if(!regionalEnemy(type)&&type>=VolcanoFirstType){
  side=e.deathScreenSide();if(!side)side=direction>3?-1:1;
 }
 if(!fallen_art::thumbnailReady[fallen_art::thumbnailType(type,side)][direction])return;
 const auto texture=fallen_art::thumbnails.texture;
 auto source=fallen_art::thumbnailCell(type,direction,side);
 source.y=texture.height-source.y-source.height;source.height=-source.height;
 const Rectangle interior{card.x+1,card.y+1,card.width-2,card.height-2};
 BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
 DrawTexturePro(texture,source,interior,{},0,WHITE);
 native_world::endBlendMode();
}
