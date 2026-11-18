#pragma once

namespace traveler_qa {
enum class Action { Run, Roll, Fall, Flinch, Strike, Windup, Release, Skill, Stow, Draw, BowLoose };
struct Shot {const char* name;Action action;Kind kind;};
constexpr Shot shots[]={
 {"run",Action::Run,Kind::Sword},{"roll",Action::Roll,Kind::Sword},
 {"fall-and-get-up",Action::Fall,Kind::Sword},{"receive-hit",Action::Flinch,Kind::Sword},
 {"axe-chop",Action::Strike,Kind::Axe},{"dagger-stab",Action::Strike,Kind::Dagger},
 {"spear-thrust",Action::Strike,Kind::Spear},{"bow-draw",Action::Strike,Kind::Bow},{"bow-loose",Action::BowLoose,Kind::Bow},
 {"sword-charge",Action::Windup,Kind::Sword},{"axe-charge",Action::Windup,Kind::Axe},
 {"dagger-charge",Action::Windup,Kind::Dagger},{"spear-charge",Action::Windup,Kind::Spear},{"bow-charge",Action::Windup,Kind::Bow},
 {"sword-power",Action::Release,Kind::Sword},{"axe-power",Action::Release,Kind::Axe},
 {"dagger-power",Action::Release,Kind::Dagger},{"spear-power",Action::Release,Kind::Spear},{"bow-power",Action::Release,Kind::Bow},
 {"strong-cut",Action::Skill,Kind::Sword},{"cleave",Action::Skill,Kind::Axe},
 {"double-thrust",Action::Skill,Kind::Dagger},{"skewer",Action::Skill,Kind::Spear},{"piercing-arrow",Action::Skill,Kind::Bow},
 {"sword-and-shield-stow",Action::Stow,Kind::Sword},{"sword-and-shield-draw",Action::Draw,Kind::Sword},
 {"dagger-stow",Action::Stow,Kind::Dagger},{"dagger-draw",Action::Draw,Kind::Dagger},
 {"axe-stow",Action::Stow,Kind::Axe},{"axe-draw",Action::Draw,Kind::Axe},
 {"spear-stow",Action::Stow,Kind::Spear},{"spear-draw",Action::Draw,Kind::Spear},
};

inline void pose(Game& actor,const Shot& shot,int direction,float phase){
 auto& state=actor.animation;
 state={};
 state.direction=Direction(direction);
 state.facingInitialized=state.motionInitialized=true;
 state.facingAngle=state.targetAngle=direction*AnimationPi/4;
 state.phase=.23f;
 const auto style=powerStyle(shot.kind);
 switch(shot.action){
 case Action::Run:
  state.moving=state.sprinting=true;
  state.phase=phase;
  state.moveBlend=state.runBlend=state.swordStow=1;
  break;
 case Action::Roll:
  actor.rollRemaining=(1-phase)*RollDuration;
  break;
 case Action::Fall:
  actor.heroKnockdownFacing=state.facingAngle;
  actor.heroKnockdownRemaining=(1-phase)*HeroKnockdownDuration;
  break;
 case Action::Flinch:
  actor.heroHitDirection={.8f,.6f};
  actor.heroHitReactionRemaining=(1-phase)*HeroHitReactionDuration;
  break;
 case Action::Windup:
  if(shot.kind==Kind::Bow){
   actor.bowDrawing=true;actor.bowDrawTime=phase*PowerHoldTime;
  }else{
   actor.heldAttackWeapon=actor.equipment[0];actor.heldAttackTime=phase*PowerHoldTime;
  }
  break;
 case Action::Release:
  if(shot.kind==Kind::Bow){
   actor.lastBowCharge=1;actor.lastBowPower=true;
   actor.bowReleaseTime=(1-phase)*powerAttack(style).duration;
  }else{
   actor.meleeDuration=powerAttack(style).duration;
   actor.swing=(1-phase)*actor.meleeDuration;
  }
  break;
 case Action::Strike:
 case Action::Skill:
  if(shot.kind==Kind::Bow){
   actor.piercingArrowArmed=shot.action==Action::Skill;
   actor.bowDrawing=true;actor.bowDrawTime=phase*BowDrawDuration;
  }else{
   actor.swingStrong=shot.action==Action::Skill;
   actor.meleeDuration=shot.kind==Kind::Spear?SpearDuration:shot.kind==Kind::Dagger?DaggerAttackDuration:
                      shot.kind==Kind::Axe?AxeAttackDuration:actor.swingStrong?StrongCutDuration:SwingDuration;
   actor.swing=(1-phase)*actor.meleeDuration;
  }
  break;
 case Action::Stow:
 case Action::Draw:
  state.sprinting=shot.action==Action::Stow;
  state.swordStow=shot.action==Action::Stow?phase:1-phase;
  break;
 case Action::BowLoose:
  actor.lastBowCharge=1;actor.bowReleaseTime=(1-phase)*.18f;
  break;
 }
}
}

template<class Check>void verifyTravelerViews(Check check){
 using namespace traveler_qa;
 const auto path=fs::path("qa-hero-art")/"traveler-motion";
 fs::create_directories(path);
 constexpr int width=244,height=208;
 constexpr float phases[]={.04f,.22f,.45f,.64f,.82f,.96f};
 RenderTexture2D stage=LoadRenderTexture(width*8,height*std::size(phases));
 std::ofstream receipt(path/"coverage.csv");
 receipt<<"action,direction,distinct_frames,solid_pixels_min,clipped_pixels\n";
 for(const auto& shot:shots){
  Game actor;
  actor.equipment[0]=item(shot.kind);
  actor.equipment[2]=item(Kind::IronwoodRoundshield);
  BeginTextureMode(stage);
  ClearBackground(BLANK);
  for(int frame=0;frame<int(std::size(phases));frame++)for(int dir=0;dir<8;dir++){
   pose(actor,shot,dir,phases[frame]);
   actor_art::drawPlayer(actor,width*(dir+.5f),height*frame+146.f,.93f);
  }
  EndTextureMode();
  Image image=LoadImageFromTexture(stage.texture);
  ImageFlipVertical(&image);
  Color* pixels=LoadImageColors(image);
  for(int dir=0;dir<8;dir++){
   std::array<std::uint64_t,std::size(phases)> hashes{};
   int clipped=0,minimum=width*height;
   for(int frame=0;frame<int(std::size(phases));frame++){
    std::uint64_t hash=14695981039346656037ULL;
    int solid=0;
    for(int y=0;y<height;y++)for(int x=0;x<width;x++){
     Color c=pixels[(frame*height+y)*image.width+dir*width+x];
     if(c.a<32)continue;
     solid++;
     clipped+=x<=1||y<=1||x+2>=width||y+2>=height;
     hash^=(std::uint64_t(y*width+x)<<32)|std::uint32_t(ColorToInt(c));
     hash*=1099511628211ULL;
    }
    minimum=std::min(solid,minimum);
    hashes[frame]=hash;
   }
   std::sort(hashes.begin(),hashes.end());
   const int distinct=int(std::unique(hashes.begin(),hashes.end())-hashes.begin());
   receipt<<shot.name<<','<<dir<<','<<distinct<<','<<minimum<<','<<clipped<<'\n';
   const std::string label=std::string(shot.name)+" / "+std::to_string(dir)+" / full player silhouette and motion";
   check(minimum>400&&clipped==0&&distinct>=3,label.c_str());
  }
  UnloadImageColors(pixels);
  check(ExportImage(image,(path/(std::string(shot.name)+".png")).string().c_str()),"eight in-game views exported");
  UnloadImage(image);
 }
 UnloadRenderTexture(stage);
}
