#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <stdexcept>
#include <exception>
#include "gamestate.h"
#include "shop.h"
#include <filesystem>
#include <memory>

// 保留算符重載
inline std::ostream& operator<<(std::ostream& os, const GameState& state) {
    switch(state) {
        case GameState::MainMenu: os << "MainMenu"; break;
        case GameState::Gameplay: os << "Gameplay"; break;
        case GameState::PauseScreen: os << "PauseScreen"; break;
        case GameState::Instruction: os << "Instruction"; break;
        case GameState::FirstCutScene: os << "FirstCutScene"; break;
        case GameState::levels: os << "Levels"; break;
        case GameState::Stage1: os << "Stage1"; break;
        case GameState::Stage2: os << "Stage2"; break;
        case GameState::Stage3: os << "Stage3"; break;
        case GameState::Shop: os << "Shop"; break;
        case GameState::Stage2CutScene: os << "Stage2CutScene"; break;
        case GameState::Stage3CutScene: os << "Stage3CutScene"; break;
        case GameState::RrroCutScene: os << "RrroCutScene"; break;
        case GameState::StoreCutScene: os << "StoreCutScene"; break;
    }
    return os;
}

// 添加一個 Position 結構體來處理位置
struct Position {
    float x, y;
    
    Position(float x = 0, float y = 0) : x(x), y(y) {}
    
    Position operator+(const Position& other) const {
        return Position(x + other.x, y + other.y);
    }
    
    Position operator-(const Position& other) const {
        return Position(x - other.x, y - other.y);
    }
    
    Position operator*(float scale) const {
        return Position(x * scale, y * scale);
    }
    
