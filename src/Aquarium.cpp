#include "Aquarium.h"
#include <cstdlib>
#include <algorithm> // for std::find
#include <cmath>     // for std::sqrt

// --- helper: small separation to prevent sticking when bumping ---
static void nudgeApart(std::shared_ptr<Creature> a, std::shared_ptr<Creature> b) {
    if (!a || !b) return;
    float dx = a->getX() - b->getX();
    float dy = a->getY() - b->getY();
    float len2 = dx*dx + dy*dy;
    if (len2 == 0.0f) { dx = 1.0f; dy = 0.0f; len2 = 1.0f; }
    const float inv = 1.0f / std::sqrt(len2);
    dx *= inv; dy *= inv;
    const float push = 4.0f;
    a->setPosition(a->getX() + dx*push, a->getY() + dy*push);
    b->setPosition(b->getX() - dx*push, b->getY() - dy*push);
}

string AquariumCreatureTypeToString(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return "BiggerFish";
        case AquariumCreatureType::NPCreature:
            return "BaseFish";
        default:
            return "UknownFish";
    }
}

// PlayerCreature Implementation
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
    this->bounce();
}

void PlayerCreature::reduceDamageDebounce() {
    if (m_damage_debounce > 0) {
        --m_damage_debounce;
    }
}

void PlayerCreature::update() {
    this->reduceDamageDebounce();
    this->move();
}

void PlayerCreature::draw() const {
    ofLogVerbose() << "PlayerCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    if (this->m_damage_debounce > 0) {
        ofSetColor(ofColor::red); // Flash red if in damage debounce
    }
    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }
    ofSetColor(ofColor::white); // Reset color
}

void PlayerCreature::changeSpeed(int speed) { m_speed = speed; }

void PlayerCreature::loseLife(int debounce) {
    if (m_damage_debounce <= 0) {
        if (m_lives > 0) this->m_lives -= 1;
        m_damage_debounce = debounce; // Set debounce frames
        ofLogNotice() << "Player lost a life! Lives remaining: " << m_lives << std::endl;
    }
    if (m_damage_debounce > 0) {
        ofLogVerbose() << "Player is in damage debounce period. Frames left: " << m_damage_debounce << std::endl;
    }
}

// NPCreature Implementation
NPCreature::NPCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 30, 1, sprite) {
    m_dx = (rand() % 3 - 1); // -1, 0, or 1
    m_dy = (rand() % 3 - 1); // -1, 0, or 1
    normalize();
    m_creatureType = AquariumCreatureType::NPCreature;
}

void NPCreature::move() {
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    if (m_dx < 0)  this->m_sprite->setFlipped(true);
    else           this->m_sprite->setFlipped(false);
    bounce();
}

void NPCreature::draw() const {
    ofLogVerbose() << "NPCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    ofSetColor(ofColor::white);
    if (m_sprite) m_sprite->draw(m_x, m_y);
}

// BiggerFish
BiggerFish::BiggerFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite) {
    m_dx = (rand() % 3 - 1);
    m_dy = (rand() % 3 - 1);
    normalize();
    setCollisionRadius(60);
    m_value = 5;
    m_creatureType = AquariumCreatureType::BiggerFish;
}

void BiggerFish::move() {
    m_x += m_dx * (m_speed * 0.5f);
    m_y += m_dy * (m_speed * 0.5f);
    if (m_dx < 0) this->m_sprite->setFlipped(true);
    else          this->m_sprite->setFlipped(false);
    bounce();
}

void BiggerFish::draw() const {
    ofLogVerbose() << "BiggerFish at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    this->m_sprite->draw(this->m_x, this->m_y);
}

// AquariumSpriteManager
AquariumSpriteManager::AquariumSpriteManager(){
    this->m_npc_fish = std::make_shared<GameSprite>("base-fish.png", 70, 70);
    this->m_big_fish = std::make_shared<GameSprite>("bigger-fish.png", 120, 120);
}

std::shared_ptr<GameSprite> AquariumSpriteManager::GetSprite(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:  return std::make_shared<GameSprite>(*this->m_big_fish);
        case AquariumCreatureType::NPCreature:  return std::make_shared<GameSprite>(*this->m_npc_fish);
        default:                                return nullptr;
    }
}

