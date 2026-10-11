#pragma once
#include "../src/intro_story.hpp"
#include <limits>

void introTests(){
    IntroSequence scene;check(!scene.active(),"diagnostics default to gameplay rather than the prologue");
    scene.begin();check(scene.chamber()&&scene.phase==IntroPhase::Opening,"new adventure starts in the observatory");
    check(storyGlyphCount("Żar, świt i łzy")==15,"Polish UTF-8 text counts characters rather than bytes");
    check(storyPrefix("Żółć",1)=="Ż"&&storyPrefix("Żółć",3)=="Żół","typewriter never splits a Polish character");
    check(storyPrefix("Świt",0).empty()&&storyPrefix("Świt",99)=="Świt","typewriter handles empty and complete text");
    scene.advance();scene.update(.05f);scene.advance();check(scene.line==0&&!scene.revealed,"rapid opening key presses cannot skip a speaker");
    for(int i=0;i<4;i++)scene.update(.05f);
    scene.advance();check(scene.line==0&&scene.textComplete(),"first advance reveals the current line without skipping it");
    scene.advance();check(scene.line==1&&!scene.revealed&&scene.age==0,"next advance changes speaker and restarts typewriter");
    float age=scene.age;scene.update(-1);scene.update(std::numeric_limits<float>::quiet_NaN());
    check(scene.age==age,"invalid elapsed time cannot corrupt cinematic state");
    for(float fps:{30.f,60.f,144.f}){
        IntroSequence automatic;automatic.begin();std::array<bool,IntroDialogue.size()> seen{};int arrivals=0;float elapsed=0;
        while(automatic.active()&&elapsed<100){
            if(automatic.phase==IntroPhase::Dialogue)seen[automatic.line]=true;
            automatic.update(1/fps);elapsed+=1/fps;arrivals+=automatic.takeArrival();
        }
        check(!automatic.active(),"automatic dialogue and teleport finish at every supported frame rate");
        check(std::all_of(seen.begin(),seen.end(),[](bool value){return value;}),"automatic prologue presents every dialogue turn");
        check(arrivals==1&&!automatic.takeArrival(),"teleport emits exactly one forest arrival");
        check(elapsed>50&&elapsed<85,"automatic dialogue leaves enough time to read without an endless scene");
    }
    for(int line:{0,5,11}){
        scene.begin();scene.enter(IntroPhase::Dialogue);scene.line=line;scene.skip();
        check(scene.phase==IntroPhase::Charging&&scene.line==IntroDialogue.size()-1,"skip still plays the mage's final words and teleport");
        int arrivals=0;for(int i=0;i<300;i++){scene.skip();scene.advance();scene.update(1.f/60);arrivals+=scene.takeArrival();}
        check(!scene.active()&&arrivals==1,"repeated skip/advance cannot duplicate or interrupt the teleport");
    }
    scene.begin();check(scene.active()&&scene.line==0&&!scene.arrivalPending,"restarting an adventure resets the entire prologue");
}