    Position& operator+=(const Position& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
    
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
    
    operator sf::Vector2f() const {
        return sf::Vector2f(x, y);
    }
};

// 1. 資源載入的異常處理
class ResourceLoadException : public std::runtime_error {
public:
    ResourceLoadException(const std::string& message) 
        : std::runtime_error("Resource Load Error: " + message) {}
};

// 2. 遊戲邏輯的異常處理
class GameLogicException : public std::runtime_error {
public:
    GameLogicException(const std::string& message) 
        : std::runtime_error("Game Logic Error: " + message) {}
};

std::vector<sf::Texture> rrroCutSceneTextures;
sf::Sprite rrroCutSceneSprite;
size_t currentRrroCutSceneFrame = 0;
std::vector<sf::Texture> presidentCutSceneTextures;
sf::Sprite presidentCutSceneSprite;
size_t currentPresidentCutSceneFrame = 0;
std::vector<sf::Texture> storeCutSceneTextures;
sf::Sprite storeCutSceneSprite;
size_t currentStoreCutSceneFrame = 0;
std::vector<sf::Texture> waterpapaTextures;
sf::Sprite waterpapaCutSceneSprite;
size_t currentWaterpapaFrame = 0;

// 在文件開頭添加一個態變數
static bool hasSeenStoreCutScene = false;

int main() {
    sf::RenderWindow window(sf::VideoMode(1200, 800), "Game");
    
    try {
        // 空的try塊，用於捕獲可能的異常
    } catch (const std::exception& e) {
        std::cerr << "Failed to create window: " << e.what() << std::endl;
        return -1;
    }

    // Game State
    GameState currentState = GameState::MainMenu;
    GameState previousState = GameState::MainMenu;

    // --- Main Menu Setup ---
    sf::Texture backgroundTexture;
    try {
        if (!backgroundTexture.loadFromFile("/Users/cpcap/GTA6/source/assets_pictures/mainMenu.png")) {
            throw ResourceLoadException("無法加載主菜單背景圖片");
        }
    } catch (const ResourceLoadException& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    sf::Sprite background(backgroundTexture);
    background.setScale(
        window.getSize().x / static_cast<float>(backgroundTexture.getSize().x),
        window.getSize().y / static_cast<float>(backgroundTexture.getSize().y)
    );

    // --- Instruction Setup ---
    sf::Texture instructionBackTexture;
    if (!instructionBackTexture.loadFromFile("/Users/cpcap/GTA6/source/assets_pictures/gameExplanation.png"))
        return -1;
    sf::Sprite instructionBackground(instructionBackTexture);
    instructionBackground.setScale(
        window.getSize().x / static_cast<float>(instructionBackTexture.getSize().x),
        window.getSize().y / static_cast<float>(instructionBackTexture.getSize().y)
    );

    // --- Levels Setup ---
    sf::Texture levelsBackTexture;
    if (!levelsBackTexture.loadFromFile("/Users/cpcap/GTA6/source/assets_pictures/levels.png"))
        return -1;
    sf::Sprite levelsBackground(levelsBackTexture);
    levelsBackground.setScale(
        window.getSize().x / static_cast<float>(levelsBackTexture.getSize().x),
        window.getSize().y / static_cast<float>(levelsBackTexture.getSize().y)
    );
    
    // Define hitboxes for each stage
    sf::Texture textureStage1, textureStage2, textureStage3, textureShop;
    if (!textureStage1.loadFromFile("/Users/cpcap/GTA6/source/assets_pictures/stage1.png") ||
        !textureStage2.loadFromFile("/Users/cpcap/GTA6/source/assets_pictures/stage2.png") ||
        !textureStage3.loadFromFile("/Users/cpcap/GTA6/source/assets_pictures/stage3.png") ||
        !textureShop.loadFromFile("/Users/cpcap/GTA6/source/assets_pictures/shop.png")){
        return -1;
    }
    
    sf::Sprite spriteStage1(textureStage1);
    spriteStage1.setPosition(205, 54);
    sf::Sprite spriteStage2(textureStage2);
    spriteStage2.setPosition(763, 261);
    sf::Sprite spriteStage3(textureStage3);
    spriteStage3.setPosition(375, 478);
    sf::Sprite spriteShop(textureShop);
    spriteShop.setPosition(580, 152);
    
    sf::Font font;
    try {
        if (!font.loadFromFile("/Users/cpcap/GTA6/source/arial.ttf")) {
            throw ResourceLoadException("Failed to load font");
        }
    } catch (const ResourceLoadException& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return -1;
    }

    const sf::Vector2f buttonSize(300, 60);
    sf::RectangleShape startGameButton(buttonSize);
    startGameButton.setFillColor(sf::Color(100, 200, 100));
    startGameButton.setOutlineColor(sf::Color::Black);
    startGameButton.setOutlineThickness(3);
    startGameButton.setPosition(
        (window.getSize().x - buttonSize.x) / 2,
        window.getSize().y - buttonSize.y - 50
    );

    sf::Text startGameText("Go To Levels!", font, 30);
    startGameText.setFillColor(sf::Color::Black);
    startGameText.setPosition(
        startGameButton.getPosition().x + (buttonSize.x - startGameText.getLocalBounds().width) / 2,
        startGameButton.getPosition().y + (buttonSize.y - startGameText.getLocalBounds().height) / 2 - 5
    );
    sf::Text goText("GO!!!", font, 30);
    goText.setFillColor(sf::Color::Black);
    goText.setPosition(
        startGameButton.getPosition().x + (buttonSize.x - goText.getLocalBounds().width) / 2,
        startGameButton.getPosition().y + (buttonSize.y - goText.getLocalBounds().height) / 2 - 5
    );

    // --- Pause Screen Setup ---
    sf::Text pauseText("Game Paused", font, 60);
    pauseText.setFillColor(sf::Color::Black);
    pauseText.setPosition(400, 150);

    sf::RectangleShape resumeButton(buttonSize);
    resumeButton.setFillColor(sf::Color(100, 200, 100));
    resumeButton.setOutlineColor(sf::Color::Black);
    resumeButton.setOutlineThickness(2);
    resumeButton.setPosition(450, 300);

    sf::Text resumeText("Resume", font, 30);
    resumeText.setFillColor(sf::Color::Black);
    resumeText.setPosition(
        resumeButton.getPosition().x + (buttonSize.x - resumeText.getLocalBounds().width) / 2,
        resumeButton.getPosition().y + (buttonSize.y - resumeText.getLocalBounds().height) / 2 - 5
    );

    sf::RectangleShape exitButton(buttonSize);
    exitButton.setFillColor(sf::Color(200, 100, 100));
    exitButton.setOutlineColor(sf::Color::Black);
    exitButton.setOutlineThickness(2);
    exitButton.setPosition(450, 400);

    sf::Text exitText("Exit to Menu", font, 30);
    exitText.setFillColor(sf::Color::Black);
    exitText.setPosition(
        exitButton.getPosition().x + (buttonSize.x - exitText.getLocalBounds().width) / 2,
        exitButton.getPosition().y + (buttonSize.y - exitText.getLocalBounds().height) / 2 - 5
    );

    // --- Gameplay Setup ---
    sf::RectangleShape rectangle(sf::Vector2f(100, 50));
    rectangle.setFillColor(sf::Color::Blue);
    rectangle.setPosition(600, 375);

    // Load sound
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile("/Users/cpcap/GTA6/source/assets_audio/click.wav"))
        return -1;
    sf::Sound buttonSound;
    buttonSound.setBuffer(buffer);

    // --- FirstCutScene Setup ---
    std::vector<sf::Texture> cutSceneTextures;
    sf::Sprite cutSceneSprite;
    size_t currentCutSceneFrame = 0;

    // 加载 FirstCutScene 图片
    try {
        for (int i = 2; i <= 21; i++) {
            sf::Texture texture;
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "/Users/cpcap/GTA6/source/First_Cutscene/%d.png", i);
            std::string path = buffer;
            
            sf::Texture newTexture;
            if (!newTexture.loadFromFile(path)) {
                throw ResourceLoadException("無法加載切換場景圖片: " + path);
            }
            cutSceneTextures.push_back(newTexture);
        }
    } catch (const ResourceLoadException& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    if (!cutSceneTextures.empty()) {
        cutSceneSprite.setTexture(cutSceneTextures[0]);
        cutSceneSprite.setScale(
            window.getSize().x / static_cast<float>(cutSceneTextures[0].getSize().x),
            window.getSize().y / static_cast<float>(cutSceneTextures[0].getSize().y)
        );
    } else {
        std::cout << "No cutscene textures were loaded!" << std::endl;
    }

    // 載入 RrroCutScene 的圖片
    for (int i = 24; i <= 40; i++) {
        sf::Texture texture;
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "/Users/cpcap/GTA6/source/rrro_Cutscene/%d.png", i);
        std::string path = buffer;
        
        if (!texture.loadFromFile(path)) {
            std::cout << "Failed to load: " << path << std::endl;
            continue;
        }
        sf::Texture newTexture;
        if (newTexture.loadFromFile(path)) {
            rrroCutSceneTextures.push_back(newTexture);
            std::cout << "Successfully loaded: " << path << std::endl;
        }
    }

