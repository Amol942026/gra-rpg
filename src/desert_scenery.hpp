#pragma once
// Included inside desert_art. Geometry is rasterized once; weather stays in world space.
std::array<RenderTexture2D,4> mountainArt{};
Texture2D sandPlume{},desertPanorama{},paintedRidge{},outsideSand{};
RenderTexture2D mountainBackdrop{};
Shader outsideGroundShader{};
int groundOriginLocation=-1,groundTransitionLocation=-1;
bool optimizeBackdrop=true,backdropDrawn=true,backdropReady=false;
Vector2 backdropCamera{};
float backdropScale=-1;
unsigned backdropBuilds=0;
constexpr float MountainWidth=600,MountainHeight=440,MountainFoot=360,MountainSpacing=3.55f;
constexpr float MountainDensity=1.85f;
void beginTransparentLayer(){
 // Straight RGB, accumulated coverage for alpha. Dust over opaque soil must
 // preserve that soil's alpha rather than cutting dark holes in the world FBO.
 constexpr int srcAlpha=0x0302,oneMinusSrcAlpha=0x0303,one=1,add=0x8006;
 rlSetBlendFactorsSeparate(srcAlpha,oneMinusSrcAlpha,one,oneMinusSrcAlpha,add,add);BeginBlendMode(BLEND_CUSTOM_SEPARATE);
}
float noise(float x,float y,int seed){
 int ix=int(std::floor(x)),iy=int(std::floor(y));float a=x-ix,b=y-iy;a=a*a*(3-2*a);b=b*b*(3-2*b);
 auto value=[&](int x,int y){return (Game::hash(x+seed*37,y+seed*71)%65536)/65535.f;};
 float top=value(ix,iy)*(1-a)+value(ix+1,iy)*a,bottom=value(ix,iy+1)*(1-a)+value(ix+1,iy+1)*a;return top*(1-b)+bottom*b;
}
struct MountainInstance {V pos;float size,height;int variant;};
const std::vector<MountainInstance>& foothillRanges(){
 static const auto ranges=[](){
  std::vector<MountainInstance> result;
  constexpr float low=-3.7f;
  constexpr float front=MapSize+2.6f;
  const V corners[]={{low,low},{front,low},{front,front},{low,front}};
  for(int side=0;side<4;side++){
   V a=corners[side],b=corners[(side+1)%4];
   if(side==0)a.y=b.y=low-1.5f;
   if(side==3)a.x=b.x=low-1.5f;
   const int segments=int(std::ceil(distance(a,b)/3.5f));
   for(int i=0;i<segments;i++){
    const unsigned seed=Game::hash(side+421,i+738);
    const float t=(i+.18f+(seed%71)*.007f)/segments;
    V p=a+(b-a)*t;
    const float drift=.74f*std::sin(i*.79f+side*1.21f)+.98f*std::sin(i*.37f+side*2.07f);
    if(side%2)p.x+=drift;else p.y+=drift;
    result.push_back({p,.66f+(seed%40)*.009f,.47f+((seed>>7)%31)*.009f,int((seed>>12)%4)});
   }
  }
  std::sort(result.begin(),result.end(),[](const auto& a,const auto& b){return a.pos.x+a.pos.y<b.pos.x+b.pos.y;});
  return result;
 }();return ranges;
}
void makeOutsideGround(){
 outsideSand=terrain_paint::load("desert-ground.png",true);
 const int painted=outsideSand.id?1:0;
 if(!outsideSand.id){
  Image pixel=GenImageColor(1,1,WHITE);
  outsideSand=LoadTextureFromImage(pixel);
  UnloadImage(pixel);
 }
 const std::string fragment=std::string(R"GLSL(#version 330
in vec2 fragTexCoord;
out vec4 finalColor;
uniform sampler2D texture0;
uniform vec2 worldOrigin;
uniform vec2 viewSize;
uniform float worldSize;
uniform int painted;
uniform int transition;
)GLSL")+terrain_paint::GroundMaterial+R"GLSL(
float grain(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
float duneEdge(float along,float side){
 return 1.13*sin(along*.43+side*2.2)+.65*sin(along*.91+side*1.4)
       +.30*sin(along*2.28+side);
}
void main(){
 vec2 q=fragTexCoord*viewSize;
 vec2 p=worldOrigin+vec2(q.x/80.0+q.y/40.0,-q.x/80.0+q.y/40.0);
 vec3 sand;
 if(painted==1)sand=groundColor(texture0,p);
 else{
  float wave=sin(p.x*.17+p.y*.095)+.38*sin(p.x*.11-p.y*.25);
  float grit=(grain(floor(p*11.0))-.5)*3.0;
  sand=vec3(211.0+wave*7.0+grit,170.0+wave*6.0+grit,112.0+wave*4.0+grit*.7)/255.0;
 }
 if(transition==0){finalColor=vec4(sand,1.);return;}
 // Extend the same sand in irregular tongues between the foothills. Its
 // fine grain breaks up the transition without outlining the map diamond.
 vec4 edge=vec4(p-.5,vec2(worldSize-1.5)-p);
 vec4 shift=vec4(duneEdge(p.y,0.),duneEdge(p.x,1.),duneEdge(p.y,2.),duneEdge(p.x,3.));
 edge+=shift;
 float apron=min(min(edge.x,edge.y),min(edge.z,edge.w));
 float textureBreak=painted==1?(texture(texture0,p/.86).r-.62)*.82:0.;
 float fade=smoothstep(-4.6,.65,apron+textureBreak);
 float floor=smoothstep(-.78,.50,landInset(p,worldSize-1.5));
 float alpha=max(fade,floor);
 finalColor=vec4(sand*alpha,alpha);
})GLSL";
 outsideGroundShader=LoadShaderFromMemory(nullptr,fragment.c_str());
 groundOriginLocation=GetShaderLocation(outsideGroundShader,"worldOrigin");
 groundTransitionLocation=GetShaderLocation(outsideGroundShader,"transition");
 Vector2 dimensions{float(WorldW),float(H)};SetShaderValue(outsideGroundShader,GetShaderLocation(outsideGroundShader,"viewSize"),&dimensions,SHADER_UNIFORM_VEC2);
 const float worldSize=MapSize;
 SetShaderValue(outsideGroundShader,GetShaderLocation(outsideGroundShader,"worldSize"),&worldSize,SHADER_UNIFORM_FLOAT);
 SetShaderValue(outsideGroundShader,GetShaderLocation(outsideGroundShader,"painted"),&painted,SHADER_UNIFORM_INT);
}
void drawOutsideGround(bool transition=false){
 if(groundOriginLocation<0||!outsideSand.id)return;
 const Vector2 origin=screenFlat({0,0});
 const Vector2 world{-origin.x/80-origin.y/40,origin.x/80-origin.y/40};
 const int mode=transition?1:0;
 BeginShaderMode(outsideGroundShader);
 SetShaderValue(outsideGroundShader,groundOriginLocation,&world,SHADER_UNIFORM_VEC2);
 SetShaderValue(outsideGroundShader,groundTransitionLocation,&mode,SHADER_UNIFORM_INT);
 DrawTexturePro(outsideSand,{0,0,float(outsideSand.width),float(outsideSand.height)},{0,0,WorldW,H},{0,0},0,WHITE);
 EndShaderMode();
}
void drawFoothills(){
 for(const auto& range:foothillRanges()){
  const Vector2 p=screenFlat(range.pos);
  const float width=MountainWidth*range.size,height=MountainHeight*range.height;
  const Rectangle dest={p.x-width*.5f,p.y-MountainFoot*range.height,width,height};
  if(dest.x+dest.width<0||dest.x>WorldW||dest.y+dest.height<0||dest.y>H)continue;
  const Texture2D texture=mountainArt[range.variant].texture;
  DrawTexturePro(texture,{0,0,float(texture.width),-float(texture.height)},dest,{},0,rgb(247,238,224));
 }
}
void makeScenery(){
 makeOutsideGround();
 desertPanorama=terrain_paint::load("desert-distance.png");
 paintedRidge=terrain_paint::load("desert-ridge.png");
 constexpr int nx=64,ny=48;constexpr float density=MountainDensity;
 Shader ridgeDetail{};
 if(paintedRidge.id){
  ridgeDetail=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;
uniform sampler2D texture0;
uniform vec2 texel;
void main(){
 vec2 uv=fragTexCoord;
 vec4 c=texture(texture0,uv);
 vec4 r=texture(texture0,uv+vec2(texel.x,0.));
 vec4 l=texture(texture0,uv-vec2(texel.x,0.));
 vec4 u=texture(texture0,uv+vec2(0.,texel.y));
 vec4 d=texture(texture0,uv-vec2(0.,texel.y));
 float cover=min(min(l.a,r.a),min(u.a,d.a));
 vec3 detail=clamp(c.rgb-(r.rgb+l.rgb+u.rgb+d.rgb)*.25,vec3(-.09),vec3(.09));
 vec3 paint=clamp(c.rgb+detail*(.66*smoothstep(.82,.99,cover)),0.,1.);
 finalColor=vec4(paint,c.a)*fragColor;
}
)GLSL");
  const Vector2 texel{1.45f/paintedRidge.width,1.25f/paintedRidge.height};
  SetShaderValue(ridgeDetail,GetShaderLocation(ridgeDetail,"texel"),&texel,SHADER_UNIFORM_VEC2);
 }
 struct Vertex{Vector2 point;float depth;Color color;};
 for(int variant=0;variant<int(mountainArt.size());variant++){
  if(paintedRidge.id){
   const int w=int(MountainWidth*density),h=int(MountainHeight*density);
   mountainArt[variant]=LoadRenderTexture(w,h);
   BeginTextureMode(mountainArt[variant]);ClearBackground(BLANK);beginTransparentLayer();
   const float top=(43+(variant%2)*20)*density,foot=(MountainFoot+29)*density;
   if(ridgeDetail.id)BeginShaderMode(ridgeDetail);
   DrawTexturePro(paintedRidge,{0,0,(variant%2?-1.f:1.f)*paintedRidge.width,float(paintedRidge.height)},
    {0,top,float(w),foot-top},{0,0},0,rgb(255-variant*5,252-variant*5,247-variant*6));
   if(ridgeDetail.id)EndShaderMode();
   native_world::endBlendMode();EndTextureMode();SetTextureFilter(mountainArt[variant].texture,TEXTURE_FILTER_BILINEAR);
   continue;
  }
  auto height=[&](float x,float y){
   float radius=x*x+y*y;if(radius>=1)return 0.f;
   float a=x+.11f-variant*.035f,b=y+.15f;
   float peaks=std::exp(-a*a*6-b*b*5.5f)+.55f*std::exp(-(x-.42f)*(x-.42f)*17-(y+.1f)*(y+.1f)*13)+.35f*std::exp(-(x+.5f)*(x+.5f)*15-(y-.22f)*(y-.22f)*17);
   float erosion=(noise(x*5,y*5,variant+9)-.5f)*.20f+(noise(x*12,y*12,variant+31)-.5f)*.08f;
   return 305*std::pow(std::max(0.f,1-radius),.55f)*std::max(0.f,peaks+erosion);
  };
  std::array<std::array<Vertex,nx+1>,ny+1> mesh{};
  for(int j=0;j<=ny;j++)for(int i=0;i<=nx;i++){
   float x=-1+2.f*i/nx,y=-1+2.f*j/ny,z=height(x,y),hR=(height(x+.015f,y)-height(x-.015f,y))/(.03f*278),hF=(height(x,y+.015f)-height(x,y-.015f))/(.03f*105);
   float len=std::sqrt(1+hR*hR+hF*hF),light=.78f+(-hR*-.38f-hF*-.4f+.78f)/len*.37f;
   float strata=std::sin(z*.105f+noise(x*6,y*6,variant+6)*4)*2.7f,grit=(noise(x*38,y*38,variant+19)-.5f)*7;
   Color base=rgb(169+z*.08f+strata+grit,120+z*.065f+strata+grit*.7f,79+z*.046f+strata+grit*.4f);
   Color col=shade(base,light);float radius=std::sqrt(x*x+y*y);col.a=(unsigned char)(255*std::clamp((1-radius)*12,0.f,1.f));
   mesh[j][i]={{300+x*278,MountainFoot+y*105*.52f-z},-.64f+(y*105*.86f+z*.52f)*.001f,col};
  }
  mountainArt[variant]=LoadRenderTexture(int(MountainWidth*density),int(MountainHeight*density));BeginTextureMode(mountainArt[variant]);ClearBackground(BLANK);BeginMode2D({{0,0},{0,0},0,density});
  beginTransparentLayer();rlDrawRenderBatchActive();rlEnableDepthTest();rlSetTexture(rlGetTextureIdDefault());
  for(int j=0;j<ny;j++)for(int i=0;i<nx;i++){
   std::array<Vertex,4> quad={mesh[j][i],mesh[j][i+1],mesh[j+1][i+1],mesh[j+1][i]};
   if(!quad[0].color.a&&!quad[1].color.a&&!quad[2].color.a&&!quad[3].color.a)continue;
   for(int tri=0;tri<2;tri++){int a=0,b=tri+1,c=tri+2;auto p=quad[a].point,q=quad[b].point,s=quad[c].point;if((q.x-p.x)*(s.y-p.y)-(q.y-p.y)*(s.x-p.x)>0)std::swap(b,c);
    rlCheckRenderBatchLimit(3);rlBegin(RL_TRIANGLES);for(int index:{a,b,c}){auto v=quad[index];rlColor4ub(v.color.r,v.color.g,v.color.b,v.color.a);rlTexCoord2f(0,0);rlVertex3f(v.point.x,v.point.y,v.depth);}rlEnd();
   }
  }
  rlSetTexture(0);rlDrawRenderBatchActive();rlDisableDepthTest();native_world::endBlendMode();EndMode2D();EndTextureMode();SetTextureFilter(mountainArt[variant].texture,TEXTURE_FILTER_BILINEAR);
 }
 if(IsShaderValid(ridgeDetail)&&ridgeDetail.id!=rlGetShaderIdDefault())UnloadShader(ridgeDetail);
 Image plume=GenImageColor(192,64,BLANK);Color* pixels=(Color*)plume.data;
 for(int y=0;y<plume.height;y++)for(int x=0;x<plume.width;x++){
  float u=(x+.5f)/plume.width,v=(y+.5f)/plume.height,center=.5f+.065f*std::sin(u*PI*3),width=.16f+.12f*std::sin(u*PI),dy=(v-center)/width;
  float envelope=std::pow(std::sin(u*PI),1.35f)*std::exp(-dy*dy*2.5f),billows=.55f+.45f*noise(u*7,v*5,76);
  pixels[y*plume.width+x]=rgb(247,211,157,int(140*envelope*billows));
 }
 sandPlume=LoadTextureFromImage(plume);UnloadImage(plume);SetTextureFilter(sandPlume,TEXTURE_FILTER_BILINEAR);
}
void unloadScenery(){
 backdropReady=false;backdropScale=-1;backdropDrawn=true;
 if(IsShaderValid(outsideGroundShader)&&outsideGroundShader.id!=rlGetShaderIdDefault())UnloadShader(outsideGroundShader);
 outsideGroundShader={};groundOriginLocation=groundTransitionLocation=-1;
 if(mountainBackdrop.id)UnloadRenderTexture(mountainBackdrop);
 mountainBackdrop={};
 for(auto& texture:mountainArt){if(texture.id)UnloadRenderTexture(texture);texture={};}
 for(auto t:{sandPlume,desertPanorama,paintedRidge,outsideSand})if(t.id)UnloadTexture(t);
 sandPlume=desertPanorama=paintedRidge=outsideSand={};
}
struct WorldBounds{float minX,maxX,minY,maxY;};
WorldBounds visibleBounds(float margin){
 auto a=toWorld({0,0}),b=toWorld({WorldW,0}),c=toWorld({0,H}),d=toWorld({WorldW,H});
 return {std::min({a.x,b.x,c.x,d.x})-margin,std::max({a.x,b.x,c.x,d.x})+margin,std::min({a.y,b.y,c.y,d.y})-margin,std::max({a.y,b.y,c.y,d.y})+margin};
}
void drawBackdrop(bool mountains=true,bool paintDistance=true){
 if(paintDistance){
  if(mountains&&desertPanorama.id)terrain_paint::drawPanorama(desertPanorama);
  else drawOutsideGround();
 }
 if(!mountains)return;
 auto bounds=visibleBounds(18);int minX=int(std::floor(bounds.minX/MountainSpacing)),maxX=int(std::ceil(bounds.maxX/MountainSpacing)),minY=int(std::floor(bounds.minY/MountainSpacing)),maxY=int(std::ceil(bounds.maxY/MountainSpacing));
 BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
 // Drawing by world depth joins neighbouring foothills in a continuous belt.
 for(int sum=minX+minY;sum<=maxX+maxY;sum++)for(int x=minX;x<=maxX;x++){
  int y=sum-x;if(y<minY||y>maxY)continue;unsigned seed=Game::hash(x+483,y+815);V pos{x*MountainSpacing+(seed%101)*.009f,y*MountainSpacing+((seed>>8)%101)*.009f};
  float distanceToMap=std::max({.5f-pos.x,.5f-pos.y,pos.x-(MapSize-1.5f),pos.y-(MapSize-1.5f),0.f});
  const bool frontSide=pos.x>MapSize-1.5f||pos.y>MapSize-1.5f;
  const bool rearFoot=std::min(pos.x,pos.y)<5.2f;
  // Near ranks join the feet of the foreground ridges to the distant ranges.
  // These ranks are painted below the playable ground, including at corners.
  if(distanceToMap<(frontSide?.9f:2.3f)&&!rearFoot)continue;
  const float size=.94f+(seed%37)*.008f;
  const float rise=size*(.58f+.42f*terrain_height::smooth(1,12,distanceToMap));
  const Vector2 p=screenFlat(pos);
  const Rectangle dest{p.x-MountainWidth*.5f*size,p.y-MountainFoot*rise,MountainWidth*size,MountainHeight*rise};
  if(dest.x+dest.width<0||dest.x>WorldW||dest.y+dest.height<0||dest.y>H)continue;
  const auto& tex=mountainArt[(seed>>13)%mountainArt.size()].texture;
  Color tint=distanceToMap>15?rgb(217,213,203):distanceToMap>7?rgb(238,229,211):WHITE;
  DrawTexturePro(tex,{0,0,float(tex.width),-float(tex.height)},dest,{},0,tint);
 }
 drawFoothills();
 if(paintDistance)drawOutsideGround(true);
 native_world::endBlendMode();
}
// Keep the detail at the pixel density of the world. A stationary camera
// reuses the single precomposited layer, including the low sand apron.
void renderBackdrop(float pixelsPerUnit){
 const float resolution=pixelsPerUnit;
 const int width=std::max(1,int(std::ceil(WorldW*resolution))),height=std::max(1,int(std::ceil(H*resolution)));
 const Vector2 camera=screenFlat({0,0});
 if(optimizeBackdrop&&backdropReady&&backdropScale==resolution&&backdropCamera.x==camera.x&&backdropCamera.y==camera.y)return;
 backdropCamera=camera;backdropScale=resolution;backdropReady=true;backdropBuilds++;
 // Opaque interior terrain completely covers the distant background. Keep
 // a generous tile inset so filtered map edges and shores still render it.
 bool covered=optimizeBackdrop;
 for(Vector2 corner:{Vector2{0,0},Vector2{WorldW,0},Vector2{0,H},Vector2{WorldW,H}}){V p=toWorld(corner);covered&=p.x>1.75f&&p.y>1.75f&&p.x<MapSize-2.75f&&p.y<MapSize-2.75f;}
 backdropDrawn=!covered;
 if(backdropDrawn){
  if(!mountainBackdrop.id||mountainBackdrop.texture.width!=width||mountainBackdrop.texture.height!=height){
   if(mountainBackdrop.id)UnloadRenderTexture(mountainBackdrop);
   mountainBackdrop=render_targets::colorOnly(width,height);
   SetTextureFilter(mountainBackdrop.texture,TEXTURE_FILTER_BILINEAR);
  }
  BeginTextureMode(mountainBackdrop);ClearBackground(rgb(211,170,112));
  BeginMode2D({{0,0},{0,0},0,resolution});drawBackdrop();EndMode2D();EndTextureMode();
 }
}
void drawCachedBackdrop(){
 if(!backdropDrawn)return;
 if(!mountainBackdrop.id){drawBackdrop();return;}
 const auto tex=mountainBackdrop.texture;
 DrawTexturePro(tex,{0,tex.height-H*backdropScale,WorldW*backdropScale,-H*backdropScale},{0,0,WorldW,H},{},0,WHITE);
}
float gust(float time){return .53f+.22f*std::sin(time*.43f)+.14f*std::sin(time*.19f+1.7f)+.10f*std::sin(time*.83f+.6f);}
void drawSandWind(){
 beginTransparentLayer();
 auto bounds=visibleBounds(9);constexpr V flow{.82f,-.38f};
 const float ageClock=game.time*wind_motion::Rate;
 // Broad, very thin veils slide over the soil. Birth/death fades are continuous;
 // speed is linear and world anchored, including during sprinting and rolling.
 constexpr float spacing=5.f*wind_motion::ParticleSpacing;
 for(int y=int(std::floor(bounds.minY/spacing));y<=int(std::ceil(bounds.maxY/spacing));y++)for(int x=int(std::floor(bounds.minX/spacing));x<=int(std::ceil(bounds.maxX/spacing));x++){
  unsigned seed=Game::hash(x+156,y+952);V origin{x*spacing+(seed%211)*.02f,y*spacing+((seed>>8)%211)*.02f};
  float lifetime=7+(seed%51)*.1f,phase=ageClock/lifetime+(seed%997)/997.f,age=phase-std::floor(phase);V pos=origin+flow*((age-.5f)*9);
  if(pos.x<.5f||pos.y<.5f||pos.x>MapSize-1.5f||pos.y>MapSize-1.5f||desertWater(pos,.3f))continue;Vector2 p=screen(pos);
  float length=135+(seed%109),thickness=24+((seed>>10)%24),wave=ageClock*.72f+seed%31,alpha=std::sin(age*PI);alpha=alpha*alpha*(.13f+.28f*gust(ageClock));
  if(p.x<-length||p.x>WorldW+length||p.y<-50||p.y>H+50)continue;
  rlCheckRenderBatchLimit(40);rlSetTexture(sandPlume.id);rlBegin(RL_QUADS);Color color=Fade(WHITE,alpha);rlColor4ub(color.r,color.g,color.b,color.a);
  for(int i=0;i<10;i++){float a=i/10.f,b=(i+1)/10.f;auto point=[&](float u,float v){return Vector2{p.x+(u-.5f)*length,p.y+(u-.5f)*length*.15f+std::sin(u*PI*2+wave)*thickness*.16f+(v-.5f)*thickness};};
   for(auto uv:{Vector2{a,0},Vector2{a,1},Vector2{b,1},Vector2{b,0}}){Vector2 q=point(uv.x,uv.y);rlTexCoord2f(uv.x,uv.y);rlVertex2f(q.x,q.y);}
  }rlEnd();rlSetTexture(0);
 }
 // Sparse saltation: tiny grains bounce a few pixels above their ground path.
 constexpr float grainSpacing=1.65f;
 for(int y=int(std::floor(bounds.minY/grainSpacing));y<=int(std::ceil(bounds.maxY/grainSpacing));y++)for(int x=int(std::floor(bounds.minX/grainSpacing));x<=int(std::ceil(bounds.maxX/grainSpacing));x++){
  unsigned seed=Game::hash(x+708,y+41);if(seed%3==0)continue;V origin{x*grainSpacing+(seed%101)*.012f,y*grainSpacing+((seed>>8)%101)*.012f};
  float lifetime=2.3f+(seed%19)*.1f,phase=ageClock/lifetime+(seed%983)/983.f,age=phase-std::floor(phase);V pos=origin+flow*((age-.5f)*4);
  if(pos.x<.5f||pos.y<.5f||pos.x>MapSize-1.5f||pos.y>MapSize-1.5f||desertWater(pos,.12f))continue;Vector2 p=screen(pos);p.y-=std::abs(std::sin(age*PI*3))*(1+seed%5);
  if(p.x<0||p.x>WorldW||p.y<0||p.y>H)continue;float opacity=std::sin(age*PI)*(.18f+gust(ageClock)*.40f),size=.65f+(seed%4)*.18f;Color color=Fade(seed%2?rgb(249,220,173):rgb(180,130,76),opacity);
  poly({{p.x-size,p.y-.4f},{p.x-size*.6f,p.y+.6f},{p.x+size*1.1f,p.y+.4f},{p.x+size*.8f,p.y-.5f}},color);
 }
 native_world::endBlendMode();
}
