#pragma once
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
void makeTowerRoomArt(){for(int i=0;i<TowerFloorCount;i++){towerFloorArt[i]=LoadRenderTexture(1024,600);BeginTextureMode(towerFloorArt[i]);ClearBackground(BLANK);drawTowerGround({512,300},i);EndTextureMode();SetTextureFilter(towerFloorArt[i].texture,TEXTURE_FILTER_BILINEAR);}}
void unloadTowerRoomArt(){for(auto& target:towerFloorArt)if(target.id)UnloadRenderTexture(target);}
void towerBook(float x,float y,float size,Color color){rect(x,y,9*size,25*size,shade(color,.65f));rect(x+size,y+size,6*size,23*size,color);line(x+size,y+5*size,x+7*size,y+5*size,Gold,size);line(x+size,y+20*size,x+7*size,y+20*size,Gold,size);}
void drawThemedFurniture(const TowerFurniture& object,float x,float y){
 int type=object.type;Color accent=towerAccent(game.towerFloor);
 if(type==3){
  DrawRectangleRounded({x-60,y-145,120,148},.15f,8,rgb(73,60,72));DrawRectangleRounded({x-46,y-112,92,98},.35f,12,rgb(21,16,24));
  for(int i=0;i<4;i++)line(x-58,y-130+i*35,x+58,y-130+i*35,rgb(128,104,100),2);
  line(x-32,y-10,x+28,y-21,rgb(91,48,26),8);line(x-29,y-24,x+33,y-10,rgb(113,65,32),6);
  intro_art::glow(x,y-36,86,85,rgb(255,131,45,35));for(int i=-2;i<=2;i++){float h=24+11*std::sin(game.time*4+i*2);poly({{x+i*11-10,y-12},{x+i*11,y-20-h},{x+i*11+10,y-12}},rgb(220+std::abs(i)*10,100+std::abs(i)*16,38));ellipse(x+i*11,y-18,5,11,Gold);}
  rect(x-68,y-148,136,10,rgb(150,113,100));for(int i=-1;i<=1;i++)icon(item(Kind::Crystal),x+i*31,y-163,.34f);
 }else if(type==5){
  DrawRectangleRounded({x-58,y-154,116,161},.07f,6,rgb(49,33,34));rect(x-52,y-149,104,151,rgb(24,24,33));
  for(int row=0;row<4;row++){float yy=y-129+row*34;for(int b=0;b<9;b++){unsigned h=Game::hash(b+game.towerFloor,row);Color c=rgb(70+h%74,65+(h>>5)%65,80+(h>>9)%71);towerBook(x-48+b*10,yy,.85f,c);}rect(x-57,yy+23,114,5,rgb(124,85,56));line(x-54,yy+24,x+53,yy+24,Gold);}
  rect(x-58,y-155,116,7,rgb(154,111,74));rect(x-58,y-152,7,159,rgb(109,73,52));rect(x+51,y-152,7,159,rgb(109,73,52));
 }else if(type==6){
  line(x,y-8,x,y-54,rgb(135,93,66),13);ellipse(x,y,29,12,rgb(122,88,72));
  poly({{x-37,y-71},{x,y-90},{x+39,y-70},{x,y-49}},rgb(149,104,73));
  poly({{x-30,y-72},{x-3,y-86},{x,y-59},{x-25,y-50}},rgb(233,213,172));poly({{x-3,y-86},{x+30,y-72},{x+26,y-49},{x,y-59}},rgb(217,198,159));
  for(int i=0;i<5;i++){line(x-24,y-71+i*3,x-7,y-79+i*3,rgb(124,95,78));line(x+5,y-75+i*3,x+24,y-66+i*3,rgb(124,95,78));}
  intro_art::glow(x,y-69,59,32,Fade(accent,.08f));
 }else if(type==7){
  for(float a:{-1.f,1.f})line(x+a*24,y-4,x+a*32,y+10,rgb(66,61,66),7);
  ellipse(x,y-27,42,36,rgb(42,45,54));ellipse(x-9,y-31,23,27,rgb(72,75,78));ellipse(x,y-57,44,21,rgb(162,164,152));ellipse(x,y-58,38,16,rgb(50,94,65));
  for(int i=0;i<7;i++){float a=i*2.7f+game.time;ellipse(x+std::sin(a)*29,y-57+std::cos(a)*9,2+std::sin(a*.7f),2,Fade(Green,.75f));}
  for(int i=0;i<5;i++){float age=std::fmod(game.time*.35f+i*.19f,1.f);ellipse(x+std::sin(age*5+i)*14,y-70-age*50,7+age*8,4+age*6,Fade(rgb(147,215,172),.12f*(1-age)));}
  line(x-30,y-13,x+28,y-13,rgb(117,77,31),5);intro_art::glow(x,y-16,66,32,rgb(211,114,48,38));
 }else if(type==8){
  for(int i=-1;i<=1;i++){
   ellipse(x+i*19,y-7,16,10,rgb(144,88,62));poly({{x+i*19-14,y-8},{x+i*19+14,y-8},{x+i*19+9,y+12},{x+i*19-9,y+12}},rgb(105,65,52));
   float sway=std::sin(game.time*1.7f+i)*3;line(x+i*19,y-10,x+i*19+sway,y-48,rgb(100,167,92),2);
   for(int j=0;j<4;j++){float side=j%2?1.f:-1.f;ellipse(x+i*19+sway+side*7,y-24-j*7,10,4,rgb(76+j*12,130+j*11,78));}intro_art::star(x+i*19+sway,y-51,4,rgb(199,185,120));
  }
 }else if(type==9||type==12||type==4){
  ellipse(x,y,34,15,rgb(70,65,85));DrawRectangleRounded({x-23,y-50,46,47},.18f,8,rgb(97,88,115));ellipse(x,y-51,34,15,rgb(169,153,173));
  if(type==4){ellipse(x,y-54,29,11,rgb(59,127,145));for(int i=0;i<3;i++)ellipseOutline(x,y-54,8+i*7+std::sin(game.time*2+i)*2,4+i*3,Fade(Blue,.5f));intro_art::glow(x,y-62,60,40,Fade(Blue,.2f));}
  else if(type==9){int index=int(&object-towerFurnishings(game.towerFloor).data());bool lit=game.towerRuneMask&(1<<index);Color c=lit?accent:rgb(101,96,117);diamond(x,y-91,16,29,c);line(x-8,y-91,x+7,y-102,lit?Text:Muted,2);line(x+7,y-102,x-2,y-75,lit?Text:Muted,2);if(lit)intro_art::glow(x,y-91,58,66,Fade(c,.13f));}
  else{float lift=std::sin(game.time*1.5f)*4;icon(item(Kind::DawnShard),x,y-90+lift,1.3f);intro_art::glow(x,y-89+lift,66,70,Fade(Gold,game.towerRuneMask==7?.18f:.08f));for(int i=1;i<5;i++){float a=i*2*PI/5+game.time*.3f;diamond(x+std::cos(a)*47,y-87+std::sin(a)*21,5,10,rgb(112,115,145,140));}}
 }else if(type==10){
  line(x,y-20,x-32,y+5,rgb(139,97,65),6);line(x,y-20,x+33,y+4,rgb(139,97,65),6);line(x,y-20,x,y+17,rgb(139,97,65),6);line(x,y-22,x,y-78,rgb(148,129,100),7);
  line(x-32,y-67,x+36,y-118,rgb(93,101,129),24);line(x-30,y-72,x+37,y-122,rgb(187,164,111),5);ellipse(x+36,y-118,14,17,rgb(167,141,89));ellipse(x+39,y-121,9,12,rgb(76,145,190));ellipse(x+40,y-125,4,6,rgb(160,213,236));
 }else if(type==11){
  ellipse(x,y,31,15,rgb(104,88,104));line(x,y-4,x,y-58,rgb(169,139,88),8);Color gold=rgb(211,182,123);
  intro_art::arc(x,y-83,43,17,0,2*PI,gold,2);ellipseOutline(x,y-83,25,40,gold);line(x-37,y-103,x+37,y-63,gold,2);ellipse(x,y-83,13,14,rgb(64,105,156));
  for(int i=0;i<4;i++){float a=game.time*.5f+i*PI*.5f;ellipse(x+std::cos(a)*41,y-83+std::sin(a)*17,4,4,i%2?Gold:Blue);}
 }
}
void drawTowerStairs(int direction){
 Vector2 p=screen(towerStairBase(direction));bool hover=hovered.kind==HoverKind::TowerStair&&hovered.index==direction;
 tower_stairs::draw(direction,p,hover);
}
void drawTowerStairLabels(){
 for(int direction:{1,-1})if(game.stairPresent(direction)){
  Vector2 p=screen(towerStairBase(direction));bool hover=hovered.kind==HoverKind::TowerStair&&hovered.index==direction;
  float x=p.x+(direction==1?-20:TowerStairRun*20),y=p.y+(direction==1?29:92);
  center(direction==1?"SCHODY W GÓRĘ":"SCHODY W DÓŁ",x,y,14,hover?Text:direction==1?Gold:Blue);
  if(game.stairReachable(direction))center("E / LPM / PPM",x,y+21,14,Text);
 }
}
