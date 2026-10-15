#pragma once
#include <chrono>
bool runWorldMotionQA(bool baseline=false){
 fs::create_directories("qa-v45");std::ofstream report(baseline?"qa-v45/baseline-pixels.txt":"qa-v45/world-pixels.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const std::string& name){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<name<<'\n';};
 const fs::path output=fs::path("qa-v45")/(std::string(baseline?"before-":"after-")+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));fs::create_directories(output);report<<"Capture directory: "<<output.generic_string()<<'\n';
 auto read=[](){Image im=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&im);return im;};
 auto worldOnly=[](){renderScene();};
 check(!audioSystem.available,"world-motion QA has no audio device");
 for(float requested:{.8f,1.f,1.2f}){
  game=Game{};intro={};towerStory={};clearStoryMenus();developerActive=developerMenuOpen=welcome=help=paused=false;game.enterMageTower();game.towerFloor=1;game.player={12,14};game.time=0;game.effects.clear();hovered={};dragging={};mouse={0,0};prepareCanvas(requested);
  const int left=int(std::round(250*renderScale)),top=int(std::round(240*renderScale)),width=int(std::round(180*renderScale)),height=int(std::round(135*renderScale));
  std::vector<Color> initial;long long probes=0,changed=0,error=0;double worst=0;
  for(int frame=0;frame<16;frame++){
   float delta=frame/(80*renderScale);game.player={12+delta,14-delta};worldOnly();Image image=read();Color* pixels=LoadImageColors(image);int frameChanged=0;
   for(int y=0;y<height;y++)for(int x=0;x<width;x++){
    Color c=pixels[(top+y)*image.width+left-frame+x];
    if(frame==0)initial.push_back(c);else{
     Color a=initial[y*width+x];int e=std::max({std::abs(int(c.r)-a.r),std::abs(int(c.g)-a.g),std::abs(int(c.b)-a.b)});probes++;frameChanged+=e>2;error+=e;
    }
   }
   if(frame){changed+=frameChanged;worst=std::max(worst,double(frameChanged)/(width*height));}
   if(frame==0){int translucent=0;for(int y=0;y<height;y++)for(int x=0;x<width;x++)translucent+=pixels[(top+y)*image.width+left+x].a!=255;check(translucent==0,"completed world remains opaque without a second alpha multiplication");}
   if(frame==0||frame==1||frame==15)ExportImage(image,(output/("tower-"+std::to_string(int(requested*100))+"-"+std::to_string(frame)+".png")).string().c_str());
   UnloadImageColors(pixels);UnloadImage(image);
  }
  check(!worldCanvas.id,"native world is copied one-to-one without image resizing");
  report<<"One-to-one native copy; buffer="<<canvas.texture.width<<'x'<<canvas.texture.height<<'\n';
  report<<"Scale="<<renderScale<<"; pixels="<<probes<<"; changes>2="<<changed<<"; mean maximum-channel error="<<double(error)/probes<<"; worst changed fraction="<<worst<<'\n';
  check(worst<.002,"static architectural edges retain their raster during one-pixel camera translations");
 }
 game=Game{};refreshDisplayLayout();report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
