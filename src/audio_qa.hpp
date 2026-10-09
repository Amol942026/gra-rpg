#pragma once
#include <atomic>

// Meter the game's own mixed output, never microphones or other applications.
std::atomic<unsigned long long> audioSamples{0},audioNonzero{0};
std::atomic<float> audioPeak{0};
void meterAudio(void* buffer,unsigned int frames) {
    const float* samples=static_cast<const float*>(buffer);float peak=0;unsigned long long active=0;
    for(unsigned int i=0;i<frames*2;i++){float v=std::abs(samples[i]);peak=std::max(peak,v);if(v>.00001f)active++;}
    audioSamples.fetch_add(frames*2);audioNonzero.fetch_add(active);
    float previous=audioPeak.load();while(previous<peak&&!audioPeak.compare_exchange_weak(previous,peak)){}
}
bool runAudioQA(bool disabled) {
    fs::create_directories("qa-v09");
    std::ofstream report(disabled?"qa-v09/audio-disabled-tests.txt":"qa-v09/audio-tests.txt");
    int checks=0,failures=0;
    auto check=[&](bool ok,const char* label){checks++;if(!ok)failures++;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
    game=Game{};game.props.clear();game.enemies.clear();game.player={15,15};
    if(disabled){
        check(!audioSystem.available,"audio device initialization can be skipped");
        game.startRoll({1,0});game.update(.05f,{},false);audioSystem.update(.05f,game,false,true);
        check(game.player.x>15&&game.sounds.empty(),"silent fallback still moves and drains sound events");
    }else{
        check(audioSystem.available,"audio output device initialized");
        check(audioSystem.loadedEffects==AudioSystem::CueCount,"all effects and voice banks loaded");
        check(audioSystem.musicReady()&&GetMusicTimeLength(audioSystem.music)>100,"streaming score loaded, duration over 100 seconds");
        if(audioSystem.available){
            AttachAudioMixedProcessor(meterAudio);
            for(int cue=0;cue<AudioSystem::CueCount;cue++){
                int before=audioSystem.playedEffects;audioSystem.play(static_cast<Cue>(cue));
                check(audioSystem.playedEffects==before+1,AudioSystem::Names[cue]);
                for(int frame=0;frame<12;frame++){audioSystem.update(1.f/60,game,false,true);WaitTime(1.0/60);}
            }
            check(audioNonzero.load()>32000,"mixer produced real nonzero audio samples");
            check(audioPeak.load()>0&&audioPeak.load()<1,"mixed score and effects stay below digital clipping");
            audioSystem.muted=true;int before=audioSystem.playedEffects;
            game.sound(Cue::Spell,game.player);audioSystem.update(.05f,game,false,true);
            check(audioSystem.playedEffects==before&&game.sounds.empty(),"mute drops events instead of replaying them later");
            for(int i=0;i<40;i++)audioSystem.update(.05f,game,false,true);
            check(audioSystem.effectiveMusicVolume()<.001f,"mute fades music to silence");audioSystem.muted=false;
            audioSystem.effectsVolume=0;audioSystem.play(Cue::Pickup);
            check(audioSystem.playedEffects==before,"effects slider at zero mutes effects");audioSystem.effectsVolume=.72f;
            game.sound(Cue::Swing,game.player);audioSystem.update(.05f,game,true,true);
            check(audioSystem.playedEffects==before&&game.sounds.empty(),"paused world cannot leak queued combat sounds");
            audioSystem.update(.05f,game,true,false);
            check(audioSystem.isSuspended()&&!IsMusicStreamPlaying(audioSystem.music),"focus loss suspends music");
            audioSystem.update(.05f,game,false,true);
            check(!audioSystem.isSuspended()&&IsMusicStreamPlaying(audioSystem.music),"focus regain resumes existing music stream");
            SeekMusicStream(audioSystem.music,GetMusicTimeLength(audioSystem.music)-.2f);
            for(int i=0;i<60;i++){audioSystem.update(1.f/60,game,false,true);WaitTime(1.0/60);}
            check(IsMusicStreamPlaying(audioSystem.music)&&GetMusicTimePlayed(audioSystem.music)<2,"score loops across its actual stream boundary");
            audioSystem.musicVolume=audioSystem.effectsVolume=1;
            for(int i=0;i<AudioSystem::CueCount;i++)audioSystem.play(static_cast<Cue>(i));
            for(int i=0;i<30;i++){audioSystem.update(1.f/60,game,false,true);WaitTime(1.0/60);}
            check(audioPeak.load()<1,"maximum-volume simultaneous effects remain below clipping");
            audioSystem.musicVolume=.42f;audioSystem.effectsVolume=.72f;
            audioSystem.toggleMusic();
            check(!audioSystem.musicEnabled()&&audioSystem.effectsEnabled()&&audioSystem.effectiveMusicVolume()==0,"music button immediately silences only the music channel");
            before=audioSystem.playedEffects;audioSystem.play(Cue::Swing);
            check(audioSystem.playedEffects==before+1,"combat sound still plays with music disabled");
            audioSystem.toggleMusic();audioSystem.toggleEffects();
            before=audioSystem.playedEffects;game.sound(Cue::Spell,game.player);
            audioSystem.update(.05f,game,false,true);
            check(audioSystem.musicEnabled()&&!audioSystem.effectsEnabled()&&audioSystem.effectiveMusicVolume()>0,"effects button leaves the music playing");
            check(audioSystem.playedEffects==before&&game.sounds.empty(),"disabled effects discard queued events without replaying later");
            audioSystem.toggleMusic();
            for(int i=0;i<12;i++){audioSystem.update(1.f/60,game,false,true);WaitTime(1.0/60);}
            audioNonzero.store(0);
            for(int i=0;i<12;i++){audioSystem.update(1.f/60,game,false,true);WaitTime(1.0/60);}
            check(audioNonzero.load()==0,"both HUD switches off produce actual silence including existing effects");
            audioSystem.toggleEffects();audioSystem.toggleMusic();
            check(audioSystem.musicVolume==.42f&&audioSystem.effectsVolume==.72f,"channel switches preserve the user's volume levels");
            DetachAudioMixedProcessor(meterAudio);
            report<<"Mixed samples: "<<audioSamples.load()<<"\nNonzero samples: "<<audioNonzero.load()<<"\nPeak: "<<audioPeak.load()<<'\n';
        }
        AudioSystem settings;settings.musicVolume=.23f;settings.effectsVolume=.61f;settings.muted=true;
        settings.musicMuted=true;settings.effectsMuted=false;
        settings.saveSettings("qa-v09/settings-test.cfg");AudioSystem restored;restored.loadSettings("qa-v09/settings-test.cfg");
        check(std::abs(restored.musicVolume-.23f)<.001f&&std::abs(restored.effectsVolume-.61f)<.001f&&restored.muted,"volume and mute settings round trip");
        check(restored.musicMuted&&!restored.effectsMuted,"independent channel states survive settings round trip");
        {std::ofstream legacy("qa-v09/legacy-settings.cfg");legacy<<"0.31 0.65 1\n";}
        restored.loadSettings("qa-v09/legacy-settings.cfg");
        check(restored.musicVolume==.31f&&restored.effectsVolume==.65f&&restored.muted&&!restored.musicMuted&&!restored.effectsMuted,"old three-field settings load without losing volumes or master mute");
        welcome=help=paused=false;audioOpen=true;mouse={0,0};prepareCanvas(1.2f);
        renderScene();saveTexture(canvas,"qa-v09/audio-settings.png");audioOpen=false;
    }
    // Use exactly the click handler used by the normal HUD, without OS input injection.
    welcome=help=paused=audioOpen=false;dragging={};
    audioSystem.muted=audioSystem.musicMuted=audioSystem.effectsMuted=false;
    auto pointAt=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
    pointAt(audioToggle(0));
    check(clickAudioHud()&&!audioSystem.musicEnabled()&&audioSystem.effectsEnabled()&&!audioOpen,"music HUD click works immediately without opening or pausing a menu");
    check(!worldPointerAllowed(mouse),"audio controls cannot select or drop onto terrain behind HUD");
    pointAt(audioToggle(1));
    check(clickAudioHud()&&!audioSystem.musicEnabled()&&!audioSystem.effectsEnabled(),"effects HUD click is independent of the music switch");
    audioSystem.toggleMute();pointAt(audioToggle(0));clickAudioHud();
    check(audioSystem.musicEnabled()&&!audioSystem.effectsEnabled(),"music button after global mute restores only music");
    audioSystem.toggleMute();pointAt(audioToggle(1));clickAudioHud();
    check(!audioSystem.musicEnabled()&&audioSystem.effectsEnabled(),"effects button after global mute restores only effects");
    paused=true;pointAt(audioToggle(0));clickAudioHud();
    check(paused&&audioSystem.musicEnabled(),"HUD switches also work while paused without resuming gameplay");paused=false;
    pointAt(audioButton());check(clickAudioHud()&&audioOpen,"F2 settings remain available from the HUD");clickAudioHud();
    check(!audioOpen,"settings button can close its own panel");
    mouse={0,0};check(!clickAudioHud(),"pointer outside audio buttons is not consumed");
    if(!disabled){
        game=Game{};audioSystem.toggleMusic();
        for(float factor:{.8f,1.f,1.2f}){
            prepareCanvas(factor);renderScene();saveTexture(canvas,TextFormat("qa-v09/hud-%d.png",int(factor*100)));
        }
        paused=true;renderScene();saveTexture(canvas,"qa-v09/paused-hud.png");paused=false;
    }
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
