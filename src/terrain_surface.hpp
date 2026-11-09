#pragma once
// Raised 2D surface using the existing painted ground caches. Heights, UVs and
// normals are prepared once. No new world-sized render target or asset is used.
namespace terrain_surface {
inline constexpr float Min=-4,Step=.5f;
inline constexpr int Cells=(MapSize+8)*2,Nodes=Cells+1;
struct Vertex{V world;Vector2 flat,raised,uv;Color light;};
struct Cell{int index;Rectangle bounds;};
struct Surface{std::vector<Vertex> vertices;std::vector<Cell> cells;unsigned texture=0;float maxRise=0;};
inline std::array<Surface,4> surfaces;
inline Shader lighting{};
inline void ring(V center,float radius,Color color){
 static const auto circle=[](){std::array<V,32> points{};for(int i=0;i<32;i++){float a=i*2*PI/32;points[i]={std::cos(a),std::sin(a)};}return points;}();
 for(int i=0;i<32;i++){Vector2 a=screen(center+circle[i]*radius),b=screen(center+circle[(i+1)%32]*radius);DrawLineEx(a,b,1,color);}
}
inline void init(){
 if(lighting.id)return;
 lighting=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
out vec4 finalColor;
void main(){vec4 c=texture(texture0,fragTexCoord);finalColor=vec4(c.rgb*fragColor.rgb*1.3,c.a*fragColor.a);}
)GLSL");
}
inline void prepare(WorldLocation realm,Texture2D texture){
 init();auto& surface=surfaces[int(realm)];surface.texture=texture.id;
 // The raised mesh extends past the terrain cache. Its transparent border
 // must not wrap to the opposite shore when UVs leave the painted texture.
 SetTextureWrap(texture,TEXTURE_WRAP_CLAMP);
 surface.maxRise=0;surface.vertices.clear();surface.cells.clear();surface.vertices.reserve(Nodes*Nodes);surface.cells.reserve(Cells*Cells);
 for(int y=0;y<Nodes;y++)for(int x=0;x<Nodes;x++){
  V p{Min+x*Step,Min+y*Step};Vector2 flat{TerrainOriginX+(p.x-p.y)*40,90+(p.x+p.y)*20};
  V slope=terrain_height::gradient(realm,p);
  float light=std::clamp(1+.38f*(slope.x*.8f+slope.y*.65f)-.16f*(slope.x*slope.x+slope.y*slope.y),.72f,1.22f);
  unsigned char tint=static_cast<unsigned char>(std::round(light*255/1.3f));
  float rise=terrain_height::pixels(realm,p);
  surface.maxRise=std::max(surface.maxRise,rise);
  surface.vertices.push_back({p,flat,{flat.x,flat.y-rise},{flat.x/TerrainWidth,1-flat.y/TerrainHeight},{tint,tint,tint,255}});
 }
 for(int y=0;y<Cells;y++)for(int x=0;x<Cells;x++){
  int index=y*Nodes+x;float left=1e9f,right=-1e9f,top=1e9f,bottom=-1e9f;
  for(int i:{index,index+Nodes,index+Nodes+1,index+1}){auto p=surface.vertices[i].raised;left=std::min(left,p.x);right=std::max(right,p.x);top=std::min(top,p.y);bottom=std::max(bottom,p.y);}
  surface.cells.push_back({index,{left,top,right-left,bottom-top}});
 }
}
inline void draw(Texture2D texture,bool shadow=false,float maskScale=1,Vector2 maskOffset={}){
 const auto& surface=surfaces[int(game.location)];if(surface.vertices.empty())return;
 Vector2 o=screenFlat({0,0}),offset{o.x-TerrainOriginX,o.y-90};
 if(!shadow)BeginShaderMode(lighting);
 int remaining=0;
 // The raised quads project within this flat screen interval before the
 // exact bounds check. Traversal retains the original back-to-front order.
 for(int row=0;row<Cells;row++){
  float y=Min+row*Step;
  float left=std::max(y-o.x/40-Step,-y-o.y/20-2*Step);
  float right=std::min(y+(WorldW-o.x)/40+Step,-y+(H+surface.maxRise-o.y)/20);
  int start=std::max(0,int(std::floor((left-Min)/Step)));
  int end=std::min(Cells-1,int(std::ceil((right-Min)/Step)));
  for(int column=start;column<=end;column++){
  const auto& cell=surface.cells[row*Cells+column];const auto& b=cell.bounds;
  if(b.x+offset.x>WorldW||b.x+b.width+offset.x<0||b.y+offset.y>H||b.y+b.height+offset.y<0)continue;
  if(remaining==0){rlCheckRenderBatchLimit(256*4);rlSetTexture(texture.id);rlBegin(RL_QUADS);remaining=256;}
  for(int index:{cell.index,cell.index+Nodes,cell.index+Nodes+1,cell.index+1}){
   const auto& v=surface.vertices[index];Color c=shadow?WHITE:v.light;rlColor4ub(c.r,c.g,c.b,c.a);
   Vector2 uv=v.uv;
   if(shadow)uv={(v.flat.x+offset.x+maskOffset.x)*maskScale/texture.width,1-(v.flat.y+offset.y+maskOffset.y)*maskScale/texture.height};
   rlTexCoord2f(uv.x,uv.y);rlVertex2f(v.raised.x+offset.x,v.raised.y+offset.y);
  }
  if(--remaining==0){rlEnd();rlSetTexture(0);}
  }
 }
 if(remaining)rlEnd();rlSetTexture(0);if(!shadow)EndShaderMode();
}
inline void unload(){if(lighting.id)UnloadShader(lighting);lighting={};for(auto& s:surfaces){s.vertices.clear();s.cells.clear();s.texture=0;}}
}
