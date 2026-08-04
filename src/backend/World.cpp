#include "World.h"
#include <cmath>
#include <algorithm>
#include <queue>

namespace bomberman {

World::World(AbstractFactory& factory, Score& score)
    : factory_(factory), score_(score) {}

Vec2 World::cellToWorld(int col, int row) const {
    double cw = 2.0 / GRID_COLS;
    double ch = 2.0 / GRID_ROWS;
    return Vec2(-1.0 + cw * (col + 0.5), -1.0 + ch * (row + 0.5));
}

// ---------------- Arena opbouwen ----------------

void World::generateArena() {
    // Cellen die vrij moeten blijven zodat elk character veilig kan spawnen
    // (klassieke "L-vormige" open hoeken uit de originele Bomberman arena).
    auto isSpawnArea = [](int col, int row) {
        const std::pair<int, int> clear[] = {
            {1, 1}, {2, 1}, {1, 2},
            {GRID_COLS - 2, 1}, {GRID_COLS - 3, 1}, {GRID_COLS - 2, 2},
            {1, GRID_ROWS - 2}, {2, GRID_ROWS - 2}, {1, GRID_ROWS - 3},
            {GRID_COLS - 2, GRID_ROWS - 2}, {GRID_COLS - 3, GRID_ROWS - 2}, {GRID_COLS - 2, GRID_ROWS - 3}
        };
        for (auto& c : clear) if (c.first == col && c.second == row) return true;
        return false;
    };

    for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLS; ++col) {
            bool border = (row == 0 || row == GRID_ROWS - 1 || col == 0 || col == GRID_COLS - 1);
            bool pillar = (row % 2 == 0 && col % 2 == 0); // alternerend patroon, zie opgave figuur 1
            Vec2 pos = cellToWorld(col, row);

            if (border || pillar) {
                walls_.push_back(factory_.createWall(pos, false)); // onverwoestbaar
            } else if (!isSpawnArea(col, row)) {
                // Vooral breekbare blokken, met een kleine kans op lege lucht.
                if (Random::instance().getDouble01() < 0.8) {
                    walls_.push_back(factory_.createWall(pos, true));
                }
            }
        }
    }

    player_ = factory_.createCharacter(cellToWorld(1, 1), false);
    characters_.push_back(player_);
    characters_.push_back(factory_.createCharacter(cellToWorld(GRID_COLS - 2, 1), true));
    characters_.push_back(factory_.createCharacter(cellToWorld(1, GRID_ROWS - 2), true));
    characters_.push_back(factory_.createCharacter(cellToWorld(GRID_COLS - 2, GRID_ROWS - 2), true));
}

// ---------------- Hulpfuncties ----------------

Wall* World::wallAtCell(int col, int row) const {
    Vec2 target = cellToWorld(col, row);
    for (auto& w : walls_) {
        if (std::abs(w->getPosition().x - target.x) < 1e-6 && std::abs(w->getPosition().y - target.y) < 1e-6)
            return w.get();
    }
    return nullptr;
}

bool World::isBombAt(int col, int row) const {
    Vec2 target = cellToWorld(col, row);
    for (auto& b : bombs_) {
        if (std::abs(b->getPosition().x - target.x) < 1e-6 && std::abs(b->getPosition().y - target.y) < 1e-6)
            return true;
    }
    return false;
}

void World::setPlayerDirection(Direction dir) {
    if (player_) player_->setDirection(dir);
}

void World::requestPlayerBomb() {
    pendingPlayerBomb_ = true;
}

// ---------------- Beweging + botsing ----------------

