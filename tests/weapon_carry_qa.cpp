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
 check(power_fx::atlas.id!=0,"the five charged effects are preloaded");
 for(int cell=hero_paint::Forged;cell<=hero_paint::Ivory;cell++)
  check(hero_paint::opaque(hero_paint::uv(cell,.5f,.5f)),"opaque swatch in the combined hero atlas");
 RenderTexture2D sample=LoadRenderTexture(360,390);
 auto save=[&](RenderTexture2D target,const fs::path& file){Image im=LoadImageFromTexture(target.texture);ImageFlipVertical(&im);check(ExportImage(im,file.string().c_str()),"renderer writes the complete QA image");UnloadImage(im);};
 int clipped=0,rendered=0,weapons=0;
 auto draw=[&](Kind kind,int direction,float phase,bool plate=false,float attack=-1,float stow=0,float run=0,
               Kind armor=Kind::None,Kind shield=Kind::Shield,float bowDraw=-1,float windup=-1){
  std::array<Item,6> gear{item(kind),{},item(shield),item(plate?Kind::GuardianArmor:armor==Kind::None?Kind::Armor:armor),item(Kind::Boots),{}};
  AnimationState state;state.direction=Direction(direction);state.phase=phase;state.moving=true;state.moveBlend=1;state.motionInitialized=true;
  state.swordStow=stow;state.runBlend=run;
  actor_art::finishShadowFrame();BeginTextureMode(sample);ClearBackground(BLANK);
  actor_art::draw(0,180,windup>=0?337.f:310.f,2.55f,state,0,false,true,kind,false,BLANK,nullptr,-1,attack,bowDraw,20,false,&gear,
                  -1,-1,0,-1,PowerStyle::Count,0,windup);EndTextureMode();
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
 for(Kind kind:{Kind::Sword,Kind::Axe,Kind::Dagger,Kind::Spear,Kind::Bow}){
  game=Game{};game.player={27,26};game.enemies.clear();game.props.clear();game.forage.clear();
  clearTransientUi();invalidatePausedWorld();intro={};towerStory={};welcome=paused=false;
  game.equipment[0]=item(kind);game.messageTime=0;
  const Vector2 target=screen(game.player+V{2,-2});
  mouse={target.x,target.y-34};leftClickGameplay({});
  const auto style=powerStyle(kind);
  int previousFrame=0;
  Color emptyRight=BLANK;
  for(int frame:{0,8,16,35}){
   for(int f=previousFrame;f<frame;f++)game.update(1.f/60,{},false);
   previousFrame=frame;
   check(std::abs(game.powerWindupProgress()-std::min(1.f,frame/(60*PowerHoldTime)))<.001f,
         "world charge meter follows the held attack clock");
   check(game.swing==0&&game.arrows.empty(),"the charge pose does not attack before release");
   renderScene();save(canvas,root/TextFormat("charge-%d-%02d.png",int(style),frame));
   Image meter=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&meter);
   const Rectangle bar=power_fx::chargeBarRect(style,screen(game.player));
   const int x=int(bar.x),y=int(bar.y)+3;
   const Color left=GetImageColor(meter,x+7,y),right=GetImageColor(meter,x+46,y);
   if(frame==0)emptyRight=right;
   check((ColorToInt(left)==ColorToInt(emptyRight))==(frame==0),"charge fill starts at the left edge");
   check((ColorToInt(right)==ColorToInt(emptyRight))==(frame!=35),"charge fill reaches the right edge at full charge");
   UnloadImage(meter);
  }
  check(game.powerHoldFraction()==1&&releaseWorldAttack()&&game.activePowerVisual()==style,
        "actual mouse handlers commit the charged weapon on release");
  check(game.powerWindupProgress()<0,"releasing hides the world charge meter");
  const auto& profile=powerAttack(style);
  const float rate=powerAttackRate(style);
  const int effectFrame=int(profile.contact*60/rate)+4;
  for(int frame=0;frame<effectFrame;frame++){
   if(rate>1){
    renderScene();
    Image release=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&release);
    ImageCrop(&release,{450,283,230,182});
    const auto file=root/TextFormat("fast-release-%d-%02d.png",int(style),frame);
    check(ExportImage(release,file.string().c_str()),"renderer writes the fast release frame");
    UnloadImage(release);
   }
   game.update(1.f/60,{},false);
  }
  renderScene();save(canvas,root/TextFormat("power-%d.png",int(style)));
  Image painted=LoadImageFromTexture(canvas.texture);
  Texture2D atlas=power_fx::atlas;power_fx::atlas={};
  renderScene();power_fx::atlas=atlas;
  Image noAtlas=LoadImageFromTexture(canvas.texture);
  Color* a=LoadImageColors(painted);Color* b=LoadImageColors(noAtlas);
  int changed=0;
  for(int j=0;j<painted.width*painted.height;j++)changed+=a[j].r!=b[j].r||a[j].g!=b[j].g||a[j].b!=b[j].b;
  check(changed>120,"charged weapon effect changes pixels in the normal scene renderer");
  UnloadImageColors(a);UnloadImageColors(b);UnloadImage(painted);UnloadImage(noAtlas);
 }
 for(Kind kind:{Kind::Sword,Kind::Axe}){
  auto legs=[&](float phase){
   draw(kind,1,phase,false,-1,0,0,Kind::None,Kind::Shield,-1,1);
   Image image=LoadImageFromTexture(sample.texture);ImageFlipVertical(&image);
   ImageCrop(&image,{115,270,130,86});
   return image;
  };
  Image first=legs(.17f),second=legs(.65f);
  int changed=0;
  for(int y=0;y<first.height;y++)for(int x=0;x<first.width;x++)
   changed+=GetImageColor(first,x,y).a!=GetImageColor(second,x,y).a;
  check(changed>180,"fully charged weapons still render a changing leg silhouette during walking");
  UnloadImage(first);UnloadImage(second);
  game=Game{};game.player={27,26};game.enemies.clear();game.props.clear();game.forage.clear();
  game.equipment[0]=item(kind);game.animation.swordStow=kind==Kind::Sword?1.f:0.f;game.messageTime=0;
  mouse=screen(game.player+V{2,-2});mouse.y-=34;leftClickGameplay({});
  for(int frame=0;frame<=54;frame++){
   game.update(1.f/60,{1,0},false);
   if(frame==11||frame==23||frame==34||frame==43||frame==54){
    renderScene();save(canvas,root/TextFormat("overhead-walk-%d-%02d.png",int(powerStyle(kind)),frame));
   }
  }
  check(game.animation.moving&&game.animation.swordStow==0&&game.powerWindupProgress()==1,
        "a fully charged walking hero has drawn the weapon and keeps moving");
 }
 game=Game{};game.props.clear();game.enemies.clear();game.player={27,26};mouse=screen(game.player+V{2,-2});mouse.y-=34;
 leftClickGameplay({});mouse={1270,455};
 check(!releaseWorldAttack()&&game.powerWindupProgress()<0&&game.swing==0,"releasing into the HUD discards a pending world click and hides the meter");
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
