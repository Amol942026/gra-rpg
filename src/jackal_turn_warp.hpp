#pragma once

// Included from jackal_cels.hpp after the cel atlas layout.
namespace turn_warp {
inline constexpr int GridW=40,GridH=36,BlockW=10,BlockH=8;
inline constexpr int FitSamples=desert_motion::JackalFacings*4;
inline constexpr float GroundPixel=AnchorY/WorldHeight*Height,LowerJoint=98;
struct SizeFit {float width=1,upper=1,lower=1,offset=0;};
inline std::array<std::array<SizeFit,Walk+1>,FitSamples> sizeFits;
inline bool hasSizeFits=false;
inline constexpr float ChestTop=38;
struct SpanFit {float start,end,gain;};
struct TorsoFit {SpanFit length{55,115,1},depth{55,85,1};};
inline std::array<std::array<TorsoFit,Walk+1>,FitSamples> torsoFits;
inline bool hasTorsoFits=false;
struct Pixels {
 Texture2D texture{};
 std::vector<Color> colors;
 bool load(const std::string& file,int w,int h){
  Image image=LoadImage(file.c_str());
  if(!image.data||image.width!=w||image.height!=h){if(image.data)UnloadImage(image);return false;}
  ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  if(image.format!=PIXELFORMAT_UNCOMPRESSED_R8G8B8A8){UnloadImage(image);return false;}
  const Color* data=static_cast<const Color*>(image.data);
  colors.assign(data,data+size_t(w)*h);
  texture=LoadTextureFromImage(image);
  UnloadImage(image);
  if(texture.id)SetTextureFilter(texture,TEXTURE_FILTER_BILINEAR);
  return texture.id!=0;
 }
 void unload(){if(texture.id)UnloadTexture(texture);texture={};colors.clear();}
};

inline Pixels flow,limbInk;
struct Part {Rectangle uv;float half,lead,trail;};
struct Leg {std::array<Vector2,4> joints;float pitch;};
using Rig = std::array<Leg,4>;
inline std::array<Part,8> parts;
inline std::array<std::array<Rig,Frames>,Paintings.size()> rigs;
inline Shader shader{};
inline bool active=false;
struct Uniforms {int other,fields,cels,flowTile,angles,mode;};
inline Uniforms uniforms{};
inline constexpr const char* Fragment=R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0,other,fields;
uniform vec4 cels, angles;
uniform vec2 flowTile;
uniform int mode;
out vec4 finalColor;

vec4 readCel(sampler2D page, vec2 tile, vec2 point, float mirror){
    const vec2 span=vec2(172.,150.);
    if(any(lessThan(point,vec2(0.)))||any(greaterThanEqual(point,span))) return vec4(0.);
    if(mirror>0.5) point.x=span.x-point.x;
    return texture(page,(tile+point)/vec2(textureSize(page,0)));
}
vec2 gridUV(vec2 origin,vec2 size,vec2 atlas){
    return (origin+clamp(fragTexCoord*size,vec2(.5),size-.5))/atlas;
}
void main(){
    float t=angles.x;
    vec2 p=fragTexCoord*vec2(172.,150.);
    vec4 delta=(texture(fields,gridUV(flowTile,vec2(40.,36.),vec2(textureSize(fields,0))))*255.-128.)*.5;
    vec4 a=readCel(texture0,cels.xy,p+delta.zw*t,angles.y);
    vec4 b=readCel(other,cels.zw,p+delta.xy*(1.-t),angles.z);
    float da=(a.a*255.-128.)*.125, db=(b.a*255.-128.)*.125;
    float alpha=clamp(mix(da,db,t)+.5,0.,1.);
    if(mode==2){if(alpha<.62) discard;finalColor=vec4(fragColor.a);return;}
    if(alpha<=0.) discard;
    if(mode==1){finalColor=vec4(fragColor.rgb,fragColor.a*alpha);return;}
    vec2 strength=vec2((1.-t)*clamp(da+.65,0.,1.),t*clamp(db+.65,0.,1.));
    float sum=strength.x+strength.y;
    vec3 ink=sum>.00001?(a.rgb*strength.x+b.rgb*strength.y)/sum : (da>db?a.rgb:b.rgb);
    finalColor=vec4(ink*fragColor.rgb,alpha*fragColor.a);
}
)GLSL";

inline void unload(){
 flow.unload();limbInk.unload();
 if(shader.id&&shader.id!=rlGetShaderIdDefault())UnloadShader(shader);
 shader={};active=false;hasSizeFits=false;hasTorsoFits=false;
}

inline void loadTorsoFits(const std::string& file){
 static_assert(sizeof(TorsoFit)==6*sizeof(float));
 hasTorsoFits=false;
 if(!FileExists(file.c_str()))return;
 int size=0;
 auto* bytes=LoadFileData(file.c_str(),&size);
 if(!bytes)return;
 const bool header=size==int(8+sizeof(torsoFits))&&std::memcmp(bytes,"JTF1",4)==0&&
                   bytes[4]==FitSamples&&bytes[5]==0&&bytes[6]==Walk+1&&bytes[7]==0;
 if(header)std::memcpy(torsoFits.data(),bytes+8,sizeof(torsoFits));
 UnloadFileData(bytes);
 if(!header)return;
 const auto valid=[](SpanFit span,float first,float last){
  const float shift=(span.end-span.start)*(1.f-span.gain)*.5f;
  return span.start>first&&span.end>span.start&&span.end<last&&span.gain>=.62f&&span.gain<=1.55f&&
         span.start+shift>first&&span.end-shift<last;
 };
 for(const auto& sector:torsoFits)for(const auto& p:sector)
  if(!valid(p.length,0,Width)||!valid(p.depth,ChestTop,LowerJoint))return;
 hasTorsoFits=true;
}

