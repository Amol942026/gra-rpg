#pragma once
// Included inside desert_art. Geometry is rasterized once; weather stays in world space.
std::array<RenderTexture2D,4> mountainArt{};Texture2D sandPlume{};RenderTexture2D mountainBackdrop{},mountainForeground{};Shader outsideGroundShader{};Texture2D groundPixel{};int groundOriginLocation=-1;
constexpr float MountainWidth=600,MountainHeight=440,MountainFoot=360,MountainSpacing=4.4f;
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
struct MountainInstance {V pos;float size;int variant;};
const std::vector<MountainInstance>& foothillRanges(){
 static const auto ranges=[](){
  std::vector<MountainInstance> result;
  constexpr float low=-3.f,high=MapSize+4.5f;
  const V corners[]={{low,low},{high,low},{high,high},{low,high}};
  for(int side=0;side<4;side++)for(int i=0;i<13;i++){
   unsigned h=Game::hash(side+421,i+738);V a=corners[side],b=corners[(side+1)%4];
   float t=(i+.12f+(h%71)*.005f)/13.f;
   result.push_back({a+(b-a)*t,.79f+(h%26)*.01f,int((h>>12)%4)});
  }
  std::sort(result.begin(),result.end(),[](const auto& a,const auto& b){return a.pos.x+a.pos.y<b.pos.x+b.pos.y;});
  return result;
 }();return ranges;
}
void makeOutsideGround(){
 const char* fragment=R"GLSL(#version 330
in vec2 fragTexCoord;
out vec4 finalColor;
uniform vec2 worldOrigin;
uniform vec2 viewSize;
float grain(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
void main(){
 vec2 q=fragTexCoord*viewSize;
 vec2 p=worldOrigin+vec2(q.x/80.0+q.y/40.0,-q.x/80.0+q.y/40.0);
 float wave=sin(p.x*.17+p.y*.095)+.38*sin(p.x*.11-p.y*.25);
 float grit=(grain(floor(p*11.0))-.5)*3.0;
 vec3 sand=vec3(211.0+wave*7.0+grit,170.0+wave*6.0+grit,112.0+wave*4.0+grit*.7);
 finalColor=vec4(sand/255.0,1.0);
})GLSL";
 outsideGroundShader=LoadShaderFromMemory(nullptr,fragment);
 groundOriginLocation=GetShaderLocation(outsideGroundShader,"worldOrigin");
 Vector2 dimensions{float(WorldW),float(H)};SetShaderValue(outsideGroundShader,GetShaderLocation(outsideGroundShader,"viewSize"),&dimensions,SHADER_UNIFORM_VEC2);
 Image pixel=GenImageColor(1,1,WHITE);groundPixel=LoadTextureFromImage(pixel);UnloadImage(pixel);
}
void drawOutsideGround(){
 V p=toWorld({0,0});Vector2 origin{p.x,p.y};
 SetShaderValue(outsideGroundShader,groundOriginLocation,&origin,SHADER_UNIFORM_VEC2);
 BeginShaderMode(outsideGroundShader);
 DrawTexturePro(groundPixel,{0,0,1,1},{0,0,WorldW,H},{0,0},0,WHITE);
 EndShaderMode();
}
void drawFoothills(){
 Vector2 hero=screen(game.player);BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
 for(const auto& range:foothillRanges()){
  Vector2 p=screen(range.pos);float size=range.size,left=p.x-MountainWidth*.5f*size,top=p.y-MountainFoot*size;
  if(left+MountainWidth*size<0||left>WorldW||top+MountainHeight*size<0||top>H)continue;
  float opacity=1;
  // A foreground ridge fades where it would cover the controllable hero.
  if(range.pos.x+range.pos.y>game.player.x+game.player.y&&hero.x>left+55*size&&hero.x<left+(MountainWidth-55)*size&&hero.y>top+20*size&&hero.y-85<top+MountainFoot*size)opacity=.34f;
  auto texture=mountainArt[range.variant].texture;
  unsigned char fade=(unsigned char)(opacity*255);
  DrawTexturePro(texture,{0,0,float(texture.width),-float(texture.height)},{left,top,MountainWidth*size,MountainHeight*size},{0,0},0,{fade,fade,fade,fade});
 }EndBlendMode();
}
void drawCachedFoothills(){
 if(!mountainForeground.id){drawFoothills();return;}
 BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);auto texture=mountainForeground.texture;
 DrawTexturePro(texture,{0,0,float(texture.width),-float(texture.height)},{0,0,WorldW,H},{0,0},0,WHITE);EndBlendMode();
}
void makeScenery(){
 makeOutsideGround();
 constexpr int nx=64,ny=48;constexpr float density=1.5f;
 struct Vertex{Vector2 point;float depth;Color color;};
 for(int variant=0;variant<int(mountainArt.size());variant++){
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
  rlSetTexture(0);rlDrawRenderBatchActive();rlDisableDepthTest();EndBlendMode();EndMode2D();EndTextureMode();SetTextureFilter(mountainArt[variant].texture,TEXTURE_FILTER_BILINEAR);
 }
 Image plume=GenImageColor(192,64,BLANK);Color* pixels=(Color*)plume.data;
 for(int y=0;y<plume.height;y++)for(int x=0;x<plume.width;x++){
  float u=(x+.5f)/plume.width,v=(y+.5f)/plume.height,center=.5f+.065f*std::sin(u*PI*3),width=.16f+.12f*std::sin(u*PI),dy=(v-center)/width;
  float envelope=std::pow(std::sin(u*PI),1.35f)*std::exp(-dy*dy*2.5f),billows=.55f+.45f*noise(u*7,v*5,76);
  pixels[y*plume.width+x]=rgb(247,211,157,int(140*envelope*billows));
 }
 sandPlume=LoadTextureFromImage(plume);UnloadImage(plume);SetTextureFilter(sandPlume,TEXTURE_FILTER_BILINEAR);
}
void unloadScenery(){if(mountainForeground.id)UnloadRenderTexture(mountainForeground);mountainForeground={};if(outsideGroundShader.id)UnloadShader(outsideGroundShader);outsideGroundShader={};if(groundPixel.id)UnloadTexture(groundPixel);groundPixel={};if(mountainBackdrop.id)UnloadRenderTexture(mountainBackdrop);mountainBackdrop={};for(auto& texture:mountainArt){if(texture.id)UnloadRenderTexture(texture);texture={};}if(sandPlume.id)UnloadTexture(sandPlume);sandPlume={};}
struct WorldBounds{float minX,maxX,minY,maxY;};
WorldBounds visibleBounds(float margin){
 auto a=toWorld({0,0}),b=toWorld({WorldW,0}),c=toWorld({0,H}),d=toWorld({WorldW,H});
 return {std::min({a.x,b.x,c.x,d.x})-margin,std::max({a.x,b.x,c.x,d.x})+margin,std::min({a.y,b.y,c.y,d.y})-margin,std::max({a.y,b.y,c.y,d.y})+margin};
}
void drawBackdrop(bool mountains=true){
 drawOutsideGround();if(!mountains)return;
 auto bounds=visibleBounds(18);int minX=int(std::floor(bounds.minX/MountainSpacing)),maxX=int(std::ceil(bounds.maxX/MountainSpacing)),minY=int(std::floor(bounds.minY/MountainSpacing)),maxY=int(std::ceil(bounds.maxY/MountainSpacing));
 BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
 // Drawing by world depth joins neighbouring foothills in a continuous belt.
 for(int sum=minX+minY;sum<=maxX+maxY;sum++)for(int x=minX;x<=maxX;x++){
  int y=sum-x;if(y<minY||y>maxY)continue;unsigned seed=Game::hash(x+483,y+815);V pos{x*MountainSpacing+(seed%101)*.009f,y*MountainSpacing+((seed>>8)%101)*.009f};
  float distanceToMap=std::max({.5f-pos.x,.5f-pos.y,pos.x-(MapSize-1.5f),pos.y-(MapSize-1.5f),0.f});
  bool frontSide=pos.x>MapSize-1.5f||pos.y>MapSize-1.5f;
  if(distanceToMap<(frontSide?7.5f:4.2f))continue;
  float size=.88f+(seed%37)*.008f;Vector2 p=screen(pos);float left=p.x-MountainWidth*.5f*size,top=p.y-MountainFoot*size;
  if(left+MountainWidth*size<0||left>WorldW||top+MountainHeight*size<0||top>H)continue;
  const auto& tex=mountainArt[(seed>>13)%mountainArt.size()].texture;
  Color tint=distanceToMap>15?rgb(217,213,203):distanceToMap>7?rgb(238,229,211):WHITE;
  DrawTexturePro(tex,{0,0,float(tex.width),-float(tex.height)},{left,top,MountainWidth*size,MountainHeight*size},{0,0},0,tint);
 }
 EndBlendMode();
}
// A softer distance layer costs far less fill rate than rendering every
// overlapping range at the supersampled resolution of the playable world.
void renderBackdrop(float pixelsPerUnit){
 float resolution=std::clamp(pixelsPerUnit*.72f,.8f,1.2f);int width=int(std::ceil(WorldW*resolution)),height=int(std::ceil(H*resolution));
 if(!mountainBackdrop.id||mountainBackdrop.texture.width!=width||mountainBackdrop.texture.height!=height){
  if(mountainBackdrop.id)UnloadRenderTexture(mountainBackdrop);mountainBackdrop=LoadRenderTexture(width,height);SetTextureFilter(mountainBackdrop.texture,TEXTURE_FILTER_BILINEAR);
 }
 BeginTextureMode(mountainBackdrop);ClearBackground(rgb(211,170,112));BeginMode2D({{0,0},{0,0},0,resolution});drawBackdrop();EndMode2D();EndTextureMode();
 if(!mountainForeground.id||mountainForeground.texture.width!=width||mountainForeground.texture.height!=height){
  if(mountainForeground.id)UnloadRenderTexture(mountainForeground);mountainForeground=LoadRenderTexture(width,height);SetTextureFilter(mountainForeground.texture,TEXTURE_FILTER_BILINEAR);
 }
 BeginTextureMode(mountainForeground);ClearBackground(BLANK);BeginMode2D({{0,0},{0,0},0,resolution});drawFoothills();EndMode2D();EndTextureMode();
}
void drawCachedBackdrop(){
 if(!mountainBackdrop.id){drawBackdrop();return;}auto tex=mountainBackdrop.texture;DrawTexturePro(tex,{0,0,float(tex.width),-float(tex.height)},{0,0,WorldW,H},{0,0},0,WHITE);
}
float gust(float time){return .53f+.22f*std::sin(time*.43f)+.14f*std::sin(time*.19f+1.7f)+.10f*std::sin(time*.83f+.6f);}
void drawSandWind(){
 beginTransparentLayer();
 auto bounds=visibleBounds(9);constexpr V flow{.82f,-.38f};
 // Broad, very thin veils slide over the soil. Birth/death fades are continuous;
 // speed is linear and world anchored, including during sprinting and rolling.
 constexpr float spacing=5.f;
 for(int y=int(std::floor(bounds.minY/spacing));y<=int(std::ceil(bounds.maxY/spacing));y++)for(int x=int(std::floor(bounds.minX/spacing));x<=int(std::ceil(bounds.maxX/spacing));x++){
  unsigned seed=Game::hash(x+156,y+952);V origin{x*spacing+(seed%211)*.02f,y*spacing+((seed>>8)%211)*.02f};
  float lifetime=7+(seed%51)*.1f,phase=game.time/lifetime+(seed%997)/997.f,age=phase-std::floor(phase);V pos=origin+flow*((age-.5f)*9);
  if(pos.x<.5f||pos.y<.5f||pos.x>MapSize-1.5f||pos.y>MapSize-1.5f||desertWater(pos,.3f))continue;Vector2 p=screen(pos);
  float length=135+(seed%109),thickness=24+((seed>>10)%24),wave=game.time*.72f+seed%31,alpha=std::sin(age*PI);alpha=alpha*alpha*(.13f+.28f*gust(game.time));
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
  float lifetime=2.3f+(seed%19)*.1f,phase=game.time/lifetime+(seed%983)/983.f,age=phase-std::floor(phase);V pos=origin+flow*((age-.5f)*4);
  if(pos.x<.5f||pos.y<.5f||pos.x>MapSize-1.5f||pos.y>MapSize-1.5f||desertWater(pos,.12f))continue;Vector2 p=screen(pos);p.y-=std::abs(std::sin(age*PI*3))*(1+seed%5);
  if(p.x<0||p.x>WorldW||p.y<0||p.y>H)continue;float opacity=std::sin(age*PI)*(.18f+gust(game.time)*.40f),size=.65f+(seed%4)*.18f;Color color=Fade(seed%2?rgb(249,220,173):rgb(180,130,76),opacity);
  poly({{p.x-size,p.y-.4f},{p.x-size*.6f,p.y+.6f},{p.x+size*1.1f,p.y+.4f},{p.x+size*.8f,p.y-.5f}},color);
 }
 EndBlendMode();
}