    if (!rrroCutSceneTextures.empty()) {
        rrroCutSceneSprite.setTexture(rrroCutSceneTextures[0]);
        rrroCutSceneSprite.setScale(
            window.getSize().x / static_cast<float>(rrroCutSceneTextures[0].getSize().x),
            window.getSize().y / static_cast<float>(rrroCutSceneTextures[0].getSize().y)
        );
    }

    // 載入 PresidentCutScene 的圖片
    for (int i = 56; i <= 71; i++) {
        sf::Texture texture;
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "/Users/cpcap/GTA6/source/president_Cutscene/%d.png", i);
        std::string path = buffer;
        
        if (!texture.loadFromFile(path)) {
            std::cout << "Failed to load: " << path << std::endl;
            continue;
        }
        sf::Texture newTexture;
        if (newTexture.loadFromFile(path)) {
            presidentCutSceneTextures.push_back(newTexture);
            std::cout << "Successfully loaded: " << path << std::endl;
        }
    }

    if (!presidentCutSceneTextures.empty()) {
        presidentCutSceneSprite.setTexture(presidentCutSceneTextures[0]);
        presidentCutSceneSprite.setScale(
            window.getSize().x / static_cast<float>(presidentCutSceneTextures[0].getSize().x),
            window.getSize().y / static_cast<float>(presidentCutSceneTextures[0].getSize().y)
        );
    }

    // 載入 StoreCutScene 的圖片
    for (int i = 2; i <= 3; i++) {
        sf::Texture texture;
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "/Users/cpcap/GTA6/source/store_Cutscene/%d.png", i);
        std::string path = buffer;
        
        if (!texture.loadFromFile(path)) {
            std::cout << "Failed to load: " << path << std::endl;
            continue;
        }
        sf::Texture newTexture;
        if (newTexture.loadFromFile(path)) {
            storeCutSceneTextures.push_back(newTexture);
            std::cout << "Successfully loaded: " << path << std::endl;
        }
    }

    if (!storeCutSceneTextures.empty()) {
        storeCutSceneSprite.setTexture(storeCutSceneTextures[0]);
        storeCutSceneSprite.setScale(
            window.getSize().x / static_cast<float>(storeCutSceneTextures[0].getSize().x),
            window.getSize().y / static_cast<float>(storeCutSceneTextures[0].getSize().y)
        );
    }

    // 加载 waterpapa_Cutscene 的图片
    for (int i = 42; i <= 54; i++) {
        sf::Texture texture;
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "/Users/cpcap/GTA6/source/waterpapa_Cutscene/%d.png", i);
        std::string path = buffer;
        
        if (!texture.loadFromFile(path)) {
            std::cout << "Failed to load: " << path << std::endl;
            continue;
        }
        sf::Texture newTexture;
        if (newTexture.loadFromFile(path)) {
            waterpapaTextures.push_back(newTexture);
            std::cout << "Successfully loaded: " << path << std::endl;
        }
    }

    if (!waterpapaTextures.empty()) {
        waterpapaCutSceneSprite.setTexture(waterpapaTextures[0]);
        waterpapaCutSceneSprite.setScale(
            window.getSize().x / static_cast<float>(waterpapaTextures[0].getSize().x),
            window.getSize().y / static_cast<float>(waterpapaTextures[0].getSize().y)
        );
    }

    while (window.isOpen()) {
        sf::Event event;
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            switch (currentState) {
                case GameState::MainMenu:
                    if (event.type == sf::Event::MouseButtonPressed) {
                        if (startGameButton.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
                            currentState = GameState::Instruction;
                            buttonSound.play();
                        }
                    }
                    break;

                case GameState::Instruction:
                    if (event.type == sf::Event::MouseButtonPressed) {
                        if (startGameButton.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
                            currentState = GameState::FirstCutScene;
                            currentCutSceneFrame = 0;
                            if (!cutSceneTextures.empty()) {
                                cutSceneSprite.setTexture(cutSceneTextures[0]);
                                cutSceneSprite.setScale(
                                    window.getSize().x / static_cast<float>(cutSceneTextures[0].getSize().x),
                                    window.getSize().y / static_cast<float>(cutSceneTextures[0].getSize().y)
                                );
                            }
                            buttonSound.play();
                        }
                    }
                    break;

                case GameState::levels:
                    if (event.type == sf::Event::MouseButtonPressed) {
                        if (spriteStage1.getGlobalBounds().contains(mousePosF.x, mousePosF.y)) {
                            currentState = GameState::Stage1CutScene;
                            currentCutSceneFrame = 0;
                            buttonSound.play();
                        }
                        else if (spriteStage2.getGlobalBounds().contains(mousePosF.x, mousePosF.y)) {
                            currentState = GameState::Stage2CutScene;
                            currentPresidentCutSceneFrame = 0;
                            if (!presidentCutSceneTextures.empty()) {
                                presidentCutSceneSprite.setTexture(presidentCutSceneTextures[0]);
                                presidentCutSceneSprite.setScale(
                                    window.getSize().x / static_cast<float>(presidentCutSceneTextures[0].getSize().x),
                                    window.getSize().y / static_cast<float>(presidentCutSceneTextures[0].getSize().y)
                                );
                            }
                            buttonSound.play();
                        }
                        else if (spriteStage3.getGlobalBounds().contains(mousePosF.x, mousePosF.y)) {
                            currentState = GameState::RrroCutScene;
                            buttonSound.play();
                        }
                        else if (spriteShop.getGlobalBounds().contains(mousePosF.x, mousePosF.y)) {
                            if (!hasSeenStoreCutScene) {
                                currentState = GameState::StoreCutScene;
                                currentStoreCutSceneFrame = 0;
                                if (!storeCutSceneTextures.empty()) {
                                    storeCutSceneSprite.setTexture(storeCutSceneTextures[0]);
                                    storeCutSceneSprite.setScale(
                                        window.getSize().x / static_cast<float>(storeCutSceneTextures[0].getSize().x),
                                        window.getSize().y / static_cast<float>(storeCutSceneTextures[0].getSize().y)
                                    );
                                }
                                hasSeenStoreCutScene = true;
                            } else {
                                currentState = GameState::Shop;
                            }
                            buttonSound.play();
                        }
                    }  
                    break;

                case GameState::Gameplay:
                    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                        previousState = currentState;
                        currentState = GameState::PauseScreen;
                        buttonSound.play();
                    }
                    break;

                case GameState::PauseScreen:
                    if (event.type == sf::Event::MouseButtonPressed) {
                        if (resumeButton.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
                            currentState = GameState::Gameplay;
                            buttonSound.play();
                        } else if (exitButton.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
                            currentState = GameState::levels;
                            buttonSound.play();
                        }
                    }
                    if (event.type == sf::Event::KeyPressed) {
                        if (event.key.code == sf::Keyboard::Escape) {
                            if (previousState == GameState::Stage1 || 
                                previousState == GameState::Stage2 || 
                                previousState == GameState::Stage3) {
                                currentState = previousState;
                            }
                        }
                    }
                    break;

                case GameState::FirstCutScene:
                    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                        currentCutSceneFrame++;
                        if (currentCutSceneFrame >= cutSceneTextures.size()) {
                            currentState = GameState::levels;
                            currentCutSceneFrame = 0;
                        } else {
                            cutSceneSprite.setTexture(cutSceneTextures[currentCutSceneFrame]);
                            cutSceneSprite.setScale(
                                window.getSize().x / static_cast<float>(cutSceneTextures[currentCutSceneFrame].getSize().x),
                                window.getSize().y / static_cast<float>(cutSceneTextures[currentCutSceneFrame].getSize().y)
                            );
                        }
                    }
                    break;

                case GameState::Stage2CutScene:
                    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                        currentPresidentCutSceneFrame++;
                        if (currentPresidentCutSceneFrame >= presidentCutSceneTextures.size()) {
                            window.close();
                            
                            // 執行 president
                            std::string command = "/Users/cpcap/GTA6/president";
                            std::cout << "Executing: " << command << std::endl;
                            int result = system(command.c_str());
                            
                            if (result != 0) {
                                std::cerr << "Failed to execute president (error code: " << result << ")" << std::endl;
                                return -1;
                            }
                            return 0;
                        } else {
                            presidentCutSceneSprite.setTexture(presidentCutSceneTextures[currentPresidentCutSceneFrame]);
                            presidentCutSceneSprite.setScale(
                                window.getSize().x / static_cast<float>(presidentCutSceneTextures[currentPresidentCutSceneFrame].getSize().x),
                                window.getSize().y / static_cast<float>(presidentCutSceneTextures[currentPresidentCutSceneFrame].getSize().y)
                            );
                        }
                    }
                    break;


                case GameState::RrroCutScene:
                    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                        currentRrroCutSceneFrame++;
                        if (currentRrroCutSceneFrame >= rrroCutSceneTextures.size()) {
                            window.close();
                            
                            // 執行 rrro
                            std::string command = "/Users/cpcap/GTA6/rrro";
                            std::cout << "Executing: " << command << std::endl;
                            int result = system(command.c_str());
                            
                            if (result != 0) {
                                std::cerr << "Failed to execute rrro (error code: " << result << ")" << std::endl;
                                return -1;
                            }
                            return 0;
                        } else {
                            rrroCutSceneSprite.setTexture(rrroCutSceneTextures[currentRrroCutSceneFrame]);
                            rrroCutSceneSprite.setScale(
                                window.getSize().x / static_cast<float>(rrroCutSceneTextures[currentRrroCutSceneFrame].getSize().x),
                                window.getSize().y / static_cast<float>(rrroCutSceneTextures[currentRrroCutSceneFrame].getSize().y)
                            );
                        }
                    }
                    break;


                case GameState::StoreCutScene:
                    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                        currentStoreCutSceneFrame++;
                        if (currentStoreCutSceneFrame >= storeCutSceneTextures.size()) {
                            currentState = GameState::Shop;
                            currentStoreCutSceneFrame = 0;
                        } else {
                            storeCutSceneSprite.setTexture(storeCutSceneTextures[currentStoreCutSceneFrame]);
                            storeCutSceneSprite.setScale(
                                window.getSize().x / static_cast<float>(storeCutSceneTextures[currentStoreCutSceneFrame].getSize().x),
                                window.getSize().y / static_cast<float>(storeCutSceneTextures[currentStoreCutSceneFrame].getSize().y)
                            );
                        }
                    }
                    break;

                case GameState::Shop: {
                    Shop shop;
                    shop.run(window, currentState);
                    break;
                }

                case GameState::Stage1CutScene:
                    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                        currentWaterpapaFrame++;
                        if (currentWaterpapaFrame >= waterpapaTextures.size()) {
                            window.close();
                            
                            std::string command = "/Users/cpcap/GTA6/waterpapa";
                            std::cout << "Executing: " << command << std::endl;
                            int result = system(command.c_str());
                            
                            if (result != 0) {
                                std::cerr << "Failed to execute waterpapa (error code: " << result << ")" << std::endl;
                                return -1;
                            }
                            return 0;
                        } else {
                            waterpapaCutSceneSprite.setTexture(waterpapaTextures[currentWaterpapaFrame]);
                            waterpapaCutSceneSprite.setScale(
                                window.getSize().x / static_cast<float>(waterpapaTextures[currentWaterpapaFrame].getSize().x),
                                window.getSize().y / static_cast<float>(waterpapaTextures[currentWaterpapaFrame].getSize().y)
                            );
                        }
                    }
                    break;
            }
        }

        // Rendering based on current state
        window.clear();
        sf::RectangleShape pauseOverlay(sf::Vector2f(window.getSize().x, window.getSize().y));
        pauseOverlay.setFillColor(sf::Color(0, 0, 0, 128));

        switch (currentState) {
            case GameState::MainMenu:
                window.draw(background);
                window.draw(startGameButton);
                window.draw(goText);
                break;

            case GameState::Instruction:
                window.draw(instructionBackground);
                window.draw(startGameButton);
                window.draw(startGameText);
                break;

            case GameState::levels:
                window.draw(levelsBackground);
                window.draw(spriteStage1);
                window.draw(spriteStage2);
                window.draw(spriteStage3);
                window.draw(spriteShop);
                break;

            case GameState::Gameplay:
                window.draw(rectangle);
                break;

            case GameState::PauseScreen: {
                switch (previousState) {
                    case GameState::Stage1:
                        break;
                    case GameState::Stage2:
                        break;
                    case GameState::Stage3:
                        break;
                }
                
                window.draw(pauseOverlay);
                window.draw(pauseText);
                window.draw(resumeButton);
                window.draw(resumeText);
                window.draw(exitButton);
                window.draw(exitText);
                break;
            }

            case GameState::FirstCutScene:
                window.draw(cutSceneSprite);
                break;

            case GameState::Stage1:
                break;
                
            case GameState::Stage2:
                break;
                
            case GameState::Stage3:
                break;
                
            case GameState::Shop:
                break;
                
            case GameState::RrroCutScene:
                if (!rrroCutSceneTextures.empty()) {
                    window.draw(rrroCutSceneSprite);
                }
                break;

            case GameState::Stage2CutScene:
                if (!presidentCutSceneTextures.empty()) {
                    window.draw(presidentCutSceneSprite);
                }
                break;

            case GameState::StoreCutScene:
                if (!storeCutSceneTextures.empty()) {
                    window.draw(storeCutSceneSprite);
                }
                break;

            case GameState::Stage1CutScene:
                window.draw(waterpapaCutSceneSprite);
                break;
        }
        window.display();
    }

    return 0;
}