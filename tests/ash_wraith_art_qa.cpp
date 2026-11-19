#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include "enemy_ability_fixture.hpp"
#include <chrono>

namespace {
constexpr int FrameWidth=180,FrameHeight=166;
constexpr Vector2 anchor{90,132};
using Pixels=std::vector<Color>;
enum class Pass {Living,Falling,Rest,Shadow,Outline,WorldCorpse};
Pixels capture(const Enemy& e,RenderTexture2D target,Pass pass,float time,const fs::path& path={}){
 BeginTextureMode(target);ClearBackground(BLANK);native_world::beginOpaqueFrame();
 if(pass==Pass::WorldCorpse){
  Enemy body=e;body.deathElapsed=time>=1?-1.f:time*enemy_death::duration(e.type);
  corpseArt(body,anchor.x,anchor.y);
 }else if(pass==Pass::Rest)volcano_actors::rest(e,anchor.x,anchor.y);
 else if(pass==Pass::Falling)volcano_actors::death(e,anchor.x,anchor.y,time);
 else if(pass==Pass::Shadow){
  BeginShaderMode(world_shadows::maskShader);world_shadows::beginMaskBlend();rlDisableBackfaceCulling();
  actor_art::beginShadowFrame();actor_art::drawEnemy(e,anchor.x,anchor.y);
  actor_art::shadowBuilding=false;
  EndBlendMode();EndShaderMode();rlEnableBackfaceCulling();
 }else volcano_actors::draw(e.type,anchor.x,anchor.y,1,e.animation,e.volcanoAction(),pass==Pass::Outline?Gold:BLANK);
 native_world::finishOpaqueBackground();EndTextureMode();
 Image image=LoadImageFromTexture(target.texture);ImageFlipVertical(&image);
 ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
 if(!path.empty()&&!ExportImage(image,path.string().c_str()))throw std::runtime_error("QA export failed");
 const auto* colors=static_cast<const Color*>(image.data);Pixels pixels(colors,colors+FrameWidth*FrameHeight);
 UnloadImage(image);return pixels;
}
int difference(const Pixels& a,const Pixels& b,int minimum=0){
 int n=0;for(size_t i=0;i<a.size();i++)if(std::abs(a[i].r-b[i].r)+std::abs(a[i].g-b[i].g)+
   std::abs(a[i].b-b[i].b)+std::abs(a[i].a-b[i].a)>minimum)n++;
 return n;
}
int filled(const Pixels& a){return int(std::count_if(a.begin(),a.end(),[](Color c){return c.a>=128;}));}
struct LateFit {float overlap;int largestEdge;};
LateFit compareLate(const Pixels& a,const Pixels& b){
 int shared=0,total=0;std::array<int,4> bounds[2]={{FrameWidth,0,FrameHeight,0},{FrameWidth,0,FrameHeight,0}};
 for(int y=0;y<FrameHeight;y++)for(int x=0;x<FrameWidth;x++){
  int alpha[]={a[y*FrameWidth+x].a,b[y*FrameWidth+x].a};
  shared+=alpha[0]>=80&&alpha[1]>=80;total+=alpha[0]>=80||alpha[1]>=80;
  for(int i=0;i<2;i++)if(alpha[i]>=128){
   auto& v=bounds[i];v[0]=std::min(v[0],x);v[1]=std::max(v[1],x);v[2]=std::min(v[2],y);v[3]=std::max(v[3],y);
  }
 }
 int edge=0;for(int i=0;i<4;i++)edge=std::max(edge,std::abs(bounds[0][i]-bounds[1][i]));
 return {float(shared)/std::max(1,total),edge};
}
}

