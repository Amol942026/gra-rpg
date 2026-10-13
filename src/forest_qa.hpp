#pragma once
#include "rarity_qa.hpp"
void exportForestOverview(){
 RenderTexture2D map=LoadRenderTexture(1440,900);BeginTextureMode(map);ClearBackground(Ink);
 txt("LAS MCHU — ROZBUDOWANA KNIEJA",30,25,30,Gold);
 txt("72 × 72 pola  ·  46 rozproszonych przeciwników  ·  nowe polany i odnogi ścieżek",32,74,17,Text);
 float factor=1000.f/TerrainWidth;
 DrawTexturePro(terrain.texture,{0,0,float(TerrainWidth),-float(TerrainHeight)},{25,150,1000,TerrainHeight*factor},{0,0},0,WHITE);
 auto point=[&](V p){return Vector2{25+(TerrainOriginX+(p.x-p.y)*40)*factor,150+(90+(p.x+p.y)*20)*factor};};
 for(const auto& prop:game.props){Vector2 p=point(prop.pos);DrawCircleV(p,treeProp(prop.type)?1.4f:1,Fade(treeProp(prop.type)?rgb(36,70,51):rgb(128,133,104),.8f));}
 const Color colors[]={rgb(146,199,114),rgb(203,144,117),rgb(222,215,179),Gold,rgb(181,137,94),rgb(180,203,96),rgb(219,113,98),rgb(129,179,149)};
 for(const auto& e:game.enemies){Vector2 p=point(e.home);DrawCircleV(p,e.type==3?5:3.6f,Ink);DrawCircleV(p,e.type==3?4:2.6f,colors[e.type]);}
 for(size_t i=0;i<ForestGlades.size();i++){
  const auto& glade=ForestGlades[i];Vector2 p=point(glade.center);ellipseOutline(p.x,p.y,7,4,Gold);
  txt(i==0?"START":glade.name,p.x+11,p.y-7,14,i==0?Text:Gold);
 }
 DrawCircleV(point({8,19}),3.5f,Text);
 panel({1060,142,350,520},Panel);txt("MIESZKAŃCY LASU",1083,163,18,Gold);
 for(int type=0;type<ForestEnemyTypeCount;type++){
  int count=int(std::count_if(game.enemies.begin(),game.enemies.end(),[&](const Enemy& e){return e.type==type;}));
  DrawCircleV({1091,215+type*39.f},4,colors[type]);txt(TextFormat("%s · %d",enemyInfo(type).name,count),1107,204+type*39.f,15,Text);
 }
 txt("Strażnik czeka w starych ruinach.",1083,548,15,Muted);
 txt("Jasne linie pokazują leśne trakty.",1083,582,15,Muted);
 txt("Złote znaczniki: polany i ruiny.",1083,616,15,Muted);
 txt("Między miejscami startowymi wrogów jest co najmniej 8 pól.",32,738,19,Text);
 txt("Las ma 2,25 raza większy obszar; początkowa przesieka pozostaje spokojna.",32,779,17,Muted);
 EndTextureMode();saveTexture(map,"qa-v25/forest-overview.png");UnloadRenderTexture(map);
}
bool runForestQA(){
 fs::create_directories("qa-v25");std::ofstream report("qa-v25/forest-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 game=Game{};welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;dragging={};mouse={0,0};refreshDisplayLayout();
 check(!audioSystem.available,"forest diagnostic opens no audio device");
 check(IsRenderTextureValid(terrain)&&terrain.texture.width==TerrainWidth&&IsTextureValid(minimapTerrain),"GPU loads the enlarged terrain and the cached minimap successfully");
 exportForestOverview();
 for(const auto& glade:ForestGlades){V camera=glade.center;
  game.player=camera;game.time=.9f;game.messageTime=0;renderScene();BeginDrawing();presentCanvas();Image actual=presentedImage();
  ExportImage(actual,TextFormat("qa-v25/forest-region-%d.png",checks));UnloadImage(actual);EndDrawing();finishFramePacing();
  check(game.walkable(camera),"added forest region renders at a traversable camera location");
 }
 for(V camera:{V{35,42},V{66,64},V{68,68}}){
  game.player=camera;game.time=1;renderScene();BeginDrawing();presentCanvas();Image actual=presentedImage();
  ExportImage(actual,TextFormat("qa-v25/forest-region-%d.png",checks));UnloadImage(actual);EndDrawing();finishFramePacing();
  check(game.walkable(camera),"expanded crossroads and distant border show accessible land beyond the former map");
 }
 RenderTexture2D sheet=LoadRenderTexture(1300,750);
 const char* directions[]={"E","SE","S","SW","W","NW","N","NE"};
 BeginTextureMode(sheet);ClearBackground(Ink);txt("NOWI MIESZKAŃCY KNIEI — OSIEM KIERUNKÓW",24,16,24,Gold);
 for(int d=0;d<8;d++)center(directions[d],220+d*140.f,56,14,Text);
 for(int type=4;type<ForestEnemyTypeCount;type++){
  txt(enemyInfo(type).name,16,112+(type-4)*155,14,Muted);
  for(int d=0;d<8;d++){AnimationState pose;pose.direction=static_cast<Direction>(d);pose.moving=true;pose.phase=.2f;pose.idleTime=.7f;
   actor_art::draw(type+1,220+d*140.f,225+(type-4)*155,1,pose,.7f,false,false,Kind::Sword);
  }
 }
 EndTextureMode();saveTexture(sheet,"qa-v25/new-monsters-8-directions.png");UnloadRenderTexture(sheet);
 for(int type=4;type<ForestEnemyTypeCount;type++){
  game=Game{};game.props.clear();game.enemies.clear();game.player={20,20};V pos=game.player+V{1,2};
  game.enemies.push_back({501,type,pos,pos,enemyInfo(type).health,enemyInfo(type).health});auto& e=game.enemies.back();e.animation.direction=Direction::SE;
  bool found=false;Vector2 base=screen(pos);
  for(int y=-int(enemyInfo(type).visualHeight);y<5&&!found;y+=3)for(int x=-55;x<55&&!found;x+=3){Vector2 p{base.x+x,base.y+y};auto hit=pickWorld(p);if(hit.kind==HoverKind::Enemy&&hit.index==e.id){mouse=p;found=true;}}
  check(found,"new species is selectable through its actual rendered geometry");
  Inspection info=inspectWorld(pickWorld(mouse));check(info.name==enemyInfo(type).name&&info.maxHp==enemyInfo(type).health&&!info.loot.empty(),"new species exposes correct HP, identity and drop-rate summary in HUD");
  renderScene();saveTexture(canvas,TextFormat("qa-v25/species-%d-hud.png",type));
  game.hit(e,1000);game.allocationRequired=false;game.attributePoints=0;
  found=false;for(int y=-35;y<20&&!found;y+=2)for(int x=-55;x<55&&!found;x+=2){Vector2 p{base.x+x,base.y+y};auto hit=pickWorld(p);if(hit.kind==HoverKind::Corpse&&hit.index==e.id){mouse=p;found=true;}}
  check(found&&rightClickWorldOrInventory()&&game.openedCorpse==e.id,"new species has a pickable persistent corpse opened by PPM");
  e.loot={};e.loot[0]=item(enemyLootTable(type)[1].kind,0,2);
  mouse={lootRect(0).x+20,lootRect(0).y+20};beginDrag(hoverSlot());mouse={bagRect(24).x+20,bagRect(24).y+20};finishDrag();
  check(e.loot[0].empty()&&game.bag[24].count==2&&material(game.bag[24].kind),"new species material drags from corpse into far-right backpack slot");
  renderScene();saveTexture(canvas,TextFormat("qa-v25/species-%d-corpse.png",type));
 }
 for(int type:{4,5}){game=Game{};game.props.clear();game.enemies.clear();Prop prop{game.player,type,1};Vector2 base=screen(prop.pos);
  bool found=false;for(int y=-180;y<-50&&!found;y+=3)for(int x=-65;x<65&&!found;x+=3)found=propContains(prop,{base.x+x,base.y+y});
  check(found&&std::abs(treeShift(prop,45,0)-treeShift(prop,45,2))>1,"new deciduous trees retain animated crown geometry and matching hit masks");
 }
 game=Game{};game.props.clear();game.enemies.clear();game.player={15,15};V pos=game.player+V{1,2};game.enemies.push_back({802,1,pos,pos,0,54});auto& wolf=game.enemies.back();wolf.alive=false;
 wolf.loot[0]=item(Kind::WolfFang,0,2);wolf.loot[1]=item(Kind::WolfFur);game.openCorpse(802);refreshDisplayLayout();mouse={lootRect(0).x+20,lootRect(0).y+20};renderScene();BeginDrawing();presentCanvas();
 Image actual=presentedImage();ExportImage(actual,"qa-v25/wolf-loot.png");UnloadImage(actual);EndDrawing();finishFramePacing();
 check(lootChance(1,wolf.loot[0])==60&&lootChance(1,wolf.loot[1])==60,"wolf loot tooltips use the configured independent chances");
 game=Game{};mouse={0,0};dragging={};report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return runRarityQA()&&failures==0;
}
