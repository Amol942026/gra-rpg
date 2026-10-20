// Focused real GPU checks for attached face paint and grounded Krita shrub.
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#include <iostream>
#undef main
int main(int argc,char** argv){
 if(argc!=2||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::create_directories("qa-face-bush");
 std::ofstream log("qa-face-bush/tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;log<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';log.flush();};
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall face/bush --no-audio");if(!IsWindowReady())return 4;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 check(PlayerBuild&&!audioSystem.available,"player code diagnostics do not open an audio device");
 check(hero_paint::atlas.id&&propArt[2].texture.width==360&&propArt[2].texture.height==460,"native face and 2x Krita bush resources decode");
 check(!forest_bush_art::source.id,"temporary bush source is released after the one-time bake");
 RenderTexture2D sample=LoadRenderTexture(400,360);
 for(int angle=0;angle<16;angle++){
  auto head=[&](bool paint){BeginTextureMode(sample);ClearBackground(BLANK);actor_art::Rig r(200,300,3,Direction::E);r.setFacing(angle*PI/8);r.setSmoothSurfaces();r.rounded({0,0,70},{18,17,21},rgb(213,177,139),paint?0:-1);r.draw();EndTextureMode();return LoadImageFromTexture(sample.texture);};
  Image a=head(false),b=head(true);Color* aa=LoadImageColors(a);Color* bb=LoadImageColors(b);int wrong=0,changed=0;
  for(int i=0;i<a.width*a.height;i++){wrong+=(aa[i].a>128)!=(bb[i].a>128);changed+=std::abs(int(aa[i].r)-bb[i].r)+std::abs(int(aa[i].g)-bb[i].g)+std::abs(int(aa[i].b)-bb[i].b)>30;}
  log<<"Yaw="<<angle<<" alpha mismatch="<<wrong<<" painted pixels="<<changed<<'\n';
  if(wrong){ImageFlipVertical(&a);ImageFlipVertical(&b);ExportImage(a,TextFormat("qa-face-bush/head-bare-%02d.png",angle));ExportImage(b,TextFormat("qa-face-bush/head-painted-%02d.png",angle));}
  check(wrong==0,"paint has exactly the head silhouette at every yaw, including both profiles");
  if(angle>0&&angle<8)check(changed>30,"fitted face paint remains visible on the front hemisphere");
  UnloadImageColors(aa);UnloadImageColors(bb);UnloadImage(a);UnloadImage(b);
 }
 game=Game{};intro={};towerStory={};clearStoryMenus();welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=false;
 game.props.clear();game.enemies.clear();game.forage.clear();game.messageTime=0;game.equipment={};game.equipment[0]=item(Kind::Sword);game.equipment[3]=item(Kind::Armor);
 RenderTexture2D faces=LoadRenderTexture(1600,720);BeginTextureMode(faces);ClearBackground(Ink);EndTextureMode();
 for(int dir=0;dir<8;dir++){
  game.animation={};game.animation.direction=Direction(dir);actor_art::finishShadowFrame();BeginTextureMode(sample);ClearBackground(BLANK);hero(200,318,3,false);EndTextureMode();
  BeginTextureMode(faces);DrawTexturePro(sample.texture,{0,0,400,-360},{float(dir%4)*400,float(dir/4)*360,400,360},{0,0},0,WHITE);EndTextureMode();
 }
 saveTexture(faces,"qa-face-bush/face-profiles.png");UnloadRenderTexture(faces);
 Image shrub=LoadImageFromTexture(propArt[2].texture);ImageFlipVertical(&shrub);Color* leaves=LoadImageColors(shrub);int contact=0,solid=0,border=0;
 for(int y=0;y<shrub.height;y++)for(int x=0;x<shrub.width;x++)if(leaves[y*shrub.width+x].a>=160){solid++;border+=x<2||x>=shrub.width-2||y<2||y>=shrub.height-2;if(x>=168&&x<=192&&y>=413&&y<=421)contact++;}
 check(solid>8000&&border==0,"shrub has an unclipped leafy silhouette");check(contact>40,"opaque stems and litter meet the exact root/ground anchor");
 ExportImage(shrub,"qa-face-bush/bush-source.png");UnloadImageColors(leaves);UnloadImage(shrub);
 RenderTexture2D sizes=LoadRenderTexture(1000,350);BeginTextureMode(sizes);ClearBackground(Ink);
 for(int i=0;i<3;i++){float s=.75f+i*.5f,x=165+i*330.f,y=285;ellipse(x,y,80*s,24*s,rgb(63,82,49));DrawTexturePro(propArt[2].texture,{0,0,360,-460},{x-90*s,y-208*s,180*s,230*s},{0,0},0,WHITE);}
 EndTextureMode();saveTexture(sizes,"qa-face-bush/bush-scales.png");UnloadRenderTexture(sizes);
 game.player={15,15};
 for(int i=0;i<3;i++){Prop p{game.player+V{-1.f+i*1.4f,-1.8f+i*.3f},2,.8f+i*.2f};Vector2 base=screen(p.pos);game.props.push_back(p);
  check(propContains(p,{base.x,base.y-18*p.size}),"leafy bush is selectable at each game scale");check(!propContains(p,{base.x,base.y-90*p.size}),"transparent space over the bush is not selectable");check(!propContains(p,{base.x-29*p.size,base.y+3*p.size}),"contact shadow does not become a solid hover target");
 }
 renderScene();check(world_shadows::ready,"new shrub integrates with real scene sorting and ground shadows");saveTexture(canvas,"qa-face-bush/bush-forest.png");
 UnloadRenderTexture(sample);hero_paint::unload();audioSystem.shutdown();sync_present::shutdown();CloseWindow();log<<checks-failures<<"/"<<checks<<" passed\n";std::cout<<"Face/bush: "<<checks-failures<<"/"<<checks<<" passed\n";return failures?2:0;
}
