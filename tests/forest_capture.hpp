#pragma once
#include <cstring>

// Replace only the requested rows of the shipped corpse atlas with fresh
// output from the same renderer that supplies live corpse picking and shadows.
template<size_t N, class Check>
void bakeForestBodies(const fs::path& root,const std::array<int,N>& types,Check check){
 constexpr int w=fallen_art::Width*fallen_art::Density,h=fallen_art::Height*fallen_art::Density;
 Image atlas=LoadImage("assets/generated/corpses-creatures-v48.png");
 bool layout=atlas.data&&atlas.width==w*8&&atlas.height==h*LegacyEnemyTypeCount;
 check(layout,"existing corpse atlas dimensions");
 if(!layout){if(atlas.data)UnloadImage(atlas);return;}
 ImageFormat(&atlas,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
 RenderTexture2D target=LoadRenderTexture(w,h);
 for(int type:types)for(int dir=0;dir<8;dir++){
  BeginTextureMode(target);ClearBackground(BLANK);
  actor_art::Rig rig(fallen_art::OriginX*fallen_art::Density,fallen_art::OriginY*fallen_art::Density,
                     fallen_art::Density,Direction(dir));
  actor_art::buildFallenForest(rig,type);rig.draw();EndTextureMode();
  Image image=LoadImageFromTexture(target.texture);ImageFlipVertical(&image);
  Color* src=LoadImageColors(image);auto* dest=static_cast<Color*>(atlas.data);
  for(int y=0;y<h;y++)std::memcpy(dest+(type*h+y)*atlas.width+dir*w,src+y*w,w*sizeof(Color));
  const auto file=root/TextFormat("fallen-%d-%d.png",type,dir);
  check(ExportImage(image,file.string().c_str()),"native fallen view "+file.filename().string());
  UnloadImageColors(src);UnloadImage(image);
 }
 UnloadRenderTexture(target);
 check(ExportImage(atlas,(root/"corpses-creatures-v48.png").string().c_str()),"updated corpse atlas");
 UnloadImage(atlas);
}
