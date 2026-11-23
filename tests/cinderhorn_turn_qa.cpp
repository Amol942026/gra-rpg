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
 volcano_actors::cinderhorn_turn::load();
 regional_enemy_art::load();
 if(!volcano_actors::cinderhorn_turn::ready())return 5;

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
 report<<"{\n  \"shared_rig\":true";
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
  int distinct=0,lowHeight=height,highHeight=0,west=0,east=0;
  double biggest=0,worstPaint=0,softPixels=0;int angleOfWorst=0;
  for(int d=1;d<=360;d++){
   Image image=scene(model(d%360),old);
   const auto* a=static_cast<const Color*>(previous.data);
   const auto* b=static_cast<const Color*>(image.data);
   if(std::memcmp(a,b,width*height*sizeof(Color)))++distinct;
   int unionArea=0,edgeChange=0,visible=0,soft=0,top=height,bottom=0,leftmost=width,rightmost=0;
   double colorChange=0;
   for(int i=0;i<width*height;i++){
    const bool left=a[i].a>=128,right=b[i].a>=128;
    unionArea+=(left||right);edgeChange+=left!=right;
    visible+=b[i].a>=26;
    soft+=b[i].a>=26&&b[i].a<230;
    if(right){
     const int x=i%width,y=i/width;
     top=std::min(top,y);bottom=std::max(bottom,y);
     leftmost=std::min(leftmost,x);rightmost=std::max(rightmost,x);
    }
    if(left||right)colorChange+=(std::abs(int(a[i].r)-b[i].r)+std::abs(int(a[i].g)-b[i].g)+std::abs(int(a[i].b)-b[i].b))/3.;
   }
   failures+=unionArea<1200;
   softPixels+=double(soft)/std::max(1,visible);
   lowHeight=std::min(lowHeight,bottom-top+1);highHeight=std::max(highHeight,bottom-top+1);
   if(d==180)west=rightmost-leftmost+1;
   if(d==360)east=rightmost-leftmost+1;
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
  if(mode==2)failures+=(highHeight-lowHeight>4||std::abs(east-west)>3);
  report<<",\n  \""<<(old?"old_walk":mode==1?"walk":mode==2?"idle":"charge_windup")<<"\": {\"distinct_steps\":"<<distinct
        <<",\"max_edge_change\":"<<biggest<<",\"worst_at_degree\":"<<angleOfWorst
        <<",\"max_rgb_change\":"<<worstPaint<<",\"mean_soft_pixel_fraction\":"<<softPixels/360
        <<",\"min_height\":"<<lowHeight<<",\"max_height\":"<<highHeight
        <<",\"width_east\":"<<east<<",\"width_west\":"<<west<<"}";
 }
 int jointFailures=0,jointChecks=0;
 {
  using namespace volcano_actors::cinderhorn_turn;
  volcano_actors::cinderhorn_turn::Mesh body;
  for(int mode=0;mode<3;mode++)for(int degree=0;degree<360;degree+=15)for(int sample=0;sample<20;sample++){
   Enemy e=enemy(degree,mode!=1);
   e.animation.phase=sample/20.f;
   if(mode==1)e.boarWindup=CinderhornCharge.windup*(1-e.animation.phase);
   if(mode==2)e.boarRush=.1f;
   const auto r=pose(e);
   body.triangles.clear();drawBody(body,r);
   const auto check=[&](Vector2 socket){++jointChecks;jointFailures+=!body.includes(socket);};
   for(int side:{-1,1})for(const bool fore:{false,true})check(legSocket(r,fore,side).screen);
   check(r.face(PollForward,0,PollHeight).screen);
   check(r.face(43.3f,0,46.3f).screen);
  }
 }
 failures+=jointFailures;
 report<<",\n  \"joint_checks\":"<<jointChecks<<",\n  \"joint_failures\":"<<jointFailures;
 int hoofChecks=0;
 float greatestHoofDrift=0;
 for(int direction=0;direction<360;direction+=45){
  namespace horn=volcano_actors::cinderhorn_turn;
  Enemy e=enemy(direction,true);
  const auto axes=actorFacing(e.animation.facingAngle);
  const Vector2 travel{1.3f*axes.x,1.3f*axes.y*ActorGroundSquash};
  const float dx=.5f*(travel.x/40+travel.y/20),dy=.5f*(travel.y/20-travel.x/40);
  for(int n=0;n<95;n++){
   const auto before=horn::pose(e);
   cinderhorn_gait::advance(e.animation,dx,dy,1.f/60,false);
   const auto after=horn::pose(e);
   for(int side:{-1,1})for(bool front:{false,true}){
    const float support=front?cinderhorn_gait::ForeSupport:cinderhorn_gait::HindSupport;
    const float start=horn::legPhase(before.phase,front,side,false),end=horn::legPhase(after.phase,front,side,false);
    if(end<start||end>support-.001f)continue;
    const auto a=horn::hoofMotion(before,front,side),b=horn::hoofMotion(after,front,side);
    const auto p=before.project(a.along,0,a.lift).screen,q=after.project(b.along,0,b.lift).screen;
    const float error=std::hypot(travel.x+(q.x-p.x)*cinderhorn_gait::Scale,travel.y+(q.y-p.y)*cinderhorn_gait::Scale);
    greatestHoofDrift=std::max(greatestHoofDrift,error);
    ++hoofChecks;
    failures+=a.lift>.001f||b.lift>.001f||std::abs(a.toe)>.001f||std::abs(b.toe)>.001f||error>.004f;
   }
  }
 }
 failures+=hoofChecks<1900;
 report<<",\n  \"planted_hoof_checks\":"<<hoofChecks<<",\n  \"max_planted_hoof_drift_pixels\":"<<greatestHoofDrift;
 int depthSamples=0,worstDepthMismatch=0;
 {
  namespace horn=volcano_actors::cinderhorn_turn;
  horn::Mesh reversed;
  for(int mode=0;mode<3;mode++)for(int d=0;d<360;d+=5){
   Enemy e=enemy(d,mode!=1);e.animation.phase=std::fmod(d/138.f,1.f);
   if(mode==1)e.boarWindup=.31f*CinderhornCharge.windup;
   if(mode==2)e.boarRush=.1f;
   const auto& model=horn::frame(e);
   reversed.triangles.assign(model.mesh.triangles.rbegin(),model.mesh.triangles.rend());
   Image normal=scene(e,false);
   BeginTextureMode(target);ClearBackground(BLANK);
   rlPushMatrix();rlTranslatef(ground.x,ground.y,0);
   rlScalef(cinderhorn_gait::Scale,cinderhorn_gait::Scale*model.riseScale,1);
   horn::paintSurface(reversed,1);
   rlPopMatrix();EndTextureMode();
   Image opposite=LoadImageFromTexture(target.texture);ImageFlipVertical(&opposite);
   const auto* a=static_cast<const Color*>(normal.data);
   const auto* b=static_cast<const Color*>(opposite.data);
   int pixels=0;
   for(int i=0;i<width*height;i++){
    const auto largest=std::max({std::abs(int(a[i].r)-b[i].r),std::abs(int(a[i].g)-b[i].g),
                                 std::abs(int(a[i].b)-b[i].b),std::abs(int(a[i].a)-b[i].a)});
    pixels+=largest>8;
   }
   ++depthSamples;worstDepthMismatch=std::max(worstDepthMismatch,pixels);
   if(pixels>6){++failures;std::cerr<<"Depth overlap "<<mode<<'/'<<d<<": "<<pixels<<'\n';}
   UnloadImage(normal);UnloadImage(opposite);
  }
 }
 report<<",\n  \"depth_samples\":"<<depthSamples<<",\n  \"max_depth_order_mismatch\":"<<worstDepthMismatch;
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
 UnloadRenderTexture(target);regional_enemy_art::unload();volcano_actors::cinderhorn_turn::unload();CloseWindow();
 return failures?1:0;
}
