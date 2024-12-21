#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>  // 為了使用 rand()
#include <ctime>    // 為了使用 time()
#include <iostream>  // 添加這行
#include <filesystem>  // 添加這行
#include <memory>  // 添加這行
using namespace sf;
using namespace std;

// 在檔案開頭定義全域常量
const float BOUNDARY_LEFT = 200.f;    // 左邊界
const float PLAY_AREA_WIDTH = 800.f;  // 遊戲區域寬度
const float ENEMY_WIDTH = 30.f;       // 敵人寬度
const float BOUNDARY_RIGHT = BOUNDARY_LEFT + PLAY_AREA_WIDTH;  // 右邊界

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
    
    Enemy(float startX, float startY) {
        speed = 0.23f;
        shape.setSize(sf::Vector2f(30.f, 30.f));  // 確保敵人有合適的大小
        shape.setPosition(startX, startY);
        shape.setFillColor(sf::Color::Red);
    }

    void update() {
        shape.move(0, speed);
    }

    // 修改碰撞檢測函數以使用 Sprite
    bool checkCollision(const Sprite& player) const {
        // 獲取精靈的邊界框
        FloatRect playerBounds = player.getGlobalBounds();
        FloatRect enemyBounds = shape.getGlobalBounds();
        
        return enemyBounds.intersects(playerBounds);
    }
};

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
    sf::SoundBuffer shootSoundBuffer;
    sf::Sound shootSound;
    sf::SoundBuffer playerHitSoundBuffer;
    sf::Sound playerHitSound;

public:
    Game(RenderWindow& win, int* killCount, int* gold) 
        : window(win), killCountPtr(killCount), goldPtr(gold) {
        std::vector<std::string> framePaths;
        for (int i = 1; i <= 24; i++) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "/Users/cpcap/GTA6/source/texture/background/frames/frame_%03d.png", i);
            std::string path = buffer;
            std::cout << "Trying to load: " << path << std::endl;
            framePaths.push_back(path);
        }

        background = new AnimatedBackground(
            framePaths, 
            0.1f, 
            sf::Vector2f(window.getSize().x, window.getSize().y)
        );

        if (!shootSoundBuffer.loadFromFile("/Users/cpcap/GTA6/source/assets_audio/shootSound.wav")) {
            std::cout << "Error loading shoot sound!" << std::endl;
        }
        shootSound.setBuffer(shootSoundBuffer);
        shootSound.setVolume(50.f);  // 設置音量（0-100）

        // 載入玩家受傷音效
        if (!playerHitSoundBuffer.loadFromFile("/Users/cpcap/GTA6/source/assets_audio/hurtSound.flac")) {
            std::cout << "Error loading player hit sound!" << std::endl;
        }
        playerHitSound.setBuffer(playerHitSoundBuffer);
        playerHitSound.setVolume(60.f);  // 設置音量
    }

    void update(float deltaTime) {
        if (background) {
            background->update(deltaTime);
        }
        // ... 其他更新邏輯 ...
    }

    void drawBackground() {
        if (background) {
            background->draw(window);
        }
    }

    // 在遊戲主循環中的繪製部分，首先繪製背景
    void draw() {
        drawBackground();
        // ... 繪製其他遊戲元素 ...
    }

    // 添加重置方法
    void reset() {
        bullets.clear();
        enemies.clear();
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
        auto bulletIt = bullets.begin();
        while (bulletIt != bullets.end()) {
            bulletIt->update();  // 使用 Bullet 類的 update 方法，而不是直接使用 velocity
            
            bool bulletHit = false;
            auto enemyIt = enemies.begin();
            
            while (enemyIt != enemies.end()) {
                if (bulletIt->shape.getGlobalBounds().intersects(enemyIt->shape.getGlobalBounds())) {
                    (*killCountPtr)++;
                    (*goldPtr) += 1000;
                    
                    std::cout << "擊中敵人！當前金幣: " << *goldPtr << std::endl;
                    
                    enemyIt = enemies.erase(enemyIt);
                    bulletHit = true;
                    break;
                } else {
                    ++enemyIt;
                }
            }
            
            // 將 isOutOfBounds 檢查移到 Game 類內部
            bool outOfBounds = bulletIt->shape.getPosition().y < 0;
            
            if (bulletHit || outOfBounds) {
                bulletIt = bullets.erase(bulletIt);
            } else {
                ++bulletIt;
            }
        }
    }

    void updateEnemies() {
        for (auto& enemy : enemies) {
            enemy.update();
        }
    }

    // 添加子彈和敵人
    void addBullet(float x, float y) {
        Bullet bullet(x, y);
        bullets.push_back(bullet);
        shootSound.play();
    }

    void addEnemy(float x, float y) {
        // 新的敵人邊界
        const float ENEMY_BOUNDARY_LEFT = 250.f;
        const float ENEMY_BOUNDARY_RIGHT = 950.f;
        const float ENEMY_WIDTH = 30.f;
        
        // 確保敵人在新的邊界內生成
        if (x < ENEMY_BOUNDARY_LEFT) {
            x = ENEMY_BOUNDARY_LEFT;
        }
        if (x > (ENEMY_BOUNDARY_RIGHT - ENEMY_WIDTH)) {
            x = ENEMY_BOUNDARY_RIGHT - ENEMY_WIDTH;
        }

        Enemy enemy(x, y);
        std::cout << "最終敵人位置X: " << x << std::endl;
        std::cout << "------------------------" << std::endl;
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
    bool checkPlayerCollision(const Sprite& playerSprite) {
        for (const auto& enemy : enemies) {
            if (enemy.checkCollision(playerSprite)) {
                return true;
            }
        }
        return false;
    }

    // 添加播放受傷音效的方法
    void playHitSound() {
        playerHitSound.play();
    }
};

