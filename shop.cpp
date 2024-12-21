#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <string>

//定義商品結構
struct Product {
    sf::FloatRect bounds;      // 藥水在背景上的點擊區域
    std::string description;   // 藥水描述
    std::string name;         // 藥水名稱
    int price;               // 藥水價格
    sf::RectangleShape buyButton;  // 購買按鈕
    sf::Text buyButtonText;        // 按鈕文字
    
    // 添加構造函數
    Product(const sf::FloatRect& b, const std::string& desc, const std::string& n, int p)
        : bounds(b), description(desc), name(n), price(p) {
        // 初始化購買按鈕
        buyButton.setSize(sf::Vector2f(80, 30));
        buyButton.setFillColor(sf::Color(50, 150, 50));  // 綠色按鈕
        
        // 設置按鈕位置在藥水�域的正下方
        buyButton.setPosition(
            bounds.left + (bounds.width - 80) / 2,  // 水平居中
            bounds.top + bounds.height + 10         // 在藥水下方留出10像素間距
        );
    }
};

int main() {
    // 創建窗口
    sf::RenderWindow window(sf::VideoMode(1200, 800), "Shop");
    window.setFramerateLimit(60);

    // 加載商店背景圖片
    sf::Texture shopTexture;
    if (!shopTexture.loadFromFile("/Users/cpcap/GTA6/texture/background/shop.png")) {
        std::cerr << "無法加載商店背景圖片！" << std::endl;
        return -1;
    }

    // 創建背景精靈
    sf::Sprite shopBackground(shopTexture);
    
    // 調整背景圖片大小以適應窗口
    float scaleX = 1200.0f / shopTexture.getSize().x;
    float scaleY = 800.0f / shopTexture.getSize().y;
    shopBackground.setScale(scaleX, scaleY);

    // 加載字體
    sf::Font font;
    if (!font.loadFromFile("/Library/Fonts/Arial Unicode.ttf")) {
        // 如果第一個字體加載失敗，嘗試其他備選字體
        if (!font.loadFromFile("/System/Library/Fonts/STHeiti Light.ttc") &&
            !font.loadFromFile("/System/Library/Fonts/Hiragino Sans GB.ttc")) {
            std::cerr << "無法加載字體！" << std::endl;
            return -1;
        }
    }

    // 設置文字編碼為 UTF-8
    setlocale(LC_ALL, "zh_CN.UTF-8");

    // 創建四個藥水的擊區域和描述（位置需要根據實際背景圖調整）
    std::vector<Product> products = {
        // 紅色藥水 (左上)
        {sf::FloatRect(643, 50, 80, 80),
         "Red Potion\nReduce 30% Damage\nPrice: 10000 coins", 
         "Red Potion", 
         10000},
        
        // 黃色藥水 (右上)
        {sf::FloatRect(873, 50, 80, 80), 
         "Blue Potion\nReflect 30% Damage\nPrice: 20000 coins", 
         "Blue Potion",
         20000},
        
        // 綠色藥水 (左下)
        {sf::FloatRect(706, 235, 80, 80), 
         "Green Potion\nDamage +30%\nPrice: 30000 coins", 
         "Green Potion", 
         30000},
        
        // 紫色藥水 (右下)
        {sf::FloatRect(960, 235, 80, 80), 
         "Purple Potion\nFire rate +50%\nPrice: 40000 coins", 
         "Purple Potion", 
         40000}
    };

    // 創建商品信息視窗
    bool showDescription = false;
    sf::RectangleShape descriptionBox;
    descriptionBox.setSize(sf::Vector2f(300, 150));
    descriptionBox.setFillColor(sf::Color(0, 0, 0, 230));  // 半透明黑色背景
    descriptionBox.setOutlineThickness(2);
    descriptionBox.setOutlineColor(sf::Color(255, 215, 0));  // 金色邊框

    // 創建商品信息文字
    sf::Text descriptionText;
    descriptionText.setFont(font);
    descriptionText.setCharacterSize(24);
    descriptionText.setFillColor(sf::Color::White);

    // 在 main() 函數中添加滑鼠座標顯示文字
    sf::Text mousePositionText;
    mousePositionText.setFont(font);
    mousePositionText.setCharacterSize(20);
    mousePositionText.setFillColor(sf::Color::White);
    mousePositionText.setPosition(10, 10);

    // 主循環
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // 處理滑鼠點擊
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                    // 檢查是否點擊了任何購買按鈕
                    for (const Product& product : products) {
                        if (product.buyButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                            // 這裡添加購買邏輯
                            std::cout << "Purchased " << product.name << " for " << product.price << " coins!" << std::endl;
                        }
                    }

                    // 原有的藥水描述框點擊檢查
                    if (showDescription) {
                        showDescription = false;
                    } else {
                        for (Product& product : products) {
                            if (product.bounds.contains(mousePos.x, mousePos.y)) {
                                showDescription = true;
                                descriptionText.setString(product.description);

                                float boxX = mousePos.x;
                                float boxY = mousePos.y + 20;

                                // 確保描述框不會超出視窗邊界
                                if (boxX + descriptionBox.getSize().x > window.getSize().x) {
                                    boxX = window.getSize().x - descriptionBox.getSize().x - 10;
                                }
                                if (boxY + descriptionBox.getSize().y > window.getSize().y) {
                                    boxY = window.getSize().y - descriptionBox.getSize().y - 10;
                                }

                                descriptionBox.setPosition(boxX, boxY);
                                descriptionText.setPosition(boxX + 10, boxY + 10);

                                break;
                            }
                        }
                    }
                }
            }
        }

        // 更新滑鼠座標顯示
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        mousePositionText.setString("X: " + std::to_string(mousePos.x) + 
                                  " Y: " + std::to_string(mousePos.y));

        window.clear();
        window.draw(shopBackground);

        // 繪製所有按鈕
        for (Product& product : products) {
            // 更新按鈕文字
            product.buyButtonText.setFont(font);
            product.buyButtonText.setString("Buy");
            product.buyButtonText.setCharacterSize(16);
            product.buyButtonText.setFillColor(sf::Color::White);
            
            // 置中按鈕文字
            float textX = product.buyButton.getPosition().x + 
                (product.buyButton.getSize().x - product.buyButtonText.getGlobalBounds().width) / 2;
            float textY = product.buyButton.getPosition().y + 
                (product.buyButton.getSize().y - product.buyButtonText.getGlobalBounds().height) / 2;
            product.buyButtonText.setPosition(textX, textY);

            window.draw(product.buyButton);
            window.draw(product.buyButtonText);
        }

        if (showDescription) {
            window.draw(descriptionBox);
            window.draw(descriptionText);
        }

        window.display();
    }

    return 0;
}
