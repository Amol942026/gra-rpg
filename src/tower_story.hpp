#pragma once
#include "intro_story.hpp"

namespace ember {
inline constexpr std::array<StoryLine,8> TowerReturnDialogue={{
    {false,"Eldrinie! Kamienny Strażnik padł. Zdobyłem pierwszą część Serca Świtu. Spójrz — jeszcze bije w niej światło."},
    {true,"Wróciłeś... i przyniosłeś nadzieję. Dobra robota, Wędrowcze. Wielu odwróciłoby się na widok tego strażnika. Ty wytrwałeś."},
    {false,"Nie było łatwo. Przez chwilę sądziłem, że jego kamienna pięść odeśle mnie tutaj w kawałkach."},
    {true,"A jednak stoisz przede mną. Pierwsza z pięciu części jest bezpieczna. Czuję, jak jej blask osłabia szczeliny, przez które próbują przedostać się demony."},
    {false,"Czy to wystarczy, by powstrzymać inwazję?"},
    {true,"Jeszcze nie. Zostały cztery części i czterej strażnicy. Ale dziś udowodniłeś, że Pieczęć Popiołu nie musi zgasnąć."},
    {false,"Daj mi chwilę na oddech. Potem opowiesz mi, dokąd prowadzi następna droga."},
    {true,"Oczywiście. Moja wieża jest teraz twoim schronieniem. Gdy będziesz gotowy, porozmawiaj ze mną — teleportuję cię do drugiej krainy. Odpocznij, zasłużyłeś."}
}};
inline constexpr std::array<StoryLine,5> TowerTalkDialogue={{
    {true,"Dobrze cię widzieć, Wędrowcze. Jak się czujesz po walce z Kamiennym Strażnikiem?"},
    {false,"Trochę poobijany, ale gotów usłyszeć, co nas czeka. Część Serca Świtu wciąż rozgrzewa mi dłoń."},
    {true,"Strzeż jej. Każda odzyskana część przybliża nas do zamknięcia bramy demonów. Zostały jeszcze cztery."},
    {false,"A kolejna wyprawa? Będzie równie przyjemna jak spotkanie z kamiennym olbrzymem?"},
    {true,"Obawiam się, że drugi strażnik nie powita cię serdeczniej. Przygotuję przejście do następnej krainy. Tymczasem odpocznij i sprawdź wyposażenie. Zawsze możesz ze mną porozmawiać."}
}};
enum class TowerPhase {Finished,FadeOut,Opening,Dialogue,Closing,Reveal};
struct TowerSequence {
    TowerPhase phase=TowerPhase::Finished;
    size_t line=0;float age=0,time=0;
    bool returning=false,revealed=false,arrivalPending=false,completionPending=false;
    static constexpr float FadeTime=.9f,OpeningTime=1.4f,ClosingTime=.45f,RevealTime=.7f;
    void enter(TowerPhase next){phase=next;age=0;revealed=false;}
    void beginReturn(){*this={};returning=true;enter(TowerPhase::FadeOut);}
    void beginTalk(){*this={};enter(TowerPhase::Dialogue);}
    bool active()const{return phase!=TowerPhase::Finished;}
    bool chamber()const{return returning&&(phase==TowerPhase::Opening||phase==TowerPhase::Dialogue||phase==TowerPhase::Closing);}
    size_t lineCount()const{return returning?TowerReturnDialogue.size():TowerTalkDialogue.size();}
    const StoryLine& current()const{return returning?TowerReturnDialogue[line]:TowerTalkDialogue[line];}
    float readingTime()const{return std::max(5.f,storyGlyphCount(current().text)/IntroSequence::GlyphsPerSecond+2.8f);}
    bool textComplete()const{return revealed||age*IntroSequence::GlyphsPerSecond>=storyGlyphCount(current().text);}
    std::string visibleText()const{return revealed?current().text:storyPrefix(current().text,size_t(age*IntroSequence::GlyphsPerSecond));}
    void nextLine(){if(line+1<lineCount()){line++;enter(TowerPhase::Dialogue);}else enter(returning?TowerPhase::Closing:TowerPhase::Finished);}
    void advance(){
        if(phase==TowerPhase::Opening){enter(TowerPhase::Dialogue);return;}
        if(phase!=TowerPhase::Dialogue||age<.15f)return;
        if(!textComplete())revealed=true;else nextLine();
    }
    void skip(){if(phase==TowerPhase::Opening||phase==TowerPhase::Dialogue)enter(returning?TowerPhase::Closing:TowerPhase::Finished);}
    void update(float dt){
        if(!active()||!std::isfinite(dt)||dt<=0)return;
        age+=std::min(dt,.1f);time+=std::min(dt,.1f);
        if(phase==TowerPhase::FadeOut&&age>=FadeTime){enter(TowerPhase::Opening);arrivalPending=true;}
        else if(phase==TowerPhase::Opening&&age>=OpeningTime)enter(TowerPhase::Dialogue);
        else if(phase==TowerPhase::Dialogue&&age>=readingTime())nextLine();
        else if(phase==TowerPhase::Closing&&age>=ClosingTime)enter(TowerPhase::Reveal);
        else if(phase==TowerPhase::Reveal&&age>=RevealTime){enter(TowerPhase::Finished);completionPending=true;}
    }
    bool takeArrival(){bool value=arrivalPending;arrivalPending=false;return value;}
    bool takeCompletion(){bool value=completionPending;completionPending=false;return value;}
    float fade()const{
        if(phase==TowerPhase::FadeOut)return storyEase(age/FadeTime);
        if(phase==TowerPhase::Opening)return 1-storyEase(age/.8f);
        if(phase==TowerPhase::Closing)return storyEase(age/ClosingTime);
        if(phase==TowerPhase::Reveal)return 1-storyEase(age/RevealTime);
        return 0;
    }
};
}
