#pragma once
#define NOMINMAX

#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include "Core.h"

/*----------------------------------
  EXTENDED CREATURE TYPE ENUM
----------------------------------*/
enum class AquariumCreatureType {
    NPCreature,
    BiggerFish,
    FastFish,      // ✅ new species #1
    ZigZagFish,    // ✅ new species #2
    PowerUp        // ✅ floating collectible
};

string AquariumCreatureTypeToString(AquariumCreatureType t);

/*----------------------------------
        POWER-UP CLASS
----------------------------------*/
class PowerUp : public Creature {
public:
    PowerUp(float x, float y, std::shared_ptr<GameSprite> sprite)
        : Creature(x, y, 0, 25, 0, sprite) {}  // radius=25, value=0

    void move() override {} // stays still

    void draw() const override {
        ofSetColor(ofColor::yellow);
        if (m_sprite) m_sprite->draw(m_x, m_y);
        ofSetColor(ofColor::white);
    }
};

/*----------------------------------
      LEVEL POPULATION ENTRY
----------------------------------*/
class AquariumLevelPopulationNode {
public:
    AquariumLevelPopulationNode() = default;
    AquariumLevelPopulationNode(AquariumCreatureType type, int population) {
        this->creatureType = type;
        this->population = population;
        this->currentPopulation = 0;
    };
    AquariumCreatureType creatureType;
    int population;
    int currentPopulation;
};

/*----------------------------------
          BASE LEVEL CLASS
----------------------------------*/
class AquariumLevel : public GameLevel {
public:
    AquariumLevel(int levelNumber, int targetScore)
        : GameLevel(levelNumber), m_level_score(0), m_targetScore(targetScore) {};
    void ConsumePopulation(AquariumCreatureType creature, int power);
    bool isCompleted() override;
    void populationReset();
    void levelReset() { m_level_score = 0; populationReset(); }

    virtual std::vector<AquariumCreatureType> Repopulate();

protected:
    std::vector<std::shared_ptr<AquariumLevelPopulationNode>> m_levelPopulation;
    int m_level_score;
    int m_targetScore;
};

/*----------------------------------
      CREATURE CLASSES
----------------------------------*/
class PlayerCreature : public Creature {
public:
    PlayerCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);

    void move() override;
    void draw() const override;
    void update();
    void setDirection(float dx, float dy);
    void changeSpeed(int speed);
    void setLives(int lives) { m_lives = lives; }
    float isXDirectionActive() { return m_dx != 0; }
    float isYDirectionActive() {return m_dy != 0; }
    float getDx() { return m_dx; }
    float getDy() { return m_dy; }

    void addToScore(int amount, int weight = 1) { m_score += amount * weight; }
    void increasePower(int value) { m_power += value; }
    void reduceDamageDebounce();
    void loseLife(int debounce);

    int getScore() const { return m_score; }
    int getLives() const { return m_lives; }
    int getPower() const { return m_power; }

private:
    int m_score = 0;
    int m_lives = 3;
    int m_power = 1;
    int m_damage_debounce = 0;
};

class NPCreature : public Creature {
public:
    NPCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    AquariumCreatureType GetType() { return m_creatureType; }

    void move() override;
    void draw() const override;

protected:
    AquariumCreatureType m_creatureType;
};

class BiggerFish : public NPCreature {
public:
    BiggerFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    void move() override;
    void draw() const override;
};

/*----------------------------------
   ✅ NEW CREATURES HERE
----------------------------------*/
class FastFish : public NPCreature {
public:
    FastFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    void move() override;
};

class ZigZagFish : public NPCreature {
public:
    ZigZagFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    void move() override;

private:
    int frameCounter = 0;
};

/*----------------------------------
     SPRITE MANAGER
----------------------------------*/
class AquariumSpriteManager {
public:
    AquariumSpriteManager();
    ~AquariumSpriteManager() = default;

    std::shared_ptr<GameSprite> GetSprite(AquariumCreatureType t);

private:
    std::shared_ptr<GameSprite> m_npc_fish;
    std::shared_ptr<GameSprite> m_big_fish;
    std::shared_ptr<GameSprite> m_fast_fish;
    std::shared_ptr<GameSprite> m_zigzag_fish;
    std::shared_ptr<GameSprite> m_powerup;
};