void World::tryMoveCharacter(Character& c, double deltaTime) {
    if (!c.isAlive()) return;
    double speed = c.getSpeed();
    Vec2 delta(0, 0);
    switch (c.getDirection()) {
        case Direction::Up:    delta = Vec2(0, -speed * deltaTime); break;
        case Direction::Down:  delta = Vec2(0,  speed * deltaTime); break;
        case Direction::Left:  delta = Vec2(-speed * deltaTime, 0); break;
        case Direction::Right: delta = Vec2( speed * deltaTime, 0); break;
        default: return;
    }

    auto standing = c.getStandingOnBomb().lock();
    auto blocked = [&](const Vec2& pos) {
        Wall probe(pos, c.getSize(), false);
        for (auto& w : walls_) if (probe.intersects(*w)) return true;
        for (auto& b : bombs_) {
            if (b.get() == standing.get()) continue; // eigen net-geplaatste bom mag je nog verlaten
            if (probe.intersects(*b)) return true;
        }
        return false;
    };

    // Elke as apart proberen: geeft een vloeiend "afglijd"-effect langs muren.
    Vec2 pos = c.getPosition();
    Vec2 tryX(pos.x + delta.x, pos.y);
    if (!blocked(tryX)) pos.x = tryX.x;
    Vec2 tryY(pos.x, pos.y + delta.y);
    if (!blocked(tryY)) pos.y = tryY.y;

    if (pos.x != c.getPosition().x || pos.y != c.getPosition().y) {
        c.setPosition(pos);
        c.notify(Event{EventType::Moved, &c});
    }

    // Zodra je de tegel van je eigen bom verlaat, mag je er niet meer doorheen.
    if (standing && !c.intersects(*standing)) {
        c.setStandingOnBomb(std::weak_ptr<Bomb>());
    }
}

// ---------------- Bommen plaatsen & laten ontploffen ----------------

void World::placeBomb(const std::shared_ptr<Character>& owner) {
    if (!owner || !owner->canPlaceBomb()) return;
    double cw = 2.0 / GRID_COLS, ch = 2.0 / GRID_ROWS;
    Vec2 pos = owner->getPosition();
    int col = static_cast<int>(std::round((pos.x + 1.0) / cw - 0.5));
    int row = static_cast<int>(std::round((pos.y + 1.0) / ch - 0.5));
    if (isBombAt(col, row)) return;

    auto bomb = factory_.createBomb(cellToWorld(col, row), owner->getBombRadius(), owner);
    bombs_.push_back(bomb);
    owner->onBombPlaced();
    owner->setStandingOnBomb(bomb);
    notify(Event{EventType::BombPlaced, owner.get()});
}

void World::updateBombs(double deltaTime) {
    // 1) lonten laten aftellen
    for (auto& b : bombs_) {
        if (!b->hasExploded()) b->update(deltaTime);
    }
    // 2) schade toepassen voor elke bom die net ontploft is (fuse of chain-reactie)
    for (auto& b : bombs_) {
        if (b->hasExploded() && b->consumeDamageFlag()) {
            explodeBomb(*b);
        }
    }
}

void World::explodeBomb(Bomb& bomb) {
    if (auto owner = bomb.getOwner().lock()) owner->onBombResolved();

    double cw = 2.0 / GRID_COLS, ch = 2.0 / GRID_ROWS;
    int col = static_cast<int>(std::round((bomb.getPosition().x + 1.0) / cw - 0.5));
    int row = static_cast<int>(std::round((bomb.getPosition().y + 1.0) / ch - 0.5));

    std::vector<std::pair<int, int>> affected;
    affected.push_back({col, row});
    spreadExplosion(col, row,  0, -1, bomb.getRadius(), affected);
    spreadExplosion(col, row,  0,  1, bomb.getRadius(), affected);
    spreadExplosion(col, row, -1,  0, bomb.getRadius(), affected);
    spreadExplosion(col, row,  1,  0, bomb.getRadius(), affected);

    applyExplosionDamage(affected, bomb);
}

