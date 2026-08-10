#include "Views.h"
#include <cmath>
#include <algorithm>

using namespace bomberman;

// ---------------- EntityView (basis) ----------------

EntityView::EntityView(std::weak_ptr<EntityModel> model, const Camera& camera)
    : model_(model), camera_(camera) {}

void EntityView::onNotify(const Event& e) {
    if (e.type == EventType::Removed) removed_ = true;
}

bool EntityView::isExpired() const { return removed_ || model_.expired(); }

// ---------------- WallView ----------------

WallView::WallView(std::weak_ptr<Wall> model, const Camera& camera, const sf::Texture& texture)
    : EntityView(model, camera), wall_(model), texture_(&texture) {}

void WallView::draw(sf::RenderWindow& window) {
    auto w = wall_.lock();
    if (!w) return;

    Vec2 p = camera_.worldToScreen(w->getPosition());
    Vec2 s = camera_.worldToScreenSize(w->getSize());
    float x = static_cast<float>(p.x - s.x / 2.0);
    float y = static_cast<float>(p.y - s.y / 2.0);
    float w_ = static_cast<float>(s.x);
    float h_ = static_cast<float>(s.y);

    if (!w->isDestructible()) {
        // Onverwoestbare muur: grijze steen met een lichte 3D-rand (bevel),
        // zoals de klassieke SNES-pilaren.
        sf::RectangleShape base(sf::Vector2f(w_, h_));
        base.setPosition(x, y);
        base.setFillColor(sf::Color(120, 122, 128));
        window.draw(base);

        sf::RectangleShape topEdge(sf::Vector2f(w_, h_ * 0.18f));
        topEdge.setPosition(x, y);
        topEdge.setFillColor(sf::Color(160, 162, 168));
        window.draw(topEdge);

        sf::RectangleShape leftEdge(sf::Vector2f(w_ * 0.12f, h_));
        leftEdge.setPosition(x, y);
        leftEdge.setFillColor(sf::Color(150, 152, 158));
        window.draw(leftEdge);

        sf::RectangleShape bottomEdge(sf::Vector2f(w_, h_ * 0.14f));
        bottomEdge.setPosition(x, y + h_ * 0.86f);
        bottomEdge.setFillColor(sf::Color(80, 82, 88));
        window.draw(bottomEdge);

        sf::RectangleShape rightEdge(sf::Vector2f(w_ * 0.1f, h_));
        rightEdge.setPosition(x + w_ * 0.9f, y);
        rightEdge.setFillColor(sf::Color(90, 92, 98));
        window.draw(rightEdge);
    } else {
        // Breekbare muur: warm bruin blok met baksteen-voegen, net als
        // de destructible blokken in klassieke Bomberman-levels.
        sf::RectangleShape base(sf::Vector2f(w_, h_));
        base.setPosition(x, y);
        base.setFillColor(sf::Color(176, 120, 64));
        window.draw(base);

        sf::RectangleShape topHighlight(sf::Vector2f(w_, h_ * 0.15f));
        topHighlight.setPosition(x, y);
        topHighlight.setFillColor(sf::Color(200, 148, 92));
        window.draw(topHighlight);

        sf::RectangleShape shadow(sf::Vector2f(w_, h_ * 0.15f));
        shadow.setPosition(x, y + h_ * 0.85f);
        shadow.setFillColor(sf::Color(130, 84, 40));
        window.draw(shadow);

        // Baksteen-voegen: 2 horizontale lijnen, verspringende verticale lijnen.
        sf::Color mortar(120, 74, 34);
        for (int row = 1; row <= 2; ++row) {
            sf::RectangleShape line(sf::Vector2f(w_, 1.5f));
            line.setPosition(x, y + h_ * (row / 3.0f));
            line.setFillColor(mortar);
            window.draw(line);
        }
        sf::RectangleShape vLine1(sf::Vector2f(1.5f, h_ / 3.0f));
        vLine1.setPosition(x + w_ * 0.5f, y);
        vLine1.setFillColor(mortar);
        window.draw(vLine1);

        sf::RectangleShape vLine2(sf::Vector2f(1.5f, h_ / 3.0f));
        vLine2.setPosition(x + w_ * 0.25f, y + h_ / 3.0f);
        vLine2.setFillColor(mortar);
        window.draw(vLine2);
        sf::RectangleShape vLine3(sf::Vector2f(1.5f, h_ / 3.0f));
        vLine3.setPosition(x + w_ * 0.75f, y + h_ / 3.0f);
        vLine3.setFillColor(mortar);
        window.draw(vLine3);

        sf::RectangleShape vLine4(sf::Vector2f(1.5f, h_ / 3.0f));
        vLine4.setPosition(x + w_ * 0.5f, y + 2.0f * h_ / 3.0f);
        vLine4.setFillColor(mortar);
        window.draw(vLine4);
    }
}

