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
        speed = 0.5f;
        shape.setRadius(7.f);
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
        speed = 0.05f;
        shape.setSize(sf::Vector2f(50.f, 50.f));  // 確保敵人有合適���小
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
};

class Projectile {
public:
    sf::Sprite bikeSprite;

    Projectile(const sf::Texture& texture, sf::Vector2f startPosition) {
        bikeSprite.setTexture(texture);
        bikeSprite.setPosition(startPosition);
        
        // Make the bike smaller - adjust these values to change size
        float scale = 0.3f;  // Reduce this value to make it smaller
        bikeSprite.setScale(scale, scale);
    }

    void update(float deltaTime) {
        // Increase the speed by increasing this multiplier
        bikeSprite.move(0.f, 600.f * deltaTime);  // Changed from 300.f to 500.f for faster movement
    }

    bool isOutOfBounds(float screenHeight) {
        return bikeSprite.getPosition().y > screenHeight;
    }
};

class Boss_3 {
public:   
    Boss_3(const std::string& texturePath, sf::Sound& transitionSnd, float startX = 600.f, float startY = 100.f);
    void update(float deltaTime, float screenHeight);
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
        return sprite.getGlobalBounds().intersects(playerSprite.getGlobalBounds());
    }
    void shoot() {
        // Create projectile at the bottom-center of the boss
        sf::Vector2f startPosition(
            sprite.getPosition().x,
            sprite.getPosition().y + sprite.getGlobalBounds().height/2
        );
        projectiles.emplace_back(projectileTexture, startPosition);
    }
    void updateProjectiles(float deltaTime, float screenHeight) {
        // Update each projectile
        for (auto& projectile : projectiles) {
            projectile.update(deltaTime);
        }

        // Remove projectiles that go out of bounds
        projectiles.erase(
            std::remove_if(projectiles.begin(), projectiles.end(),
                [screenHeight](Projectile& proj) { return proj.isOutOfBounds(screenHeight); }),
            projectiles.end());
    }
    void drawProjectiles(sf::RenderWindow& window) {
        for (const auto& projectile : projectiles) {
            window.draw(projectile.bikeSprite);
        }
    }
    const std::vector<Projectile>& getProjectiles() const {
        return projectiles;
    }

    // Add getters for boundary positions
    float getLeftBoundaryX() const { return leftBoundaryX; }
    float getRightBoundaryX() const { return rightBoundaryX; }
    bool hasBoundaries() const { return boundariesSet && revivedCount > 0; }

    // Add method to set up boundaries
    void setupBoundaries() {
        if (revivedCount > 0) {
            // Generate random positions for boundaries
            const float MIN_SPACE = 300.f;  // Minimum space between lines
            const float LEFT_LIMIT = 250.f;
            const float RIGHT_LIMIT = 950.f;
            
            // Generate random positions
            leftBoundaryX = LEFT_LIMIT + (rand() % int(RIGHT_LIMIT - LEFT_LIMIT - MIN_SPACE));
            rightBoundaryX = leftBoundaryX + MIN_SPACE + (rand() % int(RIGHT_LIMIT - leftBoundaryX - MIN_SPACE));
            
            // Set up left boundary line
            leftBoundary.setSize(sf::Vector2f(10.f, 800.f));
            leftBoundary.setPosition(leftBoundaryX, 0.f);
            leftBoundary.setFillColor(sf::Color::Red);
            
            // Set up right boundary line
            rightBoundary.setSize(sf::Vector2f(10.f, 800.f));
            rightBoundary.setPosition(rightBoundaryX, 0.f);
            rightBoundary.setFillColor(sf::Color::Red);
            
            boundariesSet = true;
            boundaryTimer.restart();
        }
    }

    void startTransition();
    void setTargetPosition(float playerX);
    bool isInTransition = false;

    bool isInTransitionState() const { return isInTransition; }

