# Bomberman — Advanced Programming 2025-2026

**Naam:** Anna Voronovska
**Studentennummer:** 20240523

Een C++-implementatie van de game Bomberman voor het vak Gevorderd Programmeren, waarbij SFML wordt gebruikt voor het grafische en interactieve gedeelte van de game.
Uitleg over de geïmplementeerde features, architectuur, designkeuzes, klassendiagrammen en andere projectdocumentatie is te vinden in `verslag.docx`.

## Bouwen en runnen

Vereisten: Ubuntu 24.04, CMake 3.28+, G++ 13+, SFML 2.6.

```bash
# Volledige game runnen
mkdir build && cd build
cmake ..
make -j4
./bomberman        # runnen vanuit de Bomberman_game/-rootmap (relatieve asset-paden)
```

```bash
# Unit tests runnen
cd tests
mkdir build && cd build
cmake ..
make -j4
./bomberman_tests
```

## Besturing

| Toets | Actie |
|---|---|
| Pijltjestoetsen | Bewegen|   
| Spatie | Bom plaatsen |

Beweging is continu, zoals de opgave vereist: het personage glijdt vloeiend over het scherm zolang een toets ingedrukt blijft.

## Projectstructuur

```
Bomberman_game/
├── src/
│   ├── backend/     — spellogica voor de hele game, geen SFML nodig
│   └── frontend/    — het play venster, tekenen, geluid (wel SFML gebruikt)
├── tests/           — unit tests
├── assets/          — sprites en geluid
└── .circleci/       — configuratie voor circleci, bouwt het project automatisch bij elke push
```

## Spelregels

- Jij (linksboven) speelt tegen 3 bots die elk in een hoek starten.
- Plaats bommen om breekbare muren te vernielen en power-ups te verzamelen.
- Een explosie kost een leven; elk character heeft er 3.
- Ergens onder een breekbare muur zit een verborgen deur — loop erover om naar de volgende level te gaan.
- Er zijn 2 levels; haal de deur op level 2 om te winnen.
- Wees als laatste over, of overleef tot de tijd (3 min/level) om niet te verliezen.

**Punten:**
- +10 per vernielde muur
- +25 per opgeraapte power-up
- +150 per gedode vijand
- +500 bij winnen
- -50 bij verliezen


## Links

- GitHub-repo: https://github.com/AnnaVoronovska/Bomberman_game.git
- CI-status: [CIRCLECI-BADGE/LINK HIER]