class Boss_2 {
private:
    int currentPhase = 1;  // 添加這行，初始階段為1
    float targetX = 600.f;  // 添加這行，設置默認值
    sf::RectangleShape reviveCircle;
    sf::Vector2f reviveCirclePosition;
    bool showReviveCircle = false;
    sf::Clock reviveCircleTimer;
    const float LINE_INTERVAL = 0.75f;  // 改為每0.75秒產生一次
    const float REVIVE_CIRCLE_DURATION = 2.0f;  // 每個光束維持2秒
    bool hasRevivedOnce = false;  // 將其移至私有成員變量
    sf::Clock moveTimer;           // 計時器用於隨機移動
    float moveInterval = 0.8f;     // 將基礎時間間隔從 2.0f 改為 0.8f
    float currentMoveDirection = 1.f;  // 當前移動方向 (-1 左, 1 右)

    struct LaserBeam {
        sf::RectangleShape shape;
        sf::Clock lifetime;
        bool active = true;
    };
    std::vector<LaserBeam> laserBeams;
    sf::Clock beamSpawnTimer;
    const float BEAM_DURATION = 2.0f;         // 光束持續時間
    const float MIN_BEAM_INTERVAL = 0.75f;    // 最小發射間隔
    const float MAX_BEAM_INTERVAL = 1.25f;    // 最大發射間隔
    float currentBeamInterval = MIN_BEAM_INTERVAL;

    sf::CircleShape semicircle;  // 添加半圓形

    sf::SoundBuffer beamSoundBuffer;
    sf::Sound beamSound;

    // 過場動畫相關
    bool isInTransition = false;
    std::vector<sf::Texture> transitionTextures;
    sf::Sprite transitionSprite;
    int currentTransitionFrame = 0;
    sf::Clock transitionTimer;
    const float TRANSITION_FRAME_TIME = 0.1f;
    bool waitingForKeyPress = false;  // 添加等待按鍵的標誌
    
    // 修改載入過場動畫圖片的方法
    void loadTransitionImages() {
        // 添加調試輸出
        std::cout << "Starting to load transition images..." << std::endl;
        
        for (int i = 76; i <= 82; i++) {
            sf::Texture texture;
            // 修改路徑格式
            std::string filename = "/Users/cpcap/GTA6/source/resources/2/" + std::to_string(i) + ".png";
            
            std::cout << "Trying to load: " << filename << std::endl;
            
            if (!texture.loadFromFile(filename)) {
                std::cout << "Failed to load transition image: " << filename << std::endl;
                continue;
            }
            
            transitionTextures.push_back(texture);
            std::cout << "Successfully loaded image " << i << std::endl;
        }
        
        std::cout << "Loaded " << transitionTextures.size() << " transition images" << std::endl;
        
        if (!transitionTextures.empty()) {
            transitionSprite.setTexture(transitionTextures[0]);
            // 設置精靈以填滿整個窗口
            float scaleX = 1200.0f / transitionTextures[0].getSize().x;  // 假設窗口寬度為1200
            float scaleY = 800.0f / transitionTextures[0].getSize().y;   // 假設窗口高度為800
            transitionSprite.setScale(scaleX, scaleY);
        }
    }

