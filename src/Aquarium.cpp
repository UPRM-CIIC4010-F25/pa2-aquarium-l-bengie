#include "Aquarium.h"
#include <cstdlib>
#include <algorithm>
#include <cmath>

/* --------------------------------------------
   helper (prevents overlapping creatures)
-------------------------------------------- */
static void nudgeApart(std::shared_ptr<Creature> a, std::shared_ptr<Creature> b)
{
    if (!a || !b) return;

    float dx = a->getX() - b->getX();
    float dy = a->getY() - b->getY();
    float len2 = dx * dx + dy * dy;

    if (len2 == 0) { dx = 1; dy = 0; len2 = 1; }

    float inv = 1.0f / std::sqrt(len2);
    float push = 4.0f;

    a->setPosition(a->getX() + dx * inv * push, a->getY() + dy * inv * push);
    b->setPosition(b->getX() - dx * inv * push, b->getY() - dy * inv * push);
}

/* --------------------------------------------
   String helper
-------------------------------------------- */
string AquariumCreatureTypeToString(AquariumCreatureType t)
{
    switch (t)
    {
    case AquariumCreatureType::NPCreature: return "NPCreature";
    case AquariumCreatureType::BiggerFish: return "BiggerFish";
    case AquariumCreatureType::FastFish:   return "FastFish";
    case AquariumCreatureType::ZigZagFish: return "ZigZagFish";
    case AquariumCreatureType::PowerUp:    return "PowerUp";
    default: return "Unknown";
    }
}

/* --------------------------------------------
   Player
-------------------------------------------- */
PlayerCreature::PlayerCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
    : Creature(x, y, speed, 10.0f, 1, sprite) {}

void PlayerCreature::setDirection(float dx, float dy) {
    m_dx = dx;
    m_dy = dy;
    normalize();
}

void PlayerCreature::move() {
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    bounce();
}

void PlayerCreature::reduceDamageDebounce() {
    if (m_damage_debounce > 0)
        --m_damage_debounce;
}

void PlayerCreature::update() {
    reduceDamageDebounce();
    move();
}

void PlayerCreature::draw() const {
    if (m_damage_debounce > 0)
        ofSetColor(ofColor::red);
    else if (m_power > 1)
        ofSetColor(ofColor::yellow); // glow effect when power > 1

    if (m_sprite) m_sprite->draw(m_x, m_y);
    ofSetColor(ofColor::white);
}

void PlayerCreature::changeSpeed(int speed) {
    m_speed = speed;
}

void PlayerCreature::loseLife(int debounce) {
    if (m_damage_debounce <= 0) {
        if (m_lives > 0) m_lives--;
        m_damage_debounce = debounce;
    }
}

/* --------------------------------------------
   NPC base creature
-------------------------------------------- */
NPCreature::NPCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
    : Creature(x, y, speed, 30, 1, sprite)
{
    m_dx = (rand() % 3 - 1);
    m_dy = (rand() % 3 - 1);
    normalize();
    m_creatureType = AquariumCreatureType::NPCreature;
}

void NPCreature::move() {
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    setFlipped(m_dx < 0);
    bounce();
}

void NPCreature::draw() const {
    if (m_sprite) m_sprite->draw(m_x, m_y);
}

/* --------------------------------------------
   Bigger Fish
-------------------------------------------- */
BiggerFish::BiggerFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
    : NPCreature(x, y, speed, sprite)
{
    setCollisionRadius(60);
    m_value = 5;
    m_creatureType = AquariumCreatureType::BiggerFish;
}

void BiggerFish::move() {
    m_x += m_dx * (m_speed * 0.5f);
    m_y += m_dy * (m_speed * 0.5f);
    setFlipped(m_dx < 0);
    bounce();
}

void BiggerFish::draw() const {
    if (m_sprite) m_sprite->draw(m_x, m_y);
}

/* --------------------------------------------
   FastFish (definitions match header)
-------------------------------------------- */
FastFish::FastFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
    : NPCreature(x, y, speed * 2, sprite) {}

void FastFish::move() {
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;

    if (rand() % 10 == 0) {
        m_dx = (rand() % 3 - 1);
        m_dy = (rand() % 3 - 1);
        normalize();
    }

    setFlipped(m_dx < 0);
    bounce();
}

