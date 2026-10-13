#pragma once
template<class Check>void verifyFallenArt(Check check){
 fs::create_directories("qa-v39");check(fallen_art::prepared==EnemyTypeCount*8,"all corpse textures and pointer masks are prepared before combat starts");
 int prepared=fallen_art::prepared;
 RenderTexture2D sample=LoadRenderTexture(240,160);
 for(int type=0;type<EnemyTypeCount;type++)for(int direction=0;direction<8;direction++){
  Enemy e{100+type,type,{},{},0,100};e.alive=false;e.animation.direction=Direction(direction);
  BeginTextureMode(sample);ClearBackground(BLANK);corpseArt(e,120,103);EndTextureMode();
  Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);int opaque=0,clipped=0,bad=0,probes=0;
  for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++){bool solid=pixels[y*im.width+x].a>=240;opaque+=solid;clipped+=solid&&(x<3||y<3||x>=im.width-3||y>=im.height-3);}
  for(int y=5;y<im.height-5;y+=5)for(int x=5;x<im.width-5;x+=5){
   bool solid=true,empty=true;for(int dy:{-2,0,2})for(int dx:{-2,0,2}){int alpha=pixels[(y+dy)*im.width+x+dx].a;solid&=alpha>=240;empty&=alpha<4;}
   if(!solid&&!empty)continue;Vector2 point{x+.5f,y+.5f};bool picked=corpseArt(e,120,103,BLANK,&point);bad+=solid?!picked:picked;probes++;
  }
  check(opaque>100&&clipped==0,"detailed fallen body stays complete and visible in all eight directions");
  check(probes>900&&bad==0,"actual cached corpse pixels agree with physical hover and looting geometry");
  UnloadImageColors(pixels);UnloadImage(im);
 }
 UnloadRenderTexture(sample);check(fallen_art::prepared==prepared,"drawing and probing corpses never rebuilds meshes or uploads textures during combat");
 RenderTexture2D gallery=LoadRenderTexture(1400,860);BeginTextureMode(gallery);ClearBackground(Ink);
 txt("POKONANE BESTIE — NOWE POZY I SZCZEGÓŁY",24,20,27,Gold);
 for(int type=0;type<EnemyTypeCount;type++){
  float x=25+(type%4)*345.f,y=87+(type/4)*247.f;panel({x,y,330,227},Panel);
  center(enemyInfo(type).name,x+165,y+15,17,Gold);
  const auto& texture=fallen_art::sprites[type][int(Direction::SE)].body;
  DrawTexturePro(texture,{0,0,float(texture.width),float(texture.height)},{x-8,y-17,float(fallen_art::Width)*1.8f,float(fallen_art::Height)*1.8f},{0,0},0,WHITE);
  center("Ciało pozostaje źródłem łupu",x+165,y+191,14,Muted);
 }
 EndTextureMode();saveTexture(gallery,"qa-v39/fallen-bestiary.png");UnloadRenderTexture(gallery);
}
