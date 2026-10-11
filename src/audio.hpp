#pragma once
#include <array>
#include <fstream>
#include <cmath>
#include <string>
#include "raylib.h"
#include "game.hpp"
#include "startup_log.hpp"

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
        "level-up","victory","enemy-attack","ui","reject","bow-shot"
    };
    float musicVolume=.42f,effectsVolume=.72f;
    bool muted=false,musicMuted=false,effectsMuted=false,available=false;
    int loadedEffects=0,playedEffects=0;
    Music music{};
private:
    struct Bank {Sound source{};std::array<Sound,3> voices{};std::array<float,3> gains{};int next=0;double last=-100;};
    std::array<Bank,CueCount> banks{};
    bool suspended=false,wasBlocked=false,savePreferences=true,deviceOpened=false;
    float currentMusicVolume=0;
    unsigned variation=0;
public:
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
        SetMasterVolume(.72f);
        AttachAudioMixedProcessor(limitEmberMix);
        for(int i=0;i<CueCount;i++){
            auto& bank=banks[i];std::string path=std::string("assets/audio/")+Names[i]+".wav";
            if(!FileExists(path.c_str()))continue;
            bank.source=LoadSound(path.c_str());if(!IsSoundValid(bank.source))continue;
            for(auto& voice:bank.voices)voice=LoadSoundAlias(bank.source);
            loadedEffects++;
        }
        startupLog.write("Sound effects ready; loading music");
        if(FileExists("assets/audio/echoes-of-the-ruins.ogg")){
            music=LoadMusicStream("assets/audio/echoes-of-the-ruins.ogg");
            if(IsMusicValid(music)){music.looping=true;SetMusicVolume(music,0);PlayMusicStream(music);}
        }
    }
    void stopEffects() {
        if(!available)return;
        for(auto& bank:banks)for(Sound voice:bank.voices)if(IsSoundValid(voice))StopSound(voice);
    }
    void play(ember::Cue cue,ember::V position={},ember::V listener={},bool spatial=false) {
        if(!available||!effectsEnabled()||effectsVolume<=0||suspended)return;
        int i=int(cue);if(i<0||i>=CueCount)return;
        auto& bank=banks[i];if(!IsSoundValid(bank.source))return;
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
        if(i<=int(ember::Cue::Roll)||cue==ember::Cue::EnemyAttack){
            variation=variation*1664525u+1013904223u;pitch=.94f+(variation%121)/1000.f;
        }
        SetSoundPitch(voice,pitch);SetSoundPan(voice,pan);
        bank.gains[slot]=attenuation*(cue==ember::Cue::Step?.52f:1.f);
        SetSoundVolume(voice,effectsVolume*bank.gains[slot]);
        PlaySound(voice);bank.last=now;playedEffects++;
    }
    void update(float dt,ember::Game& world,bool blocked,bool focused) {
        if(!available){world.sounds.clear();return;}
        if(blocked&&!wasBlocked)stopEffects();wasBlocked=blocked;
        if(!focused&&!suspended){stopEffects();if(IsMusicValid(music))PauseMusicStream(music);suspended=true;}
        if(focused&&suspended){if(IsMusicValid(music))ResumeMusicStream(music);suspended=false;}
        float target=!musicEnabled()||!focused?0:musicVolume*(blocked?.42f:world.dead()?.3f:1.f);
        for(auto& bank:banks)for(int i=0;i<3;i++)if(IsSoundValid(bank.voices[i]))
            SetSoundVolume(bank.voices[i],effectsEnabled()?effectsVolume*bank.gains[i]:0);
        currentMusicVolume+=(target-currentMusicVolume)*std::min(1.f,std::max(0.f,dt)*5);
        if(IsMusicValid(music)){
            SetMusicVolume(music,currentMusicVolume);
            if(!suspended)UpdateMusicStream(music);
        }
        if(focused)for(const auto& event:world.sounds)
            if(!blocked||event.cue==ember::Cue::LevelUp||event.cue==ember::Cue::Victory)
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
        if(musicMuted){currentMusicVolume=0;if(musicReady())SetMusicVolume(music,0);}
    }
    void toggleEffects(){
        if(muted){muted=false;effectsMuted=false;musicMuted=true;currentMusicVolume=0;if(musicReady())SetMusicVolume(music,0);}
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
        if(IsMusicValid(music))UnloadMusicStream(music);
        for(auto& bank:banks){
            for(Sound voice:bank.voices)if(IsSoundValid(voice))UnloadSoundAlias(voice);
            if(IsSoundValid(bank.source))UnloadSound(bank.source);
        }
        DetachAudioMixedProcessor(limitEmberMix);
        CloseAudioDevice();available=false;deviceOpened=false;
    }
};
