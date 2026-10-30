#pragma once
namespace volcano_art {
inline Texture2D tiles{},scenery{},sceneryEdge{},sky{},lava{},minimap{},macro{},road{},bank{},bridge{},steam{},channelWall{};inline RenderTexture2D terrain{};
inline Texture2D extraScenery{},extraSceneryEdge{};inline std::vector<unsigned char> extraSceneryAlpha;
inline constexpr int FlowFrames=60;inline constexpr float FlowFPS=30;
inline std::array<Texture2D,60> lavaFrames{};
inline std::vector<unsigned char> sceneryAlpha;
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
inline void canyonFace(const CanyonFace& face){std::array<Vector2,4> p;for(int i=0;i<4;i++){p[i]=projected(face.corners[i]);p[i].y+=face.depth[i];}textured(channelWall,p,face.from,0,face.to,1);}
inline int variant(const Prop& p){return Game::hash(int(std::round(p.pos.x*4))+p.type*131,int(std::round(p.pos.y*4))+53)%8;}
inline Rectangle source(const Prop& p){return {float((p.type-14)*360),float(variant(p)%4*460),360,460};}
inline Texture2D propTexture(const Prop& p,bool outline=false){return variant(p)>=4?(outline?extraSceneryEdge:extraScenery):(outline?sceneryEdge:scenery);}
inline void drawProp(const Prop& p,bool outline,Color tint){
 Vector2 at=screen(p.pos);DrawTexturePro(propTexture(p,outline),source(p),{at.x-90*p.size,at.y-208*p.size,180*p.size,230*p.size},{0,0},0,tint);
 if(!outline&&p.type==16&&steam.id){int frame=(int(game.time*24)+variant(p)*9)%36;DrawTexturePro(steam,{float(frame%6*128),float(frame/6*128),128,128},{at.x-49*p.size,at.y-147*p.size,98*p.size,154*p.size},{0,0},0,Fade(WHITE,tint.a/255.f*.65f));}
}
inline bool propContains(const Prop& p,Vector2 point){Vector2 at=screen(p.pos);int x=int(std::floor(((point.x-at.x)/p.size+90)*2)),y=int(std::floor(((point.y-at.y)/p.size+208)*2));if(x<0||x>=360||y<0||y>=460)return false;Rectangle s=source(p);const auto& alpha=variant(p)>=4?extraSceneryAlpha:sceneryAlpha;return alpha[(int(s.y)+y)*scenery.width+int(s.x)+x]>=160;}
inline void make(){
 if(terrain.id)return;
 tiles=LoadTexture("assets/volcano/ground.png");scenery=LoadTexture("assets/volcano/scenery.png");sceneryEdge=LoadTexture("assets/volcano/scenery-edge.png");sky=LoadTexture("assets/volcano/sky.png");macro=LoadTexture("assets/volcano/macro.png");road=LoadTexture("assets/volcano/road.png");bank=LoadTexture("assets/volcano/bank.png");bridge=LoadTexture("assets/volcano/bridge.png");steam=LoadTexture("assets/volcano/steam-flow.png");
 channelWall=LoadTexture("assets/volcano/channel-wall.png");SetTextureFilter(channelWall,TEXTURE_FILTER_BILINEAR);SetTextureWrap(channelWall,TEXTURE_WRAP_REPEAT);
 extraScenery=LoadTexture("assets/volcano/scenery-extra.png");extraSceneryEdge=LoadTexture("assets/volcano/scenery-extra-edge.png");SetTextureFilter(extraScenery,TEXTURE_FILTER_BILINEAR);SetTextureFilter(extraSceneryEdge,TEXTURE_FILTER_BILINEAR);
 Image atlas=LoadImage("assets/volcano/lava-flow.png");for(int f=0;f<60;f++){Image frame=ImageFromImage(atlas,{float(f%8*256),float(f/8*256),256,256});lavaFrames[f]=LoadTextureFromImage(frame);UnloadImage(frame);SetTextureFilter(lavaFrames[f],TEXTURE_FILTER_BILINEAR);SetTextureWrap(lavaFrames[f],TEXTURE_WRAP_REPEAT);}UnloadImage(atlas);lava=lavaFrames[0];
 for(Texture2D t:{tiles,scenery,sceneryEdge,sky,macro,road,bank,bridge,steam})if(t.id)SetTextureFilter(t,TEXTURE_FILTER_BILINEAR);
 for(Texture2D t:{tiles,road,bank,bridge})SetTextureWrap(t,TEXTURE_WRAP_REPEAT);
 Image mask=LoadImage("assets/volcano/scenery.png");Color* pixels=LoadImageColors(mask);sceneryAlpha.resize(mask.width*mask.height);for(size_t i=0;i<sceneryAlpha.size();i++)sceneryAlpha[i]=pixels[i].a;UnloadImageColors(pixels);UnloadImage(mask);
 mask=LoadImage("assets/volcano/scenery-extra.png");pixels=LoadImageColors(mask);extraSceneryAlpha.resize(mask.width*mask.height);for(size_t i=0;i<extraSceneryAlpha.size();i++)extraSceneryAlpha[i]=pixels[i].a;UnloadImageColors(pixels);UnloadImage(mask);
 terrain=render_targets::colorOnly(TerrainWidth,TerrainHeight);BeginTextureMode(terrain);ClearBackground(BLANK);
 // Painted land extends under the camera margins; lava feeds in from offscreen.
 std::array<V,4> land={V{-16,-16},V{-16,MapSize+16.f},V{MapSize+16.f,MapSize+16.f},V{MapSize+16.f,-16}};groundQuad(tiles,land,-3.2f,-3.2f,(MapSize+16)/5.f,(MapSize+16)/5.f);groundQuad(macro,land);
 for(const auto& path:VolcanoRoads){float along=0;for(int i=1;i<6;i++){V a=path[i-1],b=path[i],d=b-a;float len=length(d);V side=normalized(V{-d.y,d.x})*2.1f;groundQuad(road,{{a-side,a+side,b+side,b-side}},along/4,0,(along+len)/4,1);along+=len;}}
 lavaStrips.reserve(VolcanoRiverSegments*2);canyonFronts.reserve(VolcanoRiverSegments*2);
 for(int river=0;river<2;river++)for(int segment=0;segment<VolcanoRiverSegments;segment++){
  float y=VolcanoRiverStart+segment*.5f,z=y+.5f;V a{lavaCenter(river,y),y},b{lavaCenter(river,z),z};
  groundQuad(bank,{{a+V{-3.5f,0},a+V{3.5f,0},b+V{3.5f,0},b+V{-3.5f,0}}},y/4,0,z/4,1);
  if(volcanoBridge({a.x,(y+z)*.5f}))continue;
  std::array<V,4> strip={{a+V{-1.6f,0},b+V{-1.6f,0},b+V{1.6f,0},a+V{1.6f,0}}};
  std::array<Vector2,4> floor;for(int i=0;i<4;i++){floor[i]=projected(strip[i]);floor[i].y+=VolcanoRiverDepth;}textured(lava,floor,0,y/4,1,z/4);lavaStrips.push_back({strip,y/4,z/4});
  CanyonFace far{{{a+V{-2.3f,0},a+V{-1.6f,0},b+V{-1.6f,0},b+V{-2.3f,0}}},{{0,VolcanoRiverDepth,VolcanoRiverDepth,0}},y/2,z/2};canyonFace(far);
  canyonFronts.push_back({{{a+V{2.3f,0},a+V{1.6f,0},b+V{1.6f,0},b+V{2.3f,0}}},{{0,VolcanoRiverDepth,VolcanoRiverDepth,0}},y/2,z/2});
 }
 for(int i=0;i<4;i++){float y=VolcanoBridges[i].y;V a{lavaCenter(i/2,y),y};groundQuad(bridge,{{a+V{-5.3f,-2.8f},a+V{-5.3f,2.8f},a+V{5.3f,2.8f},a+V{5.3f,-2.8f}}},0,0,2,1);}
 // The boss's ritual arena uses the same prepainted basalt, with a clear centre.
 Vector2 centre{float(TerrainOriginX+(VolcanoBossHome.x-VolcanoBossHome.y)*40),float(90+(VolcanoBossHome.x+VolcanoBossHome.y)*20)};
 for(int ring=0;ring<3;ring++)ellipseOutline(centre.x,centre.y,180-ring*10,90-ring*5,rgb(128-ring*10,85-ring*6,77-ring*5));
 for(int n=0;n<12;n++){float a=n*PI/6;diamond(centre.x+std::cos(a)*165,centre.y+std::sin(a)*82,5,4,rgb(203,120,68));}
 EndTextureMode();SetTextureFilter(terrain.texture,TEXTURE_FILTER_BILINEAR);
 Image mini=GenImageColor(180,112,BLANK);float mx=78.f/(MapSize-2),my=48.f/(MapSize-2);
 for(int x=1;x<MapSize-1;x++)for(int y=1;y<MapSize-1;y++){V p{float(x),float(y)};ImageDrawPixel(&mini,int(90+(x-y)*mx),int(6+(x+y-2)*my),volcanoLava(p)?rgb(218,96,46):volcanoPath(p)?rgb(158,134,126):rgb(75,65,79));}
 minimap=LoadTextureFromImage(mini);UnloadImage(mini);
}
inline void prop(int type,float x,float y){if(scenery.id)DrawTexturePro(scenery,{float((type-14)*360),0,360,460},{x-90,y-208,180,230},{0,0},0,WHITE);}
inline void backdrop(){if(sky.id)DrawTexturePro(sky,{0,0,1600,700},{0,0,float(WorldW),float(H)},{0,0},0,rgb(154,132,143));}
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
 if(!ambience)return;
 for(int n=0;n<20;n++){float x=std::fmod(n*191.f+game.time*(5+n%3),float(WorldW)),y=std::fmod(n*119.f-game.time*14+H*100,float(H));ellipse(x,y,1.2f,2.5f,Fade(rgb(255,177,99),.2f+.15f*std::sin(n+game.time*2)));}
}
inline void groundAttacks(){for(const auto& f:game.fissures){Vector2 p=screen(f.pos);float t=std::min(1.f,f.age/f.delay);Color col=rgb(250,136,74);
 ellipse(p.x,p.y,65,32,Fade(col,f.struck?.25f:.08f+.15f*t));ellipseOutline(p.x,p.y,65,32,Fade(col,.65f));ellipseOutline(p.x,p.y,65*t,32*t,col);
 if(f.struck){float fade=1-std::clamp((f.age-f.delay)/.65f,0.f,1.f);for(int i=0;i<5;i++){float off=(i-2)*16;line(p.x+off,p.y+std::abs(off)*.18f,p.x+off*.8f,p.y-(35+26*std::sin(i+2))*fade,Fade(rgb(255,199,103),fade),5*fade+1);}}
}}
inline void bolt(const CinderBolt& b){Vector2 p=screen(b.pos),v{(b.velocity.x-b.velocity.y)*40,(b.velocity.x+b.velocity.y)*20};float n=std::hypot(v.x,v.y);if(n<=0)return;v.x/=n;v.y/=n;
 for(int i=4;i>=0;i--)ellipse(p.x-v.x*i*5,p.y-35-v.y*i*5,6-i*.7f,5-i*.6f,Fade(rgb(255,110+i*15,48),1-i*.16f));ellipse(p.x,p.y-35,2.3f,2.3f,rgb(255,239,183));
}
inline void unload(){for(auto t:{tiles,scenery,sceneryEdge,extraScenery,extraSceneryEdge,sky,minimap,macro,road,bank,bridge,steam,channelWall})if(t.id)UnloadTexture(t);for(auto t:lavaFrames)if(t.id)UnloadTexture(t);if(terrain.id)UnloadRenderTexture(terrain);tiles=scenery=sceneryEdge=extraScenery=extraSceneryEdge=sky=lava=minimap=macro=road=bank=bridge=steam=channelWall={};lavaFrames={};terrain={};lavaStrips.clear();canyonFronts.clear();sceneryAlpha.clear();extraSceneryAlpha.clear();}
}
