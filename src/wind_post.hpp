#pragma once

namespace wind_post {
struct Weather {
 const char* file;
 Color tint;
 float speed,rise,opacity,slope,textureScale,bend;
};
inline constexpr std::array<Weather,3> weather={{
 {"forest",{214,232,206,255},.096875f,.009375f,.226f,.095f,1.f,.016f},
 {"desert",{249,218,159,255},.123f,.016f,.207f,.16f,1.f,.037f},
 {"caldera",{212,208,205,255},.102f,.024f,.278f,.23f,1.62f,.008f},
}};
inline std::array<Texture2D,weather.size()> masks{};
inline Shader shader{};
inline int driftUniform=-1,shapeUniform=-1,actorUniform=-1,fieldUniform=-1;

inline void load(){
 if(shader.id)return;
 shader=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform vec4 drift;
uniform vec4 shape;
uniform vec4 actor;
uniform vec4 field;
out vec4 finalColor;
void main(){
 vec2 uv=fragTexCoord;
 vec2 world=uv+field.xy;
 float bend=sin(world.y*9.0+world.x*4.0+shape.y)*field.w;
 vec2 nearUV=vec2(world.x*1.24,world.y*2.12+world.x*shape.z+bend)*field.z+drift.xy;
 vec2 farUV=vec2(world.x*1.84,world.y*3.18+world.x*shape.z*.75-bend*.66)*field.z+drift.zw;
 float nearDust=texture(texture0,nearUV).a;
 float farDust=texture(texture0,farUV).a;
 float veil=clamp(nearDust*1.22+farDust*.65,0.0,1.0);
 float shelter=mix(.24,1.0,smoothstep(.42,1.37,length((uv-actor.xy)*actor.zw)));
 float edges=smoothstep(.01,.13,uv.y)*(1.0-smoothstep(.90,.995,uv.y));
 float height=mix(1.0-shape.w,1.0,smoothstep(.15,.77,uv.y));
 finalColor=vec4(fragColor.rgb,veil*shape.x*shelter*edges*height);
}
)GLSL");
 driftUniform=GetShaderLocation(shader,"drift");
 shapeUniform=GetShaderLocation(shader,"shape");
 actorUniform=GetShaderLocation(shader,"actor");
 fieldUniform=GetShaderLocation(shader,"field");
 if(driftUniform<0||shapeUniform<0||actorUniform<0||fieldUniform<0)return;
 for(size_t i=0;i<weather.size();i++){
  const std::string path=std::string("assets/sprites/wind/")+weather[i].file+".png";
  if(!FileExists(path.c_str()))continue;
  masks[i]=LoadTexture(path.c_str());
  if(masks[i].id){
   GenTextureMipmaps(&masks[i]);
   SetTextureFilter(masks[i],TEXTURE_FILTER_TRILINEAR);
   SetTextureWrap(masks[i],TEXTURE_WRAP_REPEAT);
  }
 }
}

inline void unload(){
 for(auto& mask:masks){if(mask.id)UnloadTexture(mask);mask={};}
 if(shader.id&&shader.id!=rlGetShaderIdDefault())UnloadShader(shader);
 shader={};
}

inline void draw(){
 if(game.inTower())return;
 const size_t index=game.inVolcano()?2:game.inDesert()?1:0;
 const Texture2D mask=masks[index];
 if(!mask.id)return;
 const Weather& wind=weather[index];
 const float time=game.time;
 // Undo the exact projection used by the terrain before evaluating either
 // layer. Camera movement only selects a different view of the same field.
 const Vector2 origin=screenFlat({0,0});
 const float field[]={-origin.x/WorldW,-origin.y/H,wind.textureScale,wind.bend};
 auto wrap=[](float offset){return offset-std::floor(offset);};
 const float drift[]={wrap(-time*wind.speed),wrap(time*wind.rise),
                      wrap(.41f-time*wind.speed*.56f),wrap(.56f+time*wind.rise*.64f)};
 const float gust=.76f+.16f*std::sin(time*.73f+float(index))+.08f*std::sin(time*1.31f);
 const float shape[]={wind.opacity*gust,std::fmod(time*1.02f,2*PI),wind.slope,index==0?.17f:.34f};
 const Vector2 player=screen(game.player);
 const float actor[]={player.x/WorldW,(player.y-47)/H,WorldW/146.f,H/118.f};
 BeginShaderMode(shader);
 SetShaderValue(shader,driftUniform,drift,SHADER_UNIFORM_VEC4);
 SetShaderValue(shader,shapeUniform,shape,SHADER_UNIFORM_VEC4);
 SetShaderValue(shader,actorUniform,actor,SHADER_UNIFORM_VEC4);
 SetShaderValue(shader,fieldUniform,field,SHADER_UNIFORM_VEC4);
 DrawTexturePro(mask,{0,0,float(mask.width),float(mask.height)},{0,0,WorldW,H},{},0,wind.tint);
 EndShaderMode();
}
}
