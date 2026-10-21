#pragma once
#include "sprint_qa.hpp"
#include "fallen_qa.hpp"

bool runGraphicsQA(){
 fs::create_directories("qa-v21");std::ofstream report("qa-v21/graphics-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 check(!audioSystem.available,"graphics diagnostic opens no audio device");
 verifySprintArt(check);
 verifyFallenArt(check);
 RenderTexture2D sample=LoadRenderTexture(240,260);
 auto verify=[&](int species,int dir,float roll,float strike){
  AnimationState pose;pose.direction=static_cast<Direction>(dir);pose.moving=roll<0;pose.phase=.17f;pose.idleTime=.7f;
  float attack=strike<0?0:std::sin(strike*PI);
  BeginTextureMode(sample);ClearBackground(BLANK);
  actor_art::draw(species,120,215,1,pose,attack,species==0,species==0,Kind::Sword,false,BLANK,nullptr,roll,strike);EndTextureMode();
  Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);
  int opaque=0,clipped=0,interior=0,mismatch=0;
  for(int y=0;y<260;y++)for(int x=0;x<240;x++)if(pixels[y*240+x].a>230){opaque++;clipped+=x<2||y<2||x>=238||y>=258;}
  for(int y=6;y<254;y+=7)for(int x=6;x<234;x+=7){
   bool solid=true,empty=true;for(int dy:{-3,0,3})for(int dx:{-3,0,3}){int alpha=pixels[(y+dy)*240+x+dx].a;solid&=alpha>230;empty&=alpha<8;}
   if(!solid&&!empty)continue;Vector2 point{float(x)+.5f,float(y)+.5f};
   bool hit=actor_art::draw(species,120,215,1,pose,attack,species==0,species==0,Kind::Sword,false,BLANK,&point,roll,strike);
   interior+=solid;mismatch+=solid?!hit:hit;
  }
  // Curved ribs and thin insect legs can lie entirely between the old 7 px
  // grid lines. Keep the required interior probes and find real opaque pixels
  // instead of treating a sparse-grid miss as a picking failure.
  if(interior<=2)for(int y=6;y<254&&interior<4;y+=3)for(int x=6;x<234&&interior<4;x+=3){
   bool solid=true;for(int dy:{-2,0,2})for(int dx:{-2,0,2})solid&=pixels[(y+dy)*240+x+dx].a>230;
   if(!solid)continue;Vector2 point{x+.5f,y+.5f};
   bool hit=actor_art::draw(species,120,215,1,pose,attack,species==0,species==0,Kind::Sword,false,BLANK,&point,roll,strike);
   interior++;mismatch+=!hit;
  }
  report<<"Species="<<species<<"; direction="<<dir<<"; roll="<<roll<<"; strike="<<strike<<"; opaque="<<opaque<<"; interior="<<interior<<"; mismatch="<<mismatch<<'\n';
  check(opaque>130&&clipped==0,"detailed actor pose has a complete visible silhouette without clipped parts");
  check(interior>2&&mismatch==0,"GPU-rasterized body interior and empty space match pointer geometry");
  UnloadImageColors(pixels);UnloadImage(im);
 };
 for(int species=0;species<=EnemyTypeCount;species++)for(int d=0;d<8;d++)verify(species,d,-1,-1);
 for(int d=0;d<8;d++){verify(0,d,-1,.5f);verify(0,d,.2f,-1);verify(0,d,.65f,-1);}
 UnloadRenderTexture(sample);
 RenderTexture2D sheet=LoadRenderTexture(1300,(EnemyTypeCount+1)*190+100);BeginTextureMode(sheet);ClearBackground(Ink);
 txt("SZCZEGÓŁOWE SYLWETKI — OSIEM KIERUNKÓW",24,16,24,Gold);
 const char* directions[]={"E","SE","S","SW","W","NW","N","NE"};
 for(int d=0;d<8;d++)center(directions[d],220+d*140.f,56,14,Text);
 for(int species=0;species<=EnemyTypeCount;species++){
  txt(species==0?"Wędrowiec":enemyInfo(species-1).name,16,110+species*190,14,Muted);
  for(int d=0;d<8;d++){AnimationState pose;pose.direction=static_cast<Direction>(d);pose.moving=true;pose.phase=.17f;pose.idleTime=.7f;
   actor_art::draw(species,220+d*140.f,230+species*190,1,pose,0,species==0,species==0);}
 }
 EndTextureMode();saveTexture(sheet,"qa-v21/all-actors-8-directions.png");UnloadRenderTexture(sheet);
 fs::create_directories("qa-v24/hero");
 RenderTexture2D heroSheet=LoadRenderTexture(1680,1310);BeginTextureMode(heroSheet);ClearBackground(Ink);
 txt("WĘDROWIEC — NOWA SYLWETKA I ANIMACJE",24,18,25,Gold);
 const char* poses[]={"KAPTUR","CHÓD","MIECZ / TARCZA","ŁUK","UNIK"};
 for(int d=0;d<8;d++)center(directions[d],240+d*195.f,67,14,Text);
 for(int row=0;row<5;row++){
  txt(poses[row],18,146+row*240,14,Muted);
  for(int d=0;d<8;d++){
   AnimationState state;state.direction=static_cast<Direction>(d);state.idleTime=.7f;state.moving=row==1;state.phase=.17f;
   float x=240+d*195.f,y=286+row*240;ellipse(x,y+2,49,17,rgb(23,34,36));
   actor_art::draw(0,x,y,1.9f,state,row==2?1:0,row==2,row==2||row==4,row==3?Kind::Bow:Kind::Sword,
                   false,BLANK,nullptr,row==4?.45f:-1,row==2?.5f:-1,row==3?.85f:-1);
  }
 }
 EndTextureMode();saveTexture(heroSheet,"qa-v24/hero/poses-8-directions.png");UnloadRenderTexture(heroSheet);
 for(int type=0;type<PropTypeCount;type++){
  Image im=LoadImageFromTexture(propArt[type].texture);Color* pixels=LoadImageColors(im);int opaque=0,clipped=0;
  for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++)if(pixels[y*im.width+x].a>230){opaque++;clipped+=x<2||x>=im.width-2||y<2||y>=im.height-2;}
  check(opaque>250&&clipped==0,"baked foliage, stone and ruin detail fits its full texture and pointer mask");
  UnloadImageColors(pixels);UnloadImage(im);
 }
 for(int k=1;k<ItemKindCount;k++){
  RenderTexture2D itemSample=LoadRenderTexture(64,64);BeginTextureMode(itemSample);ClearBackground(BLANK);icon(item(static_cast<Kind>(k),1),32,32);EndTextureMode();
  Image im=LoadImageFromTexture(itemSample.texture);Color* pixels=LoadImageColors(im);int opaque=0,clipped=0;
  for(int y=0;y<64;y++)for(int x=0;x<64;x++)if(pixels[y*64+x].a>230){opaque++;clipped+=x<2||x>=62||y<2||y>=62;}
  check(opaque>25&&clipped==0,"detailed item icon is visible and fits inside backpack bounds");
  UnloadImageColors(pixels);UnloadImage(im);UnloadRenderTexture(itemSample);
 }
 game=Game{};welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;mouse={0,0};dragging={};refreshDisplayLayout();
 game.messageTime=0;renderScene();BeginDrawing();presentCanvas();Image actual=presentedImage();ExportImage(actual,"qa-v21/detailed-forest.png");ExportImage(actual,"qa-v24/hero/forest.png");UnloadImage(actual);EndDrawing();finishFramePacing();
 report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
