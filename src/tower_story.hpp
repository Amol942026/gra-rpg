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
    {true,"Obawiam się, że drugi strażnik nie powita cię serdeczniej. Żywa Wydma strzeże drugiej części w Morzu Piasków. Gdy będziesz gotowy, wybierz podróż. W miejscu przybycia zostawię ci bramę powrotną do wieży."}
}};
inline constexpr std::array<StoryLine,6> DesertReturnDialogue={{{false,"Eldrinie, Żywa Wydma rozsypała się w moich rękach. Mam drugą część Serca Świtu."},{true,"Dwie części! Nie spodziewałem się, że przebijesz się przez to morze rozżarzonego piasku tak szybko. Dobrze się spisałeś."},{false,"Ta wydma naprawdę chodziła. I nie podobało jej się, że pożyczyłem jej serce."},{true,"Teraz jej moc służy Pieczęci. Trzy części wciąż czekają na odnalezienie, ale odzyskane światło jest bezpieczne."},{false,"Zanim ruszę dalej, chcę odpocząć i sprawdzić zdobyty ekwipunek."},{true,"Źródło odnowy jest do twojej dyspozycji. Gdy odpoczniesz, porozmawiaj ze mną — otworzę drogę do Kaldery Popiołu, gdzie czeka trzeci strażnik. Możesz też wrócić na pustynię po pozostawiony łup."}}};
inline constexpr std::array<StoryLine,3> DesertCompletedTalk={{{true,"Druga część jest bezpieczna. Żywa Wydma już nie podniesie się z piasku."},{false,"Może jeszcze wrócę do oazy. Zostało tam kilka miejsc do sprawdzenia."},{true,"Otworzę przejście do Morza Piasków, gdy wybierzesz podróż. Możesz wrócić przez Bramę Eldrina."}}};
inline constexpr std::array<StoryLine,4> VolcanoReturnDialogue={{{false,"Varkhor pokonany. Trzecia część Serca Świtu przetrwała ogień kaldery."},{true,"Trzy blaski połączyły się z Pieczęcią. Zostały dwa, Wędrowcze. Dziś nawet wulkan oddał nam swoją nadzieję."},{false,"Odpocznę w wieży. W kuźniach zostały jeszcze rzeczy warte obejrzenia."},{true,"Możesz wrócić do Kaldery Popiołu albo Morza Piasków. Bramy pozostaną otwarte."}}};
inline constexpr std::array<StoryLine,3> VolcanoTalkDialogue={{{true,"Żywa Wydma została pokonana. Jej upadek otworzył drogę do trzeciej krainy — Kaldery Popiołu."},{false,"Za piaskami czeka ogień?"},{true,"Bazaltowe mosty przeprowadzą cię nad lawą. Varkhor, Serce Kaldery, strzeże trzeciej części. Uważaj na oznaczone miejsca erupcji. Zostawię ci też bramę powrotu, a kapliczki odnowią twoje siły."}}};

