#pragma once
#include "raymath.h"

namespace corpse_pose_mix {
constexpr int Width=512,Height=384;
inline RenderTexture2D falling{},resting{};
inline Shader mixShader{};
inline int restingTexture=-1,phaseLocation=-1;

inline void init(){
 falling=LoadRenderTexture(Width,Height);
 resting=LoadRenderTexture(Width,Height);
 const char* fragment=R"GLSL(#version 330
in vec2 fragTexCoord;
uniform sampler2D texture0;
uniform sampler2D rest;
uniform float phase;
out vec4 finalColor;
void main(){finalColor=mix(texture(texture0,fragTexCoord),texture(rest,fragTexCoord),phase);}
)GLSL";
 mixShader=LoadShaderFromMemory(nullptr,fragment);
 restingTexture=GetShaderLocation(mixShader,"rest");
 phaseLocation=GetShaderLocation(mixShader,"phase");
}
inline void unload(){
 if(falling.id)UnloadRenderTexture(falling);
 if(resting.id)UnloadRenderTexture(resting);
 if(IsShaderValid(mixShader))UnloadShader(mixShader);
 falling={};resting={};mixShader={};
}

template<class Falling,class Resting>
inline bool draw(Vector2 anchor,float phase,Falling moving,Resting settled){
 using GetState=void(*)(unsigned,int*);
 using IsEnabled=unsigned char(*)(unsigned);
 static const auto get=reinterpret_cast<GetState>(glfwGetProcAddress("glGetIntegerv"));
 static const auto enabled=reinterpret_cast<IsEnabled>(glfwGetProcAddress("glIsEnabled"));
 if(!get||!enabled||!falling.id||!resting.id||restingTexture<0||phaseLocation<0)return false;
 const auto projection=rlGetMatrixProjection(),view=rlGetMatrixModelview();
 const auto transform=MatrixMultiply(rlGetMatrixTransform(),view);
 // Snap the buffer rectangle to device pixels. This keeps a direct draw and
 // the same pose drawn through the compositor on the same sampling grid.
 const float scaleX=std::abs(transform.m0),scaleY=std::abs(transform.m5);
 if(scaleX<.1f||scaleY<.1f)return false;
 const float left=(std::floor((anchor.x-Width*.5f/scaleX)*scaleX+transform.m12)-transform.m12)/scaleX;
 const float top=(std::floor((anchor.y-Height*.66f/scaleY)*scaleY+transform.m13)-transform.m13)/scaleY;
 const Vector2 local{anchor.x-left,anchor.y-top};
 const auto framebuffer=rlGetActiveFramebuffer();
 int viewport[4],scissor[4];get(0x0BA2,viewport);get(0x0C10,scissor);
 const bool clipped=enabled(0x0C11)!=0;
 const bool opaque=native_world::drawingWorld,depth=actor_art::sceneDepth;

 rlDrawRenderBatchActive();rlDisableScissorTest();
 rlMatrixMode(RL_MODELVIEW);rlPushMatrix();rlLoadIdentity();
 rlSetMatrixProjection(MatrixOrtho(0,Width,Height,0,0,1));
 rlSetMatrixModelview(MatrixScale(scaleX,scaleY,1));
 rlViewport(0,0,Width,Height);
 actor_art::sceneDepth=false;
 auto capture=[&](RenderTexture2D target,auto paint){
  rlEnableFramebuffer(target.id);ClearBackground(BLANK);
  native_world::beginOpaqueFrame();paint(local);native_world::finishOpaqueBackground();
  rlDrawRenderBatchActive();
 };
 capture(falling,moving);capture(resting,settled);
 actor_art::sceneDepth=depth;
 rlMatrixMode(RL_MODELVIEW);rlPopMatrix();
 rlSetMatrixProjection(projection);rlSetMatrixModelview(view);
 rlEnableFramebuffer(framebuffer);rlViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
 if(clipped){rlScissor(scissor[0],scissor[1],scissor[2],scissor[3]);rlEnableScissorTest();}
 native_world::drawingWorld=opaque;

 // Each source carries its complete premultiplied silhouette, including
 // paint and cursor outline. Interpolate it once and cover the terrain once.
 BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
 BeginShaderMode(mixShader);
 SetShaderValueTexture(mixShader,restingTexture,resting.texture);
 SetShaderValue(mixShader,phaseLocation,&phase,SHADER_UNIFORM_FLOAT);
 DrawTexturePro(falling.texture,{0,0,Width,-Height},{left,top,Width/scaleX,Height/scaleY},{},0,WHITE);
 native_world::endBlendMode();EndShaderMode();
 return true;
}
}
