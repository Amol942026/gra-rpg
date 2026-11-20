#include "raylib.h"
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include <chrono>
#include <cstring>

int main(int argc,char** argv){
 if(argc!=4||std::string_view(argv[1])!="--no-audio"||std::string_view(argv[2])!="--background-test")return 3;
 const fs::path output=fs::absolute(argv[3]);fs::create_directories(output);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_ALWAYS_RUN);
 sync_present::useOpenGL();InitWindow(320,260,"Rogacz QA");
 if(!IsWindowReady()||!IsWindowHidden()||IsAudioDeviceReady())return 4;
 volcano_actors::turn_art::load();
 regional_enemy_art::load();
 if(!volcano_actors::cinderhorn_turn::ready()||!volcano_actors::turn_art::filtered())return 5;

 constexpr int width=230,height=225;
 const Vector2 ground{width*.5f,185};
 RenderTexture2D target=LoadRenderTexture(width,height);
 auto scene=[&](const Enemy& e,bool baseline){
  BeginTextureMode(target);ClearBackground(BLANK);
  if(baseline)regional_enemy_art::drawFrame(regional_enemy_art::spriteFrame(e),e.type,e.animation.direction,
                                            ground.x,ground.y,BLANK,nullptr,false,1);
  else regional_enemy_art::draw(e,ground.x,ground.y);
  EndTextureMode();
  Image result=LoadImageFromTexture(target.texture);ImageFlipVertical(&result);
  return result;
 };
 auto radians=[](int degree){return degree*DEG2RAD;};
 auto enemy=[&](int degree,bool moving){
  Enemy e{};e.type=CinderhornType;
  const float angle=radians(degree);
  e.animation.direction=directionAtScreenAngle(angle);
  e.animation.facingAngle=e.animation.targetAngle=angle;
  e.animation.motionInitialized=e.animation.facingInitialized=true;
  e.animation.moving=moving;e.animation.phase=.16f;e.animation.moveBlend=moving?1.f:0.f;
  return e;
 };
 std::ofstream report(output/"render.json");
 report<<"{";
 int failures=0;
 const auto start=std::chrono::steady_clock::now();
 for(int mode=0;mode<4;mode++){
  const bool old=mode==0, moving=mode<2;
  const auto model=[&](int d){
   Enemy e=enemy(d,moving);
   if(mode==3)e.boarWindup=CinderhornCharge.windup*.24f;
   return e;
  };
  Image previous=scene(model(0),old);
  int distinct=0;double biggest=0,worstPaint=0;int angleOfWorst=0;
  for(int d=1;d<=360;d++){
   Image image=scene(model(d%360),old);
   const auto* a=static_cast<const Color*>(previous.data);
   const auto* b=static_cast<const Color*>(image.data);
   if(std::memcmp(a,b,width*height*sizeof(Color)))++distinct;
   int unionArea=0,edgeChange=0;
   double colorChange=0;
   for(int i=0;i<width*height;i++){
    const bool left=a[i].a>=128,right=b[i].a>=128;
    unionArea+=(left||right);edgeChange+=left!=right;
    if(left||right)colorChange+=(std::abs(int(a[i].r)-b[i].r)+std::abs(int(a[i].g)-b[i].g)+std::abs(int(a[i].b)-b[i].b))/3.;
   }
   failures+=unionArea<1200;
   const double edge=double(edgeChange)/std::max(1,unionArea);
   if(edge>biggest){biggest=edge;angleOfWorst=d;}
   worstPaint=std::max(worstPaint,colorChange/std::max(1,unionArea));
   if(mode==1&&d%45==0){
    const Enemy e=model(d%360);
    int valid=0;
    for(int y=25;y<height-20;y+=5)for(int x=15;x<width-15;x+=5){
     const Vector2 p{x+.5f,y+.5f};
     const bool hits=regional_enemy_art::draw(e,ground.x,ground.y,BLANK,&p);
     if(b[y*width+x].a>=180)valid+=hits;
     if(b[y*width+x].a<=30)failures+=hits;
    }
    failures+=valid<120;
   }
   UnloadImage(previous);previous=image;
  }
  UnloadImage(previous);
  if(!old)failures+=(distinct<350||biggest>.17);
  if(old)failures+=distinct!=8;
  if(mode)report<<",";
  report<<"\n  \""<<(old?"old_walk":mode==1?"walk":mode==2?"idle":"charge_windup")<<"\": {\"distinct_steps\":"<<distinct
        <<",\"max_edge_change\":"<<biggest<<",\"worst_at_degree\":"<<angleOfWorst
        <<",\"max_rgb_change\":"<<worstPaint<<"}";
 }
 for(int f=0;f<60;f++){
  Enemy e=enemy(6*f,true);e.animation.phase=std::fmod(f/30.f,1.f);
  Image frame=scene(e,false);
  failures+=!ExportImage(frame,(output/("turn-"+std::to_string(f)+".png")).string().c_str());
  UnloadImage(frame);
 }
 Enemy e=enemy(33,true);
 e.animation.phase=.38f;
 game.beginEnemyDeath(e,0);
 Image living=scene(e,false);
 BeginTextureMode(target);ClearBackground(BLANK);
 regional_enemy_art::cinderhornFallLayer(e,ground.x,ground.y,0,BLANK,nullptr,false,1,false);
 EndTextureMode();
 Image first=LoadImageFromTexture(target.texture);ImageFlipVertical(&first);
 failures+=std::memcmp(living.data,first.data,width*height*sizeof(Color))!=0;
 UnloadImage(living);UnloadImage(first);
 const auto elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
 report<<",\n  \"render_readback_export_seconds\":"<<elapsed<<",\n  \"failures\":"<<failures<<"\n}\n";
 UnloadRenderTexture(target);regional_enemy_art::unload();volcano_actors::turn_art::unload();CloseWindow();
 return failures?1:0;
}