enum class TowerPhase {Finished,FadeOut,Opening,Dialogue,Closing,Reveal,Choice,TravelOut,TravelIn};
struct TowerSequence {
    TowerPhase phase=TowerPhase::Finished;
    size_t line=0;float age=0,time=0;
    bool returning=false,revealed=false,arrivalPending=false,completionPending=false,offerDesert=false,desertArrivalPending=false;int fragments=1;bool offerVolcano=false,travelVolcano=false;
    static constexpr float FadeTime=.9f,OpeningTime=1.4f,ClosingTime=.45f,RevealTime=.7f;
    void enter(TowerPhase next){phase=next;age=0;revealed=false;}
    void beginReturn(int parts=1){*this={};fragments=parts;returning=true;enter(TowerPhase::FadeOut);}
    void beginTalk(bool offer=false,int parts=1,bool volcano=false){*this={};offerDesert=offer;offerVolcano=volcano;fragments=parts;enter(TowerPhase::Dialogue);}
    bool chooseVolcano(){if(phase!=TowerPhase::Choice||!offerVolcano)return false;travelVolcano=true;enter(TowerPhase::TravelOut);return true;}
    bool chooseDesert(){if(phase!=TowerPhase::Choice||!offerDesert)return false;travelVolcano=false;enter(TowerPhase::TravelOut);return true;}
    bool takeDesertArrival(){bool value=desertArrivalPending;desertArrivalPending=false;return value;}
    bool active()const{return phase!=TowerPhase::Finished;}
    bool chamber()const{return returning&&(phase==TowerPhase::Opening||phase==TowerPhase::Dialogue||phase==TowerPhase::Closing);}
    size_t lineCount()const{if(fragments>=3)return VolcanoReturnDialogue.size();if(!returning&&offerVolcano)return VolcanoTalkDialogue.size();return returning?(fragments>=2?DesertReturnDialogue.size():TowerReturnDialogue.size()):(fragments>=2?DesertCompletedTalk.size():TowerTalkDialogue.size());}
    const StoryLine& current()const{if(fragments>=3)return VolcanoReturnDialogue[line];if(!returning&&offerVolcano)return VolcanoTalkDialogue[line];return returning?(fragments>=2?DesertReturnDialogue[line]:TowerReturnDialogue[line]):(fragments>=2?DesertCompletedTalk[line]:TowerTalkDialogue[line]);}
    float readingTime()const{return std::max(5.f,storyGlyphCount(current().text)/IntroSequence::GlyphsPerSecond+2.8f);}
    bool textComplete()const{return revealed||age*IntroSequence::GlyphsPerSecond>=storyGlyphCount(current().text);}
    std::string visibleText()const{return revealed?current().text:storyPrefix(current().text,size_t(age*IntroSequence::GlyphsPerSecond));}
    void nextLine(){if(line+1<lineCount()){line++;enter(TowerPhase::Dialogue);}else enter(returning?TowerPhase::Closing:(offerDesert||offerVolcano)?TowerPhase::Choice:TowerPhase::Finished);}
    void advance(){
        if(phase==TowerPhase::Opening){enter(TowerPhase::Dialogue);return;}
        if(phase!=TowerPhase::Dialogue||age<.15f)return;
        if(!textComplete())revealed=true;else nextLine();
    }
    void skip(){if(phase==TowerPhase::Opening||phase==TowerPhase::Dialogue||phase==TowerPhase::Choice)enter(returning?TowerPhase::Closing:TowerPhase::Finished);}
    void update(float dt){
        if(!active()||!std::isfinite(dt)||dt<=0)return;
        age+=std::min(dt,.1f);time+=std::min(dt,.1f);
        if(phase==TowerPhase::FadeOut&&age>=FadeTime){enter(TowerPhase::Opening);arrivalPending=true;}
        else if(phase==TowerPhase::Opening&&age>=OpeningTime)enter(TowerPhase::Dialogue);
        else if(phase==TowerPhase::Dialogue&&age>=readingTime())nextLine();
        else if(phase==TowerPhase::TravelOut&&age>=FadeTime){enter(TowerPhase::TravelIn);desertArrivalPending=true;}
        else if(phase==TowerPhase::TravelIn&&age>=1.2f)enter(TowerPhase::Finished);
        else if(phase==TowerPhase::Closing&&age>=ClosingTime)enter(TowerPhase::Reveal);
        else if(phase==TowerPhase::Reveal&&age>=RevealTime){enter(TowerPhase::Finished);completionPending=true;}
    }
    bool takeArrival(){bool value=arrivalPending;arrivalPending=false;return value;}
    bool takeCompletion(){bool value=completionPending;completionPending=false;return value;}
    float fade()const{
        if(phase==TowerPhase::TravelOut)return storyEase(age/FadeTime);
        if(phase==TowerPhase::TravelIn)return 1-storyEase(age/1.2f);
        if(phase==TowerPhase::FadeOut)return storyEase(age/FadeTime);
        if(phase==TowerPhase::Opening)return 1-storyEase(age/.8f);
        if(phase==TowerPhase::Closing)return storyEase(age/ClosingTime);
        if(phase==TowerPhase::Reveal)return 1-storyEase(age/RevealTime);
        return 0;
    }
};
}