/* --------------------------------------------
   ZigZagFish (definitions match header)
-------------------------------------------- */
ZigZagFish::ZigZagFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
    : NPCreature(x, y, speed, sprite) {}

void ZigZagFish::move() {
    frameCounter++;
    if (frameCounter % 20 == 0)
        m_dx = -m_dx;

    m_x += m_dx * m_speed;
    m_y += m_speed * 0.6f;
    setFlipped(m_dx < 0);
    bounce();
}

/* --------------------------------------------
   Sprite Manager
-------------------------------------------- */
AquariumSpriteManager::AquariumSpriteManager()
{
    m_npc_fish    = std::make_shared<GameSprite>("base-fish.png",   70, 70);
    m_big_fish    = std::make_shared<GameSprite>("bigger-fish.png", 120, 120);
    m_fast_fish   = std::make_shared<GameSprite>("fast-fish.png",   60, 60);
    m_zigzag_fish = std::make_shared<GameSprite>("zigzag-fish.png", 60, 60);
    m_powerup     = std::make_shared<GameSprite>("powerup.png",     45, 45);
}

std::shared_ptr<GameSprite> AquariumSpriteManager::GetSprite(AquariumCreatureType t)
{
    switch (t)
    {
    case AquariumCreatureType::NPCreature: return std::make_shared<GameSprite>(*m_npc_fish);
    case AquariumCreatureType::BiggerFish: return std::make_shared<GameSprite>(*m_big_fish);
    case AquariumCreatureType::FastFish:   return std::make_shared<GameSprite>(*m_fast_fish);
    case AquariumCreatureType::ZigZagFish: return std::make_shared<GameSprite>(*m_zigzag_fish);
    case AquariumCreatureType::PowerUp:    return std::make_shared<GameSprite>(*m_powerup);
    default: return nullptr;
    }
}

/* --------------------------------------------
   Aquarium Controller
-------------------------------------------- */
Aquarium::Aquarium(int width, int height, std::shared_ptr<AquariumSpriteManager> spriteManager)
    : m_width(width), m_height(height), m_sprite_manager(spriteManager) {}

void Aquarium::addCreature(std::shared_ptr<Creature> creature)
{
    creature->setBounds(m_width - 20, m_height - 20);
    m_creatures.push_back(creature);
}

void Aquarium::addAquariumLevel(std::shared_ptr<AquariumLevel> level)
{
    if (level) m_aquariumlevels.push_back(level);
}

void Aquarium::update()
{
    for (auto& c : m_creatures)
        c->move();

    Repopulate();
}

void Aquarium::draw() const
{
    for (const auto& c : m_creatures)
        c->draw();
}

void Aquarium::removeCreature(std::shared_ptr<Creature> creature)
{
    m_creatures.erase(std::remove(m_creatures.begin(), m_creatures.end(), creature), m_creatures.end());
}

std::shared_ptr<Creature> Aquarium::getCreatureAt(int index)
{
    if (index < 0 || index >= (int)m_creatures.size()) return nullptr;
    return m_creatures[index];
}

void Aquarium::SpawnCreature(AquariumCreatureType type)
{
    int x = rand() % m_width;
    int y = rand() % m_height;
    int speed = 1 + rand() % 25;

    switch (type) {
        case AquariumCreatureType::NPCreature:
            addCreature(std::make_shared<NPCreature>(x, y, speed, m_sprite_manager->GetSprite(type)));
            break;
        case AquariumCreatureType::BiggerFish:
            addCreature(std::make_shared<BiggerFish>(x, y, speed, m_sprite_manager->GetSprite(type)));
            break;
        case AquariumCreatureType::FastFish:
            addCreature(std::make_shared<FastFish>(x, y, speed, m_sprite_manager->GetSprite(type)));
            break;
        case AquariumCreatureType::ZigZagFish:
            addCreature(std::make_shared<ZigZagFish>(x, y, speed, m_sprite_manager->GetSprite(type)));
            break;
        case AquariumCreatureType::PowerUp:
            addCreature(std::make_shared<PowerUp>(x, y, m_sprite_manager->GetSprite(type)));
            break;
    }
}