// Aquarium
Aquarium::Aquarium(int width, int height, std::shared_ptr<AquariumSpriteManager> spriteManager)
: m_width(width), m_height(height) {
    m_sprite_manager = spriteManager;
}

void Aquarium::addCreature(std::shared_ptr<Creature> creature) {
    creature->setBounds(m_width - 20, m_height - 20);
    m_creatures.push_back(creature);
}

void Aquarium::addAquariumLevel(std::shared_ptr<AquariumLevel> level){
    if (!level) return;
    this->m_aquariumlevels.push_back(level);
}

void Aquarium::update() {
    // Move everyone
    for (auto& creature : m_creatures) {
        creature->move();
    }
    // OPTIONAL: NPC-vs-NPC gentle relief to avoid piles (comment out if undesired)
    /*
    for (size_t i = 0; i < m_creatures.size(); ++i) {
        for (size_t j = i + 1; j < m_creatures.size(); ++j) {
            auto &a = m_creatures[i];
            auto &b = m_creatures[j];
            if (checkCollision(a, b)) nudgeApart(a, b);
        }
    }
    */
    this->Repopulate();
}

void Aquarium::draw() const {
    for (const auto& creature : m_creatures) {
        creature->draw();
    }
}

void Aquarium::removeCreature(std::shared_ptr<Creature> creature) {
    auto it = std::find(m_creatures.begin(), m_creatures.end(), creature);
    if (it != m_creatures.end()) {
        ofLogVerbose() << "removing creature " << std::endl;
        int selectLvl = this->currentLevel % this->m_aquariumlevels.size();
        auto npcCreature = std::static_pointer_cast<NPCreature>(creature);
        this->m_aquariumlevels.at(selectLvl)->ConsumePopulation(npcCreature->GetType(), npcCreature->getValue());
        m_creatures.erase(it);
    }
}

void Aquarium::clearCreatures() { m_creatures.clear(); }

std::shared_ptr<Creature> Aquarium::getCreatureAt(int index) {
    if (index < 0 || size_t(index) >= m_creatures.size()) return nullptr;
    return m_creatures[index];
}

void Aquarium::SpawnCreature(AquariumCreatureType type) {
    int x = rand() % this->getWidth();
    int y = rand() % this->getHeight();
    int speed = 1 + rand() % 25;

    switch (type) {
        case AquariumCreatureType::NPCreature:
            this->addCreature(std::make_shared<NPCreature>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::NPCreature)));
            break;
        case AquariumCreatureType::BiggerFish:
            this->addCreature(std::make_shared<BiggerFish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::BiggerFish)));
            break;
        default:
            ofLogError() << "Unknown creature type to spawn!";
            break;
    }
}

void Aquarium::Repopulate() {
    ofLogVerbose("entering phase repopulation");
    int selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
    ofLogVerbose() << "the current index: " << selectedLevelIdx << std::endl;
    std::shared_ptr<AquariumLevel> level = this->m_aquariumlevels.at(selectedLevelIdx);

    if (level->isCompleted()) {
        level->levelReset();
        this->currentLevel += 1;
        selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
        ofLogNotice() << "new level reached : " << selectedLevelIdx << std::endl;
        level = this->m_aquariumlevels.at(selectedLevelIdx);
        this->clearCreatures();
    }

    std::vector<AquariumCreatureType> toRespawn = level->Repopulate();
    ofLogVerbose() << "amount to repopulate : " << toRespawn.size() << std::endl;
    if (toRespawn.empty()) return;
    for (AquariumCreatureType newCreatureType : toRespawn) {
        this->SpawnCreature(newCreatureType);
    }
}

// Player-vs-NPC collision detection
std::shared_ptr<GameEvent> DetectAquariumCollisions(std::shared_ptr<Aquarium> aquarium,
                                                    std::shared_ptr<PlayerCreature> player) {
    if (!aquarium || !player) return nullptr;
    for (int i = 0; i < aquarium->getCreatureCount(); ++i) {
        std::shared_ptr<Creature> npc = aquarium->getCreatureAt(i);
        if (npc && checkCollision(player, npc)) {
            return std::make_shared<GameEvent>(GameEventType::COLLISION, player, npc);
        }
    }
    return nullptr;
}

