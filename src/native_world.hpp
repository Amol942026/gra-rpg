#pragma once
namespace native_world {
inline bool drawingWorld=false;
inline void restoreBlend(){
 if(!drawingWorld)return;
 // Correct source-over alpha keeps an already opaque background opaque.
 // RGB blends normally; no corrective full-screen pass is required.
 rlSetBlendFactorsSeparate(RL_SRC_ALPHA,RL_ONE_MINUS_SRC_ALPHA,RL_ONE,RL_ONE_MINUS_SRC_ALPHA,RL_FUNC_ADD,RL_FUNC_ADD);
 BeginBlendMode(BLEND_CUSTOM_SEPARATE);
}
inline void endBlendMode(){EndBlendMode();restoreBlend();}
inline void beginOpaqueFrame(){drawingWorld=true;restoreBlend();}
inline void finishOpaqueBackground(){drawingWorld=false;EndBlendMode();}
}
