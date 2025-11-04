#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetFrameRate(60);
    ofSetBackgroundColor(ofColor::blue);

    backgroundImage.load("background.png");
    backgroundImage.resize(ofGetWindowWidth(), ofGetWindowHeight());

    std::shared_ptr<Aquarium> myAquarium;
    std::shared_ptr<PlayerCreature> player;

    gameManager = std::make_unique<GameSceneManager>();

    gameManager->AddScene(std::make_shared<GameIntroScene>(
        GameSceneKindToString(GameSceneKind::GAME_INTRO),
        std::make_shared<GameSprite>("title.png",
            ofGetWindowWidth(), ofGetWindowHeight())
    ));

    spriteManager = std::make_shared<AquariumSpriteManager>();

    myAquarium = std::make_shared<Aquarium>(
        ofGetWindowWidth(),
        ofGetWindowHeight(),
        spriteManager
    );

    player = std::make_shared<PlayerCreature>(
        ofGetWindowWidth() / 2 - 50,
        ofGetWindowHeight() / 2 - 50,
        DEFAULT_SPEED,
        spriteManager->GetSprite(AquariumCreatureType::NPCreature)
    );

    player->setDirection(0, 0);
    player->setBounds(ofGetWindowWidth() - 20, ofGetWindowHeight() - 20);

    // ✅ Add progression levels
    myAquarium->addAquariumLevel(std::make_shared<Level_0>(0, 10));
    myAquarium->addAquariumLevel(std::make_shared<Level_1>(1, 15));
    myAquarium->addAquariumLevel(std::make_shared<Level_2>(2, 20));
    myAquarium->addAquariumLevel(std::make_shared<Level_3>(3, 30));
    myAquarium->addAquariumLevel(std::make_shared<Level_4>(4, 40));

    myAquarium->Repopulate();

    gameManager->AddScene(std::make_shared<AquariumGameScene>(
        std::move(player),
        std::move(myAquarium),
        GameSceneKindToString(GameSceneKind::AQUARIUM_GAME)
    ));

    gameManager->AddScene(std::make_shared<GameOverScene>(
        GameSceneKindToString(GameSceneKind::GAME_OVER),
        std::make_shared<GameSprite>("game-over.png",
            ofGetWindowWidth(), ofGetWindowHeight())
    ));

    ofSetLogLevel(OF_LOG_NOTICE);
}

//--------------------------------------------------------------
void ofApp::update(){

    if (gameManager->GetActiveSceneName() ==
        GameSceneKindToString(GameSceneKind::GAME_OVER))
        return;

    gameManager->UpdateActiveScene();
    ofSoundUpdate();
}

//--------------------------------------------------------------
void ofApp::draw(){
    backgroundImage.draw(0, 0);
    gameManager->DrawActiveScene();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    if (gameManager->GetActiveSceneName() ==
        GameSceneKindToString(GameSceneKind::AQUARIUM_GAME))
    {
        auto scene = std::static_pointer_cast<AquariumGameScene>(
            gameManager->GetActiveScene()
        );

        switch (key) {
            case OF_KEY_UP:    scene->GetPlayer()->setDirection(0, -1); break;
            case OF_KEY_DOWN:  scene->GetPlayer()->setDirection(0,  1); break;
            case OF_KEY_LEFT:  scene->GetPlayer()->setDirection(-1, 0); break;
            case OF_KEY_RIGHT: scene->GetPlayer()->setDirection(1,  0); break;
        }

        scene->GetPlayer()->move();
        return;
    }

    if (gameManager->GetActiveSceneName() ==
        GameSceneKindToString(GameSceneKind::GAME_INTRO))
    {
        if (key == OF_KEY_SPACE)
            gameManager->Transition(GameSceneKindToString(GameSceneKind::AQUARIUM_GAME));
    }
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    backgroundImage.resize(w, h);

    auto scene = std::static_pointer_cast<AquariumGameScene>(
        gameManager->GetScene(GameSceneKindToString(GameSceneKind::AQUARIUM_GAME))
    );

    scene->GetAquarium()->setBounds(w, h);
    scene->GetPlayer()->setBounds(w - 20, h - 20);
}

