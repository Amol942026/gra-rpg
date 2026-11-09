#pragma once
// A connected worn trail. Round only the interior corners; authored junctions,
// bridge approaches and endpoints stay within the original road clearance.
namespace volcano_roads {
struct Segment{V a,b;};
inline const std::vector<Segment>& segments(){
 static const auto value=[](){
  std::vector<Segment> result;result.reserve(80);
  for(const auto& road:VolcanoRoads){
   V previous=road.front();
   auto append=[&](V p){if(distance(previous,p)>.0001f)result.push_back({previous,p});previous=p;};
   for(size_t i=1;i+1<road.size();i++){
    V corner=road[i];float cut=std::min(2.4f,.22f*std::min(distance(corner,road[i-1]),distance(corner,road[i+1])));
    V from=corner+normalized(road[i-1]-corner)*cut,to=corner+normalized(road[i+1]-corner)*cut;append(from);
    for(int sample=1;sample<=6;sample++){float t=sample/6.f,u=1-t;append(from*(u*u)+corner*(2*u*t)+to*(t*t));}
   }
   append(road.back());
  }
  return result;
 }();return value;
}
inline float distanceTo(V p){
 float result=1e6f;for(const auto& s:segments()){
  float x=std::max({std::min(s.a.x,s.b.x)-p.x,p.x-std::max(s.a.x,s.b.x),0.f});
  float y=std::max({std::min(s.a.y,s.b.y)-p.y,p.y-std::max(s.a.y,s.b.y),0.f});
  if(x*x+y*y>=result*result)continue;result=std::min(result,desertSegmentDistance(p,s.a,s.b));
 }return result;
}
inline constexpr float FeatherLimit=2.4f;
inline float coverage(V p,float distance){
 float width=1.7f+.12f*std::sin(p.x*2.3f+p.y*.8f)+.06f*std::sin(p.y*4.2f-p.x);
 float t=std::clamp((distance-(width-.5f))/1.f,0.f,1.f);
 return 1-t*t*(3-2*t);
}
inline float coverage(V p){return coverage(p,distanceTo(p));}
}
