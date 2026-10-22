#pragma once
// Runs inside the same player-edition renderer as menu_qa.cpp, with its own
// save folder. No playback, OS input automation or production saves.
template<class Check>void forestMenuChecks(const fs::path& output,Check check){
 check(menu_scene::ready(),"four original Krita menu layers loaded");
 size_t bytes=0;for(auto t:menu_scene::layers){check(t.width==1440&&t.height==900,"menu texture dimensions within 2048 budget");bytes+=size_t(t.width)*t.height*4;}
 check(bytes==20736000,"background uses 19.8 MiB RGBA8 with no frame atlas");
 auto save=[&](RenderTexture2D target,const fs::path& path){Image im=LoadImageFromTexture(target.texture);ImageFlipVertical(&im);check(ExportImage(im,path.string().c_str()),"frame exported: "+path.filename().string());UnloadImage(im);};
 fs::create_directories(output/"animation");
 intro={};towerStory={};game=Game{};clearTransientUi();welcome=true;paused=false;menuPageTo(MenuPage::Home);mouse={0,0};prepareCanvas(.8f);
 std::array<unsigned,4> ids{};for(int i=0;i<4;i++)ids[i]=menu_scene::layers[i].id;
 // Warm once, then ensure the actual loop allocates no fonts or textures.
 menuClock=0;renderScene();size_t fonts=typography.atlasCount();auto initial=saves::encode({game,intro,towerStory,{}});
 for(int frame=0;frame<60;frame++){menuClock=frame*.2f;renderScene();save(canvas,output/"animation"/TextFormat("frame-%02d.png",frame));}
 menuClock=12;renderScene();save(canvas,output/"animation/loop-end.png");
 bool stable=true;for(int i=0;i<4;i++)stable&=ids[i]==menu_scene::layers[i].id;
 check(stable&&fonts==typography.atlasCount(),"animated menu reuses all texture and font allocations");
 check(saves::encode({game,intro,towerStory,{}})==initial,"background animation cannot advance gameplay or prologue");
 check(!native_world::drawingWorld,"animated menu restores blend state");menuClock=0;prepareCanvas(1);
 // A real v1 save with the original 64 plants, including a collected plant.
 auto legacy=saves::Snapshot{game,{}, {},{}};legacy.game.forage[3].value={};legacy.game.bag[20]=item(Kind::Mushroom,0,17);
 for(int id=49;id<=64;id++)legacy.game.forage.push_back({id,item(Kind::Mushroom),{30,30}});
 menuStore().save(1,legacy);menuSlot=1;menuLoad();
 check(game.forage.size()==48&&game.forage[3].value.empty()&&game.bag[20].count==17,"actual load migrates legacy mushrooms without respawn or inventory loss");
 // Moving actor over a stationary grid. Real simulation, real pixels and
 // world-space anchor motion make foot sliding observable in the sequence.
 fs::create_directories(output/"treant");intro={};towerStory={};welcome=paused=false;game=Game{};game.props.clear();game.enemies.clear();game.enemies.push_back({71,7,{20,20},{20,20},170,170});
 RenderTexture2D target=LoadRenderTexture(900,600);V start=game.enemies[0].pos;
 for(int frame=0;frame<120;frame++){
  game.player=game.enemies[0].pos+normalized(V{1,-1})*5;game.update(1.f/60,{},false);auto& e=game.enemies[0];
  float x=230+(e.pos.x-e.pos.y-start.x+start.y)*40*2.2f;
  actor_art::finishShadowFrame();BeginTextureMode(target);ClearBackground(rgb(18,33,37));for(int i=0;i<19;i++)line(i*50,0,i*50,600,rgb(31,49,49));line(0,440,900,440,rgb(113,124,93),2);
  actor_art::draw(8,x,440,2.2f,e.animation);EndTextureMode();save(target,output/"treant"/TextFormat("frame-%03d.png",frame));
 }
 check(distance(start,game.enemies[0].pos)>2.8f,"treant sequence uses actual chase speed");UnloadRenderTexture(target);
 check(!audioSystem.available&&audioSystem.playedEffects==0,"all forest and background checks remain silent");
}
