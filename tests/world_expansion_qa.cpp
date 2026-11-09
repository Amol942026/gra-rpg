#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include <iostream>
int main(int argc,char** argv){
 if(argc!=4||std::string(argv[1])!="--no-audio"||std::string(argv[3])!="--background-test")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_ALWAYS_RUN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall expanded realms --no-audio");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 int checks=0,failures=0,captures=0;std::ofstream log(root/"checks.txt");
 auto check=[&](bool b,const std::string& label){checks++;failures+=!b;log<<(b?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 auto reset=[&](WorldLocation realm){game=Game{};game.activateField(realm);intro={};towerStory={};clearTransientUi();welcome=help=paused=false;mouse={0,0};game.messageTime=0;};
 auto texture=[](WorldLocation realm){return realm==WorldLocation::Forest?terrain.texture:realm==WorldLocation::Desert?desert_art::terrain.texture:volcano_art::terrain.texture;};
 auto capture=[&](const std::string& name){renderScene();Image img=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&img);check(img.data&&ExportImage(img,(root/(name+".png")).string().c_str()),"actual full gameplay capture / "+name);UnloadImage(img);check(IsWindowHidden(),"capture stays hidden / "+name);captures++;};
 auto moveNear=[&](V point){float closest=1000;V best{};for(float y=-4;y<=4;y+=.25f)for(float x=-4;x<=4;x+=.25f){V p=point+V{x,y};if(game.walkable(p)&&length(V{x,y})<closest){closest=length(V{x,y});best=p;}}if(closest<1000)game.player=best;return closest<1000;};
 std::ofstream layout(root/"layout.json");layout<<'[';bool first=true;std::vector<double> completed;
 auto finish=reinterpret_cast<void(*)()>(glfwGetProcAddress("glFinish"));
 RenderTexture2D overview=render_targets::colorOnly(1800,600);BeginTextureMode(overview);ClearBackground(Ink);EndTextureMode();int column=0;
 for(auto realm:{WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano}){
  reset(realm);std::string name=realm==WorldLocation::Forest?"forest":realm==WorldLocation::Desert?"desert":"volcano";auto ground=texture(realm);
  check(IsTextureValid(ground)&&ground.width==TerrainCacheWidth&&ground.height==TerrainCacheHeight&&ground.width<8192,"bounded terrain cache loads / "+name);
  Image pixels=LoadImageFromTexture(ground);bool opaque=true;int extensionSamples=0;
  for(float y=4;y<98;y+=3)for(float x=4;x<98;x+=3){V p{x,y};if(x<72&&y<72)continue;if(realm==WorldLocation::Volcano&&volcanoLava(p,2.5f))continue;
   int px=int((TerrainOriginX+(x-y)*40)*TerrainBakeScale),py=pixels.height-1-int((90+(x+y)*20)*TerrainBakeScale);
   opaque&=GetImageColor(pixels,px,py).a>=250;extensionSamples++;
  }
  check(opaque&&extensionSamples>200,"new territory has continuous painted opaque ground / "+name);UnloadImage(pixels);
  if(!first)layout<<',';first=false;layout<<"{\"realm\":\""<<name<<"\",\"size\":"<<MapSize<<",\"enemies\":"<<game.enemies.size()<<",\"props\":"<<game.props.size()<<",\"hills\":"<<terrain_height::count(realm)<<'}';
  float factor=560.f/TerrainWidth,x0=20+column*600.f,y0=120;
  auto point=[&](V p){return Vector2{x0+(TerrainOriginX+(p.x-p.y)*40)*factor,y0+(90+(p.x+p.y)*20)*factor};};
  BeginTextureMode(overview);txt(name=="forest"?"WIELKA KNIEJA":name=="desert"?"MORZE PIASKÓW":"KALDERA POPIOŁU",x0,25,23,Gold,true);
  txt(TextFormat("102 × 102  |  %d przeciwników  |  %d obiektów",int(game.enemies.size()),int(game.props.size())),x0,65,15,Text);
  DrawTexturePro(ground,{0,0,float(ground.width),-float(ground.height)},{x0,y0,560,TerrainHeight*factor},{0,0},0,WHITE);
  for(const auto& p:game.props)DrawCircleV(point(p.pos),1.25f,realm==WorldLocation::Forest?rgb(153,196,120):rgb(235,205,154));
  for(const auto& e:game.enemies)DrawCircleV(point(e.home),2.25f,enemyBoss(e.type)?Gold:rgb(232,104,91));
  const V old[]={{.5f,.5f},{.5f,70.5f},{70.5f,70.5f},{70.5f,.5f},{.5f,.5f}};
  for(int i=0;i<4;i++)DrawLineEx(point(old[i]),point(old[i+1]),1.5f,rgb(105,204,221));
  txt("Turkus: dawny obszar  |  Czerwone punkty: wrogowie",x0,450,14,Muted);EndTextureMode();column++;
  check(moveNear(realm==WorldLocation::Forest?ForestArrival:realm==WorldLocation::Desert?DesertArrival:VolcanoArrival),"realm arrival remains walkable / "+name);capture(name+"-arrival");
  int location=0;for(V target:{V{88,30},V{88,60},V{88,88},V{24,86},V{54,86}}){
   check(moveNear(target),"walkable new region camera / "+name);game.time=2.1f;
   V saved=game.player;for(int i=0;i<6;i++){Vector2 at=screen(saved);V recovered=toWorld(at);check(distance(saved,recovered)<.03f,"height-aware pointer returns the hero's actual world position in new terrain");}
   capture(name+"-extension-"+std::to_string(location++));
   for(int frame=0;frame<12;frame++){game.hp=game.maxHp();double start=GetTime();game.update(1.f/60,{},false);renderScene();rlDrawRenderBatchActive();if(finish)finish();completed.push_back((GetTime()-start)*1000);}
   check(texture(realm).id==ground.id,"walking in new sectors reuses the same terrain cache / "+name);
  }
  check(moveNear(realm==WorldLocation::Forest?ForestGuardian:realm==WorldLocation::Desert?DesertBossHome:VolcanoBossHome),"original guardian arena remains accessible / "+name);capture(name+"-guardian");
 }
 layout<<']';saveTexture(overview,(root/"overview.png").string().c_str());UnloadRenderTexture(overview);
 check(volcano_art::roadBakeValid,"continuous enlarged road mask uses a valid native shader");
 for(size_t i=0;i<VolcanoBridges.size();i++){
  auto deck=volcano_art::bridgeDeck(int(i));V center{};for(V p:deck)center=center+p*.25f;
  check(std::abs(center.x-lavaCenter(VolcanoBridgeRivers[i],center.y))<.001f&&!volcanoLava(center),"each painted deck aligns with its actual traversable lava bridge");
 }
 check(!audioSystem.available&&audioSystem.playedEffects==0&&IsWindowHidden(),"entire graphical integration stays silent and hidden");
 std::sort(completed.begin(),completed.end());double mean=std::accumulate(completed.begin(),completed.end(),0.)/completed.size(),p95=completed[(completed.size()-1)*95/100];
 std::ofstream out(root/"summary.json");out<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"realms\":3,\"map_side\":102,\"area_multiplier\":"<<float(MapSize*MapSize)/(72*72)<<",\"captured_frames\":"<<captures<<",\"terrain_cache_width\":"<<TerrainCacheWidth<<",\"terrain_cache_height\":"<<TerrainCacheHeight<<",\"hidden\":true,\"audio_opened\":false,\"presentation_measured\":false,\"completed_update_render_mean_ms\":"<<mean<<",\"completed_update_render_p95_ms\":"<<p95<<'}';
 std::cout<<"Expanded realm renderer: "<<checks<<" checks, "<<failures<<" failures\n";audioSystem.shutdown();CloseWindow();return failures?2:0;
}
