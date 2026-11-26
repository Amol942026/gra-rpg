#pragma once
namespace marsh {
inline constexpr V Arrival{17,118},Portal{15,119},Guardian{114,27};
inline constexpr V Village{74,76},Ferry{44,100},Reedbed{28,54},Isle{61,23},Mill{109,100},Peat{70,128};
struct Route{V a,b;};
inline constexpr Route Routes[]={{Arrival,Ferry},{Ferry,Village},{Village,{92,52}},{{92,52},Guardian},{Ferry,Reedbed},{Reedbed,Isle},{Isle,Guardian},{Village,Mill},{Mill,Peat},{Peat,Arrival}};
inline float road(V p){
 float best=10000;
 for(auto road:Routes){V d=road.b-road.a,v=p-road.a;float t=std::clamp((v.x*d.x+v.y*d.y)/(d.x*d.x+d.y*d.y),0.f,1.f);V off=v-d*t;best=std::min(best,off.x*off.x+off.y*off.y);}
 return std::sqrt(best);
}
struct Pool{V center,extent;};
inline constexpr Pool Pools[]={{{30,84},{16,13}},{{51,73},{12,22}},{{58,47},{22,10}},{{92,34},{17,12}},{{88,94},{17,12}},{{42,124},{15,8}},{{118,61},{16,21}},{{16,22},{10,15}},{{113,122},{15,8}},
 {{40,108},{8,7}},{{81,79},{7,6}},{{115,98},{6.5f,8.5f}},{{22,124},{5,6}},{{121,26},{6,7}}};
// Signed shore distance in tiles. The raised causeways cut dry passages across the meres.
inline float shore(V p){
 float best=20;
 for(const auto& pool:Pools){const V d=p-pool.center;if(std::abs(d.x)>pool.extent.x+6||std::abs(d.y)>pool.extent.y+6)continue;
  float x=d.x/pool.extent.x,y=d.y/pool.extent.y;
  best=std::min(best,(std::sqrt(x*x+y*y)-1)*std::min(pool.extent.x,pool.extent.y));}
 if(best>6)return best;
 best+=.60f*std::sin(p.x*.56f+p.y*.19f)+.42f*std::sin(p.y*.72f);
 return std::max(best,2.25f-road(p));
}
inline bool deep(V p,float radius=0){return shore(p)<-.55f+radius;}
}
