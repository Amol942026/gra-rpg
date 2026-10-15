#pragma once
#include <chrono>
bool runMapLabelsQA(){
 fs::create_directories("qa-v44");std::ofstream report("qa-v44/labels-tests.txt");int checks=0,failures=0;
 const fs::path output=fs::path("qa-v44")/("capture-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 fs::create_directories(output);report<<"Capture directory: "<<output.generic_string()<<'\n';
 auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 auto reset=[](){game=Game{};intro={};towerStory={};clearStoryMenus();developerActive=developerMenuOpen=false;welcome=help=paused=audioOpen=false;dragging={};mouse={0,0};hovered={};game.enterMageTower();game.player={12,14};game.dawnQuest=DawnQuest::AtTower;game.messageTime=0;game.effects.clear();};
 auto read=[](RenderTexture2D target){Image im=LoadImageFromTexture(target.texture);ImageFlipVertical(&im);return im;};
 auto labels=[](){std::vector<MapNameplate> result;forEachMapNameplate([&](const MapNameplate& label){result.push_back(label);});return result;};
 auto crop=[&](Image image,const MapNameplate& label){Rectangle b=mapNameplateBounds(label);return ImageFromImage(image,{std::round(b.x*renderScale),std::round(b.y*renderScale),std::round(b.width*renderScale),std::round(b.height*renderScale)});};
 check(!audioSystem.available,"map-label diagnostics never open the audio device");
 reset();
 for(int floor=0;floor<TowerFloorCount;floor++){
  game.towerFloor=floor;auto names=labels();
  int stairs=0,merchants=0,mages=0;for(const auto& label:names){stairs+=label.name.find("SCHODY")==0;merchants+=label.name=="MIRON";mages+=label.name=="ELDRIN";}
  check(stairs==(floor>0)+(floor<4)&&merchants==(floor==MerchantFloor)&&mages==(floor==0),"only the existing stairs and NPCs get labels on floor "+std::to_string(floor+1));
 }
 int motionSamples=0;
 for(float requestedScale:{.8f,1.f,1.2f,1.5f}){
  prepareCanvas(requestedScale);size_t atlases=typography.atlasCount();
  RenderTexture2D overlay=LoadRenderTexture(canvas.texture.width,canvas.texture.height);
  for(int floor:{0,MerchantFloor}){
   reset();game.towerFloor=floor;std::vector<Image> reference;
   for(int frame=0;frame<16;frame++){
    // Exercise fractional camera phases in both axes, including offsets much
    // smaller than one final pixel. Whole-pixel translation is allowed;
    // glyph coverage and padding must remain byte-for-byte identical.
    game.player=V{12,14}+V{frame*.0037f,frame*.0059f};hovered={};
    BeginTextureMode(overlay);ClearBackground(BLANK);BeginMode2D({{0,0},{0,0},0,renderScale});drawWorldLabels();EndMode2D();EndTextureMode();
    Image image=read(overlay);auto names=labels();
    for(size_t i=0;i<names.size();i++){
     Image sample=crop(image,names[i]);Color* current=LoadImageColors(sample);
     int visible=0;for(int p=0;p<sample.width*sample.height;p++)visible+=current[p].a>180&&current[p].r>100;
     check(visible>20,"native glyph interiors remain visible: "+names[i].name);
     if(frame==0)reference.push_back(ImageCopy(sample));
     else{
      Color* initial=LoadImageColors(reference[i]);int different=0;
      bool sameSize=sample.width==reference[i].width&&sample.height==reference[i].height;
      if(sameSize)for(int p=0;p<sample.width*sample.height;p++)different+=current[p].r!=initial[p].r||current[p].g!=initial[p].g||current[p].b!=initial[p].b||current[p].a!=initial[p].a;
      check(sameSize&&different==0,"subpixel camera motion preserves exact glyph pixels: "+names[i].name);
      if(different)report<<"  scale="<<renderScale<<" frame="<<frame<<" changed="<<different<<'\n';
      UnloadImageColors(initial);
     }
     UnloadImageColors(current);UnloadImage(sample);
    }
    if(frame==0){
     renderScene();Image scene=read(canvas);Color* expected=LoadImageColors(image);Color* actual=LoadImageColors(scene);int probes=0,mismatch=0,outside=0;
     for(int y=0;y<image.height;y++)for(int x=0;x<image.width;x++){
      const Color a=expected[y*image.width+x],b=actual[y*image.width+x];
      if(x>=int(std::round(WorldW*renderScale)))outside+=a.a!=0;
      if(a.a>0){probes++;mismatch+=a.r!=b.r||a.g!=b.g||a.b!=b.b||a.a!=b.a;}
     }
     check(probes>60&&mismatch==0,"actual scene draws labels after the world resolve at native scale");
     check(outside==0,"map labels cannot overlap the right-hand HUD");
     std::string path=(output/("floor-"+std::to_string(floor+1)+"-"+std::to_string(int(std::round(renderScale*100)))+".png")).string();
     check(ExportImage(scene,path.c_str()),"export native-resolution label reference");
     UnloadImageColors(expected);UnloadImageColors(actual);UnloadImage(scene);
    }
    UnloadImage(image);motionSamples++;
   }
   for(auto& image:reference)UnloadImage(image);
  }
  check(typography.atlasCount()==atlases,"movement loads no new font atlases");
  UnloadRenderTexture(overlay);
 }
 reset();game.towerFloor=MerchantFloor;game.player=MerchantPosition+V{-1,0};refreshDisplayLayout();
 auto names=labels();bool merchant=false;
 for(const auto& label:names)if(label.name=="MIRON")merchant=label.bold&&!label.title&&label.size==20&&label.hint=="E / PPM · HANDEL";
 check(merchant,"reachable Miron uses real Lato Bold at size 20 and retains the trade prompt");
 renderScene();saveTexture(canvas,(output/"merchant-near-native.png").string());
 // Present an actual frame as well as FBO readbacks, respecting the existing
 // high-DPI viewport and V-sync path. Never change the saved audio preferences.
 BeginDrawing();presentCanvas();
 Image displayed=presentedImage();check(displayed.width==GetRenderWidth()&&displayed.height==GetRenderHeight(),"final presentation uses the current framebuffer resolution");
 Image source=read(canvas);Color* expected=LoadImageColors(source);Color* actual=LoadImageColors(displayed);int different=0,nonblack=0;
 for(int y=0;y<source.height;y++)for(int x=0;x<source.width;x++){
  Color a=expected[y*source.width+x],b=actual[(y+int(presentationViewport.y))*displayed.width+x+int(presentationViewport.x)];
  different+=std::abs((int(a.r)*a.a+127)/255-b.r)>1||std::abs((int(a.g)*a.a+127)/255-b.g)>1||std::abs((int(a.b)*a.a+127)/255-b.b)>1;
  nonblack+=int(b.r)+b.g+b.b>10;
 }
 check(different==0&&nonblack>source.width*source.height*.9,"final presented scene matches native label pixels without extra scaling or cropping");
 check(ExportImage(displayed,(output/"merchant-display.png").string().c_str()),"export final displayed frame");
 UnloadImageColors(expected);UnloadImageColors(actual);UnloadImage(source);UnloadImage(displayed);EndDrawing();finishFramePacing();
 reset();report<<"Camera samples: "<<motionSamples<<"\nChecks: "<<checks<<"; failures: "<<failures<<'\n';
 std::printf("Map labels: %d/%d checks passed\n",checks-failures,checks);return failures==0;
}