void World::spreadExplosion(int col, int row, int dcol, int drow, int radius,
                             std::vector<std::pair<int, int>>& affected) {
    for (int step = 1; step <= radius; ++step) {
        int c = col + dcol * step;
        int r = row + drow * step;
        if (c < 0 || c >= GRID_COLS || r < 0 || r >= GRID_ROWS) break;

        Wall* w = wallAtCell(c, r);
        if (w != nullptr) {
            if (!w->isDestructible()) break; // onverwoestbare muur stopt de explosie
            affected.push_back({c, r});
            break; // breekbare muur wordt geraakt, maar stopt de explosie erna
        }
        affected.push_back({c, r});
    }
}

    void World::applyExplosionDamage(const std::vector<std::pair<int, int>>& tiles, Bomb& source) {
    bool causedByPlayer = false;
    if (auto owner = source.getOwner().lock()) causedByPlayer = (owner == player_);

    for (auto& tile : tiles) {
        int c = tile.first, r = tile.second;
        Vec2 target = cellToWorld(c, r);

        // 1) Eerst bestaande power-ups op deze tegel vernietigen (van vóór deze explosie).
        for (auto& p : powerUps_) {
            if (std::abs(p->getPosition().x - target.x) < 1e-6 && std::abs(p->getPosition().y - target.y) < 1e-6) {
                p->markForRemoval();
            }
        }

        // 2) Pas daarna breekbare muren vernielen en eventueel een NIEUWE power-up spawnen
        //    (die overleeft deze explosie dan terecht, want stap 1 is al gebeurd).
        if (Wall* w = wallAtCell(c, r)) {
            if (w->isDestructible()) {
                w->destroy();
                if (causedByPlayer) notify(Event{EventType::BlockDestroyed, &source});
                maybeSpawnPowerUp(c, r);
            }
        }

        // Andere bommen in het bereik triggeren een kettingreactie.
        for (auto& b : bombs_) {
            if (b.get() == &source) continue;
            if (std::abs(b->getPosition().x - target.x) < 1e-6 && std::abs(b->getPosition().y - target.y) < 1e-6) {
                if (!b->hasExploded()) b->explode();
                if (b->consumeDamageFlag()) explodeBomb(*b);
            }
        }

        // Characters op deze tegel sterven onmiddellijk.
        Wall probe(target, Vec2(2.0 / GRID_COLS, 2.0 / GRID_ROWS), false);
        for (auto& c2 : characters_) {
            if (c2->isAlive() && c2->intersects(probe)) {
                bool wasPlayer = (c2 == player_);
                c2->die();
                if (wasPlayer) notify(Event{EventType::PlayerDied, &source});
                else if (causedByPlayer) notify(Event{EventType::EnemyKilled, &source});
            }
        }
    }
}

void World::maybeSpawnPowerUp(int col, int row) {
    if (Random::instance().getDouble01() > 0.25) return; // 25% kans, zoals in de opgave
    static const PowerUpType types[] = {
        PowerUpType::Fire, PowerUpType::ExtraBomb, PowerUpType::Skates,
        PowerUpType::Poison, PowerUpType::Star, PowerUpType::Shield,
        PowerUpType::Curse, PowerUpType::Slow, PowerUpType::Freeze, PowerUpType::Skull
    };
    int roll = Random::instance().getInt(0, 9);
    PowerUpType type = types[roll];
    powerUps_.push_back(factory_.createPowerUp(cellToWorld(col, row), type));
}

    // Verzamelt alle tegels die geraakt worden door een bom die nog niet ontploft is
// (kruisvorm, gestopt door muren), precies zoals een echte explosie dat zou doen.
void World::collectDangerTiles(std::vector<std::vector<bool>>& danger) const {
    danger.assign(GRID_ROWS, std::vector<bool>(GRID_COLS, false));

    auto markCell = [&](int c, int r) {
        if (c >= 0 && c < GRID_COLS && r >= 0 && r < GRID_ROWS) danger[r][c] = true;
    };

    for (auto& b : bombs_) {
        if (b->hasExploded()) continue;

        double cw = 2.0 / GRID_COLS, ch = 2.0 / GRID_ROWS;
        Vec2 bpos = b->getPosition();
        int bcol = static_cast<int>(std::round((bpos.x + 1.0) / cw - 0.5));
        int brow = static_cast<int>(std::round((bpos.y + 1.0) / ch - 0.5));
        int radius = b->getRadius();

        markCell(bcol, brow);

        const int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
        for (auto& d : dirs) {
            for (int step = 1; step <= radius; ++step) {
                int c = bcol + d[0] * step;
                int r = brow + d[1] * step;
                Wall* w = wallAtCell(c, r);
                if (w) {
                    // Onbreekbare muur stopt de explosie meteen (geen danger-tile hier);
                    // een breekbare muur wordt zelf nog geraakt, maar blokkeert verder.
                    if (w->isDestructible()) markCell(c, r);
                    break;
                }
                markCell(c, r);
            }
        }
    }
}

