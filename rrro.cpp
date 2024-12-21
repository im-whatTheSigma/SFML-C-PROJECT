#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>  // 為了使用 rand()
#include <ctime>    // 為了使用 time()
#include <iostream>  // 添加這行
#include <filesystem>  // 添加這行
#include <memory>  // 添加這行
//#include "bullet.h"
//#include "enemy.h"
//#include "animated_background.h"
//#include "bullet.h"
//#include "enemy.h"
//#include "animated_background.h"

using namespace sf;
using namespace std;

// 在檔案開頭定義全域常量
const float BOUNDARY_LEFT = 200.f;    // 左邊界
const float PLAY_AREA_WIDTH = 800.f;  // 遊戲區域寬度
const float ENEMY_WIDTH = 30.f;       // 敵人寬度
const float BOUNDARY_RIGHT = BOUNDARY_LEFT + PLAY_AREA_WIDTH;  // 右邊界

// 修改枚舉定義
enum class EnemyType {
    NORMAL = 1,    // 普通紅色敵人
    SKY = 2,       // 空中靜止敵人（原藍色敵人）
    HAND = 3,      // 手部快速敵人（原黑色敵人）
    UPHAND = 4,    // 黃色寬敵人
    LEFTHAND = 5,  // 黃色高敵人（從左往右）
    RIGHTHAND = 6  // 新增：黃色高敵人（從右往左）
};

class Bullet {
public:
    CircleShape shape;
    float speed;

    Bullet(float startX, float startY) {
        speed = 1.0f;
        shape.setRadius(5.f);
        shape.setFillColor(Color::Black);
        shape.setPosition(startX, startY);
    }

    void update() {
        shape.move(0, -speed);
    }
};

// 添加敵人類
class Enemy {
public:
    sf::RectangleShape shape;
    float speed;
    bool isIndestructible;
    EnemyType type;
    float damage;
    
    // 修改成員變量
    static sf::Texture skyTexture1;  // Sky1 材質
    static sf::Texture skyTexture2;  // Sky2 材質
    sf::Sprite skySprite1;          // Sky1 精靈
    sf::Sprite skySprite2;          // Sky2 精靈
    static bool texturesLoaded;
    sf::Clock skyImageTimer;
    bool showSky1;                  // 控制 Sky1 的顯示
    bool showSky2;                  // 控制 Sky2 的顯示

    // 添加新的靜態成員
    static sf::Texture bighandTexture;
    sf::Sprite bighandSprite;
    static bool bighandTextureLoaded;

    // 添加新的靜態成員
    static sf::Texture uphandTexture;
    sf::Sprite uphandSprite;
    static bool uphandTextureLoaded;

    // 添加新的靜態成員
    static sf::Texture lefthandTexture;
    sf::Sprite lefthandSprite;
    static bool lefthandTextureLoaded;

    Enemy(float startX, float startY, 
          const sf::Vector2f& size = sf::Vector2f(30.f, 30.f),
          const sf::Color& color = sf::Color::Red,
          bool indestructible = false,
          EnemyType enemyType = EnemyType::NORMAL)
        : isIndestructible(indestructible), type(enemyType), showSky1(true), showSky2(false) {
        
        if (type == EnemyType::SKY && !texturesLoaded) {
            bool sky1Loaded = skyTexture1.loadFromFile("/Users/cpcap/GTA6/source/boss_3_texture/character/Sky1.png");
            bool sky2Loaded = skyTexture2.loadFromFile("/Users/cpcap/GTA6/source/boss_3_texture/character/Sky2.png");
            if (sky1Loaded && sky2Loaded) {
                texturesLoaded = true;
            }
        }

        if (type == EnemyType::SKY && texturesLoaded) {
            // 設置 Sky1 精靈
            skySprite1.setTexture(skyTexture1);
            skySprite1.setOrigin(skyTexture1.getSize().x / 2.f, skyTexture1.getSize().y / 2.f);
            skySprite1.setPosition(startX, startY);
            float scaleX = 260.f / skyTexture1.getSize().x;
            float scaleY = 260.f / skyTexture1.getSize().y;
            skySprite1.setScale(scaleX, scaleY);

            // 設置 Sky2 精靈
            skySprite2.setTexture(skyTexture2);
            skySprite2.setOrigin(skyTexture2.getSize().x / 2.f, skyTexture2.getSize().y / 2.f);
            skySprite2.setPosition(startX, startY);
            scaleX = 260.f / skyTexture2.getSize().x;
            scaleY = 260.f / skyTexture2.getSize().y;
            skySprite2.setScale(scaleX, scaleY);

            skyImageTimer.restart();
        }

        // 在構造函數中添加 Bighand 材質載入
        if (type == EnemyType::HAND && !bighandTextureLoaded) {
            if (bighandTexture.loadFromFile("/Users/cpcap/GTA6/source/boss_3_texture/character/Bighand.png")) {
                bighandTextureLoaded = true;
            }
        }

        if (type == EnemyType::HAND && bighandTextureLoaded) {
            // 設置 Bighand 精靈
            bighandSprite.setTexture(bighandTexture);
            bighandSprite.setOrigin(bighandTexture.getSize().x / 2.f, bighandTexture.getSize().y / 2.f);
            
            // 設置初始位置與子彈一致
            bighandSprite.setPosition(startX, startY);
            
            // 設置大小為 200x100
            float scaleX = 200.f / bighandTexture.getSize().x;
            float scaleY = 100.f / bighandTexture.getSize().y;
            bighandSprite.setScale(scaleX, scaleY);
        }

        // 在構造函數中添加 Uphand 材質載入
        if (type == EnemyType::UPHAND && !uphandTextureLoaded) {
            if (uphandTexture.loadFromFile("/Users/cpcap/GTA6/source/boss_3_texture/character/Uphandpic.png")) {
                uphandTextureLoaded = true;
            }
        }

        if (type == EnemyType::UPHAND && uphandTextureLoaded) {
            // 設置 Uphand 精靈
            uphandSprite.setTexture(uphandTexture);
            uphandSprite.setOrigin(uphandTexture.getSize().x / 2.f, uphandTexture.getSize().y / 2.f);
            uphandSprite.setPosition(startX, startY);
            
            // 設置大小為 60x30
            float scaleX = 90.f / uphandTexture.getSize().x;
            float scaleY = 45.f / uphandTexture.getSize().y;
            uphandSprite.setScale(scaleX, scaleY);
        }

        // 在構造函數中添加 Lefthand 材質載入
        if ((type == EnemyType::LEFTHAND || type == EnemyType::RIGHTHAND) && !lefthandTextureLoaded) {
            if (lefthandTexture.loadFromFile("/Users/cpcap/GTA6/source/boss_3_texture/character/Lefthandpic.png")) {
                lefthandTextureLoaded = true;
            }
        }

        if ((type == EnemyType::LEFTHAND || type == EnemyType::RIGHTHAND) && lefthandTextureLoaded) {
            // 設置 Lefthand 精靈
            lefthandSprite.setTexture(lefthandTexture);
            lefthandSprite.setOrigin(lefthandTexture.getSize().x / 2.f, lefthandTexture.getSize().y / 2.f);
            lefthandSprite.setPosition(startX, startY);
            
            // 設置大小為 45x90
            float scaleX = 45.f / lefthandTexture.getSize().x;
            float scaleY = 90.f / lefthandTexture.getSize().y;
            lefthandSprite.setScale(scaleX, scaleY);

            // 如果是 RIGHTHAND 類型，水平翻轉圖片
            if (type == EnemyType::RIGHTHAND) {
                lefthandSprite.setScale(-scaleX, scaleY);  // 負的 scaleX 會水平翻轉圖片
            }
        }

        // 根據是否可被破壞和敵人類型來設置速度和傷害
        if (isIndestructible) {
            if (type == EnemyType::SKY) {
                speed = 0.0f;  // 從玩家處生成的靜止方形
                damage = 20.f; // SKY類型造成較高傷害
            } else {
                speed = 0.5f;  // boss生成的不可破壞敵人
                damage = 15.f; // HAND類型造成中等傷害
            }
        } else {
            if (type == EnemyType::UPHAND || type == EnemyType::LEFTHAND || type == EnemyType::RIGHTHAND) {
                speed = 0.1f;     // UPHAND 和 LEFTHAND 類型速度相同
                damage = 5.f;    // 造成較高傷害
            } else {
                speed = 0.2f;     // 可破壞的敵人速度較慢
                damage = 5.f;     // NORMAL類型造成礎傷害
            }
        }

        shape.setSize(size);
        shape.setOrigin(size.x / 2.f, size.y / 2.f);
        shape.setPosition(startX, startY);
        
        // 修改顏色設置邏輯
        if (type == EnemyType::SKY || 
            type == EnemyType::UPHAND || 
            type == EnemyType::LEFTHAND || 
            type == EnemyType::RIGHTHAND) {
            shape.setFillColor(sf::Color(255, 255, 255, 0));  // 完全透明
        } else {
            shape.setFillColor(color);
        }
    }

