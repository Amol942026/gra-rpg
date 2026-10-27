// Exercise actual player rendering, wind, transparent picking and shadows.
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#include <iostream>
#include <set>
#undef main
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];fs::create_directories(root/"trees");
 std::ofstream log(root/"checks.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const std::string& label){checks++;failures+=!ok;log<<(ok?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall forest trees --no-audio");if(!IsWindowReady())return 4;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 auto save=[&](RenderTexture2D texture,fs::path path){Image im=LoadImageFromTexture(texture.texture);ImageFlipVertical(&im);check(ExportImage(im,path.string().c_str()),"PNG written: "+path.filename().string());UnloadImage(im);};
 check(PlayerBuild&&!audioSystem.available,"real player code opens no audio device");
 for(int i=0;i<3;i++)check(forest_tree_art::baked[i]&&!forest_tree_art::sources[i].id,"Krita source baked once and released / "+std::to_string(i));
 game=Game{};game.player={20,20};game.props.clear();game.enemies.clear();game.forage.clear();game.messageTime=0;
 intro={};towerStory={};clearStoryMenus();welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=false;dragging={};mouse={0,0};
 RenderTexture2D sample=LoadRenderTexture(W,H),sheet=LoadRenderTexture(1440,900);
 BeginTextureMode(sheet);ClearBackground(rgb(22,37,33));txt("LAS MCHU / NOWE DRZEWA",44,25,30,Gold);
 const char* names[]={"Świerk","Brzoza","Dąb"};
 for(int i=0;i<3;i++){center(names[i],240+i*480.f,91,23,Text);float x=240+i*480.f,y=785,s=3.f;
  ellipse(x,y,179,31,rgb(48,65,41));DrawTexturePro(propArt[i==0?0:i==1?4:5].texture,{0,0,360,-460},{x-90*s,y-208*s,180*s,230*s},{0,0},0,WHITE);
 }
 EndTextureMode();save(sheet,root/"tree-lineup.png");UnloadRenderTexture(sheet);
 int slot=0;
 for(int type:{0,4,5}){
  auto texture=propArt[type].texture;check(texture.width==360&&texture.height==460,"2x detail cache / type "+std::to_string(type));
  Image source=LoadImageFromTexture(texture);ImageFlipVertical(&source);Color* sourcePixels=LoadImageColors(source);int opaque=0,border=0,contact=0,colors=0;
  std::set<unsigned> palette;
  for(int y=0;y<460;y++)for(int x=0;x<360;x++){
   Color color=sourcePixels[y*360+x];if(color.a>230){opaque++;border+=x<2||x>=358||y<2||y>=458;
    contact+=x>=172&&x<=188&&y>=410&&y<=420;palette.insert((color.r<<16)|(color.g<<8)|color.b);}
  }
  colors=int(palette.size());check(opaque>11000&&border==0,"unclipped detailed silhouette / type "+std::to_string(type));
  check(contact>65,"opaque roots meet the world anchor / type "+std::to_string(type));
  check(colors>150,"bark and leaf tonal detail / type "+std::to_string(type));
  check(ExportImage(source,(root/"trees"/(std::to_string(type)+"-cache.png")).string().c_str()),"cache inspection export");UnloadImageColors(sourcePixels);UnloadImage(source);
  for(float size:{.75f,1.f,1.55f}){
   std::vector<Color> previous;
   for(float time:{0.f,.3f,1.1f,2.4f}){
    Prop prop{game.player,type,size};game.time=time;game.props={prop};Vector2 base=screen(prop.pos);
    BeginTextureMode(sample);ClearBackground(BLANK);drawPropTexture(prop,texture,false,WHITE);EndTextureMode();
    Image image=LoadImageFromTexture(sample.texture);ImageFlipVertical(&image);Color* pixels=LoadImageColors(image);
    int solid=0,compared=0,mismatch=0,changed=0;
    for(int y=std::max(3,int(base.y-205*size));y<std::min(H-3,int(base.y+14*size));y+=3)
     for(int x=std::max(3,int(base.x-88*size));x<std::min(W-3,int(base.x+88*size));x+=3){
      bool inside=true,outside=true;for(int dy:{-1,0,1})for(int dx:{-1,0,1}){int a=pixels[(y+dy)*W+x+dx].a;inside&=a>230;outside&=a<5;}
      if(!inside&&!outside)continue;compared++;solid+=inside;
      mismatch+=propContains(prop,{x+.5f,y+.5f})!=inside;
     }
    check(solid>350&&compared>1000&&mismatch<=2,"wind GPU silhouette agrees with pointer / type "+std::to_string(type)+" mismatch="+std::to_string(mismatch));
    check(treeShift(prop,208,time)==0&&propContains(prop,{base.x,base.y}),"wind never detaches roots from ground");
    check(!propContains(prop,{base.x-86*size,base.y+16*size}),"transparent corners cannot be selected");
    if(!previous.empty()){for(int i=0;i<W*H;i++)changed+=pixels[i].a!=previous[i].a;check(changed>150,"crown visibly sways between actual frames");}
    previous.assign(pixels,pixels+W*H);UnloadImageColors(pixels);UnloadImage(image);
    save(sample,root/"trees"/(std::to_string(slot++)+"-wind.png"));
   }
  }
 }
 UnloadRenderTexture(sample);
 const V regions[]={V{8,19},V{14,43},V{14,61},V{59,57}};
 for(int i=0;i<4;i++){
  game=Game{};game.player=regions[i];game.time=1.1f;game.messageTime=0;mouse={0,0};renderScene();
  check(world_shadows::ready,"native trees use real terrain shadows / region "+std::to_string(i));save(canvas,root/("forest-"+std::to_string(i)+".png"));
 }
 // Keep an unobstructed comparison against the real fixed-camera forest view.
 for(int type:{0,4,5}){
  game=Game{};game.player={15,15};game.time=.9f;game.enemies.clear();game.forage.clear();game.props={{{16,16},type,1.15f}};game.messageTime=0;
  Vector2 base=screen(game.props[0].pos);bool hit=false;
  for(int y=90;y<160&&!hit;y+=3)for(int x=-35;x<35&&!hit;x+=3){mouse={base.x+x,base.y-y};auto h=pickWorld(mouse);hit=h.kind==HoverKind::Prop;}
  check(hit,"tree occlusion and hover outline work in foreground / type "+std::to_string(type));renderScene();save(canvas,root/("hover-"+std::to_string(type)+".png"));
 }
 check(!audioSystem.available,"all graphical diagnostics stayed silent");
 log<<"Checks: "<<checks<<"\nFailures: "<<failures<<'\n';std::cout<<"Forest trees: "<<checks<<" checks, "<<failures<<" failures\n";
 CloseWindow();return failures?2:0;
}