// ---------------- PowerUpView ----------------

PowerUpView::PowerUpView(std::weak_ptr<PowerUp> model, const Camera& camera, const sf::Texture& texture)
    : EntityView(model, camera), powerUp_(model), texture_(&texture) {}

void PowerUpView::draw(sf::RenderWindow& window) {
    auto p = powerUp_.lock();
    if (!p) return;

    Vec2 pos = camera_.worldToScreen(p->getPosition());
    Vec2 s = camera_.worldToScreenSize(p->getSize());

    // Coördinaten van de iconen in de spritesheet (rij, kolom van 32x32-vakjes).
    int col = 0, row = 0;
    switch (p->getType()) {
        case PowerUpType::Fire:      col = 0; row = 22; break; // vuur-icoon
        case PowerUpType::ExtraBomb: col = 1; row = 22; break; // bom-icoon
        case PowerUpType::Skates:    col = 2; row = 22; break; // skates-icoon
        case PowerUpType::Poison:    col = 3; row = 22; break; // paars flesje
        case PowerUpType::Star:      col = 4; row = 22; break; // ster
        case PowerUpType::Shield:    col = 0; row = 23; break; // schild
        case PowerUpType::Curse:     col = 2; row = 23; break; // rood beestje
        case PowerUpType::Slow:      col = 3; row = 23; break; // zwart stekelig
        case PowerUpType::Freeze:    col = 2; row = 24; break; // blauwe vlam
        case PowerUpType::Skull:     col = 4; row = 24; break; // schedel
    }

    sf::Sprite sprite;
    sprite.setTexture(*texture_);
    sprite.setTextureRect(sf::IntRect(col * 32, row * 32, 32, 32));
    sprite.setOrigin(16.f, 16.f);
    sprite.setPosition(static_cast<float>(pos.x), static_cast<float>(pos.y));
    // Iets kleiner dan een volle tegel, zodat het duidelijk een oppakbaar object is.
    float scale = static_cast<float>(std::min(s.x, s.y)) / 32.f * 1.3f;
    sprite.setScale(scale, scale);
    window.draw(sprite);
}

// ---------------- DoorView ----------------

DoorView::DoorView(std::weak_ptr<Door> model, const Camera& camera)
    : EntityView(model, camera), door_(model) {}