/*----------------------------------
       AQUARIUM CONTROLLER
----------------------------------*/
class Aquarium {
public:
    Aquarium(int width, int height, std::shared_ptr<AquariumSpriteManager> spriteManager);

    void addCreature(std::shared_ptr<Creature> creature);
    void addAquariumLevel(std::shared_ptr<AquariumLevel> level);
    void removeCreature(std::shared_ptr<Creature> creature);
    void clearCreatures();
    void update();
    void draw() const;
    void Repopulate();

    void SpawnCreature(AquariumCreatureType type);
    std::shared_ptr<Creature> getCreatureAt(int index);

    int getCreatureCount() const { return m_creatures.size(); }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    void setBounds(int w, int h) { m_width = w; m_height = h; }

private:
    int m_maxPopulation = 0;
    int m_width;
    int m_height;
    int currentLevel = 0;

    std::vector<std::shared_ptr<Creature>> m_creatures;
    std::vector<std::shared_ptr<Creature>> m_next_creatures;
    std::vector<std::shared_ptr<AquariumLevel>> m_aquariumlevels;  // ✅ must match Aquarium.cpp
    std::shared_ptr<AquariumSpriteManager> m_sprite_manager;
};

/*----------------------------------
       SCENE CONTROLLER
----------------------------------*/
std::shared_ptr<GameEvent> DetectAquariumCollisions(
    std::shared_ptr<Aquarium> aquarium,
    std::shared_ptr<PlayerCreature> player);

class AquariumGameScene : public GameScene {
public:
    AquariumGameScene(std::shared_ptr<PlayerCreature> player,
                      std::shared_ptr<Aquarium> aquarium,
                      string name)
        : m_player(std::move(player)), m_aquarium(std::move(aquarium)), m_name(name) {}

    std::shared_ptr<GameEvent> GetLastEvent() { return m_lastEvent; }
    void SetLastEvent(std::shared_ptr<GameEvent> event) { m_lastEvent = event; }
    std::shared_ptr<PlayerCreature> GetPlayer() { return m_player; }
    std::shared_ptr<Aquarium> GetAquarium() { return m_aquarium; }

    string GetName() override { return m_name; }
    void Update() override;
    void Draw() override;

private:
    void paintAquariumHUD();
    AwaitFrames updateControl{5};

    std::shared_ptr<PlayerCreature> m_player;
    std::shared_ptr<Aquarium> m_aquarium;
    std::shared_ptr<GameEvent> m_lastEvent;
    string m_name;
};

/*----------------------------------
      LEVEL DEFINITIONS
----------------------------------*/
class Level_0 : public AquariumLevel {
public:
    Level_0(int n, int target) : AquariumLevel(n, target) {
        m_levelPopulation.push_back(
            std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::NPCreature, 10));
    }
    //std::vector<AquariumCreatureType> Repopulate() override;
};

class Level_1 : public AquariumLevel {
public:
    Level_1(int n, int target) : AquariumLevel(n, target) {
        m_levelPopulation.push_back(
            std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::FastFish, 12));
    }
    //std::vector<AquariumCreatureType> Repopulate() override;
};

class Level_2 : public AquariumLevel {
public:
    Level_2(int n, int target) : AquariumLevel(n, target) {
        m_levelPopulation.push_back(
            std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::ZigZagFish, 18));
        m_levelPopulation.push_back(
            std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::PowerUp, 2));
    }
    //std::vector<AquariumCreatureType> Repopulate() override;
};

class Level_3 : public AquariumLevel {
public:
    Level_3(int n, int target) : AquariumLevel(n, target) {
        m_levelPopulation.push_back(
            std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::FastFish, 20));
        m_levelPopulation.push_back(
            std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::BiggerFish, 10));
        m_levelPopulation.push_back(
            std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::PowerUp, 1));
    }
    //std::vector<AquariumCreatureType> Repopulate() override;
};

class Level_4 : public AquariumLevel {
public:
    Level_4(int n, int target) : AquariumLevel(n, target) {
        m_levelPopulation.push_back(
            std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::ZigZagFish, 25));
        m_levelPopulation.push_back(
            std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::PowerUp, 2));
        m_levelPopulation.push_back(
            std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::BiggerFish, 5));
    }
    //std::vector<AquariumCreatureType> Repopulate() override;
};

