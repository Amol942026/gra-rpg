#pragma once
namespace world_shadows {
inline Shader maskShader{},softShader{};
inline RenderTexture2D mask{},softMask{};
struct Uniforms {int texel=-1,strength=-1,ink=-1,receiver=-1,logicalSize=-1;};
inline Uniforms uniforms{};
inline void soften();
inline bool ready=false,chamberFrame=false;
inline int visibleCasters=0,preparedActors=0;
inline float maskScale=.8f;inline Vector2 maskOffset{};
inline void init(){
 if(maskShader.id)return;
 const char* maskCode=R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
out vec4 finalColor;
void main(){if(texture(texture0,fragTexCoord).a<0.62)discard;finalColor=vec4(fragColor.a);}
)GLSL";
 const char* softCode=R"GLSL(#version 330
in vec2 fragTexCoord;
uniform sampler2D texture0;
uniform vec2 texel;
uniform float strength;
uniform vec3 ink;
uniform vec4 receiver;
uniform vec2 logicalSize;
out vec4 finalColor;
void main(){
 // Four symmetric bilinear taps keep a soft penumbra with less bandwidth.
 vec2 d=texel*.76;
 float a=(texture(texture0,fragTexCoord+d).a+texture(texture0,fragTexCoord-d).a+
          texture(texture0,fragTexCoord+vec2(d.x,-d.y)).a+texture(texture0,fragTexCoord+vec2(-d.x,d.y)).a)*.25;
 if(receiver.z>0.0){
  vec2 point=vec2(fragTexCoord.x,1.0-fragTexCoord.y)*logicalSize;
  float edge=length((point-receiver.xy)/receiver.zw);
  a*=1.0-smoothstep(.988,1.0,edge);
 }
 float opacity=a*strength;
 finalColor=vec4(ink*opacity,opacity);
}
)GLSL";
 maskShader=LoadShaderFromMemory(nullptr,maskCode);softShader=LoadShaderFromMemory(nullptr,softCode);
 shadow_geometry::prepareBatch();
 uniforms={GetShaderLocation(softShader,"texel"),GetShaderLocation(softShader,"strength"),GetShaderLocation(softShader,"ink"),GetShaderLocation(softShader,"receiver"),GetShaderLocation(softShader,"logicalSize")};
}
inline void unload(){shadow_geometry::unloadBatch();if(mask.id)UnloadRenderTexture(mask);if(softMask.id)UnloadRenderTexture(softMask);softMask={};if(maskShader.id)UnloadShader(maskShader);if(softShader.id)UnloadShader(softShader);mask={};maskShader={};softShader={};uniforms={};ready=false;}
inline void quad(Texture2D texture,const std::array<Vector2,4>& p,const std::array<Vector2,4>& uv){
 const bool reverse=(p[1].x-p[0].x)*(p[2].y-p[0].y)-(p[1].y-p[0].y)*(p[2].x-p[0].x)>0;
 const int normal[]={0,1,2,3},flipped[]={0,3,2,1};const int* order=reverse?flipped:normal;
 rlSetTexture(texture.id);rlBegin(RL_QUADS);rlColor4ub(255,255,255,255);
 for(int i=0;i<4;i++){int k=order[i];rlTexCoord2f(uv[k].x,uv[k].y);rlVertex2f(p[k].x,p[k].y);}rlEnd();rlSetTexture(0);
}
inline Vector2 projected(Vector2 p,float y){auto q=shadow_math::billboard(p.x,p.y,y);return {q.x,q.y};}
inline void sprite(Texture2D texture,Rectangle bounds,Vector2 base,bool inverted=true){
 std::array<Vector2,4> p={projected({bounds.x,bounds.y},base.y),projected({bounds.x,bounds.y+bounds.height},base.y),
 projected({bounds.x+bounds.width,bounds.y+bounds.height},base.y),projected({bounds.x+bounds.width,bounds.y},base.y)};
 float top=inverted?1:0,bottom=1-top;
 quad(texture,p,{{{0,top},{0,bottom},{1,bottom},{1,top}}});
}
inline void prop(const Prop& value){
 Vector2 base=screen(value.pos);Texture2D texture=propArt[value.type].texture;visibleCasters++;
 if(value.type==13&&volcano_art::gateFrame().id){sprite(volcano_art::gateFrame(),{base.x-90*value.size,base.y-208*value.size,180*value.size,230*value.size},base,false);return;}
 if(volcanoSceneryProp(value.type)){Rectangle s=volcano_art::source(value);auto t=volcano_art::propTexture(value);float x=base.x-90*value.size,y=base.y-208*value.size,w=180*value.size,h=230*value.size;
  quad(t,{{projected({x,y},base.y),projected({x,y+h},base.y),projected({x+w,y+h},base.y),projected({x+w,y},base.y)}},{{{s.x/t.width,s.y/t.height},{s.x/t.width,(s.y+s.height)/t.height},{(s.x+s.width)/t.width,(s.y+s.height)/t.height},{(s.x+s.width)/t.width,s.y/t.height}}});return;}
 const float size=propArtSize(value);
 if(!tree_wind::moves(value.type)){sprite(texture,{base.x-90*size,base.y-208*size,180*size,230*size},base);return;}
 // Use the exact same connected wind strips as the crown artwork.
 const auto& wind=tree_wind::samples(value,game.time);
 for(int y=0;y<230;y+=5){
  float top=base.y+(y-208)*size,bottom=top+5*size;
  float left0=base.x+(-90+(tree_wind::cached?wind.bends[y/5]:treeBend(value,float(y),game.time)))*size;
  float left1=base.x+(-90+(tree_wind::cached?wind.bends[y/5+1]:treeBend(value,float(y+5),game.time)))*size;
  std::array<Vector2,4> p={projected({left0,top},base.y),projected({left1,bottom},base.y),
      projected({left1+180*size,bottom},base.y),projected({left0+180*size,top},base.y)};
  quad(texture,p,{{{0,1-y/230.f},{0,1-(y+5)/230.f},{1,1-(y+5)/230.f},{1,1-y/230.f}}});
 }
}
template<class Draw>inline void billboard(float anchorY,Draw draw){
 // The affine ground projection is also used for vector art such as Eldrin.
 float matrix[]={1,0,0,0,-shadow_math::LightX,-shadow_math::LightY,0,0,0,0,1,0,
                 shadow_math::LightX*anchorY,(1+shadow_math::LightY)*anchorY,0,1};
 rlPushMatrix();rlMultMatrixf(matrix);draw();rlPopMatrix();
}
inline void player(Vector2 p,float size=1,const AnimationState* special=nullptr){
 actor_art::shadowCacheKey=0;
 actor_art::drawPlayer(game,p.x,p.y,size,special);
 actor_art::shadowCacheKey=-1;visibleCasters++;
}
inline void mage(Vector2 p,float size,float time,float casting){
 billboard(p.y,[&]{intro_art::mage(p.x,p.y,size,time,casting);});visibleCasters++;
}
inline void furniture(int type,Vector2 base,float time,int floor){
 auto parts=tower_furniture::parts(type,time,floor);
 for(int i=0;i<parts.count;i++){
  const auto& part=parts.values[i];if(part.id==tower_furniture::Flame)continue; // A flame emits light.
  Vector2 at{base.x+part.shift.x,base.y+part.shift.y};
  const auto& ground=tower_furniture::groundShadows[part.id];
  if(ground.art.id)shadow_geometry::draw(ground,at);
  else{
   const auto& s=tower_furniture::sprites[part.id];
   sprite(s.art.texture,{at.x-s.ox*part.scale,at.y-s.oy*part.scale,s.w*.5f*part.scale,s.h*.5f*part.scale},base);
  }
 }visibleCasters++;
}
inline void arrow(const Arrow& shot){
 arrow_art::draw(shot,true);
}
inline bool near(Vector2 p,float extent=240){return p.x>-extent&&p.x<WorldW+extent&&p.y>-extent*.5f&&p.y<H+extent*.6f;}
inline void beginMaskBlend(){
 // Treat overlapping faces and casters as a coverage union, including during a fade.
 rlSetBlendFactors(RL_ONE,RL_ONE,RL_MAX);
 BeginBlendMode(BLEND_CUSTOM);
}
inline void legacyCorpseShadow(const Enemy& e,Vector2 p){
 const float progress=e.deathProgress(),weight=enemy_death::restingBlend(e.type,progress);
 const auto& ground=fallen_art::spriteFor(e).ground;
 // Overlap full-strength intervals so shared silhouette pixels keep their density.
 const float outgoing=ground.art.id?enemy_death::ease(2*(1-weight)):1.f;
 const float incoming=enemy_death::ease(2*weight);
 animatedLegacyCorpse(e,p.x,p.y,progress,BLANK,nullptr,true,outgoing);
 if(e.type==3&&incoming>0&&actor_art::guardianSpread(progress)<1){
  actor_art::Rig fragments(p.x,p.y,GuardianBodyScale,e.animation.direction,true);
  actor_art::buildFallenGuardian(fragments,e.deathSide(),actor_art::guardianSpread(progress));
  fragments.drawShadow(incoming);
 }else shadow_geometry::draw(ground,p,incoming);
}
inline void contactShadow(Vector2 p,Rectangle base,float coverage){
 if(coverage<=0)return;
 const Vector2 center{p.x+base.x+base.width*.5f,p.y+base.y+base.height*.5f};
 constexpr int Segments=24;
 std::array<Vector2,Segments+2> fan;
 fan[0]=center;
 for(int i=0;i<=Segments;i++){
  const float angle=i*(2*PI/Segments);
  fan[i+1]={center.x+std::cos(angle)*base.width*.5f,center.y+std::sin(angle)*base.height*.5f};
 }
 DrawTriangleFan(fan.data(),int(fan.size()),Fade(WHITE,coverage));
}
inline Rectangle paintedGroundFootprint(const Enemy& e){
 if(newDesertEnemy(e.type)){
  const bool jackal=e.type==DuneJackalType;
  const float half=jackal?28.f:37.f;
  const float offset=jackal?0.f:float(e.deathScreenSide())*15.f;
  return {offset-half,-7,2*half,13};
 }
 if(e.type==CinderhornType)return regional_enemy_art::cinderhornFootprint(e);
 if(e.type==SporekinType)return regional_enemy_art::sporekinFootprint(e);
 return {};
}
inline bool paintedCorpseShadow(const Enemy& e,Vector2 p){
 const auto base=paintedGroundFootprint(e);
 if(base.width<=0)return false;
 const float progress=e.deathProgress(),weight=e.corpseGrounded()?1.f:enemy_death::restingBlend(e.type,progress);
 if(weight<1){
  billboard(p.y,[&]{regionalCorpse(e,p.x,p.y,BLANK,nullptr,true,enemy_death::ease(2*(1-weight)));});
 }
 contactShadow(p,base,enemy_death::ease(2*weight));
 return true;
}
inline void calderaCorpseShadow(const Enemy& e,Vector2 p){
 const auto base=volcano_actors::groundFootprint(e);
 const float weight=base.width<=0?0.f:e.corpseGrounded()?1.f:enemy_death::restingBlend(e.type,e.deathProgress());
 if(weight<1){
  volcano_actors::death(e,p.x,p.y,e.deathProgress(),BLANK,nullptr,true,enemy_death::ease(2*(1-weight)));
  // Restore coverage before drawing the fading contact patch; it needs
  // the caster's opacity in the red channel, just like the next object.
  if(volcano_actors::turn_art::forType(e.type)&&volcano_actors::turn_art::filtered())BeginShaderMode(maskShader);
 }
 // The silhouette is painted from an elevated camera. Its full height must
 // not be cast as though the corpse were still standing over the terrain.
 contactShadow(p,base,enemy_death::ease(2*weight));
}
inline void prepare(bool chamber=false){
 tree_wind::beginFrame();
 init();ready=false;chamberFrame=chamber;visibleCasters=0;preparedActors=0;
 // A half-size mask saves fill and blur work. Its origin advances only on
 // whole mask pixels; the final quad compensates the remaining native pixel.
 // This keeps static shadow coverage invariant while the camera scrolls.
 maskScale=chamber?.8f:renderScale*.5f;maskOffset={};
 int width=int(std::ceil((chamber?W:WorldW)*double(maskScale)-.001)),height=int(std::ceil(H*double(maskScale)-.001));
 if(!chamber){
  Vector2 camera=cameraPixels();float x=std::fmod(camera.x,2.f),y=std::fmod(camera.y,2.f);if(x<0)x+=2;if(y<0)y+=2;
  maskOffset={(x+2)/renderScale,(y+2)/renderScale};width+=2;height+=2;
 }
 const bool raised=!chamber&&!game.inTower();
 if(raised){maskOffset.y+=terrain_height::MaxPixels;height+=int(std::ceil(terrain_height::MaxPixels*maskScale*2));}
 if(!mask.id||mask.texture.width!=width||mask.texture.height!=height){
  if(mask.id)UnloadRenderTexture(mask);mask=render_targets::colorOnly(width,height);SetTextureFilter(mask.texture,TEXTURE_FILTER_BILINEAR);
 }
 if(!mask.id||!IsShaderValid(maskShader)||!IsShaderValid(softShader))return;
 struct FlatScope{bool previous;FlatScope(bool active):previous(flatGroundProjection){flatGroundProjection=active;}~FlatScope(){flatGroundProjection=previous;}} flatScope(raised);
 actor_art::beginShadowFrame();shadow_geometry::beginBatch();BeginTextureMode(mask);ClearBackground(BLANK);BeginMode2D({{maskOffset.x*maskScale,maskOffset.y*maskScale},{0,0},0,maskScale});
 BeginShaderMode(maskShader);beginMaskBlend();rlDisableBackfaceCulling();
 if(chamber){
  float time=intro.chamber()?intro.time:towerStory.time;
  float charge=intro.chamber()?(intro.phase==IntroPhase::Charging?storyEase(intro.age/IntroSequence::ChargeTime):intro.phase==IntroPhase::Departing?1:0):0;
  AnimationState pose;pose.direction=Direction::E;pose.idleTime=time;actor_art::shadowElevation=charge*24;player({510,532-charge*24},2.5f,&pose);actor_art::shadowElevation=0;mage({930,505},2.7f,time,charge);
  for(float x:{242.f,1200.f})billboard(511,[&]{intro_art::pillar(x,181,330);});
  billboard(490,[&]{intro_art::candle(329,490,time);});billboard(494,[&]{intro_art::candle(1118,494,time+2);});
 }else if(game.inTower()){
  player(screen(game.player));
  const auto& furnishings=towerFurnishings(game.towerFloor);
  for(const auto& object:furnishings)furniture(object.type,screen(object.pos),game.time,game.towerFloor);
  for(int dir:{1,-1})if(game.stairPresent(dir)){shadow_geometry::draw(tower_stairs::groundShadows[tower_stairs::index(dir)],screen(towerStairBase(dir)));visibleCasters++;}
  if(game.towerFloor==0)mage(screen(MagePosition),1,game.time,towerStory.phase==TowerPhase::TravelOut?storyEase(towerStory.age/.9f):0);
 }else{
  for(const auto& p:game.props){
   Vector2 q=screen(p.pos);
   // Projected bounds include the crown, ground pivot and maximum wind bend.
   const float size=propArtSize(p);
   if(q.x>-260*size&&q.x<WorldW+115*size&&q.y>-70*size&&q.y<H+12*size)prop(p);
  }
  for(const auto& e:game.enemies){
   Vector2 p=screen(e.pos);
   if(!e.alive)p=corpseDrawPoint(e,p);
   if(!e.alive&&!e.corpseGrounded()&&e.type<LegacyEnemyTypeCount){
    if(near(p,270)){legacyCorpseShadow(e,p);visibleCasters++;}
    continue;
   }
   if(regionalEnemy(e.type)){
    if(near(p,215)){
     if(e.alive||!paintedCorpseShadow(e,p))
      billboard(p.y,[&]{if(e.alive)regional_enemy_art::draw(e,p.x,p.y,BLANK,nullptr,true);else regionalCorpse(e,p.x,p.y,BLANK,nullptr,true);});
     if(!e.alive)beginMaskBlend();
     visibleCasters++;
    }
    continue;
   }
   if(e.alive){
    float half=enemyBoss(e.type)?150:90,height=enemyInfo(e.type).visualHeight;
    bool casts=p.x>-(half+shadow_math::LightX*height)&&p.x<WorldW+half
           &&p.y>-(half*.55f+shadow_math::LightY*height)&&p.y<H+half*.55f;
    bool body=p.x>-140&&p.x<WorldW+140&&p.y>-20&&p.y<H+160;
    if(!casts&&!body)continue;
    actor_art::shadowCasting=casts;actor_art::shadowCacheKey=100+e.id;
    actor_art::drawEnemy(e,p.x,p.y);
    actor_art::shadowCacheKey=-1;actor_art::shadowCasting=true;
   }else if(e.type>=VolcanoFirstType){
    if(near(p,200))calderaCorpseShadow(e,p);
   }else{
    const auto& shadow=fallen_art::spriteFor(e).ground;
    const auto& tex=shadow.art.texture;
    if(p.x+tex.width*.5f-shadow.origin.x<0||p.x-shadow.origin.x>WorldW||p.y+tex.height*.5f-shadow.origin.y<0||p.y-shadow.origin.y>H)continue;
    shadow_geometry::draw(shadow,p);
   }
   // Directional cutouts merge their layers with a dedicated coverage shader.
   if(e.alive&&volcano_actors::turn_art::forType(e.type)&&volcano_actors::turn_art::filtered())BeginShaderMode(maskShader);
   visibleCasters++;
   if(e.alive&&e.type==3&&e.boulderWindup>0&&guardian_art::stone.id){
    auto socket=Game::boulderHand(e,e.boulderProgress());Vector2 p=screen(socket.ground);
    if(near(p,200))sprite(guardian_art::stone,{p.x-BoulderSpriteSize*.5f,p.y-socket.height-BoulderSpriteSize*.5f,BoulderSpriteSize,BoulderSpriteSize},p,false);
   }
  }
  if(game.hasFountains())for(int i=0;i<FountainCount;i++){Vector2 p=screen(game.fountains()[i]);if(near(p,200)){billboard(p.y,[&]{fountain_art::draw(i,false);});visibleCasters++;}}
  for(const auto& shot:game.arrows)if(near(screen(shot.pos),70)){arrow(shot);visibleCasters++;}
  for(const auto& b:game.boulders){Vector2 p=screen(b.pos);if(near(p,200)&&guardian_art::stone.id){sprite(guardian_art::stone,{p.x-BoulderSpriteSize*.5f,p.y-b.height-BoulderSpriteSize*.5f,BoulderSpriteSize,BoulderSpriteSize},p,false);visibleCasters++;}}
  for(const auto& impact:game.boulderImpacts)if(near(screen(impact.pos),180)&&guardian_art::fragments.id)for(int piece=0;piece<guardian_break::Pieces;piece++){
   Vector2 p=guardian_art::fragmentGround(impact,piece);billboard(p.y,[&]{guardian_art::fragment(impact,piece);});visibleCasters++;
  }
  if(!game.dead()||int(game.time*5)%2==0)player(screen(game.player));
  if(game.sandTrapped()){Vector2 p=screen(game.player);billboard(p.y,[&]{sand_prison::drawMound(p,false);sand_prison::drawMound(p,true);});}
  for(const auto& item:game.forage)if(!item.value.empty()&&near(screen(item.pos),40)&&(dragging.place!=Place::Forage||dragging.index!=item.id)){
   Vector2 p=screen(item.pos);billboard(p.y,[&]{icon(item.value,p.x,p.y-12,.68f);});visibleCasters++;
  }
 }
 for(const auto& item:game.ground)if(game.groundVisible(item)&&!item.value.empty()&&near(screen(item.pos),40)&&(dragging.place!=Place::Ground||dragging.index!=item.id)){
  Vector2 p=screen(item.pos);billboard(p.y,[&]{icon(item.value,p.x,p.y-14,.58f);});visibleCasters++;
 }
 shadow_geometry::flushBatch(maskShader);
 // A sprite-only batch may still be queued when there are no mesh casters.
 EndBlendMode();EndShaderMode();rlEnableBackfaceCulling();EndMode2D();EndTextureMode();
 preparedActors=int(actor_art::frameRigs.size());actor_art::endShadowPreparation();ready=true;soften();
}
inline void soften(){
 if(!ready)return;
 if(!softMask.id||softMask.texture.width!=mask.texture.width||softMask.texture.height!=mask.texture.height){
  if(softMask.id)UnloadRenderTexture(softMask);softMask=render_targets::colorOnly(mask.texture.width,mask.texture.height);SetTextureFilter(softMask.texture,TEXTURE_FILTER_BILINEAR);
 }
 BeginTextureMode(softMask);
 // Preserve the previous physical penumbra width at every display scale.
 float texel[]={1.f/((chamberFrame?W:WorldW)*.8f),1.f/(H*.8f)};
 float strength=chamberFrame?.32f:game.inDesert()?.34f:game.inTower()?.30f:.32f;
 float ink[]={.025f,.04f,.05f};
 SetShaderValue(softShader,uniforms.texel,texel,SHADER_UNIFORM_VEC2);
 SetShaderValue(softShader,uniforms.strength,&strength,SHADER_UNIFORM_FLOAT);
 SetShaderValue(softShader,uniforms.ink,ink,SHADER_UNIFORM_VEC3);
 float logicalSize[]={mask.texture.width/maskScale,mask.texture.height/maskScale};
 Vector2 floor=screen(TowerCenter);float receiver[]={floor.x+maskOffset.x,floor.y+maskOffset.y,game.inTower()?TowerRadius*56.5685f:0.f,TowerRadius*28.2843f};
 if(chamberFrame){receiver[0]=720;receiver[1]=460;receiver[2]=540;receiver[3]=148;}
 SetShaderValue(softShader,uniforms.receiver,receiver,SHADER_UNIFORM_VEC4);
 SetShaderValue(softShader,uniforms.logicalSize,logicalSize,SHADER_UNIFORM_VEC2);
 BeginShaderMode(softShader);
 // The shader emits premultiplied coverage and writes every pixel.
 rlSetBlendFactors(RL_ONE,RL_ZERO,RL_FUNC_ADD);
 BeginBlendMode(BLEND_CUSTOM);
 DrawTexturePro(mask.texture,{0,0,float(mask.texture.width),-float(mask.texture.height)},{0,0,float(softMask.texture.width),float(softMask.texture.height)},{0,0},0,WHITE);native_world::endBlendMode();EndShaderMode();EndTextureMode();
}
inline void draw(){
 if(!ready||!softMask.id)return;
 // Filtering happens once at mask resolution, not nine times per world pixel.
 BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
 if(!chamberFrame&&!game.inTower())terrain_surface::draw(softMask.texture,true,maskScale,maskOffset);
 else DrawTexturePro(softMask.texture,{0,0,float(softMask.texture.width),-float(softMask.texture.height)},
 {-maskOffset.x,-maskOffset.y,softMask.texture.width/maskScale,softMask.texture.height/maskScale},{0,0},0,WHITE);
 native_world::endBlendMode();
}
}
