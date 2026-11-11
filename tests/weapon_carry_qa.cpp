#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#include <iostream>
#undef main
int checks=0,failures=0;
void check(bool ok,const char* label){checks++;if(!ok){failures++;std::cerr<<"FAIL: "<<label<<'\n';}}
#include "weapon_carry_tests.hpp"
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];fs::create_directories(root/"frames");
 weaponCarryTests();
 SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();InitWindow(W,H,"Emberfall weapon clearance --no-audio");if(!IsWindowReady())return 4;
 audioSystem.init(false,false);makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
 check(hero_paint::atlas.id&&hero_paint::weaponsLoaded,"held weapon materials loaded");
 for(int cell=hero_paint::Forged;cell<=hero_paint::Ivory;cell++)
  check(hero_paint::opaque(hero_paint::uv(cell,.5f,.5f)),"opaque swatch in the combined hero atlas");
 RenderTexture2D sample=LoadRenderTexture(360,390);
 auto save=[&](RenderTexture2D target,const fs::path& file){Image im=LoadImageFromTexture(target.texture);ImageFlipVertical(&im);check(ExportImage(im,file.string().c_str()),"renderer writes the complete QA image");UnloadImage(im);};
 int clipped=0,rendered=0,weapons=0;
 auto draw=[&](Kind kind,int direction,float phase,bool plate=false,float attack=-1,float stow=0,float run=0,
               Kind armor=Kind::None,Kind shield=Kind::Shield,float bowDraw=-1){
  std::array<Item,6> gear{item(kind),{},item(shield),item(plate?Kind::GuardianArmor:armor==Kind::None?Kind::Armor:armor),item(Kind::Boots),{}};
  AnimationState state;state.direction=Direction(direction);state.phase=phase;state.moving=true;state.moveBlend=1;state.motionInitialized=true;
  state.swordStow=stow;state.runBlend=run;
  actor_art::finishShadowFrame();BeginTextureMode(sample);ClearBackground(BLANK);
  actor_art::draw(0,180,310,2.55f,state,0,false,true,kind,false,BLANK,nullptr,-1,attack,bowDraw,20,false,&gear);EndTextureMode();
  Image im=LoadImageFromTexture(sample.texture);Color* px=LoadImageColors(im);
  for(int y=0;y<390;y++)for(int x=0;x<360;x++)if(px[y*360+x].a>180&&(x<2||x>=358||y<2||y>=388))clipped++;
  UnloadImageColors(px);UnloadImage(im);rendered++;
 };
 for(int kind=1;kind<ItemKindCount;kind++)if(gearSlot(Kind(kind))==0){
  weapons++;RenderTexture2D sheet=LoadRenderTexture(720,1560);BeginTextureMode(sheet);ClearBackground(Ink);EndTextureMode();
  for(int dir=0;dir<8;dir++)for(int step=0;step<4;step++){
   draw(Kind(kind),dir,step/4.f);BeginTextureMode(sheet);
   DrawTexturePro(sample.texture,{0,0,360,-390},{float(step)*180,float(dir)*195,180,195},{0,0},0,WHITE);EndTextureMode();
  }
  save(sheet,root/("weapon-"+std::to_string(kind)+".png"));UnloadRenderTexture(sheet);
 }
 for(Kind kind:{Kind::Sword,Kind::Axe,Kind::Dagger,Kind::Bow}){
  draw(kind,1,.25f);
  Image painted=LoadImageFromTexture(sample.texture);
  const bool loaded=hero_paint::weaponsLoaded;
  hero_paint::weaponsLoaded=false;
  draw(kind,1,.25f);
  hero_paint::weaponsLoaded=loaded;
  Image flat=LoadImageFromTexture(sample.texture);
  Color* p=LoadImageColors(painted);Color* q=LoadImageColors(flat);
  int changed=0,alphaChanged=0;
  for(int i=0;i<360*390;i++){
   changed+=p[i].r!=q[i].r||p[i].g!=q[i].g||p[i].b!=q[i].b;
   alphaChanged+=p[i].a!=q[i].a;
  }
  check(changed>100&&alphaChanged==0,"paint changes each weapon type without losing opaque pixels");
  UnloadImageColors(p);UnloadImageColors(q);UnloadImage(painted);UnloadImage(flat);
 }
 constexpr int bowCount=13,bowRows=(bowCount+1)/2;
 RenderTexture2D bows=LoadRenderTexture(8*180,bowRows*195);
 BeginTextureMode(bows);ClearBackground(Ink);EndTextureMode();
 int bowRow=0;
 for(int k=1;k<ItemKindCount;k++)if(archetype(Kind(k))==Kind::Bow){
  int column=0;
  for(float charge:{0.f,.45f,1.f,1.f}){
   draw(Kind(k),column==3?5:1,.25f,false,-1,0,0,Kind::None,Kind::None,charge);
   BeginTextureMode(bows);
   DrawTexturePro(sample.texture,{0,0,360,-390},{(column+4*(bowRow/bowRows))*180.f,(bowRow%bowRows)*195.f,180,195},{0,0},0,WHITE);
   EndTextureMode();column++;
  }
  bowRow++;
 }
 check(bowRow==bowCount,"all bow variants rendered while drawing the string");
 save(bows,root/"bow-draw.png");UnloadRenderTexture(bows);
 for(int frame=0;frame<60;frame++){
  RenderTexture2D movie=LoadRenderTexture(1440,400);BeginTextureMode(movie);ClearBackground(Ink);EndTextureMode();
  int column=0;for(Kind kind:{Kind::Sword,Kind::Axe,Kind::Dagger,Kind::Bow}){
   draw(kind,1,frame/60.f);BeginTextureMode(movie);
   DrawTexturePro(sample.texture,{0,0,360,-390},{column*360.f,0,360,390},{0,0},0,WHITE);EndTextureMode();column++;
  }
  save(movie,root/"frames"/TextFormat("frame-%03d.png",frame));UnloadRenderTexture(movie);
 }
 // A bulky pauldron is checked independently of the starting travel coat.
 RenderTexture2D plate=LoadRenderTexture(1440,780);BeginTextureMode(plate);ClearBackground(Ink);EndTextureMode();
 int index=0;for(int dir:{0,1,2,3})for(float phase:{0.f,.5f}){
  draw(Kind::GuardianSword,dir,phase,true);BeginTextureMode(plate);
  DrawTexturePro(sample.texture,{0,0,360,-390},{float(index%4)*360,float(index/4)*390,360,390},{0,0},0,WHITE);EndTextureMode();index++;
 }
 save(plate,root/"guardian-armour.png");UnloadRenderTexture(plate);
 // All chest items are checked from the front, side and back on the real rig.
 RenderTexture2D armorSheet=LoadRenderTexture(3*235,11*246);
 BeginTextureMode(armorSheet);ClearBackground(Ink);EndTextureMode();
 int armors=0;
 for(int kind=1;kind<FirstAdditionalArmor;kind++)if(gearSlot(Kind(kind))==3){
  int column=0;
  for(int direction:{2,0,6}){
   draw(Kind::Sword,direction,.28f,false,-1,0,0,Kind(kind),Kind::None);
   BeginTextureMode(armorSheet);
   DrawTexturePro(sample.texture,{0,0,360,-390},{column*235.f,armors*246.f,235,246},{0,0},0,WHITE);
   const std::string name=itemName(item(Kind(kind)));
   DrawTextEx(GetFontDefault(),name.c_str(),{column*235.f+6,armors*246.f+226},11,1,LIGHTGRAY);
   EndTextureMode();column++;
  }
  armors++;
 }
 check(armors==11,"all equipped armor and cloak variants render in the close-up");
 save(armorSheet,root/"equipped-armor.png");UnloadRenderTexture(armorSheet);
 RenderTexture2D newArmors=LoadRenderTexture(5*270,2*223);
 BeginTextureMode(newArmors);ClearBackground(Ink);EndTextureMode();
 for(int i=0;i<AdditionalArmorCount;i++){
  const Kind armor=Kind(FirstAdditionalArmor+i);
  const float x=i%5*270.f,y=i/5*223.f;
  check(hero_paint::opaque(hero_paint::uv(hero_paint::NewArmorFirstCell+i,.5f,.5f)),"new armor has a loaded material swatch");
  BeginTextureMode(newArmors);
  check(inventory_icons::draw(item(armor),x+31,y+99,1.13f,1),"new armor renders its inventory icon");
  txt(itemName(item(armor)),x+12,y+195,14,Text);
  EndTextureMode();
  int view=0;
  for(int direction:{2,5}){
   draw(Kind::Sword,direction,.28f,false,-1,0,0,armor,Kind::None);
   BeginTextureMode(newArmors);
   DrawTexturePro(sample.texture,{46,0,252,-390},{x+45+view*104.f,y,128,198},{0,0},0,WHITE);
   EndTextureMode();view++;
  }
 }
 save(newArmors,root/"new-armors.png");UnloadRenderTexture(newArmors);
 RenderTexture2D newWeapons=LoadRenderTexture(5*270,2*230);
 BeginTextureMode(newWeapons);ClearBackground(Ink);EndTextureMode();
 check(hero_paint::realmWeaponsLoaded,"realm weapon materials preload");
 for(int i=0;i<RealmWeaponCount;i++){
  const Kind weapon=Kind(FirstRealmWeapon+i);
  const bool bow=archetype(weapon)==Kind::Bow;
  const float x=i%5*270.f,y=i/5*230.f;
  check(hero_paint::opaque(hero_paint::uv(hero_paint::RealmWeaponFirstCell+i,.5f,.5f)),"realm weapon has an opaque material swatch");
  BeginTextureMode(newWeapons);
  check(inventory_icons::draw(item(weapon),x+31,y+99,1.35f,1),"realm weapon renders its inventory icon");
  txt(itemName(item(weapon)),x+12,y+195,14,Text);
  txt(std::to_string(itemDropChance(weapon))+"%  ·  Atak +"+std::to_string(power(item(weapon))),
      x+12,y+213,12,rarity(item(weapon)));
  EndTextureMode();
  for(int frame=0;frame<2;frame++){
   draw(weapon,1,.28f,false,!bow&&frame?.45f:-1,0,0,Kind::None,Kind::None,bow&&frame?1.f:-1);
   BeginTextureMode(newWeapons);
   DrawTexturePro(sample.texture,{30,0,300,-390},{x+48+frame*105.f,y,123,195},{0,0},0,WHITE);
   EndTextureMode();
  }
 }
 save(newWeapons,root/"new-weapons.png");UnloadRenderTexture(newWeapons);
 RenderTexture2D attacks=LoadRenderTexture(1440,585);BeginTextureMode(attacks);ClearBackground(Ink);EndTextureMode();
 int row=0;for(Kind kind:{Kind::Sword,Kind::Axe,Kind::Dagger}){
  int column=0;for(float p:{0.f,.02f,.06f,.12f,.5f,.85f,.96f,1.f}){
   draw(kind,1,.375f,false,p);BeginTextureMode(attacks);
   DrawTexturePro(sample.texture,{0,0,360,-390},{column*180.f,row*195.f,180,195},{0,0},0,WHITE);EndTextureMode();column++;
  }row++;
 }
 save(attacks,root/"attack-transitions.png");UnloadRenderTexture(attacks);
 for(int stage=0;stage<=10;stage++){
  RenderTexture2D sheet=LoadRenderTexture(720,780);BeginTextureMode(sheet);ClearBackground(Ink);EndTextureMode();
  for(int dir=0;dir<8;dir++){
   draw(Kind::Sword,dir,.1875f,false,-1,stage/10.f,1);BeginTextureMode(sheet);
   DrawTexturePro(sample.texture,{0,0,360,-390},{float(dir%4)*180,float(dir/4)*390,180,195},{0,0},0,WHITE);EndTextureMode();
  }
  save(sheet,root/TextFormat("stow-%02d.png",stage));UnloadRenderTexture(sheet);
 }
 check(clipped==0,"all weapons stay inside the rendered frame");
 std::ofstream report(root/"render-verification.json");report<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"weapons\":"<<weapons<<",\"rendered_poses\":"<<rendered<<",\"clipped_pixels\":"<<clipped<<",\"audio\":false,\"window_hidden\":true}\n";
 UnloadRenderTexture(sample);hero_paint::unload();audioSystem.shutdown();sync_present::shutdown();CloseWindow();
 std::cout<<"Weapon carry: "<<checks-failures<<'/'<<checks<<" checks, "<<weapons<<" weapons, "<<rendered<<" rendered poses\n";return failures?2:0;
}
