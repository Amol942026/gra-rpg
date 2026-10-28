#pragma once
#include <type_traits>
#include "sword_pose.hpp"
#include "hero_motion.hpp"
#include "hero_sheath.hpp"
#include "hero_knockdown.hpp"
#include "shadow_geometry.hpp"
#include "hero_paint.hpp"
#include "creature_paint.hpp"
#include "generated/creature_motion_data.hpp"
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
struct P3 {
    float r, f, z;
    P3 operator+(P3 b) const { return {r+b.r, f+b.f, z+b.z}; }
    P3 operator-(P3 b) const { return {r-b.r, f-b.f, z-b.z}; }
    P3 operator*(float n) const { return {r*n, f*n, z*n}; }
};
struct WeaponPose {P3 hand,elbow,blade;};
inline P3 swordPoint(sword_pose::Point p){return {p.r,p.f,p.z};}
inline WeaponPose forwardStrike(float progress,float lift=0,bool heavy=false){
 (void)lift;auto pose=hero_motion::evaluate(0,0,0,0,progress,heavy);
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
// Reuse one bounded dynamic GPU buffer across live creature draws. The meshes
// still come from the same CPU rig used for picking and physical shadows.
inline bool gpuCreatureDraw=true,gpuHeroDraw=true;
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
 creatureVertices.reserve(creatureCapacity*3);creatureUV.reserve(creatureCapacity*2);creatureColors.reserve(creatureCapacity*4);
 if(!creatureMaterial.maps)creatureMaterial=LoadMaterialDefault();
}
inline void submitActorBatch(unsigned texture){
 int count=int(creatureVertices.size()/3);if(!count)return;
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
 Matrix identity{};identity.m0=identity.m5=identity.m10=identity.m15=1;DrawMesh(drawing,material,identity);
}
inline void drawCreatureFaces(const std::vector<Face>& faces,bool paint,float depthBase,float depthScale){
 creatureVertices.clear();creatureUV.clear();creatureColors.clear();unsigned texture=rlGetTextureIdDefault();
 for(const auto& f:faces){
  if(bool(f.textureId)!=paint)continue;if(f.textureId)texture=f.textureId;
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
 submitActorBatch(texture);
}
inline void drawActorOutline(const std::vector<Face>& faces,Color color){
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
class Rig {
    float x, y, scale, fx, fy, rx, ry;
    float rollAngle=0, tuck=0,runLean=0,runSin=0,runCos=1,bodySin=0,bodyCos=1;
    bool smoothSurfaces=false,bodyRotating=false,softBeams=false,groundBounded=false;
    const hero_motion::Pose* skeleton=nullptr;int skinBone=-1;
    std::vector<Face> faces;
    shadow_geometry::Polygons shadowFaces;
    bool collectShadow=false,closedShadow=false;
    float shadowLift=shadowElevation;
    Vector2 depthMin{1e9f,1e9f},depthMax{-1e9f,-1e9f};
public:
    Rig(float px,float py,float size,Direction dir,bool collect=false):x(px),y(py),scale(size),collectShadow(collect||(shadowBuilding&&shadowCasting)) {
        float angle=int(dir)*PI/4;
        fx=std::cos(angle);fy=std::sin(angle)/.52f;
        float len=std::hypot(fx,fy);fx/=len;fy/=len;rx=fy;ry=-fx;
    }
    void setSkeleton(const hero_motion::Pose* pose){skeleton=pose;skinBone=-1;}
    void reuseStorage(Rig& other){faces.swap(other.faces);faces.clear();shadowFaces.swap(other.shadowFaces);shadowFaces.clear();}
    void setBone(int bone){skinBone=bone;}
    void setFacing(float angle){
        fx=std::cos(angle);fy=std::sin(angle)/.52f;
        float n=std::hypot(fx,fy);fx/=n;fy/=n;rx=fy;ry=-fx;
    }
    void setSmoothSurfaces(){smoothSurfaces=true;}
    void setCreatureSurfaces(){smoothSurfaces=true;softBeams=true;}
    void setGroundBounded(){groundBounded=true;}
    void setRun(float amount){runLean=std::clamp(amount,0.f,1.f)*.12f;runSin=std::sin(runLean);runCos=std::cos(runLean);}
    void setBodyYaw(float angle){bodyRotating=angle!=0;if(bodyRotating){bodySin=std::sin(angle);bodyCos=std::cos(angle);}else{bodySin=0;bodyCos=1;}}
    P3 bodyPoint(P3 p)const{
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
        return {x+(p.r*rx+p.f*fx)*scale,y+((p.r*ry+p.f*fy)*.52f-p.z)*scale};
    }
    Vector2 groundProject(P3 p) const {
        auto q=shadow_math::ground(x+(p.r*rx+p.f*fx)*scale,y+(p.r*ry+p.f*fy)*.52f*scale+shadowLift,p.z*scale+shadowLift);
        return {q.x,q.y};
    }
    const shadow_geometry::Polygons& shadows()const{return shadowFaces;}
    void drawShadow()const{shadow_geometry::raster(shadowFaces);}
    void face(P3 a,P3 b,P3 c,P3 d,Color col,float lighting=1) {
        a=rollPoint(bodyPoint(a));b=rollPoint(bodyPoint(b));c=rollPoint(bodyPoint(c));d=rollPoint(bodyPoint(d));
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
    void materialQuad(P3 a,P3 b,P3 c,P3 d,int cell,float u0,float v0,float u1,float v1,Color tint){
        if(!creature_paint::atlas.id)return;bool saved=collectShadow;collectShadow=false;size_t before=faces.size();
        face(a,b,c,d,tint);collectShadow=saved;
        if(faces.size()!=before){auto& f=faces.back();f.textureId=creature_paint::atlas.id;
            f.uv={creature_paint::uv(cell,u0,v0),creature_paint::uv(cell,u1,v0),creature_paint::uv(cell,u1,v1),creature_paint::uv(cell,u0,v1)};}
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
    // Continuous skin along a bent path, with shared lighting at each ring.
    void tube(const std::vector<P3>& path,float width,float depth,Color col){
        if(path.size()<2)return;constexpr int sides=12;
        bool savedClosed=closedShadow;if(softBeams)closedShadow=true;
        int step=softBeams&&scale<1.6f?2:1;
        std::vector<std::array<P3,sides>> points(path.size());std::vector<std::array<Color,sides>> colors(path.size());
        static const auto circle=[](){std::array<P3,sides> result{};for(int i=0;i<sides;i++)result[i]={std::cos(i*2*PI/sides),std::sin(i*2*PI/sides),0};return result;}();
        for(size_t j=0;j<path.size();j++){
            P3 axis=path[std::min(j+1,path.size()-1)]-path[j?j-1:0];float len=std::sqrt(axis.r*axis.r+axis.f*axis.f+axis.z*axis.z);axis=axis*(1/std::max(.001f,len));
            float flat=std::hypot(axis.r,axis.f);P3 u=flat>.001f?P3{axis.f/flat,-axis.r/flat,0}:P3{1,0,0};
            P3 v{axis.f*u.z-axis.z*u.f,axis.z*u.r-axis.r*u.z,axis.r*u.f-axis.f*u.r};
            for(int i=0;i<sides;i+=step){P3 normal=u*circle[i].r+v*circle[i].f;points[j][i]=path[j]+u*(circle[i].r*width*.5f)+v*(circle[i].f*depth*.5f);colors[j][i]=shade(col,surfaceLight(normal));}
        }
        for(size_t j=0;j+1<path.size();j++)for(int i=0;i<sides;i+=step){int n=(i+step)%sides;surfaceQuad(points[j][i],points[j][n],points[j+1][n],points[j+1][i],colors[j][i],colors[j][n],colors[j+1][n],colors[j+1][i]);}
        rounded(path.front(),{width,depth,std::min(width,depth)},col);rounded(path.back(),{width,depth,std::min(width,depth)},col);
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
    void rounded(P3 center,P3 size,Color col,int paintCell=-1,bool paintBack=false){
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
                float lighting=surfaceLight(n);colors[j][i]=shade(col,lighting);paintColors[j][i]=shade(WHITE,lighting);
            }
            for(size_t j=0;j+1<sphere.size();j++)for(size_t i=0;i<sphere[0].size();i++){
                size_t n=(i+1)%sphere[0].size();P3 normal=(sphere[j][i]+sphere[j][n]+sphere[j+1][n]+sphere[j+1][i])*.25f;
                size_t before=faces.size();face(point(sphere[j][i]),point(sphere[j][n]),point(sphere[j+1][n]),point(sphere[j+1][i]),col,surfaceLight(normal));
                if(smoothSurfaces&&faces.size()!=before){auto& f=faces.back();f.smooth=true;f.vertexColors={colors[j][i],colors[j][n],colors[j+1][n],colors[j+1][i]};}
                // Paint the original front skin, using its exact vertices.
                // No second shell, depth offset or protruding rectangular edge.
                bool creature=paintCell>=100;
                unsigned paintTexture=creature?creature_paint::atlas.id:hero_paint::atlas.id;
                if(paintCell>=0&&paintTexture&&(creature||normal.f*(paintBack?-1:1)>.001f)&&faces.size()!=before){
                    // Features and hair paint are transparent. Retain the
                    // original skin/hair under every painted pixel.
                    faces.push_back(faces.back());
                    auto& f=faces.back();f.textureId=paintTexture;
                    const P3 unit[]={sphere[j][i],sphere[j][n],sphere[j+1][n],sphere[j+1][i]};
                    for(int k=0;k<4;k++){
                        float u=unit[k].r*(paintBack?-1:1),v=unit[k].z;
                        if(creature){
                            // Project material marks onto the dominant curved
                            // surface, including flanks and dorsal fur/scales.
                            if(std::abs(normal.z)>std::max(std::abs(normal.r),std::abs(normal.f))){u=unit[k].r;v=unit[k].f;}
                            else if(std::abs(normal.r)>std::abs(normal.f)){u=unit[k].f;v=unit[k].z;}
                        }
                        u=std::clamp((u+1)*.5f,.00390625f,.99609375f);v=std::clamp((1-v)*.5f,.00390625f,.99609375f);
                        f.uv[k]=creature?creature_paint::uv(paintCell-100,u,v):hero_paint::uv(paintCell,u,v);
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
    void tapered(P3 a,P3 b,float width,float depth,Color col,float tipScale=1){
        P3 axis=b-a;float length=std::sqrt(axis.r*axis.r+axis.f*axis.f+axis.z*axis.z);if(length<.001f)return;
        bool savedClosed=closedShadow;if(softBeams)closedShadow=true;
        axis=axis*(1/length);float flat=std::hypot(axis.r,axis.f);
        P3 u=flat>.001f?P3{axis.f/flat,-axis.r/flat,0}:P3{1,0,0};
        P3 v{axis.f*u.z-axis.z*u.f,axis.z*u.r-axis.r*u.z,axis.r*u.f-axis.f*u.r};
        static const auto ring=[](){std::array<Vector2,16> points{};for(int i=0;i<16;i++)points[i]={std::cos(i*2*PI/16),std::sin(i*2*PI/16)};return points;}();
        auto radial=[&](int i,float scale){return u*(ring[i].x*width*.5f*scale)+v*(ring[i].y*depth*.5f*scale);};
        int step=softBeams&&scale<1.6f?2:1;
        for(int i=0;i<16;i+=step){
            int n=(i+step)%16;P3 normal=u*((ring[i].x+ring[n].x)*.5f)+v*((ring[i].y+ring[n].y)*.5f);
            if(softBeams){
                Color ca=shade(col,surfaceLight(u*ring[i].x+v*ring[i].y)),cb=shade(col,surfaceLight(u*ring[n].x+v*ring[n].y));
                surfaceQuad(a+radial(i,1),a+radial(n,1),b+radial(n,tipScale),b+radial(i,tipScale),ca,cb,cb,ca);
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
                    if((face.textureId==creature_paint::atlas.id?creature_paint::opaque(uv):hero_paint::opaque(uv)))return true;
                }
                continue;
            }
            if (CheckCollisionPointTriangle(point,p[0],p[1],p[2]) ||
                CheckCollisionPointTriangle(point,p[0],p[2],p[3])) return true;
        }
        return false;
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
    void draw(Color outline=BLANK) {
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
        if(smoothSurfaces&&(softBeams||gpuHeroDraw)&&gpuCreatureDraw){drawCreatureFaces(faces,paintPass!=0,depthBase,depthScale);continue;}
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
                rlCheckRenderBatchLimit(256*6);rlSetTexture(texture);rlBegin(RL_TRIANGLES);rlSetTexture(texture);
                activeTexture=texture;remaining=256;
            }
            for(int triangle=0;triangle<2;triangle++){
                int a=0,b=triangle+1,c=triangle+2;const auto& p=f.points;
                if((p[b].x-p[a].x)*(p[c].y-p[a].y)-(p[b].y-p[a].y)*(p[c].x-p[a].x)>0)std::swap(b,c);
                for(int index:{a,b,c}){Color color=f.smooth?f.vertexColors[index]:f.color;rlColor4ub(color.r,color.g,color.b,color.a);rlTexCoord2f(f.uv[index].x,f.uv[index].y);if(smoothSurfaces)rlVertex3f(p[index].x,p[index].y,sceneDepth?depthBase+(f.vertexDepth[index]+.5f)*depthScale:f.vertexDepth[index]);else rlVertex2f(p[index].x,p[index].y);}
            }remaining--;
        }if(activeTexture)rlEnd();}rlSetTexture(0);
        if(smoothSurfaces){rlDrawRenderBatchActive();if(depthFunction)depthFunction(0x0201);rlDisableDepthTest();}
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
#include "forest_creatures.hpp"

// species: player, slime, wolf, skeleton, guardian, boar, spider, bandit, treant.
inline bool draw(int species,float x,float y,float size,const AnimationState& state,
                 float attack=0,bool helmet=false,bool shield=false,Kind weapon=Kind::Sword,bool flash=false,
                 Color outline=BLANK,const Vector2* probe=nullptr,float roll=-1,float swingProgress=-1,float bowDraw=-1,int quiverArrows=3,bool empowered=false,const std::array<Item,6>* equipment=nullptr,float guardianThrow=-1,float knockdown=-1,float fallR=0,float fallF=-1) {
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
    const float run=species==0&&!rolling&&!striking&&bowDraw<0?std::clamp(state.runBlend,0.f,1.f):0;
    const float lift=state.moving?(1-run)*std::abs(std::cos(cycle))*1.7f+run*(1.5f+.9f*std::cos(cycle*2)):std::sin(state.idleTime*2.4f)*.6f;
    const float bodyScale=species==4?GuardianBodyScale:species==8?1.18f:1.f;
    Rig working(x,y,size*bodyScale,state.direction);
    if(state.facingInitialized)working.setFacing(state.facingAngle);
    Rig* cached=nullptr;
    int cacheKey=shadowCacheKey;if(probe&&pickingFromPreparedFrame&&species==0)cacheKey=0;
    if(!shadowBuilding&&(!probe||pickingFromPreparedFrame)&&cacheKey>=0)for(auto& entry:frameRigs)if(entry.first==cacheKey){cached=&entry.second;break;}
    Rig& rig=cached?*cached:working;
    if(!cached){
    if(shadowBuilding&&!recycledRigs.empty()){working.reuseStorage(recycledRigs.back());recycledRigs.pop_back();}

    if(species==0){
        buildHero(rig,lift,stride,cycle,tuck,helmet,shield,weapon,flash,rolling,striking,strikeProgress,bowDraw,quiverArrows,run,empowered,state,roll,appearance,knockdown,fallR,fallF);
    } else if(species>=9){
        buildDesert(rig,species-1,state,attack,flash);
    } else {
        buildForestCreature(rig,species-1,state,attack,flash,guardianThrow);
    }
    }
    if(shadowBuilding&&!probe){
        rig.drawShadow();
        if(shadowCacheKey>=0)frameRigs.emplace_back(shadowCacheKey,std::move(working));
        return false;
    }
    if(probe)return rig.contains(*probe);
    float trail=striking?sword_pose::trailStrength(strikeProgress,empowered):0;
    if(trail>0){
        // One short ribbon follows the exact physical edge during the cut.
        // Windup and recovery do not draw another apparent hit.
        float finish=std::min(strikeProgress,sword_pose::cutEnd(empowered));
        float start=std::max(sword_pose::cutStart(empowered),finish-(empowered?.16f:.13f));
        float bladeScale=weapon==Kind::Dagger?.65f:1.f;
        for(int i=0;i<10;i++){
            float a=start+(finish-start)*i/10,b=start+(finish-start)*(i+1)/10;
            auto first=forwardStrike(a,lift,empowered),second=forwardStrike(b,lift,empowered);
            Vector2 innerA=rig.project(first.hand+first.blade*(.68f*bladeScale)),outerA=rig.project(first.hand+first.blade*bladeScale);
            Vector2 innerB=rig.project(second.hand+second.blade*(.68f*bladeScale)),outerB=rig.project(second.hand+second.blade*bladeScale);
            float opacity=trail*(.025f+i*.023f)*(empowered?1.2f:1.f);
            poly({innerA,outerA,outerB,innerB},Fade(empowered?rgb(255,155,65):Gold,opacity));
            DrawLineEx(outerA,outerB,(empowered?2.4f:1.f)*size,Fade(Text,opacity));
        }
    }
    rig.draw(outline);
    return false;
}
inline bool drawEnemy(const Enemy& e,float x,float y,Color outline=BLANK,const Vector2* probe=nullptr){
 int previous=shadowCacheKey;if(probe&&pickingFromPreparedFrame)shadowCacheKey=100+e.id;
 bool hit=draw(e.type+1,x,y,1,e.animation,e.attackPose(),false,false,Kind::Sword,e.flash>0,outline,probe,-1,-1,-1,3,false,nullptr,e.boulderProgress());shadowCacheKey=previous;return hit;
}
}