private:
    std::vector<Projectile> projectiles;
    sf::Texture projectileTexture; // Texture for projectiles
    sf::Texture texture;
    sf::Sprite sprite;
    int currentPhase = 1;  // 添加這行，初始階段為1
    float x, y;
    float moveSpeed;
    bool movingRight;
    float bossWidth;
    float bossHeight;
    float targetX = 600.f;  // 添加這行，設置默認值
    sf::RectangleShape healthBarBackground;
    sf::RectangleShape healthBar;
    float maxHealth;
    float currentHealth;

    bool isInvincible;
    sf::Clock invincibilityTimer;
    const float invincibilityDuration = 0.5f;  // 無敵時間持續0.5秒

    int revivedCount = 0;  // 蹤復活次數

    sf::Clock moveTimer;           // 計時器用於隨機移動
    float moveInterval = 0.5f;     // 將基礎時間間隔從 2.0f 改為 0.8f
    float currentMoveDirection = 1.f;  // 當前移動方向 (-1 左, 1 右)

    // Add a separate timer for shooting
    sf::Clock shootTimer;
    float shootInterval = 0.8f;  // Shoot every 0.8 seconds

    // 移動邏輯
    void move();

    // Replace the boundary rectangles with lines
    sf::RectangleShape leftBoundary;
    sf::RectangleShape rightBoundary;
    const float BOUNDARY_REGEN_INTERVAL = 5.0f;  // Change to 5 seconds
    const float LINE_LENGTH = 800.f;  // Length of the lines

    // Add boundary lines for second phase
    float leftBoundaryX;
    float rightBoundaryX;
    bool boundariesSet = false;

    // Add timer for boundary regeneration
    sf::Clock boundaryTimer;

    sf::Sound& transitionSound;  // Reference to the transition sound
        // 過場動畫相關

    std::vector<sf::Texture> transitionTextures;
    sf::Sprite transitionSprite;
    int currentTransitionFrame = 0;
    sf::Clock transitionTimer;
    const float TRANSITION_FRAME_TIME = 0.1f;
    bool waitingForKeyPress = false;  // 添加等待按鍵的標誌
    
    // 修改載入過場動畫圖片的方法
    void loadTransitionImages() {
        std::cout << "Starting to load transition images..." << std::endl;
        
        for (int i = 0; i <= 3; i++) {
            sf::Texture texture;
            // Fix the path - remove "textures" and use "texture"
            std::string filename = "/Users/cpcap/GTA6/source/texture/story/slide" + std::to_string(i) + ".png";
            
            std::cout << "Trying to load: " << filename << std::endl;
            
            if (!texture.loadFromFile(filename)) {
                std::cout << "Failed to load transition image: " << filename << std::endl;
                continue;
            }
            
            transitionTextures.push_back(texture);
            std::cout << "Successfully loaded image " << i << std::endl;
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

};

Boss_3::Boss_3(const std::string& texturePath, sf::Sound& transitionSnd, float startX, float startY)
    : x(startX), y(startY), moveSpeed(0.1f), health(100), movingRight(true), isInvincible(false),
      currentMoveDirection(1.f), moveInterval(0.8f), transitionSound(transitionSnd) {  // 初始間隔也改為 0.8f

    if (!texture.loadFromFile(texturePath)) {
        std::cout << "Error loading boss texture!" << std::endl;
        //std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
        return;
    }
    if (!projectileTexture.loadFromFile("/Users/cpcap/GTA6/source/texture/character/bike.png")) {
        std::cout << "Error loading bike texture!" << std::endl;
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

    // 初始化隨機數生成器（如果還沒初始化）
    static bool seeded = false;
    if (!seeded) {
        srand(time(nullptr));
        seeded = true;
    }
    loadTransitionImages();
    
    // 設置過場動畫精靈的位置（根據需要調整）
    transitionSprite.setPosition(0, 0);
}


void Boss_3::move() {
    if (isInTransition) return;  // 如果在過場動畫中，不移動
    
    if (!hasRevivedOnce()) {
        // 一階段移動邏輯
        if (moveTimer.getElapsedTime().asSeconds() >= moveInterval) {
            currentMoveDirection = (rand() % 2) * 2 - 1;  // 隨機選擇 -1 或 1
            moveTimer.restart();
        }
        x += moveSpeed * currentMoveDirection;
    } else {
        // 第二階段移動邏輯
        float enhancedSpeed = moveSpeed * 1.2f;
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

void Boss_3::setTargetPosition(float playerX) {
    targetX = playerX;  // 儲存玩家位置供第二階段使用
}

void Boss_3::update(float deltaTime, float screenHeight) {
    if (isInTransition) {
        isInvincible = true;
        // Only handle space key press during transition
        if (waitingForKeyPress && sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            if (transitionTimer.getElapsedTime().asSeconds() >= 0.2f) {
                currentTransitionFrame++;
                std::cout << "Moving to next slide: " << currentTransitionFrame << std::endl;
                
                if (currentTransitionFrame >= transitionTextures.size()) {
                    std::cout << "Transition complete, starting phase 2" << std::endl;
                    isInTransition = false;
                    isInvincible = false;
                    revivedCount++;  // Now increment revivedCount
                    setupBoundaries();  // Setup boundaries for phase 2
                    transitionSound.play();  // Play transition sound
                } else {
                    // Update sprite texture for next slide
                    transitionSprite.setTexture(transitionTextures[currentTransitionFrame]);
                }
                
                transitionTimer.restart();
                waitingForKeyPress = false;
            }
        } else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            waitingForKeyPress = true;
        }
        return;  // Skip other updates during transition
    }

    // Move the boss
    move();

    // Only handle boundaries and shooting in second phase
    if (revivedCount > 0) {
        // Check if it's time to regenerate boundaries
        if (boundaryTimer.getElapsedTime().asSeconds() >= BOUNDARY_REGEN_INTERVAL) {
            setupBoundaries();  // This will generate new random positions
        }

        // Shooting logic
        if (shootTimer.getElapsedTime().asSeconds() >= shootInterval) {
            shoot();
            shootTimer.restart();
        }
    }

    // Update existing projectiles
    updateProjectiles(deltaTime, screenHeight);
    
    // Check invincibility timer
    if (isInvincible && invincibilityTimer.getElapsedTime().asSeconds() >= invincibilityDuration) {
        isInvincible = false;
    }
}

void Boss_3::draw(sf::RenderWindow& window) {
    // If in transition, only draw the transition slide
    if (isInTransition) {
        if (!transitionTextures.empty() && currentTransitionFrame < transitionTextures.size()) {
            window.draw(transitionSprite);
        }
        return;  // Don't draw anything else during transition
    }

    // Normal drawing code
    window.draw(sprite);
    window.draw(healthBarBackground);
    window.draw(healthBar);
    
    if (boundariesSet && revivedCount > 0) {
        window.draw(leftBoundary);
        window.draw(rightBoundary);
    }
    
    for (const auto& projectile : projectiles) {
        window.draw(projectile.bikeSprite);
    }
}

void Boss_3::takeDamage(int damage) {
    if (!isInvincible) {
        int actualDamage = 5;
        health -= actualDamage;
        
        if (health <= 0 && revivedCount == 0) {
            // First death - start story sequence
            std::cout << "Triggering transition animation..." << std::endl;
            startTransition();  // Start transition first
            health = maxHealth;  // Reset health
            currentHealth = maxHealth;
            isInvincible = true;
            // Don't increment revivedCount or setup boundaries yet - wait until transition ends
        } else if (health <= 0 && revivedCount > 0) {
            // Second death
            health = 0;
            currentHealth = 0.f;
            healthBar.setSize(Vector2f(40.f, 0.f));
        }
        
        // Update health bar
        currentHealth = health;
        float healthPercentage = currentHealth / maxHealth;
        float healthBarHeight = healthPercentage * 600.f;
        healthBarHeight = std::max(0.f, std::min(healthBarHeight, 600.f));
        healthBar.setSize(Vector2f(40.f, healthBarHeight));
        healthBar.setPosition(50.f, 100.f + (600.f - healthBarHeight));
        
        // Set brief invincibility after taking damage
        isInvincible = true;
        invincibilityTimer.restart();
        
        std::cout << "Boss took " << actualDamage << " damage. Remaining Health: " << health << std::endl;
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

void Boss_3::startTransition() {
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
    srand(time(0));  // 初始化隨機數生成器
    
    // 加載玩家材質
    Texture playerTexture;
    if (!playerTexture.loadFromFile("/Users/cpcap/GTA6/source/texture/character/player.png")) {
        cout << "Error loading player texture!" << endl;
        return -1;
    }

    // Load transitionsound
    sf::SoundBuffer transitionBuffer;
    if (!transitionBuffer.loadFromFile("/Users/cpcap/GTA6/source/assets_audio/transitionSound.wav")) // Ensure this path is correct
        return -1;
    sf::Sound transitionSound;
    transitionSound.setBuffer(transitionBuffer);

    // Load shootsound
    sf::SoundBuffer shootBuffer;
    if (!shootBuffer.loadFromFile("/Users/cpcap/GTA6/source/assets_audio/shootSound.wav")) // Ensure this path is correct
        return -1;
    sf::Sound shootSound;
    shootSound.setBuffer(shootBuffer);

    // Load hurtsound
    sf::SoundBuffer hurtBuffer;
    if (!hurtBuffer.loadFromFile("/Users/cpcap/GTA6/source/assets_audio/hurtSound.flac")) // Ensure this path is correct
        return -1;
    sf::Sound hurtSound;
    hurtSound.setBuffer(hurtBuffer);

    // Load slide textures (過廠動畫)
    std::vector<sf::Texture> slideTextures(3);
    if (!slideTextures[0].loadFromFile("/Users/cpcap/GTA6/source/texture/story/slide1.png") ||
        !slideTextures[1].loadFromFile("/Users/cpcap/GTA6/source/texture/story/slide2.png") ||
        !slideTextures[2].loadFromFile("/Users/cpcap/GTA6/source/texture/story/slide3.png")) {
        std::cerr << "Failed to load one or more slides!" << std::endl;
        return -1;
    }

    // Variables for transition logic
    size_t currentSlideIndex = 0;
    sf::Clock slideTimer;
    const float slideDuration = 2.0f; // Display each slide for 2 seconds
    bool transitionActive = true;    // Controls transition state

    // Create sprites for the slides
    std::vector<sf::Sprite> slides;
    for (const auto& texture : slideTextures) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        slides.push_back(sprite);
    }

    // 創建玩家精靈替代原的 CircleShape
    Sprite playerSprite(playerTexture);
    // 設置精原點為中心
    playerSprite.setOrigin(playerTexture.getSize().x / 2.f, playerTexture.getSize().y / 2.f);
    
    float x = BOUNDARY_LEFT + PLAY_AREA_WIDTH/2;  // 在遊戲區域中心
    float y = 730.f;  // 原始值 740.f，我們可以減小這個值來使角色往上移

    playerSprite.setPosition(x, y);
    
    // 在 main 函數中，修改玩家精靈的縮放比例
    float desiredWidth = 117.f;   // 期望的寬度
    float desiredHeight = 182.f;  // 期望的高度（可以調整這個值來改變高度）

    playerSprite.setScale(
        desiredWidth / playerTexture.getSize().x,
        desiredHeight / playerTexture.getSize().y
    );
    
    float moveSpeed = 0.3f;

    // 獲取玩家精靈的實際寬度（慮縮放後的大小）
    float playerWidth = playerSprite.getGlobalBounds().width;

    // 設置移動邊界，考慮玩家寬度
    float leftBound = BOUNDARY_LEFT;                         // 左邊界
    float rightBound = BOUNDARY_LEFT + PLAY_AREA_WIDTH - playerWidth;  // 右邊界減去玩家寬���

    // 子彈容
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

    // 人關變量
    std::vector<Enemy> enemies;
    Clock enemySpawnTimer;  // 用於計時生成敵人
    
    // 添加無敵時間計時器
    Clock invincibilityTimer;
    bool isInvincible = false;
    float invincibilityDuration = 1.0f;  // 1無敵時間

    // 在創建 Game 實例之前定義 killCount
    int killCount = 0;
    int gold = 0;  // 初始金幣

    // 修改載入字體的部分
    sf::Font font;
    if (!font.loadFromFile("/Users/cpcap/GTA6/source/assets_fonts/cour.ttf")) {  // macOS 系統字體路徑
        // 如果 Arial 載入失敗，嘗試載入系統默認字體
        if (!font.loadFromFile("/System/Library/Fonts/STHeiti Light.ttc")) {
            std::cout << "Error loading font!" << std::endl;
        }
    }

    // 修改數器文字設置
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
    goldText.setPosition(10.f, 40.f);  // 位置在擊殺數方
    goldText.setStyle(sf::Text::Bold);  // 添加粗體樣式

    // 建遊戲實實例
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

    // 添加子彈射計時器
    Clock shootTimer;
    const float shootCooldown = 0.5f;  // 射擊冷卻時間（秒）

    // 添加敵人生成計時器
    const float enemySpawnInterval = 0.3f;  // 更短的生成間隔
    const float ENEMY_SPAWN_CHANCE = 0.5f;  // 每次檢查時生成敵人的機率

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
    Clock autoShootTimer;  // 自動發射計時器
    const float autoShootInterval = 0.5f;  // 每0.5秒發射一次，你可以調整這個值

    sf::Clock clock;  // 添加時間來計算幀時間
    
    // 直接創建 Boss_3 實例
    std::unique_ptr<Boss_3> boss3 = std::make_unique<Boss_3>(
        "/Users/cpcap/GTA6/source/texture/character/uncleBoss.png",
        transitionSound,  // Pass the transition sound
        600.f,  // x 位置
        130.f   // y 位置改為 150.f
    );

    // 在 main 函數開始處
    const float SHOOT_COOLDOWN = 0.4f;  // 射擊冷卻時間
    sf::Clock shootCooldownTimer;       // 射擊冷卻計時器
    
    // Load background music
    sf::Music backgroundMusic;
    if (!backgroundMusic.openFromFile("/Users/cpcap/GTA6/source/assets_audio/backgroundMusic.wav")) {  // Adjust path as needed
        std::cout << "Error loading background music!" << std::endl;
        // Don't return -1 here, game can still run without music
    } else {
        backgroundMusic.setLoop(true);  // Make the music loop
        backgroundMusic.setVolume(50.f);  // Adjust volume (0-100)
        backgroundMusic.play();
    }



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
                    killCount++;  // 每按一次J加一個擊殺數
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
                    
                    // 如果血量歸零，觸發遊戲結束
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

        // 在遊戲循環中，修改碰撞檢測的部分
        if (!isInvincible) {
            auto enemyIt = game.getEnemies().begin();
            while (enemyIt != game.getEnemies().end()) {
                if (enemyIt->checkCollision(playerSprite)) {
                    // 扣血
                    currentHealth = std::max(0.f, currentHealth - 10.f);
                    float healthBarHeight = (currentHealth/maxHealth) * 200.f;
                    healthBar.setSize(Vector2f(20.f, healthBarHeight));
                    healthBar.setPosition(1150.f, 300.f + (200.f - healthBarHeight));
                    hurtSound.play();

                    // 設置無敵敵時間
                    isInvincible = true;
                    invincibilityTimer.restart();
                    
                    // 增加擊殺數和金幣
                    killCount++;
                    gold += 1000;  // 每擊敗一個敵人增加 1000 金
                    
                    // 立即更新 UI 文
                    killCountText.setString("Kills: " + std::to_string(killCount));
                    goldText.setString("Gold: " + std::to_string(gold));
                    
                    // 移除敵人
                    enemyIt = game.getEnemies().erase(enemyIt);
                    
                    // 檢查是否死亡
                    if (currentHealth <= 0) {
                        isGameOver = true;
                    }
                    
                    break;
                } else {
                    ++enemyIt;
                }
            }
        }

        // 修改血量檢測邏輯
        if (currentHealth <= 0) {
            isGameOver = true;
        }

        window.clear();

        // 修改遊戲狀態查的邏輯
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
                float newX = x - moveSpeed;
                if (boss3 && boss3->hasBoundaries()) {
                    // Only apply boss boundaries if they exist
                    newX = std::max(boss3->getLeftBoundaryX(), newX);
                } else {
                    // Otherwise use normal game boundaries
                    newX = std::max(BOUNDARY_LEFT, newX);
                }
                x = newX;
            }
            if (Keyboard::isKeyPressed(Keyboard::Right)) {
                float newX = x + moveSpeed;
                if (boss3 && boss3->hasBoundaries()) {
                    // Only apply boss boundaries if they exist
                    newX = std::min(boss3->getRightBoundaryX(), newX);
                } else {
                    // Otherwise use normal game boundaries
                    newX = std::min(BOUNDARY_RIGHT - playerWidth, newX);
                }
                x = newX;
            }
            // 添加上下移動
            if (Keyboard::isKeyPressed(Keyboard::Up)) {
                // 限制上邊界，確保家不會移出畫面
                y = std::max(playerSprite.getGlobalBounds().height/2.f, y - moveSpeed);
            }
            if (Keyboard::isKeyPressed(Keyboard::Down)) {
                // 限制下邊界，確保玩家不會移出畫面
                y = std::min(window.getSize().y - playerSprite.getGlobalBounds().height/2.f, y + moveSpeed);
            }

            playerSprite.setPosition(x, y);
            
            // 射擊控制
            if (Keyboard::isKeyPressed(Keyboard::Space) && 
                shootCooldownTimer.getElapsedTime().asSeconds() >= SHOOT_COOLDOWN) {
                // 從玩家中心位置發射子彈
                float bulletX = playerSprite.getPosition().x;
                float bulletY = playerSprite.getPosition().y - playerSprite.getGlobalBounds().height/2.f;
                
                game.addBullet(bulletX, bulletY);
                shootSound.play(); 
                shootCooldownTimer.restart();  // 重置冷卻計時器
            }

            // 修改敵人生成邏輯
            if (enemySpawnTimer.getElapsedTime().asSeconds() >= enemySpawnInterval) {
                // 只在 Boss 第一次死亡之前生成敵人
                if (!boss3 || !boss3->hasRevivedOnce()) {
                    // 使用隨機數決定是否生成敵人
                    float spawnRoll = static_cast<float>(rand()) / RAND_MAX;
                    
                    if (spawnRoll < ENEMY_SPAWN_CHANCE) {
                        // 隨機決定這次生的敵人數量（1-3個，平均約2個）
                        int enemyCount = 1 + (rand() % 3);
                        
                        const float ENEMY_BOUNDARY_LEFT = 250.f;
                        const float ENEMY_BOUNDARY_RIGHT = 950.f;
                        const float ENEMY_WIDTH = 30.f;
                        const float ENEMY_SPACING = 50.f;  // 敵人之間的最小間距
                        
                        for (int i = 0; i < enemyCount; ++i) {
                            // 生成隨位置
                            float randomX;
                            bool validPosition;
                            int maxAttempts = 10;  // 最大嘗試次數
                            
                            do {
                                validPosition = true;
                                randomX = ENEMY_BOUNDARY_LEFT + 
                                    (static_cast<float>(rand()) / RAND_MAX) * 
                                    (ENEMY_BOUNDARY_RIGHT - ENEMY_BOUNDARY_LEFT - ENEMY_WIDTH);
                                
                                // 檢查與現有人的距離
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

            // 修 Boss 死亡檢測
            if (boss3 && boss3->isDead()) {
                if (boss3->hasRevivedOnce()) {
                    // 第二次死亡時遊戲勝利
                    gameWon = true;
                }
                transitionSound.play();
            }

            // 更新遊戲邏輯
            game.updateBullets();
            game.updateEnemies();

            // 檢測玩家與敵人和Boss的碰撞
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

                // 如果發生任何碰撞，處理扣血和無敵時間
                if (collisionOccurred) {
                    // 扣血
                    currentHealth = std::max(0.f, currentHealth - 10.f);
                    float healthBarHeight = (currentHealth/maxHealth) * 200.f;
                    healthBar.setSize(Vector2f(20.f, healthBarHeight));
                    healthBar.setPosition(1150.f, 300.f + (200.f - healthBarHeight));
                    hurtSound.play();

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

            // ��新並繪製擊殺數
            killCountText.setString("Kills: " + std::to_string(killCount));
            goldText.setString("Gold: " + std::to_string(gold));
            window.draw(killCountText);
            window.draw(goldText);

            // 移除檢查擊殺數的條件，直接新和繪製 Boss
            if (boss3) {
                boss3->update(deltaTime, PLAY_AREA_WIDTH);
                boss3->draw(window);

                // 檢查子彈是否擊中 Boss
                auto& bullets = game.getBullets();
                auto bulletIt = bullets.begin();
                while (bulletIt != bullets.end()) {
                    if (boss3->getSprite().getGlobalBounds().intersects(bulletIt->shape.getGlobalBounds())) {
                        if (!boss3->isInvincibleState()) {  // 檢查Boss是否處於無敵狀態
                            boss3->takeDamage(10);  // Boss 受到 10 點傷害
                            bulletIt = bullets.erase(bulletIt);  // 移除擊中的子彈彈
                            
                            // 檢查 Boss 是否死亡
                            if (boss3->isDead()) {
                                boss3.reset();  // 移除 Boss
                                gameWon = true;  // 遊戲勝利
                                break;
                            }
                        } else {
                            ++bulletIt;  // 如果Boss處於無無敵狀態，子彈穿過
                        }
                    } else {
                        ++bulletIt;
                    }
                }
            }

            // Check for bike collisions with player
            if (boss3) {
                // First check if boss is in transition
                if (!boss3->isInTransitionState()) {
                    // Then check player invincibility
                    if (!isInvincible) {
                        for (const auto& projectile : boss3->getProjectiles()) {
                            if (projectile.bikeSprite.getGlobalBounds().intersects(playerSprite.getGlobalBounds())) {
                                // Reduce player health
                                currentHealth = std::max(0.f, currentHealth - 10.f);
                                float healthBarHeight = (currentHealth/maxHealth) * 200.f;
                                healthBar.setSize(Vector2f(20.f, healthBarHeight));
                                healthBar.setPosition(1150.f, 300.f + (200.f - healthBarHeight));
                                hurtSound.play();

                                // Set player invincibility
                                isInvincible = true;
                                invincibilityTimer.restart();
                                
                                if (currentHealth <= 0) {
                                    isGameOver = true;
                                }
                                break;
                            }
                        }
                    }
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