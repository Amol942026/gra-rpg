# Dziesięć broni Mirona

[Broń, rzadkość, źródło łupu i cena](../../../LOOT.md#dziesięć-dodatkowych-broni)
są w katalogu gry. [Podgląd](bron-mirona.png) pokazuje ikony i broń w rękach
bohatera, wyrenderowane na plikach rozpakowanej wersji gracza.

W pliku `bron-mirona.kra` każda broń ma osobną warstwę z paletą i wzorem
inkrustacji. Eksport PNG został odczytany do
`src/generated/additional_weapon_paints.hpp`. Animacja korzysta z istniejących
czterech typów broni. Pochodzenie i hashe zapisano w `provenance.json`.

[Raport QA](QA.json) obejmuje losowania z ciał i skrzyń, zakup wszystkich
dziesięciu broni przez dwuklik w sklepie, wygląd po wyposażeniu oraz
zgodność zapisów. Próby gry wykonano w ukryciu, bez dźwięku.