void DoorView::draw(sf::RenderWindow& window) {
    auto d = door_.lock();
    if (!d) return;

    Vec2 p = camera_.worldToScreen(d->getPosition());
    Vec2 s = camera_.worldToScreenSize(d->getSize());
    float x = static_cast<float>(p.x - s.x / 2.0);
    float y = static_cast<float>(p.y - s.y / 2.0);
    float w_ = static_cast<float>(s.x);
    float h_ = static_cast<float>(s.y);

    // Zachte gouden gloed erachter, zodat de deur meteen opvalt tussen de
    // muren/vloertegels wanneer ze net vrijkomt.
    pulse_ += (1.0 / 60.0) * 3.0;
    float glowAlpha = 90.f + 50.f * static_cast<float>(std::sin(pulse_));
    sf::RectangleShape glow(sf::Vector2f(w_ * 1.25f, h_ * 1.25f));
    glow.setPosition(x - w_ * 0.125f, y - h_ * 0.125f);
    glow.setFillColor(sf::Color(255, 210, 90, static_cast<sf::Uint8>(glowAlpha)));
    window.draw(glow);

    // Houten deurkozijn (donkerbruin).
    sf::RectangleShape frame(sf::Vector2f(w_, h_));
    frame.setPosition(x, y);
    frame.setFillColor(sf::Color(90, 58, 30));
    window.draw(frame);

    // Deurpaneel: donkere opening waar de speler "doorheen" loopt.
    sf::RectangleShape panel(sf::Vector2f(w_ * 0.72f, h_ * 0.82f));
    panel.setPosition(x + w_ * 0.14f, y + h_ * 0.15f);
    panel.setFillColor(sf::Color(35, 22, 14));
    window.draw(panel);

    // Boogvormige top: klein rechthoekje bovenaan, geeft een poort-silhouet.
    sf::RectangleShape arch(sf::Vector2f(w_ * 0.5f, h_ * 0.18f));
    arch.setPosition(x + w_ * 0.25f, y + h_ * 0.06f);
    arch.setFillColor(sf::Color(60, 38, 20));
    window.draw(arch);

    // Gouden deurklink.
    sf::CircleShape handle(std::max(1.5f, w_ * 0.06f));
    handle.setPosition(x + w_ * 0.68f, y + h_ * 0.52f);
    handle.setFillColor(sf::Color(255, 210, 90));
    window.draw(handle);
}

// ---------------- BombView ----------------

BombView::BombView(std::weak_ptr<Bomb> model, const Camera& camera, const sf::Texture& texture)
    : EntityView(model, camera), bomb_(model), texture_(&texture) {}

void BombView::onNotify(const Event& e) {
    if (e.type == EventType::BombExploded) {
        exploding_ = true;
        explodeAnimTimer_ = 0.0;
        // Laatste geldige positie/grootte cachen: het Model verdwijnt vlak hierna.
        if (auto b = bomb_.lock()) {
            lastPos_ = camera_.worldToScreen(b->getPosition());
            lastSize_ = camera_.worldToScreenSize(b->getSize());
        }
    }
    EntityView::onNotify(e); // laat de basisklasse 'Removed' nog steeds verwerken
}

bool BombView::isExpired() const {
    constexpr double EXPLODE_ANIM_TIME = 0.35;
    if (exploding_) return explodeAnimTimer_ >= EXPLODE_ANIM_TIME;
    return EntityView::isExpired();
}