// Echte BFS naar de dichtstbijzijnde niet-gevaarlijke, bewandelbare tegel.
// Geeft de eerste stap van dat pad terug via outDir. Retourneert false als
// er geen enkele veilige tegel bereikbaar is.
bool World::findEscapeDirection(int fromCol, int fromRow,
                                 const std::vector<std::vector<bool>>& danger,
                                 Direction& outDir) const {
    struct Node { int col, row, firstStepIdx; };
    std::vector<std::vector<bool>> visited(GRID_ROWS, std::vector<bool>(GRID_COLS, false));
    std::queue<Node> q;

    const int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    const Direction dirEnum[4] = {Direction::Right, Direction::Left, Direction::Down, Direction::Up};

    // Als we nu al veilig staan, hoeven we niet te vluchten.
    if (fromRow >= 0 && fromRow < GRID_ROWS && fromCol >= 0 && fromCol < GRID_COLS
        && !danger[fromRow][fromCol]) {
        return false;
    }

    visited[fromRow][fromCol] = true;
    // Eerste ring: elke geldige buur start een eigen "tak" met bijhorende eerste stap.
    for (int i = 0; i < 4; ++i) {
        int c = fromCol + dirs[i][0], r = fromRow + dirs[i][1];
        if (c < 0 || c >= GRID_COLS || r < 0 || r >= GRID_ROWS) continue;
        if (wallAtCell(c, r) != nullptr) continue;
        if (visited[r][c]) continue;
        visited[r][c] = true;
        q.push({c, r, i});
    }

    while (!q.empty()) {
        Node n = q.front(); q.pop();
        if (!danger[n.row][n.col]) {
            outDir = dirEnum[n.firstStepIdx];
            return true;
        }
        for (int i = 0; i < 4; ++i) {
            int c = n.col + dirs[i][0], r = n.row + dirs[i][1];
            if (c < 0 || c >= GRID_COLS || r < 0 || r >= GRID_ROWS) continue;
            if (wallAtCell(c, r) != nullptr) continue;
            if (visited[r][c]) continue;
            visited[r][c] = true;
            q.push({c, r, n.firstStepIdx});
        }
    }
    return false; // geen veilige tegel bereikbaar
}

