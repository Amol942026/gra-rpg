#pragma once
#include "sprint_qa.hpp"
#include "fallen_qa.hpp"

template<class Check>
void verifyJackalTurn(Check check,std::ostream& report){
 namespace jackal=volcano_actors::jackal_blender;
 const bool ready=jackal::ready()&&!volcano_actors::jackal_rig::ready();
 check(ready,"jackal uses baked Blender frames in the game renderer");
 if(!ready)return;
 Enemy e{135,DuneJackalType,{20,20},{20,20},100,100};
 auto& a=e.animation;
 a.facingInitialized=a.motionInitialized=true;
 bool facings=true,walk=true,padded=true,feet=true,actions=true;
 int sampled=0;
 for(int i=0;i<jackal::Facings;i++){
  const float yaw=i*2*PI/jackal::Facings;
  a.facingAngle=std::atan2(std::sin(yaw)*ActorGroundSquash,std::cos(yaw));
  a.moveBlend=0;
  const auto standing=jackal::select(e,-1);
  facings&=standing.facing==i&&standing.index==jackal::Walk;
  a.moveBlend=1;
  for(int p=0;p<jackal::Walk;p++){
   a.phase=float(p)/jackal::Walk;
   const auto frame=jackal::select(e,-1);
   const auto& page=jackal::pages[frame.facing/jackal::PerPage];
   const auto rect=jackal::tile(frame);
   facings&=frame.facing==i&&frame.index==p;
   int top=jackal::Height,bottom=-1,filled=0;
   for(int y=0;y<jackal::Height;y++)for(int x=0;x<jackal::Width;x++){
    const auto alpha=page.alpha[(int(rect.y)+y)*jackal::SheetWidth+int(rect.x)+x];
    if(alpha<128)continue;
    padded&=x>0&&x<jackal::Width-1&&y>0&&y<jackal::Height-1;
    top=std::min(top,y);bottom=std::max(bottom,y);filled++;
   }
   walk&=filled>=650;
   feet&=top<47&&bottom>=jackal::AnchorY-8&&bottom<jackal::AnchorY+22;
   ++sampled;
  }
  e.windup=.25f*meleeWindup(e.type);
  const auto bite=jackal::select(e,-1);
  e.windup=0;
  actions&=bite.facing==i&&bite.index>jackal::Walk&&bite.index<=jackal::Walk+jackal::Attack;
  e.deathMotion=a;
  for(int side:{-1,1}){
   e.deathDirection={float(side),float(-side)};
   const auto down=jackal::select(e,1);
   const int first=jackal::Walk+1+jackal::Attack+(side>0?jackal::Fall:0);
   actions&=down.facing==i&&down.index==first+jackal::Fall-1;
  }
 }
 a.moveBlend=1;a.phase=1;
 check(facings,"jackal holds the walk phase while using all baked headings");
 check(jackal::select(e,-1).index==0,"jackal walk wraps back to the first frame");
 check(walk,"all jackal walking frames contain a full painted silhouette");
 check(padded,"jackal silhouette stays inside the padded atlas cells");
 check(feet,"jackal walk stays registered to its common ground anchor");
 check(actions,"jackal bite and both falls select baked Blender frames");
 report<<"Jackal sampled walking frames: "<<sampled<<'\n';
}

