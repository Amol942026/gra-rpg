# Osiem kierunków animacji

Każdy z pięciu typów postaci (`hero`, `slime`, `wolf`, `skeleton`, `guardian`) ma trzy arkusze: `idle` (bezczynność), `walk` (chód) i `attack` (atak).

- Rozmiar arkusza: **1024 × 1152**, przezroczyste tło.
- Rozmiar klatki: **128 × 144**.
- Wiersze od góry: **E, SE, S, SW, W, NW, N, NE** w układzie ekranu.
- Kolumny od lewej: **8 klatek** cyklu danego kierunku.
- Punkt stóp: **(64, 125)** w komórce.

To eksporty tych samych geometrycznych modeli i póz, których używa gra. Renderowanie w grze obsługuje też bieżący sprzęt bohatera i płynne odmierzanie czasu bezczynności oraz zamachu. Cykle chodu korzystają z ośmiu klatek wybieranych na podstawie faktycznie pokonanej odległości.

Ponowny eksport: `Emberfall.exe --visual-qa`. Źródła: `src/animation.hpp`, `src/actor_art.hpp` i `src/actor_view.hpp`.
