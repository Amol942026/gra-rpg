#pragma once
#include "game.hpp"
namespace ember {
inline const char* dawnQuestHint(const Game& game){
 if(game.inTower())return game.dawnShardUnclaimed(WorldLocation::Desert)?"Odłamek Żywej Wydmy czeka na pustyni.":game.dawnShardUnclaimed(WorldLocation::Volcano)?"Odłamek Varkhora czeka w Kalderze.":"Schody i wyposażenie · E / PPM";
 if(game.inVolcano())return game.dawnShardUnclaimed(WorldLocation::Volcano)?"Zbierz fragment z ciała Varkhora.":game.recoveredDawnPart(2)?"Część Varkhora odzyskana. Wróć do maga.":"Varkhor: północny wschód Kaldery.";
 if(game.inDesert())return game.dawnShardUnclaimed(WorldLocation::Desert)?"Zbierz fragment z ciała Żywej Wydmy.":game.recoveredDawnPart(1)?"Część Wydmy odzyskana. Wróć do maga.":"Żywa Wydma: południowy wschód.";
 return game.dawnShardUnclaimed(WorldLocation::Forest)?"Zbierz część Serca Świtu z ciała.":game.recoveredDawnPart(0)?"Pierwsza część odzyskana.":"Odnajdź Strażnika na północy Kniei.";
}
inline const char* mageTravelDescription(const Game& game,bool volcano){
 if(game.dawnShardUnclaimed(WorldLocation::Desert))return "Odłamek Żywej Wydmy nadal czeka w Morzu Piasków. Możesz po niego wrócić lub wyruszyć do Kaldery Popiołu. Bramy obu krain prowadzą do wieży.";
 if(game.dawnShardUnclaimed(WorldLocation::Volcano))return "Varkhor został pokonany, lecz jego część Serca Świtu pozostała w Kalderze. Wróć po nią; Brama Eldrina sprowadzi cię potem do wieży.";
 if(volcano)return game.volcanoBossDead?"Varkhor został pokonany. Możesz wrócić na szlaki Kaldery lub do Morza Piasków. Obie bramy powrotne pozostają otwarte.":"W Kalderze Popiołu Varkhor strzeże trzeciej części Serca Świtu. Przez lawę prowadzą bazaltowe mosty. Brama przy miejscu przybycia pozwoli ci wrócić.";
 return game.desertBossDead?"Żywa Wydma została pokonana. Droga do oazy i pozostawionego łupu pozostaje otwarta. Wrócisz do wieży przez Bramę Eldrina.":"Żywa Wydma strzeże drugiej części Serca Świtu w Morzu Piasków. Brama przy miejscu przybycia zapewni ci powrót do wieży.";
}
inline const char* dawnReturnSubtitle(const Game& game){
 return game.recoveredDawnFragments()>=3?"Trzy części odzyskane. Wciąż brakuje dwóch.":game.recoveredDawnFragments()>=2?"Dwie części odzyskane. Wciąż brakuje trzech.":"Pierwsza część odzyskana. Wyprawa trwa.";
}
inline const char* towerChronicleText(int page,const Game& game){
 if(page==0)return "Moc Serca Świtu utworzyła Pieczęć Popiołu, granicę między światem a Otchłanią. Dawno temu artefakt podzielono na pięć części i ukryto pod strażą. Dziś Pieczęć słabnie. Eldrin podtrzymuje ją zaklęciami; wszystkie pięć części pozwoli odnowić jej moc.";
 if(page==1)return game.recoveredDawnPart(0)?"Kamienny Strażnik strzegł pierwszej części w północnych ruinach Wielkiej Kniei. Wędrowiec przełamał jego obronę i odzyskał odłamek. Prawdziwe części pozostają w plecaku bohatera. Runy w sali pieczęci ukazują tylko ich odbicia, a nie złożony artefakt.":"Kamienny Strażnik pilnuje pierwszej części w północnych ruinach Wielkiej Kniei. Pokonanie strażnika nie wystarczy: trzeba jeszcze zabrać odłamek z jego szczątków. Runy sali pieczęci ukażą odbicie każdej odzyskanej części.";
 return "Wieża Eldrina chroni podróżnych. Źródło w sali przybycia odnawia siły, Miron na trzecim piętrze prowadzi handel, a w pracowni można warzyć mikstury. Wyższe piętra mieszczą kroniki i obserwatorium. Tutaj mag szuka śladów pozostałych strażników.";
}
inline const char* towerConstellationHint(const Game& game){
 if(game.dawnShardUnclaimed(WorldLocation::Desert))return "Ślad odłamka nadal prowadzi do Morza Piasków.";
 if(game.dawnShardUnclaimed(WorldLocation::Volcano))return "Ślad odłamka nadal prowadzi do Kaldery Popiołu.";
 return game.recoveredDawnFragments()>=3?"Eldrin szuka śladów dwóch pozostałych strażników.":game.desertBossDead?"Szlak trzeciego strażnika wiedzie do Kaldery Popiołu.":"Szlak drugiego strażnika wiedzie do Morza Piasków.";
}
}