void Aquarium::Repopulate()
{
    if (m_aquariumlevels.empty()) return;

    auto level = m_aquariumlevels[currentLevel % m_aquariumlevels.size()];

    if (level->isCompleted()) {
        level->levelReset();
        currentLevel++;
        clearCreatures();
        return;
    }

    for (auto t : level->Repopulate())
        SpawnCreature(t);
}

void Aquarium::clearCreatures()
{
    m_creatures.clear();
}

/* --------------------------------------------
   Collision check
-------------------------------------------- */
std::shared_ptr<GameEvent> DetectAquariumCollisions(
    std::shared_ptr<Aquarium> aq,
    std::shared_ptr<PlayerCreature> player)
{
    if (!aq || !player) return nullptr;

    for (int i = 0; i < aq->getCreatureCount(); ++i) {
        auto npc = aq->getCreatureAt(i);
        if (npc && checkCollision(player, npc))
            return std::make_shared<GameEvent>(GameEventType::COLLISION, player, npc);
    }
    return nullptr;
}

/* --------------------------------------------
   Scene Logic
-------------------------------------------- */
void AquariumGameScene::Update()
{
    m_player->update();

    if (updateControl.tick())
    {
        auto event = DetectAquariumCollisions(m_aquarium, m_player);

        if (event && event->isCollisionEvent())
        {
            auto target = event->creatureB;

            // Power-up handling
            if (target->getCollisionRadius() == 25 && target->getValue() == 0) {
                m_player->increasePower(1);
                m_player->changeSpeed(m_player->getSpeed() + 2);
                m_aquarium->removeCreature(target);
            }
            else if (m_player->getPower() < target->getValue()) {
                nudgeApart(m_player, target);
                m_player->loseLife(3 * 60);
            }
            else {
                m_player->addToScore(1, target->getValue());
                m_aquarium->removeCreature(target);
            }
        }

        m_aquarium->update();
    }
}

void AquariumGameScene::Draw()
{
    m_player->draw();
    m_aquarium->draw();
    paintAquariumHUD();
}

void AquariumGameScene::paintAquariumHUD()
{
    float x = ofGetWindowWidth() - 150;
    ofDrawBitmapString("Score: " + std::to_string(m_player->getScore()), x, 20);
    ofDrawBitmapString("Power: " + std::to_string(m_player->getPower()), x, 30);
    ofDrawBitmapString("Lives: " + std::to_string(m_player->getLives()), x, 40);
}

/* --------------------------------------------
   Repopulate Implementations (Levels)
-------------------------------------------- */
std::vector<AquariumCreatureType> Level_0::Repopulate() {
    std::vector<AquariumCreatureType> list;
    for (auto node : m_levelPopulation) {
        int delta = node->population - node->currentPopulation;
        while (delta-- > 0) list.push_back(node->creatureType);
        node->currentPopulation = node->population;
    }
    return list;
}

std::vector<AquariumCreatureType> Level_1::Repopulate() {
    std::vector<AquariumCreatureType> list;
    for (auto node : m_levelPopulation) {
        int delta = node->population - node->currentPopulation;
        while (delta-- > 0) list.push_back(node->creatureType);
        node->currentPopulation = node->population;
    }
    return list;
}

std::vector<AquariumCreatureType> Level_2::Repopulate() {
    std::vector<AquariumCreatureType> list;
    for (auto node : m_levelPopulation) {
        int delta = node->population - node->currentPopulation;
        while (delta-- > 0) list.push_back(node->creatureType);
        node->currentPopulation = node->population;
    }
    return list;
}

/* --------------------------------------------
   Repopulate Implementations (Levels 3 & 4)
-------------------------------------------- */
std::vector<AquariumCreatureType> Level_3::Repopulate() {
    std::vector<AquariumCreatureType> list;
    for (auto node : m_levelPopulation) {
        int delta = node->population - node->currentPopulation;
        while (delta-- > 0) {
            list.push_back(node->creatureType);
        }
        node->currentPopulation = node->population;
    }
    return list;
}

std::vector<AquariumCreatureType> Level_4::Repopulate() {
    std::vector<AquariumCreatureType> list;
    for (auto node : m_levelPopulation) {
        int delta = node->population - node->currentPopulation;
        while (delta-- > 0) {
            list.push_back(node->creatureType);
        }
        node->currentPopulation = node->population;
    }
    return list;
}




