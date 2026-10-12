#pragma once
// Rendering checks read back GPU pixels; gameplay interactions use the real picker.
bool runTowerFurnitureQA(){
 using namespace tower_furniture;
 fs::create_directories("qa-v32");std::ofstream report("qa-v32/tower-art-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 auto reset=[](){game=Game{};intro={};towerStory={};clearStoryMenus();developerActive=developerMenuOpen=false;game.enterMageTower();game.dawnQuest=DawnQuest::AtTower;game.messageTime=0;game.player={12,15};game.time=2.7f;mouse={0,0};};
 auto shot=[](const std::string& name){renderScene();saveTexture(canvas,"qa-v32/"+name+".png");};
 reset();refreshDisplayLayout();check(!audioSystem.available,"furniture diagnostic opens no audio device");
 for(int id=0;id<SpriteCount;id++){
  const auto& s=sprites[id];Image im=LoadImageFromTexture(s.art.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);
  int solid=0,clipped=0,mismatch=0;std::vector<unsigned> colors;
  for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++){
   const auto& c=pixels[y*im.width+x];bool opaque=c.a>=210;solid+=opaque;clipped+=opaque&&(x<4||y<4||x>=im.width-4||y>=im.height-4);
   mismatch+=(s.mask[y*s.w+x]!=0)!=opaque;if(opaque&&x%3==0&&y%3==0)colors.push_back((unsigned(c.r)<<16)|(unsigned(c.g)<<8)|c.b);
  }
  std::sort(colors.begin(),colors.end());auto unique=std::unique(colors.begin(),colors.end())-colors.begin();
  check(solid>100&&clipped==0,"sprite "+std::to_string(id)+": detailed physical shape has no cut-off edges");
  check(unique>=(id==Flame?3:5),"sprite "+std::to_string(id)+": distinct material and lighting shades survive GPU rendering");
  check(mismatch==0,"sprite "+std::to_string(id)+": every physical mask pixel agrees with independent GPU readback");
  UnloadImageColors(pixels);UnloadImage(im);
 }
 // Render only physical parts, without glows and steam, at their actual animated transforms.
 // Compare solid interiors and empty margins to avoid treating bilinear edge coverage as a surface.
 auto physicalImage=[](int type,float time){RenderTexture2D rt=LoadRenderTexture(512,640);BeginTextureMode(rt);ClearBackground(BLANK);BeginMode2D({{0,0},{0,0},0,2});auto p=parts(type,time,0);for(int i=0;i<p.count;i++)drawSprite(p.values[i].id,{128+p.values[i].shift.x,270+p.values[i].shift.y},p.values[i].scale,p.values[i].tint);EndMode2D();EndTextureMode();Image im=LoadImageFromTexture(rt.texture);ImageFlipVertical(&im);UnloadRenderTexture(rt);return im;};
 for(int type=0;type<ObjectCount;type++){
  for(float time:{.4f,3.7f}){
   Image im=physicalImage(type,time);Color* pixels=LoadImageColors(im);int probes=0,mismatch=0;
   for(int y=4;y<im.height-4;y+=4)for(int x=4;x<im.width-4;x+=4){bool solid=true,empty=true;for(int dy:{-2,0,2})for(int dx:{-2,0,2}){auto a=pixels[(y+dy)*im.width+x+dx].a;solid&=a>=240;empty&=a<6;}if(!solid&&!empty)continue;
    Vector2 point{(x+.5f)*.5f-128,(y+.5f)*.5f-270};bool picked=contains(type,{0,0},point,time,0);probes++;mismatch+=solid?!picked:picked;
   }
   check(probes>1000&&mismatch==0,"object "+std::to_string(type)+": animated physical silhouette matches GPU pixels at time "+std::to_string(time));UnloadImageColors(pixels);UnloadImage(im);
  }
  if(type==0||type==1||type==2||type==3||type==8||type==11||type==12){Image a=physicalImage(type,.4f),b=physicalImage(type,3.7f);Color* x=LoadImageColors(a);Color* y=LoadImageColors(b);int changed=0;for(int i=0;i<a.width*a.height;i++)changed+=std::abs(int(x[i].a)-y[i].a)+std::abs(int(x[i].r)-y[i].r)+std::abs(int(x[i].g)-y[i].g)+std::abs(int(x[i].b)-y[i].b)>80;check(changed>25,"object "+std::to_string(type)+": physical animation changes visible pixels");UnloadImageColors(x);UnloadImageColors(y);UnloadImage(a);UnloadImage(b);}
 }
 for(int gallery=0;gallery<2;gallery++){
  RenderTexture2D rt=LoadRenderTexture(1280,680);BeginTextureMode(rt);ClearBackground(Ink);txt(gallery==0?"WIEŻA ELDRINA — WYPOSAŻENIE":"WIEŻA ELDRINA — MAGIA I PRZYRZĄDY",24,14,23,Gold);
  for(int cell=0;cell<8;cell++){int id=gallery*8+cell;if(id>=15)continue;float x=cell%4*320.f,y=52+cell/4*310.f;rect(x+8,y+4,304,300,rgb(24,31,39));line(x+12,y+280,x+300,y+280,Line);Vector2 base{x+130,y+236};
   if(id==Window)drawSprite(Window,base,.9f);else draw(id==AlchemyTable?1:id,base,2.7f,id==AlchemyTable?2:0,id,false);
   hero(x+234,y+236,1);const char* name=id==AlchemyTable?"Stół alchemiczny":id==Window?"Kamienne okno":towerObjectName(id);center(name,x+160,y+283,17,Text);center("SKALA BOHATERA",x+240,y+254,9,Muted);
  }EndTextureMode();saveTexture(rt,"qa-v32/furniture-detail-"+std::to_string(gallery+1)+".png");UnloadRenderTexture(rt);
 }
 reset();
 for(int floor=0;floor<TowerFloorCount;floor++){
  game.towerFloor=floor;game.player={12,15};game.time=2.7f;
  for(int index=0;index<int(towerFurnishings(floor).size());index++){
   auto object=towerFurnishings(floor)[index];Vector2 base=screen(object.pos);bool found=false;
   for(int y=-230;y<=40&&!found;y+=3)for(int x=-110;x<=110;x+=3){Vector2 point{base.x+x,base.y+y};auto hit=pickWorld(point);if(hit.kind==HoverKind::TowerObject&&hit.index==index){mouse=point;found=true;break;}}
   check(found&&inspectWorld(pickWorld(mouse)).name==towerObjectName(object.type),"floor "+std::to_string(floor+1)+" / "+towerObjectName(object.type)+": actual shape selects the correct inspection HUD");
   if(found&&((floor==1&&object.type==6)||(floor==2&&object.type==7)||(floor==4&&object.type==11)))shot("hover-floor-"+std::to_string(floor+1)+"-type-"+std::to_string(object.type));
  }
  for(float size:{.8f,1.f,1.2f}){prepareCanvas(size);mouse={0,0};shot("floor-"+std::to_string(floor+1)+"-"+std::to_string(int(size*100)));}
  refreshDisplayLayout();mouse={0,0};renderScene();BeginDrawing();presentCanvas();Image display=presentedImage();check(display.data&&display.width==GetRenderWidth()&&display.height==GetRenderHeight(),"floor "+std::to_string(floor+1)+": decorated room reaches actual fullscreen framebuffer");if(display.data){ExportImage(display,("qa-v32/floor-"+std::to_string(floor+1)+"-display.png").c_str());UnloadImage(display);}EndDrawing();finishFramePacing();
 }
 reset();refreshDisplayLayout();report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}