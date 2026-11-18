#pragma once
namespace drake_mouth {
inline constexpr int Tile=96,Views=5,Levels=8;
inline constexpr int origin[Views][2]={{91,70},{89,43},{47,42},{7,43},{5,70}};
inline Texture2D art{},mask{};
inline std::vector<unsigned char> alpha;
inline void unload(){
 if(art.id)UnloadTexture(art);
 if(mask.id)UnloadTexture(mask);
 art={};mask={};alpha.clear();
}
inline void load(){
 if(art.id)return;
 const char* file="assets/volcano/ember-drake-mouth.png";
 if(!FileExists(file))return;
 Image head=LoadImage(file);
 if(!head.data)return;
 if(head.width!=Views*Tile||head.height!=Levels*Tile){UnloadImage(head);return;}
 ImageFormat(&head,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
 art=LoadTextureFromImage(head);
 const auto* pixels=static_cast<const Color*>(head.data);
 const int area=head.width*head.height;
 alpha.resize(area);
 std::vector<unsigned char> whiteAlpha(area*2,255);
 for(int i=0;i<area;i++)alpha[i]=whiteAlpha[2*i+1]=pixels[i].a;
 mask=LoadTextureFromImage({whiteAlpha.data(),head.width,head.height,1,PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA});
 UnloadImage(head);
 if(!art.id||!mask.id){unload();return;}
 SetTextureFilter(art,TEXTURE_FILTER_BILINEAR);
 SetTextureFilter(mask,TEXTURE_FILTER_BILINEAR);
}
inline bool pick(int direction,int level,float u,float v){
 const int x=int(std::floor(u*1.5f))-origin[direction][0];
 const int y=int(std::floor(v*1.5f))-origin[direction][1];
 return x>=0&&y>=0&&x<Tile&&y<Tile&&alpha[((level-1)*Tile+y)*art.width+direction*Tile+x]>=100;
}
}
