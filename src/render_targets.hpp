#pragma once
namespace render_targets {
inline unsigned colorOnlyTargets=0;
inline std::uint64_t omittedDepthPixels=0;
inline RenderTexture2D colorOnly(int width,int height){
 RenderTexture2D target{};target.id=rlLoadFramebuffer();
 if(target.id){
  rlEnableFramebuffer(target.id);target.texture.width=width;target.texture.height=height;
  target.texture.format=PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;target.texture.mipmaps=1;
  target.texture.id=rlLoadTexture(nullptr,width,height,target.texture.format,1);
  if(target.texture.id)rlFramebufferAttach(target.id,target.texture.id,RL_ATTACHMENT_COLOR_CHANNEL0,RL_ATTACHMENT_TEXTURE2D,0);
  bool complete=target.texture.id&&rlFramebufferComplete(target.id);rlDisableFramebuffer();
  if(complete){colorOnlyTargets++;omittedDepthPixels+=std::uint64_t(width)*height;return target;}
  UnloadRenderTexture(target);
 }
 return LoadRenderTexture(width,height);
}
}
