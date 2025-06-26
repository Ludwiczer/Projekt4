# Sprawozdanie z projektu: Symulator Windy w C++ z GDI+

## Cel projektu
Celem projektu było stworzenie symulatora windy w budynku 5-piętrowym z wizualizacją 2D za pomocą biblioteki GDI+. Program ma obsługiwać ruch windy, pasażerów czekających na piętrach oraz ich przejazdy do wybranych pięter.

## Opis działania
- Winda porusza się pionowo w szybcie windy o wymiarach.
- Na każdym piętrze znajdują się przyciski umożliwiające wezwanie windy i wybranie docelowego piętra.
- Pasażerowie reprezentowani są jako kolorowe kółka z numerem docelowego piętra.
- Winda ma ograniczenie udźwigu (600 kg), uwzględniane przy wsiadaniu pasażerów (średnio 70 kg na osobę).
- Animacja ruchu windy i pasażerów działa płynnie, odświeżana co ok. 16 ms (60 FPS).
- Program obsługuje kolejkowanie wywołań i inteligentne zarządzanie kolejnością odwiedzanych pięter.

## Struktura kodu
- `main.cpp` - punkt wejścia, konfiguracja okna, główna pętla komunikatów.
- Funkcja `update()` - logika ruchu windy, obsługa pasażerów i kolejek.
- Funkcja `drawElevator(Graphics&)` - rysowanie windy, pasażerów, przycisków i UI.
- Struktury danych: `Passenger`, kolejki pasażerów oczekujących i jadących.

## Problemy i rozwiązania
- Eliminacja migotania ekranu – zastosowano podwójne buforowanie w procedurze WM_PAINT.
- Zarządzanie kolejką pięter i ograniczeniem udźwigu windy.

## Możliwe ulepszenia
- Dodanie animacji drzwi windy (otwieranie/zamykanie).
- Zastąpienie kulkowych pasażerów animowanymi sprite'ami.

## Podsumowanie
Projekt zrealizowano w C++ z użyciem GDI+, prezentując podstawową symulację windy wraz z prostą grafiką i interakcją użytkownika. Pozwolił na ćwiczenie zarządzania stanem aplikacji okienkowej oraz obsługi grafiki 2D.

**Autorzy:** Oliwier Stasiuk 203891 Patryk Kowiel 203591