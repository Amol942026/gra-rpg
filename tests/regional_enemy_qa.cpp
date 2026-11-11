#include "raylib.h"
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main

int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 const fs::path output=fs::absolute(argv[2]);
 if(!fs::create_directory(output))return 4;
 fs::current_path(fs::absolute(argv[0]).parent_path());
 SetTraceLogLevel(LOG_ERROR);
 SetConfigFlags(FLAG_WINDOW_HIDDEN);
 sync_present::useOpenGL();
 InitWindow(W,H,"Emberfall regional creatures --no-audio");
 if(!IsWindowReady())return 5;
 audioSystem.init(false,false);
 makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();
 makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);

 int checks=0,failures=0;
 std::ofstream results(output/"checks.txt");
 auto check=[&](bool pass,const std::string& text){
  checks++;failures+=!pass;
  results<<(pass?"PASS ":"FAIL ")<<text<<'\n';
 };
 auto count=[](const Game& world,int type){
  return std::count_if(world.enemies.begin(),world.enemies.end(),[type](const Enemy& e){return e.type==type;});
 };
 auto playCount=[](const Game& world,Cue cue){
  return std::count_if(world.sounds.begin(),world.sounds.end(),[cue](const SoundEvent& e){return e.cue==cue;});
 };

 for(int i=0;i<RegionalEnemyTypeCount;i++){
  const int type=SporekinType+i;
  const auto& atlas=regional_enemy_art::atlases[i];
  const auto realm=i==0?WorldLocation::Forest:i==1?WorldLocation::Desert:WorldLocation::Volcano;
  const std::string slug=regional_enemy_art::Names[i];
  check(atlas.art.id&&atlas.silhouette.id,"atlas and hover masks "+slug);
  check(atlas.alpha.size()==size_t(atlas.art.width)*atlas.art.height,"complete RGBA alpha "+slug);
  game=Game{};game.activateField(realm);
  check(count(game,type)==RegionalEncountersPerRealm,"authored encounters "+slug);
  for(const auto& e:game.enemies)if(regionalEnemy(e.type)){
   check(e.type==type&&game.enemyPositionClear(e.pos,e.radius()),"safe spawn "+std::to_string(e.id));
  }
  auto first=std::find_if(game.enemies.begin(),game.enemies.end(),[type](const Enemy& e){return e.type==type;});
  if(first==game.enemies.end())continue;
  int id=first->id;
  Enemy* e=&*first;
  game.player=e->pos+V{1,0};
  for(V delta:{V{1,0},V{0,-1},V{-1,0},V{0,1},V{.8f,.8f}}){
   if(game.walkable(e->pos+delta)&&game.clearEnemySegment(e->pos,e->pos+delta,0)){
    game.player=e->pos+delta;
    break;
   }
  }
  intro={};towerStory={};clearTransientUi();invalidatePausedWorld();
  welcome=paused=false;game.time=2;game.messageTime=0;mouse={0,0};
  e->animation.direction=Direction::SE;
  renderScene();
  saveTexture(canvas,(output/(slug+".png")).string());
  for(Direction facing:{Direction::SE,Direction::NW}){
   e->animation.direction=facing;
   for(int pose=0;pose<5;pose++){
    e->alive=pose!=4;e->animation.moving=pose==1;e->animation.phase=.4f;
    e->windup=pose==2?.09f:0;e->strike=0;e->flash=pose==3?.16f:0;
    bool picked=false;
    for(int y=15;y<150&&!picked;y+=3)for(int x=34;x<207&&!picked;x+=3){
     Vector2 probe{float(x),float(y)};
     picked=e->alive?actor_art::drawEnemy(*e,120,121,BLANK,&probe):corpseArt(*e,120,121,BLANK,&probe);
    }
    Vector2 outside{19,150};
    check(picked&&!regional_enemy_art::draw(*e,120,121,BLANK,&outside),"pose mask "+slug+" "+std::to_string(pose));
   }
  }
  e->alive=true;e->animation.moving=false;e->flash=e->strike=0;e->windup=.01f;
  game.sounds.clear();
  game.update(.02f,{},false);
  check(playCount(game,enemyAttackCue(type))==1,"one attack cue at impact "+slug);
  e=game.findEnemy(id);
  if(!e){check(false,"encounter after attack "+slug);continue;}
  game.sounds.clear();
  game.hit(*e,10000);
  check(playCount(game,enemyHitCue(type))==1&&playCount(game,enemyFallCue(type))==1,"hit and fall cues "+slug);
  check(e->strike>0&&game.findCorpse(id),"fall clock and permanent corpse "+slug);
  game.player=e->pos;
  game.allocationRequired=false;
  check(game.openCorpse(id),"corpse opens its own loot "+slug);
  for(int step=0;step<30;step++)game.update(.02f,{},false);
  check(regional_enemy_art::frame(*e)==regional_enemy_art::FallStart+regional_enemy_art::FallFrames-1,"fall settles "+slug);
  for(Cue cue:{enemyAttackCue(type),enemyHitCue(type),enemyFallCue(type)}){
   for(int variant=1;variant<=3;variant++){
    const std::string stem=AudioSystem::Names[int(cue)];
    const std::string path="assets/audio/"+stem+(variant==1?"":"-v"+std::to_string(variant))+".wav";
    Wave wave=LoadWave(path.c_str());
    check(IsWaveValid(wave)&&wave.sampleRate==48000&&wave.sampleSize==16&&wave.channels==2&&wave.frameCount>9600,"real raylib WAV decode "+path);
    UnloadWave(wave);
   }
  }
 }

 // Encode the previous format with all three fields already visited.
 saves::Snapshot prior;
 auto& world=prior.game;
 world.activateField(WorldLocation::Desert);
 world.activateField(WorldLocation::Volcano);
 world.enterMageTower();
 for(auto* field:{&world.enemies,&world.forestState.enemies,&world.desertState.enemies}){
  field->erase(std::remove_if(field->begin(),field->end(),[](const Enemy& e){return regionalEnemy(e.type);}),field->end());
 }
 auto& oldCorpse=world.forestState.enemies.front();
 oldCorpse.alive=false;oldCorpse.hp=0;oldCorpse.loot[0]=item(Kind::Gold,0,19);
 const int oldId=oldCorpse.id;
 saves::Archive payload(false,{},17),envelope(false);
 payload(prior);
 std::string magic="EMBERFALL-SAVE";
 int version=17;size_t size=payload.bytes.size();uint64_t hash=saves::checksum(payload.bytes);
 envelope(magic,version,size,hash);
 auto migrated=saves::decode(envelope.bytes+payload.bytes);
 check(migrated.game.inTower()&&migrated.game.rng==world.rng&&migrated.game.combatRng==world.combatRng,"old save keeps tower and RNG state");
 for(auto realm:{WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano}){
  migrated.game.activateField(realm);
  int type=realm==WorldLocation::Forest?SporekinType:realm==WorldLocation::Desert?DuneEyeType:CinderhornType;
  check(count(migrated.game,type)==RegionalEncountersPerRealm,"old save receives four of its regional species");
  if(realm==WorldLocation::Forest){
   const Enemy* body=migrated.game.findCorpse(oldId);
   check(body&&body->loot[0].kind==Kind::Gold&&body->loot[0].count==19,"legacy loot survives migration");
  }
 }
 auto bytes=saves::encode(migrated);
 check(saves::encode(saves::decode(bytes))==bytes,"new save round trip does not duplicate encounters");
 check(!audioSystem.available,"audio device remains closed");
 std::ofstream report(output/"report.json");
 report<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks
       <<",\"failures\":"<<failures<<",\"audio_device_opened\":false,\"window_hidden\":true}\n";
 regional_enemy_art::unload();audioSystem.shutdown();sync_present::shutdown();CloseWindow();
 return failures?2:0;
}