    void startPhaseTwo() {
        currentPhase = 2;
        currentHealth = maxHealth;  // 重置血量
        isInvincible = false;      // 取消無敵狀態
        moveSpeed *= 2.0f;         // 加快移動速度
    }

    bool isDeadState = false;  // 改名為 isDeadState
    int health = 100;         // 確保有初始血量

    void spawnBeam() {
        LaserBeam newBeam;
        float beamHeight = 800.f - y;  // 假設視窗高度為800
        newBeam.shape.setSize(sf::Vector2f(40.f, beamHeight));
        
        // 修改為半透明白色
        newBeam.shape.setFillColor(sf::Color(255, 255, 255, 120));  // RGBA: 白色，透明度為120
        
        newBeam.shape.setPosition(x - 20.f, y);  // 從Boss中心位置發射
        newBeam.lifetime.restart();
        laserBeams.push_back(newBeam);
        
        // 播放音效
        beamSound.play();
    }

public:
    Boss_2(const std::string& texturePath, float startX = 600.f, float startY = 100.f);
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

    bool hasRevived() const { 
        return hasRevivedOnce; 
    }

    // 添加碰撞檢測方法
    bool checkCollision(const sf::Sprite& playerSprite) const {
        return sprite.getGlobalBounds().intersects(playerSprite.getGlobalBounds());
    }

    bool checkLineCollision(const sf::Sprite& playerSprite) const {
        // 檢查所有活躍光束的碰撞
        for (const auto& beam : laserBeams) {
            if (beam.shape.getGlobalBounds().intersects(playerSprite.getGlobalBounds())) {
                return true;
            }
        }
        return false;
    }

    void move();
    void setTargetPosition(float playerX);

    void startTransition();

    bool isInTransitionState() const {
        return isInTransition;
    }

private:
    sf::Texture texture;
    sf::Sprite sprite;
    float x, y;
    float moveSpeed;
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

};

Boss_2::Boss_2(const std::string& texturePath, float startX, float startY)
    : x(startX), y(startY), moveSpeed(0.1f), health(150), movingRight(true), isInvincible(false) {

    if (!texture.loadFromFile(texturePath)) {
        std::cout << "Error loading boss texture!" << std::endl;
        //std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
        return;
    }

    sprite.setTexture(texture);
    sprite.setOrigin(texture.getSize().x / 2.f, texture.getSize().y / 2.f);
    sprite.setPosition(x, y);

    float desiredWidth = 300.f;   // 從 200.f 增加到 260.f (1.3倍)
    float desiredHeight = 300.f;  // 從 200.f 增加到 260.f (1.3倍)
    sprite.setScale(
        desiredWidth / texture.getSize().x,
        desiredHeight / texture.getSize().y
    );

    bossWidth = sprite.getGlobalBounds().width;
    bossHeight = sprite.getGlobalBounds().height;

        // 修改血條初始化
    healthBarBackground.setSize(Vector2f(40.f, 600.f));
    healthBar.setSize(Vector2f(40.f, 600.f));
    
    // 設置血條位置（改為左側）
    healthBarBackground.setPosition(50.f, 100.f);
    healthBar.setPosition(50.f, 100.f);
    
    // 修改血條顏色
    healthBarBackground.setFillColor(Color(100, 100, 100));  // 深灰色背景
    healthBar.setFillColor(Color::Red);                      // 紅色血條
    
    // 添加血條邊框
    healthBarBackground.setOutlineThickness(2.f);
    healthBarBackground.setOutlineColor(Color::White);       // 白色邊框
    
    // 設置血量
    maxHealth = 150.f;
    currentHealth = 150.f;

    // 初始化隨機數生成器（如果還沒有初始化）
    static bool seeded = false;
    if (!seeded) {
        srand(time(nullptr));
        seeded = true;
    }

    // 修改直線的尺寸和位置
    float lineHeight = 800.f - y;  // 從Boss中心到螢幕底部的高度
    reviveCircle.setSize(sf::Vector2f(40.f, lineHeight));  // 寬度40像素
    reviveCircle.setOrigin(20.f, 0.f);  // 設置原點在直線的頂部中心
    reviveCircle.setPosition(x, y);  // 從Boss中心開始
    reviveCircle.setFillColor(sf::Color::White);  // 改為白色
    
    // 設置圓形
    semicircle.setRadius(20.f);  // 半徑與直線寬度的一半相同
    semicircle.setFillColor(sf::Color::White);
    semicircle.setOrigin(20.f, 20.f);  // 設置原點在半圓形的底部中心
    semicircle.setRotation(180.f);  // 旋轉180度使其朝上

    // 初始化光束相關變量
    currentBeamInterval = MIN_BEAM_INTERVAL;  // 設置初始間隔
    beamSpawnTimer.restart();  // 重置計時器

    // 載入音效
    if (!beamSoundBuffer.loadFromFile("/Users/cpcap/GTA6/source/assets_audio/beam.wav")) {  // 使用已有的 shoot.ogg
        std::cout << "Error loading beam sound!" << std::endl;
    }
    beamSound.setBuffer(beamSoundBuffer);
    beamSound.setVolume(50.f);

    loadTransitionImages();
    
    // 設置過場動畫精靈的位置（根據需要調整）
    transitionSprite.setPosition(0, 0);
}