inline void loadSizeFits(const std::string& file){
 static_assert(sizeof(SizeFit)==4*sizeof(float));
 hasSizeFits=false;
 if(!FileExists(file.c_str()))return;
 int size=0;
 auto* bytes=LoadFileData(file.c_str(),&size);
 if(!bytes)return;
 const bool header=size>=8&&bytes[4]==FitSamples&&bytes[5]==0&&bytes[6]==Walk+1&&bytes[7]==0;
 const int stride=header&&std::memcmp(bytes,"JSC2",4)==0?sizeof(SizeFit):
                  header&&std::memcmp(bytes,"JSC1",4)==0?3*sizeof(float):0;
 const bool layout=stride>0&&size==8+FitSamples*(Walk+1)*stride;
 if(layout){
  const auto* cursor=bytes+8;
  for(auto& sector:sizeFits)for(auto& pose:sector){
   pose={};
   std::memcpy(&pose,cursor,stride);
   cursor+=stride;
  }
 }
 UnloadFileData(bytes);
 if(!layout)return;
 const auto valid=[](float s){return std::isfinite(s)&&s>=.6f&&s<=1.55f;};
 for(const auto& sector:sizeFits)for(const auto& p:sector)
  if(!valid(p.width)||!valid(p.upper)||!valid(p.lower)||!std::isfinite(p.offset)||std::abs(p.offset)>16.f)return;
 hasSizeFits=true;
}

inline bool loadRig(const std::string& file){
 static_assert(sizeof(Part)==7*sizeof(float)&&sizeof(Leg)==9*sizeof(float));
 int size=0;
 auto* bytes=LoadFileData(file.c_str(),&size);
 if(!bytes)return false;
 const bool header=size==int(4+sizeof(parts)+sizeof(rigs))&&std::memcmp(bytes,"JTR1",4)==0;
 if(header){
  std::memcpy(parts.data(),bytes+4,sizeof(parts));
  std::memcpy(rigs.data(),bytes+4+sizeof(parts),sizeof(rigs));
 }
 UnloadFileData(bytes);
 if(!header)return false;
 for(const auto& p:parts){
  if(!std::isfinite(p.uv.x)||!std::isfinite(p.uv.y)||
     !std::isfinite(p.uv.width)||!std::isfinite(p.uv.height)||
     !std::isfinite(p.half)||!std::isfinite(p.lead)||!std::isfinite(p.trail)||
     p.uv.width<=0||p.uv.height<=0||p.uv.x<1||p.uv.y<1||p.half<=0||p.lead<=0||p.trail<=0||
     p.uv.x+p.uv.width>=limbInk.texture.width||p.uv.y+p.uv.height>=limbInk.texture.height)return false;
 }
 for(const auto& page:rigs)for(const auto& frame:page)for(const auto& leg:frame){
  if(!std::isfinite(leg.pitch)||std::abs(leg.pitch)>120)return false;
  for(Vector2 joint:leg.joints)if(!std::isfinite(joint.x)||!std::isfinite(joint.y)||
                                  std::abs(joint.x)>400||std::abs(joint.y)>400)return false;
 }
 return true;
}

inline bool load(const std::string& root){
 if(!FileExists((root+"turn-fields.png").c_str()))return false;
 for(int id:Paintings)if(!FileExists((root+"distance-"+std::to_string(id)+".png").c_str())||
                         !FileExists((root+"body-"+std::to_string(id)+".png").c_str()))return false;
 if(!flow.load(root+"turn-fields.png",(GridW+2)*BlockW*4,(GridH+2)*BlockH*4)){unload();return false;}
 if(!limbInk.load(root+"turn-limbs.png",480,152)||!loadRig(root+"turn-rig.bin")){unload();return false;}
 shader=LoadShaderFromMemory(nullptr,Fragment);
 if(!shader.id||shader.id==rlGetShaderIdDefault()){unload();return false;}
 const auto loc=[](const char* name){return GetShaderLocation(shader,name);};
 uniforms={loc("other"),loc("fields"),loc("cels"),loc("flowTile"),loc("angles"),loc("mode")};
 loadSizeFits(root+"turn-size.bin");
 loadTorsoFits(root+"turn-torso.bin");
 active=true;return true;
}

inline Vector2 fieldOrigin(int direction,int pose){
 return {float(((direction%4)*BlockW+pose%BlockW)*(GridW+2)+1),
         float(((direction/4)*BlockH+pose/BlockW)*(GridH+2)+1)};
}
inline std::array<float,4> lookup(const Pixels& layer,Vector2 origin,Vector2 point,int w,int h){
 const float x=origin.x+std::clamp(point.x*w/Width-.5f,0.f,float(w-1));
 const float y=origin.y+std::clamp(point.y*h/Height-.5f,0.f,float(h-1));
 const int px=int(x),py=int(y),pitch=layer.texture.width;
 const Color* p=layer.colors.data()+py*pitch+px;
 const auto tap=[&](unsigned char Color::* c){
  return caldera_quadruped::mix(caldera_quadruped::mix(p[0].*c,p[1].*c,x-px),
                               caldera_quadruped::mix(p[pitch].*c,p[pitch+1].*c,x-px),y-py);
 };
 return {tap(&Color::r),tap(&Color::g),tap(&Color::b),tap(&Color::a)};
}
}
