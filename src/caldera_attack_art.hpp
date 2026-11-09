#pragma once
#include "generated/caldera_attack_motion.hpp"

namespace caldera_attack_art {
enum class Clip {Cleave,Breath,Eruption,Cinder};
inline Texture2D atlas{};

inline void load(){
 if(atlas.id)return;
 atlas=LoadTexture("assets/volcano/caldera-attack-fx.png");
 if(atlas.id&&(atlas.width!=2048||atlas.height!=512)){UnloadTexture(atlas);atlas={};}
 if(atlas.id)SetTextureFilter(atlas,TEXTURE_FILTER_BILINEAR);
}
inline void unload(){if(atlas.id)UnloadTexture(atlas);atlas={};}

inline bool draw(Clip clip,Vector2 at,Vector2 size,float angle,float phase,float opacity=1,Vector2 anchor={.5f,.5f}){
 if(!atlas.id)return false;
 const auto key=caldera_attack_motion::effect(int(clip),phase);
 const float width=size.x*key.width,height=size.y*key.height;
 DrawTexturePro(atlas,{float(int(clip)*512+1),1,510,510},{at.x,at.y,width,height},
                {width*anchor.x,height*anchor.y},angle+key.angle,Fade(WHITE,std::clamp(opacity*key.opacity,0.f,1.f)));
 return true;
}
}
