#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <string_view>

namespace ember {
enum class IntroPhase { Finished, Opening, Dialogue, Charging, Departing, Arriving };
struct StoryLine {bool mage;const char* text;};
inline constexpr std::array<StoryLine,12> IntroDialogue={{
    {true,"Wędrowcze... dobrze, że dotarłeś. Spójrz na niebo. Te czerwone blizny nie są zwiastunem burzy."},
    {false,"Widziałem je nad wioską. Zwierzęta uciekają z lasu, a nocą ktoś woła spośród ruin. Co się dzieje?"},
    {true,"Pęka Pieczęć Popiołu. Po drugiej stronie czeka armia demonów. Gdy brama się otworzy, żaden mur nie ochroni naszego świata."},
    {false,"Jesteś magiem, Eldrinie. Nie możesz jej zamknąć?"},
    {true,"Moja magia jedynie opóźnia ich nadejście. Potrzebujemy Serca Świtu — artefaktu, który kiedyś oddzielił nasz świat od Otchłani."},
    {false,"Powiedz, gdzie go znajdę. Jeśli istnieje choć cień szansy, spróbuję."},
    {true,"Serce rozbito na pięć części i ukryto w odległych krainach. Każdej strzeże osobny strażnik. Musisz zdobyć wszystkie pięć i złączyć je ponownie."},
    {false,"Pięciu strażników przeciw jednemu wędrowcowi... Mam nadzieję, że nie wybrałeś mnie tylko dlatego, że byłem najbliżej."},
    {true,"Wybrałem cię, bo mimo strachu przyszedłeś pomóc. Pierwszej części strzeże Kamienny Strażnik, w północnej części Lasu Mchu."},
    {false,"Zacznę od lasu. A kiedy wrócę, będziesz mi winien długą opowieść... i porządny posiłek."},
    {true,"Umowa stoi. Otworzę ci drogę. Pamiętaj: nie musisz dziś ocalić całego świata. Musisz tylko zrobić pierwszy krok."},
    {true,"Ruszaj więc"}
}};

// Reveal complete UTF-8 characters, including Polish letters, never half a glyph.
inline size_t storyGlyphCount(std::string_view text){
    size_t count=0;for(unsigned char c:text)if((c&0xc0)!=0x80)count++;return count;
}
inline std::string storyPrefix(std::string_view text,size_t glyphs){
    size_t at=0,count=0;
    while(at<text.size()&&count<glyphs){at++;while(at<text.size()&&(static_cast<unsigned char>(text[at])&0xc0)==0x80)at++;count++;}
    return std::string(text.substr(0,at));
}
inline float storyEase(float t){t=std::clamp(t,0.f,1.f);return t*t*(3-2*t);}
struct IntroSequence {
    IntroPhase phase=IntroPhase::Finished;
    size_t line=0;
    float age=0,time=0;
    bool revealed=false,arrivalPending=false;
    static constexpr float OpeningTime=1.8f,ChargeTime=1.6f,DepartTime=.9f,ArrivalTime=2.f,GlyphsPerSecond=48.f;
    void begin(){phase=IntroPhase::Opening;line=0;age=time=0;revealed=arrivalPending=false;}
    bool active()const{return phase!=IntroPhase::Finished;}
    bool chamber()const{return active()&&phase!=IntroPhase::Arriving;}
    const StoryLine& current()const{return IntroDialogue[line];}
    float readingTime()const{return std::max(5.f,storyGlyphCount(current().text)/GlyphsPerSecond+2.8f);}
    bool textComplete()const{return revealed||age*GlyphsPerSecond>=storyGlyphCount(current().text);}
    std::string visibleText()const{return revealed?current().text:storyPrefix(current().text,size_t(age*GlyphsPerSecond));}
    void enter(IntroPhase next){phase=next;age=0;revealed=false;}
    void nextLine(){if(line+1<IntroDialogue.size()){line++;enter(IntroPhase::Dialogue);}else enter(IntroPhase::Charging);}
    void advance(){
        if(phase==IntroPhase::Opening){enter(IntroPhase::Dialogue);return;}
        if(phase!=IntroPhase::Dialogue||age<.15f)return;
        if(!textComplete())revealed=true;else nextLine();
    }
    void skip(){
        if(phase!=IntroPhase::Opening&&phase!=IntroPhase::Dialogue)return;
        line=IntroDialogue.size()-1;enter(IntroPhase::Charging);
    }
    void update(float dt){
        if(!active()||!std::isfinite(dt)||dt<=0)return;
        dt=std::min(dt,.1f);age+=dt;time+=dt;
        if(phase==IntroPhase::Opening&&age>=OpeningTime)enter(IntroPhase::Dialogue);
        else if(phase==IntroPhase::Dialogue&&age>=readingTime())nextLine();
        else if(phase==IntroPhase::Charging&&age>=ChargeTime)enter(IntroPhase::Departing);
        else if(phase==IntroPhase::Departing&&age>=DepartTime){enter(IntroPhase::Arriving);arrivalPending=true;}
        else if(phase==IntroPhase::Arriving&&age>=ArrivalTime)enter(IntroPhase::Finished);
    }
    bool takeArrival(){bool pending=arrivalPending;arrivalPending=false;return pending;}
    float fade()const{
        if(phase==IntroPhase::Departing)return storyEase(age/DepartTime);
        if(phase==IntroPhase::Arriving)return 1-storyEase(age/.45f);
        if(phase==IntroPhase::Opening)return 1-storyEase(age/.8f);
        return 0;
    }
};
}