    void update() {
        if (type == EnemyType::SKY) {
            float elapsedTime = skyImageTimer.getElapsedTime().asSeconds();
            
            // 在 1 秒時切換圖片
            if (elapsedTime >= 1.0f && showSky1) {
                showSky1 = false;
                showSky2 = true;
            }

            // 更新兩個精靈的位置
            sf::Vector2f currentPos = shape.getPosition();
            skySprite1.setPosition(currentPos);
            skySprite2.setPosition(currentPos);
        }
        
        if (type == EnemyType::LEFTHAND) {
            shape.move(speed, 0);
        } else if (type == EnemyType::RIGHTHAND) {
            shape.move(-speed, 0);
        } else {
            shape.move(0, speed);
            // 如果是 HAND 類型，同步更新 Bighand 的位置
            if (type == EnemyType::HAND && bighandTextureLoaded) {
                bighandSprite.setPosition(shape.getPosition());
            }
        }
        
        // 在更新位置時同步更新 Uphand 精靈的位置
        if (type == EnemyType::UPHAND && uphandTextureLoaded) {
            uphandSprite.setPosition(shape.getPosition());
        }

        // 在更新位置時同步更新 Lefthand 精靈的位置
        if ((type == EnemyType::LEFTHAND || type == EnemyType::RIGHTHAND) && lefthandTextureLoaded) {
            lefthandSprite.setPosition(shape.getPosition());
        }
    }

    // 修改碰撞檢測函數以返回傷害值
    float checkCollision(const Sprite& player) const {
        // 如果是 SKY 類型，檢查時間而不是顏色
        if (type == EnemyType::SKY) {
            float elapsedTime = skyImageTimer.getElapsedTime().asSeconds();
            // 在第一秒內不造成傷害
            if (elapsedTime < 1.0f) {
                return 0.f;
            }
            // 在 1.0-1.5 秒之間才造成傷害
            if (elapsedTime >= 1.5f) {
                return 0.f;
            }
        }
        
        // 獲取精靈的邊界框
        FloatRect playerBounds = player.getGlobalBounds();
        FloatRect enemyBounds = shape.getGlobalBounds();
        
        // 如果發生碰撞，返回對應的傷害值
        if (enemyBounds.intersects(playerBounds)) {
            return damage;
        }
        return 0.f;  // 沒有碰撞返回0傷害
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(shape);
        if (type == EnemyType::SKY) {
            if (showSky1) {
                window.draw(skySprite1);
            } else if (showSky2) {
                window.draw(skySprite2);
            }
        }
        // 添加 Bighand 的繪製
        if (type == EnemyType::HAND && bighandTextureLoaded) {
            window.draw(bighandSprite);
        }
        
        // 添加 Uphand 的繪製
        if (type == EnemyType::UPHAND && uphandTextureLoaded) {
            window.draw(uphandSprite);
        }

        // 添加 Lefthand 的繪製
        if ((type == EnemyType::LEFTHAND || type == EnemyType::RIGHTHAND) && lefthandTextureLoaded) {
            window.draw(lefthandSprite);
        }
    }
};

// 在類外初始化靜態成員
sf::Texture Enemy::skyTexture1;
sf::Texture Enemy::skyTexture2;
bool Enemy::texturesLoaded = false;

// 在類外初始化新的靜態成員
sf::Texture Enemy::bighandTexture;
bool Enemy::bighandTextureLoaded = false;

// 在類外初始化新的靜態成員
sf::Texture Enemy::uphandTexture;
bool Enemy::uphandTextureLoaded = false;

// 在類外初始化新的靜態成員
sf::Texture Enemy::lefthandTexture;
bool Enemy::lefthandTextureLoaded = false;

class AnimatedBackground {
private:
    std::vector<sf::Texture> frames;
    sf::Sprite sprite;
    float frameTime;
    float currentTime;
    size_t currentFrame;
    sf::Vector2f scale;

public:
    AnimatedBackground(const std::vector<std::string>& framePaths, float frameDuration, const sf::Vector2f& windowSize) {
        frameTime = frameDuration;
        currentTime = 0.0f;
        currentFrame = 0;

        // 加載所有幀
        for (const auto& path : framePaths) {
            sf::Texture texture;
            if (!texture.loadFromFile(path)) {
                std::cout << "Error loading frame: " << path << std::endl;
                continue;
            }
            frames.push_back(texture);
        }

        if (!frames.empty()) {
            sprite.setTexture(frames[0]);
            
            // 計算縮放比例以適口
            float scaleX = windowSize.x / frames[0].getSize().x;
            float scaleY = windowSize.y / frames[0].getSize().y;
            scale = sf::Vector2f(scaleX, scaleY);
            sprite.setScale(scale);
        }
    }

    void update(float deltaTime) {
        if (frames.empty()) return;

        currentTime += deltaTime;
        if (currentTime >= frameTime) {
            currentTime = 0;
            currentFrame = (currentFrame + 1) % frames.size();
            sprite.setTexture(frames[currentFrame]);
            sprite.setScale(scale);  // 確保縮放保持不變
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }
};

class Game {
private:
    RenderWindow& window;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    int* killCountPtr;
    int* goldPtr;  // 添加金幣指針
    AnimatedBackground* background;  // 使用普通指針管理背景
    sf::Clock blueEnemyTimer;      // 藍色敵人的計時器
    sf::Clock blueEnemyLifeTimer;  // 藍色敵人存活時間的計時器
    bool isBlueEnemyActive;        // 追踪藍色敵人是否存在
    sf::Vector2f playerPosition;  // 添加存玩家位置的變量
    sf::Texture bossBackgroundTexture;  // 添加 Boss 背景材質
    sf::Sprite bossBackgroundSprite;    // 添加 Boss 背景精靈
    bool isBossBackground = false;      // 追踪當前是否為 Boss 背景

