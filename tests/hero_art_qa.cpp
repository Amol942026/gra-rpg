// Actual player renderer, equipment and scene integration. Never opens audio.
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#include <iostream>
#undef main
int main(int argc,char** argv){
 if(argc!=2||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::create_directories("qa-hero-art/frames");
 std::ofstream report("qa-hero-art/render-tests.txt");int checks=0,failed=0;
 auto check=[&](bool ok,const char* name){checks++;failed+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<name<<'\n';report.flush();};
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall hero art --no-audio");
 if(!IsWindowReady())return 4;audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 check(PlayerBuild&&!audioSystem.available,"real player renderer uses no audio device");
 check(hero_paint::atlas.id&&hero_paint::atlas.width==512&&hero_paint::atlas.height==384,"Krita atlas decodes and is preloaded once");
 game=Game{};intro={};towerStory={};clearStoryMenus();welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=false;
 game.props.clear();game.enemies.clear();game.forage.clear();game.messageTime=0;
 const std::array<Item,6> outfit={item(Kind::Sword),{},item(Kind::Shield),item(Kind::WolfCloak),item(Kind::Boots),{}};
 game.equipment=outfit;
 RenderTexture2D sample=LoadRenderTexture(400,420),sheet=LoadRenderTexture(1600,840);
 BeginTextureMode(sheet);ClearBackground(Ink);EndTextureMode();
 auto draw=[&](float phase,int mode,Direction direction){
  game.animation={};game.animation.direction=direction;game.animation.phase=phase;game.animation.idleTime=phase*2;
  game.animation.moving=mode==1||mode==2||mode==5;game.animation.runBlend=mode==2?1:0;
  game.swing=0;game.bowDrawing=false;game.bowReleaseTime=0;game.rollRemaining=0;game.equipment=outfit;
  game.swingStrong=false;
  if(mode==3||mode==4){game.meleeDuration=mode==4?StrongCutDuration:SwingDuration;game.swing=(1-phase)*game.meleeDuration;game.swingStrong=mode==4;}
  if(mode==5){game.equipment[0]=item(Kind::Bow);game.bowDrawing=true;game.bowDrawTime=BowDrawDuration*phase;}
  if(mode==6)game.rollRemaining=(1-phase)*RollDuration;
  actor_art::finishShadowFrame();BeginTextureMode(sample);ClearBackground(BLANK);hero(200,320,2.7f,false);EndTextureMode();
 };
 for(int dir=0;dir<8;dir++){
  draw(.1f,0,Direction(dir));BeginTextureMode(sheet);DrawTexturePro(sample.texture,{0,0,400,-420},{float(dir%4)*400,float(dir/4)*420,400,420},{0,0},0,WHITE);EndTextureMode();
 }
 saveTexture(sheet,"qa-hero-art/turntable.png");UnloadRenderTexture(sheet);
 draw(0,0,Direction::S);Image face=LoadImageFromTexture(sample.texture);ImageFlipVertical(&face);Color* facePixels=LoadImageColors(face);int faceSolid=0,faceWhite=0;
 for(int y=87;y<170;y++)for(int x=168;x<232;x++){Color c=facePixels[y*400+x];if(c.a>200){faceSolid++;faceWhite+=c.r>240&&c.g>240&&c.b>240;}}
 check(faceSolid>500&&faceWhite<faceSolid/100,"paint survives rlgl batch flushes without an opaque white face rectangle");UnloadImageColors(facePixels);UnloadImage(face);
 const char* modes[]={"idle","walk","run","slash","heavy","bow","roll"};
 RenderTexture2D contact=LoadRenderTexture(1680,2240);BeginTextureMode(contact);ClearBackground(Ink);EndTextureMode();
 for(int mode=0;mode<7;mode++)for(int frame=0;frame<60;frame++){
  float phase=frame/60.f;draw(phase,mode,Direction::SE);
  Image image=LoadImageFromTexture(sample.texture);ImageFlipVertical(&image);Color* pixels=LoadImageColors(image);int solid=0,border=0;
  for(int y=0;y<image.height;y++)for(int x=0;x<image.width;x++)if(pixels[y*image.width+x].a>32){solid++;if(x<3||x>=image.width-3||y<3||y>=image.height-3)border++;}
  check(solid>1600&&border==0,"full painted hero and weapon remain inside the fixed frame");
  UnloadImageColors(pixels);UnloadImage(image);
  saveTexture(sample,(std::string("qa-hero-art/frames/")+modes[mode]+"-"+(frame<10?"0":"")+std::to_string(frame)+".png").c_str());
  if(frame%10==0){BeginTextureMode(contact);DrawTexturePro(sample.texture,{0,0,400,-420},{float(frame/10)*280,float(mode)*320,280,294},{0,0},0,WHITE);center(modes[mode],frame/10*280+140,mode*320+300,16,Gold);EndTextureMode();}
 }
 saveTexture(contact,"qa-hero-art/motion-contact.png");UnloadRenderTexture(contact);
 draw(.23f,1,Direction::SE);Image painted=LoadImageFromTexture(sample.texture);
 unsigned id=hero_paint::atlas.id;hero_paint::atlas.id=0;draw(.23f,1,Direction::SE);Image plain=LoadImageFromTexture(sample.texture);hero_paint::atlas.id=id;
 Color* a=LoadImageColors(painted);Color* b=LoadImageColors(plain);int changed=0;
 for(int i=0;i<painted.width*painted.height;i++)if(std::abs(int(a[i].r)-b[i].r)+std::abs(int(a[i].g)-b[i].g)+std::abs(int(a[i].b)-b[i].b)>35)changed++;
 report<<"Krita paint changes "<<changed<<" actual rendered pixels\n";check(changed>300,"native Krita face hair and fabric are used by the real renderer");
 UnloadImageColors(a);UnloadImageColors(b);UnloadImage(painted);UnloadImage(plain);
 game=Game{};game.equipment=outfit;game.player={8,19};game.messageTime=0;renderScene();saveTexture(canvas,"qa-hero-art/forest.png");
 game.activateField(WorldLocation::Desert);game.player=DesertArrival;renderScene();saveTexture(canvas,"qa-hero-art/desert.png");
 UnloadRenderTexture(sample);report<<checks-failed<<"/"<<checks<<" passed\n";report.close();
 hero_paint::unload();audioSystem.shutdown();sync_present::shutdown();CloseWindow();std::cout<<"Hero renderer: "<<checks-failed<<"/"<<checks<<" passed\n";return failed?2:0;
}
