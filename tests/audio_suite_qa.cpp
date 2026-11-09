// Executes the real AudioSystem against an in-memory output sink. LoadWave is
// raylib's real decoder; every device/transport function is replaced locally.
// This cannot initialize hardware, open a window or emit audio.
#include "raylib.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
namespace offline {
struct Voice {Wave wave{};bool alias=false,playing=false;float volume=0,pitch=1,pan=.5f;};
struct Score {Wave wave{};bool playing=false;float volume=0;};
std::vector<Voice*> voices;std::vector<Score*> scores;double clock=0;bool device=false;int starts=0,plays=0,loads=0,updates=0;float masterGain=0;
Voice* voice(Sound s){return reinterpret_cast<Voice*>(s.stream.buffer);}
Score* score(Music m){return static_cast<Score*>(m.ctxData);}
void init(){device=true;starts++;}bool ready(){return device;}void close(){device=false;}
void master(float gain){masterGain=gain;}void processor(AudioCallback){}
Sound load(const char* path){Wave w=LoadWave(path);if(!w.data)return {};auto* v=new Voice;v->wave=w;voices.push_back(v);Sound s{};s.stream.buffer=reinterpret_cast<rAudioBuffer*>(v);s.frameCount=w.frameCount;loads++;return s;}
bool valid(Sound s){return s.frameCount&&s.stream.buffer;}
Sound alias(Sound src){auto* v=new Voice(*voice(src));v->alias=true;voices.push_back(v);Sound s=src;s.stream.buffer=reinterpret_cast<rAudioBuffer*>(v);return s;}
void unload(Sound s){auto* v=voice(s);if(!v->alias)UnloadWave(v->wave);voices.erase(std::find(voices.begin(),voices.end(),v));delete v;}
void play(Sound s){voice(s)->playing=true;plays++;}void stop(Sound s){voice(s)->playing=false;}
void volume(Sound s,float f){voice(s)->volume=f;}void pitch(Sound s,float f){voice(s)->pitch=f;}void pan(Sound s,float f){voice(s)->pan=f;}
Music loadMusic(const char* path){Wave w=LoadWave(path);if(!w.data)return {};auto* s=new Score;s->wave=w;scores.push_back(s);Music m{};m.ctxData=s;m.frameCount=w.frameCount;return m;}
bool validMusic(Music m){return m.ctxData&&m.frameCount;}
void unloadMusic(Music m){auto* s=score(m);UnloadWave(s->wave);scores.erase(std::find(scores.begin(),scores.end(),s));delete s;}
void playMusic(Music m){score(m)->playing=true;}void stopMusic(Music m){score(m)->playing=false;}
bool playing(Music m){return score(m)->playing;}void musicVolume(Music m,float f){score(m)->volume=f;}
void updateMusic(Music){updates++;}double time(){return clock;}
}
#define InitAudioDevice offline::init
#define IsAudioDeviceReady offline::ready
#define CloseAudioDevice offline::close
#define SetMasterVolume offline::master
#define AttachAudioMixedProcessor offline::processor
#define DetachAudioMixedProcessor offline::processor
#define LoadSound offline::load
#define LoadSoundAlias offline::alias
#define IsSoundValid offline::valid
#define UnloadSound offline::unload
#define UnloadSoundAlias offline::unload
#define PlaySound offline::play
#define StopSound offline::stop
#define SetSoundVolume offline::volume
#define SetSoundPitch offline::pitch
#define SetSoundPan offline::pan
#define LoadMusicStream offline::loadMusic
#define IsMusicValid offline::validMusic
#define UnloadMusicStream offline::unloadMusic
#define PlayMusicStream offline::playMusic
#define StopMusicStream offline::stopMusic
#define PauseMusicStream offline::stopMusic
#define ResumeMusicStream offline::playMusic
#define IsMusicStreamPlaying offline::playing
#define SetMusicVolume offline::musicVolume
#define UpdateMusicStream offline::updateMusic
#define GetTime offline::time
#include "../src/audio.hpp"
#undef IsAudioDeviceReady

