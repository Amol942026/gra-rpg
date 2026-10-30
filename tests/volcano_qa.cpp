#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include <iostream>
int main(int argc,char** argv){
 if(argc!=4||std::string(argv[1])!="--no-audio"||std::string(argv[3])!="--background-test")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_ALWAYS_RUN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall volcano --no-audio");if(!IsWindowReady())return 5;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1.2f);
 int checks=0,failures=0;std::ofstream log(root/"checks.txt");auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;log<<(ok?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 auto reset=[](){game=Game{};intro={};towerStory={};clearTransientUi();welcome=help=paused=false;mouse={0,0};game.messageTime=0;};
 auto capture=[&](const std::string& name){renderScene();Image im=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&im);check(ExportImage(im,(root/(name+".png")).string().c_str()),"integrated screenshot / "+name);UnloadImage(im);check(IsWindowHidden(),"window remains hidden / "+name);};
 check(volcano_art::terrain.id&&volcano_art::terrain.depth.id==0&&volcano_art::lava.id,"third terrain and Synfig lava are preloaded without unused depth");
 check(volcano_art::lavaStrips.size()>100&&volcano_art::lavaFrames[59].id&&volcano_art::steam.id,"continuous lava ribbons and native steam loop preloaded");
 check(volcano_art::extraScenery.id&&volcano_art::extraSceneryEdge.id&&volcano_art::extraSceneryAlpha.size()==1440*1840,"new boulders, stumps, craters and monoliths preloaded with alpha");
 for(int type=14;type<=17;type++)for(int variant=0;variant<8;variant++){
  Prop p{{10,10},type,1};for(int n=0;n<100&&volcano_art::variant(p)!=variant;n++)p.pos.x+=.25f;
  check(volcano_art::variant(p)==variant,"deterministic scenery variant available");Vector2 at=screen(p.pos);bool hit=false;
  for(int y=0;y<230&&!hit;y+=3)for(int x=0;x<180&&!hit;x+=3)hit=propContains(p,{at.x+x-90,at.y+y-208});
  check(hit&&!propContains(p,{at.x+100,at.y-100}),"variant picking follows actual opaque pixels and rejects empty surroundings");
 }
 RenderTexture2D sheet=LoadRenderTexture(1280,2048);
 for(int type=VolcanoFirstType;type<EnemyTypeCount;type++){
  const auto& atlas=volcano_actors::atlases[type-VolcanoFirstType];check(atlas.art.id&&atlas.silhouette.id&&atlas.alpha.size()==size_t(atlas.art.width)*atlas.art.height,"actual Krita cutouts and picking masks loaded / "+std::to_string(type));
  BeginTextureMode(sheet);ClearBackground(rgb(38,40,50));
  for(int d=0;d<8;d++)for(int action=0;action<4;action++){
   Enemy e{2000,type,{},{},100,100};e.animation.direction=Direction(d);e.animation.phase=.24f;e.animation.moveBlend=action==1?1:0;e.animation.moving=action==1;e.animation.motionInitialized=true;
   if(action==2)e.windup=.12f;float x=160+action*320,y=245+d*256;
   if(action==3)corpseArt(e,x,y);else actor_art::drawEnemy(e,x,y);
   bool hit=false;for(int yy=-30;yy<225&&!hit;yy+=8)for(int xx=-170;xx<=170&&!hit;xx+=8){Vector2 p{x+xx,y-yy};hit=action==3?corpseArt(e,x,y,BLANK,&p):actor_art::drawEnemy(e,x,y,BLANK,&p);}
   check(hit,"actual cutout body/corpse can be selected / "+std::to_string(type)+" / "+std::to_string(d)+" / "+std::to_string(action));
   Vector2 outside{x+220,y+90};check(!(action==3?corpseArt(e,x,y,BLANK,&outside):actor_art::drawEnemy(e,x,y,BLANK,&outside)),"transparent surroundings reject mouse selection");
  }
  EndTextureMode();Image im=LoadImageFromTexture(sheet.texture);ImageFlipVertical(&im);check(ExportImage(im,(root/(std::string(volcano_actors::names[type-VolcanoFirstType])+"-poses.png")).string().c_str()),"eight-direction idle, locomotion, attack and corpse sheet");UnloadImage(im);
 }
 UnloadRenderTexture(sheet);
 reset();game.activateField(WorldLocation::Volcano);game.player={25,48};game.time=0;game.messageTime=0;
 RenderTexture2D river=render_targets::colorOnly(WorldW,H);std::vector<Color> reference;
 for(int n=0;n<2;n++){
  game.time=n*.5f;BeginTextureMode(river);ClearBackground(BLACK);volcano_art::flow(false);EndTextureMode();Image im=LoadImageFromTexture(river.texture);auto* pixels=LoadImageColors(im);
  if(n==0)reference.assign(pixels,pixels+im.width*im.height);else{int changed=0;for(int i=0;i<im.width*im.height;i++)changed+=std::abs(int(pixels[i].r)-reference[i].r)+std::abs(int(pixels[i].g)-reference[i].g)+std::abs(int(pixels[i].b)-reference[i].b)>30;check(changed>1500,"actual isolated river surface visibly flows without actors or ambient particles");}
  UnloadImageColors(pixels);UnloadImage(im);
 }
 UnloadRenderTexture(river);
 fs::create_directories(root/"river-frames");for(int n=0;n<30;n++){game.time=n/15.f;renderScene();Image im=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&im);char filename[32];std::snprintf(filename,sizeof(filename),"frame-%02d.png",n);check(ExportImage(im,(root/"river-frames"/filename).string().c_str()),"actual game river preview frame");UnloadImage(im);}
 for(int type=14;type<=17;type++){
  auto found=std::find_if(game.props.begin(),game.props.end(),[&](const Prop& p){return p.type==type&&volcano_art::variant(p)>=4;});check(found!=game.props.end(),"additional scenery family actually placed in the playable field");
  if(found!=game.props.end()){V at=found->pos;game.player=at+V{-2,0};game.time=1;capture("extra-scenery-"+std::to_string(type));}
 }
 reset();game.enterMageTower();game.player=MagePosition;game.collectedFragments=1;game.dawnQuest=DawnQuest::AtTower;
 check(talkToMage()&&!towerStory.offerVolcano,"actual mage UI does not expose locked third realm");towerStory={};game.desertBossDead=true;
 check(talkToMage()&&towerStory.offerVolcano,"actual mage UI offers third realm after second boss death");
 for(int i=0;i<4000&&towerStory.phase!=TowerPhase::Choice;i++)towerAction(false,false,.05f,true);
 check(towerStory.phase==TowerPhase::Choice&&!game.inVolcano(),"dialogue reaches a choice without automatic teleport");capture("mage-choice");
 Rectangle travel=volcanoTravelButton();mouse={travel.x+travel.width/2,travel.y+travel.height/2};check(clickMageTravelChoice()&&towerStory.travelVolcano,"clicking the actual volcanic button chooses the third realm");
 for(int i=0;i<100;i++)towerAction(false,false,.05f,true);
 check(game.inVolcano()&&distance(game.player,VolcanoArrival)==0&&!towerStory.active(),"fade transition arrives in the real third field");mouse={0,0};capture("arrival");
 for(int id=0;id<2;id++){
  game.player=game.fountains()[id]+V{2,0};game.hp=20;game.mana=8;capture(id==0?"mana-full":"health-full");
  check(game.useFountain(id)&&game.fountainSpent(id),"actual third shrine restores and begins draining");
  for(int i=0;i<25;i++)game.update(.05f,{},false);capture(id==0?"mana-empty":"health-empty");
  check(game.fountainDraining(id)==0&&!game.useFountain(id),"actual third shrine becomes dry and cannot refill");
 }
 for(int id=4;id<6;id++){game.player=game.chests[id].pos+V{2,0};check(game.openChest(id),"actual third chest can be opened");capture("chest-"+std::to_string(id));game.openedChest=-1;}
 game.player=VolcanoBossHome+V{-4,0};auto* boss=game.findEnemy(VolcanoBossId);boss->eruptionCd=0;game.update(.05f,{},false);check(game.fissures.size()==3&&boss->eruptionWindup>0,"real third boss telegraphs three eruption zones");capture("boss-warning");
 for(int i=0;i<20;i++){game.hp=float(game.maxHp());game.update(.05f,{},false);}capture("boss-eruption");
 check(std::any_of(game.fissures.begin(),game.fissures.end(),[](const auto& f){return f.struck;}),"real eruption displays its impact after the warning");
 game.level=30;game.hit(*boss,10000);game.allocationRequired=false;game.player=boss->pos;check(game.openCorpse(VolcanoBossId),"actual third boss corpse opens");capture("boss-fallen");
 check(game.quickTransfer({Place::Corpse,QuestLootSlot,VolcanoBossId})&&game.dawnFragments()==3,"actual corpse transfers third fragment into backpack");startTowerReturn();check(towerStory.returning&&towerStory.fragments==3,"third fragment starts the correct return scene");
 std::vector<double> timings;auto finish=reinterpret_cast<void(*)()>(glfwGetProcAddress("glFinish"));
 const V views[]={VolcanoArrival,{25,48},{39,43},{52,58},{55,28},VolcanoBossHome};
 for(int view=0;view<6;view++){
  reset();game.activateField(WorldLocation::Volcano);game.player=views[view];game.messageTime=0;game.time=2;capture("landscape-"+std::to_string(view));
  for(int frame=0;frame<80;frame++){game.time+=1.f/60;double t=GetTime();renderScene();rlDrawRenderBatchActive();if(finish)finish();if(frame>=20)timings.push_back((GetTime()-t)*1000);}
 }
 check(IsWindowHidden()&&!audioSystem.available&&audioSystem.playedEffects==0,"entire integration run remains hidden and silent");check(PlayerBuild&&!developerActive,"actual player renderer without developer mode");
 std::sort(timings.begin(),timings.end());double mean=std::accumulate(timings.begin(),timings.end(),0.)/timings.size(),p95=timings[(timings.size()-1)*95/100];
 std::ofstream report(root/"summary.json");report<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"species\":7,\"directions\":8,\"pose_views\":224,\"hidden\":true,\"audio_opened\":false,\"presentation_measured\":false,\"completed_render_mean_ms\":"<<mean<<",\"completed_render_p95_ms\":"<<p95<<"}";
 std::cout<<"Volcano integration: "<<checks<<" checks, "<<failures<<" failures; hidden render "<<mean<<" ms\n";audioSystem.shutdown();CloseWindow();return failures?2:0;
}