void BombView::draw(sf::RenderWindow& window) {
    constexpr double EXPLODE_ANIM_TIME = 0.35;

    if (exploding_) {
        // ---- Ontploffings-animatie: 4 groeiende burst-frames uit de spritesheet ----
        explodeAnimTimer_ += 1.0 / 60.0;
        int frame = std::min(3, static_cast<int>(explodeAnimTimer_ / (EXPLODE_ANIM_TIME / 4.0)));

        sf::Sprite sprite;
        sprite.setTexture(*texture_);
        sprite.setTextureRect(sf::IntRect((10 + frame) * 32, 22 * 32, 32, 32));
        sprite.setOrigin(16.f, 16.f);
        sprite.setPosition(static_cast<float>(lastPos_.x), static_cast<float>(lastPos_.y));
        // Iets groter dan de tegel zelf, zodat de burst goed "uitdeint".
        float scale = static_cast<float>(std::max(lastSize_.x, lastSize_.y)) / 32.f * 1.6f;
        sprite.setScale(scale, scale);
        window.draw(sprite);
        return;
    }

    auto b = bomb_.lock();
    if (!b) return;

    Vec2 pos = camera_.worldToScreen(b->getPosition());
    Vec2 s = camera_.worldToScreenSize(b->getSize());
    lastPos_ = pos;
    lastSize_ = s;

    // ---- Tikkende bom: vaste sprite, met een simpele "ademende" puls i.p.v. frame-swap.
    // Puls versnelt naarmate de lont korter wordt.
    double urgency = b->getFuseTime() > 0.0 ? (b->getTimer() / b->getFuseTime()) : 0.0; // 0..1
    pulse_ += (1.0 / 60.0) * (4.0 + urgency * 8.0);
    float breathe = 1.0f + 0.12f * static_cast<float>(std::sin(pulse_));

    sf::Sprite sprite;
    sprite.setTexture(*texture_);
    sprite.setTextureRect(sf::IntRect(7 * 32, 22 * 32, 32, 32)); // vaste bom-sprite
    sprite.setOrigin(16.f, 16.f);
    sprite.setPosition(static_cast<float>(pos.x), static_cast<float>(pos.y));
    float scale = static_cast<float>(std::min(s.x, s.y)) / 32.f * 1.4f * breathe;
    sprite.setScale(scale, scale);
    window.draw(sprite);
}

// ---------------- CharacterView ----------------

CharacterView::CharacterView(std::weak_ptr<Character> model, const Camera& camera,
                              const sf::Texture& texture, int rowBase, int colBase)
    : EntityView(model, camera), character_(model), texture_(&texture), rowBase_(rowBase), colBase_(colBase) {}

void CharacterView::draw(sf::RenderWindow& window) {
    auto c = character_.lock();
    if (!c || !c->isAlive()) return;

    Vec2 pos = camera_.worldToScreen(c->getPosition());
    Vec2 s = camera_.worldToScreenSize(c->getSize());

    Direction dir = c->getDirection();
    bool moving = (dir != Direction::None);
    if (moving) lastDirection_ = dir; // laatste bewogen richting onthouden voor idle-pose

    // Animatie voortgang: enkel de frame doorschuiven als het personage effectief beweegt.
    constexpr double FRAME_TIME = 0.12; // seconden per animatie-frame
    constexpr int FRAME_COUNT = 4;
    if (moving) {
        animTimer_ += 1.0 / 60.0; // grove tick-benadering; geen deltaTime beschikbaar in draw()
        if (animTimer_ >= FRAME_TIME) {
            animTimer_ = 0.0;
            frame_ = (frame_ + 1) % FRAME_COUNT;
        }
    } else {
        frame_ = 0; // stilstaand = eerste frame van de laatste richting (nette "idle" pose)
    }

    // Rij-offset per richting: 0 = naar beneden, 1 = naar boven, 2 = naar links (rechts = zelfde rij, gespiegeld).
    int rowOffset = 0;
    bool flip = false;
    switch (lastDirection_) {
        case Direction::Down:  rowOffset = 0; break;
        case Direction::Up:    rowOffset = 1; break;
        case Direction::Left:  rowOffset = 2; break;
        case Direction::Right: rowOffset = 2; flip = true; break;
        default:                rowOffset = 0; break;
    }
    int row = rowBase_ + rowOffset;
    int col = colBase_ + frame_;

    sf::Sprite sprite;
    sprite.setTexture(*texture_);
    sprite.setTextureRect(sf::IntRect(col * 32, row * 32, 32, 32));
    sprite.setOrigin(16.f, 16.f);
    sprite.setPosition(static_cast<float>(pos.x), static_cast<float>(pos.y));

    float scaleX = static_cast<float>(s.x) / 32.f;
    float scaleY = static_cast<float>(s.y) / 32.f;
    if (flip) scaleX = -scaleX; // spiegel horizontaal voor "naar rechts lopen"
    sprite.setScale(scaleX, scaleY);

    window.draw(sprite);
}