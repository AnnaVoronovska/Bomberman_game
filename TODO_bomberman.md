# TODO

## ❌ Nog te doen

### Verplicht
- [x] Death-animatie — personage verdwijnt nu gewoon i.p.v. animatie
      (`Views.cpp` regel 272). Verplicht volgens opgave.
- [x] Unit tests voor de logica (World, Entities, Score, Bomb-explosie-logica)
- [x] Levels 2, 2de sletel ook -> "You Win" + animatie
- [x] Bot AI daadwerkelijk aanzetten/testen in-game — check of `updateBotAI`,
      pathfinding en escape-logic ook echt actief zijn en getest
- [x] `.hpp` → `.h` hernoemen waar nog nodig (comments verwijzen soms nog naar `.hpp`)
- [ ] Class-diagrammen maken (zoals figuur 3 in de opgave) voor je verslag
- [ ] Verslag schrijven
- [x] AI-gegenereerde code opschonen: comments nalopen, overbodige AI-stijl-comments
      verwijderen, zorg dat je alles kan uitleggen (telt zwaar mee, 40% verdediging!)
- [ ] Opdracht herlezen en zien of ik iets gemist heb

### Te verifiëren
- [x] `valgrind` runnen — opgave vraagt dit expliciet (memory leaks + initialisatie)
- [ ] Check of alle primitieve types overal expliciet geïnitialiseerd zijn

### Optioneel (bonus, max 10%)
- [x] Victory-animatie (toevoegen dat het ook wanneer je op de deur staat)
- [x] Sounds/muziek
- [x] Extra design patterns (Command, Visitor)
- [ ] Generic programming/templates (nog niet gebruikt in de code)
- [ ] Multi-threading
- [ ] Meson als build-systeem i.p.v. CMake


