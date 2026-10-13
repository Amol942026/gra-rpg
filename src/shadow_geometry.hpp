#pragma once
#include "shadow_math.hpp"
namespace shadow_geometry {
using Polygons=std::vector<std::array<Vector2,4>>;
struct Sprite {RenderTexture2D art{};Vector2 origin{};};
inline bool recording=false;
inline std::vector<float> packed;
inline Mesh gpuMesh{};
inline Material gpuMaterial{};
inline int capacity=0;
inline void beginBatch(){recording=true;packed.clear();}
inline void unloadBatch(){if(gpuMesh.vaoId)UnloadMesh(gpuMesh);if(gpuMaterial.maps)MemFree(gpuMaterial.maps);gpuMesh={};gpuMaterial={};capacity=0;packed.clear();recording=false;}
inline void flushBatch(Shader shader){
 recording=false;if(packed.empty())return;
 int count=int(packed.size()/3);
 if(count>capacity){
  if(gpuMesh.vaoId)UnloadMesh(gpuMesh);gpuMesh={};capacity=std::max(count,48000);
  gpuMesh.vertexCount=capacity;gpuMesh.triangleCount=capacity/3;
  gpuMesh.vertices=static_cast<float*>(MemAlloc(capacity*3*sizeof(float)));
  std::fill(gpuMesh.vertices,gpuMesh.vertices+capacity*3,0.f);UploadMesh(&gpuMesh,true);
 }
 if(!gpuMaterial.maps)gpuMaterial=LoadMaterialDefault();
 rlDrawRenderBatchActive();UpdateMeshBuffer(gpuMesh,0,packed.data(),int(packed.size()*sizeof(float)),0);
 Mesh drawing=gpuMesh;drawing.vertexCount=count;drawing.triangleCount=count/3;
 Material material=gpuMaterial;material.shader=shader;
 Matrix identity{};identity.m0=identity.m5=identity.m10=identity.m15=1;
 DrawMesh(drawing,material,identity);
}
inline void raster(const Polygons& polygons,Vector2 shift={0,0}){
 rlSetTexture(rlGetTextureIdDefault());
 for(const auto& p:polygons)for(size_t j=1;j+1<p.size();j++){
  int a=0,b=int(j),c=int(j+1);
  if((p[b].x-p[a].x)*(p[c].y-p[a].y)-(p[b].y-p[a].y)*(p[c].x-p[a].x)>0)std::swap(b,c);
  if(recording){for(int k:{a,b,c}){packed.push_back(p[k].x+shift.x);packed.push_back(p[k].y+shift.y);packed.push_back(0.f);}}
  else{
   rlCheckRenderBatchLimit(3);rlBegin(RL_TRIANGLES);rlColor4ub(255,255,255,255);
   for(int k:{a,b,c}){rlTexCoord2f(0,0);rlVertex2f(p[k].x+shift.x,p[k].y+shift.y);}rlEnd();
  }
 }rlSetTexture(0);
}
inline Sprite bake(const Polygons& polygons){
 float l=1e6f,t=1e6f,r=-1e6f,b=-1e6f;
 for(const auto& p:polygons)for(Vector2 v:p){l=std::min(l,v.x);t=std::min(t,v.y);r=std::max(r,v.x);b=std::max(b,v.y);}
 if(r<l)return {};
 l=std::floor(l)-2;t=std::floor(t)-2;r=std::ceil(r)+2;b=std::ceil(b)+2;
 Sprite result;result.origin={-l,-t};result.art=LoadRenderTexture(int((r-l)*2),int((b-t)*2));
 BeginTextureMode(result.art);ClearBackground(BLANK);BeginMode2D({{0,0},{0,0},0,2});
 raster(polygons,result.origin);EndMode2D();EndTextureMode();SetTextureFilter(result.art.texture,TEXTURE_FILTER_BILINEAR);return result;
}
inline void draw(const Sprite& sprite,Vector2 base){
 if(!sprite.art.id)return;const auto& tex=sprite.art.texture;
 DrawTexturePro(tex,{0,0,float(tex.width),-float(tex.height)},{base.x-sprite.origin.x,base.y-sprite.origin.y,tex.width*.5f,tex.height*.5f},{0,0},0,WHITE);
}
inline void unload(Sprite& s){if(s.art.id)UnloadRenderTexture(s.art);s={};}
}