void Boss_2::move() {
    if (isInTransition) return;  // 如果在過場動畫中，不移動
    
    if (!hasRevivedOnce) {
        // 第一階段移動邏輯
        if (moveTimer.getElapsedTime().asSeconds() >= moveInterval) {
            currentMoveDirection = (rand() % 2) * 2 - 1;  // 隨機選擇 -1 或 1
            moveTimer.restart();
        }
        x += moveSpeed * currentMoveDirection;
    } else {
        // 第二階段移動邏輯
        float enhancedSpeed = moveSpeed * 2.0f;
        if (moveTimer.getElapsedTime().asSeconds() >= moveInterval) {
            currentMoveDirection = (rand() % 2) * 2 - 1;
            moveTimer.restart();
        }
        x += enhancedSpeed * currentMoveDirection;
    }
    
    // 確保Boss不會超出邊界
    if (x <= 200.f) {
        x = 200.f;
        currentMoveDirection = 1.f;
    }
    else if (x >= 1000.f) {
        x = 1000.f;
        currentMoveDirection = -1.f;
    }
    
    sprite.setPosition(x, y);
}

void Boss_2::setTargetPosition(float playerX) {
    targetX = playerX;  // 儲存玩家位置供第二階段使用
}

void Boss_2::update() {
    if (isInTransition) {
        // 檢查是否按下空白鍵
        if (waitingForKeyPress && sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            // 防止連續觸發，等待鬆開空白鍵
            if (transitionTimer.getElapsedTime().asSeconds() >= 0.2f) {  // 添加少許延遲
                std::cout << "Space pressed, moving to next frame" << std::endl;
                currentTransitionFrame++;
                
                if (currentTransitionFrame >= transitionTextures.size()) {
                    std::cout << "Transition animation completed" << std::endl;
                    isInTransition = false;
                    currentTransitionFrame = 0;
                    startPhaseTwo();
                } else {
                    transitionSprite.setTexture(transitionTextures[currentTransitionFrame]);
                }
                
                transitionTimer.restart();
                waitingForKeyPress = false;  // 等待鬆開空白鍵
            }
        }
        else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            waitingForKeyPress = true;  // 空白鍵已鬆開，可以接受下一次按鍵
        }
        return;
    }
    
    // 如果不在過場動畫中，繼續正常移動
    move();

    // 第二階段的光束邏輯
    if (hasRevivedOnce && !isInTransition) {
        // 檢查是否應該發射新的光束
        if (beamSpawnTimer.getElapsedTime().asSeconds() >= currentBeamInterval) {
            spawnBeam();
            beamSpawnTimer.restart();
            
            // 設置下一次發射的隨機間隔
            float randomFactor = static_cast<float>(rand()) / RAND_MAX;
            currentBeamInterval = MIN_BEAM_INTERVAL + 
                randomFactor * (MAX_BEAM_INTERVAL - MIN_BEAM_INTERVAL);
        }
    }

    // 更新現有光束
    auto beamIt = laserBeams.begin();
    while (beamIt != laserBeams.end()) {
        if (beamIt->lifetime.getElapsedTime().asSeconds() >= BEAM_DURATION) {
            beamIt = laserBeams.erase(beamIt);
        } else {
            ++beamIt;
        }
    }
}

void Boss_2::draw(sf::RenderWindow& window) {
    if (isInTransition) {
        window.draw(transitionSprite);
        return;
    }

    // 先繪製Boss
    window.draw(sprite);
    
    // 繪製血條
    window.draw(healthBarBackground);
    window.draw(healthBar);
    
    // 繪製所有活躍的光束
    for (const auto& beam : laserBeams) {
        window.draw(beam.shape);
    }
}

