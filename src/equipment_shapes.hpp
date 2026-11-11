#pragma once
#include <array>
#include <cstddef>

namespace ember::equipment_shape {
struct Point {float x,y;};
// The authored 2D profiles used by the existing inventory illustrations
// (blender_equipment_icons.py). Materials vary per Kind; silhouettes vary by
// archetype. Keep the worn equipment on this same visual contract.
inline constexpr std::array<Point,7> Shield{{{-1.15f,1.35f},{1.15f,1.35f},{1.28f,.2f},{.82f,-.9f},{0,-1.75f},{-.82f,-.9f},{-1.28f,.2f}}};
inline constexpr std::array<Point,9> Helmet{{{-1.25f,-.65f},{-1.1f,.65f},{-.7f,1.38f},{0,1.65f},{.7f,1.38f},{1.1f,.65f},{1.25f,-.65f},{.8f,-1.35f},{-.8f,-1.35f}}};
inline constexpr std::array<Point,6> Visor{{{-1.02f,-.2f},{-.86f,.28f},{.86f,.28f},{1.02f,-.2f},{.65f,-.58f},{-.65f,-.58f}}};
inline constexpr std::array<Point,5> Sword{{{-.18f,-.65f},{-.27f,1.4f},{0,1.9f},{.27f,1.4f},{.18f,-.65f}}};
inline constexpr std::array<Point,5> Dagger{{{-.18f,-.65f},{-.27f,.75f},{0,1.25f},{.27f,.75f},{.18f,-.65f}}};
inline constexpr std::array<Point,10> Axe{{{-.1f,.66f},{-.45f,.82f},{-1.1f,.5f},{-1.35f,.9f},{-1.35f,1.6f},{-.7f,1.85f},{-.15f,1.55f},{.48f,1.62f},{.7f,1.25f},{.44f,.86f}}};
inline constexpr std::array<Point,11> Cuirass{{{-.62f,1.35f},{-1.35f,.94f},{-1.14f,.1f},{-.78f,-.4f},{-.88f,-1.3f},{0,-1.6f},{.88f,-1.3f},{.78f,-.4f},{1.14f,.1f},{1.35f,.94f},{.62f,1.35f}}};
inline constexpr std::array<Point,4> Shoulder{{{-.4f,.52f},{.4f,.4f},{.66f,-.23f},{-.02f,-.42f}}};
inline constexpr std::array<Point,6> Pendant{{{-.52f,-.35f},{0,.32f},{.52f,-.35f},{.42f,-1.1f},{0,-1.55f},{-.42f,-1.1f}}};
inline constexpr float BowHalfLength=27.f, BowCurve=11.f;

constexpr float cross(Point a,Point b,Point c){return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);}
template<std::size_t N>struct Triangles {std::array<std::array<int,3>,N-2> indices{};int count=0;};
template<std::size_t N>constexpr auto triangulate(const std::array<Point,N>& points){
    // Clip ears at compile time. The axe and breastplate have concave edges.
    Triangles<N> result;
    std::array<int,N> remaining{};
    float area=0;
    for(std::size_t i=0;i<N;i++){
        remaining[i]=int(i);
        const auto next=points[(i+1)%N];
        area+=points[i].x*next.y-next.x*points[i].y;
    }
    const float winding=area<0?-1.f:1.f;
    int count=int(N);
    while(count>2){
        bool found=false;
        for(int i=0;i<count;i++){
            const int a=remaining[(i+count-1)%count],b=remaining[i],c=remaining[(i+1)%count];
            if(winding*cross(points[a],points[b],points[c])<=.00001f)continue;
            bool occupied=false;
            for(int j=0;j<count;j++){
                const int vertex=remaining[j];
                if(vertex==a||vertex==b||vertex==c)continue;
                const Point p=points[vertex];
                if(winding*cross(points[a],points[b],p)>=-.00001f&&
                   winding*cross(points[b],points[c],p)>=-.00001f&&
                   winding*cross(points[c],points[a],p)>=-.00001f){
                    occupied=true;
                    break;
                }
            }
            if(occupied)continue;
            result.indices[result.count++]=winding>0?std::array<int,3>{a,b,c}:std::array<int,3>{a,c,b};
            for(int j=i;j<count-1;j++)remaining[j]=remaining[j+1];
            count--;
            found=true;
            break;
        }
        if(!found)break;
    }
    return result;
}
inline constexpr auto ShieldTriangles=triangulate(Shield);
inline constexpr auto VisorTriangles=triangulate(Visor),PendantTriangles=triangulate(Pendant);
inline constexpr auto HelmetTriangles=triangulate(Helmet);
inline constexpr auto SwordTriangles=triangulate(Sword),DaggerTriangles=triangulate(Dagger);
inline constexpr auto AxeTriangles=triangulate(Axe);
inline constexpr auto CuirassTriangles=triangulate(Cuirass);
inline constexpr auto ShoulderTriangles=triangulate(Shoulder);
static_assert(ShieldTriangles.count==5&&HelmetTriangles.count==7&&VisorTriangles.count==4&&SwordTriangles.count==3&&DaggerTriangles.count==3&&AxeTriangles.count==8&&CuirassTriangles.count==9&&ShoulderTriangles.count==2&&PendantTriangles.count==4);
}
