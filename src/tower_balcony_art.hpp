#pragma once

namespace tower_balcony {
Texture2D valley{},back{},front{};
void load(){
 valley=LoadTexture("assets/sprites/tower-valley.png");
 back=LoadTexture("assets/sprites/tower-balcony-back.png");
 front=LoadTexture("assets/sprites/tower-balcony-front.png");
 for(auto texture:{valley,back,front})if(texture.id)SetTextureFilter(texture,TEXTURE_FILTER_BILINEAR);
}
void unload(){for(auto texture:{valley,back,front})if(texture.id)UnloadTexture(texture);valley=back=front={};}
void panorama(){if(valley.id)DrawTexturePro(valley,{0,0,float(valley.width),float(valley.height)},{0,0,WorldW,H},{0,0},0,WHITE);}
void terrace(bool foreground){
 auto texture=foreground?front:back;if(!texture.id)return;
 Vector2 anchor=screen(towerBalconyPoint(6.1f));
 DrawTexturePro(texture,{0,0,480,320},{anchor.x-64,anchor.y-160,480,320},{0,0},0,WHITE);
}
void sign(){
 Vector2 entrance=screen(towerBalconyPoint(6.1f));
 if(entrance.x<WorldW-90&&entrance.x>0){
  center("BALKON",entrance.x+100,entrance.y-135,20,Gold,true);
  center("Dolina Świtu",entrance.x+100,entrance.y-111,15,Text);
 }
}
}