void Boss_2::takeDamage(int damage) {
    if (isInvincible) return;
    
    currentHealth -= damage;
    health -= damage;
    
    // 更新血條
    float healthBarHeight = (currentHealth / maxHealth) * 600.f;
    healthBar.setSize(Vector2f(40.f, healthBarHeight));
    healthBar.setPosition(50.f, 100.f + (600.f - healthBarHeight));
    
    // 第一階段結束時觸發過場動畫
    if (currentHealth <= 0 && !hasRevivedOnce) {
        std::cout << "Triggering transition animation..." << std::endl;
        startTransition();
        currentHealth = maxHealth;
        health = maxHealth;
        revivedCount++;
        hasRevivedOnce = true;
        isInvincible = true;  // 在過場動畫期間保持無敵
    }
    // 第二階段的死亡
    else if (currentHealth <= 0 && hasRevivedOnce) {
        isDeadState = true;
        health = 0;
    }
}

bool Boss_2::isDead() const {
    return health <= 0 && revivedCount > 0;  // 只有在復活過且血量為0時才算真正死亡
}

const sf::Sprite& Boss_2::getSprite() const {
    return sprite;
}

float Boss_2::getX() const {
    return x;
}

float Boss_2::getY() const {
    return y;
}

float Boss_2::getWidth() const {
    return bossWidth;
}

float Boss_2::getHeight() const {
    return bossHeight;
}

void Boss_2::setPosition(float newX, float newY) {
    x = newX;
    y = newY;
    sprite.setPosition(x, y);
}

void Boss_2::startTransition() {
    std::cout << "Starting transition animation..." << std::endl;
    isInTransition = true;
    currentTransitionFrame = 0;
    transitionTimer.restart();
    waitingForKeyPress = true;  // 初始化為等待按鍵狀態
    
    if (!transitionTextures.empty()) {
        transitionSprite.setTexture(transitionTextures[0]);
    }
}

