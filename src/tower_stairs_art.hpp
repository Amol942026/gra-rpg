#pragma once
// Full-size masonry projected with the same 40:20 floor axes as the room.
// Static geometry and exact opaque silhouettes are cached at 2x resolution.
namespace tower_stairs {
std::array<shadow_geometry::Sprite,2> groundShadows;
std::array<shadow_geometry::Polygons,2> shadowMeshes;
std::array<std::vector<V>,2> footprints;
constexpr int ArtWidth=800,ArtHeight=800;constexpr float ArtScale=2,OriginX=174,OriginY=298;
struct P {float x,y,z;};
struct Face {std::vector<Vector2> vertices;float depth;Color color;bool clipped=false;int detail=0;};
struct Mesh {
 float x,y;std::vector<Face> faces;std::vector<Vector2> opening;shadow_geometry::Polygons shadowFaces;std::vector<V> groundVertices{};
 Vector2 project(P p)const{return {x+(p.x-p.y)*40,y+(p.x+p.y)*20-p.z};}
 void face(std::initializer_list<P> points,Color color,bool cut=false,int detail=0){
  Face out;out.color=color;out.clipped=cut;out.detail=detail;out.depth=0;
  for(P p:points){out.vertices.push_back(project(p));out.depth+=(p.x+p.y)*34.6f+p.z*.5f;groundVertices.push_back({p.x,p.y});}
  out.depth/=points.size();faces.push_back(std::move(out));
  if(std::any_of(points.begin(),points.end(),[](P p){return p.z>0;})){
   std::vector<Vector2> ground;for(P p:points){auto q=shadow_math::ground((p.x-p.y)*40,(p.x+p.y)*20,p.z);ground.push_back({q.x,q.y});}
   if(ground.size()==4)shadowFaces.push_back({ground[0],ground[1],ground[2],ground[3]});
   else for(size_t i=1;i+1<ground.size();i++)shadowFaces.push_back({ground[0],ground[i],ground[i+1],ground[i+1]});
  }
 }
 void box(float a,float b,float left,float right,float low,float high,Color stone,bool cut=false,bool joints=false){
  face({{b,left,low},{b,right,low},{b,right,high},{b,left,high}},shade(stone,.71f),cut,joints?2:0);
  face({{a,right,low},{b,right,low},{b,right,high},{a,right,high}},shade(stone,.84f),cut,joints?2:0);
  face({{a,left,high},{b,left,high},{b,right,high},{a,right,high}},shade(stone,1.13f),cut,1);
 }
 void rail(float a,float b,float w,float lowA,float lowB){
  const float half=.105f;
  face({{a,w-half,lowA},{a,w+half,lowA},{a,w+half,lowA+7},{a,w-half,lowA+7}},rgb(109,111,108));
  face({{a,w+half,lowA},{b,w+half,lowB},{b,w+half,lowB+7},{a,w+half,lowA+7}},rgb(145,148,141));
  face({{a,w-half,lowA+7},{b,w-half,lowB+7},{b,w+half,lowB+7},{a,w+half,lowA+7}},rgb(204,204,187),false,1);
  face({{b,w-half,lowB},{b,w+half,lowB},{b,w+half,lowB+7},{b,w-half,lowB+7}},rgb(155,157,148));
 }
 static float cross(Vector2 a,Vector2 b,Vector2 p){return (b.x-a.x)*(p.y-a.y)-(b.y-a.y)*(p.x-a.x);}
 std::vector<Vector2> crop(std::vector<Vector2> subject)const{
  if(opening.empty())return subject;
  float orientation=cross(opening[0],opening[1],opening[2])>=0?1.f:-1.f;
  for(size_t edge=0;edge<opening.size()&&!subject.empty();edge++){
   Vector2 a=opening[edge],b=opening[(edge+1)%opening.size()];std::vector<Vector2> next;
   Vector2 previous=subject.back();float before=cross(a,b,previous)*orientation;
   for(Vector2 current:subject){float now=cross(a,b,current)*orientation;
    if((now>=0)!=(before>=0)){float t=before/(before-now);next.push_back({previous.x+(current.x-previous.x)*t,previous.y+(current.y-previous.y)*t});}
    if(now>=0)next.push_back(current);previous=current;before=now;
   }subject=std::move(next);
  }return subject;
 }
 void segment(Vector2 a,Vector2 b,Color color,float thickness,bool cut)const{
  if(cut){float sign=cross(opening[0],opening[1],opening[2])>=0?1.f:-1.f;
   for(size_t i=0;i<opening.size();i++){
    Vector2 p=opening[i],q=opening[(i+1)%opening.size()];float ca=cross(p,q,a)*sign,cb=cross(p,q,b)*sign;
    if(ca<0&&cb<0)return;if((ca<0)!=(cb<0)){float t=ca/(ca-cb);Vector2 c{a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t};if(ca<0)a=c;else b=c;}
   }
  }DrawLineEx(a,b,thickness,color);
 }
 void draw(){
  std::stable_sort(faces.begin(),faces.end(),[](const Face&a,const Face&b){return a.depth<b.depth;});
  for(const auto& f:faces){auto points=f.clipped?crop(f.vertices):f.vertices;if(points.size()<3)continue;
   for(size_t i=1;i+1<points.size();i++){Vector2 a=points[0],b=points[i],c=points[i+1];if(cross(a,b,c)>0)std::swap(b,c);DrawTriangle(a,b,c,f.color);}
   const auto& v=f.vertices;
   if(f.detail&&v.size()==4){
    segment(v[0],v[1],shade(f.color,1.28f),.9f,f.clipped);
    segment(v[1],v[2],shade(f.color,.63f),.7f,f.clipped);
    // Offset stone joints, worn nosings and fine chips on each tread.
    auto lerp=[](Vector2 a,Vector2 b,float t){return Vector2{a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t};};
    if(f.detail==1){
     for(float t:{.34f,.68f})segment(lerp(v[0],v[3],t),lerp(v[1],v[2],t),shade(f.color,.7f),.65f,f.clipped);
     Vector2 a=lerp(v[0],v[1],.21f),b=lerp(v[0],v[1],.36f);segment(a,b,shade(f.color,.7f),1.3f,f.clipped);
     Vector2 c=lerp(v[0],v[2],.64f);segment(c,{c.x+3,c.y+1.2f},shade(f.color,1.08f),.7f,f.clipped);
    }else for(float t:{.25f,.5f,.75f})segment(lerp(v[0],v[3],t),lerp(v[1],v[2],t),shade(f.color,.7f),.7f,f.clipped);
   }
  }
 }
};
void model(int direction,float x,float y){
 Mesh m{x,y,{},{},{},{}};const float run=TowerStairRun,width=TowerStairWidth*.5f,rise=TowerStairRise,height=rise*TowerStairSteps;
 Color stone=rgb(150,155,153);
 if(direction==1){

  for(int i=0;i<TowerStairSteps;i++){
   float a=run*i/TowerStairSteps,b=run*(i+1)/TowerStairSteps,h=(i+1)*rise;
   m.box(-b,-a,-width,width,0,h,shade(stone,.93f+(Game::hash(i,91)%12)*.009f),false,true);
   // A shallow bevel below the bright stone nosing gives each riser thickness.
   m.box(-a-.025f,-a+.018f,-width-.016f,width+.016f,h-2.4f,h,rgb(189,189,175));
  }
  m.box(-run-.3f,-run,-width,width,height-13,height,rgb(157,162,158),false,true);
  for(int side:{-1,1}){
   float w=side*(width+.1f);
   for(int i:{0,4,8,14}){float t=run*i/TowerStairSteps,z=height*i/TowerStairSteps;
    m.box(-t-.13f,-t+.13f,w-.13f,w+.13f,z,z+34,rgb(132,139,138),false,true);
    m.box(-t-.18f,-t+.18f,w-.18f,w+.18f,z+31,z+36,rgb(183,186,174));
    m.box(-t-.17f,-t+.17f,w-.17f,w+.17f,z,z+5,rgb(113,122,123));
   }m.rail(0,-run,w,34,height+34);
  }
 }else{
  m.opening={m.project({0,-width,0}),m.project({run,-width,0}),m.project({run,width,0}),m.project({0,width,0})};
  poly({m.opening[0],m.opening[1],m.opening[2],m.opening[3]},rgb(12,17,26));
  // The below-floor flight is occluded by the floor opening, as a real shaft is.
  m.face({{0,-width,0},{run,-width,0},{run,-width,-height},{0,-width,-height}},rgb(69,78,85),true,2);
  m.face({{0,-width,0},{0,width,0},{0,width,-height},{0,-width,-height}},rgb(46,56,65),true,2);
  for(int i=TowerStairSteps-1;i>=0;i--){float a=run*i/TowerStairSteps,b=run*(i+1)/TowerStairSteps,h=-i*rise;
   m.box(a,b,-width,width,h-rise,h,shade(stone,1-i*.053f),true,true);
  }
  // Heavy coping and waist-height stone guardrails border the open stairwell.
  for(int side:{-1,1}){
   float w=side*(width+.1f);
   m.box(0,run+.14f,w-.13f,w+.13f,-8,4,rgb(116,127,131),false,true);
   for(float t:{.16f,run*.52f,run}){
    m.box(t-.1f,t+.1f,w-.12f,w+.12f,4,29,rgb(133,144,147),false,true);
    m.box(t-.15f,t+.15f,w-.17f,w+.17f,27,32,rgb(187,193,186));
   }m.rail(.07f,run+.07f,w,29,29);
  }
  m.box(run-.02f,run+.16f,-width-.2f,width+.2f,-8,5,rgb(119,131,134),false,true);
  m.box(run-.04f,run+.18f,-width-.2f,width+.2f,29,36,rgb(190,195,184));
  // Entrance threshold remains open and broad enough for the player's footprint.
  m.box(-.18f,0,-width,width,-5,1,rgb(182,186,176));
 }
 shadowMeshes[direction==1?0:1]=m.shadowFaces;
 footprints[direction==1?0:1]=std::move(m.groundVertices);
 m.draw();
}
std::array<RenderTexture2D,2> art{};std::array<Texture2D,2> outlines{};
std::array<std::vector<unsigned char>,2> masks{};
int index(int direction){return direction==1?0:1;}
void make(){
 for(int dir:{1,-1}){int i=index(dir);art[i]=LoadRenderTexture(ArtWidth,ArtHeight);
  BeginTextureMode(art[i]);ClearBackground(BLANK);BeginMode2D({{0,0},{0,0},0,ArtScale});model(dir,OriginX,OriginY);EndMode2D();EndTextureMode();
  groundShadows[i]=shadow_geometry::bake(shadowMeshes[i]);shadowMeshes[i].clear();
  SetTextureFilter(art[i].texture,TEXTURE_FILTER_BILINEAR);
  Image source=LoadImageFromTexture(art[i].texture);ImageFlipVertical(&source);Color* pixels=LoadImageColors(source);
  masks[i].resize(ArtWidth*ArtHeight);for(int n=0;n<ArtWidth*ArtHeight;n++)masks[i][n]=pixels[n].a>=210?255:0;UnloadImageColors(pixels);UnloadImage(source);
  Image border=GenImageColor(ArtWidth,ArtHeight,BLANK);auto* out=static_cast<Color*>(border.data);
  for(int y=4;y<ArtHeight-4;y++)for(int x=4;x<ArtWidth-4;x++)if(!masks[i][y*ArtWidth+x]){
   bool edge=false;for(int dy=-4;dy<=4&&!edge;dy++)for(int dx=-4;dx<=4;dx++)if(dx*dx+dy*dy<=16&&masks[i][(y+dy)*ArtWidth+x+dx]){edge=true;break;}
   if(edge)out[y*ArtWidth+x]=WHITE;
  }outlines[i]=LoadTextureFromImage(border);SetTextureFilter(outlines[i],TEXTURE_FILTER_BILINEAR);UnloadImage(border);
 }
}
void unload(){for(int i=0;i<2;i++){shadow_geometry::unload(groundShadows[i]);if(art[i].id)UnloadRenderTexture(art[i]);if(outlines[i].id)UnloadTexture(outlines[i]);art[i]={};outlines[i]={};masks[i].clear();}}
bool contains(int direction,Vector2 base,Vector2 point){
 int i=index(direction),x=int(std::floor((point.x-base.x+OriginX)*ArtScale)),y=int(std::floor((point.y-base.y+OriginY)*ArtScale));
 return x>=0&&x<ArtWidth&&y>=0&&y<ArtHeight&&!masks[i].empty()&&masks[i][y*ArtWidth+x];
}
void draw(int direction,Vector2 p,bool hover){
 int i=index(direction);Rectangle source{0,0,float(ArtWidth),-float(ArtHeight)},target{p.x-OriginX,p.y-OriginY,ArtWidth/ArtScale,ArtHeight/ArtScale};
 if(hover)DrawTexturePro(outlines[i],{0,0,float(ArtWidth),float(ArtHeight)},target,{0,0},0,Gold);
 DrawTexturePro(art[i].texture,source,target,{0,0},0,WHITE);
}
}
