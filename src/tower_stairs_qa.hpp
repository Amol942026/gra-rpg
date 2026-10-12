#pragma once
bool runTowerStairsQA(){
 fs::create_directories("qa-v31");std::ofstream report("qa-v31/stairs-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 auto reset=[](){game=Game{};intro={};towerStory={};clearStoryMenus();developerActive=developerMenuOpen=false;game.enterMageTower();game.dawnQuest=DawnQuest::AtTower;game.messageTime=0;mouse={0,0};};
 auto shot=[](const char* name){std::string path=std::string("qa-v31/")+name+".png";renderScene();saveTexture(canvas,path);};
 reset();refreshDisplayLayout();check(!audioSystem.available,"stairs diagnostic opens no audio device");
 for(int direction:{1,-1}){
  int i=tower_stairs::index(direction);Image im=LoadImageFromTexture(tower_stairs::art[i].texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);
  int left=im.width,top=im.height,right=0,bottom=0,solid=0,clipped=0,probes=0,mismatch=0;
  for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++)if(pixels[y*im.width+x].a>=210){left=std::min(left,x);right=std::max(right,x);top=std::min(top,y);bottom=std::max(bottom,y);solid++;clipped+=x<4||y<4||x>=im.width-4||y>=im.height-4;}
  check(solid>20000&&clipped==0,"full-size cached stone silhouette contains many opaque pixels and has no cut-off edges");
  check((right-left)/tower_stairs::ArtScale>140&&(bottom-top)/tower_stairs::ArtScale>(direction==1?170:80),"stair flight and stairwell have architectural proportions relative to the 90-pixel hero");
  for(int y=7;y<im.height-7;y+=7)for(int x=7;x<im.width-7;x+=7){bool opaque=true,empty=true;for(int dy:{-2,0,2})for(int dx:{-2,0,2}){int a=pixels[(y+dy)*im.width+x+dx].a;opaque&=a>=230;empty&=a<8;}
   if(!opaque&&!empty)continue;Vector2 p{(x+.5f)/tower_stairs::ArtScale-tower_stairs::OriginX,(y+.5f)/tower_stairs::ArtScale-tower_stairs::OriginY};bool hit=tower_stairs::contains(direction,{0,0},p);probes++;mismatch+=opaque?!hit:hit;
  }check(probes>500&&mismatch==0,"mouse silhouette agrees with actual GPU stone geometry and excludes transparent margins");UnloadImageColors(pixels);UnloadImage(im);
 }
 RenderTexture2D detail=LoadRenderTexture(1000,460);BeginTextureMode(detail);ClearBackground(Ink);txt("KAMIENNE SCHODY — SKALA BOHATERA",24,15,24,Gold);
 tower_stairs::draw(1,{290,355},false);hero(392,355,1);tower_stairs::draw(-1,{670,295},false);hero(605,355,1);txt("W GÓRĘ — BIEG I SPOCZNIK",118,405,16,Text);txt("W DÓŁ — STOPNIE W SZYBIE",620,405,16,Text);EndTextureMode();saveTexture(detail,"qa-v31/stairs-detail.png");UnloadRenderTexture(detail);
 for(float scale:{.8f,1.f,1.2f}){prepareCanvas(scale);game.towerFloor=1;game.player={12,15};mouse={0,0};shot(TextFormat("library-stairs-%d",int(scale*100)));}
 refreshDisplayLayout();
 for(int floor=0;floor<TowerFloorCount;floor++){
  game.towerFloor=floor;game.player={12,15};mouse={0,0};shot(TextFormat("floor-%d",floor+1));
  for(int direction:{1,-1})if(game.stairPresent(direction)){
   Vector2 p=screen(towerStairBase(direction));bool found=false,outsideOld=false;
   for(int y=-205;y<=90;y+=4)for(int x=-145;x<=155;x+=4){Vector2 point{p.x+x,p.y+y};WorldHover hit=pickWorld(point);if(hit.kind==HoverKind::TowerStair&&hit.index==direction){found=true;outsideOld|=x<-43||x>43||y<-69||y>15;}}
   check(found&&outsideOld,"real world picking reaches enlarged stairs outside the former miniature rectangle");
  }
 }
 game.towerFloor=1;game.player={12,15};Vector2 p=screen(TowerStairUp);bool found=false;
 for(int y=-190;y<0&&!found;y+=3)for(int x=-140;x<35;x+=3){Vector2 point{p.x+x,p.y+y};auto hit=pickWorld(point);if(hit.kind==HoverKind::TowerStair&&hit.index==1){mouse=point;found=true;break;}}
 check(found&&inspectWorld(pickWorld(mouse)).name=="Schody w górę","hovered stone staircase retains inspection HUD information");shot("upstairs-hover");
 game.player=TowerStairUp+V{1,0};check(nearestTowerInteraction()&&game.towerFloor==2&&game.walkable(game.player),"E ascends from the new full-size staircase to a safe landing");
 game.player=TowerStairDown+V{-1,0};p=screen(TowerStairDown);found=false;
 for(int y=-45;y<80&&!found;y+=3)for(int x=-40;x<145;x+=3){Vector2 point{p.x+x,p.y+y};auto hit=pickWorld(point);if(hit.kind==HoverKind::TowerStair&&hit.index==-1){mouse=point;found=true;break;}}
 check(found&&rightClickWorldOrInventory()&&game.towerFloor==1&&game.walkable(game.player),"PPM on recessed stone shaft returns to previous floor at a safe landing");
 reset();refreshDisplayLayout();report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
