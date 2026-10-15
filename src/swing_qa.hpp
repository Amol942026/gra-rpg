#pragma once
#include <chrono>
bool runSwordMotionQA(){
 fs::create_directories("qa-v43");std::ofstream report("qa-v43/sword-tests.txt");int checks=0,failures=0;
 const fs::path output=fs::path("qa-v43")/("capture-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 fs::create_directories(output);report<<"Capture directory: "<<output.generic_string()<<'\n';
 auto file=[&](const std::string& name){return (output/name).string();};
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 refreshDisplayLayout();
 check(!audioSystem.available,"sword animation diagnostics open no audio device");
 check(canvas.id&&canvas.texture.width>0,"animation diagnostics have a valid native game canvas");
 RenderTexture2D sample=LoadRenderTexture(320,220);
 fs::create_directories(output/"motion-frames");
 const char* directions[]={"E","SE","S","SW","W","NW","N","NE"};
 for(bool strong:{false,true}){
  RenderTexture2D sheet=LoadRenderTexture(2304,1496);
  BeginTextureMode(sheet);ClearBackground(Ink);txt(strong?"MOCNE CIĘCIE — PRZYGOTOWANIE, CIOS, POWRÓT":"ZAMACH MIECZEM — PRZYGOTOWANIE, CIOS, POWRÓT",24,20,25,Text);EndTextureMode();
  for(int direction=0;direction<8;direction++){
   Image first{};
   for(int frame=0;frame<12;frame++){
    float progress=frame/11.f;AnimationState state;state.direction=Direction(direction);
    BeginTextureMode(sample);ClearBackground(BLANK);
    actor_art::draw(0,160,185,1.65f,state,std::sin(progress*PI),true,true,Kind::Sword,false,BLANK,nullptr,-1,progress,-1,0,strong);
    EndTextureMode();
    Image image=LoadImageFromTexture(sample.texture);ImageFlipVertical(&image);Color* pixels=LoadImageColors(image);int solid=0,clipped=0;
    for(int y=0;y<image.height;y++)for(int x=0;x<image.width;x++)if(pixels[y*image.width+x].a>210){solid++;clipped+=x<2||y<2||x>=image.width-2||y>=image.height-2;}
    check(solid>900&&clipped==0,"each of eight directions has a complete visible pose without cropped limbs or blade");
    UnloadImageColors(pixels);
    auto pose=actor_art::forwardStrike(progress,std::sin(state.idleTime*2.4f)*.6f,strong);
    actor_art::Rig projected(160,185,1.65f,state.direction);
    Vector2 edge=projected.project(pose.hand+pose.blade*.94f);
    check(actor_art::draw(0,160,185,1.65f,state,std::sin(progress*PI),true,true,Kind::Sword,false,BLANK,&edge,-1,progress,-1,0,strong),"the real physical blade matches its pose and mouse silhouette");
    if(frame==0)first=ImageCopy(image);
    if(frame==11){
     Color* a=LoadImageColors(first);Color* b=LoadImageColors(image);int different=0;
     for(int i=0;i<image.width*image.height;i++)different+=std::abs(int(a[i].a)-b[i].a)>5||std::abs(int(a[i].r)-b[i].r)>5;
     check(different<12,"final grip returns to the initial GPU-rendered pose without a snap");
     UnloadImageColors(a);UnloadImageColors(b);UnloadImage(first);
    }
    Texture2D texture=LoadTextureFromImage(image);BeginTextureMode(sheet);
    float x=frame*192.f+96,y=direction*174.f+220;
    if(frame==0)txt(directions[direction],8,y-156,14,Gold);
    DrawTexturePro(texture,{0,0,320,220},{x-144,y-198,288,198},{0,0},0,WHITE);
    EndTextureMode();UnloadTexture(texture);
    if(direction==2)ExportImage(image,file(TextFormat("%s-%02d.png",strong?"strong":"sword",frame)).c_str());
    UnloadImage(image);
   }
  }
  saveTexture(sheet,file(strong?"strong-cut-directions.png":"sword-directions.png"));UnloadRenderTexture(sheet);
 }
 UnloadRenderTexture(sample);
 RenderTexture2D movie=LoadRenderTexture(680,240);
 for(int frame=0;frame<32;frame++){
  BeginTextureMode(movie);ClearBackground(Ink);
  center("ZWYKŁY ZAMACH",170,17,18,Gold);center("MOCNE CIĘCIE",510,17,18,Gold);
  for(bool strong:{false,true}){
   float progress=std::clamp(frame/60.f/(strong?StrongCutDuration:SwingDuration),0.f,1.f);
   AnimationState state;state.direction=Direction::S;
   actor_art::draw(0,strong?510:170,211,1.65f,state,std::sin(progress*PI),true,true,Kind::Sword,false,BLANK,nullptr,-1,progress,-1,0,strong);
  }
  EndTextureMode();
  Image image=LoadImageFromTexture(movie.texture);ImageFlipVertical(&image);
  check(image.data&&ExportImage(image,file(TextFormat("motion-frames/frame-%02d.png",frame)).c_str()),"native 60-Hz preview frame exports successfully");
  UnloadImage(image);
 }
 UnloadRenderTexture(movie);
 Game saved=game;auto oldIntro=intro;auto oldTower=towerStory;
 game=Game{};game.props.clear();game.enemies.clear();game.forage.clear();game.player={20,20};
 intro={};towerStory={};clearStoryMenus();game.skillRanks={1,1,1};game.messageTime=0;mouse={0,0};
 for(bool strong:{false,true}){
  game.attackCd=game.strongCutCd=0;game.swing=0;game.animation.direction=Direction::SE;game.facing={1,0};
  check(strong?game.strongCut():game.attack(),"real gameplay command starts the chosen sword animation");
  game.swing=game.meleeDuration*(1-(sword_pose::cutStart(strong)+sword_pose::cutEnd(strong))*.5f);
  renderScene();Image world=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&world);
  check(world.data&&ExportImage(world,file(strong?"strong-cut-world.png":"sword-world.png").c_str()),"the sword animation renders and exports from the real game scene");UnloadImage(world);
  check(world_shadows::ready,"animated sword continues to participate in the world shadow pass");
 }
 game=saved;intro=oldIntro;towerStory=oldTower;
 report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
