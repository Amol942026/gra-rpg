# Łup z przeciwników — Emberfall 0.20

Każda pozycja w tabeli jest losowana **niezależnie przy śmierci przeciwnika**. Jeden wilk może zostawić jednocześnie kieł i sierść, tylko jeden z tych materiałów albo żaden. Szansa dotyczy pojawienia się stosu; jego ilość jest następnie losowana z podanego przedziału. 75% nie gwarantuje trzech dropów na każde cztery zabójstwa.

Wszystkie gatunki zostawiają dodatkowo **4–15 złotych monet ze szansą 100%**. Łup pozostaje w ciele: podejdź na maksymalnie 2,5 pola, otwórz je **PPM** i przeciągnij przedmioty do plecaka. Ponowne otwarcie ciała nie losuje nowej zawartości.

| Przeciwnik | Pozostały łup: szansa i ilość |
|---|---|
| Szlam | Śluz szlama **80%**, 1–3; Mikstura zdrowia **15%**, 1; Odłamek echa **30%**, 1–2; Bursztynowy amulet **1%**, 1 |
| Wilk | Kieł wilka **75%**, 1–2; Sierść wilka **55%**, 1–2; Mikstura zdrowia **12%**, 1; Płaszcz z wilczej skóry **3%**, 1, niepospolity |
| Szkielet | Fragment kości **70%**, 1–3; Sztylet łowcy **8%**, 1; Mikstura many **18%**, 1; Żelazny hełm **5%**, 1, niepospolity |
| Strażnik ruin | Miecz wędrowca **100%**, 1, legendarny; Pierścień żaru **100%**, 1, legendarny; Esencja kniei **100%**, 1–2, niepospolita; Kamień runiczny **50%**, 1 |
| Dzik | Kieł dzika **65%**, 1–2; Skóra dzika **80%**, 1; Mikstura zdrowia **10%**, 1; Zbroja zwiadowcy **4%**, 1, niepospolita |
| Leśny pająk | Pajęcza nić **85%**, 1–3; Woreczek jadu **40%**, 1; Mikstura many **15%**, 1; Bursztynowy amulet **2%**, 1, niepospolity |
| Leśny rozbójnik | Odznaka rozbójnika **60%**, 1; Sztylet łowcy **12%**, 1, niepospolity; Mikstura zdrowia **25%**, 1–2; Buty podróżnika **8%**, 1, niepospolite |
| Ożywione drzewo | Prastara kora **90%**, 1–3; Esencja kniei **35%**, 1; Kamień runiczny **8%**, 1; Bursztynowy amulet **4%**, 1, niepospolity |

Przedmioty bez oznaczenia rzadkości są pospolite. Materiały łączą się w stosy do **99 sztuk**. Pełny stos lub plecak nie usuwa nadmiaru — zostaje w ciele. Na razie materiały są zbieranymi trofeami; nie ma jeszcze rzemiosła ani handlu.

Nowy sprzęt można założyć: sztylet w miejsce broni, płaszcz w miejsce zbroi, amulet w miejsce pierścienia. Ich premie pokazuje podpowiedź przedmiotu. Podpowiedź łupu w ciele pokazuje też dokładną szansę dla tego gatunku, a sekcja **POD KURSOREM** podaje główne materiały przeciwnika i ich szanse.

Normalne wyprawy otrzymują nowe losowe ziarno łupu. Testy korzystają ze stałego ziarna, aby wykrywać błędy powtarzalnie. Tabele i ilości edytuje się w `src/loot.hpp`.