bool runGraphicsQA(){
 fs::create_directories("qa-v21");std::ofstream report("qa-v21/graphics-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 check(!audioSystem.available,"graphics diagnostic opens no audio device");
 verifySprintArt(check);
 verifyFallenArt(check);
 verifyJackalTurn(check,report);
 {
  const double started=GetTime();
  int count=0;
  bool intact=true;
  const auto texture=inventory_icons::weaponAtlas.texture;
  Image atlas=LoadImageFromTexture(texture);
  ImageFlipVertical(&atlas);
  const int tile=inventory_icons::WeaponCell,columns=inventory_icons::WeaponColumns;
  std::array<std::vector<std::vector<unsigned char>>,ItemKindCount> masks;
  for(int i=1;i<ItemKindCount;i++){
   const Kind kind=Kind(i),shape=archetype(kind);
   const int slot=inventory_icons::weaponCell(kind);
   if(slot<0)continue;
   count++;
   if(shape==Kind::Axe)intact&=actor_art::hero_weapon_art::axeFaces[i].count==8;
   if(shape==Kind::Spear)intact&=actor_art::hero_weapon_art::spearFaces[i-FirstSpear].count==5;
   if(shape==Kind::Sword||shape==Kind::Dagger)intact&=actor_art::hero_weapon_art::bladeFaces[i].count==7;
   const int x=slot%columns*tile,y=slot/columns*tile;
   std::vector<unsigned char> mask;
   int visible=0,border=0;
   for(int v=0;v<tile;v++)for(int u=0;u<tile;u++){
    const bool solid=GetImageColor(atlas,x+u,y+v).a>140;
    visible+=solid;border+=solid&&(u<2||v<2||u>=tile-2||v>=tile-2);
    mask.push_back(solid);
   }
   intact&=visible>160&&border==0;
   auto& seen=masks[int(shape)];
   intact&=std::find(seen.begin(),seen.end(),mask)==seen.end();
   seen.push_back(std::move(mask));
  }
  report<<"Shared weapon silhouettes: "<<count<<'\n';
  check(texture.id&&count==47&&intact,"every weapon uses its distinct, complete shared silhouette in the inventory and on the hero");
  ExportImage(atlas,"qa-v21/weapon-icons.png");UnloadImage(atlas);
  report<<"Weapon icon render and export: "<<GetTime()-started<<" s\n";
 }
 {
  const double started=GetTime();
  RenderTexture2D sheet=LoadRenderTexture(1120,584);
  BeginTextureMode(sheet);ClearBackground(Ink);
  const std::array<Kind,3> weapons={Kind::Dagger,Kind::Axe,Kind::Spear};
  const char* labels[]={"SZTYLET","TOPÓR","WŁÓCZNIA"};
  for(int row=0;row<3;row++){
   center(labels[row],63,103+row*185.f,15,Gold);
   for(int pose=0;pose<5;pose++)for(int angle=0;angle<2;angle++){
    AnimationState state{};state.direction=angle?Direction::N:Direction::S;
    state.moving=state.motionInitialized=true;state.phase=.16f;state.moveBlend=1;
    state.swordStow=state.runBlend=pose*.25f;
    const float x=176+pose*187.f+angle*83,y=147+row*185.f;
    actor_art::draw(0,x,y,1.22f,state,0,false,angle!=0,weapons[row]);
   }
  }
  EndTextureMode();saveTexture(sheet,"qa-v21/weapon-stow.png");UnloadRenderTexture(sheet);
  sheet=LoadRenderTexture(1400,650);
  BeginTextureMode(sheet);ClearBackground(Ink);
  const std::array<Kind,6> fittings={Kind::Axe,Kind::OathAxe,Kind::BasaltCleaver,Kind::Spear,Kind::BriarSpear,Kind::DawnSpear};
  for(int i=0;i<int(fittings.size());i++){
   const Kind kind=fittings[i];
   const float x=(i%3)*462.f,y=(i/3)*320.f;
   inventory_icons::draw(item(kind),x+63,y+155,2.f,1);
   for(int view=0;view<2;view++){
    AnimationState state{};
    state.moving=state.motionInitialized=true;state.phase=.27f;
    state.direction=view?Direction::E:Direction::N;
    state.swordStow=state.runBlend=state.moveBlend=1;
    actor_art::draw(0,x+181+view*148,y+271,2.65f,state,0,false,false,kind);
   }
   center(itemName(item(kind)),x+230,y+291,15,Gold);
  }
  EndTextureMode();saveTexture(sheet,"qa-v21/weapon-back-fit.png");UnloadRenderTexture(sheet);
  report<<"Weapon pose render and export: "<<GetTime()-started<<" s\n";
  for(Kind kind:weapons){
   bool contact=true,stored=true,feet=true,flat=true;
   for(int gait=0;gait<12;gait++)for(int frame=0;frame<=36;frame++){
    const float phase=frame/36.f;
    auto natural=hero_motion::evaluate(gait/12.f,1,phase,1);
    auto pose=natural;
    float slide=0;
    if(kind==Kind::Spear)slide=spear_motion::apply(pose,kind,-1,1,0);
    else hero_weapon_motion::carry(pose,kind);
    auto mount=weapon_stow::apply(pose,natural,kind,phase,slide);
    using namespace hero_motion;
    if(phase<.5f)contact&=length(mount.grip-pose.world[data::HandR].p)<.02f;
    if(phase>=.66f){
     const int anchor=kind==Kind::Dagger?data::Pelvis:data::Spine;
     stored&=length(mount.grip-pose.point(anchor,weapon_stow::socket(kind)))<.02f;
     stored&=length(mount.axis-pose.vector(anchor,weapon_stow::storedAxis(kind)))<.02f;
     if(kind==Kind::Dagger)stored&=mount.daggerVisible<.125f;
     else{
      const Vec side=pose.vector(data::Spine,{1,0,0});
      const Vec forward=pose.vector(data::Spine,{0,1,0});
      const auto dot=[](Vec a,Vec b){return a.x*b.x+a.y*b.y+a.z*b.z;};
      flat&=std::abs(dot(mount.axis,forward))<.015f&&std::abs(dot(mount.across,forward))<.015f;
      if(kind==Kind::Axe)flat&=dot(mount.across,side)<-.98f;
     }
    }
    for(int foot:{data::FootL,data::FootR})feet&=length(pose.world[foot].p-natural.world[foot].p)<.01f;
   }
   check(contact&&stored&&feet,"weapon follows the grip and then its mount while gait keeps both feet independent");
   if(kind!=Kind::Dagger)check(flat,"back-stowed weapon follows the plane of the torso and the axe blade points left from the rear");
   Game test;test.enemies.clear();test.props.clear();test.equipment[0]=item(kind);test.player={20,20};
   for(int f=0;f<44;f++)test.update(1.f/60,{1,0},false,true);
   const bool stowed=test.animation.swordStow==1;
   test.update(1.f/60,{1,0},false,false);const bool begins=test.animation.swordStow<1&&test.animation.swordStow>.9f;
   for(int f=0;f<44;f++)test.update(1.f/60,{1,0},false,false);
   check(stowed&&begins&&test.animation.swordStow==0,"running stows the weapon and walking reverses the gesture");
   test.animation.swordStow=1;
   check(test.attack()&&test.animation.swordStow==0,"combat draws the equipped melee weapon immediately");
  }
 }
 {
  const double started=GetTime();
  using namespace hero_motion;
  for(Kind weapon:{Kind::Sword,Kind::Axe,Kind::Dagger,Kind::None}){
   bool contact=true,continuous=true,stored=true,limbs=true;
   float maxJump=0,minNormalDot=1;
   for(int gait=0;gait<12;gait++){
    hero_motion::Transform previous{};
    for(int frame=0;frame<=36;frame++){
     const float phase=frame/36.f;
     auto pose=evaluate(gait/12.f,1,phase,.3f);
     const Pose untwisted=pose;
     if(weapon==Kind::Sword)hero_sheath::apply(pose,phase);
     const Pose natural=pose;
     const auto mount=shield_stow::apply(pose,weapon,phase,12.3f,true,weapon==Kind::Sword?&untwisted:nullptr);
     const Vec wrist=pose.world[data::HandL].p,elbow=pose.world[data::ForearmL].p;
     if(phase<=.625f)contact&=length(mount.p-wrist-rotate(mount.q,shield_stow::HeldOffset))<.025f;
     if(phase>=.75f)stored&=length(mount.p-pose.point(data::Spine,shield_stow::socket(weapon,12.3f)))<.025f;
     limbs&=std::abs(length(wrist-elbow)-13)<.025f&&
            std::abs(length(elbow-pose.world[data::UpperArmL].p)-13)<.025f&&
            length(natural.world[data::FootL].p-pose.world[data::FootL].p)<.025f&&
            length(natural.world[data::HandR].p-pose.world[data::HandR].p)<.025f;
     if(frame>0){
      maxJump=std::max(maxJump,length(mount.p-previous.p));
      const auto a=rotate(previous.q,{0,1,0}),b=rotate(mount.q,{0,1,0});
      minNormalDot=std::min(minNormalDot,a.x*b.x+a.y*b.y+a.z*b.z);
     }
     previous=mount;
    }
   }
   continuous=maxJump<7.f&&minNormalDot>.84f;
   report<<"Shield weapon "<<int(weapon)<<": maximum step "<<maxJump<<", minimum facing dot "<<minNormalDot<<'\n';
   check(continuous&&contact&&stored&&limbs,
         "shield stays on the solved left hand during a continuous arc and seats on the spine before release");
  }
  auto sheet=render_targets::colorOnly(1728,620);
  BeginTextureMode(sheet);ClearBackground(Ink);
  for(int stage=0;stage<=8;stage++)for(int view=0;view<2;view++){
   AnimationState state;
   state.direction=view?Direction::NW:Direction::SE;
   state.moving=state.motionInitialized=true;state.phase=.2f;state.moveBlend=1;
   state.swordStow=state.runBlend=stage/8.f;
   actor_art::draw(0,96+stage*192.f,270+view*284.f,2.4f,state,0,false,true,Kind::Sword);
  }
  EndTextureMode();saveTexture(sheet,"qa-v21/shield-stow.png");UnloadRenderTexture(sheet);
  report<<"Shield motion render and export: "<<GetTime()-started<<" s\n";
 }
 {
  const double started=GetTime();
  auto sheet=LoadRenderTexture(1200,796);
  BeginTextureMode(sheet);ClearBackground(Ink);
  txt("TARCZE I HEŁMY TRZECH KRAIN",28,21,24,Gold);
  center("Przód",419,55,15,Muted);center("Profil",656,55,15,Muted);
  center("Bieg od tyłu",927,55,15,Muted);
  constexpr Kind shields[]={Kind::IronwoodRoundshield,Kind::SunwheelShield,Kind::ObsidianRoundshield};
  constexpr Kind helmets[]={Kind::RivetedGreathelm,Kind::SunwardHelm,Kind::AshenVisor};
  bool atlasAvailable=true;
  for(int realm=0;realm<3;realm++){
   const float y=91+realm*225.f;
   DrawLine(28,int(y),1172,int(y),Line);
   const auto shield=item(shields[realm]),helmet=item(helmets[realm]);
   atlasAvailable&=inventory_icons::draw(shield,62,y+62,1.7f,1);
   atlasAvailable&=inventory_icons::draw(helmet,62,y+145,1.7f,1);
   txt(itemName(shield),103,y+51,15,Text);txt(itemName(helmet),103,y+134,15,Text);
   const std::array<Item,6> gear{item(Kind::Sword),helmet,shield,item(Kind::Armor),item(Kind::Boots),Item{}};
   for(int view=0;view<3;view++){
    AnimationState pose{};
    pose.direction=view==0?Direction::S:view==1?Direction::E:Direction::N;
    if(view==2){pose.moving=pose.motionInitialized=true;pose.phase=.19f;pose.runBlend=pose.swordStow=pose.moveBlend=1;}
    actor_art::draw(0,419+view*247.f,y+190,2.05f,pose,0,false,false,Kind::Sword,false,BLANK,nullptr,-1,-1,-1,3,false,&gear);
   }
  }
  EndTextureMode();
  check(atlasAvailable,"all six new protective items are available through the live inventory atlas");
  saveTexture(sheet,"qa-v21/protective-gear.png");UnloadRenderTexture(sheet);

  sheet=LoadRenderTexture(1110,405);
  BeginTextureMode(sheet);ClearBackground(Ink);
  txt("KAPTURY WĘDROWCA",26,19,24,Gold);
  constexpr Kind looks[]={Kind::None,Kind::WebCowl,Kind::SlimeCowl,Kind::WolfHood,Kind::BanditHelm};
  for(int column=0;column<5;column++){
   const int x=column*216+24;
   const Kind kind=looks[column];
   center(column==0?"Bez kaptura":itemName(item(kind)),x+99,61,15,Text);
   const std::array<Item,6> gear{Item{},kind==Kind::None?Item{}:item(kind),Item{},item(Kind::Armor),Item{},Item{}};
   for(int view=0;view<3;view++){
    const int y=92+104*view;
    BeginScissorMode(x,y,198,95);
    AnimationState pose{};pose.direction=view==0?Direction::S:view==1?Direction::E:Direction::N;
    actor_art::draw(0,x+99.f,y+276.f,3.15f,pose,0,false,false,Kind::None,false,BLANK,nullptr,-1,-1,-1,3,false,&gear);
    EndScissorMode();
   }
  }
  EndTextureMode();saveTexture(sheet,"qa-v21/hood-profiles.png");UnloadRenderTexture(sheet);

  sheet=LoadRenderTexture(1420,486);
  BeginTextureMode(sheet);ClearBackground(Ink);
  txt("HEŁMY OTWARTE",24,18,24,Gold);
  bool openIcons=true;
  for(int column=0;column<int(OpenHelmetKinds.size());column++){
   const int x=column*199+14;
   const auto helmet=item(OpenHelmetKinds[column]);
   openIcons&=inventory_icons::draw(helmet,x+97.f,83,1.4f,1);
   center(itemName(helmet),x+97.f,120,13,Text);
   const std::array<Item,6> gear{Item{},helmet,Item{},item(Kind::Armor),Item{},Item{}};
   for(int view=0;view<3;view++){
    const int y=145+111*view;
    BeginScissorMode(x,y,193,100);
    AnimationState pose{};pose.direction=view==0?Direction::S:view==1?Direction::E:Direction::N;
    actor_art::draw(0,x+97.f,y+276.f,3.15f,pose,0,false,false,Kind::None,false,BLANK,nullptr,-1,-1,-1,3,false,&gear);
    EndScissorMode();
   }
  }
  EndTextureMode();
  check(openIcons,"all seven open helmets use their worn design in the live inventory atlas");
  saveTexture(sheet,"qa-v21/open-helmets.png");UnloadRenderTexture(sheet);
  Image iconAtlas=LoadImageFromTexture(inventory_icons::gearAtlas.texture);
  if(iconAtlas.data){
   const auto* colors=static_cast<const Color*>(iconAtlas.data);
   constexpr int cell=inventory_icons::Cell;
   for(int x0=0;x0<iconAtlas.width;x0+=cell){
    int painted=0,edge=0;
    for(int y=0;y<cell;y++)for(int x=0;x<cell;x++){
     const int alpha=colors[y*iconAtlas.width+x0+x].a;
     painted+=alpha>30;
     edge+=alpha>0&&(x<4||y<4||x>=cell-4||y>=cell-4);
    }
    check(painted>90&&edge==0,"a shared helmet or shield icon has complete art and transparent padding");
   }
   UnloadImage(iconAtlas);
  }
  report<<"Protective gear render and export: "<<GetTime()-started<<" s\n";
 }
 {
  const double started=GetTime();
  auto sheet=LoadRenderTexture(1200,966);
  BeginTextureMode(sheet);ClearBackground(Ink);
  txt("WŁÓCZNIE WĘDROWCA",24,18,24,Gold);
  const char* columns[]={"Chód", "Pchnięcie", "Bieg z bronią na plecach"};
  for(int i=0;i<3;i++)center(columns[i],400+i*268.f,52,15,Muted);
  for(int row=0;row<SpearCount;row++){
   const auto spear=item(Kind(FirstSpear+row));
   const float y=79+row*176.f;
   DrawLine(24,int(y),1175,int(y),Line);
   icon(spear,65,y+81,1.6f);
   txt(itemName(spear),116,y+76,15,Text);
   const std::array<Item,6> gear{spear,Item{},Item{},item(Kind::Armor),item(Kind::Boots),Item{}};
   for(int view=0;view<3;view++){
    AnimationState pose{};pose.direction=view==2?Direction::NW:Direction::E;
    pose.moving=pose.motionInitialized=view!=1;pose.phase=.29f;pose.moveBlend=pose.moving?1.f:0.f;
    if(view==2)pose.runBlend=pose.swordStow=1;
    const float swing=view==1?float(SpearImpactFrame)/SpearFrames:-1.f;
    actor_art::draw(0,400+view*268.f,y+155,1.64f,pose,0,false,false,Kind::Spear,false,BLANK,nullptr,-1,swing,-1,3,false,&gear);
   }
  }
  EndTextureMode();saveTexture(sheet,"qa-v21/spear-proportions.png");UnloadRenderTexture(sheet);
  report<<"Spear proportions render and export: "<<GetTime()-started<<" s\n";
 }
 {
  const double started=GetTime();
  const auto& power=powerAttack(PowerStyle::Spear);
  auto sheet=LoadRenderTexture(1100,435);
  BeginTextureMode(sheet);ClearBackground(Ink);
  txt("PUNKT TRAFIENIA WŁÓCZNI",24,16,24,Gold);
  const char* labels[]={"Zwykły atak", "Mocny atak po przytrzymaniu LPM"};
  Game sample;
  for(bool charged:{false,true}){
   const float progress=charged?power.contact/power.duration:float(SpearImpactFrame)/SpearFrames;
   const auto style=charged?PowerStyle::Spear:PowerStyle::Count;
   const float y=charged?396.f:221.f;
   txt(labels[int(charged)],24,y-141,15,Text);
   bool allTipsMatch=true;
   for(int id=FirstSpear;id<FirstSpear+SpearCount;id++)for(int dir=0;dir<8;dir++){
    sample.equipment[0]=item(Kind(id));
    sample.animation.direction=Direction(dir);
    sample.animation.facingInitialized=true;
    sample.animation.facingAngle=dir*PI/4;
    sample.animation.idleTime=.6f;
    const Vector2 feet{360.f+(id-FirstSpear)*159.f,y};
    const auto contact=sample.spearContact(charged);
    const Vector2 tip=power_fx::spearPoint(feet,contact,1);
    const auto draw=[&](const Vector2* probe){
     return actor_art::draw(0,feet.x,feet.y,1,sample.animation,std::sin(progress*PI),false,false,Kind::Spear,false,
         BLANK,probe,-1,progress,-1,3,false,&sample.equipment,-1,-1,0,-1,style,progress);
    };
    // The point lies on an edge that the strict triangle hit test can exclude.
    // Check nearby subpixel points in several directions for foreshortened tips.
    bool onSpear=draw(&tip);
    for(float radius:{.6f,1.2f,1.8f})for(int step=0;step<16&&!onSpear;step++){
     const float angle=step*PI/8;
     const Vector2 samplePoint{tip.x+radius*std::cos(angle),tip.y+radius*std::sin(angle)};
     onSpear=draw(&samplePoint);
    }
    allTipsMatch&=onSpear;
    if(!onSpear)report<<"Spear tip at "<<tip.x<<','<<tip.y<<", item "<<id<<", facing "<<dir<<", charged "<<charged<<'\n';
    if(dir==int(Direction::E)){
     draw(nullptr);
     DrawCircleLines(int(tip.x),int(tip.y),6,Green);
     DrawCircleV(tip,1.5f,Green);
    }
   }
   check(allTipsMatch,"ordinary and charged world hit endpoints lie on the drawn blade tips of all five spear profiles");
  }
  EndTextureMode();saveTexture(sheet,"qa-v21/spear-contact.png");UnloadRenderTexture(sheet);
  report<<"Spear contact render and export: "<<GetTime()-started<<" s\n";
 }
 {
  const double started=GetTime();
  auto sheet=LoadRenderTexture(1510,762);
  BeginTextureMode(sheet);ClearBackground(Ink);
  txt("BESTIE WIELKIEJ KNIEI",24,16,24,Gold);
  const char* captions[]={"Wilk · chód", "Wilk · ugryzienie", "Dzik · chód", "Dzik · atak kłami", "Dzik · szarża"};
  for(int row=0;row<5;row++){
   const bool wolf=row<2;
   const int type=wolf?1:BoarType;
   const float y=61+row*137.f;
   DrawLine(22,int(y),1488,int(y),Line);
   txt(captions[row],24,y+58,16,Text);
   for(int frame=0;frame<8;frame++){
    const float f=(frame+.15f)/8;
    Enemy enemy{50,type,{},{},enemyInfo(type).health,enemyInfo(type).health};
    auto& pose=enemy.animation;pose.direction=Direction::SE;
    pose.idleTime=f;pose.phase=f;pose.motionInitialized=true;
    if(row==0||row==2){pose.moving=true;pose.moveBlend=1;}
    if(row==1||row==3){
     const float windup=meleeWindup(type),recovery=meleeRecovery(type),now=f*(windup+recovery);
     if(now<windup)enemy.windup=windup-now;else enemy.strike=windup+recovery-now;
    }
    if(row==4){
     const float now=f*(BoarChargeWindup+BoarChargeDuration+BoarChargeRecovery);
     if(now<BoarChargeWindup)enemy.boarWindup=BoarChargeWindup-now;
     else if(now<BoarChargeWindup+BoarChargeDuration){
      enemy.boarRush=BoarChargeWindup+BoarChargeDuration-now;pose.moving=true;pose.moveBlend=1;pose.runBlend=1;
     }else enemy.boarRecovery=BoarChargeWindup+BoarChargeDuration+BoarChargeRecovery-now;
    }
    BeginMode2D({{235+frame*170.f,y+118},{0,0},0,1.55f});
    actor_art::drawEnemy(enemy,0,0);
    EndMode2D();
   }
  }
  EndTextureMode();saveTexture(sheet,"qa-v21/forest-beasts.png");UnloadRenderTexture(sheet);
  report<<"Forest beasts render and export: "<<GetTime()-started<<" s\n";
 }
 {
  const double started=GetTime();
  auto sheet=LoadRenderTexture(1600,674);
  BeginTextureMode(sheet);ClearBackground(Ink);
  txt("CHÓD WILKA I DZIKA",24,16,24,Gold);
  const char* captions[]={"Wilk · profil", "Wilk · trzy czwarte", "Dzik · profil", "Dzik · trzy czwarte"};
  for(int row=0;row<4;row++){
   const int type=row<2?1:BoarType;
   const float y=62+row*149.f;
   DrawLine(22,int(y),1578,int(y),Line);
   txt(captions[row],24,y+61,15,Text);
   Enemy enemy{51,type,{},{},enemyInfo(type).health,enemyInfo(type).health};
   auto& pose=enemy.animation;
   pose.direction=row%2?Direction::SE:Direction::E;
   pose.moving=pose.motionInitialized=true;
   pose.moveBlend=1;
   for(int frame=0;frame<8;frame++){
    pose.phase=frame/8.f;
    const Vector2 anchor{240+frame*181.f,y+125};
    ellipse(anchor.x,anchor.y+3,72,11,Card);
    BeginMode2D({anchor,{0,0},0,1.7f});
    actor_art::drawEnemy(enemy,0,0);
    EndMode2D();
   }
  }
  EndTextureMode();saveTexture(sheet,"qa-v21/forest-gait.png");UnloadRenderTexture(sheet);
  report<<"Forest gait render and export: "<<GetTime()-started<<" s\n";
 }
 {
  RenderTexture2D target=LoadRenderTexture(384,330);
  const auto verifyOutline=[&](auto draw,const char* label){
   auto sample=[&](Color color){
    BeginTextureMode(target);ClearBackground(BLANK);draw(color);EndTextureMode();
    return LoadImageFromTexture(target.texture);
   };
   Image normal=sample(BLANK),marked=sample(Gold);
   const auto* a=static_cast<const Color*>(normal.data);
   const auto* b=static_cast<const Color*>(marked.data);
   int rim=0,body=0,recolored=0;
   for(int i=0;i<normal.width*normal.height;i++){
    rim+=a[i].a<12&&b[i].a>80;
    if(a[i].a>249){body++;recolored+=std::abs(a[i].r-b[i].r)+std::abs(a[i].g-b[i].g)+std::abs(a[i].b-b[i].b)>8;}
   }
   report<<"Outline "<<label<<": rim="<<rim<<"; unchanged body="<<body-recolored<<"/"<<body<<'\n';
   check(rim>35&&body>200&&recolored<body*.04f,label);
   UnloadImage(normal);UnloadImage(marked);
  };
  for(int type=0;type<EnemyTypeCount;type++){
   Enemy e{800+type,type,{},{},100,100};e.animation.direction=Direction::SW;e.animation.moving=true;e.animation.phase=.29f;
   verifyOutline([&](Color c){actor_art::drawEnemy(e,192,267,c);},"enemy outline hugs the animated silhouette");
  }
  for(auto realm:{WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano})for(int state=0;state<3;state++){
   TreasureChest chest{};chest.location=realm;chest.opened=state>0;if(state==1)chest.loot[0]=item(Kind::Gold);
   verifyOutline([&](Color c){treasure_art::draw(chest,{192,179},c);},"chest outline uses the lid and contents of its current state");
  }
  UnloadRenderTexture(target);
 }
 RenderTexture2D sample=LoadRenderTexture(240,260);
 auto verify=[&](int species,int dir,float roll,float strike){
  AnimationState pose;pose.direction=static_cast<Direction>(dir);pose.moving=roll<0;pose.phase=.17f;pose.idleTime=.7f;
  float attack=strike<0?0:std::sin(strike*PI);
  BeginTextureMode(sample);ClearBackground(BLANK);
  actor_art::draw(species,120,215,1,pose,attack,species==0,species==0,Kind::Sword,false,BLANK,nullptr,roll,strike);EndTextureMode();
  Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);
  int opaque=0,clipped=0,interior=0,mismatch=0;
  for(int y=0;y<260;y++)for(int x=0;x<240;x++)if(pixels[y*240+x].a>230){opaque++;clipped+=x<2||y<2||x>=238||y>=258;}
  for(int y=6;y<254;y+=7)for(int x=6;x<234;x+=7){
   bool solid=true,empty=true;for(int dy:{-3,0,3})for(int dx:{-3,0,3}){int alpha=pixels[(y+dy)*240+x+dx].a;solid&=alpha>230;empty&=alpha<8;}
   if(!solid&&!empty)continue;Vector2 point{float(x)+.5f,float(y)+.5f};
   bool hit=actor_art::draw(species,120,215,1,pose,attack,species==0,species==0,Kind::Sword,false,BLANK,&point,roll,strike);
   interior+=solid;mismatch+=solid?!hit:hit;
  }
  // Thin wolf legs and branch silhouettes can fall between the wide probes.
  // Sample the GPU's opaque interior at pixel centers and avoid its AA fringe.
  if(interior<=2)for(int y=2;y<258&&interior<12;y++)for(int x=2;x<238&&interior<12;x++){
   bool solid=true;for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++)solid&=pixels[(y+dy)*240+x+dx].a>230;
   if(!solid)continue;Vector2 point{x+.5f,y+.5f};
   bool hit=actor_art::draw(species,120,215,1,pose,attack,species==0,species==0,Kind::Sword,false,BLANK,&point,roll,strike);
   interior++;mismatch+=!hit;
  }
  report<<"Species="<<species<<"; direction="<<dir<<"; roll="<<roll<<"; strike="<<strike<<"; opaque="<<opaque<<"; interior="<<interior<<"; mismatch="<<mismatch<<'\n';
  check(opaque>130&&clipped==0,"detailed actor pose has a complete visible silhouette without clipped parts");
  check(interior>2&&mismatch==0,"GPU-rasterized body interior and empty space match pointer geometry");
  UnloadImageColors(pixels);UnloadImage(im);
 };
 for(int species=0;species<=EnemyTypeCount;species++)for(int d=0;d<8;d++)verify(species,d,-1,-1);
 for(int d=0;d<8;d++){verify(0,d,-1,.5f);verify(0,d,.2f,-1);verify(0,d,.65f,-1);}
 UnloadRenderTexture(sample);
 RenderTexture2D sheet=LoadRenderTexture(1300,(EnemyTypeCount+1)*190+100);BeginTextureMode(sheet);ClearBackground(Ink);
 txt("SZCZEGÓŁOWE SYLWETKI — OSIEM KIERUNKÓW",24,16,24,Gold);
 const char* directions[]={"E","SE","S","SW","W","NW","N","NE"};
 for(int d=0;d<8;d++)center(directions[d],220+d*140.f,56,14,Text);
 for(int species=0;species<=EnemyTypeCount;species++){
  txt(species==0?"Wędrowiec":enemyInfo(species-1).name,16,110+species*190,14,Muted);
  for(int d=0;d<8;d++){AnimationState pose;pose.direction=static_cast<Direction>(d);pose.moving=true;pose.phase=.17f;pose.idleTime=.7f;
   actor_art::draw(species,220+d*140.f,230+species*190,1,pose,0,species==0,species==0);}
 }
 EndTextureMode();saveTexture(sheet,"qa-v21/all-actors-8-directions.png");UnloadRenderTexture(sheet);
 fs::create_directories("qa-v24/hero");
 RenderTexture2D heroSheet=LoadRenderTexture(1680,1310);BeginTextureMode(heroSheet);ClearBackground(Ink);
 txt("WĘDROWIEC — NOWA SYLWETKA I ANIMACJE",24,18,25,Gold);
 const char* poses[]={"KAPTUR","CHÓD","MIECZ / TARCZA","ŁUK","UNIK"};
 for(int d=0;d<8;d++)center(directions[d],240+d*195.f,67,14,Text);
 for(int row=0;row<5;row++){
  txt(poses[row],18,146+row*240,14,Muted);
  for(int d=0;d<8;d++){
   AnimationState state;state.direction=static_cast<Direction>(d);state.idleTime=.7f;state.moving=row==1;state.phase=.17f;
   float x=240+d*195.f,y=286+row*240;ellipse(x,y+2,49,17,rgb(23,34,36));
   actor_art::draw(0,x,y,1.9f,state,row==2?1:0,row==2,row==2||row==4,row==3?Kind::Bow:Kind::Sword,
                   false,BLANK,nullptr,row==4?.45f:-1,row==2?.5f:-1,row==3?.85f:-1);
  }
 }
 EndTextureMode();saveTexture(heroSheet,"qa-v24/hero/poses-8-directions.png");UnloadRenderTexture(heroSheet);
 for(int type=0;type<PropTypeCount;type++){
  Image im=LoadImageFromTexture(propArt[type].texture);Color* pixels=LoadImageColors(im);int opaque=0,clipped=0;
  for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++)if(pixels[y*im.width+x].a>230){opaque++;clipped+=x<2||x>=im.width-2||y<2||y>=im.height-2;}
  check(opaque>250&&clipped==0,"baked foliage, stone and ruin detail fits its full texture and pointer mask");
  UnloadImageColors(pixels);UnloadImage(im);
 }
 const double sceneryStart=GetTime();
 refreshDisplayLayout();
 {
  Image bank=LoadImageFromTexture(terrain.texture);ImageFlipVertical(&bank);
  const float kx=float(bank.width)/TerrainWidth,ky=float(bank.height)/TerrainHeight;
  auto sample=[&](V p){
   const int x=int((TerrainOriginX+(p.x-p.y)*40)*kx),y=int((90+(p.x+p.y)*20)*ky);
   return GetImageColor(bank,x,y);
  };
  auto alpha=[&](V p){return sample(p).a;};
  game=Game{};
  auto cliff=render_targets::colorOnly(int(std::round(WorldW*renderScale)),int(std::round(H*renderScale)));
  const auto renderCliff=[&](V position){
   game.player=position;
   BeginTextureMode(cliff);ClearBackground(BLANK);BeginMode2D({{0,0},{0,0},0,renderScale});
   forest_cliff::draw();
   EndMode2D();EndTextureMode();
   Image view=LoadImageFromTexture(cliff.texture);ImageFlipVertical(&view);
   return view;
  };
  check(forest_cliff::rock.id&&IsShaderValid(forest_cliff::material)&&forest_cliff::material.id!=rlGetShaderIdDefault(),
        "forest escarpment loads the layered stone material");
  for(int side=0;side<4;side++){
   int dry=0,exterior=0,wide=0,narrow=0;
   bool collisionAligned=true;
   for(int i=8;i<MapSize-8;i++){
    const auto edge=forest_shore::walkEdgePoint(float(i),side,.20f);
    collisionAligned&=game.walkable(edge)&&!game.walkable(forest_shore::walkEdgePoint(float(i),side,-.20f));
    dry+=alpha(edge)>=243;
    exterior+=alpha(forest_shore::walkEdgePoint(float(i),side,-1.25f))<32;
    const float overhang=forest_shore::cliffRim(float(i),side);
    wide+=overhang>.36f;narrow+=overhang<.22f;
   }
   check(collisionAligned&&dry==MapSize-16&&exterior==MapSize-16&&wide>12&&narrow>12,
         "forest paths meet the irregular cliff rim without an invisible margin or detached ground");
   for(float corner:{MapWalkMin,MapWalkMax}){
    const int adjacent=(side%2==0?1:0)+(corner==MapWalkMin?0:2);
    const float alongAdjacent=side<2?MapWalkMin:MapWalkMax;
    bool joined=true;
    for(size_t row=0;row<forest_cliff::Drop.size();row++){
     const auto a=forest_cliff::vertex(corner,side,row).position,b=forest_cliff::vertex(alongAdjacent,adjacent,row).position;
     joined&=std::abs(a.x-b.x)<.01f&&std::abs(a.y-b.y)<.01f;
    }
    check(joined,"forest cliff terraces join continuously at the corner");
   }
   const auto beyondRim=[&](int x,int y,Vector2 origin){
    const float u=((x+.5f)/renderScale-origin.x)/40,v=((y+.5f)/renderScale-origin.y)/20;
    const float axis=(side%2==0?v+u:v-u)*.5f;
    return side<2?axis<MapWalkMin-1.5f:axis>MapWalkMax+1.5f;
   };
   if(side<2){
    Image exposed=renderCliff(forest_shore::walkEdgePoint(81,side,2.f));
    const auto origin=screenFlat({0,0});
    int exteriorSamples=0,rockSamples=0;
    for(int y=12;y<exposed.height;y+=12)for(int x=12;x<exposed.width;x+=12)if(beyondRim(x,y,origin)){
     ++exteriorSamples;rockSamples+=GetImageColor(exposed,x,y).a>30;
    }
    check(exteriorSamples>100&&rockSamples*100<exteriorSamples*3,
          "upper forest rim opens directly onto the valley without an exposed rock shelf");
    UnloadImage(exposed);
   }else for(float inset:{.25f,8.5f}){
    Image exposed=renderCliff(forest_shore::walkEdgePoint(81,side,inset));
    const auto origin=screenFlat({0,0});
    int exteriorSamples=0,rockSamples=0;
    const int bottom=exposed.height-1;
    for(int x=3;x<exposed.width-3;x+=3)if(beyondRim(x,bottom,origin)){
     ++exteriorSamples;rockSamples+=GetImageColor(exposed,x,bottom).a>245;
    }
    report<<"Lower forest edge "<<side<<", camera inset "<<inset<<": bottom rock pixels "<<rockSamples<<" / "<<exteriorSamples<<'\n';
    check(exteriorSamples>40&&rockSamples*100>=exteriorSamples*98,
          "lower forest cliff remains opaque at the actual bottom row of the viewport");
    UnloadImage(exposed);
   }
  }
  UnloadRenderTexture(cliff);
  UnloadImage(bank);
 }
 {
  Image source=LoadImage(forest_tree_art::files[2]);
  Image baked=LoadImageFromTexture(propArt[5].texture);ImageFlipVertical(&baked);
  Color* original=LoadImageColors(source);Color* shaded=LoadImageColors(baked);
  int foliage=0,preserved=0;long long before=0,after=0,greenOverRed=0;
  if(original&&shaded&&source.width==baked.width&&source.height==baked.height)
   for(int i=0;i<source.width*source.height;i++){
    const auto& a=original[i];const auto& b=shaded[i];
    if(a.a<235||int(a.g)-a.b<30||int(a.g)-a.r<-8)continue;
    foliage++;preserved+=b.a==a.a;before+=a.g;after+=b.g;greenOverRed+=int(b.g)-b.r;
   }
  check(foliage>1500&&preserved==foliage&&after*100<before*85&&greenOverRed>foliage*7,
        "forest foliage is darker green and retains its opaque silhouette");
  if(original)UnloadImageColors(original);if(shaded)UnloadImageColors(shaded);
  UnloadImage(source);UnloadImage(baked);
 }
 {
  auto sample=render_targets::colorOnly(820,560);
  game=Game{};game.player={25,25};game.time=.65f;
  for(const auto& [type,height]:{std::pair{0,220},std::pair{4,220},std::pair{5,220},
                                std::pair{7,100},std::pair{8,66},std::pair{12,300}}){
   const Prop prop{toWorld({350,480}),type,1};
   const Vector2 base=screen(prop.pos);
   BeginTextureMode(sample);ClearBackground(BLANK);
   drawPropTexture(prop,propArt[type].texture,false,WHITE);
   EndTextureMode();
   Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);
   const auto* pixels=static_cast<const Color*>(im.data);
   int high=im.height,selected=0,mismatches=0,compared=0;
   for(int y=5;y<im.height-5;y+=2)for(int x=5;x<im.width-5;x+=2){
    const auto pixel=pixels[y*im.width+x];
    if(pixel.a>245)high=std::min(high,y);
    if(pixel.a>245||pixel.a==0){
     const bool hit=propContains(prop,{x+.5f,y+.5f});
     mismatches+=hit!=(pixel.a>245);compared++;
     selected+=hit;
    }
   }
   const int forestSlot=forest_tree_art::slot(type);
   const bool baked=forestSlot>=0?forest_tree_art::baked[forestSlot]:
                                  desert_plant_art::baked[desert_plant_art::slot(type)];
   check(baked&&propArt[type].texture.width==540&&propArt[type].texture.height==690&&base.y-high>=height,
         "detailed trees and cacti render at the expected height above their roots");
   check(selected>(type==7||type==8?300:800)&&mismatches<compared*.007f&&propContains(prop,{base.x,base.y-3}),
         "enlarged canopy remains selectable through its real wind-deformed pixels and rooted at the ground");
   const float support=std::max(210.f,208*propArtSize(prop));
   check(propVisible(prop,{base.x,H+support-1})&&!propVisible(prop,{base.x,H+support+1}),
         "scenery visibility accounts for the scaled sprite at the viewport edge");
   if(type==7||type==8)check(base.y-high<=(type==7?125:95),
                             "cactus height remains comparable to the hero");
   UnloadImage(im);
  }
  actor_art::finishShadowFrame();
  const Vector2 feet{350,480};
  actor_art::Silhouette silhouette;
  BeginTextureMode(sample);ClearBackground(BLANK);hero(feet.x,feet.y,1,false,&silhouette);EndTextureMode();
  const Vector2 treeBase{feet.x,feet.y+34},beside{feet.x+140,feet.y+20};
  const Prop tree{toWorld(treeBase),0,1};
  check(silhouette.ready&&propOpacity(tree,treeBase,feet,true,silhouette)<.5f,
        "opaque foreground branches and trunk covering the drawn hero lower tree opacity");
  check(propOpacity(tree,beside,feet,true,silhouette)==1&&propOpacity(tree,treeBase,feet,false,silhouette)==1,
        "transparent pixels near the hero and scenery painted behind the hero stay opaque");
  UnloadRenderTexture(sample);
 }
 check(terrain_paint::ready[true]&&terrain_paint::ready[false]&&volcano_art::roadBakeValid,
       "all three realms bake their painted trails into the height-mapped terrain");
 check(forest_water::valley.id&&desert_art::desertPanorama.id&&desert_art::paintedRidge.id&&
       volcano_art::abyssDistance.width==1672&&volcano_art::abyssCliff.width==1536,
       "painted panoramas and regional edge materials load before rendering");
 check(desert_art::oasis_water::ready()&&volcano_art::lavaClock>=0&&volcano_art::lava.width==640,
       "oasis and lava render with their live material shaders and painted lava texture");
 for(bool volcanic:{false,true}){
  game=Game{};game.activateField(volcanic?WorldLocation::Volcano:WorldLocation::Desert);
  game.player=volcanic?V{25,48}:DesertOasis+V{2.8f,0};
  auto surface=render_targets::colorOnly(WorldW,H);
  auto sampleFluid=[&](float t){
   game.time=t;BeginTextureMode(surface);ClearBackground(BLANK);
   if(volcanic)volcano_art::flow(false);else desert_art::oasis_water::draw();
   EndTextureMode();return LoadImageFromTexture(surface.texture);
  };
  Image first=sampleFluid(.2f),second=sampleFluid(1.8f);
  const auto* a=static_cast<const Color*>(first.data);
  const auto* b=static_cast<const Color*>(second.data);
  int changed=0,covered=0;
  for(int i=0;i<first.width*first.height;i++)if(a[i].a>230){
   covered++;
   changed+=std::abs(int(a[i].r)-b[i].r)+std::abs(int(a[i].g)-b[i].g)+std::abs(int(a[i].b)-b[i].b)>8;
  }
  report<<(volcanic?"Lava":"Oasis")<<" motion pixels: "<<changed<<" / "<<covered<<'\n';
  check(covered>10000&&changed>650,"isolated fluid changes visibly with time");
  if(!volcanic){
   const Vector2 p=screen(DesertOasis);
   check(GetImageColor(first,int(p.x),H-1-int(p.y)).a>240&&GetImageColor(first,int(p.x+140),H-1-int(p.y+69)).a==0,
         "oasis shader keeps its irregular wet shore inside a transparent margin");
  }
  UnloadImage(first);UnloadImage(second);UnloadRenderTexture(surface);
  game.time=1.3f;game.messageTime=0;welcome=paused=help=false;mouse={0,0};renderScene();
  saveTexture(canvas,volcanic?"qa-v21/lava-flow-painted.png":"qa-v21/oasis-water.png");
 }
 for(bool forest:{true,false}){
  Image routes=terrain_paint::routeMask(forest);
  const auto coverage=[&](V p){return GetImageColor(routes,int(p.x*terrain_paint::Samples),int(p.y*terrain_paint::Samples)).r;};
  const V junction=forest?V{39,84}:V{21,20};
  const V openGround=forest?V{27,27}:V{9,35};
  check(coverage(junction)>235&&coverage(openGround)==0,"route paint reaches its junction and leaves open ground distinct");
  UnloadImage(routes);
 }
 for(const auto& realm:{std::pair{WorldLocation::Forest,"forest"},std::pair{WorldLocation::Desert,"desert"},std::pair{WorldLocation::Volcano,"caldera"}}){
  game=Game{};game.activateField(realm.first);welcome=paused=help=false;
  game.player=game.inVolcano()?VolcanoArrival:game.inDesert()?DesertArrival:ForestArrival;
  game.messageTime=0;mouse={0,0};renderScene();saveTexture(canvas,TextFormat("qa-v21/scenery-%s.png",realm.second));
  if(realm.first==WorldLocation::Forest){
   game.player={2,82};renderScene();saveTexture(canvas,"qa-v21/forest-grove-shore.png");
   game.player={MapWalkMax-2.f,81};renderScene();saveTexture(canvas,"qa-v21/forest-cliff-foreground.png");
   game.player={81,MapWalkMax-2.f};renderScene();saveTexture(canvas,"qa-v21/forest-cliff-foreground-opposite.png");
  }
  game.player=game.inVolcano()?V{25,48}:game.inDesert()?V{21,20}:ForestRitualApproach;
  if(realm.first==WorldLocation::Forest){
   const auto& chest=game.chests[ForestCircleChestId];
   const auto center=screen(ForestRitualCircle),anchor=screen(chest.pos);
   check(center.x==anchor.x&&center.y==anchor.y&&game.walkable(game.player)&&game.chestReachable(chest.id),
         "forest chest is centered in the ritual circle and reachable from the field arrival");
  }
  renderScene();saveTexture(canvas,TextFormat("qa-v21/trail-%s.png",realm.second));
  if(game.hasFountains()){
   const bool volcanic=game.inVolcano();
   const auto& water=volcanic?fountain_art::volcanoAtlas:fountain_art::atlas;
   check(water[0].id&&water[1].id&&water[0].width==1024&&water[0].height==960,"regional fountains provide complete fill and drain animations");
   const V gate=volcanic?VolcanoPortal:DesertPortal;
   const auto gateProp=std::find_if(game.props.begin(),game.props.end(),[&](const Prop& p){return p.type==13&&distance(p.pos,gate)<.01f;});
   check(volcano_art::gateFrame().id&&gateProp!=game.props.end()&&IsShaderValid(volcano_art::portalClip),"regional portal loads its frame and the aperture shader");
   if(gateProp!=game.props.end()){
    game.player=gate+V{1.35f,1.35f};const Vector2 p=screen(gate);
    check(propContains(*gateProp,{p.x,p.y-93})&&!propContains(*gateProp,{p.x+86,p.y-172}),"portal accepts the magical opening and rejects the transparent corner");
    renderScene();saveTexture(canvas,TextFormat("qa-v21/gate-%s.png",realm.second));
   }
   game.player=game.fountains()[0]+V{1.5f,1.5f};
   renderScene();saveTexture(canvas,TextFormat("qa-v21/fountain-%s.png",realm.second));
  }
 }
 report<<"Scenery render and export: "<<GetTime()-sceneryStart<<" s\n";
 for(int k=1;k<ItemKindCount;k++){
  RenderTexture2D itemSample=LoadRenderTexture(64,64);BeginTextureMode(itemSample);ClearBackground(BLANK);icon(item(static_cast<Kind>(k),1),32,32);EndTextureMode();
  Image im=LoadImageFromTexture(itemSample.texture);Color* pixels=LoadImageColors(im);int opaque=0,clipped=0;
  for(int y=0;y<64;y++)for(int x=0;x<64;x++)if(pixels[y*64+x].a>230){opaque++;clipped+=x<2||x>=62||y<2||y>=62;}
  if(opaque<=25||clipped!=0)report<<"Item "<<k<<": opaque="<<opaque<<"; clipped="<<clipped<<'\n';
  check(opaque>25&&clipped==0,"detailed item icon is visible and fits inside backpack bounds");
  UnloadImageColors(pixels);UnloadImage(im);UnloadRenderTexture(itemSample);
 }
 game=Game{};welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;mouse={0,0};dragging={};refreshDisplayLayout();
 game.messageTime=0;renderScene();BeginDrawing();presentCanvas();Image actual=presentedImage();ExportImage(actual,"qa-v21/detailed-forest.png");ExportImage(actual,"qa-v24/hero/forest.png");UnloadImage(actual);EndDrawing();finishFramePacing();
 report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