int main() {
    RenderWindow window(VideoMode(1200, 800), "SFML works!");
    
    // 添加背景音樂
    sf::Music backgroundMusic;
    if (!backgroundMusic.openFromFile("/Users/cpcap/GTA6/source/assets_audio/backgroundMusic.wav")) {
        std::cout << "Error loading background music!" << std::endl;
    } else {
        backgroundMusic.setLoop(true);  // 設置循環播放
        backgroundMusic.setVolume(30.f);  // 設置音量（0-100）
        backgroundMusic.play();  // 開始播放
    }


    srand(time(0));  // 初始化隨機數生成器
    
    // 加載玩家材質
    Texture playerTexture;
    if (!playerTexture.loadFromFile("/Users/cpcap/GTA6/source/resources/character/player.png")) {
        cout << "Error loading player texture!" << endl;
        return -1;
    }

    //新增
    // 加載Boss_2材質
    Boss_2("/Users/cpcap/GTA6/source/resources/character/boss_2.png");
    
    // 創建玩家精靈替代原來的 CircleShape
    Sprite playerSprite(playerTexture);
    // 設置精靈原點為中心
    playerSprite.setOrigin(playerTexture.getSize().x / 2.f, playerTexture.getSize().y / 2.f);
    
    float x = BOUNDARY_LEFT + PLAY_AREA_WIDTH/2;  // 在遊戲區域中心
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

    // 設置移動邊界，考慮玩家寬度
    float leftBound = BOUNDARY_LEFT;                         // 左邊界
    float rightBound = BOUNDARY_LEFT + PLAY_AREA_WIDTH - playerWidth;  // 右邊界減去玩家寬度

    // 子彈容器
    std::vector<Bullet> bullets;
    bool spacePressed = false;

    // 修改血條設置
    RectangleShape healthBarBackground(Vector2f(20.f, 200.f));
    RectangleShape healthBar(Vector2f(20.f, 200.f));

    // 設置血條位置（螢幕中間偏右）
    healthBarBackground.setPosition(1150.f, 300.f);  // x=1150 會在螢幕右側，y=300 會在螢幕中間偏上
    healthBar.setPosition(1150.f, 300.f);           // 保持與背景相同位置

    // 設置血條顏色和邊框
    healthBarBackground.setFillColor(Color(100, 100, 100));
    healthBar.setFillColor(Color::Green);

    // 添加血條邊框
    healthBarBackground.setOutlineThickness(2.f);
    healthBarBackground.setOutlineColor(Color::White);
    
    // 設置血量
    float maxHealth = 100.f;
    float currentHealth = 100.f;

    // 敵人關變量
    std::vector<Enemy> enemies;
    Clock enemySpawnTimer;  // 用於計時生成敵人
    
    // 添加無敵時間計時器
    Clock invincibilityTimer;
    bool isInvincible = false;
    float invincibilityDuration = 1.0f;  // 1無敵時間

    // 在創建 Game 實例前定義 killCount
    int killCount = 0;
    int gold = 0;  // 初始金幣

    // 修改載入字體的部分
    sf::Font font;
    if (!font.loadFromFile("/Users/cpcap/GTA6/source/resources/fonts/arial.ttf")) {  // macOS 系統字體路徑
        // 如果 Arial 載入失敗，嘗試載入系統默認字體
        if (!font.loadFromFile("/Users/cpcap/GTA6/source/resources/fonts/PingFangTC-Regular.otf")) {
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
    
    // 創建提示文字
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

    // 添加遊戲狀態
    bool isGameOver = false;
    bool gameWon = false;

    // 添加子彈發射計時器
    Clock shootTimer;
    const float shootCooldown = 0.5f;  // 射擊冷卻時間（秒）

    // 添加敵人生成計時器
    const float ENEMY_SPAWN_INTERVAL = 0.25f;  // 調整為0.25秒
    const float ENEMY_SPAWN_CHANCE = 0.5f;     // 調整為50%的生成機率

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

    // 設置勝利文字位置
    gameWonText.setPosition(
        window.getSize().x/2 - gameWonText.getGlobalBounds().width/2,
        window.getSize().y/2 - gameWonText.getGlobalBounds().height/2 - 50
    );
    victoryPromptText.setPosition(
        window.getSize().x/2 - victoryPromptText.getGlobalBounds().width/2,
        window.getSize().y/2 + 50
    );

    // 在 main 函數開始處添加自動發射的計時器和間隔設置
    Clock autoShootTimer;  // 自動射計時器
    const float autoShootInterval = 0.5f;  // 每0.5秒發射一次，你可以調整這個值

    sf::Clock clock;  // 添加時間來計算幀時間
    
    // 直接創建 Boss_2 實例
    std::unique_ptr<Boss_2> boss3 = std::make_unique<Boss_2>(
        "/Users/cpcap/GTA6/source/resources/character/boss_2.png",
        600.f,  // x 位置
        130.f   // y 位置改為 150.f
    );

    // 在 main 函數開始處
    const float SHOOT_COOLDOWN = 0.4f;  // 射擊冷卻時間
    sf::Clock shootCooldownTimer;       // 射擊冷卻計時器

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
                // 添加H鍵扣血的測試
                else if (event.key.code == Keyboard::H) {
                    currentHealth = std::max(0.f, currentHealth - 10.f);
                    float healthBarHeight = (currentHealth/maxHealth) * 200.f;
                    healthBar.setSize(Vector2f(20.f, healthBarHeight));
                    healthBar.setPosition(1150.f, 300.f + (200.f - healthBarHeight));
                    
                    // 如果血量歸��，觸發遊戲結束
                    if (currentHealth <= 0) {
                        isGameOver = true;
                    }
                }
            }
            
            // 遊戲結束時的按鍵處理
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
                    killCount = 0;  // 重置擊殺數
                    gold = 0;   // 重置金幣數量
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
                    
                    // 如果血量歸，觸發遊戲結束
                    if (currentHealth <= 0) {
                        isGameOver = true;
                    }
                }
            }

            // 添加勝時的按鍵處理
            if (gameWon && event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::R) {
                    // 重置遊戲狀態
                    currentHealth = maxHealth;
                    healthBar.setSize(Vector2f(20.f, 200.f));
                    x = BOUNDARY_LEFT + PLAY_AREA_WIDTH/2;
                    y = 730.f;
                    playerSprite.setPosition(x, y);
                    game.reset();
                    killCount = 0;  // 重置擊殺數
                    gold = 0;   // 重置金幣數量
                    gameWon = false;
                    
                    // 更新顯示文字
                    killCountText.setString("Kills: 0");
                    goldText.setString("Gold: 0");
                }
                else if (event.key.code == Keyboard::Escape) {
                    window.close();
                }
            }
        }

        // 在遊戲循環中修改碰撞檢測的部分
        if (!isInvincible) {
            bool collisionOccurred = false;

            // 檢查與敵人的碰撞
            auto enemyIt = game.getEnemies().begin();
            while (enemyIt != game.getEnemies().end()) {
                if (enemyIt->checkCollision(playerSprite)) {
                    collisionOccurred = true;
                    enemyIt = game.getEnemies().erase(enemyIt);
                    killCount++;
                    gold += 1000;
                    break;
                } else {
                    ++enemyIt;
                }
            }

            // 檢查與Boss的碰撞
            if (boss3 && boss3->checkCollision(playerSprite)) {
                collisionOccurred = true;
            }

            // 檢查與Boss光束的碰撞
            if (boss3 && boss3->checkLineCollision(playerSprite)) {
                collisionOccurred = true;
            }

            // 如果發生任何碰撞，處理扣血和音效
            if (collisionOccurred) {
                // 播放受傷音效
                game.playHitSound();
                
                // 扣血
                currentHealth = std::max(0.f, currentHealth - 10.f);
                float healthBarHeight = (currentHealth/maxHealth) * 200.f;
                healthBar.setSize(Vector2f(20.f, healthBarHeight));
                healthBar.setPosition(1150.f, 300.f + (200.f - healthBarHeight));
                
                // 設置無敵時間
                isInvincible = true;
                invincibilityTimer.restart();
                
                // 更新UI文字
                killCountText.setString("Kills: " + std::to_string(killCount));
                goldText.setString("Gold: " + std::to_string(gold));
                
                // 檢查是否死亡
                if (currentHealth <= 0) {
                    isGameOver = true;
                }
            }
        }

        // 修改血量檢查邏輯
        if (currentHealth <= 0) {
            isGameOver = true;
        }

        window.clear();

        // 修改遊戲狀態檢查的邏輯
        if (!isGameOver && !gameWon) {  // 確保兩個狀態互斥
            game.update(deltaTime);  // 更新遊戲狀態，包括背景動畫
            game.drawBackground();   // 繪製背景
            
            // 繪製敵人
            for (const auto& enemy : game.getEnemies()) {
                window.draw(enemy.shape);
            }
            
            // 繪製玩家和子彈
            window.draw(playerSprite);
            for (const auto& bullet : game.getBullets()) {
                window.draw(bullet.shape);
            }
            
            // 繪製條
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
                // 限制下邊界，確保玩家會移出畫面
                y = std::min(window.getSize().y - playerSprite.getGlobalBounds().height/2.f, y + moveSpeed);
            }

            playerSprite.setPosition(x, y);
            
            // 射擊控制
            if (Keyboard::isKeyPressed(Keyboard::Space)) {
                // 檢查是否在過場動畫中
                bool inTransition = (boss3 && boss3->isInTransitionState());
                
                if (!inTransition && shootCooldownTimer.getElapsedTime().asSeconds() >= SHOOT_COOLDOWN) {
                    // 從玩家中心位置發射子彈
                    float bulletX = playerSprite.getPosition().x;
                    float bulletY = playerSprite.getPosition().y - playerSprite.getGlobalBounds().height/2.f;
                    
                    game.addBullet(bulletX, bulletY);
                    shootCooldownTimer.restart();
                }
            }

            // 修改敵人生成邏輯
            if (enemySpawnTimer.getElapsedTime().asSeconds() >= ENEMY_SPAWN_INTERVAL) {
                if (!boss3 || !boss3->hasRevived()) {
                    float spawnRoll = static_cast<float>(rand()) / RAND_MAX;
                    
                    if (spawnRoll < ENEMY_SPAWN_CHANCE) {
                        // 調整每次生成的敵人數量
                        int enemyCount = 1 + (rand() % 3);  // 改回1-3個
                        
                        const float ENEMY_BOUNDARY_LEFT = 250.f;
                        const float ENEMY_BOUNDARY_RIGHT = 950.f;
                        const float ENEMY_WIDTH = 30.f;
                        const float ENEMY_SPACING = 45.f;  // 稍微增加間距
                        
                        for (int i = 0; i < enemyCount; ++i) {
                            // 生成隨機位置
                            float randomX;
                            bool validPosition;
                            int maxAttempts = 10;  // 最大嘗試次數
                            
                            do {
                                validPosition = true;
                                randomX = ENEMY_BOUNDARY_LEFT + 
                                    (static_cast<float>(rand()) / RAND_MAX) * 
                                    (ENEMY_BOUNDARY_RIGHT - ENEMY_BOUNDARY_LEFT - ENEMY_WIDTH);
                                
                                // 檢查與現有敵人的距離
                                for (const auto& existingEnemy : game.getEnemies()) {
                                    float existingX = existingEnemy.shape.getPosition().x;
                                    if (abs(randomX - existingX) < ENEMY_SPACING) {
                                        validPosition = false;
                                        break;
                                    }
                                }
                                
                                maxAttempts--;
                            } while (!validPosition && maxAttempts > 0);
                            
                            // 確保在邊界內
                            if (randomX > (ENEMY_BOUNDARY_RIGHT - ENEMY_WIDTH)) {
                                randomX = ENEMY_BOUNDARY_RIGHT - ENEMY_WIDTH;
                            }
                            
                            // 如果找到有效位置或達到最大嘗試次數，生成敵人
                            if (validPosition || maxAttempts <= 0) {
                                game.addEnemy(randomX, 0.f);
                            }
                        }
                    }
                }
                enemySpawnTimer.restart();
            }

            // 修改 Boss 死亡檢測
            if (boss3 && boss3->isDead()) {
                if (boss3->hasRevived()) {
                    // 第二次死亡時遊戲勝利
                    gameWon = true;
                }
            }

            // 更新遊戲邏輯
            game.updateBullets();
            game.updateEnemies();

            // 檢測玩家與人和Boss的碰撞
            if (!isInvincible) {
                bool collisionOccurred = false;

                // 檢查與敵人的碰撞
                auto enemyIt = game.getEnemies().begin();
                while (enemyIt != game.getEnemies().end()) {
                    if (enemyIt->checkCollision(playerSprite)) {
                        collisionOccurred = true;
                        enemyIt = game.getEnemies().erase(enemyIt);
                        killCount++;
                        gold += 1000;
                        break;
                    } else {
                        ++enemyIt;
                    }
                }

                // 檢查與Boss的碰撞
                if (boss3 && boss3->checkCollision(playerSprite)) {
                    collisionOccurred = true;
                }

                // 檢查與Boss光束的碰撞
                if (boss3 && boss3->checkLineCollision(playerSprite)) {
                    collisionOccurred = true;
                }

                // 如果發生任何碰撞，處理扣血和音效
                if (collisionOccurred) {
                    // 播放受傷音效
                    game.playHitSound();
                    
                    // 扣血
                    currentHealth = std::max(0.f, currentHealth - 10.f);
                    float healthBarHeight = (currentHealth/maxHealth) * 200.f;
                    healthBar.setSize(Vector2f(20.f, healthBarHeight));
                    healthBar.setPosition(1150.f, 300.f + (200.f - healthBarHeight));
                    
                    // 設置無敵時間
                    isInvincible = true;
                    invincibilityTimer.restart();
                    
                    // 更新UI文字
                    killCountText.setString("Kills: " + std::to_string(killCount));
                    goldText.setString("Gold: " + std::to_string(gold));
                    
                    // 檢查是否死亡
                    if (currentHealth <= 0) {
                        isGameOver = true;
                    }
                }
            }

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
                boss3->setTargetPosition(playerSprite.getPosition().x);
                boss3->update();
                boss3->draw(window);

                // 檢查子彈是否擊中 Boss
                auto& bullets = game.getBullets();
                auto bulletIt = bullets.begin();
                while (bulletIt != bullets.end()) {
                    if (boss3->getSprite().getGlobalBounds().intersects(bulletIt->shape.getGlobalBounds())) {
                        if (!boss3->isInvincibleState()) {  // 檢查Boss是否處於無敵狀態
                            boss3->takeDamage(10);  // 將傷害從 10 調整為 30
                            bulletIt = bullets.erase(bulletIt);  // 移除擊中的子彈
                            
                            // 檢查 Boss 是否死亡
                            if (boss3->isDead()) {
                                boss3.reset();  // 移除 Boss
                                gameWon = true;  // 遊戲勝利
                                break;
                            }
                        } else {
                            ++bulletIt;  // 果Boss處於無敵狀態，子彈穿過
                        }
                    } else {
                        ++bulletIt;
                    }
                }
            }

            // 檢查與線的碰撞
            if (!isInvincible && boss3 && boss3->checkLineCollision(playerSprite)) {
                currentHealth = std::max(0.f, currentHealth - 10.f);
                float healthBarHeight = (currentHealth/maxHealth) * 200.f;
                healthBar.setSize(Vector2f(20.f, healthBarHeight));
                healthBar.setPosition(1150.f, 300.f + (200.f - healthBarHeight));
                
                isInvincible = true;
                invincibilityTimer.restart();
                
                if (currentHealth <= 0) {
                    isGameOver = true;
                }
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