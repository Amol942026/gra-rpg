#pragma once
#include <type_traits>
#include "raymath.h"
#include "sword_pose.hpp"
#include "hero_motion.hpp"
#include "hero_bow_motion.hpp"
#include "hero_quiver.hpp"
#include "hero_sheath.hpp"
#include "shield_stow.hpp"
#include "weapon_profiles.hpp"
#include "weapon_stow.hpp"
#include "spear_motion.hpp"
#include "power_motion.hpp"
#include "hero_weapon_motion.hpp"
#include "hero_knockdown.hpp"
#include "hero_hit_reaction.hpp"
#include "traveler_gestures.hpp"
#include "enemy_death_settle.hpp"
#include "shadow_geometry.hpp"
#include "volcano_actors.hpp"
#include "regional_enemy_art.hpp"
#include "hero_paint.hpp"
#include "creature_paint.hpp"
#include "bandit_gait.hpp"
#include "generated/creature_motion_data.hpp"
#include "generated/cobra_motion_data.hpp"
#include "generated/forest_motion_data.hpp"
#include "generated/wolf_motion_data.hpp"
extern "C" void (*glfwGetProcAddress(const char*))(void);

// Small geometric character rigs: all parts turn in depth, including the face,
// back, weapon and limbs. A back view is not a mirrored front sprite.
namespace actor_art {
inline bool shadowBuilding=false,shadowCull=true,shadowCasting=true,pickingFromPreparedFrame=false;
inline int shadowCacheKey=-1;
inline float shadowElevation=0;
// ClearBackground starts one depth buffer per scene. Disjoint actor ranges
// retain worldDrawOrder while keeping precise self-occlusion inside each body;
// isolated previews use the ordinary clear. Reserve space for uncached NPCs.
inline bool allowSceneDepth=true,sceneDepth=false;
inline int sceneDepthOrder=0;
inline float sceneDepthStep=.01f;
inline RenderTexture2D edgeCoverage{};
inline void initEdgeCoverage(){
 edgeCoverage=LoadRenderTexture(448,352);
 if(edgeCoverage.id)SetTextureFilter(edgeCoverage.texture,TEXTURE_FILTER_BILINEAR);
}
inline void unloadEdgeCoverage(){if(edgeCoverage.id)UnloadRenderTexture(edgeCoverage);edgeCoverage={};}
inline Vector2 reserveSpriteDepth(){
 rlDrawRenderBatchActive();
 if(sceneDepth)return {-.95f+sceneDepthOrder++*sceneDepthStep,sceneDepthStep*.5f};
 static const auto clear=reinterpret_cast<void(*)(unsigned)>(glfwGetProcAddress("glClear"));
 if(clear)clear(0x00000100); // GL_DEPTH_BUFFER_BIT
 return {-.95f,.90f};
}
struct P3 {
    float r, f, z;
    P3 operator+(P3 b) const { return {r+b.r, f+b.f, z+b.z}; }
    P3 operator-(P3 b) const { return {r-b.r, f-b.f, z-b.z}; }
    P3 operator*(float n) const { return {r*n, f*n, z*n}; }
};
struct WeaponPose {P3 hand,elbow,blade;};
struct HeroReaction {float progress=-1;hero_motion::Vec away{};};
inline P3 swordPoint(sword_pose::Point p){return {p.r,p.f,p.z};}
inline WeaponPose forwardStrike(float progress,float lift=0,bool heavy=false,HeroReaction reaction={}){
 (void)lift;auto pose=hero_motion::evaluate(0,0,0,0,progress,heavy);
 if(reaction.progress>=0)hero_hit_reaction::apply(pose,reaction.progress,reaction.away);
 auto hand=pose.world[hero_motion::data::HandR].p,elbow=pose.world[hero_motion::data::ForearmR].p;
 auto v=sword_pose::idleBlade(0);auto blade=pose.vector(hero_motion::data::Weapon,{v.r,v.f,v.z});
 return {{hand.x,hand.y,hand.z},{elbow.x,elbow.y,elbow.z},{blade.x,blade.y,blade.z}};
}

struct Face {
    std::array<Vector2,4> points;
    float depth;
    Color color;
    std::array<Color,4> vertexColors{};
    std::array<float,4> vertexDepth{};
    bool smooth=false;
    unsigned textureId=0;
    std::array<Vector2,4> uv{};
};
inline bool paintCovers(const Face& face,Vector2 uv){
    if(creature_paint::opaqueCobraTexture(face.textureId))return true;
    if(face.textureId==hero_paint::turn::atlas.id)return hero_paint::turn::opaque(uv,face.color.r);
    return face.textureId==creature_paint::atlas.id?creature_paint::opaque(uv):hero_paint::opaque(uv);
}
struct Silhouette {
    static constexpr int Step=2,Width=128,Height=102;
    std::array<unsigned char,Width*Height> covered;
    Vector2 origin{};
    int left=Width,right=-1,top=Height,bottom=-1,count=0;
    bool ready=false;
    Vector2 point(int col,int row)const{return {origin.x+(col+.5f)*Step,origin.y+(row+.5f)*Step};}
    bool has(int col,int row)const{return covered[row*Width+col]!=0;}
    void reset(Vector2 anchor){
        origin={anchor.x-Width*Step*.5f,anchor.y-156.f};
        covered.fill(0);left=Width;right=-1;top=Height;bottom=-1;count=0;ready=true;
    }
    void mark(int col,int row){
        covered[row*Width+col]=1;
        count++;
        left=std::min(left,col);right=std::max(right,col);
        top=std::min(top,row);bottom=std::max(bottom,row);
    }
};
// Reuse one bounded dynamic GPU buffer across live creature draws. The meshes
// still come from the same CPU rig used for picking and physical shadows.
inline bool gpuCreatureDraw=true,gpuHeroDraw=true,fastPacking=true;
inline Mesh creatureMesh{};inline Material creatureMaterial{};inline int creatureCapacity=0;
inline std::vector<float> creatureVertices,creatureUV;
inline std::vector<unsigned char> creatureColors;
inline void unloadCreatureBatch(){
 if(creatureMesh.vaoId)UnloadMesh(creatureMesh);if(creatureMaterial.maps)MemFree(creatureMaterial.maps);
 creatureMesh={};creatureMaterial={};creatureCapacity=0;creatureVertices.clear();creatureUV.clear();creatureColors.clear();
}
inline void prepareCreatureBatch(int count=196608){
 if(count<=creatureCapacity)return;
 if(creatureMesh.vaoId)UnloadMesh(creatureMesh);creatureMesh={};creatureCapacity=((count+4095)/4096)*4096;
 creatureMesh.vertexCount=creatureCapacity;creatureMesh.triangleCount=creatureCapacity/3;
 creatureMesh.vertices=static_cast<float*>(MemAlloc(creatureCapacity*3*sizeof(float)));
 creatureMesh.texcoords=static_cast<float*>(MemAlloc(creatureCapacity*2*sizeof(float)));
 creatureMesh.colors=static_cast<unsigned char*>(MemAlloc(creatureCapacity*4));
 std::fill_n(creatureMesh.vertices,creatureCapacity*3,0.f);std::fill_n(creatureMesh.texcoords,creatureCapacity*2,0.f);std::fill_n(creatureMesh.colors,creatureCapacity*4,255);
 UploadMesh(&creatureMesh,true);
 creatureVertices.resize(creatureCapacity*3);creatureUV.resize(creatureCapacity*2);creatureColors.resize(creatureCapacity*4);
 if(!creatureMaterial.maps)creatureMaterial=LoadMaterialDefault();
}
inline void submitActorBatch(unsigned texture,int count=-1,unsigned char opacity=255){
 if(count<0)count=int(creatureVertices.size()/3);if(!count)return;
 if(count>creatureCapacity)prepareCreatureBatch(count);
 rlDrawRenderBatchActive();
 static const auto stream=reinterpret_cast<void(*)(unsigned,std::ptrdiff_t,const void*,unsigned)>(glfwGetProcAddress("glBufferData"));
 if(stream){
  // Orphan the previous payload. Updating the same live GPU storage for the
  // next creature forces the driver to wait for its preceding draw to finish.
  rlEnableVertexBuffer(creatureMesh.vboId[0]);stream(0x8892,count*3*sizeof(float),creatureVertices.data(),0x88E0);
  rlEnableVertexBuffer(creatureMesh.vboId[1]);stream(0x8892,count*2*sizeof(float),creatureUV.data(),0x88E0);
  rlEnableVertexBuffer(creatureMesh.vboId[3]);stream(0x8892,count*4,creatureColors.data(),0x88E0);rlDisableVertexBuffer();
 }else{
  UpdateMeshBuffer(creatureMesh,0,creatureVertices.data(),count*3*sizeof(float),0);
  UpdateMeshBuffer(creatureMesh,1,creatureUV.data(),count*2*sizeof(float),0);
  UpdateMeshBuffer(creatureMesh,3,creatureColors.data(),count*4,0);
 }
 Mesh drawing=creatureMesh;drawing.vertexCount=count;drawing.triangleCount=count/3;
 Material material=creatureMaterial;material.maps[MATERIAL_MAP_DIFFUSE].texture={texture,1,1,1,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
 if(texture==hero_paint::turn::atlas.id)material.shader=hero_paint::turn::shader;
 material.maps[MATERIAL_MAP_DIFFUSE].color={255,255,255,opacity};
 Matrix identity{};identity.m0=identity.m5=identity.m10=identity.m15=1;DrawMesh(drawing,material,identity);
}
inline void drawCreatureFacesLegacy(const std::vector<Face>& faces,bool paint,float depthBase,float depthScale,unsigned char opacity=255){
 creatureVertices.clear();creatureUV.clear();creatureColors.clear();unsigned texture=rlGetTextureIdDefault();
 for(const auto& f:faces){
  if(bool(f.textureId)!=paint)continue;
  if(f.textureId&&f.textureId!=texture){
   submitActorBatch(texture,int(creatureVertices.size()/3),opacity);
   creatureVertices.clear();creatureUV.clear();creatureColors.clear();texture=f.textureId;
  }
  for(int triangle=0;triangle<2;triangle++){
   int a=0,b=triangle+1,c=triangle+2;const auto& p=f.points;
   if((p[b].x-p[a].x)*(p[c].y-p[a].y)-(p[b].y-p[a].y)*(p[c].x-p[a].x)>0)std::swap(b,c);
   for(int i:{a,b,c}){
    float z=sceneDepth?depthBase+(f.vertexDepth[i]+.5f)*depthScale:f.vertexDepth[i];
    creatureVertices.insert(creatureVertices.end(),{p[i].x,p[i].y,z});
    creatureUV.insert(creatureUV.end(),{f.uv[i].x,f.uv[i].y});Color color=f.smooth?f.vertexColors[i]:f.color;
    creatureColors.insert(creatureColors.end(),{color.r,color.g,color.b,color.a});
   }
  }
 }
 submitActorBatch(texture,int(creatureVertices.size()/3),opacity);
}
inline void drawActorOutlineLegacy(const std::vector<Face>& faces,Color color){
 creatureVertices.clear();creatureUV.clear();creatureColors.clear();
 for(const auto& f:faces)for(int edge=0;edge<4;edge++){
  Vector2 a=f.points[edge],b=f.points[(edge+1)%4],d{b.x-a.x,b.y-a.y};float len=std::hypot(d.x,d.y);if(len<.00001f)continue;
  Vector2 r{-d.y*2/len,d.x*2/len};std::array<Vector2,4> p={{{a.x-r.x,a.y-r.y},{a.x+r.x,a.y+r.y},{b.x+r.x,b.y+r.y},{b.x-r.x,b.y-r.y}}};
  for(int tri=0;tri<2;tri++){int i=0,j=tri+1,k=tri+2;if((p[j].x-p[i].x)*(p[k].y-p[i].y)-(p[j].y-p[i].y)*(p[k].x-p[i].x)>0)std::swap(j,k);
   for(int n:{i,j,k}){creatureVertices.insert(creatureVertices.end(),{p[n].x,p[n].y,0});creatureUV.insert(creatureUV.end(),{0,0});creatureColors.insert(creatureColors.end(),{color.r,color.g,color.b,color.a});}
  }
 }
 submitActorBatch(rlGetTextureIdDefault());
}
inline void beginPacking(int maximum){
 if(maximum>creatureCapacity)prepareCreatureBatch(maximum);
 // Sizes stay at their preloaded capacity during normal rendering. Switching
 // to the reference path in QA may shrink them; restore once, before packing.
 if(creatureVertices.size()<size_t(creatureCapacity)*3)creatureVertices.resize(creatureCapacity*3);
 if(creatureUV.size()<size_t(creatureCapacity)*2)creatureUV.resize(creatureCapacity*2);
 if(creatureColors.size()<size_t(creatureCapacity)*4)creatureColors.resize(creatureCapacity*4);
}
inline void drawCreatureFaces(const std::vector<Face>& faces,bool paint,float depthBase,float depthScale,unsigned char opacity=255){
 if(!fastPacking){drawCreatureFacesLegacy(faces,paint,depthBase,depthScale,opacity);return;}
 beginPacking(int(faces.size())*6);int count=0;unsigned texture=rlGetTextureIdDefault();
 float* vertices=creatureVertices.data();float* uv=creatureUV.data();unsigned char* colors=creatureColors.data();
 for(const auto& f:faces){
  if(bool(f.textureId)!=paint)continue;
  if(f.textureId&&f.textureId!=texture){
   submitActorBatch(texture,count,opacity);count=0;texture=f.textureId;
   vertices=creatureVertices.data();uv=creatureUV.data();colors=creatureColors.data();
  }
  for(int triangle=0;triangle<2;triangle++){
   int a=0,b=triangle+1,c=triangle+2;const auto& p=f.points;
   if((p[b].x-p[a].x)*(p[c].y-p[a].y)-(p[b].y-p[a].y)*(p[c].x-p[a].x)>0)std::swap(b,c);
   for(int i:{a,b,c}){
    float z=sceneDepth?depthBase+(f.vertexDepth[i]+.5f)*depthScale:f.vertexDepth[i];
    *vertices++=p[i].x;*vertices++=p[i].y;*vertices++=z;
    *uv++=f.uv[i].x;*uv++=f.uv[i].y;Color color=f.smooth?f.vertexColors[i]:f.color;
    *colors++=color.r;*colors++=color.g;*colors++=color.b;*colors++=color.a;count++;
   }
  }
 }
 submitActorBatch(texture,count,opacity);
}
inline void drawActorOutline(const std::vector<Face>& faces,Color color){
 if(!fastPacking){drawActorOutlineLegacy(faces,color);return;}
 beginPacking(int(faces.size())*24);int count=0;
 float* vertices=creatureVertices.data();float* uv=creatureUV.data();unsigned char* colors=creatureColors.data();
 for(const auto& f:faces)for(int edge=0;edge<4;edge++){
  Vector2 a=f.points[edge],b=f.points[(edge+1)%4],d{b.x-a.x,b.y-a.y};float len=std::hypot(d.x,d.y);if(len<.00001f)continue;
  Vector2 r{-d.y*2/len,d.x*2/len};std::array<Vector2,4> p={{{a.x-r.x,a.y-r.y},{a.x+r.x,a.y+r.y},{b.x+r.x,b.y+r.y},{b.x-r.x,b.y-r.y}}};
  for(int tri=0;tri<2;tri++){int i=0,j=tri+1,k=tri+2;if((p[j].x-p[i].x)*(p[k].y-p[i].y)-(p[j].y-p[i].y)*(p[k].x-p[i].x)>0)std::swap(j,k);
   for(int n:{i,j,k}){
    *vertices++=p[n].x;*vertices++=p[n].y;*vertices++=0;*uv++=0;*uv++=0;
    *colors++=color.r;*colors++=color.g;*colors++=color.b;*colors++=color.a;count++;
   }
  }
 }
 submitActorBatch(rlGetTextureIdDefault(),count);
}
class Rig {
public:
    using Deform=P3(*)(P3,const void*);
private:
    float x, y, scale, fx, fy, rx, ry;
    float rollAngle=0, tuck=0,runLean=0,runSin=0,runCos=1,bodySin=0,bodyCos=1;
    bool smoothSurfaces=false,bodyRotating=false,softBeams=false,groundBounded=false;
    const hero_motion::Pose* skeleton=nullptr;int skinBone=-1;
    const hero_motion::Transform* attachment=nullptr;
    Deform deform=nullptr;
    const void* deformContext=nullptr;
    unsigned char opacity=255;
    std::vector<Face> faces;
    shadow_geometry::Polygons shadowFaces;
    bool collectShadow=false,closedShadow=false;
    float shadowLift=shadowElevation;
    Vector2 hitLean{};
    Vector2 depthMin{1e9f,1e9f},depthMax{-1e9f,-1e9f};
    bool filterEdges=false;
public:
    Rig(float px,float py,float size,Direction dir,bool collect=false):x(px),y(py),scale(size),collectShadow(collect||(shadowBuilding&&shadowCasting)) {
        setFacing(int(dir)*PI/4);
    }
    void setSkeleton(const hero_motion::Pose* pose){skeleton=pose;skinBone=-1;}
    void reuseStorage(Rig& other){faces.swap(other.faces);faces.clear();shadowFaces.swap(other.shadowFaces);shadowFaces.clear();}
    Vector2 anchor()const{return {x,y};}
    void moveAnchor(float px,float py){
        float dx=px-x,dy=py-y;if(std::abs(dx)+std::abs(dy)<.00001f)return;
        for(auto& face:faces)for(auto& p:face.points){p.x+=dx;p.y+=dy;}
        depthMin.x+=dx;depthMin.y+=dy;depthMax.x+=dx;depthMax.y+=dy;x=px;y=py;
    }
    void fitProjectedBounds(Rectangle body,Rectangle shadow,float amount){
        if(amount<=0)return;
        const auto fitting=[amount](Vector2 low,Vector2 high,Rectangle target){
            const float w=high.x-low.x,h=high.y-low.y;
            if(w<=0||h<=0||target.width<=0||target.height<=0)return std::array<float,4>{1,1,0,0};
            const float sx=1+amount*(std::clamp(target.width/w,.48f,1.8f)-1),sy=1+amount*(std::clamp(target.height/h,.48f,1.8f)-1);
            const float mx=(low.x+high.x)*.5f,my=(low.y+high.y)*.5f;
            return std::array<float,4>{sx,sy,mx+amount*(target.x+target.width*.5f-mx)-sx*mx,
               my+amount*(target.y+target.height*.5f-my)-sy*my};
        };
        const auto apply=[](Vector2& p,const std::array<float,4>& fit){p={p.x*fit[0]+fit[2],p.y*fit[1]+fit[3]};};
        const auto shape=fitting(depthMin,depthMax,body);
        for(auto& face:faces)for(auto& p:face.points)apply(p,shape);
        apply(depthMin,shape);apply(depthMax,shape);
        if(shadowFaces.empty()||shadow.width<=0||shadow.height<=0)return;
        Vector2 low{1e9f,1e9f},high{-1e9f,-1e9f};
        for(const auto& face:shadowFaces)for(auto p:face){
            low.x=std::min(low.x,p.x);low.y=std::min(low.y,p.y);high.x=std::max(high.x,p.x);high.y=std::max(high.y,p.y);
        }
        const auto floor=fitting(low,high,shadow);
        for(auto& face:shadowFaces)for(auto& p:face)apply(p,floor);
    }
    void setBone(int bone){skinBone=bone;}
    void setAttachment(const hero_motion::Transform* transform){attachment=transform;}
    void setDeformation(Deform transform,const void* context=nullptr){deform=transform;deformContext=context;}
    void setVisibility(float value){opacity=static_cast<unsigned char>(std::clamp(value,0.f,1.f)*255+.5f);}
    void setFacing(float angle){
        const auto axis=actorFacing(angle);
        fx=axis.x;fy=axis.y;rx=fy;ry=-fx;
    }
    void setSmoothSurfaces(){smoothSurfaces=true;}
    void setCreatureSurfaces(){smoothSurfaces=true;softBeams=true;}
    void setEdgeCoverage(){filterEdges=true;}
    void setHitLean(Vector2 lean){hitLean=lean;}
    void setGroundBounded(){groundBounded=true;}
    void setRun(float amount){runLean=std::clamp(amount,0.f,1.f)*.12f;runSin=std::sin(runLean);runCos=std::cos(runLean);}
    void setBodyYaw(float angle){bodyRotating=angle!=0;if(bodyRotating){bodySin=std::sin(angle);bodyCos=std::cos(angle);}else{bodySin=0;bodyCos=1;}}
    P3 bodyPoint(P3 p)const{
        if(attachment){
            auto v=attachment->p+hero_motion::rotate(attachment->q,{p.r,p.f,p.z});
            return {v.x,v.y,v.z};
        }
        if(skeleton&&skinBone>=0){auto v=skeleton->point(skinBone,{p.r,p.f,p.z});return {v.x,v.y,v.z};}
        if(skeleton&&skinBone==-2){
            float w=std::clamp((p.z-15)/43.f,0.f,1.f);
            auto v=skeleton->point(hero_motion::data::Pelvis,{p.r,p.f,p.z})*(1-w)+skeleton->point(hero_motion::data::Spine,{p.r,p.f,p.z})*w;
            return {v.x,v.y,v.z};
        }
        if(!bodyRotating)return p;return {p.r*bodyCos+p.f*bodySin,p.f*bodyCos-p.r*bodySin,p.z};
    }
    void setRoll(float progress,float amount) {rollAngle=progress*2*PI;tuck=amount;}
    P3 rollPoint(P3 p) const {
        if(runLean>0&&p.z>35){
            float h=p.z-35,f=p.f;
            p.f=f*runCos+h*runSin;
            p.z=35+h*runCos-f*runSin;
        }
        if(tuck<=0)return p;
        float z=(p.z-38)*(1-.32f*tuck),f=p.f;
        return {p.r*(1-.12f*tuck),f*std::cos(rollAngle)+z*std::sin(rollAngle),
                38-9*tuck+z*std::cos(rollAngle)-f*std::sin(rollAngle)};
    }
    Vector2 project(P3 p) const {
        return {x+(p.r*rx+p.f*fx+p.z*hitLean.x)*scale,y+((p.r*ry+p.f*fy)*ActorGroundSquash-p.z+p.z*hitLean.y)*scale};
    }
    Vector2 groundProject(P3 p) const {
        auto q=shadow_math::ground(x+(p.r*rx+p.f*fx+p.z*hitLean.x)*scale,
                                   y+((p.r*ry+p.f*fy)*.52f+p.z*hitLean.y)*scale+shadowLift,p.z*scale+shadowLift);
        return {q.x,q.y};
    }
    const shadow_geometry::Polygons& shadows()const{return shadowFaces;}
    void drawShadow(float opacity=1)const{shadow_geometry::raster(shadowFaces,opacity);}
    void face(P3 a,P3 b,P3 c,P3 d,Color col,float lighting=1) {
        a=rollPoint(bodyPoint(a));b=rollPoint(bodyPoint(b));c=rollPoint(bodyPoint(c));d=rollPoint(bodyPoint(d));
        if(deform){a=deform(a,deformContext);b=deform(b,deformContext);c=deform(c,deformContext);d=deform(d,deformContext);}
        if(groundBounded){a.z=std::max(.5f,a.z);b.z=std::max(.5f,b.z);c.z=std::max(.5f,c.z);d.z=std::max(.5f,d.z);}
        // Capture every physical surface, including surfaces hidden from the camera.
        if(collectShadow&&col.a>=160){
            std::array<Vector2,4> p={groundProject(a),groundProject(b),groundProject(c),groundProject(d)};
            float sunArea=(p[1].x-p[0].x)*(p[2].y-p[0].y)-(p[1].y-p[0].y)*(p[2].x-p[0].x)
                         +(p[2].x-p[0].x)*(p[3].y-p[0].y)-(p[2].y-p[0].y)*(p[3].x-p[0].x);
            // Closed volumes need only their light-facing skin. Thin sheets stay double sided.
            if(!closedShadow||!shadowCull||sunArea>.001f)shadowFaces.push_back(p);
        }
        Vector2 pa=project(a),pb=project(b),pc=project(c),pd=project(d);
        // Opaque closed meshes need only their camera-facing surfaces. This
        // keeps the richer silhouettes inexpensive and matches pointer picking.
        float area=(pb.x-pa.x)*(pc.y-pa.y)-(pb.y-pa.y)*(pc.x-pa.x)
                  +(pc.x-pa.x)*(pd.y-pa.y)-(pc.y-pa.y)*(pd.x-pa.x);
        if(area<=.001f)return;
        float depth=0;
        for(P3 p:{a,b,c,d})depth+=(p.r*ry+p.f*fy)*.86f+p.z*.52f;
        faces.push_back({{pa,pb,pc,pd},depth*.25f,shade(col,lighting)});
        if(softBeams)for(Vector2 p:{pa,pb,pc,pd}){depthMin.x=std::min(depthMin.x,p.x);depthMin.y=std::min(depthMin.y,p.y);depthMax.x=std::max(depthMax.x,p.x);depthMax.y=std::max(depthMax.y,p.y);}
        if(smoothSurfaces){int i=0;for(P3 p:{a,b,c,d})faces.back().vertexDepth[i++]=-.5f+((p.r*ry+p.f*fy)*.86f+p.z*.52f)*.001f;}

    }
    void surfaceQuad(P3 a,P3 b,P3 c,P3 d,Color ca,Color cb,Color cc,Color cd){
        size_t before=faces.size();face(a,b,c,d,ca);
        if(faces.size()!=before){auto& f=faces.back();f.smooth=true;f.vertexColors={ca,cb,cc,cd};}
    }
    void paintedCreatureQuad(const std::array<P3,4>& points,const std::array<Vector2,4>& uv,
                             const std::array<float,4>& light,int cell,Color fallback){
        const auto texture=creature_paint::texture(cell);
        const Color color=texture?WHITE:fallback;
        const size_t before=faces.size();
        surfaceQuad(points[0],points[1],points[2],points[3],shade(color,light[0]),shade(color,light[1]),
                    shade(color,light[2]),shade(color,light[3]));
        if(!texture||faces.size()==before)return;
        auto& f=faces.back();f.textureId=texture;
        for(int i=0;i<4;i++)f.uv[i]=creature_paint::uv(cell,uv[i].x,uv[i].y);
    }
    void paintedWeaponFace(const std::array<P3,4>& p,const std::array<Vector2,4>& uv,int cell,Color tint){
        size_t before=faces.size();
        face(p[0],p[1],p[2],p[3],tint);
        if(!hero_paint::weaponCellAvailable(cell)||!hero_paint::atlas.id||faces.size()==before)return;
        auto& f=faces.back();f.textureId=hero_paint::atlas.id;
        for(size_t i=0;i<p.size();i++)f.uv[i]=hero_paint::uv(cell,uv[i].x,uv[i].y);
    }
    void materialQuad(P3 a,P3 b,P3 c,P3 d,int cell,float u0,float v0,float u1,float v1,Color tint){
        if(!creature_paint::atlas.id)return;bool saved=collectShadow;collectShadow=false;size_t before=faces.size();
        face(a,b,c,d,tint);collectShadow=saved;
        if(faces.size()!=before){auto& f=faces.back();f.textureId=creature_paint::atlas.id;
            f.uv={creature_paint::uv(cell,u0,v0),creature_paint::uv(cell,u1,v0),creature_paint::uv(cell,u1,v1),creature_paint::uv(cell,u0,v1)};}
    }
    void wrapCreatureFace(int cell,float u0,float v0,float u1,float v1,Color lightA,Color lightB,Color lightC,Color lightD,bool solid=false){
        if(solid){
            // Sample between the first and last texel centers of the atlas cell.
            constexpr float inset=.5f/creature_paint::Tile;
            u0=inset+u0*(1-2*inset);u1=inset+u1*(1-2*inset);
            v0=inset+v0*(1-2*inset);v1=inset+v1*(1-2*inset);
        }else faces.push_back(faces.back());
        auto& painted=faces.back();
        painted.textureId=creature_paint::texture(cell);
        painted.uv={creature_paint::uv(cell,u0,v0),creature_paint::uv(cell,u1,v0),
                    creature_paint::uv(cell,u1,v1),creature_paint::uv(cell,u0,v1)};
        painted.smooth=true;
        painted.vertexColors={lightA,lightB,lightC,lightD};
    }
    // Paint follows the same articulated surfaces and depth test as equipment.
    // Decals add no shadow geometry; their silhouette is already in the body.
    void decal(P3 a,P3 b,P3 c,P3 d,int cell,float u0=0,float v0=0,float u1=1,float v1=1,Color tint=WHITE,bool back=false){
        if(!hero_paint::atlas.id)return;
        bool saved=collectShadow;collectShadow=false;size_t before=faces.size();
        if(back)face(d,c,b,a,tint);else face(a,b,c,d,tint);
        collectShadow=saved;
        if(faces.size()==before)return;
        auto& f=faces.back();f.textureId=hero_paint::atlas.id;
        std::array<Vector2,4> coords={hero_paint::uv(cell,u0,v0),hero_paint::uv(cell,u1,v0),hero_paint::uv(cell,u1,v1),hero_paint::uv(cell,u0,v1)};
        f.uv=back?std::array<Vector2,4>{coords[3],coords[2],coords[1],coords[0]}:coords;
    }
    // The head is a painted cutout. Its pivot and tilt follow the head joint,
    // while the active view follows the direction of the face relative to
    // the camera, including during a backfall or a turn of the shoulders.
    void paintedHead(P3 center,bool flash,bool covered){
        if(collectShadow){
            const auto before=faces.size();
            rounded(center,{18.5f,17.4f,22.4f},WHITE);
            faces.resize(before);
        }
        const auto posed=[&](P3 offset){return rollPoint(bodyPoint(center+offset));};
        const P3 origin=posed({}),across=posed({1,0,0})-origin;
        const P3 forward=posed({0,1,0})-origin,up=posed({0,0,1})-origin;
        const auto depth=[&](P3 p){return (p.r*ry+p.f*fy)*.86f+p.z*.52f;};
        const float sideView=depth(across),frontView=depth(forward);
        const float yaw=std::atan2(-sideView,frontView);
        const bool turning=hero_paint::turn::atlas.id!=0;
        const auto frame=hero_paint::turn::sample(yaw);
        const int view=std::clamp(int(std::round(std::abs(yaw)/(PI*.25f))),0,hero_paint::HeadViews-1);
        const bool mirror=sideView>0&&view!=0&&view!=hero_paint::HeadViews-1;
        const float flat=std::max(.0001f,std::hypot(sideView,frontView));
        const P3 right=across*(frontView/flat)-forward*(sideView/flat);
        const Vector2 pivot=project(origin),upper=project(origin+up),edge=project(origin+right);
        Vector2 vertical{(upper.x-pivot.x)/scale,(upper.y-pivot.y)/scale};
        const float projectedLength=std::hypot(vertical.x,vertical.y);
        if(projectedLength>.08f){
            vertical.x/=projectedLength;vertical.y/=projectedLength;
        }else{
            const float len=std::max(.0001f,std::hypot(edge.x-pivot.x,edge.y-pivot.y));
            vertical={(edge.y-pivot.y)/len,(pivot.x-edge.x)/len};
        }
        const Vector2 horizontal{-vertical.y,vertical.x};
        const float span=covered?22.4f:24.9f;
        const float height=span*(.88f+.12f*std::min(1.10f,projectedLength));
        // The gentle depth crown lets a weapon pass in front or behind the
        // face just as it does at the temples. UV alpha also drives picking.
        constexpr int grid=4;
        struct Vertex {Vector2 point,uv;float distance;};
        std::array<std::array<Vertex,grid+1>,grid+1> vertices{};
        const float base=depth(origin),rise=depth(up);
        for(int row=0;row<=grid;row++)for(int col=0;col<=grid;col++){
            const float u=float(col)/grid;
            // The crown sits inside an open helmet or hood. Keeping the
            // exposed face as one painting preserves eyes, cheeks and ears.
            const float v=(covered?.21f:0.f)+(covered?.79f:1.f)*row/grid;
            const float dx=(u-.5f)*span,dy=(.5f-v)*height;
            const float relief=std::sqrt(std::max(0.f,1.f-4.f*((u-.5f)*(u-.5f)+(v-.5f)*(v-.5f))));
            vertices[row][col]={{pivot.x+scale*(horizontal.x*dx+vertical.x*dy),
                                 pivot.y+scale*(horizontal.y*dx+vertical.y*dy)},
                                turning?hero_paint::turn::uv(frame.index,u,v):hero_paint::headUV(view,mirror?1-u:u,v,flash),
                                base+dy*rise+5.2f*relief};
        }
        for(int row=0;row<grid;row++)for(int col=0;col<grid;col++){
            Face face{};
            const Vertex* corners[]={&vertices[row][col],&vertices[row][col+1],
                                     &vertices[row+1][col+1],&vertices[row+1][col]};
            face.color=turning?Color{frame.blend,static_cast<unsigned char>(flash?255:0),0,255}:(flash?Text:WHITE);
            face.textureId=turning?hero_paint::turn::atlas.id:hero_paint::atlas.id;
            for(int k=0;k<4;k++){
                face.points[k]=corners[k]->point;face.uv[k]=corners[k]->uv;
                face.vertexDepth[k]=-.5f+corners[k]->distance*.001f;
                face.depth+=corners[k]->distance*.25f;
            }
            faces.push_back(face);
        }
    }
    // Continuous skin along a bent path, with shared lighting at each ring.
    void tube(const std::vector<P3>& path,float width,float depth,Color col,int material=-1,bool twoSidedShadow=false,
              const std::vector<float>& ringScale={}){
        if(path.size()<2)return;constexpr int sides=12;
        const bool tapered=ringScale.size()==path.size();
        const bool solid=material==creature_paint::CobraCell&&(creature_paint::cobraSkin.id||creature_paint::solidCobraSkin);
        bool savedClosed=closedShadow;
        // At a tight bend the ring frames can twist a quad across the light.
        closedShadow=softBeams&&!twoSidedShadow;
        int step=softBeams&&scale<1.6f&&!tapered?2:1;
        std::vector<std::array<P3,sides>> points(path.size());std::vector<std::array<Color,sides>> colors(path.size());
        const bool paint=creature_paint::available(material);
        std::vector<std::array<Color,sides>> lights(paint?path.size():0);
        static const auto circle=[](){std::array<P3,sides> result{};for(int i=0;i<sides;i++)result[i]={std::cos(i*2*PI/sides),std::sin(i*2*PI/sides),0};return result;}();
        P3 previousRight{1,0,0};
        for(size_t index=0;index<path.size();index++){
            const size_t j=tapered?path.size()-1-index:index;
            const float radius=tapered?ringScale[j]:1.f;
            P3 axis=path[std::min(j+1,path.size()-1)]-path[j?j-1:0];float len=std::sqrt(axis.r*axis.r+axis.f*axis.f+axis.z*axis.z);axis=axis*(1/std::max(.001f,len));
            float flat=std::hypot(axis.r,axis.f);P3 u=flat>.001f?P3{axis.f/flat,-axis.r/flat,0}:P3{1,0,0};
            if(tapered){
                // Carry the frame from the head towards the tail. Recomputing it
                // from the ground direction flips the skin at a vertical neck.
                const float along=previousRight.r*axis.r+previousRight.f*axis.f+previousRight.z*axis.z;
                const P3 projected=previousRight-axis*along;
                const float length=std::sqrt(projected.r*projected.r+projected.f*projected.f+projected.z*projected.z);
                if(length>.001f)u=projected*(1/length);
                previousRight=u;
            }
            P3 v{axis.f*u.z-axis.z*u.f,axis.z*u.r-axis.r*u.z,axis.r*u.f-axis.f*u.r};
            for(int i=0;i<sides;i+=step){
                P3 normal=u*circle[i].r+v*circle[i].f;
                points[j][i]=path[j]+u*(circle[i].r*width*.5f*radius)+v*(circle[i].f*depth*.5f*radius);
                const float lighting=surfaceLight(normal);
                colors[j][i]=shade(col,lighting);
                if(paint)lights[j][i]=shade(WHITE,lighting);
            }
        }
        for(size_t j=0;j+1<path.size();j++)for(int i=0;i<sides;i+=step){
            int n=(i+step)%sides;
            const size_t before=faces.size();
            surfaceQuad(points[j][i],points[j][n],points[j+1][n],points[j+1][i],colors[j][i],colors[j][n],colors[j+1][n],colors[j+1][i]);
            if(paint&&faces.size()!=before){
                const float column=float((i+sides*3/4)%sides),segments=float(path.size()-1);
                wrapCreatureFace(material,column/sides,float(j)/segments,(column+step)/sides,float(j+1)/segments,
                                 lights[j][i],lights[j][n],lights[j+1][n],lights[j+1][i],solid);
            }
        }
        const P3 cap{width,depth,std::min(width,depth)};
        if(!tapered){
            rounded(path.front(),cap,col,paint?100+material:-1,false,-1,true);
            rounded(path.back(),cap,col,paint?100+material:-1,false,-1,true);
        }
        closedShadow=savedClosed;

    }
    void prism(const std::array<P3,8>& p,Color col) {
        bool savedClosed=closedShadow;closedShadow=true;
        // Bottom, back, front, left, right, top, with a fixed world light.
        face(p[0],p[3],p[2],p[1],col,.6f);
        face(p[0],p[1],p[5],p[4],col,.78f+.12f*fx);
        face(p[3],p[7],p[6],p[2],col,.82f-.12f*fx);
        face(p[0],p[4],p[7],p[3],col,.84f+.13f*rx);
        face(p[1],p[2],p[6],p[5],col,.84f-.13f*rx);
        face(p[4],p[5],p[6],p[7],col,1.13f);

        closedShadow=savedClosed;
    }
    void box(P3 center,P3 size,Color col) {
        float r=size.r/2,f=size.f/2,z=size.z/2;
        prism({center+P3{-r,-f,-z},center+P3{r,-f,-z},center+P3{r,f,-z},center+P3{-r,f,-z},
               center+P3{-r,-f,z},center+P3{r,-f,z},center+P3{r,f,z},center+P3{-r,f,z}},col);
    }
    // Three octagonal rings give bodies curved shoulders and bevelled edges.
    // The fixed vertices avoid per-frame trigonometry for every body part.
    void form(P3 center,P3 size,Color col) {
        if(smoothSurfaces){rounded(center,size,col);return;}
        constexpr float ring[8][2]={{1,0},{.7071f,.7071f},{0,1},{-.7071f,.7071f},
                                   {-1,0},{-.7071f,-.7071f},{0,-1},{.7071f,-.7071f}};
        std::array<std::array<P3,8>,3> p;
        const float level[]={-.5f,.08f,.5f},radius[]={.72f,1.f,.72f};
        for(int j=0;j<3;j++)for(int i=0;i<8;i++)p[j][i]=center+P3{ring[i][0]*size.r*.5f*radius[j],ring[i][1]*size.f*.5f*radius[j],size.z*level[j]};
        for(int j=0;j<2;j++)for(int i=0;i<8;i++){
            int n=(i+1)%8;float normalR=(ring[i][0]+ring[n][0])*.5f,normalF=(ring[i][1]+ring[n][1])*.5f;
            float light=.86f+.18f*(normalR*rx+normalF*fx)+(j?.14f:-.06f);
            face(p[j][i],p[j][n],p[j+1][n],p[j+1][i],col,light);
        }
        for(int i=0;i<8;i+=2){
            face(center+P3{0,0,size.z*.5f},p[2][i],p[2][(i+1)%8],p[2][(i+2)%8],col,1.18f);
            face(center-P3{0,0,size.z*.5f},p[0][(i+2)%8],p[0][(i+1)%8],p[0][i],col,.6f);
        }
    }
    void reserve(size_t count){faces.reserve(count);}
    float surfaceLight(P3 normal)const{
        if(skeleton&&skinBone!=-1){
            int bone=skinBone>=0?skinBone:hero_motion::data::Spine;
            auto n=skeleton->vector(bone,{normal.r,normal.f,normal.z});normal={n.x,n.y,n.z};
        }else if(bodyRotating)normal={normal.r*bodyCos+normal.f*bodySin,normal.f*bodyCos-normal.r*bodySin,normal.z};
        return .83f+.18f*(normal.r*rx+normal.f*fx)+.29f*normal.z+.08f*(normal.r*fx-normal.f*rx);
    }
    // The hero is seen in close-up as well as on the map. Cached sphere normals
    // give rounded silhouettes and smooth shading without per-part trig work.
    template<size_t Sides,size_t Bands>static const auto& unitSphere(){
        static const auto sphere=[](){
            std::array<std::array<P3,Sides>,Bands+1> result{};
            for(size_t j=0;j<=Bands;j++)for(size_t i=0;i<Sides;i++){
                float a=i*2*PI/Sides,b=-PI/2+j*PI/Bands,r=std::max(0.f,std::cos(b));
                result[j][i]={std::cos(a)*r,std::sin(a)*r,std::sin(b)};
            }
            return result;
        }();
        return sphere;
    }
    void rounded(P3 center,P3 size,Color col,int paintCell=-1,bool paintBack=false,int reverseCell=-1,bool wrapPaint=false,bool tintPaint=false){
        bool savedClosed=closedShadow;closedShadow=true;
        auto point=[&](P3 p){return center+P3{p.r*size.r*.5f,p.f*size.f*.5f,p.z*size.z*.5f};};
        auto surface=[&](const auto& sphere){
            // Shared vertex normals remove flat facet bands without tessellating
            // every small animated part. Silhouettes and pointer geometry agree.
            std::array<std::array<Color,std::tuple_size_v<std::decay_t<decltype(sphere[0])>>>,std::tuple_size_v<std::decay_t<decltype(sphere)>>> colors{};
            decltype(colors) paintColors{};
            if(smoothSurfaces)for(size_t j=0;j<sphere.size();j++)for(size_t i=0;i<sphere[0].size();i++){
                P3 n=sphere[j][i];
                if(!(softBeams&&scale<1.6f)){
                    n={n.r/std::max(1.f,size.r),n.f/std::max(1.f,size.f),n.z/std::max(1.f,size.z)};
                    float length=std::sqrt(n.r*n.r+n.f*n.f+n.z*n.z);if(length>0)n=n*(1/length);
                }
                float lighting=surfaceLight(n);colors[j][i]=shade(col,lighting);paintColors[j][i]=shade(tintPaint?col:WHITE,lighting);
            }
            for(size_t j=0;j+1<sphere.size();j++)for(size_t i=0;i<sphere[0].size();i++){
                size_t n=(i+1)%sphere[0].size();P3 normal=(sphere[j][i]+sphere[j][n]+sphere[j+1][n]+sphere[j+1][i])*.25f;
                size_t before=faces.size();face(point(sphere[j][i]),point(sphere[j][n]),point(sphere[j+1][n]),point(sphere[j+1][i]),col,surfaceLight(normal));
                if(smoothSurfaces&&faces.size()!=before){auto& f=faces.back();f.smooth=true;f.vertexColors={colors[j][i],colors[j][n],colors[j+1][n],colors[j+1][i]};}
                // Paint the original front skin, using its exact vertices.
                // No second shell, depth offset or protruding rectangular edge.
                bool reverse=reverseCell>=0&&normal.f<0;
                int cell=reverse?reverseCell:paintCell;
                bool back=reverse||paintBack;
                bool creature=cell>=100;
                unsigned paintTexture=creature?creature_paint::texture(cell-100):hero_paint::atlas.id;
                if(cell>=0&&paintTexture&&(creature||normal.f*(back?-1:1)>.001f)&&faces.size()!=before){
                    // Features and hair paint are transparent. Retain the
                    // original skin/hair under every painted pixel.
                    const bool solidSkin=creature_paint::opaqueCobraTexture(paintTexture);
                    if(!solidSkin)faces.push_back(faces.back());
                    auto& f=faces.back();f.textureId=paintTexture;
                    const P3 unit[]={sphere[j][i],sphere[j][n],sphere[j+1][n],sphere[j+1][i]};
                    for(int k=0;k<4;k++){
                        float u=unit[k].r*(back?-1:1),v=unit[k].z;
                        if(wrapPaint){
                            // Continuous coordinates around the rounded body.
                            // Keep the single wrap seam on the rear of the rig.
                            size_t sides=sphere[0].size();
                            float column=float((i+sides/4)%sides)+(k==1||k==2?1:0);
                            float row=float(j)+(k>=2?1:0);
                            u=2*column/sides-1;v=2*row/(sphere.size()-1)-1;
                        }else if(creature){
                            // Project material marks onto the dominant curved
                            // surface, including flanks and dorsal fur/scales.
                            if(std::abs(normal.z)>std::max(std::abs(normal.r),std::abs(normal.f))){u=unit[k].r;v=unit[k].f;}
                            else if(std::abs(normal.r)>std::abs(normal.f)){u=unit[k].f;v=unit[k].z;}
                        }
                        u=std::clamp((u+1)*.5f,.00390625f,.99609375f);v=std::clamp((1-v)*.5f,.00390625f,.99609375f);
                        if(paintTexture==creature_paint::cobraSkin.id)u=.11f+u*.78f;
                        f.uv[k]=creature?creature_paint::uv(cell-100,u,v):hero_paint::uv(cell,u,v);
                        const Color light[]={paintColors[j][i],paintColors[j][n],paintColors[j+1][n],paintColors[j+1][i]};
                        f.vertexColors[k]=light[k];
                    }
                    f.smooth=true;
                }
            }
        };
        if(softBeams&&scale<1.6f){
            // World-scale limbs cover just a handful of pixels. Preserve the
            // curved, smoothly shaded contour without close-up tessellation.
            float extent=std::max({size.r,size.f,size.z});
            if(extent>70)surface(unitSphere<16,8>());
            else if(extent>24)surface(unitSphere<12,6>());
            else if(extent>10)surface(unitSphere<10,6>());
            else if(extent>7)surface(unitSphere<8,4>());
            else if(extent>3)surface(unitSphere<6,4>());
            else if(extent*scale>=.9f)surface(unitSphere<6,2>());
        }else if(smoothSurfaces){
            float extent=std::max({size.r,size.f,size.z});
            if(extent>70)surface(unitSphere<24,12>());
            else if(extent>24)surface(unitSphere<16,8>());
            else surface(unitSphere<12,6>());
        }else if(scale>=1.8f)surface(unitSphere<24,10>());else surface(unitSphere<16,8>());

        closedShadow=savedClosed;
    }
    void tapered(P3 a,P3 b,float width,float depth,Color col,float tipScale=1,int material=-1){
        P3 axis=b-a;float length=std::sqrt(axis.r*axis.r+axis.f*axis.f+axis.z*axis.z);if(length<.001f)return;
        bool savedClosed=closedShadow;if(softBeams)closedShadow=true;
        axis=axis*(1/length);float flat=std::hypot(axis.r,axis.f);
        P3 u=flat>.001f?P3{axis.f/flat,-axis.r/flat,0}:P3{1,0,0};
        P3 v{axis.f*u.z-axis.z*u.f,axis.z*u.r-axis.r*u.z,axis.r*u.f-axis.f*u.r};
        static const auto ring=[](){std::array<Vector2,16> points{};for(int i=0;i<16;i++)points[i]={std::cos(i*2*PI/16),std::sin(i*2*PI/16)};return points;}();
        auto radial=[&](int i,float scale){return u*(ring[i].x*width*.5f*scale)+v*(ring[i].y*depth*.5f*scale);};
        int step=softBeams&&scale<1.6f?2:1;
        const bool paint=creature_paint::available(material);
        for(int i=0;i<16;i+=step){
            int n=(i+step)%16;P3 normal=u*((ring[i].x+ring[n].x)*.5f)+v*((ring[i].y+ring[n].y)*.5f);
            if(softBeams||paint){
                const float lightA=surfaceLight(u*ring[i].x+v*ring[i].y),lightB=surfaceLight(u*ring[n].x+v*ring[n].y);
                Color ca=shade(col,lightA),cb=shade(col,lightB);
                const size_t before=faces.size();
                surfaceQuad(a+radial(i,1),a+radial(n,1),b+radial(n,tipScale),b+radial(i,tipScale),ca,cb,cb,ca);
                if(paint&&faces.size()!=before){
                    const float column=float((i+12)%16);
                    const Color la=shade(WHITE,lightA),lb=shade(WHITE,lightB);
                    wrapCreatureFace(material,column/16,0,(column+step)/16,1,la,lb,lb,la);
                }
            }else face(a+radial(i,1),a+radial(n,1),b+radial(n,tipScale),b+radial(i,tipScale),col,surfaceLight(normal));
            face(a,a+radial(n,1),a+radial(i,1),a,col,surfaceLight(axis*-1));
            face(b,b+radial(i,tipScale),b+radial(n,tipScale),b,col,surfaceLight(axis));
        }
        closedShadow=savedClosed;
    }
    void spike(P3 base,P3 tip,float width,float depth,Color col) {
        P3 axis=tip-base;float n=std::sqrt(axis.r*axis.r+axis.f*axis.f+axis.z*axis.z);if(n<.001f)return;
        axis=axis*(1/n);float flat=std::hypot(axis.r,axis.f);
        P3 u=flat>.001f?P3{axis.f/flat,-axis.r/flat,0}:P3{1,0,0};
        P3 v{axis.f*u.z-axis.z*u.f,axis.z*u.r-axis.r*u.z,axis.r*u.f-axis.f*u.r};
        u=u*(width*.5f);v=v*(depth*.5f);
        std::array<P3,4> p={base-u-v,base+u-v,base+u+v,base-u+v};
        for(int i=0;i<4;i++)face(p[i],p[(i+1)%4],tip,tip,col,.83f+i*.09f);
        face(p[3],p[2],p[1],p[0],col,.65f);
    }
    void limb(P3 a,P3 b,float width,float depth,Color col) {
        float r=width/2,f=depth/2;
        prism({a+P3{-r,-f,0},a+P3{r,-f,0},a+P3{r,f,0},a+P3{-r,f,0},
               b+P3{-r,-f,0},b+P3{r,-f,0},b+P3{r,f,0},b+P3{-r,f,0}},col);
    }
    // A weapon needs a cross-section perpendicular to its blade, including
    // when the sword becomes almost horizontal during the forward cut.
    void beam(P3 a,P3 b,float width,float depth,Color col) {
        if(smoothSurfaces){tapered(a,b,width,depth,col);return;}
        P3 axis=b-a;float n=std::sqrt(axis.r*axis.r+axis.f*axis.f+axis.z*axis.z);
        if(n<.001f)return;axis=axis*(1/n);
        float flat=std::hypot(axis.r,axis.f);
        P3 u=flat>.001f?P3{axis.f/flat,-axis.r/flat,0}:P3{1,0,0};
        P3 v{axis.f*u.z-axis.z*u.f,axis.z*u.r-axis.r*u.z,axis.r*u.f-axis.f*u.r};
        u=u*(width*.5f);v=v*(depth*.5f);
        prism({a-u-v,a+u-v,a+u+v,a-u+v,b-u-v,b+u-v,b+u+v,b-u+v},col);
    }
    void blob(float width,float height,Color col,float lean=0) {
        constexpr int sides=12;
        const float radii[]={.8f,1.f,.78f,.22f};
        const float levels[]={1, .32f*height,.76f*height,height};
        for(int j=0;j<3;j++)for(int i=0;i<sides;i++){
            float a=i*2*PI/sides,b=(i+1)*2*PI/sides;
            auto point=[&](float angle,int ring){return P3{std::cos(angle)*width*radii[ring],std::sin(angle)*width*radii[ring]+lean*ring/3,levels[ring]};};
            face(point(a,j),point(b,j),point(b,j+1),point(a,j+1),col,.78f+j*.09f+std::cos(a)*.1f);
        }
    }
    bool contains(Vector2 point) const {
        for (const Face& face : faces) {
            const auto& p = face.points;
            if(face.textureId){
                for(int t=0;t<2;t++){
                    int a=0,b=t+1,c=t+2;
                    if(!CheckCollisionPointTriangle(point,p[a],p[b],p[c]))continue;
                    float den=(p[b].y-p[c].y)*(p[a].x-p[c].x)+(p[c].x-p[b].x)*(p[a].y-p[c].y);if(std::abs(den)<.00001f)continue;
                    float u=((p[b].y-p[c].y)*(point.x-p[c].x)+(p[c].x-p[b].x)*(point.y-p[c].y))/den;
                    float v=((p[c].y-p[a].y)*(point.x-p[c].x)+(p[a].x-p[c].x)*(point.y-p[c].y))/den;
                    Vector2 uv{face.uv[a].x*u+face.uv[b].x*v+face.uv[c].x*(1-u-v),face.uv[a].y*u+face.uv[b].y*v+face.uv[c].y*(1-u-v)};
                    if(paintCovers(face,uv))return true;
                }
                continue;
            }
            if (CheckCollisionPointTriangle(point,p[0],p[1],p[2]) ||
                CheckCollisionPointTriangle(point,p[0],p[2],p[3])) return true;
        }
        return false;
    }
    void rasterize(Silhouette& mask)const{
        mask.reset(anchor());
        for(const Face& face:faces){
            if(!face.smooth&&face.color.a<128)continue;
            for(int t=0;t<2;t++){
                const int ib=t+1,ic=t+2;
                const Vector2 a=face.points[0],b=face.points[ib],c=face.points[ic];
                const float bx=b.x-a.x,by=b.y-a.y,cx=c.x-a.x,cy=c.y-a.y;
                const float det=bx*cy-by*cx;
                if(std::abs(det)<.00001f)continue;
                const auto grid=[](float bound,float origin){return (bound-origin)/Silhouette::Step-.5f;};
                const int left=std::max(0,int(std::ceil(grid(std::min({a.x,b.x,c.x}),mask.origin.x))));
                const int right=std::min(Silhouette::Width-1,int(std::floor(grid(std::max({a.x,b.x,c.x}),mask.origin.x))));
                const int top=std::max(0,int(std::ceil(grid(std::min({a.y,b.y,c.y}),mask.origin.y))));
                const int bottom=std::min(Silhouette::Height-1,int(std::floor(grid(std::max({a.y,b.y,c.y}),mask.origin.y))));
                const float inv=1/det;
                for(int row=top;row<=bottom;row++)for(int col=left;col<=right;col++){
                    if(mask.has(col,row))continue;
                    const Vector2 point=mask.point(col,row);
                    const float rx=point.x-a.x,ry=point.y-a.y;
                    const float u=(rx*cy-ry*cx)*inv,v=(bx*ry-by*rx)*inv,w=1-u-v;
                    if(u<0||v<0||w<0)continue;
                    if(face.smooth&&face.vertexColors[0].a*w+face.vertexColors[ib].a*u+face.vertexColors[ic].a*v<128)continue;
                    if(face.textureId){
                        const auto& uv=face.uv;
                        const Vector2 texel{uv[0].x*w+uv[ib].x*u+uv[ic].x*v,uv[0].y*w+uv[ib].y*u+uv[ic].y*v};
                        if(!paintCovers(face,texel))continue;
                    }
                    mask.mark(col,row);
                }
            }
        }
    }
    void clearDepth(void(*clear)(unsigned))const{
        if(!clear)return;
        static const auto get=reinterpret_cast<void(*)(unsigned,int*)>(glfwGetProcAddress("glGetIntegerv"));
        static const auto enabled=reinterpret_cast<unsigned char(*)(unsigned)>(glfwGetProcAddress("glIsEnabled"));
        if(!softBeams||!get||!enabled||depthMax.x<depthMin.x){clear(0x00000100);return;}
        int viewport[4],previous[4];get(0x0BA2,viewport);get(0x0C10,previous);bool clipped=enabled(0x0C11)!=0;
        auto view=rlGetMatrixModelview(),projection=rlGetMatrixProjection();Vector2 low{1e9f,1e9f},high{-1e9f,-1e9f};
        for(float x:{depthMin.x,depthMax.x})for(float y:{depthMin.y,depthMax.y}){
            float vx=view.m0*x+view.m4*y+view.m12,vy=view.m1*x+view.m5*y+view.m13,vz=view.m2*x+view.m6*y+view.m14;
            float w=projection.m3*vx+projection.m7*vy+projection.m11*vz+projection.m15;
            float px=viewport[0]+((projection.m0*vx+projection.m4*vy+projection.m8*vz+projection.m12)/w+1)*viewport[2]*.5f;
            float py=viewport[1]+((projection.m1*vx+projection.m5*vy+projection.m9*vz+projection.m13)/w+1)*viewport[3]*.5f;
            low.x=std::min(low.x,px);low.y=std::min(low.y,py);high.x=std::max(high.x,px);high.y=std::max(high.y,py);
        }
        int left=int(std::floor(low.x))-2,bottom=int(std::floor(low.y))-2,right=int(std::ceil(high.x))+2,top=int(std::ceil(high.y))+2;
        if(clipped){left=std::max(left,previous[0]);bottom=std::max(bottom,previous[1]);right=std::min(right,previous[0]+previous[2]);top=std::min(top,previous[1]+previous[3]);}
        // A creature occupies a small rectangle. Clearing the entire native
        // world depth buffer for each animal consumed integrated-GPU bandwidth.
        rlEnableScissorTest();rlScissor(left,bottom,std::max(0,right-left),std::max(0,top-bottom));clear(0x00000100);
        rlScissor(previous[0],previous[1],previous[2],previous[3]);if(!clipped)rlDisableScissorTest();
    }
    bool drawWithEdgeCoverage(Color outline){
        static const auto get=reinterpret_cast<void(*)(unsigned,int*)>(glfwGetProcAddress("glGetIntegerv"));
        static const auto enabled=reinterpret_cast<unsigned char(*)(unsigned)>(glfwGetProcAddress("glIsEnabled"));
        if(!get||!enabled||!edgeCoverage.id||depthMax.x<depthMin.x)return false;
        const auto projection=rlGetMatrixProjection(),view=rlGetMatrixModelview(),local=rlGetMatrixTransform();
        if(local.m0!=1||local.m5!=1||local.m1!=0||local.m4!=0||local.m12!=0||local.m13!=0||
           view.m1!=0||view.m4!=0||projection.m15!=1)return false;
        int viewport[4];get(0x0BA2,viewport);
        const float px=projection.m0*viewport[2]*.5f,py=-projection.m5*viewport[3]*.5f;
        const float sx=px*view.m0,sy=py*view.m5;
        if(sx<=0||sy<=0)return false;
        const float tx=(1+projection.m12)*viewport[2]*.5f+px*view.m12;
        const float ty=(1-projection.m13)*viewport[3]*.5f+py*view.m13;
        const int margin=outline.a?6:2;
        const float left=std::floor(depthMin.x*sx+tx)-margin,top=std::floor(depthMin.y*sy+ty)-margin;
        const int width=int(std::ceil(depthMax.x*sx+tx)-left)+margin;
        const int height=int(std::ceil(depthMax.y*sy+ty)-top)+margin;
        if(2*width>edgeCoverage.texture.width||2*height>edgeCoverage.texture.height)return false;
        const bool clipped=enabled(0x0C11)!=0,world=native_world::drawingWorld,depth=sceneDepth;
        int scissor[4];if(clipped)get(0x0C10,scissor);
        const unsigned framebuffer=rlGetActiveFramebuffer();
        if(framebuffer==edgeCoverage.id)return false;

        // Resolve four coverage samples over just the cobra's pixel-aligned
        // bounding box. Preserve the world's depth, camera and scissor state.
        rlDrawRenderBatchActive();rlEnableFramebuffer(edgeCoverage.id);
        rlViewport(0,0,width*2,height*2);rlEnableScissorTest();rlScissor(0,0,width*2,height*2);
        ClearBackground(BLANK);
        rlSetMatrixProjection(MatrixOrtho(0,width*2,height*2,0,0,1));
        Matrix sampleView=MatrixScale(sx*2,sy*2,1);
        sampleView.m12=(tx-left)*2;sampleView.m13=(ty-top)*2;
        rlSetMatrixModelview(sampleView);
        native_world::beginOpaqueFrame();sceneDepth=false;
        draw(outline,false);
        rlDrawRenderBatchActive();
        sceneDepth=depth;native_world::drawingWorld=world;
        rlSetMatrixProjection(projection);rlSetMatrixModelview(view);
        rlEnableFramebuffer(framebuffer);rlViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
        if(clipped)rlScissor(scissor[0],scissor[1],scissor[2],scissor[3]);else rlDisableScissorTest();

        BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
        DrawTexturePro(edgeCoverage.texture,{0,0,float(width*2),-float(height*2)},
                       {(left-tx)/sx,(top-ty)/sy,width/sx,height/sy},{},0,WHITE);
        native_world::endBlendMode();
        return true;
    }
    void draw(Color outline=BLANK,bool coverage=true) {
        if(!opacity)return;
        if(coverage&&filterEdges&&scale<1.75f&&drawWithEdgeCoverage(outline))return;
        // In transparent targets material decals must not erase the solid
        // skin's alpha where their texture fades out.
        const bool transparentTarget=smoothSurfaces&&!native_world::drawingWorld;
        if(transparentTarget)native_world::beginSourceOver();
        float depthBase=sceneDepth?-.95f+sceneDepthOrder++*sceneDepthStep:0,depthScale=sceneDepthStep*.5f;
        if(!smoothSurfaces)std::stable_sort(faces.begin(),faces.end(),[](const Face&a,const Face&b){return a.depth<b.depth;});
        // Outline pass goes behind the entire opaque body, so internal edges
        // are covered by the normal rendering instead of becoming a wireframe.
        if(outline.a){if(gpuCreatureDraw&&smoothSurfaces)drawActorOutline(faces,outline);else for(const Face& face:faces)for(int i=0;i<4;i++)
            DrawLineEx(face.points[i],face.points[(i+1)%4],4,outline);}
        // Intersecting organic surfaces need per-pixel depth, not whole-face
        // painter sorting: that sorting exposes teeth along rounded joins.
        if(smoothSurfaces){static const auto clear=reinterpret_cast<void(*)(unsigned int)>(glfwGetProcAddress("glClear"));rlDrawRenderBatchActive();if(!sceneDepth)clearDepth(clear);rlEnableDepthTest();}
        // Finish the opaque skin first. Transparent paint borders must never
        // hide a body part that has not been drawn yet.
        static const auto depthFunction=reinterpret_cast<void(*)(unsigned int)>(glfwGetProcAddress("glDepthFunc"));
        for(int paintPass=0;paintPass<2;paintPass++){
        // Equal depth is intentional for paint on the original hair surface;
        // no geometric extrusion or positional bias is needed.
        if(paintPass&&smoothSurfaces&&depthFunction){rlDrawRenderBatchActive();depthFunction(0x0203);}
        if(smoothSurfaces&&(softBeams||gpuHeroDraw)&&gpuCreatureDraw){drawCreatureFaces(faces,paintPass!=0,depthBase,depthScale,opacity);continue;}
        unsigned activeTexture=0;int remaining=0;
        for(const auto& f:faces){
            if(bool(f.textureId)!=bool(paintPass))continue;
            unsigned texture=f.textureId?f.textureId:rlGetTextureIdDefault();
            // A texture switch opens a default QUADS draw call; changing that
            // to TRIANGLES resets its texture. Establish both states before
            // submitting vertices, including after an automatic buffer flush.
            // Reserve a bounded chunk. Changing GPU state for each tiny face
            // dominated the cost of detailed creatures. Vertex order, depth
            // and material passes stay identical, including on a batch flush.
            if(texture!=activeTexture||remaining==0){
                if(activeTexture)rlEnd();
                if(texture!=activeTexture){
                    if(activeTexture&&activeTexture==hero_paint::turn::atlas.id)EndShaderMode();
                    if(texture==hero_paint::turn::atlas.id)BeginShaderMode(hero_paint::turn::shader);
                }
                rlCheckRenderBatchLimit(256*6);rlSetTexture(texture);rlBegin(RL_TRIANGLES);rlSetTexture(texture);
                activeTexture=texture;remaining=256;
            }
            for(int triangle=0;triangle<2;triangle++){
                int a=0,b=triangle+1,c=triangle+2;const auto& p=f.points;
                if((p[b].x-p[a].x)*(p[c].y-p[a].y)-(p[b].y-p[a].y)*(p[c].x-p[a].x)>0)std::swap(b,c);
                for(int index:{a,b,c}){Color color=f.smooth?f.vertexColors[index]:f.color;rlColor4ub(color.r,color.g,color.b,opacity==255?color.a:color.a*opacity/255);rlTexCoord2f(f.uv[index].x,f.uv[index].y);if(smoothSurfaces)rlVertex3f(p[index].x,p[index].y,sceneDepth?depthBase+(f.vertexDepth[index]+.5f)*depthScale:f.vertexDepth[index]);else rlVertex2f(p[index].x,p[index].y);}
            }remaining--;
        }
        if(activeTexture){rlEnd();if(activeTexture==hero_paint::turn::atlas.id)EndShaderMode();}
        }rlSetTexture(0);
        if(smoothSurfaces){rlDrawRenderBatchActive();if(depthFunction)depthFunction(0x0201);rlDisableDepthTest();}
        if(transparentTarget)EndBlendMode();
    }
};

// The shadow pass builds each animated mesh once. The body pass reuses it.
inline std::vector<std::pair<int,Rig>> frameRigs;
inline std::vector<Rig> recycledRigs;
inline void recycleFrameRigs(){
 // Retain bounded vector storage, not old poses or cached picking results.
 // Both shadow and body passes still use newly evaluated animation each frame.
 for(auto& entry:frameRigs)if(recycledRigs.size()<64)recycledRigs.push_back(std::move(entry.second));
 frameRigs.clear();
}
inline void beginShadowFrame(){recycleFrameRigs();shadowBuilding=true;shadowCacheKey=-1;shadowElevation=0;shadowCasting=true;sceneDepth=false;sceneDepthOrder=0;}
inline void endShadowPreparation(){shadowBuilding=false;shadowCacheKey=-1;sceneDepth=allowSceneDepth;sceneDepthOrder=0;sceneDepthStep=.8f/std::max(8,int(frameRigs.size())+8);}
inline void finishShadowFrame(){recycleFrameRigs();shadowCacheKey=-1;shadowBuilding=false;shadowElevation=0;sceneDepth=false;sceneDepthOrder=0;}
inline void prepareKnockdown(const Game& g,AnimationState& state,float& fallR,float& fallF){
 state.moving=false;state.moveBlend=0;state.stopSprint();state.facingInitialized=true;state.facingAngle=state.targetAngle=g.knockdownFacingAngle();
 fallR=0;fallF=-1;
}
#include "hero_art.hpp"
#include "desert_actors.hpp"
#include "generated/forest_melee_data.hpp"
#include "forest_creatures.hpp"
#include "legacy_death_art.hpp"

// species: player, slime, wolf, skeleton, guardian, boar, spider, bandit, treant.
inline bool draw(int species,float x,float y,float size,const AnimationState& state,
                 float attack=0,bool helmet=false,bool shield=false,Kind weapon=Kind::Sword,bool flash=false,
                 Color outline=BLANK,const Vector2* probe=nullptr,float roll=-1,float swingProgress=-1,float bowDraw=-1,int quiverArrows=3,bool empowered=false,const std::array<Item,6>* equipment=nullptr,float guardianThrow=-1,float knockdown=-1,float fallR=0,float fallF=-1,
                 PowerStyle power=PowerStyle::Count,float powerAge=0,float windup=-1,float enemyMelee=-1,Vector2 hitLean={},
                 Silhouette* fallbackSilhouette=nullptr,HeroReaction reaction={},float special=-1,float bowRelease=-1,float bowPreparation=1.f) {
    if(marshEnemy(species-1)){Enemy e{};e.type=species-1;e.animation=state;e.strike=attack>0?.14f:0;return marsh_art::actor(e,x,y,outline,probe,shadowBuilding,size);}
    if(regionalEnemy(species-1)){
        if(shadowBuilding&&!shadowCasting)return false;
        Enemy e{};e.type=species-1;e.animation=state;e.strike=attack>0?.14f:0;e.flash=flash?.1f:0;
        return regional_enemy_art::draw(e,x,y,outline,probe,shadowBuilding,size);
    }
    if(species>VolcanoFirstType){if(shadowBuilding&&!shadowCasting)return false;return volcano_actors::draw(species-1,x,y,size,state,attack!=0?std::clamp(std::abs(attack)*.7f,0.f,1.f):-1,outline,probe,shadowBuilding,false,flash);}
    HeroAppearance appearance;
    if(species==0){appearance=equipment?heroAppearance(*equipment):legacyHeroAppearance(helmet,shield,weapon);weapon=appearance[0].shape;helmet=appearance[1].present();shield=appearance[2].present();}
    bool rolling=species==0&&roll>=0;
    bool archer=species==0&&weapon==Kind::Bow;
    bool striking=species==0&&weapon!=Kind::None&&!archer&&!rolling&&(swingProgress>=0||attack>0);
    float strikeProgress=swingProgress>=0?swingProgress:std::asin(std::clamp(attack,0.f,1.f))/PI;
    float tuck=rolling?std::clamp(std::min(roll,1-roll)/.18f,0.f,1.f):0;
    tuck=tuck*tuck*(3-2*tuck);
    const float cycle=state.phase*(2*PI);
    const float stride=state.moving?std::sin(cycle):0;
    const float run=species==0&&!rolling&&!striking&&bowDraw<0&&windup<0?std::clamp(state.runBlend,0.f,1.f):0;
    const float lift=state.moving?(1-run)*std::abs(std::cos(cycle))*1.7f+run*(1.5f+.9f*std::cos(cycle*2)):std::sin(state.idleTime*2.4f)*.6f;
    const float bodyScale=species==4?GuardianBodyScale:species==8?treant_gait::Scale:1.f;
    Rig working(x,y,size*bodyScale,state.direction);
    if(state.facingInitialized)working.setFacing(state.facingAngle);
    Rig* cached=nullptr;
    int cacheKey=shadowCacheKey;if(probe&&pickingFromPreparedFrame&&species==0)cacheKey=0;
    if(!shadowBuilding&&(!probe||pickingFromPreparedFrame)&&cacheKey>=0)for(auto& entry:frameRigs)if(entry.first==cacheKey){cached=&entry.second;break;}
    Rig& rig=cached?*cached:working;
    // Shadow preparation uses a flat receiver before the receiver mesh is
    // raised. Reanchor its reused body geometry to the actual ground height.
    if(cached)rig.moveAnchor(x,y);
    if(!cached){
    if(shadowBuilding&&!recycledRigs.empty()){working.reuseStorage(recycledRigs.back());recycledRigs.pop_back();}
    rig.setHitLean(hitLean);

    if(species==0){
        buildHero(rig,lift,stride,cycle,tuck,helmet,shield,weapon,flash,rolling,striking,strikeProgress,bowDraw,quiverArrows,run,empowered,state,roll,appearance,knockdown,fallR,fallF,power,powerAge,windup,reaction,bowRelease,bowPreparation);
    } else if(species>=9){
        buildDesert(rig,species-1,state,attack,flash,special);
    } else {
        buildForestCreature(rig,species-1,state,attack,flash,guardianThrow,enemyMelee);
    }
    }
    if(shadowBuilding&&!probe){
        rig.drawShadow();
        if(shadowCacheKey>=0)frameRigs.emplace_back(shadowCacheKey,std::move(working));
        return false;
    }
    if(probe)return rig.contains(*probe);
    if(fallbackSilhouette&&!cached)rig.rasterize(*fallbackSilhouette);
    float trail=striking&&weapon==Kind::Sword&&power!=PowerStyle::Sword?sword_pose::trailStrength(strikeProgress,empowered):0;
    if(trail>0){
        // One short ribbon follows the exact physical edge during the cut.
        // Windup and recovery do not draw another apparent hit.
        float finish=std::min(strikeProgress,sword_pose::cutEnd(empowered));
        float start=std::max(sword_pose::cutStart(empowered),finish-(empowered?.16f:.13f));
        for(int i=0;i<10;i++){
            float a=start+(finish-start)*i/10,b=start+(finish-start)*(i+1)/10;
            auto first=forwardStrike(a,lift,empowered,reaction),second=forwardStrike(b,lift,empowered,reaction);
            Vector2 innerA=rig.project(first.hand+first.blade*.68f),outerA=rig.project(first.hand+first.blade);
            Vector2 innerB=rig.project(second.hand+second.blade*.68f),outerB=rig.project(second.hand+second.blade);
            float opacity=trail*(.025f+i*.023f)*(empowered?1.2f:1.f);
            poly({innerA,outerA,outerB,innerB},Fade(empowered?rgb(255,155,65):Gold,opacity));
            DrawLineEx(outerA,outerB,(empowered?2.4f:1.f)*size,Fade(Text,opacity));
        }
    }
    rig.draw(outline);
    return false;
}
inline bool drawPlayer(const Game& g,float x,float y,float size=1,const AnimationState* special=nullptr,
                       const Vector2* probe=nullptr,Silhouette* fallbackSilhouette=nullptr){
 AnimationState state=special?*special:g.animation;
 const float swing=special?-1:g.swingProgress(),attack=swing>=0?std::sin(swing*PI):0;
 const float roll=special?-1:g.rollProgress(),fall=special?-1:g.knockdownProgress();
 float fallR=0,fallF=-1;
 if(fall>=0)prepareKnockdown(g,state,fallR,fallF);
 HeroReaction reaction;
 if(!special&&roll<0&&fall<0&&g.hitReactionProgress()>=0){
  const float angle=state.facingInitialized?state.facingAngle:int(state.direction)*AnimationPi/4;
  const auto forward=actorFacing(angle);
  const auto d=g.heroHitDirection;
  const auto away=normalized(V{d.x-d.y,(d.x+d.y)*.5f/ActorGroundSquash});
  const float side=away.x*forward.y-away.y*forward.x;
  reaction={g.hitReactionProgress(),{.35f*side,-1,0}};
 }
 return draw(0,x,y,size,state,attack,!g.equipment[1].empty(),!g.equipment[2].empty(),archetype(g.equipment[0].kind),
             false,BLANK,probe,roll,swing,special?-1:g.visibleBowCharge(),g.ammoCount(),!special&&g.empoweredPose(),
             &g.equipment,-1,fall,fallR,fallF,special?PowerStyle::Count:g.activePowerVisual(),g.powerVisualAge(),
             special?-1:g.powerWindupProgress(),-1,{},fallbackSilhouette,reaction,-1,
             !special&&g.bowReleaseTime>0?1-g.bowReleaseTime/(g.lastBowPower?powerAttack(PowerStyle::Bow).duration:.18f):-1,
             special?-1.f:g.bowNockProgress());
}
template<class Draw> bool drawHitSprite(float y,Vector2 lean,const Vector2* probe,Draw draw){
 if(shadowBuilding||(lean.x==0&&lean.y==0))return draw(probe);
 if(probe){
  const float dy=(probe->y-y)/(1-lean.y);
  const Vector2 source{probe->x+lean.x*dy,y+dy};
  return draw(&source);
 }
 rlPushMatrix();
 const float matrix[]={1,0,0,0,-lean.x,1-lean.y,0,0,0,0,1,0,lean.x*y,lean.y*y,0,1};
 rlMultMatrixf(matrix);
 const bool hit=draw(nullptr);
 rlPopMatrix();
 return hit;
}
inline bool drawEnemy(const Enemy& e,float x,float y,Color outline=BLANK,const Vector2* probe=nullptr){
 const V impulse=e.hitLean();
 const Vector2 lean{impulse.x,impulse.y};
 if(marshEnemy(e.type))return drawHitSprite(y,lean,probe,[&](const Vector2* point){return marsh_art::actor(e,x,y,outline,point,shadowBuilding);});
 if(regionalEnemy(e.type))return drawHitSprite(y,lean,probe,[&](const Vector2* point){
  return regional_enemy_art::draw(e,x,y,outline,point,shadowBuilding);
 });
 if(e.type>=VolcanoFirstType){
  if(shadowBuilding&&!shadowCasting)return false;
  const float action=e.volcanoAction(),special=game.volcanoSpecialPose(e);
  const bool impactFlash=e.type==SlagCrawlerType&&e.flash>EnemyHitRecovery-.045f;
  return drawHitSprite(y,lean,probe,[&](const Vector2* point){
   return volcano_actors::draw(e.type,x,y,1,e.animation,action,outline,point,shadowBuilding,false,impactFlash,special,
                             nullptr,0,1,0,nullptr,volcano_actors::punchTarget(e));
  });
 }
 int previous=shadowCacheKey;if(probe&&pickingFromPreparedFrame)shadowCacheKey=100+e.id;
 bool hit=draw(e.type+1,x,y,1,e.animation,e.attackPose(),false,false,Kind::Sword,false,outline,probe,-1,-1,-1,3,false,nullptr,
               e.boulderProgress(),-1,0,-1,PowerStyle::Count,0,-1,e.type==BoarType?e.boarVisualPhase():e.heavyMeleePhase(),lean,
               nullptr,{},(e.type==CopperScarabType||e.type==SandCobraType)&&e.abilityBusy()?e.abilityAge:-1.f);
 shadowCacheKey=previous;return hit;
}
}
