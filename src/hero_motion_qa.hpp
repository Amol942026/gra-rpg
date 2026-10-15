#pragma once
#include <chrono>
template<class Check>void verifyHeroAnimation(Check check){
 const fs::path output=fs::path("qa-v47")/("capture-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 fs::create_directories(output/"frames");
 check(!audioSystem.available,"skeletal animation diagnostics open no audio device");
 auto base=hero_motion::evaluate(0,0,0,0),translated=base;
 for(auto& b:translated.world)b.p=b.p+hero_motion::Vec{20,-13,7};
 translated.bind();
 actor_art::Rig a(0,0,1,Direction::SE),b(0,0,1,Direction::SE);
 a.setSkeleton(&base);b.setSkeleton(&translated);a.setBone(hero_motion::data::Spine);b.setBone(hero_motion::data::Spine);
 check(std::abs(a.surfaceLight({.4f,.6f,.7f})-b.surfaceLight({.4f,.6f,.7f}))<.00001f,"skeletal normals rotate without bone translation changing the material lighting");

 auto draw=[&](int mode,float progress,int direction,float x,float y,float size,float time){
  AnimationState state;state.direction=Direction(direction);state.idleTime=time;
  state.moving=mode<2;state.phase=progress;state.runBlend=mode==1?1:0;state.sprinting=mode==1;
  float swing=mode==2?progress:-1,roll=mode==3?progress:-1;
  actor_art::draw(0,x,y,size,state,swing>=0?std::sin(swing*PI):0,false,true,Kind::Sword,false,BLANK,nullptr,roll,swing,-1,20);
 };
 RenderTexture2D sample=LoadRenderTexture(320,280);
 RenderTexture2D sheet=LoadRenderTexture(1600,1040);
 BeginTextureMode(sheet);ClearBackground(Ink);txt("SZKIELET / CHÓD / BIEG / MIECZ / UNIK",24,17,24,Gold);EndTextureMode();
 for(int mode=0;mode<4;mode++)for(int direction=0;direction<8;direction++)for(int phase=0;phase<3;phase++){
  float progress=.18f+phase*.27f;
  BeginTextureMode(sample);ClearBackground(BLANK);draw(mode,progress,direction,160,228,1.3f,0);EndTextureMode();
  Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);
  int solid=0,clipped=0,mismatch=0,interior=0;
  for(int y=0;y<280;y++)for(int x=0;x<320;x++){int a=pixels[y*320+x].a;solid+=a>230;clipped+=a>230&&(x<2||y<2||x>=318||y>=278);}
  check(solid>200&&clipped==0,"Blender-driven body and equipment have complete silhouettes in every action and facing");
  AnimationState state;state.direction=Direction(direction);state.moving=mode<2;state.phase=progress;state.runBlend=mode==1?1:0;state.sprinting=mode==1;
  for(int y=70;y<246;y+=13)for(int x=84;x<236;x+=13){
   bool opaque=true,empty=true;for(int dy:{-2,0,2})for(int dx:{-2,0,2}){int a=pixels[(y+dy)*320+x+dx].a;opaque&=a>230;empty&=a<8;}
   if(!opaque&&!empty)continue;Vector2 point{x+.5f,y+.5f};
   float swing=mode==2?progress:-1,roll=mode==3?progress:-1;
   bool hit=actor_art::draw(0,160,228,1.3f,state,swing>=0?std::sin(swing*PI):0,false,true,Kind::Sword,false,BLANK,&point,roll,swing,-1,20);
   mismatch+=opaque?!hit:hit;interior+=opaque;
  }
  check(interior>1&&mismatch==0,"skeletal GPU silhouette and mouse geometry use the same animated joints");
  if(phase==1){
   Texture2D texture=LoadTextureFromImage(im);BeginTextureMode(sheet);
   DrawTexturePro(texture,{0,0,320,280},{direction*200.f-10,mode*240.f+45,224,196},{0,0},0,WHITE);
   EndTextureMode();UnloadTexture(texture);
  }
  UnloadImageColors(pixels);UnloadImage(im);
 }
 saveTexture(sheet,(output/"eight-directions.png").string());UnloadRenderTexture(sheet);UnloadRenderTexture(sample);
 RenderTexture2D movie=LoadRenderTexture(900,640);
 for(int frame=0;frame<120;frame++){
  float t=frame/60.f;
  BeginTextureMode(movie);ClearBackground(Ink);
  const char* labels[]={"CHÓD","BIEG / SHIFT","ZAMACH MIECZEM","PRZETURLANIE"};
  for(int mode=0;mode<4;mode++){
   float x=225+(mode%2)*450.f,y=280+(mode/2)*310.f;
   center(labels[mode],x,y-260,23,Gold);
   ellipse(x,y+2,36,14,rgb(8,15,18));
   if(mode<2)draw(mode,std::fmod(t*(mode==0?WalkSpeed*.38f:WalkSpeed*RunMultiplier*.31f),1.f),1,x,y,2.4f,t);
   else{
    float duration=mode==2?SwingDuration:RollDuration,cycle=mode==2?.8f:1.f,phase=std::fmod(t,cycle);
    if(phase<duration)draw(mode,phase/duration,1,x,y,2.4f,0);
    else draw(4,0,1,x,y,2.4f,0);
   }
  }
  EndTextureMode();Image im=LoadImageFromTexture(movie.texture);ImageFlipVertical(&im);
  check(im.data&&ExportImage(im,(output/"frames"/TextFormat("frame-%03d.png",frame)).string().c_str()),"native 60-Hz articulated animation preview frame exports");
  UnloadImage(im);
 }
 UnloadRenderTexture(movie);
 std::ofstream meta(output/"capture.json");meta<<"{\"fps\":60,\"frames\":120,\"width\":900,\"height\":640,\"source\":\"native game renderer\",\"audio\":false}\n";
}