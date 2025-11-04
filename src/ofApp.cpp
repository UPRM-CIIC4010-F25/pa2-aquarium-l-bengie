#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetFrameRate(60);
    ofSetBackgroundColor(ofColor::blue);
    backgroundImage.load("background.png");
    backgroundImage.resize(ofGetWindowWidth(), ofGetWindowHeight());

    std::shared_ptr<Aquarium> myAquarium;
    std::shared_ptr<PlayerCreature> player;

    // Scene Manager
    gameManager = std::make_unique<GameSceneManager>();

    // INTRO SCENE
    gameManager->AddScene(std::make_shared<GameIntroScene>(
        GameSceneKindToString(GameSceneKind::GAME_INTRO),
        std::make_shared<GameSprite>("title.png",
            ofGetWindowWidth(), ofGetWindowHeight())
    ));

    // SPRITE MANAGER
    spriteManager = std::make_shared<AquariumSpriteManager>();

    // AQUARIUM + PLAYER
    myAquarium = std::make_shared<Aquarium>(
        ofGetWindowWidth(), ofGetWindowHeight(), spriteManager
    );

    player = std::make_shared<PlayerCreature>(
        ofGetWindowWidth()/2 - 50,
        ofGetWindowHeight()/2 - 50,
        DEFAULT_SPEED,
        spriteManager->GetSprite(AquariumCreatureType::NPCreature)
    );

    player->setDirection(0, 0);
    player->setBounds(ofGetWindowWidth() - 20, ofGetWindowHeight() - 20);

    // LEVELS
    myAquarium->addAquariumLevel(std::make_shared<Level_0>(0, 10));
    myAquarium->addAquariumLevel(std::make_shared<Level_1>(1, 15));
    myAquarium->addAquariumLevel(std::make_shared<Level_2>(2, 20));
    myAquarium->Repopulate();

    // GAME SCENE
    gameManager->AddScene(std::make_shared<AquariumGameScene>(
        std::move(player),
        std::move(myAquarium),
        GameSceneKindToString(GameSceneKind::AQUARIUM_GAME)
    ));

    // GAME OVER SCENE
    gameManager->AddScene(std::make_shared<GameOverScene>(
        GameSceneKindToString(GameSceneKind::GAME_OVER),
        std::make_shared<GameSprite>("game-over.png",
            ofGetWindowWidth(), ofGetWindowHeight())
    ));

    // Font setup
    gameOverTitle.load("Verdana.ttf", 12, true, true);
    gameOverTitle.setLineHeight(34.0f);
    gameOverTitle.setLetterSpacing(1.035);

    ofSetLogLevel(OF_LOG_NOTICE);

    // ✅ Ambient Underwater Loop
    if (!ambientUnderwater.load("underwater.wav")) {
        ofLogWarning() << "Could not load underwater.wav (place it in bin/data).";
    } else {
        ambientUnderwater.setLoop(true);
        ambientUnderwater.setMultiPlay(false);
        ambientUnderwater.setVolume(0.35f);
        ambientUnderwater.play();
    }
}

//--------------------------------------------------------------
void ofApp::update(){

    if (gameManager->GetActiveSceneName() ==
        GameSceneKindToString(GameSceneKind::GAME_OVER))
        return;

    if (gameManager->GetActiveSceneName() ==
        GameSceneKindToString(GameSceneKind::AQUARIUM_GAME))
    {
        auto gameScene = std::static_pointer_cast<AquariumGameScene>(
            gameManager->GetActiveScene()
        );

        if (gameScene->GetLastEvent() != nullptr &&
            gameScene->GetLastEvent()->isGameOver())
        {
            gameManager->Transition(GameSceneKindToString(
                GameSceneKind::GAME_OVER));
            return;
        }
    }

    gameManager->UpdateActiveScene();
    ofSoundUpdate();
}

//--------------------------------------------------------------
void ofApp::draw(){
    backgroundImage.draw(0, 0);
    gameManager->DrawActiveScene();
}

//--------------------------------------------------------------
void ofApp::exit(){ }

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    if (lastEvent.isGameExit()) {
        ofLogNotice() << "Game ended. Press ESC to exit.";
        return;
    }

    if (gameManager->GetActiveSceneName() ==
        GameSceneKindToString(GameSceneKind::AQUARIUM_GAME))
    {
        auto scene = std::static_pointer_cast<AquariumGameScene>(
            gameManager->GetActiveScene()
        );

        switch (key) {
            case OF_KEY_UP:    scene->GetPlayer()->setDirection(0, -1); break;
            case OF_KEY_DOWN:  scene->GetPlayer()->setDirection(0,  1); break;
            case OF_KEY_LEFT:
                scene->GetPlayer()->setDirection(-1, 0);
                scene->GetPlayer()->setFlipped(true);
                break;
            case OF_KEY_RIGHT:
                scene->GetPlayer()->setDirection(1, 0);
                scene->GetPlayer()->setFlipped(false);
                break;
        }

        scene->GetPlayer()->move();
        return;
    }

    if (gameManager->GetActiveSceneName() ==
        GameSceneKindToString(GameSceneKind::GAME_INTRO))
    {
        if (key == OF_KEY_SPACE) {
            gameManager->Transition(GameSceneKindToString(
                GameSceneKind::AQUARIUM_GAME));
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){ }

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){ }
void ofApp::mouseDragged(int x, int y, int button){ }
void ofApp::mousePressed(int x, int y, int button){ }
void ofApp::mouseReleased(int x, int y, int button){ }
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){ }
void ofApp::mouseEntered(int x, int y){ }
void ofApp::mouseExited(int x, int y){ }

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    backgroundImage.resize(w, h);

    auto scene = std::static_pointer_cast<AquariumGameScene>(
        gameManager->GetScene(GameSceneKindToString(GameSceneKind::AQUARIUM_GAME))
    );

    scene->GetAquarium()->setBounds(w, h);
    scene->GetPlayer()->setBounds(w - 20, h - 20);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){ }
void ofApp::dragEvent(ofDragInfo dragInfo){ }
