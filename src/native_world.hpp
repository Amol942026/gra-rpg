#pragma once
namespace native_world {
inline bool drawingWorld=false;
inline void beginSourceOver(){
 // Blend RGB normally while keeping the coverage of the pixels underneath.
 rlSetBlendFactorsSeparate(RL_SRC_ALPHA,RL_ONE_MINUS_SRC_ALPHA,RL_ONE,RL_ONE_MINUS_SRC_ALPHA,RL_FUNC_ADD,RL_FUNC_ADD);
 BeginBlendMode(BLEND_CUSTOM_SEPARATE);
}
inline void restoreBlend(){if(drawingWorld)beginSourceOver();}
inline void endBlendMode(){EndBlendMode();restoreBlend();}
inline void beginOpaqueFrame(){drawingWorld=true;restoreBlend();}
inline void finishOpaqueBackground(){drawingWorld=false;EndBlendMode();}
}