    std::vector<sf::Texture> middleSceneTextures;
    sf::Sprite middleSceneSprite;
    bool isMiddleScene = false;
    size_t currentMiddleSceneFrame = 0;
    const std::string middleSceneBasePath = "/Users/cpcap/GTA6/source/boss_3_texture/boss_3_middle";

public:
    Game(RenderWindow& win, int* killCount, int* gold) 
        : window(win), killCountPtr(killCount), goldPtr(gold), isBlueEnemyActive(false) {
        std::vector<std::string> framePaths;
        for (int i = 1; i <= 24; i++) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "/Users/cpcap/GTA6/source/boss_3_texture/background/frames/frame_%03d.png", i);
            std::string path = buffer;
            std::cout << "Trying to load: " << path << std::endl;
            framePaths.push_back(path);
        }

        background = new AnimatedBackground(
            framePaths, 
            0.1f, 
            sf::Vector2f(window.getSize().x, window.getSize().y)
        );

        // 載入過場動畫圖片
        for (int i = 1; i <= 2; ++i) {
            sf::Texture texture;
            std::string path = middleSceneBasePath + std::to_string(i) + ".png";
            if (texture.loadFromFile(path)) {
                middleSceneTextures.push_back(texture);
            } else {
                std::cout << "Error loading middle scene texture: " << path << std::endl;
            }
        }
    }

    // 添加設置玩家位置方法
    void setPlayerPosition(const sf::Vector2f& pos) {
        playerPosition = pos;
    }

    void update(float deltaTime) {
        if (background) {
            background->update(deltaTime);
        }

        // 修改空中敵人的生成位置
        if (!isBlueEnemyActive) {
            if (blueEnemyTimer.getElapsedTime().asSeconds() >= 5.0f) {  // 每5秒檢查一次
                // 使用玩家的位置來生成敵人
                addEnemy(playerPosition.x, playerPosition.y, 
                        sf::Vector2f(ENEMY_WIDTH * 7.f, ENEMY_WIDTH * 7.f),
                        sf::Color::White,  // 初始顏色為白色
                        true,
                        EnemyType::SKY);
                
                isBlueEnemyActive = true;
                blueEnemyLifeTimer.restart();
            }
        } else {
            float elapsedTime = blueEnemyLifeTimer.getElapsedTime().asSeconds();
            
            // 在1秒時改變顏色（保持透明）
            if (elapsedTime >= 1.0f && elapsedTime < 1.5f) {
                // 到並改變敵人顏色（保持透明）
                auto& enemies = getEnemies();
                for (auto& enemy : enemies) {
                    if (enemy.type == EnemyType::SKY) {
                        enemy.shape.setFillColor(sf::Color(0, 0, 0, 0));  // 完全透明的黑色
                    }
                }
            }
            
            // 2秒後移除敵人
            if (elapsedTime >= 1.5f) {
                // 找到並移除敵人
                auto& enemies = getEnemies();
                for (size_t i = 0; i < enemies.size(); ++i) {
                    if (enemies[i].type == EnemyType::SKY) {
                        enemies.erase(enemies.begin() + i);
                        break;
                    }
                }
                
                isBlueEnemyActive = false;
                blueEnemyTimer.restart();  // 重置生成時器
            }
        }
    }

    void draw() {
        drawBackground();
        
        // 繪製敵人
        for (const auto& enemy : enemies) {
            enemy.draw(window);  // 使用Enemy類的draw方法
        }
        
        for (const auto& bullet : bullets) {
            window.draw(bullet.shape);
        }
    }

    void reset() {
        bullets.clear();
        enemies.clear();
    }

    void drawBackground() {
        if (isBossBackground) {
            window.draw(bossBackgroundSprite);
        } else if (background) {
            background->draw(window);
        }
    }

    // 添加獲取敵人和子彈的方法
    const std::vector<Enemy>& getEnemies() const {
        return enemies;
    }

    std::vector<Bullet>& getBullets() {
        return bullets;
    }

    // 添加更新方法
    void updateBullets() {
        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(),
                [this](Bullet& bullet) {
                    bullet.update();
                    // 查子彈是否超出畫面
                    if (bullet.shape.getPosition().y < 0) return true;
                    
                    // 使用迭代器而不是索引來移除敵人
                    auto enemyIt = std::find_if(enemies.begin(), enemies.end(),
                        [&bullet](const Enemy& enemy) {
                            return !enemy.isIndestructible && 
                                   bullet.shape.getGlobalBounds().intersects(enemy.shape.getGlobalBounds());
                        });
                    
                    if (enemyIt != enemies.end()) {
                        enemies.erase(enemyIt);
                        (*killCountPtr)++;
                        (*goldPtr) += 1000;
                        return true;
                    }
                    return false;
                }
            ),
            bullets.end()
        );
    }

    void updateEnemies() {
        auto it = enemies.begin();
        while (it != enemies.end()) {
            it->update();
            
            // 檢查是否超出邊界
            if (it->type == EnemyType::LEFTHAND) {
                // 如果 LEFTHAND 敵人超出右邊界，則移除
                if (it->shape.getPosition().x > BOUNDARY_RIGHT) {
                    it = enemies.erase(it);
                    continue;
                }
            } else if (it->type == EnemyType::RIGHTHAND) {
                // 如果 RIGHTHAND 敵人超出左邊界，則移除
                if (it->shape.getPosition().x < BOUNDARY_LEFT) {
                    it = enemies.erase(it);
                    continue;
                }
            } else {
                // 其他敵人的邊界檢查（超出下邊界）
                if (it->shape.getPosition().y > window.getSize().y) {
                    it = enemies.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }

    // 添加子彈和敵人
    void addBullet(float x, float y) {
        Bullet bullet(x, y);
        bullets.push_back(bullet);
    }

    void addEnemy(float x, float y, 
                 const sf::Vector2f& size = sf::Vector2f(30.f, 30.f),
                 const sf::Color& color = sf::Color::Red,
                 bool indestructible = false,
                 EnemyType enemyType = EnemyType::NORMAL) {
        Enemy enemy(x, y, size, color, indestructible, enemyType);
        enemies.push_back(enemy);
    }

    void removeEnemy(size_t index) {
        if (index < enemies.size()) {
            enemies.erase(enemies.begin() + index);
        }
    }

    // 修改 getEnemies 方法返回引用，這樣可以直接修改敵人容器
    std::vector<Enemy>& getEnemies() {
        return enemies;
    }

    // 修改檢測玩家碰撞的方法
    bool checkPlayerCollision(const Sprite& playerSprite, float& damageDealt) {
        for (const auto& enemy : enemies) {
            float damage = enemy.checkCollision(playerSprite);
            if (damage > 0) {
                damageDealt = damage;  // 設置實際造成的傷害
                return true;
            }
        }
        return false;
    }

    void switchToBossBackground() {
        if (!isBossBackground) {
            if (bossBackgroundTexture.loadFromFile("/Users/cpcap/GTA6/source/boss_3_texture/background/boss_3background.png")) {
                bossBackgroundSprite.setTexture(bossBackgroundTexture);
                
                // 調整背景大小以適應窗口
                float scaleX = window.getSize().x / (float)bossBackgroundTexture.getSize().x;
                float scaleY = window.getSize().y / (float)bossBackgroundTexture.getSize().y;
                bossBackgroundSprite.setScale(scaleX, scaleY);
                
                isBossBackground = true;
                if (background) {
                    delete background;
                    background = nullptr;
                }
            }
        }
    }

    void startMiddleScene() {
        if (!middleSceneTextures.empty()) {
            currentMiddleSceneFrame = 0;
            middleSceneSprite.setTexture(middleSceneTextures[0]);
            
            // 調整大小以適應窗口
            float scaleX = window.getSize().x / (float)middleSceneTextures[0].getSize().x;
            float scaleY = window.getSize().y / (float)middleSceneTextures[0].getSize().y;
            middleSceneSprite.setScale(scaleX, scaleY);
            
            isMiddleScene = true;
        }
    }

    bool nextMiddleSceneFrame() {
        if (isMiddleScene && currentMiddleSceneFrame < middleSceneTextures.size() - 1) {
            currentMiddleSceneFrame++;
            middleSceneSprite.setTexture(middleSceneTextures[currentMiddleSceneFrame]);
            
            // 調整大小以適應窗口
            float scaleX = window.getSize().x / (float)middleSceneTextures[currentMiddleSceneFrame].getSize().x;
            float scaleY = window.getSize().y / (float)middleSceneTextures[currentMiddleSceneFrame].getSize().y;
            middleSceneSprite.setScale(scaleX, scaleY);
            
            return true;
        }
        return false;
    }

    void endMiddleScene() {
        isMiddleScene = false;
        switchToBossBackground();  // 在過場動畫結束時切換背景
    }

    bool isInMiddleScene() const { return isMiddleScene; }

    void drawMiddleScene() {
        if (isMiddleScene) {
            window.draw(middleSceneSprite);
        }
    }
};

class Boss_3 {
public:
    Boss_3(const std::string& texturePath, float startX = 600.f, float startY = 100.f);
    void update();
    void draw(sf::RenderWindow& window);
    void takeDamage(int damage); // 如果需要處理Boss受到
    bool isDead() const;         // 檢查Boss是否死亡

    // 獲取Boss的位置和尺寸
    const sf::Sprite& getSprite() const;
    float getX() const;
    float getY() const;
    float getWidth() const;
    float getHeight() const;

    void setPosition(float newX, float newY); // 設置Boss的位置

    bool isInvincibleState() const { return isInvincible; }  // 從 private 移到 public

    // 添加獲取當前血量的方法
    float getCurrentHealth() const {
        return currentHealth;
    }

    // 添加獲取最大血量的方法
    float getMaxHealth() const {
        return maxHealth;
    }

    bool hasRevivedOnce() const { return revivedCount > 0; }

    // 添加碰撞檢測方法
    bool checkCollision(const sf::Sprite& playerSprite) const {
        if (revivedCount > 0) {
            // 獲取子彈的位置
            sf::FloatRect bulletBounds = playerSprite.getGlobalBounds();
            
            // 檢查子彈是否在指定域 (x: 200-1000, y: 0-150)
            if (bulletBounds.left >= 200.f && 
                bulletBounds.left + bulletBounds.width <= 1000.f &&
                bulletBounds.top >= 0.f && 
                bulletBounds.top + bulletBounds.height <= 150.f) {
                return true;
            }
            return false;
        } else {
            // 原始碰撞檢測邏輯（第一階段）
            return sprite.getGlobalBounds().intersects(playerSprite.getGlobalBounds());
        }
    }

    // 添加發射子彈的方法
    bool shouldShoot() {
        // 果已經復活，使用不同的擊邏輯
        if (revivedCount > 0) {
            return false;  // 第二階段不再發射彈
        }

        // 第一階段原始射擊邏輯
        if (attackTimer.getElapsedTime().asSeconds() >= attackInterval) {
            if (!isPreparingShooting) {
                isPreparingShooting = true;
                shootDelayTimer.restart();
                return false;
            } else if (shootDelayTimer.getElapsedTime().asSeconds() >= 0.5f) {
                attackTimer.restart();
                attackInterval = 2.0f + (static_cast<float>(rand()) / RAND_MAX) * 4.0f;
                isPreparingShooting = false;
                return true;
            }
        }
        return false;
    }
    
    // 獲取 Boss 當位置的方法
    sf::Vector2f getPosition() const {
        return sprite.getPosition();
    }

    // 添加一個標記來示是否剛剛復活
    bool justRevived = false;

private:
    sf::Texture texture;
    sf::Sprite sprite;
    float x, y;
    float moveSpeed;
    int health;
    bool movingRight;
    float bossWidth;
    float bossHeight;

    sf::RectangleShape healthBarBackground;
    sf::RectangleShape healthBar;
    float maxHealth;
    float currentHealth;

    bool isInvincible;
    sf::Clock invincibilityTimer;
    const float invincibilityDuration = 0.5f;  // 無敵時間持續0.5秒

    int revivedCount = 0;  // 蹤復活次數

    sf::Clock moveTimer;           // 計時器用於隨機移動
    float moveInterval = 0.8f;     // 將基礎時間間隔從 2.0f 改為 0.8f
    float currentMoveDirection = 1.f;  // 當前移動方向 (-1 左, 1 右)

    sf::Clock attackTimer;
    float attackInterval;  // 移除固定值，改為動態設置
    
    sf::Clock shootDelayTimer;    // 添加延計時器
    bool isPreparingShooting;     // 添加準備射擊標記
    
    // 移動邏
    void move();
};

Boss_3::Boss_3(const std::string& texturePath, float startX, float startY)
    : x(startX), y(startY), moveSpeed(0.1f), health(100), movingRight(true), isInvincible(false),
      currentMoveDirection(1.f), moveInterval(0.8f), isPreparingShooting(false) {  // 初始間隔也改為 0.8f

    if (!texture.loadFromFile(texturePath)) {
        std::cout << "Error loading boss texture!" << std::endl;
        return;
    }

    sprite.setTexture(texture);
    sprite.setOrigin(texture.getSize().x / 2.f, texture.getSize().y / 2.f);
    sprite.setPosition(x, y);

    // 根據是否復活來設置不同的尺寸
    float desiredWidth = revivedCount > 0 ? 800.f : 300.f;   // 復活後寬度改為 800.f
    float desiredHeight = revivedCount > 0 ? 800.f : 300.f;  // 高度也相應調整
    
    sprite.setScale(
        desiredWidth / texture.getSize().x,
        desiredHeight / texture.getSize().y
    );

    bossWidth = sprite.getGlobalBounds().width;
    bossHeight = sprite.getGlobalBounds().height;

        // 初始化血條
    healthBarBackground.setSize(Vector2f(40.f, 600.f));
    healthBar.setSize(Vector2f(40.f, 600.f));
    
    // 設置血條位置
    healthBarBackground.setPosition(50.f, 100.f);
    healthBar.setPosition(50.f, 100.f);
    
    // 修改血條顏色
    healthBarBackground.setFillColor(Color(100, 100, 100));  // 深灰色背景
    healthBar.setFillColor(Color::Red);                      // 紅色血條
    
    // 添加血條邊框
    healthBarBackground.setOutlineThickness(2.f);
    healthBarBackground.setOutlineColor(Color::White);       // 白色邊框
    
    // 設置血量
    maxHealth = 100.f;
    currentHealth = 100.f;

    // 初始化隨機數生成器（如果還沒有初始化）
    static bool seeded = false;
    if (!seeded) {
        srand(time(nullptr));
        seeded = true;
    }

    // 初始化第一次攻擊間隔
    attackInterval = 2.0f + (static_cast<float>(rand()) / RAND_MAX) * 4.0f;  // 2-6秒
}

void Boss_3::move() {
    // 如果正在準備射擊，不進行移動
    if (isPreparingShooting) {
        return;
    }

    // 每隔一段時間隨機改變方向
    if (moveTimer.getElapsedTime().asSeconds() >= moveInterval) {
        // 隨機決定新的移動方向，只有右種選擇
        currentMoveDirection = (rand() % 2) * 2 - 1;  // 會得到 -1 或 1
        
        // 重置計時器並隨機設置一次改變方向的時間間隔（0.5-1.5秒）
        moveTimer.restart();
        moveInterval = 0.5f + (rand() % 10) / 10.0f;
    }

    // 移動
    x += moveSpeed * currentMoveDirection;
    
    // 確保不超出邊界
    if (x <= 200.f) {  // 左邊界
        x = 200.f;
        currentMoveDirection = 1.f;  // 強制向右移動
    }
    else if (x >= 1000.f) {  // 右邊界
        x = 1000.f;
        currentMoveDirection = -1.f;  // 強制向左移
    }
    
    sprite.setPosition(x, y);  // 更新精靈位置
}

void Boss_3::update() {
    if (!isPreparingShooting) {  // 只在不準備射擊時移動
        move();
    }
    
    // 使用三元運算符簡化無敵狀態檢查
    isInvincible = isInvincible && invincibilityTimer.getElapsedTime().asSeconds() < invincibilityDuration;
}

void Boss_3::draw(sf::RenderWindow& window) {
    // 只繪製未復活繪製 Boss 精靈
    if (revivedCount == 0) {
        window.draw(sprite);
    }
    // 終繪製血條
    window.draw(healthBarBackground);
    window.draw(healthBar);
}

void Boss_3::takeDamage(int damage) {
    if (!isInvincible) {
        // 根據活態決定傷害值
        int actualDamage = revivedCount > 0 ? 2 : 4;  // 復活後傷害降為1，第一階段保持3
        health -= actualDamage;
        
        if (health <= 0) {
            if (revivedCount == 0) {
                // 第一次血量歸零時重新回滿
                health = 100;
                currentHealth = 100.f;
                revivedCount++;
                justRevived = true;
                
                // 更新 Boss 尺寸
                float desiredWidth = 800.f;   // 復活後的新尺寸
                float desiredHeight = 800.f;
                sprite.setScale(
                    desiredWidth / texture.getSize().x,
                    desiredHeight / texture.getSize().y
                );
                
                bossWidth = sprite.getGlobalBounds().width;
                bossHeight = sprite.getGlobalBounds().height;
                
                // 更新血條顯示
                float healthBarHeight = 600.f;
                healthBar.setSize(Vector2f(40.f, healthBarHeight));
                healthBar.setPosition(50.f, 100.f);
                
                std::cout << "Boss revived with full health!" << std::endl;
            } else {
                // 第二次血量歸零時真正死亡
                health = 0;
                currentHealth = 0.f;
                healthBar.setSize(Vector2f(40.f, 0.f));
            }
        }
        
        // 更新血條顯示
        currentHealth = health;
        float healthPercentage = currentHealth / maxHealth;
        float healthBarHeight = healthPercentage * 600.f;
        
        // 確保血條不會超出邊框
        healthBarHeight = std::max(0.f, std::min(healthBarHeight, 600.f));
        
        healthBar.setSize(Vector2f(40.f, healthBarHeight));
        healthBar.setPosition(50.f, 100.f + (600.f - healthBarHeight));
        
        std::cout << "Boss took " << actualDamage << " damage. Remaining Health: " << health << std::endl;

        isInvincible = true;
        invincibilityTimer.restart();
    }
}

bool Boss_3::isDead() const {
    return health <= 0;
}

const sf::Sprite& Boss_3::getSprite() const {
    return sprite;
}

float Boss_3::getX() const {
    return x;
}

float Boss_3::getY() const {
    return y;
}

float Boss_3::getWidth() const {
    return bossWidth;
}

float Boss_3::getHeight() const {
    return bossHeight;
}

void Boss_3::setPosition(float newX, float newY) {
    x = newX;
    y = newY;
    sprite.setPosition(x, y);
}

int main() {
    RenderWindow window(VideoMode(1200, 800), "SFML works!");
    srand(time(0));  // 始化隨機數生成器
    
    // 加載玩家材質
    Texture playerTexture;
    if (!playerTexture.loadFromFile("/Users/cpcap/GTA6/source/boss_3_texture/character/player.png")) {
        cout << "Error loading player texture!" << endl;
        return -1;
    }

    //新
    // 加載BOSS_3材質
    Boss_3("/Users/cpcap/GTA6/source/boss_3_texture/character/boss_3.png");
    
    // 創建玩家精靈替代原來的 CircleShape
    Sprite playerSprite(playerTexture);
    // 設置精靈原點為中心
    playerSprite.setOrigin(playerTexture.getSize().x / 2.f, playerTexture.getSize().y / 2.f);
    
    float x = BOUNDARY_LEFT + PLAY_AREA_WIDTH/2;  // 在戲區中心
    float y = 730.f;  // 原始值是 740.f，我們可以減小這個值來使角色往上移

    playerSprite.setPosition(x, y);
    
    // 在 main 函數中，修改玩家精靈的縮放比例
    float desiredWidth = 117.f;   // 期望的寬度
    float desiredHeight = 182.f;  // 期望的高度（可以調整這個值來改變高度）

    playerSprite.setScale(
        desiredWidth / playerTexture.getSize().x,
        desiredHeight / playerTexture.getSize().y
    );
    
    float moveSpeed = 0.2f;

    // 獲取玩家精靈的實際寬度（考慮縮放後的大小）
    float playerWidth = playerSprite.getGlobalBounds().width;

    // 設置移動界，考慮玩家寬度
    float leftBound = BOUNDARY_LEFT;                         // 左邊界
    float rightBound = BOUNDARY_LEFT + PLAY_AREA_WIDTH - playerWidth;  // 右邊界減去玩寬度

    // 子彈容器
    std::vector<Bullet> bullets;
    bool spacePressed = false;

    // 修改血條設置
    RectangleShape healthBarBackground(Vector2f(20.f, 200.f));
    RectangleShape healthBar(Vector2f(20.f, 200.f));

    // 設置血條位置（螢幕中間偏右）
    healthBarBackground.setPosition(1150.f, 300.f);  // x=1150 會在幕右側，y=300 在螢幕中間偏上
    healthBar.setPosition(1150.f, 300.f);           // 保持背景相位

    // 設置血顏色和邊框
    healthBarBackground.setFillColor(Color(100, 100, 100));
    healthBar.setFillColor(Color::Green);

    // 添加血條邊框
    healthBarBackground.setOutlineThickness(2.f);
    healthBarBackground.setOutlineColor(Color::White);
    
    // 設置血量
    float maxHealth = 300.f;
    float currentHealth = 300.f;

    // 敵人關變量
    std::vector<Enemy> enemies;
    Clock enemySpawnTimer;  // 於計時生成敵人
    
    // 添加無敵時間計時器
    Clock invincibilityTimer;
    bool isInvincible = false;
    float invincibilityDuration = 1.0f;  // 1無敵間

    // 創建 Game 實例之前定義 killCount
    int killCount = 0;
    int gold = 0;  // 初始金幣

    // 修改載入字體的部分
    sf::Font font;
    if (!font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {  // macOS 系統字體路徑
        // 如果 Arial 載入敗，嘗試載入系統默認字體
        if (!font.loadFromFile("/System/Library/Fonts/STHeiti Light.ttc")) {
            std::cout << "Error loading font!" << std::endl;
        }
    }

    // 修改計數器文字設置
    sf::Text killCountText;
    killCountText.setFont(font);
    killCountText.setCharacterSize(30);
    killCountText.setFillColor(sf::Color::White);
    killCountText.setPosition(10.f, 10.f);
    killCountText.setStyle(sf::Text::Bold);  // 添加粗體樣式

    // 新增金幣文字
    sf::Text goldText;
    goldText.setFont(font);
    goldText.setCharacterSize(30);
    goldText.setFillColor(sf::Color::White);
    goldText.setPosition(10.f, 40.f);  // 位置在擊殺數下方
    goldText.setStyle(sf::Text::Bold);  // 添加粗體樣式

    // 建遊戲實例
    Game game(window, &killCount, &gold);

    // 創建遊戲結束文字
    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setString("Game Over!");
    gameOverText.setCharacterSize(50);
    gameOverText.setFillColor(sf::Color::Red);
    
    // 創示文字
    sf::Text promptText;
    promptText.setFont(font);
    promptText.setString("Press R to Restart or ESC to Quit");
    promptText.setCharacterSize(30);
    promptText.setFillColor(sf::Color::White);
    
    // 設置文字位置
    gameOverText.setPosition(
        window.getSize().x/2 - gameOverText.getGlobalBounds().width/2,
        window.getSize().y/2 - gameOverText.getGlobalBounds().height/2 - 50
    );
    promptText.setPosition(
        window.getSize().x/2 - promptText.getGlobalBounds().width/2,
        window.getSize().y/2 + 50
    );

    // 添遊戲狀態
    bool isGameOver = false;
    bool gameWon = false;

    // 添加子彈發射計時器
    Clock shootTimer;
    const float shootCooldown = 0.5f;  // 射擊冷卻時間（秒）

    // 添加敵人生成計時器
    const float enemySpawnInterval = 0.3f;  // 更短的生成間隔
    const float ENEMY_SPAWN_CHANCE = 0.4f;  // 每次檢查時生成敵人的機率

    // 創建勝利文字
    sf::Text gameWonText;
    gameWonText.setFont(font);
    gameWonText.setString("Victory!");
    gameWonText.setCharacterSize(50);
    gameWonText.setFillColor(sf::Color::Green);

    // 創建勝利提示文字
    sf::Text victoryPromptText;
    victoryPromptText.setFont(font);
    victoryPromptText.setString("Press R to Play Again or ESC to Quit");
    victoryPromptText.setCharacterSize(30);
    victoryPromptText.setFillColor(sf::Color::White);

    // 設置勝利字位置
    gameWonText.setPosition(
        window.getSize().x/2 - gameWonText.getGlobalBounds().width/2,
        window.getSize().y/2 - gameWonText.getGlobalBounds().height/2 - 50
    );
    victoryPromptText.setPosition(
        window.getSize().x/2 - victoryPromptText.getGlobalBounds().width/2,
        window.getSize().y/2 + 50
    );

    // 在 main 函數開始處添加自動發射的計時器和間隔設置
    Clock autoShootTimer;  // 自動發射時器
    const float autoShootInterval = 0.5f;  // 0.5秒發射一次，你可以調整這個值

    sf::Clock clock;  // 添加時間來計算幀時間
    
    // 直接創建 Boss_3 實例
    std::unique_ptr<Boss_3> boss3 = std::make_unique<Boss_3>(
        "/Users/cpcap/GTA6/source/boss_3_texture/character/boss_3.png",
        600.f,  // x 位置
        130.f   // y 位置改 150.f
    );

    // 在 main 函數開始處
    const float SHOOT_COOLDOWN = 0.4f;  // 射擊冷卻時
    sf::Clock shootCooldownTimer;       // 射擊冷卻計時器

    bool bKeyPressed = false;  // 追踪 B 鍵狀態
    bool spaceKeyPressed = false;  // 改為追踪空白鍵狀態

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
            
            // 添加調試模式的擊殺數增加
            if (!gameWon && !isGameOver && event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::J) {
                    killCount++;  // 每按一次J增加一個擊殺數
                    gold += 1000; // 每次擊殺增加1000金幣
                    // 更新擊殺數顯示
                    killCountText.setString("Kills: " + std::to_string(killCount));
                    goldText.setString("Gold: " + std::to_string(gold));
                }
                // 添加H鍵扣血的測
                else if (event.key.code == Keyboard::H) {
                    currentHealth = std::max(0.f, currentHealth - 10.f);
                    float healthBarHeight = (currentHealth/maxHealth) * 200.f;
                    healthBar.setSize(Vector2f(20.f, healthBarHeight));
                    healthBar.setPosition(1150.f, 300.f + (200.f - healthBarHeight));
                    
                    // 如果血量歸零，觸發遊戲結束
                    if (currentHealth <= 0) {
                        isGameOver = true;
                    }
                }
            }
            
            // 遊戲結束按鍵處理
            if (isGameOver && event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::R)
                {
                    // 重置遊戲狀態
                    currentHealth = maxHealth;
                    healthBar.setSize(Vector2f(20.f, 200.f));
                    x = BOUNDARY_LEFT + PLAY_AREA_WIDTH/2;
                    y = 730.f;
                    playerSprite.setPosition(x, y);
                    game.reset();
                    killCount = 0;
                    gold = 0;
                    isGameOver = false;
                    
                    // 更新顯示文字
                    killCountText.setString("Kills: 0");
                    goldText.setString("Gold: 0");
                }
                else if (event.key.code == Keyboard::Escape)
                {
                    window.close();
                }
            }

            // 添加調試模式的按鍵檢測
            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::H && !isGameOver) {
                    // 按H鍵扣血
                    currentHealth = std::max(0.f, currentHealth - 10.f);
                    float healthBarHeight = (currentHealth/maxHealth) * 200.f;
                    healthBar.setSize(Vector2f(20.f, healthBarHeight));
                    healthBar.setPosition(1150.f, 300.f + (200.f - healthBarHeight));
                    
                    // 如果血歸，觸發遊戲結束
                    if (currentHealth <= 0) {
                        isGameOver = true;
                    }
                }
            }

            // 添加勝時的按鍵理
            if (gameWon && event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::R) {
                    // 重置遊戲狀態
                    currentHealth = maxHealth;
                    healthBar.setSize(Vector2f(20.f, 200.f));
                    x = BOUNDARY_LEFT + PLAY_AREA_WIDTH/2;
                    y = 730.f;
                    playerSprite.setPosition(x, y);
                    game.reset();
                    killCount = 0;
                    gold = 0;
                    gameWon = false;
                    
                    // 更新顯示文字
                    killCountText.setString("Kills: 0");
                    goldText.setString("Gold: 0");
                }
                else if (event.key.code == Keyboard::Escape) {
                    window.close();
                }
            }

            // 將 B 鍵檢測改為空白鍵檢測
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Space) {
                if (!spaceKeyPressed && game.isInMiddleScene()) {
                    spaceKeyPressed = true;
                    if (!game.nextMiddleSceneFrame()) {
                        // 如果沒有下一幀，結束過場動畫並切換背景
                        game.endMiddleScene();
                    }
                }
            }
            if (event.type == Event::KeyReleased && event.key.code == Keyboard::Space) {
                spaceKeyPressed = false;
            }
        }

        // 遊戲循環中，修改碰撞檢測的部分
        if (!isInvincible) {
            // 檢查與敵人的碰撞
            auto enemyIt = game.getEnemies().begin();
            while (enemyIt != game.getEnemies().end()) {
                float damage = enemyIt->checkCollision(playerSprite);
                if (damage > 0) {
                    // 扣血和增加計數只在非無敵狀態下進行
                    currentHealth = std::max(0.f, currentHealth - damage);
                    float healthBarHeight = (currentHealth/maxHealth) * 200.f;
                    healthBar.setSize(Vector2f(20.f, healthBarHeight));
                    healthBar.setPosition(1150.f, 300.f + (200.f - healthBarHeight));
                    
                    // 設置無敵狀態
                    isInvincible = true;
                    invincibilityTimer.restart();
                    
                    // 增加擊殺數和金幣
                    killCount++;
                    gold += 1000;
                    
                    // 更新UI文字
                    killCountText.setString("Kills: " + std::to_string(killCount));
                    goldText.setString("Gold: " + std::to_string(gold));
                    
                    // 檢查是否死亡
                    if (currentHealth <= 0) {
                        isGameOver = true;
                    }
                    
                    // 移除敵人
                    enemyIt = game.getEnemies().erase(enemyIt);
                    continue;
                }
                ++enemyIt;
            }
        } else {
            // 在無敵狀態下，只移除碰撞的敵人，不進行其他操作
            auto enemyIt = game.getEnemies().begin();
            while (enemyIt != game.getEnemies().end()) {
                float damage = enemyIt->checkCollision(playerSprite);
                if (damage > 0) {
                    // 只移除敵人，不增加計數或扣血
                    enemyIt = game.getEnemies().erase(enemyIt);
                    continue;
                }
                ++enemyIt;
            }
        }

        // 修改血量檢查邏輯
        if (currentHealth <= 0) {
            isGameOver = true;
            gameWon = false;
        }

        window.clear();

        if (game.isInMiddleScene()) {
            // 只繪製過場動畫
            game.drawMiddleScene();
        } else if (!isGameOver && !gameWon) {  // 確保兩個狀態斥
            game.update(deltaTime);  // 更新遊戲狀態，包括背景動畫
            game.drawBackground();   // 繪製背景
            
            // 繪製敵人
            for (const auto& enemy : game.getEnemies()) {
                enemy.draw(window);  // 使用Enemy類的draw方法
            }
            
            // 繪製玩家和子彈
            window.draw(playerSprite);
            for (const auto& bullet : game.getBullets()) {
                window.draw(bullet.shape);
            }
            
            // 繪繪製條
            window.draw(healthBarBackground);
            window.draw(healthBar);

            // 遊戲邏輯更新
            if (Keyboard::isKeyPressed(Keyboard::Left)) {
                x = std::max(leftBound + playerWidth/2.f, x - moveSpeed);
            }
            if (Keyboard::isKeyPressed(Keyboard::Right)) {
                x = std::min(rightBound + playerWidth/2.f, x + moveSpeed);
            }
            // 添加上下移動
            if (Keyboard::isKeyPressed(Keyboard::Up)) {
                // 限制上邊界，確保玩家不會移出畫面
                y = std::max(playerSprite.getGlobalBounds().height/2.f, y - moveSpeed);
            }
            if (Keyboard::isKeyPressed(Keyboard::Down)) {
                // 限制下邊界，確保玩家不會移出畫面
                y = std::min(window.getSize().y - playerSprite.getGlobalBounds().height/2.f, y + moveSpeed);
            }

            playerSprite.setPosition(x, y);
            // 更新 Game 類中的玩家位置
            game.setPlayerPosition(playerSprite.getPosition());
            
            // 射擊控制
            if (Keyboard::isKeyPressed(Keyboard::Space) && 
                shootCooldownTimer.getElapsedTime().asSeconds() >= SHOOT_COOLDOWN) {
                // 從玩家中心位置發射子彈
                float bulletX = playerSprite.getPosition().x;
                float bulletY = playerSprite.getPosition().y - playerSprite.getGlobalBounds().height/2.f;
                
                game.addBullet(bulletX, bulletY);
                shootCooldownTimer.restart();  // 重置冷卻計時器
            }

            // 修改敵人生成邏輯
            if (enemySpawnTimer.getElapsedTime().asSeconds() >= enemySpawnInterval) {
                // Boss 復活後敵人生成邏輯
                if (boss3 && boss3->hasRevivedOnce()) {
                    float spawnRoll = static_cast<float>(rand()) / RAND_MAX;
                    
                    if (spawnRoll < 0.7f) {  // 總生成機率
                        // 決定成哪種類型的敵人
                        if (spawnRoll < 0.3f) {  // 0.4的機率生成 UPHAND
                            int enemyCount = 1 + (rand() % 2);
                            
                            const float ENEMY_BOUNDARY_LEFT = 250.f;
                            const float ENEMY_BOUNDARY_RIGHT = 950.f;
                            const float NORMAL_ENEMY_WIDTH = 30.f;
                            const float UPHAND_ENEMY_WIDTH = NORMAL_ENEMY_WIDTH * 2.f;  // 兩倍寬度
                            const float ENEMY_SPACING = 50.f;
                            
                            for (int i = 0; i < enemyCount; ++i) {
                                float randomX;
                                bool validPosition;
                                int maxAttempts = 10;
                                
                                do {
                                    validPosition = true;
                                    randomX = ENEMY_BOUNDARY_LEFT + 
                                        (static_cast<float>(rand()) / RAND_MAX) * 
                                        (ENEMY_BOUNDARY_RIGHT - ENEMY_BOUNDARY_LEFT - UPHAND_ENEMY_WIDTH);
                                    
                                    for (const auto& existingEnemy : game.getEnemies()) {
                                        float existingX = existingEnemy.shape.getPosition().x;
                                        if (abs(randomX - existingX) < ENEMY_SPACING) {
                                            validPosition = false;
                                            break;
                                        }
                                    }
                                    
                                    maxAttempts--;
                                } while (!validPosition && maxAttempts > 0);
                                
                                if (randomX > (ENEMY_BOUNDARY_RIGHT - UPHAND_ENEMY_WIDTH)) {
                                    randomX = ENEMY_BOUNDARY_RIGHT - UPHAND_ENEMY_WIDTH;
                                }            
                                if (validPosition || maxAttempts <= 0) {
                                    // 生成黃色 UPHAND 敵人
                                    game.addEnemy(
                                        randomX, 
                                        0.f,
                                        sf::Vector2f(NORMAL_ENEMY_WIDTH * 2.f, NORMAL_ENEMY_WIDTH),  // 兩倍寬度
                                        sf::Color::Yellow,  // 黃色
                                        false,              // 可被摧毀
                                        EnemyType::UPHAND   // UPHAND 類型
                                    );
                                }
                            }
                        } else {  // 0.2的機率生成 LEFTHAND 或 RIGHTHAND
                            const float NORMAL_ENEMY_WIDTH = 30.f;
                            const float HAND_HEIGHT = NORMAL_ENEMY_WIDTH * 2.f;  // 兩倍高度
                            
                            // 隨機生成 Y 座標
                            float randomY = NORMAL_ENEMY_WIDTH + 
                                (static_cast<float>(rand()) / RAND_MAX) * 
                                (window.getSize().y - HAND_HEIGHT - NORMAL_ENEMY_WIDTH);
                            
                            // 隨機決定是生成 LEFTHAND 還是 RIGHTHAND
                            if (rand() % 2 == 0) {
                                // 從左側生成 LEFTHAND
                                game.addEnemy(
                                    0.f,  // 最左側
                                    randomY,
                                    sf::Vector2f(NORMAL_ENEMY_WIDTH, HAND_HEIGHT),
                                    sf::Color::Yellow,
                                    false,
                                    EnemyType::LEFTHAND
                                );
                            } else {
                                // 從右側生成 RIGHTHAND
                                game.addEnemy(
                                    1200.f,  // 最右側
                                    randomY,
                                    sf::Vector2f(NORMAL_ENEMY_WIDTH, HAND_HEIGHT),
                                    sf::Color::Yellow,
                                    false,
                                    EnemyType::RIGHTHAND
                                );
                            }
                        }
                    }
                } else {
                    // Boss 復活之前的敵人生成邏輯
                    float spawnRoll = static_cast<float>(rand()) / RAND_MAX;
                    
                    if (spawnRoll < ENEMY_SPAWN_CHANCE) {
                        int enemyCount = 1 + (rand() % 2);  // 生成1-2個敵人
                        
                        const float ENEMY_BOUNDARY_LEFT = 250.f;
                        const float ENEMY_BOUNDARY_RIGHT = 950.f;
                        const float NORMAL_ENEMY_WIDTH = 30.f;
                        const float ENEMY_SPACING = 50.f;
                        
                        for (int i = 0; i < enemyCount; ++i) {
                            float randomX;
                            bool validPosition;
                            int maxAttempts = 10;
                            
                            do {
                                validPosition = true;
                                randomX = ENEMY_BOUNDARY_LEFT + 
                                    (static_cast<float>(rand()) / RAND_MAX) * 
                                    (ENEMY_BOUNDARY_RIGHT - ENEMY_BOUNDARY_LEFT - NORMAL_ENEMY_WIDTH);
                                
                                for (const auto& existingEnemy : game.getEnemies()) {
                                    float existingX = existingEnemy.shape.getPosition().x;
                                    if (abs(randomX - existingX) < ENEMY_SPACING) {
                                        validPosition = false;
                                        break;
                                    }
                                }
                                
                                maxAttempts--;
                            } while (!validPosition && maxAttempts > 0);
                            
                            if (validPosition || maxAttempts <= 0) {
                                // 生成紅色 NORMAL 敵人
                                game.addEnemy(
                                    randomX, 
                                    0.f,
                                    sf::Vector2f(NORMAL_ENEMY_WIDTH, NORMAL_ENEMY_WIDTH),  // 正常尺寸
                                    sf::Color::Red,    // 紅色
                                    false,             // 可被摧
                                    EnemyType::NORMAL  // NORMAL 類型
                                );
                            }
                        }
                    }
                }
                enemySpawnTimer.restart();
            }

            // 修改 Boss 死亡檢測
            if (boss3 && boss3->isDead()) {
                if (boss3->hasRevivedOnce()) {
                    // 第二次死亡時遊戲勝利
                    gameWon = true;
                }
            }

            // 更新遊戲邏輯
            game.updateBullets();
            game.updateEnemies();

            // 檢查無敵時間是否結束
            if (isInvincible && invincibilityTimer.getElapsedTime().asSeconds() >= invincibilityDuration) {
                isInvincible = false;
            }

            // 再檢查失敗條件
            if (currentHealth <= 0) {
                isGameOver = true;
                gameWon = false;
            }

            // 更新並繪製擊殺數
            killCountText.setString("Kills: " + std::to_string(killCount));
            goldText.setString("Gold: " + std::to_string(gold));
            window.draw(killCountText);
            window.draw(goldText);

            // 移除檢查擊殺數的條件，直接更新和繪製 Boss
            if (boss3) {
                boss3->update();
                boss3->draw(window);

                // 檢查 Boss 是否剛剛復活
                if (boss3->justRevived) {
                    game.startMiddleScene();  // 開始過場動畫
                    boss3->justRevived = false;
                }
                
                // 檢查子彈是否擊中 Boss
                auto& bullets = game.getBullets();
                auto bulletIt = bullets.begin();
                while (bulletIt != bullets.end()) {
                    if (boss3->getSprite().getGlobalBounds().intersects(bulletIt->shape.getGlobalBounds())) {
                        if (!boss3->isInvincibleState()) {  // 檢查Boss是否處於無敵狀態
                            boss3->takeDamage(10);  // Boss 受到 10 點傷害
                            bulletIt = bullets.erase(bulletIt);  // 移除擊中的子彈
                            
                            // 檢查 Boss 是否死亡
                            if (boss3->isDead()) {
                                boss3.reset();  // 除 Boss
                                gameWon = true;  // 遊戲勝
                                break;
                            }
                        } else {
                            ++bulletIt;  // 如果Boss處於無敵狀態，子彈穿過
                        }
                    } else {
                        ++bulletIt;
                    }
                }
            }

            // Boss 射擊邏輯
            if (boss3 && !boss3->hasRevivedOnce() && boss3->shouldShoot()) {
                // 從 Boss 位置發射透明長方形子彈
                sf::Vector2f bossPos = boss3->getPosition();
                
                // 設置子彈大小
                const float ENEMY_BASE_SIZE = 30.f;
                sf::Vector2f bulletSize(ENEMY_BASE_SIZE * 7.f, ENEMY_BASE_SIZE);
                
                // 發射不可摧毀的子彈，設置完全透明
                game.addEnemy(
                    bossPos.x,           
                    bossPos.y + 50.f,    
                    bulletSize,          
                    sf::Color(0, 0, 0, 0),  // 修改為完全透明的黑色
                    true,                    // 設置為不可摧毀
                    EnemyType::HAND         // 設置為 HAND 類
                );
            }
        }
        else if (gameWon) {
            // 繪製勝利畫面
            window.draw(gameWonText);
            window.draw(victoryPromptText);
            // 不繪製擊殺數和金幣
        }
        else if (isGameOver) {
            // 繪製遊戲結束畫面
            window.draw(gameOverText);
            window.draw(promptText);
            // 不繪製擊殺數和金幣
        }

        // 在遊戲結束畫面中顯示金幣數量
        sf::Text goldText("Gold: " + std::to_string(gold), font, 30);
        goldText.setFillColor(sf::Color::Black);
        goldText.setPosition(10, 40);  // 調整位置以顯示金幣

        window.display();
    }

    return 0;
}