int main(int argc,char** argv){
 if(argc!=4||std::string(argv[1])!="--no-audio"||std::string(argv[3])!="--background-test")return 3;
 namespace fs=std::filesystem;fs::current_path(fs::absolute(argv[0]).parent_path());fs::path root=argv[2];if(fs::exists(root))return 4;fs::create_directories(root);
 SetTraceLogLevel(LOG_ERROR);std::ofstream log(root/"checks.txt");int checks=0,failures=0;
 auto check=[&](bool b,const std::string& label){checks++;failures+=!b;log<<(b?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 AudioSystem audio;audio.init(true,false);ember::Game world;
 check(!IsAudioDeviceReady(),"real raylib hardware output stays closed");
 check(audio.loadedEffects==AudioSystem::CueCount&&audio.loadedVariations==AudioSystem::CueCount*3&&audio.loadedScores==4,"real AudioSystem preloads every effect, variation and regional score through the offline sink");
 check(offline::loads==AudioSystem::CueCount*3&&offline::voices.size()==size_t(AudioSystem::CueCount*6)&&offline::scores.size()==4,"source and alias ownership is bounded");
 check(audio.musicEnabled()&&audio.effectsEnabled()&&audio.musicVolume==.85f&&audio.effectsVolume==.95f&&offline::masterGain==.9f,"fresh defaults enable both channels with louder music, effects and master gain");
 for(auto* v:offline::voices)if(!v->alias){
  Wave w=v->wave;check(w.channels==2&&w.sampleRate==48000&&w.sampleSize==16&&w.frameCount>4000,"raylib fully decodes each PCM16 stereo effect");
  auto* samples=static_cast<short*>(w.data);int peak=0;for(unsigned i=0;i<w.frameCount*2;i++)peak=std::max(peak,std::abs(int(samples[i])));
  check(peak>100&&peak<32000,"decoded effect is non-silent with headroom");
 }
 for(auto* s:offline::scores)check(s->wave.channels==2&&s->wave.sampleRate==48000&&s->wave.frameCount>48000*80,"raylib decodes every actual Vorbis track and complete musical phrase");
 for(int cue=0;cue<AudioSystem::CueCount;cue++){
  std::vector<offline::Voice*> selected;
  for(int v=0;v<3;v++){for(auto* p:offline::voices)p->playing=false;offline::clock+=.2;int before=offline::plays;audio.play(static_cast<ember::Cue>(cue));
   check(offline::plays==before+1,"actual cue routes into one voice / "+std::string(AudioSystem::Names[cue]));
   for(auto* p:offline::voices)if(p->playing)selected.push_back(p);
  }
  check(selected.size()==3&&selected[0]!=selected[1]&&selected[1]!=selected[2],"successive events select three authored variants / "+std::string(AudioSystem::Names[cue]));
 }
 for(auto realm:{ember::WorldLocation::Desert,ember::WorldLocation::Volcano,ember::WorldLocation::MageTower,ember::WorldLocation::Forest}){
  world.location=realm;int desired=AudioSystem::scoreFor(world);int before=offline::loads;
  for(int frame=0;frame<100;frame++){offline::clock+=.05;audio.update(.05f,world,false,true);}
  bool quiet=true;for(int i=0;i<4;i++)if(i!=desired)quiet&=offline::scores[i]->volume<.001f&&!offline::scores[i]->playing;
  check(offline::score(audio.music)==offline::scores[desired]&&offline::scores[desired]->playing&&offline::scores[desired]->volume>.4f&&quiet,"region selects its score, fades in and stops the previous score");
  check(offline::loads==before,"realm transition does no sound-file I/O");
 }
 audio.toggleMusic();bool silent=true;for(auto* s:offline::scores)silent&=s->volume==0;check(silent&&audio.effectsEnabled(),"music toggle immediately silences every crossfading score alone");audio.toggleMusic();
 int played=audio.playedEffects;audio.toggleEffects();world.sound(ember::Cue::ChestOpen,world.player);audio.update(.05f,world,false,true);
 check(audio.playedEffects==played&&world.sounds.empty()&&audio.effectiveMusicVolume()>0,"disabled effects discard events while music continues");audio.toggleEffects();
 world.sound(ember::Cue::Swing,world.player);audio.update(.05f,world,true,true);check(audio.playedEffects==played&&world.sounds.empty(),"pause drains queued combat without replay");
 for(int frame=0;frame<80;frame++)audio.update(.05f,world,true,true);check(std::abs(audio.effectiveMusicVolume()-audio.musicVolume*.65f)<.001f,"menu and pause retain sixty-five percent music instead of excessively ducking it");
 audio.update(.05f,world,false,false);silent=true;for(auto* s:offline::scores)silent&=!s->playing;check(audio.isSuspended()&&silent,"focus loss pauses every active score");
 audio.update(.05f,world,false,true);check(!audio.isSuspended()&&offline::playing(audio.music),"focus return resumes the existing selected score");
 float mix[]={-9,-1,-.5f,0,.5f,1,9,0};limitEmberMix(mix,4);check(std::abs(mix[0])<.961f&&mix[4]==.5f&&mix[2]==-.5f,"real stereo mix limiter bounds overlapping peaks while preserving quieter dynamics");
 // A story's reveal blocks combat input, but its arrival burst must be heard.
 offline::clock+=2;played=audio.playedEffects;world.sound(ember::Cue::TeleportArrival,world.player);world.sound(ember::Cue::FireballCast,world.player);
 audio.update(.05f,world,true,true,true);check(audio.playedEffects==played+1&&world.sounds.empty(),"cinematic arrival alone crosses the story input block, without leaking combat");
 offline::clock+=2;played=audio.playedEffects;world.sound(ember::Cue::TeleportArrival,world.player);audio.update(.05f,world,true,true,false);
 check(audio.playedEffects==played&&world.sounds.empty(),"actual pause drops arrival instead of replaying it later");
 offline::clock+=2;world.sound(ember::Cue::TeleportArrival,world.player);audio.update(.05f,world,true,false,true);
 check(audio.playedEffects==played&&world.sounds.empty(),"unfocused cinematic cannot emit arrival audio");
 audio.update(.05f,world,false,true);world.bowDrawing=true;world.sound(ember::Cue::BowDraw,world.player);offline::clock+=2;played=audio.playedEffects;audio.update(.05f,world,false,true);
 auto bowPlaying=[](){int active=0;for(auto* p:offline::voices)if(p->alias&&p->playing&&p->wave.frameCount==67200)active++;return active;};
 check(audio.playedEffects==played+1&&bowPlaying()==1,"real bow draw event starts exactly one authored 1.4-second tension voice");
 world.cancelBow();audio.update(.05f,world,false,true);check(bowPlaying()==0,"bow cancellation stops the tension voice immediately");
 world.bowDrawing=true;world.sound(ember::Cue::BowDraw,world.player);world.cancelBow();played=audio.playedEffects;audio.update(.05f,world,false,true);
 check(audio.playedEffects==played&&bowPlaying()==0,"press and release in one update cannot start stale tension audio");
 world.bowDrawing=true;world.sound(ember::Cue::BowDraw,world.player);offline::clock+=2;audio.update(.05f,world,false,true);audio.update(.05f,world,true,true);
 check(bowPlaying()==0,"opening pause stops an active bow tension sound");world.cancelBow();
 offline::clock+=2;played=audio.playedEffects;world.sound(ember::Cue::SpellLearn,world.player);world.sound(ember::Cue::IceCast,world.player);audio.update(.05f,world,true,true);
 check(audio.playedEffects==played+1&&world.sounds.empty(),"paid lesson confirmation can sound in its menu while combat cues remain blocked");
 audio.toggleEffects();offline::clock+=2;played=audio.playedEffects;world.sound(ember::Cue::SpellLearn,world.player);audio.update(.05f,world,true,true);
 check(audio.playedEffects==played&&world.sounds.empty(),"muted lesson confirmations are discarded instead of replayed after closing the menu");audio.toggleEffects();
 AudioSystem disabled;int starts=offline::starts;disabled.init(false,false);world.sound(ember::Cue::Roll,world.player);disabled.update(.05f,world,false,true);disabled.shutdown();check(offline::starts==starts&&!disabled.available&&world.sounds.empty(),"no-audio mode skips the output sink and all preloading");
 audio.shutdown();check(!IsAudioDeviceReady()&&!offline::device&&offline::voices.empty()&&offline::scores.empty(),"shutdown releases all sources and aliases once with real hardware still closed");
 std::ofstream out(root/"summary.json");out<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"effects\":"<<AudioSystem::CueCount<<",\"variants\":"<<AudioSystem::CueCount*3<<",\"music_tracks\":4,\"real_raylib_decode\":true,\"transport\":\"offline in-memory sink\",\"window_created\":false,\"audio_device_opened\":false,\"audio_played\":false}";
 std::cout<<"Offline audio integration: "<<checks<<" checks, "<<failures<<" failures\n";return failures?2:0;
}
