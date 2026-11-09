#pragma once
// Included inside ember after V, realm IDs and the authored field layouts.
// Ground elevation is world data, shared by movement and all renderers. It is
// derived from XY, so existing saves recover the correct height immediately.
namespace terrain_height {
enum class Travel {Surface,PlanarImpulse};
inline constexpr float PixelsPerUnit=32,MaxPixels=128;
struct Hill{V center,radius;float height;};
// Larger forms interleave with lower knolls. Unequal elliptical footprints
// merge into ridges and saddles, keeping quiet low ground between groups.
inline constexpr std::array<Hill,48> Forest={{
 {{14,55},{9,10},2.8f},{{40,18},{10,10},3.1f},{{55,39},{11,11},3.3f},{{42,59},{8.5f,8.5f},2.3f},
 {{12,17},{8,9},1.f},{{25,12},{8.5f,7},1.7f},{{56,12},{11,9},3.6f},{{21,31},{10,10},2.3f},
 {{43,34},{7,8},.8f},{{61,59},{9,8},1.3f},{{26,61},{6.5f,8},.7f},{{9,38},{6.5f,9},1.6f},
 {{32,45},{9,8},1.9f},{{61,25},{7,8},1.1f},
 {{78,12},{7.5f,8.0f},0.9f},{{78,31},{9.0f,9.0f},1.8f},{{78,50},{10.0f,7.0f},2.7f},{{78,70},{9.0f,10.0f},3.3f},
 {{78,90},{7.5f,8.0f},1.3f},{{92,12},{9.0f,9.0f},2.2f},{{92,31},{10.0f,9.0f},3.6f},{{92,50},{8.0f,10.0f},0.9f},
 {{92,70},{7.5f,8.0f},1.8f},{{92,90},{9.0f,9.0f},2.7f},{{12,85},{10.0f,9.0f},3.3f},{{32,85},{8.0f,10.0f},1.3f},
 {{52,85},{7.5f,8.0f},2.2f},{{66,85},{9.0f,9.0f},3.6f},
 {{112,13},{10,10},2.8f},{{131,19},{8,9},1.1f},{{114,36},{12,11},1.7f},{{134,42},{9,12},3.1f},
 {{114,60},{10,13},2.4f},{{132,67},{7,9},1.4f},{{113,85},{9,13},3.2f},{{131,94},{9,11},2.0f},
 {{116,110},{10,11},1.0f},{{132,119},{9,10},3.0f},{{114,133},{9,10},2.5f},{{132,137},{8,8},0.8f},
 {{12,118},{9,11},1.7f},{{24,131},{9,10},3.0f},{{37,113},{9,10},2.5f},{{49,132},{10,10},1.3f},
 {{61,116},{10,12},3.2f},{{74,133},{9,10},2.6f},{{84,115},{8,12},1.2f},{{98,128},{9,12},3.1f}
}};
inline constexpr std::array<Hill,48> Desert={{
 {{38,17},{10,8},2.8f},{{54,27},{9.5f,10},3.f},{{30,52},{10,11},2.5f},{{12,61},{7.5f,8},1.8f},
 {{13,12},{8,8},.65f},{{23,25},{10,8},1.4f},{{57,12},{10,8},3.6f},{{8,31},{6,8},1.3f},
 {{36,35},{9,8},1.8f},{{62,43},{7,9},1.1f},{{46,49},{8,9},2.f},{{56,62},{10,7},2.5f},
 {{33,65},{8,6},.85f},{{23,9},{7,6},1.25f},
 {{78,12},{7.5f,8.0f},2.7f},{{78,31},{9.0f,9.0f},3.3f},{{78,50},{10.0f,7.0f},1.3f},{{78,70},{8.0f,10.0f},2.2f},
 {{78,90},{9.0f,9.0f},3.6f},{{92,12},{9.0f,9.0f},0.9f},{{92,31},{10.0f,7.0f},1.8f},{{92,50},{8.0f,10.0f},2.7f},
 {{92,70},{9.0f,9.0f},3.3f},{{92,90},{9.0f,9.0f},1.3f},{{12,85},{10.0f,7.0f},2.2f},{{32,85},{9.0f,10.0f},3.6f},
 {{52,85},{7.5f,8.0f},0.9f},{{66,85},{9.0f,9.0f},1.8f},
 {{113,9},{10,9},1.8f},{{132,19},{10,9},3.1f},{{115,33},{12,9},2.7f},{{133,44},{8,11},1.5f},
 {{113,58},{10,10},2.1f},{{130,66},{12,9},3.2f},{{114,81},{11,10},3.3f},{{131,93},{10,9},1.1f},
 {{116,104},{12,10},2.4f},{{134,114},{8,11},1.9f},{{113,130},{10,11},1.4f},{{133,135},{9,9},3.0f},
 {{12,117},{9,12},2.1f},{{24,134},{10,10},3.2f},{{37,113},{9,10},1.3f},{{49,129},{11,11},2.5f},
 {{64,116},{11,11},3.1f},{{77,135},{10,9},0.9f},{{85,116},{8,10},2.1f},{{99,128},{10,13},2.8f}
}};
inline constexpr std::array<Hill,54> Volcano={{
 {{12,51},{8,9},2.5f},{{13,26},{8.5f,9},3.1f},{{38,52},{6,7},1.7f},{{61,41},{7.5f,9},2.7f},{{62,61},{6.5f,7},2.2f},
 {{9,11},{6.5f,7},1.35f},{{15,14},{8,8},2.f},{{10,39},{7,6},.75f},{{16,63},{7,6},1.1f},
 {{37,11},{6.5f,8},2.25f},{{38,28},{6.5f,8},1.15f},{{40,42},{6.5f,7},2.3f},{{39,64},{6.5f,6},.65f},
 {{62,27},{7.5f,8},1.4f},{{61,9},{9,7},3.5f},{{60,52},{6.5f,7},.8f},
 {{78,12},{7.5f,8.0f},2.7f},{{78,31},{9.0f,9.0f},3.3f},{{78,50},{10.0f,7.0f},1.3f},{{78,70},{8.0f,10.0f},2.2f},
 {{78,90},{9.0f,9.0f},3.6f},{{92,12},{9.0f,9.0f},0.9f},{{92,31},{10.0f,7.0f},1.8f},{{92,50},{8.0f,10.0f},2.7f},
 {{92,70},{9.0f,9.0f},3.3f},{{92,90},{9.0f,9.0f},1.3f},{{12,83},{10.0f,7.0f},2.2f},{{38,83},{8.0f,10.0f},2.4f},
 {{63,83},{7.5f,8.0f},0.9f},{{12,95},{9.0f,9.0f},1.8f},{{38,95},{10.0f,7.0f},1.9f},{{63,95},{9.0f,10.0f},3.3f},
 {{113,13},{10,9},2.8f},{{132,20},{10,9},1.1f},{{114,33},{11,10},1.7f},{{134,43},{9,11},3.3f},
 {{113,57},{10,12},2.6f},{{131,66},{11,9},1.5f},{{113,79},{11,11},3.1f},{{133,94},{9,11},2.4f},
 {{114,105},{11,10},1.0f},{{133,117},{9,12},3.1f},{{115,133},{10,10},2.8f},{{134,137},{8,8},1.0f},
 {{13,114},{8,10},2.2f},{{12,131},{8,11},3.0f},{{37,115},{6,11},1.8f},{{38,132},{7,11},2.7f},
 {{65,115},{10,11},3.1f},{{66,134},{10,10},1.8f},{{82,112},{9,10},2.2f},{{83,133},{10,9},3.0f},
 {{99,118},{10,11},1.4f},{{101,135},{8,9},2.2f}
}};
inline constexpr size_t Count=Forest.size()+Desert.size()+Volcano.size();
inline size_t count(WorldLocation realm){return realm==WorldLocation::Forest?Forest.size():realm==WorldLocation::Desert?Desert.size():realm==WorldLocation::Volcano?Volcano.size():0;}
inline const Hill& hill(WorldLocation realm,size_t index){return realm==WorldLocation::Forest?Forest[index]:realm==WorldLocation::Desert?Desert[index]:Volcano[index];}
inline float smooth(float a,float b,float v){float t=std::clamp((v-a)/(b-a),0.f,1.f);return t*t*(3-2*t);}
template<size_t N>inline float sample(const std::array<Hill,N>& hills,V p){
 float h=0;
 for(const auto& hill:hills){
  V delta=p-hill.center;
  if(std::abs(delta.x)>=hill.radius.x||std::abs(delta.y)>=hill.radius.y)continue;
  float x=delta.x/hill.radius.x,y=delta.y/hill.radius.y,t=1-x*x-y*y;
  if(t>0)h+=hill.height*t*t;
 }
 return h;
}
inline float height(WorldLocation realm,V p){
 if(realm==WorldLocation::Forest)return sample(Forest,p);
 if(realm==WorldLocation::Desert)return sample(Desert,p);
 if(realm!=WorldLocation::Volcano)return 0;
 float h=sample(Volcano,p);if(h==0)return 0;
 // Existing lava beds, crossings and shrine approaches stay level. Hills
 // blend into their banks instead of lifting the water or blocking a bridge.
 float shore=std::min(std::abs(p.x-lavaCenter(0,p.y)),std::abs(p.x-lavaCenter(1,p.y)));
 return h*smooth(3.8f,12.f,shore);
}
inline float pixels(WorldLocation realm,V p){return height(realm,p)*PixelsPerUnit;}
inline V gradient(WorldLocation realm,V p){constexpr float e=.025f;return {(height(realm,p+V{e,0})-height(realm,p-V{e,0}))/(2*e),(height(realm,p+V{0,e})-height(realm,p-V{0,e}))/(2*e)};}
inline float surfaceDistance(WorldLocation realm,V a,V b){float d=height(realm,b)-height(realm,a);V v=b-a;return std::sqrt(v.x*v.x+v.y*v.y+d*d);}
inline V step(WorldLocation realm,V from,V delta){float flat=length(delta);if(flat<.000001f)return delta;float rise=height(realm,from+delta)-height(realm,from);return delta*(flat/std::sqrt(flat*flat+rise*rise));}
}
