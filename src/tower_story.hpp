#pragma once
#include "intro_story.hpp"

namespace ember {
inline constexpr std::array<StoryLine,8> TowerReturnDialogue={{
    {false,"Eldrinie, Kamienny Strażnik padł. Mam pierwszą część Serca Świtu. Nawet pod kamiennym pyłem nie przestaje świecić."},
    {true,"Dobrze się spisałeś, Wędrowcze. Wysłałem cię przeciw mocy starszej od tej wieży, a jednak wróciłeś. Odzyskaliśmy pierwszą z pięciu części."},
    {false,"Następnym razem uprzedź mnie, jak daleko sięga kamienna pięść."},
    {true,"Zapamiętam. Zachowaj odłamek przy sobie. Runy wieży ukazują tylko jego odbicie; prawdziwe Serce złożymy, gdy odzyskasz całą piątkę."},
    {false,"A Pieczęć? Mamy jeszcze czas?"},
    {true,"Moje zaklęcia wciąż ją podtrzymują. Pozostały cztery części. Następnej strzeże Żywa Wydma w Morzu Piasków."},
    {false,"Więc najpierw odpocznę, potem pomówimy o drodze przez piaski."},
    {true,"Wieża jest twoim schronieniem. Źródło odnowi twoje siły, a Miron na trzecim piętrze uzupełni zapasy. Mogę też nauczyć cię władać ogniem, mrozem i burzą. Każda lekcja wymaga złota na runy i odczynniki. Wróć, gdy będziesz gotowy."}
}};
inline constexpr std::array<StoryLine,5> TowerTalkDialogue={{
    {true,"Pierwsza część jest w twoich rękach. Kolejny szlak wiedzie przez Morze Piasków."},
    {false,"Co powinienem wiedzieć o strażniku pustyni?"},
    {true,"Żywa Wydma przybiera kształt olbrzyma i więzi intruzów w piasku. Szukaj jej na południowym wschodzie. To ona strzeże drugiej części."},
    {false,"Będę uważał pod nogi. Otworzysz mi drogę?"},
    {true,"Gdy wybierzesz podróż. Przy miejscu przybycia czeka Brama Eldrina — wrócisz nią do wieży, kiedy zechcesz odpocząć lub uzupełnić zapasy."}
}};
inline constexpr std::array<StoryLine,6> DesertReturnDialogue={{
    {false,"Żywa Wydma rozsypała się, Eldrinie. W piasku znalazłem drugą część. Jej blask rozpoznał odłamek z Kniei."},
    {true,"Dwie z pięciu części znów są razem. Dobrze się spisałeś. Strzeż obu — do odnowienia Pieczęci będziemy potrzebowali całego Serca."},
    {false,"Czuję jeszcze piasek w zbroi. Mam nadzieję, że nie każesz mi zaraz wracać na pustynię."},
    {true,"Trzeciej części szukaj w Kalderze Popiołu, krainie Varkhora, zwanego Sercem Kaldery. Bazaltowe mosty prowadzą tam nad lawą."},
    {false,"Zanim wyruszę, odpocznę i sprawdzę ekwipunek. Na piaskach przekonałem się, ile od tego zależy."},
    {true,"Słusznie. Skorzystaj ze źródła i zapasów Mirona. Gdy wrócisz, otworzę przejście do Kaldery. Droga do oazy również pozostaje otwarta."}
}};
inline constexpr std::array<StoryLine,3> DesertCompletedTalk={{
    {true,"Odłamek Żywej Wydmy jest już bezpieczny. Jej straż nad drugą częścią dobiegła końca."},
    {false,"W Morzu Piasków zostały jeszcze miejsca, które chciałbym zbadać."},
    {true,"Otworzę ci przejście. Brama Eldrina przy miejscu przybycia sprowadzi cię z powrotem do wieży."}
}};
inline constexpr std::array<StoryLine,4> VolcanoReturnDialogue={{
    {false,"Varkhor padł. Mam trzecią część Serca Świtu. Jej światło przetrwało nawet żar Kaldery."},
    {true,"Dokonałeś wielkiej rzeczy, Wędrowcze. Odzyskaliśmy trzy części; wciąż brakuje dwóch. Zachowaj je razem — ich moc będzie potrzebna do odnowienia Pieczęci."},
    {false,"Kiedy wyjąłem odłamek z popiołu, usłyszałem dzwon. W kuźni nie było już nic, co mogłoby wydać taki dźwięk."},
    {true,"Znam ten ton. Na Mokradłach Cichych Dzwonów woda porwała niegdyś całą osadę. Jej dzwon wciąż odpowiada Sercu. Odpocznij. Gdy wrócisz, otworzę ci drogę."}
}};
inline constexpr std::array<StoryLine,3> VolcanoTalkDialogue={{
    {true,"Droga do Kaldery Popiołu jest otwarta. Varkhor, Serce Kaldery, strzeże tam trzeciej części artefaktu."},
    {false,"Jak dotrzeć do niego, nie kończąc w rzece lawy?"},
    {true,"Trzymaj się bazaltowych mostów. Szukaj Varkhora na północnym wschodzie i schodź z miejsc, gdzie ziemia zapowiada erupcję. Brama przy miejscu przybycia pozwoli ci wrócić."}
}};
inline constexpr std::array<StoryLine,3> VolcanoCompletedTalk={{
    {true,"Na mokradłach szukaj domów na palach. Za ostatnim domem grobla skręca ku dzwonnicy. Tam Dzwonnik Topieli nosi czwartą część pod spiżowym kloszem."},
    {false,"Skoro dzwon odpowiada odłamkom, Dzwonnik usłyszy, jak nadchodzę. Po czym poznam, że szykuje się do ciosu?"},
    {true,"Patrz na wodę. Nim uderzy serce dzwonu, kręgi zacisną się u jego stóp. Przeprawię cię na groblę. Przy niej zostawię bramę powrotną."}
}};
inline constexpr std::array<StoryLine,3> DesertShardPendingTalk={{
    {true,"Żywa Wydma padła, lecz jej odłamek nie wrócił z tobą. Samo zwycięstwo nad strażnikiem nie wystarczy — potrzebujemy części Serca."},
    {false,"Została wśród jej szczątków. Wrócę po nią przez bramę."},
    {true,"Otworzę drogę do Morza Piasków. Przejście do Kaldery także jest dostępne, ale nie zapomnij o odłamku pozostawionym na pustyni."}
}};
inline constexpr std::array<StoryLine,3> VolcanoShardPendingTalk={{
    {true,"Varkhor już ci nie zagrozi, lecz trzecia część nadal spoczywa przy jego szczątkach. Nie zostawiaj jej w Kalderze."},
    {false,"Potrzebowałem oddechu po walce. Wrócę i zabiorę odłamek."},
    {true,"Brama jest gotowa. Przygotuj się w wieży, a potem wybierz powrót do Kaldery Popiołu."}
}};
inline constexpr std::array<StoryLine,4> VolcanoReturnMissingDesertDialogue={{
    {false,"Varkhor pokonany, Eldrinie. Przyniosłem część Serca, której strzegł w Kalderze."},
    {true,"Dobrze się spisałeś. Pamiętaj jednak o odłamku Żywej Wydmy — został na pustyni. Nie mamy jeszcze trzech części."},
    {false,"Wrócę do jej szczątków. Nie po to przemierzyłem te krainy, żeby zostawić tam część Serca."},
    {true,"Otworzę ci przejście do Morza Piasków. Zabierz odłamek; każda z pięciu części jest potrzebna do odnowienia Pieczęci."}
}};
inline constexpr std::array<StoryLine,3> VolcanoMissingDesertTalk={{
    {true,"Odłamek Varkhora jest już z tobą. Część Żywej Wydmy nadal jednak czeka pośród jej szczątków w Morzu Piasków."},
    {false,"Muszę wrócić na pustynię i domknąć tę wyprawę."},
    {true,"Wybierz przejście do Morza Piasków. Brama przy miejscu przybycia znów sprowadzi cię do wieży."}
}};

inline constexpr std::array<StoryLine,4> MarshReturnDialogue={{
 {false,"Dzwonnik padł. Gdy wyjąłem odłamek spod klosza, po wodzie przeszła ostatnia fala. Potem usłyszałem już tylko żaby."},
 {true,"Przez tyle lat pełnił straż pośród pustych domów. Dobrze, że wreszcie może zamilknąć. Połóż czwartą część przy pozostałych, Wędrowcze."},
 {false,"Cztery głosy brzmią razem. Wciąż brakuje jednego. Słyszysz, skąd odpowiada?"},
 {true,"Jeszcze nie. Tej nocy zostanę przy runach. Ty odpocznij. Jeśli zechcesz wrócić na mokradła, grobla wciąż jest bezpieczna."}
}};
inline constexpr std::array<StoryLine,2> MarshShardPendingTalk={{
 {false,"Dzwonnik umilkł, lecz nie zabrałem odłamka. Został wśród jego łańcuchów."},
 {true,"Otworzę ci drogę na groblę. Wróć pod dzwonnicę i przynieś część Serca, zanim skryje ją woda."}
}};
inline constexpr std::array<StoryLine,2> MarshCompletedTalk={{
 {false,"Czy runy pokazały ci drogę do ostatniej części?"},
 {true,"Wciąż nasłuchuję. Możesz tymczasem wrócić do osady lub odwiedzić wcześniejsze krainy. Bramy pozostają otwarte."}
}};
struct StoryDialogueView {const StoryLine* lines;size_t size;};
template<size_t N>constexpr StoryDialogueView storyDialogue(const std::array<StoryLine,N>& lines){return {lines.data(),N};}

enum class TowerPhase {Finished,FadeOut,Opening,Dialogue,Closing,Reveal,Choice,TravelOut,TravelIn};
struct TowerSequence {
    TowerPhase phase=TowerPhase::Finished;
    size_t line=0;float age=0,time=0;
    bool returning=false,revealed=false,arrivalPending=false,completionPending=false,offerDesert=false,desertArrivalPending=false;int fragments=1;bool offerVolcano=false,travelVolcano=false,offerMarsh=false,travelMarsh=false,missingMarshShard=false;
    static constexpr float FadeTime=.9f,OpeningTime=1.4f,ClosingTime=.45f,RevealTime=.7f;
    void enter(TowerPhase next){phase=next;age=0;revealed=false;}
    bool missingDesertShard=false,missingVolcanoShard=false;
    void beginReturn(int parts=1,bool missingDesert=false){*this={};fragments=parts;missingDesertShard=missingDesert;returning=true;enter(TowerPhase::FadeOut);}
    void beginTalk(bool offer=false,int parts=1,bool volcano=false,bool missingDesert=false,bool missingVolcano=false,bool marsh=false,bool missingMarsh=false){*this={};offerMarsh=marsh;missingMarshShard=missingMarsh;offerDesert=offer;offerVolcano=volcano;fragments=parts;missingDesertShard=missingDesert;missingVolcanoShard=missingVolcano;enter(TowerPhase::Dialogue);}
    bool chooseMarsh(){if(phase!=TowerPhase::Choice||!offerMarsh)return false;travelMarsh=true;enter(TowerPhase::TravelOut);return true;}
    bool chooseVolcano(){if(phase!=TowerPhase::Choice||!offerVolcano)return false;travelVolcano=true;enter(TowerPhase::TravelOut);return true;}
    bool chooseDesert(){if(phase!=TowerPhase::Choice||!offerDesert)return false;travelVolcano=false;enter(TowerPhase::TravelOut);return true;}
    bool takeDesertArrival(){bool value=desertArrivalPending;desertArrivalPending=false;return value;}
    bool active()const{return phase!=TowerPhase::Finished;}
    bool chamber()const{return returning&&(phase==TowerPhase::Opening||phase==TowerPhase::Dialogue||phase==TowerPhase::Closing);}
    StoryDialogueView dialogue()const{
        if(returning){if(fragments>=4)return storyDialogue(MarshReturnDialogue);if(fragments>=3)return missingDesertShard?storyDialogue(VolcanoReturnMissingDesertDialogue):storyDialogue(VolcanoReturnDialogue);return fragments>=2?storyDialogue(DesertReturnDialogue):storyDialogue(TowerReturnDialogue);}
        if(missingMarshShard)return storyDialogue(MarshShardPendingTalk);
        if(fragments>=4)return storyDialogue(MarshCompletedTalk);
        if(missingDesertShard)return fragments>=3?storyDialogue(VolcanoMissingDesertTalk):storyDialogue(DesertShardPendingTalk);
        if(missingVolcanoShard)return storyDialogue(VolcanoShardPendingTalk);
        if(fragments>=3)return storyDialogue(VolcanoCompletedTalk);
        if(offerVolcano)return storyDialogue(VolcanoTalkDialogue);
        return fragments>=2?storyDialogue(DesertCompletedTalk):storyDialogue(TowerTalkDialogue);
    }
    size_t lineCount()const{return dialogue().size;}
    const StoryLine& current()const{return dialogue().lines[line];}
    float readingTime()const{return std::max(5.f,storyGlyphCount(current().text)/IntroSequence::GlyphsPerSecond+2.8f);}
    bool textComplete()const{return revealed||age*IntroSequence::GlyphsPerSecond>=storyGlyphCount(current().text);}
    std::string visibleText()const{return revealed?current().text:storyPrefix(current().text,size_t(age*IntroSequence::GlyphsPerSecond));}
    void nextLine(){if(line+1<lineCount()){line++;enter(TowerPhase::Dialogue);}else enter(returning?TowerPhase::Closing:(offerDesert||offerVolcano||offerMarsh)?TowerPhase::Choice:TowerPhase::Finished);}
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
