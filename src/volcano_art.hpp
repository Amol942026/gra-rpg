#pragma once
namespace volcano_art {
inline Texture2D tiles{},scenery{},sceneryEdge{},sky{},lava{},minimap{},macro{},road{},bank{},bridge{},steam{},channelWall{};inline RenderTexture2D terrain{};
inline Shader bridgeBlend{};
inline Texture2D extraScenery{},extraSceneryEdge{};inline std::vector<unsigned char> extraSceneryAlpha;
inline Texture2D abyssCliff{},abyssDistance{};
inline constexpr int FlowFrames=60;inline constexpr float FlowFPS=30;
inline std::array<Texture2D,60> lavaFrames{};
inline std::vector<unsigned char> sceneryAlpha;
inline Texture2D portalFrame{},portalLoop{},portalOutline{};inline std::vector<unsigned char> portalMask;
inline int portalSample(float time){return int(std::floor(std::fmod(std::max(0.f,time)*60+.0001f,120.f)));}
inline void drawPortal(const Prop& p,bool outline,Color tint){
 Vector2 at=screen(p.pos);Rectangle bounds{at.x-90*p.size,at.y-208*p.size,180*p.size,230*p.size};
 if(outline){DrawTexturePro(portalOutline,{0,0,360,460},bounds,{0,0},0,tint);return;}
 int f=portalSample(game.time);DrawTexturePro(portalLoop,{float(f%15*132+2),float(f/15*212+2),128,208},{at.x-55*p.size,at.y-190*p.size,110*p.size,195*p.size},{0,0},0,tint);
 DrawTexturePro(portalFrame,{0,0,360,460},bounds,{0,0},0,tint);
}
inline bool portalContains(const Prop& p,Vector2 point){Vector2 at=screen(p.pos);int x=int(std::floor(((point.x-at.x)/p.size+90)*2)),y=int(std::floor(((point.y-at.y)/p.size+208)*2));return x>=0&&x<360&&y>=0&&y<460&&!portalMask.empty()&&portalMask[y*360+x];}
struct Strip{std::array<V,4> corners;float from,to;};inline std::vector<Strip> lavaStrips;
struct CanyonFace{std::array<V,4> corners;std::array<float,4> depth;float from,to;};inline std::vector<CanyonFace> canyonFronts;
inline Vector2 projected(V p){return {TerrainOriginX+(p.x-p.y)*40,90+(p.x+p.y)*20};}
inline void textured(Texture2D texture,const std::array<Vector2,4>& p,float u0=0,float v0=0,float u1=1,float v1=1){
 rlSetTexture(texture.id);rlBegin(RL_QUADS);rlColor4ub(255,255,255,255);
 const Vector2 uv[]={{u0,v0},{u0,v1},{u1,v1},{u1,v0}};
 // Isometric ribbons can run in either direction; keep front-face winding.
 bool flip=(p[1].x-p[0].x)*(p[2].y-p[0].y)-(p[1].y-p[0].y)*(p[2].x-p[0].x)>0;
 const int normal[]={0,1,2,3},reverse[]={0,3,2,1};for(int k=0;k<4;k++){int i=flip?reverse[k]:normal[k];rlTexCoord2f(uv[i].x,uv[i].y);rlVertex2f(p[i].x,p[i].y);}rlEnd();rlSetTexture(0);
}
inline void groundQuad(Texture2D texture,const std::array<V,4>& points,float u0=0,float v0=0,float u1=1,float v1=1){std::array<Vector2,4> p;for(int i=0;i<4;i++)p[i]=projected(points[i]);textured(texture,p,u0,v0,u1,v1);}
inline std::array<V,4> bridgeDeck(int index){
 const float y=VolcanoBridges[index].y;
 const V center{lavaCenter(VolcanoBridgeRivers[index],y),y};
 const float span=VolcanoBridgeHalfSpan,depth=VolcanoBridgeHalfDepth;
 return {center+V{-span,-depth},center+V{-span,depth},center+V{span,depth},center+V{span,-depth}};
}
inline void initBridgeBlend(){
 bridgeBlend=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
uniform sampler2D texture0;
out vec4 finalColor;
void main(){
 vec2 uv=fragTexCoord;
 float end=abs(uv.x-1.0);
 float side=abs(uv.y*2.0-1.0);
 float grit=.045*sin(uv.y*24.0+uv.x*17.0)+.028*sin(uv.y*61.0-uv.x*47.0);
 float fade=smoothstep(.48,.97,end+grit+side*side*.11);
 float width=mix(1.0,.74,smoothstep(.46,.99,end));
 float shoulder=smoothstep(width-.12,width,side+grit)*smoothstep(.46,.88,end);
 vec4 flagstone=texture(texture0,uv);
 vec3 dust=flagstone.rgb*vec3(.92,.86,.90);
 finalColor=vec4(mix(flagstone.rgb,dust,fade),flagstone.a*(1.0-fade)*(1.0-shoulder)*(1.0-smoothstep(.93,1.0,end)));
}
)GLSL");
}
inline void canyonFace(const CanyonFace& face){std::array<Vector2,4> p;for(int i=0;i<4;i++){p[i]=projected(face.corners[i]);p[i].y+=face.depth[i];}textured(channelWall,p,face.from,0,face.to,1);}
inline int variant(const Prop& p){return Game::hash(int(std::round(p.pos.x*4))+p.type*131,int(std::round(p.pos.y*4))+53)%8;}
inline Rectangle source(const Prop& p){return {float((p.type-14)*360),float(variant(p)%4*460),360,460};}
inline Texture2D propTexture(const Prop& p,bool outline=false){return variant(p)>=4?(outline?extraSceneryEdge:extraScenery):(outline?sceneryEdge:scenery);}
inline void drawProp(const Prop& p,bool outline,Color tint){
 Vector2 at=screen(p.pos);DrawTexturePro(propTexture(p,outline),source(p),{at.x-90*p.size,at.y-208*p.size,180*p.size,230*p.size},{0,0},0,tint);
 if(!outline&&p.type==16&&steam.id){int frame=(int(game.time*24)+variant(p)*9)%36;DrawTexturePro(steam,{float(frame%6*128),float(frame/6*128),128,128},{at.x-49*p.size,at.y-147*p.size,98*p.size,154*p.size},{0,0},0,Fade(WHITE,tint.a/255.f*.65f));}
}
inline bool propContains(const Prop& p,Vector2 point){Vector2 at=screen(p.pos);int x=int(std::floor(((point.x-at.x)/p.size+90)*2)),y=int(std::floor(((point.y-at.y)/p.size+208)*2));if(x<0||x>=360||y<0||y>=460)return false;Rectangle s=source(p);const auto& alpha=variant(p)>=4?extraSceneryAlpha:sceneryAlpha;return alpha[(int(s.y)+y)*scenery.width+int(s.x)+x]>=160;}
inline constexpr float BoundaryLow=MapWalkMin-.25f,BoundaryHigh=MapWalkMax+.25f;
inline std::array<int,4> boundarySides{};
inline std::vector<CanyonFace> abyssFaces,abyssFalls;
inline std::vector<std::array<V,4>> abyssLips;
inline void prepareAbyss(){
 // Painted 2D cliff strips drop below the platform. Their lower alpha fades
 // into the distant caldera; there is no raised wall or line of boulders.
 const float low=BoundaryLow,high=BoundaryHigh;
 const V corners[]={{low,low},{low,high},{high,high},{high,low},{low,low}};
 const V outward[]={{-1,0},{0,1},{1,0},{0,-1}};
 const int segments=int(std::ceil((high-low)/1.2f));boundarySides={};
 abyssFaces.clear();abyssLips.clear();abyssFalls.clear();abyssFaces.reserve(segments*4);abyssLips.reserve(segments*4);
 auto depth=[&](int side,int n){int vertex=(side*segments+n)%(segments*4);return 480.f+45*std::sin(vertex*.41f)+float(Game::hash(vertex+87,72)%70);};
 for(int side=0;side<4;side++)for(int n=0;n<segments;n++){
  V a=corners[side]+(corners[side+1]-corners[side])*(float(n)/segments);
  V b=corners[side]+(corners[side+1]-corners[side])*(float(n+1)/segments);
  float jagA=.16f+float(Game::hash(n+51,side+810)%24)*.01f,jagB=.16f+float(Game::hash(n+52,side+810)%24)*.01f;
  V rimA=a+outward[side]*jagA,rimB=b+outward[side]*jagB;
  V spreadA=outward[side]*.7f,spreadB=spreadA;
  // Adjacent sides share their complete corner, including its lower point.
  // This closes the seam all the way down into the mist.
  if(n==0){V cornerOut=outward[side]+outward[(side+3)%4];rimA=a+cornerOut*.25f;spreadA=cornerOut*.35f;}
  if(n==segments-1){V cornerOut=outward[side]+outward[(side+1)%4];rimB=b+cornerOut*.25f;spreadB=cornerOut*.35f;}
  abyssLips.push_back({a,rimA,rimB,b});boundarySides[side]++;
  abyssFaces.push_back({{{rimA,rimA+spreadA,rimB+spreadB,rimB}},{{0,depth(side,n),depth(side,n+1),0}},n*.19f,(n+1)*.19f});
 }
 for(int river=0;river<2;river++)for(int edge=0;edge<2;edge++){
  float y=edge?high:low;V out{0,edge?1.f:-1.f},a{lavaCenter(river,y)-1.6f,y},b{lavaCenter(river,y)+1.6f,y};
  abyssFalls.push_back({{{a,a+out*.7f,b+out*.7f,b}},{{VolcanoRiverDepth,540,540,VolcanoRiverDepth}},0,1});
 }
}
inline void bakeBoundary(){for(size_t n=0;n<abyssLips.size();n++)groundQuad(tiles,abyssLips[n],n*.23f,0,(n+1)*.23f,.2f);}
inline void drawAbyssFaces(Texture2D texture,const std::vector<CanyonFace>& faces,bool fall=false){
 rlSetTexture(texture.id);rlBegin(RL_QUADS);
 for(const auto& f:faces){std::array<Vector2,4> p;float left=1e9f,right=-1e9f,top=1e9f,bottom=-1e9f;
  for(int i=0;i<4;i++){p[i]=screen(f.corners[i]);p[i].y+=f.depth[i];left=std::min(left,p[i].x);right=std::max(right,p[i].x);top=std::min(top,p[i].y);bottom=std::max(bottom,p[i].y);}
  if(right<0||left>WorldW||bottom<0||top>H)continue;
  const Vector2 uv[]={{f.from,0},{f.from,1},{f.to,1},{f.to,0}};
  bool flip=(p[1].x-p[0].x)*(p[2].y-p[0].y)-(p[1].y-p[0].y)*(p[2].x-p[0].x)>0;const int normal[]={0,1,2,3},reverse[]={0,3,2,1};
  for(int k=0;k<4;k++){int i=flip?reverse[k]:normal[k];unsigned char alpha=fall&&(i==1||i==2)?0:255;rlColor4ub(255,255,255,alpha);rlTexCoord2f(uv[i].x,uv[i].y);rlVertex2f(p[i].x,p[i].y);}
 }rlEnd();rlSetTexture(0);
}
#include "volcano_road_art.hpp"
inline void make(){
 if(terrain.id)return;
 portalFrame=LoadTexture("assets/volcano/portal-frame.png");portalLoop=LoadTexture("assets/volcano/portal-loop.png");portalOutline=LoadTexture("assets/volcano/portal-outline.png");for(auto t:{portalFrame,portalLoop,portalOutline})SetTextureFilter(t,TEXTURE_FILTER_BILINEAR);
 Image gateMask=LoadImage("assets/volcano/portal-mask.png");Color* gatePixels=LoadImageColors(gateMask);portalMask.resize(gateMask.width*gateMask.height);for(size_t i=0;i<portalMask.size();i++)portalMask[i]=gatePixels[i].r>=160;UnloadImageColors(gatePixels);UnloadImage(gateMask);
 tiles=LoadTexture("assets/volcano/ground.png");scenery=LoadTexture("assets/volcano/scenery.png");sceneryEdge=LoadTexture("assets/volcano/scenery-edge.png");sky=LoadTexture("assets/volcano/sky.png");macro=LoadTexture("assets/volcano/macro.png");road=LoadTexture("assets/volcano/road.png");bank=LoadTexture("assets/volcano/bank.png");bridge=LoadTexture("assets/volcano/bridge.png");steam=LoadTexture("assets/volcano/steam-flow.png");
 channelWall=LoadTexture("assets/volcano/channel-wall.png");SetTextureFilter(channelWall,TEXTURE_FILTER_BILINEAR);SetTextureWrap(channelWall,TEXTURE_WRAP_REPEAT);
 extraScenery=LoadTexture("assets/volcano/scenery-extra.png");extraSceneryEdge=LoadTexture("assets/volcano/scenery-extra-edge.png");SetTextureFilter(extraScenery,TEXTURE_FILTER_BILINEAR);SetTextureFilter(extraSceneryEdge,TEXTURE_FILTER_BILINEAR);
 abyssCliff=LoadTexture("assets/volcano/abyss-cliff.png");abyssDistance=LoadTexture("assets/volcano/abyss-distance.png");
 SetTextureFilter(abyssCliff,TEXTURE_FILTER_BILINEAR);
 rlTextureParameters(abyssCliff.id,RL_TEXTURE_WRAP_S,RL_TEXTURE_WRAP_REPEAT);
 rlTextureParameters(abyssCliff.id,RL_TEXTURE_WRAP_T,RL_TEXTURE_WRAP_CLAMP);
 SetTextureFilter(abyssDistance,TEXTURE_FILTER_BILINEAR);prepareAbyss();
 Image atlas=LoadImage("assets/volcano/lava-flow.png");for(int f=0;f<60;f++){Image frame=ImageFromImage(atlas,{float(f%8*256),float(f/8*256),256,256});lavaFrames[f]=LoadTextureFromImage(frame);UnloadImage(frame);SetTextureFilter(lavaFrames[f],TEXTURE_FILTER_BILINEAR);SetTextureWrap(lavaFrames[f],TEXTURE_WRAP_REPEAT);}UnloadImage(atlas);lava=lavaFrames[0];
 for(Texture2D t:{tiles,scenery,sceneryEdge,sky,macro,road,bank,bridge,steam})if(t.id)SetTextureFilter(t,TEXTURE_FILTER_BILINEAR);
 for(Texture2D t:{tiles,road,bank,bridge})SetTextureWrap(t,TEXTURE_WRAP_REPEAT);
 initBridgeBlend();
 Image mask=LoadImage("assets/volcano/scenery.png");Color* pixels=LoadImageColors(mask);sceneryAlpha.resize(mask.width*mask.height);for(size_t i=0;i<sceneryAlpha.size();i++)sceneryAlpha[i]=pixels[i].a;UnloadImageColors(pixels);UnloadImage(mask);
 mask=LoadImage("assets/volcano/scenery-extra.png");pixels=LoadImageColors(mask);extraSceneryAlpha.resize(mask.width*mask.height);for(size_t i=0;i<extraSceneryAlpha.size();i++)extraSceneryAlpha[i]=pixels[i].a;UnloadImageColors(pixels);UnloadImage(mask);
 terrain=render_targets::colorOnly(TerrainCacheWidth,TerrainCacheHeight);BeginTextureMode(terrain);ClearBackground(BLANK);BeginMode2D({{0,0},{0,0},0,TerrainBakeScale});
 // Feathered road/dust layers must preserve opaque basalt underneath. The
 // default alpha blend squares coverage and punches translucent holes in the
 // cache, exposing the distant backdrop as streaks along raised road edges.
 rlSetBlendFactorsSeparate(RL_SRC_ALPHA,RL_ONE_MINUS_SRC_ALPHA,RL_ONE,RL_ONE_MINUS_SRC_ALPHA,RL_FUNC_ADD,RL_FUNC_ADD);BeginBlendMode(BLEND_CUSTOM_SEPARATE);
 // The surface ends at its actual cliff edge. Transparent surroundings reveal
 // the panorama and the deep faces underneath, instead of continuing the land.
 std::array<V,4> land={V{BoundaryLow,BoundaryLow},V{BoundaryLow,BoundaryHigh},V{BoundaryHigh,BoundaryHigh},V{BoundaryHigh,BoundaryLow}};
 groundQuad(tiles,land,BoundaryLow/5,BoundaryLow/5,BoundaryHigh/5,BoundaryHigh/5);groundQuad(macro,land);
 bakeRoad();
 lavaStrips.reserve(VolcanoRiverSegments*2);canyonFronts.reserve(VolcanoRiverSegments*2);
 for(int river=0;river<2;river++)for(int segment=0;segment<VolcanoRiverSegments;segment++){
  float y=std::max(BoundaryLow,VolcanoRiverStart+segment*.5f),z=std::min(BoundaryHigh,VolcanoRiverStart+(segment+1)*.5f);if(z<=y)continue;
  V a{lavaCenter(river,y),y},b{lavaCenter(river,z),z};
  groundQuad(bank,{{a+V{-3.5f,0},a+V{3.5f,0},b+V{3.5f,0},b+V{-3.5f,0}}},y/4,0,z/4,1);
  std::array<V,4> strip={{a+V{-1.6f,0},b+V{-1.6f,0},b+V{1.6f,0},a+V{1.6f,0}}};
  std::array<Vector2,4> floor;for(int i=0;i<4;i++){floor[i]=projected(strip[i]);floor[i].y+=VolcanoRiverDepth;}textured(lava,floor,0,y/4,1,z/4);lavaStrips.push_back({strip,y/4,z/4});
  CanyonFace far{{{a+V{-2.3f,0},a+V{-1.6f,0},b+V{-1.6f,0},b+V{-2.3f,0}}},{{0,VolcanoRiverDepth,VolcanoRiverDepth,0}},y/2,z/2};canyonFace(far);
  canyonFronts.push_back({{{a+V{2.3f,0},a+V{1.6f,0},b+V{1.6f,0},b+V{2.3f,0}}},{{0,VolcanoRiverDepth,VolcanoRiverDepth,0}},y/2,z/2});
 }
 // The boss's ritual arena uses the same prepainted basalt, with a clear centre.
 Vector2 centre{float(TerrainOriginX+(VolcanoBossHome.x-VolcanoBossHome.y)*40),float(90+(VolcanoBossHome.x+VolcanoBossHome.y)*20)};
 for(int ring=0;ring<3;ring++)ellipseOutline(centre.x,centre.y,180-ring*10,90-ring*5,rgb(128-ring*10,85-ring*6,77-ring*5));
 for(int n=0;n<12;n++){float a=n*PI/6;diamond(centre.x+std::cos(a)*165,centre.y+std::sin(a)*82,5,4,rgb(203,120,68));}
 bakeBoundary();
 EndBlendMode();EndMode2D();EndTextureMode();SetTextureFilter(terrain.texture,TEXTURE_FILTER_BILINEAR);
 terrain_surface::prepare(WorldLocation::Volcano,terrain.texture);
 Image mini=GenImageColor(180,112,BLANK);float mx=78.f/(MapSize-2),my=48.f/(MapSize-2);
 for(int x=1;x<MapSize-1;x++)for(int y=1;y<MapSize-1;y++){V p{float(x),float(y)};ImageDrawPixel(&mini,int(90+(x-y)*mx),int(6+(x+y-2)*my),volcanoLava(p)?rgb(218,96,46):volcano_roads::distanceTo(p)<1.4f?rgb(158,134,126):rgb(75,65,79));}
 for(int edge=0;edge<4;edge++){
  const V corners[]={{BoundaryLow,BoundaryLow},{BoundaryLow,BoundaryHigh},{BoundaryHigh,BoundaryHigh},{BoundaryHigh,BoundaryLow},{BoundaryLow,BoundaryLow}};
  V a=corners[edge],b=corners[edge+1];ImageDrawLine(&mini,int(90+(a.x-a.y)*mx),int(6+(a.x+a.y-2)*my),int(90+(b.x-b.y)*mx),int(6+(b.x+b.y-2)*my),rgb(157,131,108));
 }
 minimap=LoadTextureFromImage(mini);UnloadImage(mini);
}
inline void prop(int type,float x,float y){if(scenery.id)DrawTexturePro(scenery,{float((type-14)*360),0,360,460},{x-90,y-208,180,230},{0,0},0,WHITE);}
inline void backdrop(){
 if(sky.id)DrawTexturePro(sky,{0,0,float(sky.width),float(sky.height)},{0,0,float(WorldW),float(H)},{0,0},0,WHITE);
 if(abyssDistance.id){float drift=(game.player.x-game.player.y)*1.3f;
  DrawTexturePro(abyssDistance,{0,0,float(abyssDistance.width),float(abyssDistance.height)},{-WorldW*.3f+drift,-H*.1f-(game.player.x+game.player.y)*.3f,WorldW*1.6f,H*1.2f},{0,0},0,WHITE);}
 drawAbyssFaces(abyssCliff,abyssFaces);
 drawAbyssFaces(lavaFrames[int(std::fmod(std::max(0.f,game.time)*FlowFPS,float(FlowFrames)))],abyssFalls,true);
}
inline void flow(bool ambience=true){
 struct Visible {std::array<Vector2,4> points;float from,to;};std::array<Visible,VolcanoRiverSegments*2> visible{};int count=0;
 for(const auto& strip:lavaStrips){std::array<Vector2,4> p;float left=1e9f,right=-1e9f,top=1e9f,bottom=-1e9f;for(int i=0;i<4;i++){p[i]=screen(strip.corners[i]);p[i].y+=VolcanoRiverDepth;left=std::min(left,p[i].x);right=std::max(right,p[i].x);top=std::min(top,p[i].y);bottom=std::max(bottom,p[i].y);}if(right<0||left>WorldW||bottom<0||top>H)continue;visible[count++]={p,strip.from,strip.to};}
 float sample=std::fmod(game.time*FlowFPS,float(FlowFrames)),blend=sample-std::floor(sample);int first=int(sample);
 // Two preloaded native Synfig samples interpolate smoothly at the game's rate.
 // Batch each pass: no alternating textures, per-frame allocations or file I/O.
 for(int pass=0;pass<2;pass++){
  unsigned char alpha=pass?static_cast<unsigned char>(std::round(blend*255)):255;if(!alpha)continue;
  rlSetTexture(lavaFrames[(first+pass)%FlowFrames].id);rlBegin(RL_QUADS);rlColor4ub(255,255,255,alpha);
  for(int n=0;n<count;n++){const auto& q=visible[n];const Vector2 uv[]={{0,q.from},{0,q.to},{1,q.to},{1,q.from}};
   for(int i=0;i<4;i++){rlTexCoord2f(uv[i].x,uv[i].y);rlVertex2f(q.points[i].x,q.points[i].y);}}
  rlEnd();rlSetTexture(0);
 }
 // Foreground canyon faces occlude the lower molten surface. One texture batch,
 // precomputed geometry, no per-frame allocations, extra render target or I/O.
 rlSetTexture(channelWall.id);rlBegin(RL_QUADS);rlColor4ub(255,255,255,255);
 for(const auto& face:canyonFronts){std::array<Vector2,4> p;float left=1e9f,right=-1e9f,top=1e9f,bottom=-1e9f;
  for(int i=0;i<4;i++){p[i]=screen(face.corners[i]);p[i].y+=face.depth[i];left=std::min(left,p[i].x);right=std::max(right,p[i].x);top=std::min(top,p[i].y);bottom=std::max(bottom,p[i].y);}if(right<0||left>WorldW||bottom<0||top>H)continue;
  const Vector2 uv[]={{face.from,0},{face.from,1},{face.to,1},{face.to,0}};
  const bool flip=(p[1].x-p[0].x)*(p[2].y-p[0].y)-(p[1].y-p[0].y)*(p[2].x-p[0].x)>0;const int normal[]={0,1,2,3},reverse[]={0,3,2,1};
  for(int k=0;k<4;k++){int i=flip?reverse[k]:normal[k];rlTexCoord2f(uv[i].x,uv[i].y);rlVertex2f(p[i].x,p[i].y);}
 }rlEnd();rlSetTexture(0);
 // The deck spans the channel, then feathers into the baked road on each bank.
 const bool blended=IsShaderValid(bridgeBlend);
 if(blended)BeginShaderMode(bridgeBlend);
 for(int index=0;index<int(VolcanoBridges.size());index++){
  const auto deck=bridgeDeck(index);
  std::array<Vector2,4> points{};
  for(int corner=0;corner<4;corner++)points[corner]=screen(deck[corner]);
  if(points[3].x<0||points[1].x>WorldW||points[2].y<0||points[0].y>H)continue;
  textured(bridge,points,0,0,2,1);
 }
 if(blended)EndShaderMode();
 if(!ambience)return;
 for(int n=0;n<20;n++){float x=std::fmod(n*191.f+game.time*(5+n%3),float(WorldW)),y=std::fmod(n*119.f-game.time*14+H*100,float(H));ellipse(x,y,1.2f,2.5f,Fade(rgb(255,177,99),.2f+.15f*std::sin(n+game.time*2)));}
}
inline void groundAttacks(){
 for(const auto& f:game.fissures){
  Vector2 p=screen(f.pos);
  if(p.x<-115||p.x>WorldW+115||p.y<-70||p.y>H+80)continue;
  float t=std::clamp(f.age/f.delay,0.f,1.f);
  float fade=f.struck?1-std::clamp((f.age-f.delay)/.65f,0.f,1.f):1.f;
  Color hot=rgb(255,156,71);
  ellipse(p.x,p.y,70,35,Fade(rgb(135,43,43),(.30f+.24f*t)*fade));
  terrain_surface::ring(f.pos,1.35f,Fade(hot,(.66f+.32f*t)*fade));
  terrain_surface::ring(f.pos,.27f+t*.94f,Fade(rgb(255,219,145),.65f*fade));
  for(int i=0;i<7;i++){
   float a=i*2*PI/7+.31f;
   V direction{std::cos(a),std::sin(a)};
   Vector2 inside=screen(f.pos+direction*(.28f+.26f*t)),outside=screen(f.pos+direction*(.61f+.58f*t));
   Vector2 bend{(inside.x+outside.x)*.5f-4,(inside.y+outside.y)*.5f+3};
   DrawLineEx(inside,bend,1.5f,Fade(hot,fade*t));DrawLineEx(bend,outside,1.5f,Fade(hot,fade*t));
  }
 }
}
inline void eruptionPlumes(){
 for(const auto& f:game.fissures){
  if(!f.struck)continue;
  Vector2 p=screen(f.pos);
  if(p.x<-130||p.x>WorldW+130||p.y<-45||p.y>H+200)continue;
  float phase=std::clamp((f.age-f.delay)/.65f,0.f,1.f);
  if(caldera_attack_art::draw(caldera_attack_art::Clip::Eruption,p,{162,197},0,phase,1,{.5f,.83f}))continue;
  float fade=1-phase;
  for(int i=0;i<5;i++){
   float off=(i-2)*16;
   line(p.x+off,p.y+std::abs(off)*.18f,p.x+off*.8f,p.y-(35+26*std::sin(i+2))*fade,Fade(rgb(255,199,103),fade),5*fade+1);
  }
 }
}
inline void bolt(const CinderBolt& b){Vector2 p=screen(b.pos),v{(b.velocity.x-b.velocity.y)*40,(b.velocity.x+b.velocity.y)*20};float n=std::hypot(v.x,v.y);if(n<=0)return;v.x/=n;v.y/=n;
 float phase=std::fmod(std::max(0.f,7-b.remaining)*.29f,1.f);
 if(caldera_attack_art::draw(caldera_attack_art::Clip::Cinder,{p.x,p.y-35},{80,48},std::atan2(v.y,v.x)*RAD2DEG,phase,1,{.64f,.5f}))return;
 for(int i=4;i>=0;i--)ellipse(p.x-v.x*i*5,p.y-35-v.y*i*5,6-i*.7f,5-i*.6f,Fade(rgb(255,110+i*15,48),1-i*.16f));ellipse(p.x,p.y-35,2.3f,2.3f,rgb(255,239,183));
}
inline void unload(){for(auto t:{tiles,scenery,sceneryEdge,extraScenery,extraSceneryEdge,sky,minimap,macro,road,bank,bridge,steam,channelWall,portalFrame,portalLoop,portalOutline,abyssCliff,abyssDistance})if(t.id)UnloadTexture(t);for(auto t:lavaFrames)if(t.id)UnloadTexture(t);if(terrain.id)UnloadRenderTexture(terrain);if(IsShaderValid(bridgeBlend))UnloadShader(bridgeBlend);bridgeBlend={};tiles=scenery=sceneryEdge=extraScenery=extraSceneryEdge=sky=lava=minimap=macro=road=bank=bridge=steam=channelWall=portalFrame=portalLoop=portalOutline=abyssCliff=abyssDistance={};lavaFrames={};terrain={};lavaStrips.clear();canyonFronts.clear();abyssFaces.clear();abyssFalls.clear();abyssLips.clear();sceneryAlpha.clear();extraSceneryAlpha.clear();portalMask.clear();}
}
