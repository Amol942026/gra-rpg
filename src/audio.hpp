#pragma once
#include <array>
#include <fstream>
#include <cmath>
#include <string>
#include "raylib.h"
#include "game.hpp"
#include "startup_log.hpp"
#include "music_blend.hpp"

// Preserve headroom when several enemies, a spell and a fanfare overlap.
// The mixer is stereo float; this callback allocates nothing on the audio thread.
inline void limitEmberMix(void* buffer,unsigned int frames) {
    float* samples=static_cast<float*>(buffer);
    for(unsigned int i=0;i<frames*2;i++){
        float magnitude=std::abs(samples[i]);
        if(magnitude>.8f)samples[i]=std::copysign(.8f+.16f*(1-std::exp(-(magnitude-.8f)/.16f)),samples[i]);
    }
}

class AudioSystem {
public:
    static constexpr int CueCount=int(ember::Cue::Count);
    inline static constexpr const char* Names[CueCount]={
        "step","swing","slime-hit","wolf-hit","bone-hit","stone-hit","enemy-fall",
        "hurt","death","roll","dodge","spell","potion","pickup","equip","drop",
        "level-up","victory","enemy-attack","ui","reject","bow-shot",
        "chest-open","weapon-equip","chitin-hit","fire-hit","beast-attack","stone-attack",
        "magic-attack","venom-cast","copper-wave","obsidian-cleave","fire-breath",
        "bone-attack","chitin-attack","serpent-attack","reptile-attack","wood-attack","armored-attack","blade-attack","heavy-swing",
        "fireball-cast","fireball-impact","teleport-arrival","bow-draw",
        "ice-cast","ice-impact","lightning-cast","lightning-impact","spell-learn",
        "sporekin-attack","sporekin-hit","sporekin-fall",
        "dune-eye-attack","dune-eye-hit","dune-eye-fall",
        "cinderhorn-attack","cinderhorn-hit","cinderhorn-fall","spear-thrust","spear-impact",
        "spear-briar-thrust","spear-briar-impact","spear-oasis-thrust","spear-oasis-impact",
        "spear-basalt-thrust","spear-basalt-impact","spear-dawn-thrust","spear-dawn-impact",
        "power-sword","power-axe","power-dagger","power-spear","power-bow","cinderhorn-charge"
    };
    static constexpr int ScoreCount=4;
    inline static constexpr const char* ScoreNames[ScoreCount]={"echoes-of-the-ruins","desert","volcano","tower"};
    static constexpr float DefaultMusicVolume=.85f,DefaultEffectsVolume=.95f,MasterGain=.9f;
    float musicVolume=DefaultMusicVolume,effectsVolume=DefaultEffectsVolume;
    bool muted=false,musicMuted=false,effectsMuted=false,available=false;
    int loadedEffects=0,loadedVariations=0,loadedScores=0,playedEffects=0;
    Music music{};
private:
    struct Bank {std::array<Sound,3> sources{},voices{};std::array<float,3> gains{};int next=0;double last=-100;};
    std::array<Bank,CueCount> banks{};
    std::array<Music,ScoreCount> scores{};
    MusicBlend musicBlend{};
    bool suspended=false,wasBlocked=false,savePreferences=true,deviceOpened=false;
    float currentMusicVolume=0;
    unsigned variation=0;
    void applyMusicVolume(){for(int i=0;i<ScoreCount;i++)if(IsMusicValid(scores[i]))SetMusicVolume(scores[i],currentMusicVolume*musicBlend.weights[i]);}
public:
    static int scoreFor(const ember::Game& world){return world.inTower()?3:world.inDesert()?1:world.inVolcano()?2:0;}
    void loadSettings(const char* path="audio-settings.cfg") {
        std::ifstream file(path);float m,e;int mute;
        if(file>>m>>e>>mute&&std::isfinite(m)&&std::isfinite(e)){
            musicVolume=std::clamp(m,0.f,1.f);effectsVolume=std::clamp(e,0.f,1.f);muted=mute!=0;
            // The original settings contain only volumes and the master mute.
            int musicMute=0,effectsMute=0;
            musicMuted=effectsMuted=false;
            if(file>>musicMute>>effectsMute){musicMuted=musicMute!=0;effectsMuted=effectsMute!=0;}
        }
    }
    void saveSettings(const char* path="audio-settings.cfg") const {
        std::ofstream file(path);if(file)file<<musicVolume<<' '<<effectsVolume<<' '<<int(muted)<<' '<<int(musicMuted)<<' '<<int(effectsMuted)<<'\n';
    }
    void init(bool enabled=true,bool preferences=true) {
        savePreferences=preferences;
        if(preferences)loadSettings();
        if(!enabled)return;
        startupLog.write("Opening audio device");
        InitAudioDevice();deviceOpened=IsAudioDeviceReady();available=deviceOpened;
        startupLog.write(available?"Audio device ready":"Audio device unavailable");
        if(!available)return; // The game remains playable without an audio device.
        SetMasterVolume(MasterGain);
        AttachAudioMixedProcessor(limitEmberMix);
        for(int i=0;i<CueCount;i++){
            auto& bank=banks[i];std::string stem=std::string("assets/audio/")+Names[i];
            for(int v=0;v<3;v++){
                std::string path=stem+(v?"-v"+std::to_string(v+1):"")+".wav";
                if(FileExists(path.c_str())){bank.sources[v]=LoadSound(path.c_str());if(IsSoundValid(bank.sources[v]))loadedVariations++;}
                Sound source=IsSoundValid(bank.sources[v])?bank.sources[v]:bank.sources[0];
                if(IsSoundValid(source))bank.voices[v]=LoadSoundAlias(source);
            }
            if(!IsSoundValid(bank.sources[0]))continue;
            loadedEffects++;
        }
        startupLog.write("Sound effects ready; loading music");
        for(int i=0;i<ScoreCount;i++){
            std::string path=std::string("assets/audio/")+ScoreNames[i]+".ogg";
            if(!FileExists(path.c_str()))continue;scores[i]=LoadMusicStream(path.c_str());
            if(IsMusicValid(scores[i])){scores[i].looping=true;SetMusicVolume(scores[i],0);loadedScores++;}
        }
        music=scores[0];if(IsMusicValid(music))PlayMusicStream(music);
    }
    void stopEffects() {
        if(!available)return;
        for(auto& bank:banks)for(Sound voice:bank.voices)if(IsSoundValid(voice))StopSound(voice);
    }
    void stopCue(ember::Cue cue) {
        if(!available)return;
        for(Sound voice:banks[int(cue)].voices)if(IsSoundValid(voice))StopSound(voice);
    }
    void play(ember::Cue cue,ember::V position={},ember::V listener={},bool spatial=false) {
        if(!available||!effectsEnabled()||effectsVolume<=0||suspended)return;
        int i=int(cue);if(i<0||i>=CueCount)return;
        auto& bank=banks[i];if(!IsSoundValid(bank.sources[0]))return;
        double now=GetTime();if(now-bank.last<(cue==ember::Cue::Step?.085:.045))return;
        float attenuation=1,pan=.5f;
        if(spatial){
            ember::V delta=position-listener;float d=ember::length(delta);if(d>=12)return;
            attenuation=std::clamp(1-d/12,.05f,1.f);
            // raylib's pan=0 is right; isometric screen X increases with x-y.
            pan=std::clamp(.5f-(delta.x-delta.y)*.045f,.1f,.9f);
        }
        int slot=bank.next;Sound voice=bank.voices[slot];bank.next=(slot+1)%3;
        if(!IsSoundValid(voice))return;
        StopSound(voice);float pitch=1;
        if(cue!=ember::Cue::Ui&&cue!=ember::Cue::Reject&&cue!=ember::Cue::LevelUp&&cue!=ember::Cue::Victory&&cue!=ember::Cue::Pickup&&cue!=ember::Cue::BowDraw){
            variation=variation*1664525u+1013904223u;pitch=.975f+(variation%51)/1000.f;
        }
        SetSoundPitch(voice,pitch);SetSoundPan(voice,pan);
        bank.gains[slot]=attenuation*(cue==ember::Cue::Step?.52f:1.f);
        SetSoundVolume(voice,effectsVolume*bank.gains[slot]);
        PlaySound(voice);bank.last=now;playedEffects++;
    }
    void update(float dt,ember::Game& world,bool blocked,bool focused,bool cinematicArrival=false) {
        if(!available){world.sounds.clear();return;}
        if(blocked&&!wasBlocked)stopEffects();wasBlocked=blocked;
        if(!world.bowDrawing)stopCue(ember::Cue::BowDraw);
        if(!focused&&!suspended){stopEffects();for(auto score:scores)if(IsMusicValid(score))PauseMusicStream(score);suspended=true;}
        if(focused&&suspended){for(int i=0;i<ScoreCount;i++)if(IsMusicValid(scores[i])&&musicBlend.weights[i]>.001f)ResumeMusicStream(scores[i]);suspended=false;}
        int desired=scoreFor(world);if(!IsMusicValid(scores[desired]))desired=0;
        musicBlend.update(dt,desired);music=scores[desired];
        float target=!musicEnabled()||!focused?0:musicVolume*(blocked?.65f:world.dead()?.3f:1.f);
        for(auto& bank:banks)for(int i=0;i<3;i++)if(IsSoundValid(bank.voices[i]))
            SetSoundVolume(bank.voices[i],effectsEnabled()?effectsVolume*bank.gains[i]:0);
        currentMusicVolume+=(target-currentMusicVolume)*std::min(1.f,std::max(0.f,dt)*5);
        applyMusicVolume();
        for(int i=0;i<ScoreCount;i++)if(IsMusicValid(scores[i])&&!suspended){
            if(musicBlend.weights[i]>.001f){if(!IsMusicStreamPlaying(scores[i]))PlayMusicStream(scores[i]);UpdateMusicStream(scores[i]);}
            else if(IsMusicStreamPlaying(scores[i]))StopMusicStream(scores[i]);
        }
        if(focused)for(const auto& event:world.sounds)
            if((event.cue!=ember::Cue::BowDraw||world.bowDrawing)&&(!blocked||event.cue==ember::Cue::LevelUp||event.cue==ember::Cue::Victory||event.cue==ember::Cue::SpellLearn||(cinematicArrival&&event.cue==ember::Cue::TeleportArrival)))
                play(event.cue,{event.x,event.y},world.player,true);
        world.sounds.clear();
    }
    void toggleMute(){muted=!muted;if(muted)stopEffects();}
    bool musicEnabled()const{return !muted&&!musicMuted;}
    bool effectsEnabled()const{return !muted&&!effectsMuted;}
    void toggleMusic(){
        // A channel button can re-enable just that channel after M muted both.
        if(muted){muted=false;musicMuted=false;effectsMuted=true;}
        else musicMuted=!musicMuted;
        if(musicMuted){currentMusicVolume=0;applyMusicVolume();}
    }
    void toggleEffects(){
        if(muted){muted=false;effectsMuted=false;musicMuted=true;currentMusicVolume=0;applyMusicVolume();}
        else effectsMuted=!effectsMuted;
        if(effectsMuted)stopEffects();
    }
    bool musicReady()const{return IsMusicValid(music);}
    float effectiveMusicVolume()const{return currentMusicVolume;}
    bool isSuspended()const{return suspended;}
    void shutdown() {
        if(savePreferences)saveSettings();
        if(!deviceOpened)return;
        stopEffects();
        for(auto score:scores)if(IsMusicValid(score))UnloadMusicStream(score);music={};
        for(auto& bank:banks){
            for(Sound voice:bank.voices)if(IsSoundValid(voice))UnloadSoundAlias(voice);
            for(auto source:bank.sources)if(IsSoundValid(source))UnloadSound(source);
        }
        DetachAudioMixedProcessor(limitEmberMix);
        CloseAudioDevice();available=false;deviceOpened=false;
    }
};
