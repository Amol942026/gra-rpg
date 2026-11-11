#include "raylib.h"
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main

int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 const fs::path output=fs::absolute(argv[2]);
 if(!fs::create_directory(output))return 4;
 fs::current_path(fs::absolute(argv[0]).parent_path());
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);
 sync_present::useOpenGL();InitWindow(W,H,"Emberfall wind QA --no-audio");
 if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();
 sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();
 tower_stairs::make();tower_furniture::make();prepareCanvas(1);

 std::ofstream log(output/"checks.txt");
 int checks=0,failures=0;
 auto check=[&](bool pass,const std::string& label){
  checks++;failures+=!pass;log<<(pass?"PASS ":"FAIL ")<<label<<'\n';
 };
 auto read=[](RenderTexture2D target){
  Image image=LoadImageFromTexture(target.texture);ImageFlipVertical(&image);return image;
 };
 RenderTexture2D layer=LoadRenderTexture(W,H);
 auto wind=[&](float time){
  game.time=time;
  BeginTextureMode(layer);ClearBackground(BLANK);
  native_world::beginOpaqueFrame();wind_post::draw();native_world::finishOpaqueBackground();
  EndTextureMode();return read(layer);
 };
 check(wind_post::shader.id!=rlGetShaderIdDefault()&&wind_post::driftUniform>=0&&
       wind_post::shapeUniform>=0&&wind_post::actorUniform>=0&&wind_post::fieldUniform>=0,
       "real shader and world projection uniforms");
 const WorldLocation regions[]={WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano};
 const V positions[]={{36,32},{29,33},{36,46}};
 for(int i=0;i<3;i++){
  const auto& profile=wind_post::weather[i];
  std::string name=profile.file;
  auto& mask=wind_post::masks[i];
  check(mask.id&&mask.width==640&&mask.height==320&&mask.mipmaps>1,"loaded painted texture "+name);
  game=Game{};game.activateField(regions[i]);game.player=positions[i];
  intro={};towerStory={};clearTransientUi();invalidatePausedWorld();
  welcome=paused=false;mouse={0,0};game.messageTime=0;
  const Vector2 player=screen(game.player);
  Image a=wind(8.6f),b=wind(9.7f);
  Color* first=LoadImageColors(a);Color* next=LoadImageColors(b);
  int occupied=0,moved=0,leaked=0,peak=0,actorPeak=0;
  long long coverage=0;
  for(int y=0;y<H;y++)for(int x=0;x<W;x++){
   const int p=y*W+x;
   if(x>=WorldW){leaked+=first[p].a!=0||next[p].a!=0;continue;}
   const int opacity=first[p].a;
   occupied+=opacity>=3;
   moved+=std::abs(opacity-int(next[p].a))>=2;
   coverage+=opacity;
   peak=std::max(peak,std::max(opacity,int(next[p].a)));
   if(std::hypot(float(x)-player.x,float(y)-player.y+47)<36){
    actorPeak=std::max(actorPeak,std::max(opacity,int(next[p].a)));
   }
  }
  const double mean=double(coverage)/(WorldW*H*255);
  constexpr double minimumMean[]={.0034,.0105,.0019};
  log<<name<<" alpha mean "<<mean<<", max "<<peak<<", near hero "<<actorPeak
     <<", visibly moved pixels "<<moved<<'\n';
  check(mean>minimumMean[i]&&occupied>7000&&moved>6000,"visible and moving gusts "+name);
  check(peak<=std::ceil(255*profile.opacity)+1&&mean<.026,"bounded transparent coverage "+name);
  check(actorPeak<=std::ceil(255*profile.opacity*.24f)+2,"local opacity reduced over hero "+name);
  check(leaked==0,"wind stops at world viewport "+name);
  const Vector2 camera=cameraPixels();
  for(V movement:{V{1.25f,-.9f},V{.9f,.8f}}){
   game.player=positions[i]+movement;
   const Vector2 shifted=cameraPixels();
   const int dx=int(shifted.x-camera.x),dy=int(shifted.y-camera.y);
   Image pan=wind(8.6f);
   Color* panned=LoadImageColors(pan);
   int covered=0,signal=0,error=0;
   // Compare the same map pixels below the hero and inside the constant
   // opacity band. Camera-edge and hero protection masks are screen-local.
   for(int y=700;y<802;y++)for(int x=20;x<WorldW-20;x++){
    const int px=x-dx,py=y-dy;
    if(px<20||px>=WorldW-20||py<700||py>=802)continue;
    const int before=first[y*W+x].a,after=panned[py*W+px].a;
    covered+=before>1;
    signal+=before;
    error+=std::abs(before-after);
   }
   log<<name<<" camera delta "<<dx<<','<<dy<<", wind pixels "<<covered
      <<", aligned error "<<error<<'/'<<signal<<'\n';
   check(covered>250&&error<std::max(12,signal/45),"world wind holds position during camera pan "+name);
   UnloadImageColors(panned);UnloadImage(pan);
  }
  game.player=positions[i];
  UnloadImageColors(first);UnloadImageColors(next);UnloadImage(a);UnloadImage(b);

  game.time=8.6f;
  game.fireballs.push_back({game.player+V{1.1f,-.5f},{1,0},3,10,false,Spell::Fireball,.26f});
  game.effects.push_back({game.player+V{1.1f,-.5f},"27",1,0,true,false,90});
  renderScene();
  Image active=read(canvas);
  Texture2D texture=mask;
  mask={};renderScene();mask=texture;
  Image before=read(canvas);
  Color* p=LoadImageColors(active);Color* q=LoadImageColors(before);
  int world=0,hud=0;
  for(int y=0;y<H;y++)for(int x=0;x<W;x++){
   const int n=y*W+x;
   const bool changed=p[n].r!=q[n].r||p[n].g!=q[n].g||p[n].b!=q[n].b;
   if(x<WorldW)world+=changed;
   else hud+=changed;
  }
  check(world>3000&&hud==0,"post effect changes world pixels only "+name);
  check(ExportImage(active,(output/(name+".png")).string().c_str())&&
        ExportImage(before,(output/(name+"-before.png")).string().c_str()),"real scene exported "+name);
  UnloadImageColors(p);UnloadImageColors(q);UnloadImage(active);UnloadImage(before);
 }
 game.enterMageTower();
 Image interior=wind(8.6f);
 Color* pixels=LoadImageColors(interior);
 check(std::none_of(pixels,pixels+W*H,[](Color color){return color.a!=0;}),"no weather indoors");
 UnloadImageColors(pixels);UnloadImage(interior);UnloadRenderTexture(layer);
 check(!audioSystem.available,"audio device remains closed");
 std::ofstream report(output/"report.json");
 report<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures
       <<",\"window_hidden\":true,\"audio_device_opened\":false}\n";
 wind_post::unload();audioSystem.shutdown();sync_present::shutdown();CloseWindow();
 return failures?2:0;
}
