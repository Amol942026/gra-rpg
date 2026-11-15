#pragma once

namespace forest_cliff {
// The near wall runs below the lowest visible point, even when the hero stands
// right on the rim. Across the valley the rock falls behind the turf silhouette.
inline constexpr std::array<float,8> Drop{0,51,77,96,156,222,362,H+96.f};
inline constexpr std::array<float,8> FrontSpread{0,.30f,.44f,1.18f,1.57f,2.16f,2.35f,2.55f};
inline constexpr std::array<float,8> BackSpread{0,.03f,.06f,.09f,.12f,.15f,.18f,.21f};
inline constexpr float RockTextureHeight=203;
struct Vertex {Vector2 position,uv;Color tint;};
struct Face {std::array<Vertex,4> points;Rectangle bounds;};
inline std::vector<Face> faces;
inline Texture2D rock{};
inline Shader material{};

inline Vertex vertex(float along,int side,size_t ledge){
 const auto& spreads=side<2?BackSpread:FrontSpread;
 const bool corner=along==MapWalkMin||along==MapWalkMax;
 const int neighbor=(side%2==0?1:0)+(along==MapWalkMin?0:2);
 const auto& adjacent=neighbor<2?BackSpread:FrontSpread;
 const float rim=forest_shore::cliffRim(along,side);
 V lip=forest_shore::walkEdgePoint(along,side,-rim);
 V p=forest_shore::walkEdgePoint(along,side,-rim-spreads[ledge]);
 if(corner){
  const float sign=along==MapWalkMin?-1.f:1.f;
  // Both faces use the same lip, shelf and foot at a shared corner.
  if(side%2==0){p.y+=sign*(rim+adjacent[ledge]);lip.y+=sign*rim;}
  else{p.x+=sign*(rim+adjacent[ledge]);lip.x+=sign*rim;}
 }
 const float fold=std::sin(lip.x*.63f+lip.y*.77f)+.38f*std::sin(lip.x*1.81f-lip.y*1.56f);
 const float bench=std::sin(along*.35f+side*1.9f)*std::clamp(std::min(along-MapWalkMin,MapWalkMax-along),0.f,1.f);
 const float warp=spreads[ledge]*(.08f*fold+.06f*bench);
 if(side%2==0)p.x+=(side==0?-warp:warp);else p.y+=(side==1?-warp:warp);
 if(corner){
  const float spread=adjacent[ledge]*.08f*fold*(along==MapWalkMin?-1.f:1.f);
  if(side%2==0)p.y+=spread;else p.x+=spread;
 }
 const float depth=Drop[ledge]+(ledge==0?-4.f:fold*(ledge<4?10.f:18.f));
 const float rise=terrain_height::pixels(WorldLocation::Forest,lip);
 const unsigned char light=static_cast<unsigned char>((side==3?190:224)+13*fold);
 return {{(p.x-p.y)*40,(p.x+p.y)*20-rise+depth},{along*.19f+side*.41f,Drop[ledge]/RockTextureHeight},
         {light,light,light,255}};
}

inline void init(){
 constexpr int segments=192;
 const float span=MapWalkMax-MapWalkMin;
 faces.clear();faces.reserve(4*segments*(Drop.size()-1));
 for(int side=0;side<4;side++){
  std::array<std::array<Vertex,Drop.size()>,segments+1> rim;
  for(int i=0;i<=segments;i++){
   const float along=i==segments?MapWalkMax:MapWalkMin+span*i/segments;
   for(size_t row=0;row<Drop.size();row++)rim[i][row]=vertex(along,side,row);
  }
  for(size_t row=Drop.size()-1;row>0;row--)for(int i=0;i<segments;i++){
   std::array<Vertex,4> quad{rim[i][row-1],rim[i][row],rim[i+1][row],rim[i+1][row-1]};
   auto a=quad[0].position,b=quad[1].position,c=quad[2].position;
   if((b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x)>0)std::swap(quad[1],quad[3]);
   float left=a.x,right=a.x,top=a.y,bottom=a.y;
   for(const auto& v:quad){
    left=std::min(left,v.position.x);right=std::max(right,v.position.x);
    top=std::min(top,v.position.y);bottom=std::max(bottom,v.position.y);
   }
   faces.push_back({quad,{left,top,right-left,bottom-top}});
  }
 }
 rock=terrain_paint::load("caldera-cliff.png",true);
 material=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
out vec4 finalColor;
void main(){
 vec2 p=fragTexCoord;
 float bend=.053*sin(p.x*4.8)+.022*sin(p.x*14.0);
 vec2 grain=vec2(p.x*.77,(p.y+bend)*1.12);
 vec3 base=texture(texture0,grain).rgb;
 vec3 detail=texture(texture0,vec2(p.x*1.71,p.y*2.48)+vec2(.31,.39)).rgb;
 float relief=clamp(dot(base,vec3(.21,.72,.07))*.80+dot(detail,vec3(.21,.72,.07))*.35,0.,1.);
 float steps=p.y*13.+2.4*sin(p.x*2.0)+.66*sin(p.x*8.1+p.y*5.7)+relief*2.5;
 float seam=smoothstep(.71,.96,fract(steps));
 float ribs=.79+.14*sin(p.x*13.9-.62*sin(p.y*6.4))+.07*sin(p.x*25.5);
 vec3 slate=mix(vec3(.118,.154,.145),vec3(.62,.615,.51),relief*.9+.08)*ribs;
 slate*=1.-.39*seam;
 float moss=smoothstep(.49,.75,relief)*(1.-smoothstep(.21,.78,p.y))*(.52+.48*sin(p.x*3.7));
 slate=mix(slate,vec3(.225,.304,.158),moss*.55);
 slate*=mix(fragColor.r,1.,.21);
 slate=mix(slate,vec3(.125,.171,.158),smoothstep(.5,3.2,p.y)*.43);
 finalColor=vec4(slate,fragColor.a);
}
)GLSL");
}

inline void draw(){
 constexpr float margin=34.f;
 const V p=game.player;
 if(std::min({p.x-MapWalkMin,p.y-MapWalkMin,MapWalkMax-p.x,MapWalkMax-p.y})>margin)return;
 const bool textured=rock.id&&IsShaderValid(material)&&material.id!=rlGetShaderIdDefault();
 if(textured)BeginShaderMode(material);
 const Vector2 origin=screenFlat({0,0});
 rlSetTexture(textured?rock.id:rlGetTextureIdDefault());rlBegin(RL_QUADS);
 for(const auto& face:faces){
  const auto& box=face.bounds;
  if(box.x+origin.x>WorldW||box.x+box.width+origin.x<0||box.y+origin.y>H||box.y+box.height+origin.y<0)continue;
  for(const auto& point:face.points){
   const auto tint=point.tint;
   if(textured)rlColor4ub(tint.r,tint.g,tint.b,tint.a);
   else rlColor4ub(tint.r/4,tint.g/3,tint.b/3,tint.a);
   rlTexCoord2f(point.uv.x,point.uv.y);rlVertex2f(point.position.x+origin.x,point.position.y+origin.y);
  }
 }
 rlEnd();rlSetTexture(0);if(textured)EndShaderMode();
}

inline void unload(){
 if(rock.id)UnloadTexture(rock);
 if(IsShaderValid(material)&&material.id!=rlGetShaderIdDefault())UnloadShader(material);
}
}
