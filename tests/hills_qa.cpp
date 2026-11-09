#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include <iostream>
int main(int argc,char** argv){
 if(argc!=4||std::string(argv[1])!="--no-audio"||std::string(argv[3])!="--background-test")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_ALWAYS_RUN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall walkable hills --no-audio");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 int checks=0,failures=0;std::ofstream log(root/"checks.txt");auto check=[&](bool ok,const char* label){checks++;failures+=!ok;log<<(ok?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 check(IsWindowHidden()&&!audioSystem.available,"all elevation diagnostics remain hidden and silent");
 check(IsShaderValid(terrain_surface::lighting),"slope lighting shader is supported by the actual graphics driver");
 check(volcano_art::roadBakeValid,"continuous road union was baked using a valid native shader and mask");
 Image cachedGround=LoadImageFromTexture(volcano_art::terrain.texture);bool opaque=true;
 for(float y=2;y<MapWalkMax;y+=.5f)for(float x=2;x<MapWalkMax;x+=.5f){Vector2 p=volcano_art::projected({x,y});opaque&=GetImageColor(cachedGround,int(p.x*TerrainBakeScale),cachedGround.height-1-int(p.y*TerrainBakeScale)).a==255;}
 check(opaque,"feathered roads and ash preserve opaque land without holes revealing the distant backdrop");UnloadImage(cachedGround);
 for(const auto& road:VolcanoRoads)for(V junction:road)check(volcano_roads::coverage(junction)>.99f,"every authored junction and endpoint has continuous opaque road coverage");
 bool bridgeSafe=true,centerContinuous=true;
 for(const auto& segment:volcano_roads::segments())for(int i=0;i<=12;i++){
  V p=segment.a+(segment.b-segment.a)*(i/12.f);bridgeSafe&=!volcanoLava(p);centerContinuous&=volcano_roads::coverage(p)>.99f;
 }
 check(bridgeSafe,"rounded path centerlines cross lava only on existing safe bridges");
 check(centerContinuous,"one road material covers all curved centerline segments without seams");
 auto reset=[&](WorldLocation realm){game=Game{};game.activateField(realm);game.player=realm==WorldLocation::Forest?ForestArrival:realm==WorldLocation::Desert?DesertArrival:VolcanoArrival;intro={};towerStory={};clearTransientUi();welcome=paused=help=false;mouse={0,0};game.messageTime=0;};
 auto shot=[&](const std::string& name){renderScene();saveTexture(canvas,(root/(name+".png")).string().c_str());};
 std::ofstream elevations(root/"elevations.json");elevations<<'[';bool first=true;
 std::vector<double> timings;auto finish=reinterpret_cast<void(*)()>(glfwGetProcAddress("glFinish"));
 RenderTexture2D actorProbe=LoadRenderTexture(W,H);
 for(WorldLocation realm:{WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano}){
  reset(realm);int count=int(terrain_height::count(realm));const char* name=realm==WorldLocation::Forest?"forest":realm==WorldLocation::Desert?"desert":"volcano";
  // Flood from the actual arrival. Dense connected ridges need not have a
  // zero-height patch inside each individual hill's bounding rectangle.
  constexpr int Grid=MapSize*2;std::vector<unsigned char> reachable(Grid*Grid);std::vector<int> frontier;
  auto point=[](int index){return V{float(index%Grid)*.5f,float(index/Grid)*.5f};};
  int start=-1;float nearest=2;
  for(int index=0;index<Grid*Grid;index++){V p=point(index);float d=distance(p,game.player);if(d<nearest&&game.walkable(p)&&game.walkable((p+game.player)*.5f)){nearest=d;start=index;}}
  if(start>=0){frontier.push_back(start);reachable[start]=1;}
  for(size_t next=0;next<frontier.size();next++){
   int current=frontier[next],x=current%Grid,y=current/Grid;V at=point(current);
   for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++){
    int nx=x+dx,ny=y+dy;if((dx==0&&dy==0)||nx<0||nx>=Grid||ny<0||ny>=Grid)continue;
    int index=ny*Grid+nx;V destination=point(index);
    if(!reachable[index]&&game.walkable(destination)&&game.walkable((at+destination)*.5f)){reachable[index]=1;frontier.push_back(index);}
   }
  }
  check(frontier.size()>5000,"the actual arrival connects to a large walkable realm with all scenery enabled");
  const auto& mesh=terrain_surface::surfaces[int(realm)];check(mesh.vertices.size()==size_t(terrain_surface::Nodes*terrain_surface::Nodes),"realm has a precomputed raised surface with shared vertices");
  bool finite=true,monotone=true;float largest=0,minimumJacobian=100;
  for(float y=1;y<71;y+=.5f)for(float x=1;x<71;x+=.5f){V p{x,y},g=terrain_height::gradient(realm,p);float h=game.elevation(p),j=40-terrain_height::PixelsPerUnit*(g.x+g.y);finite&=std::isfinite(h)&&h>=0&&h*terrain_height::PixelsPerUnit<=terrain_height::MaxPixels;monotone&=j>1;largest=std::max(largest,h);minimumJacobian=std::min(minimumJacobian,j);}
  check(finite&&largest>2,"world heights are finite, bounded and substantially above the plain");check(monotone,"all ramps have a unique ground ray intersection without folded geometry");
  check(count>=14,"each realm contains many more authored hills with unequal footprints and heights");
  for(int hill=0;hill<count;hill++){
   auto h=terrain_height::hill(realm,hill);
   game.player=h.center;float crest=game.elevation(game.player);check(crest>.3f,"each authored landform has substantial world-space elevation");
   float reachableHeight=0;
   for(int index:frontier){V p=point(index);if(distance(p,h.center)<2)reachableHeight=std::max(reachableHeight,game.elevation(p));}
   check(reachableHeight>crest*.9f,"high ground near each authored summit is reachable from the actual arrival with scenery and collisions enabled");
   for(float resolution:{.8f,1.f,1.25f,1.5f,2.f}){renderScale=resolution;for(V p:std::array<V,5>{{h.center,h.center+V{h.radius.x*.7f,0},h.center+V{0,h.radius.y*.7f},h.center-V{h.radius.x,0},h.center-V{0,h.radius.y}}}){V restored=toWorld(screen(p));check(distance(p,restored)<.06f,"screen picking roundtrips on crests and slopes at each display scale");}}
   renderScale=1;
   // Real movement uses the same ground profile. Isolate ramps from scenery
   // for a deterministic ascent/descent, then restore the actual realm below.
   auto props=std::move(game.props);auto enemies=std::move(game.enemies);game.props.clear();game.enemies.clear();
   game.player={h.center.x,std::max(MapWalkMin+.2f,h.center.y-h.radius.y-1)};float peak=0,travelError=0,rise=0,descent=0;
   float end=std::min(MapWalkMax-.2f,h.center.y+h.radius.y+1);
   for(int step=0;step<1200&&game.player.y<end;step++){V before=game.player;game.move({0,.03f});float travelled=game.surfaceDistance(before,game.player),change=game.elevation(game.player)-game.elevation(before);rise+=std::max(0.f,change);descent+=std::max(0.f,-change);peak=std::max(peak,game.elevation(game.player));travelError=std::max(travelError,std::abs(travelled-.03f));}
   check(peak>crest*.95f&&rise>.15f&&descent>.15f&&game.player.y>=end,"hero walks up and down the actual dense terrain across each hill");
   check(travelError<.001f,"movement follows three-dimensional ground distance rather than flat XY speed");
   game.player=h.center+V{20,0};float hp=enemyInfo(0).health;V from=h.center-V{0,h.radius.y+1};game.enemies.push_back({8000,0,from,from,hp,hp});float enemyPeak=0;
   for(int step=0;step<650&&game.enemies[0].pos.y<h.center.y+h.radius.y+1;step++){game.moveEnemies(.02f,{{0,4}});enemyPeak=std::max(enemyPeak,game.elevation(game.enemies[0].pos));}
   check(enemyPeak>crest*.95f,"enemy movement climbs the same actual ground height as the hero");
   game.props=std::move(props);game.enemies=std::move(enemies);game.player=h.center;game.messageTime=0;
   shot(std::string(name)+"-hill-"+std::to_string(hill));
   // renderScene intentionally releases prepared rigs. Inspect a fresh real
   // shadow/body pass and its GPU pixels, rather than a released cache.
   world_shadows::prepare();Vector2 foot=screen(game.player);
   BeginTextureMode(actorProbe);ClearBackground(BLANK);hero(foot.x,foot.y,1);EndTextureMode();
   auto heroRig=std::find_if(actor_art::frameRigs.begin(),actor_art::frameRigs.end(),[](const auto& entry){return entry.first==0;});
   check(heroRig!=actor_art::frameRigs.end()&&std::hypot(heroRig->second.anchor().x-foot.x,heroRig->second.anchor().y-foot.y)<.01f,"prepared hero geometry is anchored on the raised ground, not on its flat shadow receiver");
   Image body=LoadImageFromTexture(actorProbe.texture);ImageFlipVertical(&body);Color* pixels=LoadImageColors(body);int lowest=-1;
   for(int y=0;y<H;y++)for(int x=int(foot.x)-16;x<=int(foot.x)+16;x++)if(pixels[y*W+x].a>96)lowest=std::max(lowest,y);
   check(lowest>=foot.y-5&&lowest<=foot.y+12,"actual GPU-rendered feet meet the raised ground instead of floating below the summit");
   UnloadImageColors(pixels);UnloadImage(body);actor_art::finishShadowFrame();
   if(!first)elevations<<',';first=false;elevations<<"{\"realm\":\""<<name<<"\",\"hill\":"<<hill<<",\"crest\":"<<crest<<",\"crest_pixels\":"<<crest*32<<",\"min_projection_jacobian\":"<<minimumJacobian<<'}';
  }
  reset(realm);auto h=terrain_height::hill(realm,0);
  game.enemies.clear();game.props.clear();game.player=h.center-V{0,h.radius.y+1};game.equipment[4]={};fs::create_directories(root/name);
  for(int frame=0;frame<420;frame++){
   game.update(1.f/60,{0,1},false);game.messageTime=0;
   if(frame%7==0){renderScene();Image full=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&full);Image crop=ImageFromImage(full,{0,120,1050,660});ImageResize(&crop,525,330);char file[40];std::snprintf(file,sizeof(file),"frame-%03d.png",frame/7);ExportImage(crop,(root/name/file).string().c_str());UnloadImage(crop);UnloadImage(full);}
  }
  game.player=h.center;
  for(int frame=0;frame<65;frame++){double start=GetTime();renderScene();rlDrawRenderBatchActive();if(finish)finish();if(frame>=15)timings.push_back((GetTime()-start)*1000);}
 }
 elevations<<']';elevations.close();
 reset(WorldLocation::Volcano);int junction=0;
 for(V p:std::array<V,6>{{{16,54},{38,43},{17,23},{39,58},{61,42},{lavaCenter(0,48),48}}}){game.player=p;shot("volcano-junction-"+std::to_string(junction++));}
 game.player={16,54};RenderTexture2D groundProbe=LoadRenderTexture(WorldW,H);Vector2 origin=screenFlat({0,0});
 BeginTextureMode(groundProbe);ClearBackground(BLANK);DrawTexturePro(volcano_art::terrain.texture,{0,0,float(TerrainCacheWidth),-float(TerrainCacheHeight)},{origin.x-TerrainOriginX,origin.y-90,float(TerrainWidth),float(TerrainHeight)},{0,0},0,WHITE);EndTextureMode();saveTexture(groundProbe,(root/"road-flat-cache.png").string().c_str());
 BeginTextureMode(groundProbe);ClearBackground(BLANK);terrain_surface::draw(volcano_art::terrain.texture);EndTextureMode();saveTexture(groundProbe,(root/"road-raised-ground.png").string().c_str());
 auto& vertices=terrain_surface::surfaces[int(WorldLocation::Volcano)].vertices;std::vector<Color> colors;for(auto& v:vertices){colors.push_back(v.light);v.light={196,196,196,255};}
 BeginTextureMode(groundProbe);ClearBackground(BLANK);terrain_surface::draw(volcano_art::terrain.texture);EndTextureMode();saveTexture(groundProbe,(root/"road-unlit-ground.png").string().c_str());
 for(size_t i=0;i<vertices.size();i++)vertices[i].light=colors[i];UnloadRenderTexture(groundProbe);
 reset(WorldLocation::Volcano);for(int river=0;river<2;river++)for(float y=2;y<70;y+=1)check(game.elevation({lavaCenter(river,y),y})==0,"lava beds remain level beneath the raised basalt terrain");
 game=Game{};game.enterMageTower();check(game.elevation(game.player)==0,"existing tower floors keep their separate height system");
 std::sort(timings.begin(),timings.end());double mean=std::accumulate(timings.begin(),timings.end(),0.)/timings.size(),p95=timings[(timings.size()-1)*95/100];
 std::ofstream summary(root/"summary.json");summary<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"realms\":3,\"walkable_hills\":"<<terrain_height::Count<<",\"road_segments\":"<<volcano_roads::segments().size()<<",\"movement_frames\":180,\"hidden\":true,\"audio_opened\":false,\"completed_render_mean_ms\":"<<mean<<",\"completed_render_p95_ms\":"<<p95<<",\"presentation_measured\":false}";
 std::cout<<"Hills: "<<checks<<" checks, "<<failures<<" failures; hidden render "<<mean<<" ms\n";UnloadRenderTexture(actorProbe);audioSystem.shutdown();CloseWindow();return failures?2:0;
}
