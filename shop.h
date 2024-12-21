#ifndef SHOP_H
#define SHOP_H

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "gamestate.h"

//定義商品結構
struct Product {
    sf::FloatRect bounds;      // 藥水在背景上的點擊區域
    std::string description;   // 藥水描述
    std::string name;         // 藥水名稱
    int price;               // 藥水價格
    sf::RectangleShape buyButton;  // 購買按鈕
    sf::Text buyButtonText;        // 按鈕文字
    
    Product(const sf::FloatRect& b, const std::string& desc, const std::string& n, int p)
        : bounds(b), description(desc), name(n), price(p) {
        buyButton.setSize(sf::Vector2f(80, 30));
        buyButton.setFillColor(sf::Color(50, 150, 50));
        buyButton.setPosition(
            bounds.left + (bounds.width - 80) / 2,
            bounds.top + bounds.height + 10
        );
    }
};

class Shop {
public:
    Shop() {
        initializeShop();
    }

    void run(sf::RenderWindow& window, GameState& currentState) {
        if (!isInitialized) {
            std::cerr << "Shop not properly initialized!" << std::endl;
            return;
        }

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
                
                handleEvents(event, window);
            }

            render(window);

            // 檢查是否需要退出商店
            if (shouldExit) {
                currentState = GameState::levels;
                shouldExit = false;
                return;
            }
        }
    }

private:
    bool isInitialized = false;
    bool shouldExit = false;
    sf::Texture shopTexture;
    sf::Sprite shopBackground;
    sf::Font font;
    std::vector<Product> products;
    bool showDescription = false;
    sf::RectangleShape descriptionBox;
    sf::Text descriptionText;
    sf::Text mousePositionText;
    bool showPurchaseMessage = false;
    float purchaseMessageTimer = 0.0f;
    const float MESSAGE_DISPLAY_TIME = 1.5f;  // 顯示1.5秒
    sf::Text purchaseText;
    sf::Clock clock;  // 用於計時

    void initializeShop() {
        // 加載商店背景圖片
        if (!shopTexture.loadFromFile("/Users/cpcap/GTA6/source/texture/background/shop.png")) {
            std::cerr << "無法加載商店背景圖片！" << std::endl;
            return;
        }

        // 設置背景
        shopBackground.setTexture(shopTexture);
        float scaleX = 1200.0f / shopTexture.getSize().x;
        float scaleY = 800.0f / shopTexture.getSize().y;
        shopBackground.setScale(scaleX, scaleY);

        // 加載字體
        if (!font.loadFromFile("/Library/Fonts/Arial Unicode.ttf")) {
            if (!font.loadFromFile("/System/Library/Fonts/STHeiti Light.ttc") &&
                !font.loadFromFile("/System/Library/Fonts/Hiragino Sans GB.ttc")) {
                std::cerr << "無法加載字體！" << std::endl;
                return;
            }
        }

        // 初始化商品
        products = {
            {sf::FloatRect(643, 50, 80, 80),
            "Red Potion\nReduce 30% Damage\nPrice: 10000 coins", 
            "Red Potion", 
            10000},
            
            // 黃色藥水 (右上)
            {sf::FloatRect(873, 50, 80, 80), 
            "Yellow Potion\nReflect 30% Damage\nPrice: 20000 coins", 
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

        // 初始化描述框
        descriptionBox.setSize(sf::Vector2f(300, 150));
        descriptionBox.setFillColor(sf::Color(0, 0, 0, 230));
        descriptionBox.setOutlineThickness(2);
        descriptionBox.setOutlineColor(sf::Color(255, 215, 0));

        // 初始化文字
        descriptionText.setFont(font);
        descriptionText.setCharacterSize(24);
        descriptionText.setFillColor(sf::Color::White);

        mousePositionText.setFont(font);
        mousePositionText.setCharacterSize(20);
        mousePositionText.setFillColor(sf::Color::White);
        mousePositionText.setPosition(10, 10);

        // 初始化購買提示文字
        purchaseText.setFont(font);
        purchaseText.setString("Purchased!");
        purchaseText.setCharacterSize(40);
        purchaseText.setFillColor(sf::Color::Green);
        // 將文字置中
        purchaseText.setPosition(
            (1200 - purchaseText.getGlobalBounds().width) / 2,
            (800 - purchaseText.getGlobalBounds().height) / 2
        );

        isInitialized = true;
    }

    void handleEvents(const sf::Event& event, sf::RenderWindow& window) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                shouldExit = true;
                return;
            }
        }

        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                // 檢查購買按鈕點擊
                for (const Product& product : products) {
                    if (product.buyButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        std::cout << "Purchased " << product.name << " for " << product.price << " coins!" << std::endl;
                        // 顯示購買提示
                        showPurchaseMessage = true;
                        purchaseMessageTimer = 0.0f;
                        clock.restart();  // 重置計時器
                        return;
                    }
                }

                // 處理描述框顯示
                handleDescriptionBox(mousePos, window);
            }
        }
    }

    void handleDescriptionBox(const sf::Vector2i& mousePos, sf::RenderWindow& window) {
        if (showDescription) {
            showDescription = false;
        } else {
            for (const Product& product : products) {
                if (product.bounds.contains(mousePos.x, mousePos.y)) {
                    showDescription = true;
                    descriptionText.setString(product.description);

                    float boxX = mousePos.x;
                    float boxY = mousePos.y + 20;

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

    void render(sf::RenderWindow& window) {
        // 更新購買提示計時器
        if (showPurchaseMessage) {
            purchaseMessageTimer += clock.restart().asSeconds();
            if (purchaseMessageTimer >= MESSAGE_DISPLAY_TIME) {
                showPurchaseMessage = false;
            }
        } else {
            clock.restart();
        }

        window.clear();
        window.draw(shopBackground);

        // 繪製所有按鈕
        for (Product& product : products) {
            product.buyButtonText.setFont(font);
            product.buyButtonText.setString("Buy");
            product.buyButtonText.setCharacterSize(16);
            product.buyButtonText.setFillColor(sf::Color::White);
            
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

        // 如果需要顯示購買提示，則繪製它
        if (showPurchaseMessage) {
            // 創建一個半透明的黑色背景
            sf::RectangleShape messageBackground;
            messageBackground.setSize(sf::Vector2f(300, 100));
            messageBackground.setFillColor(sf::Color(0, 0, 0, 180));
            messageBackground.setPosition(
                (1200 - messageBackground.getSize().x) / 2,
                (800 - messageBackground.getSize().y) / 2
            );
            
            // 確保文字在背景中央
            purchaseText.setPosition(
                messageBackground.getPosition().x + (messageBackground.getSize().x - purchaseText.getGlobalBounds().width) / 2,
                messageBackground.getPosition().y + (messageBackground.getSize().y - purchaseText.getGlobalBounds().height) / 2
            );

            window.draw(messageBackground);
            window.draw(purchaseText);
        }

        window.draw(mousePositionText);
        window.display();
    }
};

#endif // SHOP_H 