#pragma once
namespace hero_portrait {
inline bool enabled=true,ready=false;
inline RenderTexture2D art{};
inline std::array<Item,EquipSize> equipment{};
inline float pixelScale=-1;
inline int arrows=-1;
inline unsigned builds=0;
void unload(){if(art.id)UnloadRenderTexture(art);art={};ready=false;pixelScale=-1;}
void prepare(){
 if(!enabled)return;
 bool same=ready&&pixelScale==renderScale&&arrows==std::min(3,game.ammoCount());
 for(int i=0;i<EquipSize;i++)same&=equipment[i].kind==game.equipment[i].kind&&equipment[i].tier==game.equipment[i].tier&&equipment[i].empty()==game.equipment[i].empty();
 if(same)return;
 if(!canvas.id)return;
 static const auto copy=reinterpret_cast<void(*)(unsigned,int,int,int,int,int,int,int)>(glfwGetProcAddress("glCopyTexSubImage2D"));
 if(!copy)return;
 int left=int(std::round(1081*renderScale)),top=int(std::round(69*renderScale));
 int width=int(std::round(61*renderScale)),height=int(std::round(66*renderScale));
 if(!art.id||art.texture.width!=width||art.texture.height!=height){if(art.id)UnloadRenderTexture(art);art=render_targets::colorOnly(width,height);SetTextureFilter(art.texture,TEXTURE_FILTER_POINT);}
 bool depth=actor_art::sceneDepth;actor_art::sceneDepth=false;
 // Use the exact native canvas transform, then copy only the portrait pixels
 // on the GPU. A tiny translated viewport changes subpixel rasterization.
 BeginTextureMode(canvas);BeginMode2D({{0,0},{0,0},0,renderScale});clip(1081,69,61,66);
 DrawRectangleRec({1081,69,61,66},rgb(37,54,51));hero(1110,137,.8f,true);rlDrawRenderBatchActive();
 rlEnableTexture(art.texture.id);copy(0x0DE1,0,0,0,left,canvas.texture.height-top-height,width,height);rlDisableTexture();
 EndScissorMode();EndMode2D();EndTextureMode();actor_art::sceneDepth=depth;
 equipment=game.equipment;arrows=std::min(3,game.ammoCount());pixelScale=renderScale;ready=true;builds++;
}
void draw(){
 if(!enabled||!ready){hero(1110,137,.8f,true);return;}
 float x=std::round(1081*renderScale)/renderScale,y=std::round(69*renderScale)/renderScale;
 // These are finished canvas pixels, including the panel beneath the hero.
 // Copy RGBA rather than applying edge alpha to the panel a second time.
 rlSetBlendFactors(RL_ONE,RL_ZERO,RL_FUNC_ADD);BeginBlendMode(BLEND_CUSTOM);
 DrawTexturePro(art.texture,{0,0,float(art.texture.width),-float(art.texture.height)},{x,y,art.texture.width/renderScale,art.texture.height/renderScale},{0,0},0,WHITE);
 EndBlendMode();
}
}
