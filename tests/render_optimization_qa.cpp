#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include <iostream>
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_ALWAYS_RUN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall renderer equivalence --no-audio");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 int checks=0,failures=0,pairs=0,maxDifference=0;std::ofstream log(root/"checks.txt");
 auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;log<<(ok?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 auto difference=[](Image a,Image b){auto* p=LoadImageColors(a);auto* q=LoadImageColors(b);int count=0;for(int i=0;i<a.width*a.height;i++)count+=std::abs(int(p[i].r)-q[i].r)+std::abs(int(p[i].g)-q[i].g)+std::abs(int(p[i].b)-q[i].b)+std::abs(int(p[i].a)-q[i].a)>6;UnloadImageColors(p);UnloadImageColors(q);return count;};
 auto reset=[](){game=Game{};intro={};towerStory={};clearTransientUi();welcome=help=paused=false;mouse={0,0};game.messageTime=0;};
 check(terrain.depth.id==0&&desert_art::terrain.depth.id==0,"both static terrain textures omit unused depth attachments");
 check(render_targets::omittedDepthPixels>=std::uint64_t(TerrainWidth)*TerrainHeight*2,"large static maps avoid redundant depth storage");
 RenderTexture2D sample=LoadRenderTexture(480,480);int capacity=actor_art::creatureCapacity;unsigned shadowAllocations=shadow_geometry::allocations;
 auto pair=[&](const std::string& label,auto draw){
  actor_art::finishShadowFrame();actor_art::gpuCreatureDraw=false;BeginTextureMode(sample);ClearBackground(BLANK);draw();EndTextureMode();Image reference=LoadImageFromTexture(sample.texture);
  actor_art::gpuCreatureDraw=true;BeginTextureMode(sample);ClearBackground(BLANK);draw();EndTextureMode();Image actual=LoadImageFromTexture(sample.texture);
  int changed=difference(reference,actual);pairs++;maxDifference=std::max(maxDifference,changed);check(changed<32,label+" GPU vs original / changed="+std::to_string(changed));
  if(changed>=32){ImageFlipVertical(&actual);ImageFlipVertical(&reference);ExportImage(actual,(root/(label+"-actual.png")).string().c_str());ExportImage(reference,(root/(label+"-reference.png")).string().c_str());}
  UnloadImage(reference);UnloadImage(actual);
 };
 reset();
 auto actor=[&](int action,AnimationState state,Color outline=BLANK){actor_art::draw(0,240,390,2,state,0,!game.equipment[1].empty(),!game.equipment[2].empty(),archetype(game.equipment[0].kind),false,outline,nullptr,-1,action==3?.37f:-1,action==4?.6f:-1,game.ammoCount(),false,&game.equipment,-1,action==5?.5f:-1,0,-1);};
 for(int dir=0;dir<8;dir++)for(int action=0;action<6;action++){
  reset();if(action==4)game.equipment[0]=item(Kind::Bow);AnimationState state;state.direction=Direction(dir);state.phase=.23f;state.moving=action>0&&action<3;state.motionInitialized=true;state.moveBlend=state.moving?1:0;state.runBlend=action==2?1:0;state.swordStow=action==2?1:0;
  pair("hero-"+std::to_string(dir)+"-"+std::to_string(action),[&]{actor(action,state,dir==0?Gold:BLANK);});
 }
 int equipmentCount=0;
 for(int k=0;k<int(Kind::Count);k++)if(int slot=gearSlot(Kind(k));slot>=0){reset();game.equipment[slot]=item(Kind(k),2);AnimationState s;s.direction=Direction::SE;pair("gear-"+std::to_string(k),[&]{actor(0,s);});equipmentCount++;}
 for(int type=0;type<EnemyTypeCount;type++)for(int dir=0;dir<8;dir++){
  AnimationState s;s.direction=Direction(dir);s.moving=true;s.phase=.17f;s.moveBlend=1;s.motionInitialized=true;
  pair("creature-"+std::to_string(type)+"-"+std::to_string(dir),[&]{actor_art::draw(type+1,240,390,type==3?.7f:type==11?.8f:1.8f,s,0,false,false,Kind::Sword,false,dir==0?Gold:BLANK);});
 }
 check(actor_art::creatureCapacity==capacity,"body and outline buffers stay within their preloaded capacity");UnloadRenderTexture(sample);
 for(float factor:{1.f,1.2f}){
  prepareCanvas(factor);reset();renderScene();unsigned builds=hero_portrait::builds;
  for(int i=0;i<12;i++)renderScene();check(hero_portrait::builds==builds,"unchanged portrait is not regenerated during regular frames");
  for(int k=0;k<int(Kind::Count);k++)if(int slot=gearSlot(Kind(k));slot>=0){
   game.equipment[slot]=item(Kind(k),1);hero_portrait::enabled=true;renderScene();Image cached=LoadImageFromTexture(canvas.texture);
   hero_portrait::enabled=false;renderScene();Image direct=LoadImageFromTexture(canvas.texture);hero_portrait::enabled=true;
   int changed=difference(cached,direct);check(changed<32,"portrait pixels match direct rendering for gear "+std::to_string(k)+" / changed="+std::to_string(changed));
   if(k==1){ImageFlipVertical(&cached);ImageFlipVertical(&direct);ExportImage(cached,(root/("portrait-"+std::to_string(factor)+"-cached.png")).string().c_str());ExportImage(direct,(root/("portrait-"+std::to_string(factor)+"-direct.png")).string().c_str());}
   UnloadImage(cached);UnloadImage(direct);
  }
 }
 prepareCanvas(1);reset();game.activateField(WorldLocation::Desert);game.dawnQuest=DawnQuest::AtTower;game.time=2.3f;
 const V views[]={DesertArrival,DesertOasis+V{0,-5},DesertBossHome+V{0,-4},{35,35},{25,22},{50,37},{2,2},{2,35},{2,69},{35,69},{69,69},{69,35},{69,2},{35,2}};
 for(int view=0;view<int(std::size(views));view++){
  game.player=views[view];desert_art::optimizeBackdrop=false;renderScene();Image original=LoadImageFromTexture(canvas.texture);
  desert_art::optimizeBackdrop=true;desert_art::backdropReady=false;renderScene();Image optimized=LoadImageFromTexture(canvas.texture);int changed=difference(original,optimized);
  check(changed<32,"occlusion/camera backdrop cache preserves final desert pixels / view="+std::to_string(view)+" changed="+std::to_string(changed));
  if(changed>=32){ImageFlipVertical(&original);ImageFlipVertical(&optimized);ExportImage(original,(root/("backdrop-"+std::to_string(view)+"-reference.png")).string().c_str());ExportImage(optimized,(root/("backdrop-"+std::to_string(view)+"-actual.png")).string().c_str());}
  UnloadImage(original);UnloadImage(optimized);unsigned before=desert_art::backdropBuilds;for(int i=0;i<8;i++)renderScene();check(before==desert_art::backdropBuilds,"stationary camera reuses both distant layers");
 }
 reset();game.enemies[0].pos=game.player+V{2,-2};world_shadows::prepare();Enemy& e=game.enemies[0];Vector2 at=screen(e.pos);int probes=0;
 for(int y=-100;y<25;y+=7)for(int x=-75;x<75;x+=7){Vector2 p{at.x+x,at.y+y};actor_art::pickingFromPreparedFrame=false;bool reference=actor_art::drawEnemy(e,at.x,at.y,BLANK,&p);actor_art::pickingFromPreparedFrame=true;bool cached=actor_art::drawEnemy(e,at.x,at.y,BLANK,&p);check(reference==cached,"prepared current-frame actor retains exact pointer geometry");probes++;}
 actor_art::pickingFromPreparedFrame=false;actor_art::finishShadowFrame();
 prepareCanvas(1.2f);reset();game.activateField(WorldLocation::Desert);game.player=DesertOasis+V{0,-5};game.messageTime=0;
 nativeWorldRendering=true;renderScene();Image native=LoadImageFromTexture(canvas.texture);
 nativeWorldRendering=false;renderScene();Image balanced=LoadImageFromTexture(canvas.texture);
 check(renderScale==1.2f&&canvas.texture.width==1728&&canvas.texture.height==1080,"desert render cap preserves native final canvas and scale");
 check(worldCanvas.texture.width==945&&worldCanvas.texture.height==810,"only desert scenery uses the smaller world buffer");
 Image nativeHud=ImageFromImage(native,{WorldW*renderScale,0,float(canvas.texture.width)-WorldW*renderScale,float(canvas.texture.height)}),balancedHud=ImageFromImage(balanced,{WorldW*renderScale,0,float(canvas.texture.width)-WorldW*renderScale,float(canvas.texture.height)});
 check(difference(nativeHud,balancedHud)==0,"entire equipment and statistics HUD stays pixel-identical at native display resolution");
 UnloadImage(nativeHud);UnloadImage(balancedHud);ImageFlipVertical(&native);ImageFlipVertical(&balanced);ExportImage(native,(root/"desert-native.png").string().c_str());ExportImage(balanced,(root/"desert-balanced.png").string().c_str());UnloadImage(native);UnloadImage(balanced);
 check(actor_art::recycledRigs.size()<=64,"retained actor storage is bounded");
 const struct {const char* name;V at;bool desert;} costViews[]={{"forest-north",{24,41},false},{"forest-dense",{40,25},false},{"forest-boss",{21,8},false},{"forest-edge",{2,2},false},{"desert-oasis",DesertOasis+V{0,-5},true},{"desert-boss",DesertBossHome+V{0,-4},true}};
 auto finish=reinterpret_cast<void(*)()>(glfwGetProcAddress("glFinish"));
 auto percentile=[](std::vector<double> values){std::sort(values.begin(),values.end());return values[(values.size()-1)*95/100];};
 std::ofstream cost(root/"render-cost.json");cost<<"{\"hidden\":true,\"audio_opened\":false,\"presentation_measured\":false,\"scenes\":[";bool costComma=false;
 for(const auto& view:costViews){
  reset();if(view.desert)game.activateField(WorldLocation::Desert);game.player=view.at;game.messageTime=0;game.time=2.3f;
  tree_wind::shared=false;actor_art::fastPacking=false;renderScene();Image before=LoadImageFromTexture(canvas.texture);
  tree_wind::shared=true;actor_art::fastPacking=true;renderScene();Image after=LoadImageFromTexture(canvas.texture);
  check(difference(before,after)==0,std::string("shared wind and direct packing preserve every pixel / ")+view.name);UnloadImage(before);UnloadImage(after);
  Game initial=game;
  std::array<std::vector<double>,2> measurements;
  for(int trial=0;trial<3;trial++)for(int order=0;order<2;order++){
   bool optimized=(order+trial)%2;game=initial;
   tree_wind::shared=optimized;actor_art::fastPacking=optimized;
   for(int frame=0;frame<100;frame++){
    game.time=frame/60.f;game.animation.advance(.015f,.005f,1.f/60,true);
    for(auto& e:game.enemies)e.animation.advance(.01f,.005f,1.f/60);
    double start=GetTime();renderScene();rlDrawRenderBatchActive();if(finish)finish();
    if(frame>=20)measurements[optimized].push_back((GetTime()-start)*1000);
   }
  }
  for(int optimized=0;optimized<2;optimized++){
   const auto& elapsed=measurements[optimized];
   if(costComma)cost<<',';costComma=true;
   cost<<"{\"name\":\""<<view.name<<"\",\"optimized\":"<<(optimized?"true":"false")<<",\"trials\":3,\"frames\":"<<elapsed.size()<<",\"completed_render_mean_ms\":"<<std::accumulate(elapsed.begin(),elapsed.end(),0.)/elapsed.size()<<",\"completed_render_p95_ms\":"<<percentile(elapsed)<<",\"shadow_casters\":"<<world_shadows::visibleCasters<<",\"prepared_actors\":"<<world_shadows::preparedActors<<"}";cost.flush();
  }
 }
 tree_wind::shared=true;actor_art::fastPacking=true;cost<<"]}";
 check(tree_wind::frameSamples.size()<=256&&tree_wind::frameSamples.capacity()==256,"shared wind cache stays within its preloaded budget");
 check(actor_art::creatureCapacity==capacity&&actor_art::creatureVertices.size()==size_t(capacity)*3,"direct packing reuses preloaded actor storage");
 check(shadow_geometry::allocations==shadowAllocations,"all scenes reuse shadow buffers allocated during loading");
 check(IsWindowHidden(),"all renderer and cost tests kept their window hidden");
 check(!audioSystem.available&&audioSystem.playedEffects==0,"all tests remained silent");
 std::ofstream out(root/"summary.json");out<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"actor_pairs\":"<<pairs<<",\"max_changed_actor_pixels\":"<<maxDifference<<",\"equipment_variants\":"<<equipmentCount<<",\"pointer_probes\":"<<probes<<",\"audio_opened\":false}";
 std::cout<<"Renderer equivalence: "<<checks<<" checks, "<<failures<<" failures\n";CloseWindow();return failures?2:0;
}