// ---------------- Eenvoudige bot-AI ----------------
// Volgt de 4 vereisten uit de opgave, in volgorde van prioriteit:
// 1) overleven (weg van bommen), 2) power-ups rapen,
// 3) muren afbreken / vijanden aanvallen, 4) willekeurig rondlopen.
void World::updateBotAI(const std::shared_ptr<Character>& bot, double deltaTime) {
    (void)deltaTime;
    if (!bot->isAlive()) { bot->setDirection(Direction::None); return; }

    double cw = 2.0 / GRID_COLS, ch = 2.0 / GRID_ROWS;
    Vec2 pos = bot->getPosition();

    int myCol = static_cast<int>(std::round((pos.x + 1.0) / cw - 0.5));
    int myRow = static_cast<int>(std::round((pos.y + 1.0) / ch - 0.5));

    // 1) Overleven: bereken alle gevaarlijke tegels en vlucht via echte BFS
    //    naar de dichtstbijzijnde bereikbare veilige tegel.
    std::vector<std::vector<bool>> danger;
    collectDangerTiles(danger);

    Direction escapeDir;
    if (findEscapeDirection(myCol, myRow, danger, escapeDir)) {
        bot->setDirection(escapeDir);
        return;
    }

    // 2) Hebzucht: loop naar de dichtstbijzijnde power-up.
    if (!powerUps_.empty()) {
        auto closest = std::min_element(powerUps_.begin(), powerUps_.end(),
            [&](const std::shared_ptr<PowerUp>& a, const std::shared_ptr<PowerUp>& b) {
                double da = std::abs(a->getPosition().x - pos.x) + std::abs(a->getPosition().y - pos.y);
                double db = std::abs(b->getPosition().x - pos.x) + std::abs(b->getPosition().y - pos.y);
                return da < db;
            });
        Vec2 target = (*closest)->getPosition();
        double dx = target.x - pos.x, dy = target.y - pos.y;
        if (std::abs(dx) > std::abs(dy)) bot->setDirection(dx > 0 ? Direction::Right : Direction::Left);
        else                              bot->setDirection(dy > 0 ? Direction::Down  : Direction::Up);
        return;
    }

    // 3) Agressie: plaats een bom naast een breekbare muur of een naburige vijand,
    //    maar alleen als er ná plaatsing ook echt nog een veilige tegel bereikbaar is.
    int col = myCol;
    int row = myRow;

    bool nearBreakable = false;
    const int offsets[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    for (auto& off : offsets) {
        Wall* w = wallAtCell(col + off[0], row + off[1]);
        if (w && w->isDestructible()) nearBreakable = true;
    }

    bool enemyNearby = false;
    for (auto& other : characters_) {
        if (other == bot || !other->isAlive()) continue;
        double d = std::abs(other->getPosition().x - pos.x) + std::abs(other->getPosition().y - pos.y);
        if (d < cw * (bot->getBombRadius() + 1)) enemyNearby = true;
    }

    bool hasEscapeRoute = false;
    if ((nearBreakable || enemyNearby) && bot->canPlaceBomb() && !isBombAt(col, row)) {
        // Simuleer: voeg de hypothetische bom toe aan de danger-map en kijk of er
        // dan nog een bereikbare veilige tegel is.
        std::vector<std::vector<bool>> simDanger = danger;
        auto markCell = [&](int c, int r) {
            if (c >= 0 && c < GRID_COLS && r >= 0 && r < GRID_ROWS) simDanger[r][c] = true;
        };
        markCell(col, row);
        int radius = bot->getBombRadius();
        for (auto& off : offsets) {
            for (int step = 1; step <= radius; ++step) {
                int c = col + off[0] * step, r = row + off[1] * step;
                Wall* w = wallAtCell(c, r);
                if (w) {
                    if (w->isDestructible()) markCell(c, r);
                    break;
                }
                markCell(c, r);
            }
        }
        Direction dummy;
        hasEscapeRoute = findEscapeDirection(col, row, simDanger, dummy);
    }

    if ((nearBreakable || enemyNearby) && bot->canPlaceBomb() && !isBombAt(col, row) && hasEscapeRoute) {
        placeBomb(bot);
        bot->setDirection(Direction::None);
        return;
    }

    // 4) Fallback: af en toe van richting veranderen, willekeurig rondlopen.
    if (bot->getDirection() == Direction::None || Random::instance().getDouble01() < 0.02) {
        int r = Random::instance().getInt(0, 3);
        bot->setDirection(r == 0 ? Direction::Up : r == 1 ? Direction::Down
                          : r == 2 ? Direction::Left : Direction::Right);
    }
}

// ---------------- Hoofdlus van de spellogica ----------------

void World::update(double deltaTime) {
    if (gameOver_) return;

    if (pendingPlayerBomb_) {
        placeBomb(player_);
        pendingPlayerBomb_ = false;
    }

    tryMoveCharacter(*player_, deltaTime);
    for (std::size_t i = 1; i < characters_.size(); ++i) {
        // AI aan: elke bot beslist zelf (vluchten voor gevaar, powerup grijpen,
        // bom plaatsen bij muur/vijand, anders random rondlopen).
        updateBotAI(characters_[i], deltaTime);
        tryMoveCharacter(*characters_[i], deltaTime);
    }

    updateBombs(deltaTime);

    // Power-ups oprapen.
    for (auto& c : characters_) {
        if (!c->isAlive()) continue;
        for (auto& p : powerUps_) {
            if (c->intersects(*p)) {
                c->applyPowerUp(p->getType());
                bool isPlayer = (c == player_);
                p->collect();
                if (isPlayer) notify(Event{EventType::PowerUpCollected, p.get()});
                break;
            }
        }
    }

    if (player_->isAlive()) {
        notify(Event{EventType::Tick, player_.get(), deltaTime});
    }

    checkWinCondition();
    removeDeadEntities();
}

void World::checkWinCondition() {
    if (gameOver_) return;

    if (!player_->isAlive()) {
        gameOver_ = true;
        playerWon_ = false;
        return;
    }

    bool anyBotAlive = false;
    for (std::size_t i = 1; i < characters_.size(); ++i) {
        if (characters_[i]->isAlive()) anyBotAlive = true;
    }
    if (!anyBotAlive) {
        gameOver_ = true;
        playerWon_ = true;
        notify(Event{EventType::PlayerWon, player_.get()});
    }
}

void World::removeDeadEntities() {
    walls_.erase(std::remove_if(walls_.begin(), walls_.end(),
        [](const std::shared_ptr<Wall>& w) { return w->isMarkedForRemoval(); }), walls_.end());
    bombs_.erase(std::remove_if(bombs_.begin(), bombs_.end(),
        [](const std::shared_ptr<Bomb>& b) { return b->isMarkedForRemoval(); }), bombs_.end());
    powerUps_.erase(std::remove_if(powerUps_.begin(), powerUps_.end(),
        [](const std::shared_ptr<PowerUp>& p) { return p->isMarkedForRemoval(); }), powerUps_.end());
}

} // namespace bomberman