// AquariumGameScene
void AquariumGameScene::Update(){
    std::shared_ptr<GameEvent> event;
    this->m_player->update();

    if (this->updateControl.tick()) {
        event = DetectAquariumCollisions(this->m_aquarium, this->m_player);
        if (event != nullptr && event->isCollisionEvent()) {
            ofLogVerbose() << "Collision detected between player and NPC!" << std::endl;
            if (event->creatureB != nullptr) {
                event->print();
                if (this->m_player->getPower() < event->creatureB->getValue()) {
                    ofLogNotice() << "Player is too weak to eat the creature!" << std::endl;
                    nudgeApart(this->m_player, event->creatureB); // bump feedback
                    this->m_player->loseLife(3*60);               // 3 seconds @60fps
                    if (this->m_player->getLives() <= 0) {
                        this->m_lastEvent = std::make_shared<GameEvent>(GameEventType::GAME_OVER, this->m_player, nullptr);
                        return;
                    }
                } else {
                    this->m_aquarium->removeCreature(event->creatureB);
                    this->m_player->addToScore(1, event->creatureB->getValue());
                    if (this->m_player->getScore() % 25 == 0) {
                        this->m_player->increasePower(1);
                        ofLogNotice() << "Player power increased to " << this->m_player->getPower() << "!" << std::endl;
                    }
                }
            } else {
                ofLogError() << "Error: creatureB is null in collision event." << std::endl;
            }
        }
        this->m_aquarium->update();
    }
}

void AquariumGameScene::Draw() {
    this->m_player->draw();
    this->m_aquarium->draw();
    this->paintAquariumHUD();
}

void AquariumGameScene::paintAquariumHUD(){
    float panelWidth = ofGetWindowWidth() - 150;
    ofDrawBitmapString("Score: " + std::to_string(this->m_player->getScore()), panelWidth, 20);
    ofDrawBitmapString("Power: " + std::to_string(this->m_player->getPower()), panelWidth, 30);
    ofDrawBitmapString("Lives: " + std::to_string(this->m_player->getLives()), panelWidth, 40);
    for (int i = 0; i < this->m_player->getLives(); ++i) {
        ofSetColor(ofColor::red);
        ofDrawCircle(panelWidth + i * 20, 50, 5);
    }
    ofSetColor(ofColor::white);
}

// AquariumLevel impls
void AquariumLevel::populationReset(){
    for(auto node: this->m_levelPopulation){
        node->currentPopulation = 0;
    }
}

void AquariumLevel::ConsumePopulation(AquariumCreatureType creatureType, int power){
    for(std::shared_ptr<AquariumLevelPopulationNode> node: this->m_levelPopulation){
        ofLogVerbose() << "consuming from this level creatures" << std::endl;
        if(node->creatureType == creatureType){
            ofLogVerbose() << "-consuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << std::endl;
            if(node->currentPopulation == 0) return;
            node->currentPopulation -= 1;
            ofLogVerbose() << "+consuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << std::endl;
            this->m_level_score += power;
            return;
        }
    }
}

bool AquariumLevel::isCompleted(){ return this->m_level_score >= this->m_targetScore; }

std::vector<AquariumCreatureType> Level_0::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate;
    for (auto node : this->m_levelPopulation){
        int delta = node->population - node->currentPopulation;
        ofLogVerbose() << "to Repopulate :  " << delta << std::endl;
        if (delta > 0){
            for (int i = 0; i < delta; ++i) toRepopulate.push_back(node->creatureType);
            node->currentPopulation += delta;
        }
    }
    return toRepopulate;
}

std::vector<AquariumCreatureType> Level_1::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate;
    for (auto node : this->m_levelPopulation){
        int delta = node->population - node->currentPopulation;
        if (delta > 0){
            for (int i = 0; i < delta; ++i) toRepopulate.push_back(node->creatureType);
            node->currentPopulation += delta;
        }
    }
    return toRepopulate;
}

std::vector<AquariumCreatureType> Level_2::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate;
    for (auto node : this->m_levelPopulation){
        int delta = node->population - node->currentPopulation;
        if (delta > 0){
            for (int i = 0; i < delta; ++i) toRepopulate.push_back(node->creatureType);
            node->currentPopulation += delta;
        }
    }
    return toRepopulate;
}



