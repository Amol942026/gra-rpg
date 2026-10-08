#pragma once
bool runForestQA(){
 fs::create_directories("qa-v20");std::ofstream report("qa-v20/forest-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 game=Game{};welcome=help=paused=audioOpen=statsOpen=spellBookOpen=false;dragging={};mouse={0,0};refreshDisplayLayout();
 check(!audioSystem.available,"forest diagnostic opens no audio device");
 for(V camera:{V{8,19},V{11,33},V{25,38},V{37,34},V{40,42}}){
  game.player=camera;game.time=.9f;game.messageTime=0;renderScene();BeginDrawing();presentCanvas();Image actual=presentedImage();
  ExportImage(actual,TextFormat("qa-v20/forest-region-%d.png",checks));UnloadImage(actual);EndDrawing();finishFramePacing();
  check(game.walkable(camera),"added forest region renders at a traversable camera location");
 }
 RenderTexture2D sheet=LoadRenderTexture(1300,750);
 const char* directions[]={"E","SE","S","SW","W","NW","N","NE"};
 BeginTextureMode(sheet);ClearBackground(Ink);txt("NOWI MIESZKAŃCY KNIEI — OSIEM KIERUNKÓW",24,16,24,Gold);
 for(int d=0;d<8;d++)center(directions[d],220+d*140.f,56,14,Text);
 for(int type=4;type<EnemyTypeCount;type++){
  txt(enemyInfo(type).name,16,112+(type-4)*155,14,Muted);
  for(int d=0;d<8;d++){AnimationState pose;pose.direction=static_cast<Direction>(d);pose.moving=true;pose.phase=.2f;pose.idleTime=.7f;
   actor_art::draw(type+1,220+d*140.f,225+(type-4)*155,1,pose,.7f,false,false,Kind::Sword);
  }
 }
 EndTextureMode();saveTexture(sheet,"qa-v20/new-monsters-8-directions.png");UnloadRenderTexture(sheet);
 for(int type=4;type<EnemyTypeCount;type++){
  game=Game{};game.props.clear();game.enemies.clear();game.player={20,20};V pos=game.player+V{1,2};
  game.enemies.push_back({501,type,pos,pos,enemyInfo(type).health,enemyInfo(type).health});auto& e=game.enemies.back();e.animation.direction=Direction::SE;
  bool found=false;Vector2 base=screen(pos);
  for(int y=-int(enemyInfo(type).visualHeight);y<5&&!found;y+=3)for(int x=-55;x<55&&!found;x+=3){Vector2 p{base.x+x,base.y+y};auto hit=pickWorld(p);if(hit.kind==HoverKind::Enemy&&hit.index==e.id){mouse=p;found=true;}}
  check(found,"new species is selectable through its actual rendered geometry");
  Inspection info=inspectWorld(pickWorld(mouse));check(info.name==enemyInfo(type).name&&info.maxHp==enemyInfo(type).health&&!info.loot.empty(),"new species exposes correct HP, identity and drop-rate summary in HUD");
  renderScene();saveTexture(canvas,TextFormat("qa-v20/species-%d-hud.png",type));
  game.hit(e,1000);game.allocationRequired=false;game.attributePoints=0;
  found=false;for(int y=-35;y<20&&!found;y+=2)for(int x=-55;x<55&&!found;x+=2){Vector2 p{base.x+x,base.y+y};auto hit=pickWorld(p);if(hit.kind==HoverKind::Corpse&&hit.index==e.id){mouse=p;found=true;}}
  check(found&&rightClickWorldOrInventory()&&game.openedCorpse==e.id,"new species has a pickable persistent corpse opened by PPM");
  e.loot={};e.loot[0]=item(enemyLootTable(type)[1].kind,0,2);
  mouse={lootRect(0).x+20,lootRect(0).y+20};beginDrag(hoverSlot());mouse={bagRect(24).x+20,bagRect(24).y+20};finishDrag();
  check(e.loot[0].empty()&&game.bag[24].count==2&&material(game.bag[24].kind),"new species material drags from corpse into far-right backpack slot");
  renderScene();saveTexture(canvas,TextFormat("qa-v20/species-%d-corpse.png",type));
 }
 for(int type:{4,5}){game=Game{};game.props.clear();game.enemies.clear();Prop prop{game.player,type,1};Vector2 base=screen(prop.pos);
  bool found=false;for(int y=-180;y<-50&&!found;y+=3)for(int x=-65;x<65&&!found;x+=3)found=propContains(prop,{base.x+x,base.y+y});
  check(found&&std::abs(treeShift(prop,45,0)-treeShift(prop,45,2))>1,"new deciduous trees retain animated crown geometry and matching hit masks");
 }
 game=Game{};game.props.clear();game.enemies.clear();game.player={15,15};V pos=game.player+V{1,2};game.enemies.push_back({802,1,pos,pos,0,54});auto& wolf=game.enemies.back();wolf.alive=false;
 wolf.loot[0]=item(Kind::WolfFang,0,2);wolf.loot[1]=item(Kind::WolfFur);game.openCorpse(802);refreshDisplayLayout();mouse={lootRect(0).x+20,lootRect(0).y+20};renderScene();BeginDrawing();presentCanvas();
 Image actual=presentedImage();ExportImage(actual,"qa-v20/wolf-loot.png");UnloadImage(actual);EndDrawing();finishFramePacing();
 check(lootChance(1,wolf.loot[0])==75&&lootChance(1,wolf.loot[1])==55,"wolf loot tooltips use the configured independent chances");
 game=Game{};mouse={0,0};dragging={};report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
