#pragma once
#include "painted_turn_shape.hpp"

namespace wraith_death_art::painted_motion {
inline painted_turn_shape::Material surface;
inline Shader shader{};
struct Locations {int left,right,flow,relief,phase;};
inline Locations locations{};
inline bool ready(){return surface.ready()&&shader.id&&shader.id!=rlGetShaderIdDefault();}
inline void unload(){
 if(ready())UnloadShader(shader);
 shader={};
 surface.unload();
}
inline void load(int width,int height){
 surface.load("assets/volcano/wraith-turn/shape",width,height);
 if(!surface.ready())return;
 const auto fragment=std::string("#version 330\n")+painted_turn_shape::shaderCode+R"GLSL(
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;
uniform sampler2D texture0,motionMap,clothRelief;
uniform vec4 tileA[6],tileB[6];
uniform vec3 phase;
void main(){
    int mode=int(floor(fragTexCoord.x/16.));
    float tag=fragTexCoord.x-16.*float(mode);
    int part=int(floor(tag/2.));
    vec2 uv=vec2(tag-2.*float(part),fragTexCoord.y);
    vec4 a=tileA[part],b=tileB[part];
    vec4 paint=paintedShape(texture0,motionMap,int(phase.x),part,a,b,phase.y,uv);
    if(mode==2){
        if(paint.a<.62)discard;
        finalColor=vec4(fragColor.a);
        return;
    }
    if(paint.a<.0039)discard;
    if(mode==1){
        finalColor=vec4(fragColor.rgb,paint.a*fragColor.a);
        return;
    }
    if(phase.z>0.){
        vec2 dim=vec2(textureSize(clothRelief,0));
        vec2 qa=(a.xy+uv*a.zw)/dim,qb=(b.xy+uv*b.zw)/dim;
        vec4 volume=mix(texture(clothRelief,qa),texture(clothRelief,qb),phase.y);
        vec3 n=normalize(vec3(-19.*vec2(dFdx(volume.a),dFdy(volume.a)),1.));
        float shade=.78+.34*max(0.,dot(n,normalize(vec3(-.52,.71,.62))));
        vec3 cooled=clamp(paint.rgb+volume.rgb-vec3(128./255.),0.,1.)*shade;
        paint.rgb=mix(paint.rgb,cooled,phase.z);
    }
    finalColor=paint*fragColor;
}
)GLSL";
 shader=LoadShaderFromMemory(nullptr,fragment.c_str());
 if(!ready()){unload();return;}
 const auto at=[](const char* name){return GetShaderLocation(shader,name);};
 locations={at("tileA[0]"),at("tileB[0]"),at("motionMap"),at("clothRelief"),at("phase")};
}
inline Rectangle pixels(int d,int p){
 const auto& tile=ash_wraith_painted::tiles[d][p];
 return {float(tile.x),float(tile.y),float(tile.w),float(tile.h)};
}
inline void begin(int left,int right,float t,float fold,Texture2D relief){
 Vector4 a[6],b[6];
 for(int p=0;p<6;p++){
  const auto first=pixels(left,p),second=pixels(right,p);
  a[p]={first.x,first.y,first.width,first.height};
  b[p]={second.x,second.y,second.width,second.height};
 }
 BeginShaderMode(shader);
 SetShaderValueV(shader,locations.left,a,SHADER_UNIFORM_VEC4,6);
 SetShaderValueV(shader,locations.right,b,SHADER_UNIFORM_VEC4,6);
 const Vector3 phase{float(left),t,relief.id?fold:0.f};
 SetShaderValue(shader,locations.phase,&phase,SHADER_UNIFORM_VEC3);
 SetShaderValueTexture(shader,locations.flow,surface.flow);
 if(phase.z>0)SetShaderValueTexture(shader,locations.relief,relief);
}
}