int main(int argc,char** argv){
 if(argc!=4||std::string(argv[1])!="--no-audio"||std::string(argv[2])!="--background-test")return 2;
 const fs::path dir=argv[3];fs::create_directories(dir);
 std::ofstream log(dir/"wraith-checks.txt");
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_ALWAYS_RUN);
 sync_present::useOpenGL();InitWindow(FrameWidth,FrameHeight,"Emberfall / Wraith QA");
 if(!IsWindowReady()||!IsWindowHidden()||IsAudioDeviceReady())return 3;
 volcano_actors::load();caldera_attack_art::load();world_shadows::init();auto target=render_targets::colorOnly(FrameWidth,FrameHeight);
 int checks=0,failures=0;
 auto check=[&](bool ok,std::string s){++checks;failures+=!ok;log<<(ok?"PASS ":"FAIL ")<<s<<'\n';};
 check(wraith_death_art::pages[0].art.id&&wraith_death_art::pages[0].authoredRest,"painted directions and material loaded");
 check(wraith_death_art::rotation::ready(),"registered turn material and edge-distance fields loaded");
 const auto start=std::chrono::steady_clock::now();
 {
  // A woven stripe has only one colour at its surface location. With point
  // filtering, a second colour in the interior reveals a double exposure;
  // opaque interiors can be tested separately from antialiasing at the hems.
  auto& weave=wraith_death_art::rotation::weave;
  const auto production=weave;
  Image bands=GenImageColor(256,6*96,BLANK);
  auto* ink=static_cast<Color*>(bands.data);
  for(int y=0;y<bands.height;y++)for(int x=0;x<bands.width;x++)
   ink[y*bands.width+x]=((x/8+y/5)&1)?Color{240,21,0,255}:Color{0,21,240,255};
  weave=LoadTextureFromImage(bands);
  UnloadImage(bands);
  SetTextureWrap(weave,TEXTURE_WRAP_REPEAT);
  SetTextureFilter(weave,TEXTURE_FILTER_POINT);
  game=enemyAbilityArena(AshWraithType);auto& e=game.enemies.front();
  e.animation.facingInitialized=true;e.animation.idleTime=.5f;
  for(float degrees:{24.5f,66.25f,115.5f,165.25f,205.25f,246.25f,288.5f,337.5f}){
   e.animation.facingAngle=degrees*DEG2RAD;
   int red=0,blue=0,mixed=0;
   for(auto c:capture(e,target,Pass::Living,0)){
    if(c.a<253)continue;
    if(c.r>208&&c.b<32)++red;
    else if(c.b>208&&c.r<32)++blue;
    else ++mixed;
   }
   check(red>180&&blue>180&&mixed<.08f*(red+blue),std::to_string(degrees)+
         " one woven pigment across the body, red/blue/mixed "+std::to_string(red)+"/"+
         std::to_string(blue)+"/"+std::to_string(mixed));
  }
  UnloadTexture(weave);weave=production;
 }
 {
  game=enemyAbilityArena(AshWraithType);auto& e=game.enemies.front();
  e.animation.facingInitialized=true;e.animation.phase=.2f;e.animation.idleTime=.5f;
  auto frame=[&](float a){
   e.animation.facingAngle=a;e.animation.direction=directionAtScreenAngle(a);
   return capture(e,target,Pass::Living,0);
  };
  auto previous=frame(0);int repeats=0;float worstOverlap=1;
  for(int deg=1;deg<=360;deg++){
   auto image=frame(deg*PI/180.f);
   repeats+=difference(previous,image)==0;
   worstOverlap=std::min(worstOverlap,compareLate(previous,image).overlap);
   previous=std::move(image);
  }
  check(!repeats&&worstOverlap>.96f,"360 frozen-phase one-degree steps, repeated frames "+
        std::to_string(repeats)+", worst silhouette overlap "+std::to_string(worstOverlap));
  int discontinuity=0;
  for(int key=0;key<32;key++){
   const int sector=key/4;const float left=wraith_death_art::paintedAngles[sector];
   const float right=sector==7?2*PI:wraith_death_art::paintedAngles[sector+1];
   const float center=left+(right-left)*(key%4)*.25f;
   const auto before=frame(center-.0001f),after=frame(center+.0001f);
   discontinuity=std::max(discontinuity,difference(before,after,24));
  }
  check(discontinuity<30,"largest painted seam, including 359/0, pixels changing by >24 at +/-0.006 deg: "+
        std::to_string(discontinuity));
 }
 for(int k=0;k<32;k++){
  game=enemyAbilityArena(AshWraithType);
  auto& e=game.enemies.front();e.animation.facingInitialized=true;
  const int base=k/4;const float t=(k%4)*.25f;
  const float a=wraith_death_art::paintedAngles[base],b=base==7?2*PI:wraith_death_art::paintedAngles[base+1];
  e.animation.facingAngle=a+(b-a)*t;e.animation.direction=Direction((base+(t>=.5f))%8);
  e.animation.idleTime=.4f;e.animation.phase=.3f;e.animation.moveBlend=.8f;
  const auto id=std::to_string(k);
  check(wraith_death_art::at(e.animation).index==k,id+" heading selects one painted view");
  auto live=capture(e,target,Pass::Living,0,k==5?dir/"flight.png":fs::path{});
  const auto shadow=capture(e,target,Pass::Shadow,0,k==5?dir/"shadow.png":fs::path{});
  const auto outlined=capture(e,target,Pass::Outline,0);
  const int area=filled(live);
  check(area>800&&filled(shadow)>120&&filled(outlined)>area,id+" living, ground shadow and outline "+
        std::to_string(area)+"/"+std::to_string(filled(shadow))+"/"+std::to_string(filled(outlined)));
  int tested=0,errors=0;
  for(int y=12;y<FrameHeight-10;y+=5)for(int x=18;x<FrameWidth-18;x+=5){
   const auto alpha=live[y*FrameWidth+x].a;if(alpha>16&&alpha<224)continue;
   const Vector2 point{x+.5f,y+.5f};
   const bool hit=volcano_actors::draw(e.type,anchor.x,anchor.y,1,e.animation,e.volcanoAction(),BLANK,&point);
   tested++;errors+=hit!=(alpha>=224);
  }
  check(errors<=std::max(3,tested/90),id+" rendered and clickable silhouettes agree, mismatches "+std::to_string(errors));
  e.windup=meleeWindup(e.type)*.29f;e.strike=0;
  auto casting=capture(e,target,Pass::Living,0,k==5?dir/"windup.png":fs::path{});
  check(difference(live,casting,24)>area/3,id+" visible tell");
  e.windup=0;e.strike=meleeRecovery(e.type);
  auto releasing=capture(e,target,Pass::Living,0,k==5?dir/"release.png":fs::path{});
  game.launchCinder(e);
  const auto& bolt=game.cinders.back();
  const auto point=volcano_art::boltAnchor(bolt,&e);const auto projected=volcano_art::cinderScreen(e.pos);
  const auto pose=wraith_death_art::samplePose(e.animation,.45f);
  auto palm=wraith_death_art::suspended(pose,5,pose.palm[1]);const float scale=volcano_actors::bodyScale(3);
  check(std::hypot(point.x-projected.x-palm.x*scale,point.y-projected.y-palm.y*scale)<.01f&&bolt.aimRange==0,id+" ember originates in throwing sleeve");
  game.beginEnemyDeath(e,0);e.alive=false;
  const float contact=enemy_death::groundContact(e.type);
  auto early=capture(e,target,Pass::Falling,.00001f);
  auto almost=capture(e,target,Pass::Falling,contact*.9999f);
  auto at=capture(e,target,Pass::Falling,contact);
  auto rest=capture(e,target,Pass::Rest,1,k==5?dir/"rest.png":fs::path{});
  check(difference(releasing,early,16)<area*.05f,id+" continuous first fall pose");
  check(difference(almost,at,16)<area*.03f&&difference(at,rest)==0,id+" last settling frame and permanent corpse share pose");
  check(filled(rest)>area*.13f&&filled(rest)<area*.92f,id+" hollow robe settles into a visible pile");
 }
 for(int k=0;k<40;k++)for(int side:{1,-1}){
  Enemy e{900+AshWraithType,AshWraithType,{25,25},{25,25},100,100};
  const int base=k<32?k/4:k-32;const float a=wraith_death_art::paintedAngles[base];
  const float b=base==7?2*PI:wraith_death_art::paintedAngles[base+1];
  e.animation.facingAngle=k<32?a+(b-a)*(k%4)*.25f:base*PI/4+.1f;
  e.animation.direction=k<32?directionAtScreenAngle(e.animation.facingAngle):Direction(base);
  e.animation.motionInitialized=e.animation.facingInitialized=true;
  e.animation.moveBlend=.84f;e.animation.phase=.28f;e.animation.idleTime=.4f;
  game.hit(e,10000,DamageType::Physical,false,flankImpulse(e.animation,side));
  const float contact=enemy_death::groundContact(e.type);
  const auto rest=capture(e,target,Pass::WorldCorpse,1);
  for(float t:{contact*.92f,contact-.04f}){
   const auto fit=compareLate(capture(e,target,Pass::WorldCorpse,t),rest);
   check(fit.overlap>=.83f&&fit.largestEdge<=4,std::to_string(k)+"/"+std::to_string(side)+" late world corpse, overlap "+
         std::to_string(fit.overlap)+" max edge "+std::to_string(fit.largestEdge));
  }
 }
 const double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
 log<<"checks "<<checks<<" failures "<<failures<<" render/readback/verification seconds "<<seconds<<'\n';
 UnloadRenderTexture(target);caldera_attack_art::unload();volcano_actors::unload();CloseWindow();
 return failures?1:0;
}
