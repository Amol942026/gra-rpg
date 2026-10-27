#pragma once
#include "tower_balcony_art.hpp"
Color towerAccent(int floor){const Color colors[]={Gold,rgb(157,183,214),rgb(137,207,143),rgb(184,139,248),rgb(118,189,247)};return colors[std::clamp(floor,0,4)];}
std::array<RenderTexture2D,TowerFloorCount> towerFloorArt{};
void drawTowerGround(Vector2 c,int floor){
 float rx=TowerRadius*56.5685f,ry=TowerRadius*28.2843f;Color accent=towerAccent(floor);
 ellipse(c.x,c.y+24,rx+17,ry+15,rgb(12,12,25));ellipse(c.x,c.y+13,rx+10,ry+8,rgb(55,45,72));
 ellipse(c.x,c.y,rx+8,ry+4,shade(accent,.7f));ellipse(c.x,c.y,rx,ry,rgb(70,64,87));
 const Color tones[]={rgb(89,71,69),rgb(62,73,88),rgb(59,82,70),rgb(67,55,92),rgb(50,69,93)};
 for(int x=5;x<=19;x++)for(int y=5;y<=19;y++){
  if(distance({float(x),float(y)},TowerCenter)>TowerRadius-.75f)continue;
  float px=c.x+(x-y)*40,py=c.y+(x+y-24)*20;unsigned h=Game::hash(x,y);
  diamond(px,py,39.5f,19.7f,shade(tones[floor],.91f+(h%20)*.01f));line(px-38,py,px,py-19,Fade(accent,.13f));
 }
 for(int ring=0;ring<3;ring++)ellipseOutline(c.x,c.y,rx-12-ring*6,ry-6-ring*3,Fade(accent,.35f-ring*.07f));
 if(floor==0||floor==1){
  diamond(c.x,c.y,178,88,floor==0?rgb(104,51,43):rgb(45,60,87));
  Vector2 v[]={{c.x,c.y-80},{c.x+162,c.y},{c.x,c.y+80},{c.x-162,c.y}};
  for(int i=0;i<4;i++)DrawLineEx(v[i],v[(i+1)%4],2,Fade(accent,.7f));
  for(int i=-3;i<=3;i++)diamond(c.x+i*27,c.y,6,9,Fade(accent,.45f));
 }else if(floor==2){
  ellipseOutline(c.x,c.y,90,45,Fade(accent,.4f));for(int i=0;i<6;i++){float a=i*PI/3;diamond(c.x+std::cos(a)*79,c.y+std::sin(a)*40,7,10,Fade(accent,.5f));}
 }else{
  ellipseOutline(c.x,c.y,126,63,Fade(accent,.6f));ellipseOutline(c.x,c.y,113,56,Fade(accent,.3f));
  for(int i=0;i<16;i++){float a=i*PI/8;intro_art::star(c.x+std::cos(a)*115,c.y+std::sin(a)*57,3,accent);if(i%2==0)line(c.x+std::cos(a)*103,c.y+std::sin(a)*51,c.x+std::cos(a+PI*.5f)*103,c.y+std::sin(a+PI*.5f)*51,Fade(accent,.14f));}
 }
}
void makeTowerRoomArt(){tower_balcony::load();for(int i=0;i<TowerFloorCount;i++){towerFloorArt[i]=LoadRenderTexture(1024,600);BeginTextureMode(towerFloorArt[i]);ClearBackground(BLANK);drawTowerGround({512,300},i);EndTextureMode();SetTextureFilter(towerFloorArt[i].texture,TEXTURE_FILTER_BILINEAR);}}
void unloadTowerRoomArt(){tower_balcony::unload();for(auto& target:towerFloorArt)if(target.id)UnloadRenderTexture(target);}
void drawTowerStairs(int direction){
 Vector2 p=screen(towerStairBase(direction));bool hover=hovered.kind==HoverKind::TowerStair&&hovered.index==direction;
 tower_stairs::draw(direction,p,hover);
}
