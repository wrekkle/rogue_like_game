#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

// 地图尺寸
const int MAP_WIDTH = 800;
const int MAP_HEIGHT = 800;
const int MAX_LEVEL = 3;

// 碰撞检测函数
bool isCollision(const sf::RectangleShape& rect1, const sf::RectangleShape& rect2) {
    return rect1.getGlobalBounds().intersects(rect2.getGlobalBounds());
}

// 地图类
class Map {
public:
    Map(const std::string& relativePath) {
        std::string fullPath = "resources/maps/" + relativePath;
        std::cout << "Loading map: " << fullPath << std::endl;

        if (!texture.loadFromFile(fullPath)) {
            std::cerr << "ERROR: Failed to load map texture!" << std::endl;
            // 创建一个默认的灰色背景
            texture.create(MAP_WIDTH, MAP_HEIGHT);
            sf::Image blankImage;
            blankImage.create(MAP_WIDTH, MAP_HEIGHT, sf::Color(100, 100, 100));
            texture.update(blankImage);
        }

        sprite.setTexture(texture);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

private:
    sf::Texture texture;
    sf::Sprite sprite;
};

// 玩家类
class Player {
public:
    Player() {
        shape.setSize(sf::Vector2f(50, 50));
        shape.setFillColor(sf::Color::Blue);
        shape.setPosition(375, 375);
        health = 5;
        invincibilityFrames = 0;
        shootCooldown = 0;
    }

    void move(float dx, float dy) {
        sf::Vector2f newPos = shape.getPosition() + sf::Vector2f(dx, dy);
        if (newPos.x >= 0 && newPos.x + shape.getSize().x <= static_cast<float>(MAP_WIDTH)) {
            shape.move(dx, 0);
        }
        if (newPos.y >= 0 && newPos.y + shape.getSize().y <= static_cast<float>(MAP_HEIGHT)) {
            shape.move(0, dy);
        }
    }

    sf::RectangleShape getShape() const {
        return shape;
    }

    int getHealth() const {
        return health;
    }

    void reduceHealth() {
        if (health > 0 && invincibilityFrames == 0) {
            health--;
            invincibilityFrames = 30;
        }
    }

    void updateInvincibility() {
        if (invincibilityFrames > 0) {
            invincibilityFrames--;
        }
    }

    void reset() {
        shape.setPosition(375, 375);
        health = 5;
        invincibilityFrames = 0;
    }

    bool canShoot() const {
        return shootCooldown == 0;
    }

    void setShootCooldown() {
        shootCooldown = 15;
    }

    void updateShootCooldown() {
        if (shootCooldown > 0) {
            shootCooldown--;
        }
    }

private:
    sf::RectangleShape shape;
    int health;
    int invincibilityFrames;
    int shootCooldown;
};

// 近战怪物基类
class MeleeMonster {
public:
    MeleeMonster() {
        shape.setSize(sf::Vector2f(30, 30));
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(rand() % (MAP_WIDTH - 30), rand() % (MAP_HEIGHT - 30));
    }

    virtual void moveTowards(const sf::Vector2f& target) {
        sf::Vector2f direction = target - shape.getPosition();
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0) {
            direction /= length;
            sf::Vector2f newPos = shape.getPosition() + direction * 1.0f;
            if (newPos.x >= 0 && newPos.x + shape.getSize().x <= static_cast<float>(MAP_WIDTH)) {
                shape.move(direction.x * 1.0f, 0);
            }
            if (newPos.y >= 0 && newPos.y + shape.getSize().y <= static_cast<float>(MAP_HEIGHT)) {
                shape.move(0, direction.y * 1.0f);
            }
        }
    }

    sf::RectangleShape getShape() const {
        return shape;
    }

    bool isHit(const sf::RectangleShape& bullet) const {
        return isCollision(shape, bullet);
    }

protected:
    sf::RectangleShape shape;
};

// 蓝色近战怪物（具有传送和变大能力）
class BlueMeleeMonster : public MeleeMonster {
public:
    BlueMeleeMonster() : MeleeMonster() {
        shape.setFillColor(sf::Color::Blue);
        teleportCooldown = 0;
        enlargeCooldown = 0;
        isTeleporting = false;
        teleportTimer = 0;
        originalSize = shape.getSize();
        isEnlarged = false;
        enlargeTimer = 0;
    }

    void moveTowards(const sf::Vector2f& target) override {
        if (isTeleporting) {
            teleportTimer++;
            if (teleportTimer < 90) {
                float scaleFactor = 1 - (static_cast<float>(teleportTimer) / 90);
                shape.setSize(sf::Vector2f(originalSize.x * scaleFactor, originalSize.y * scaleFactor));
            }
            else {
                sf::Vector2f newPos = target;
                newPos.x -= shape.getSize().x / 2;
                newPos.y -= shape.getSize().y / 2;
                shape.setPosition(newPos);
                shape.setSize(originalSize);
                isTeleporting = false;
                teleportTimer = 0;
                teleportCooldown = 600;
            }
            return;
        }

        if (teleportCooldown > 0) {
            teleportCooldown--;
        }
        else if (rand() % 100 == 0) {
            isTeleporting = true;
            teleportTimer = 0;
        }

        if (enlargeCooldown > 0) {
            enlargeCooldown--;
        }
        else if (rand() % 100 == 0) {
            shape.setSize(sf::Vector2f(60, 60));
            enlargeCooldown = 600;
            isEnlarged = true;
            enlargeTimer = 0;
        }

        if (isEnlarged) {
            enlargeTimer++;
            if (enlargeTimer >= 180) {
                shape.setSize(originalSize);
                isEnlarged = false;
                enlargeTimer = 0;
            }
        }

        if (!isTeleporting) {
            MeleeMonster::moveTowards(target);
        }
    }

private:
    int teleportCooldown;
    int enlargeCooldown;
    bool isTeleporting;
    int teleportTimer;
    sf::Vector2f originalSize;
    bool isEnlarged;
    int enlargeTimer;
};

// 红色近战怪物（具有变大能力）
class RedMeleeMonster : public MeleeMonster {
public:
    RedMeleeMonster() : MeleeMonster() {
        shape.setFillColor(sf::Color::Red);
        enlargeCooldown = 0;
        isEnlarged = false;
        enlargeTimer = 0;
        originalSize = shape.getSize();
    }

    void moveTowards(const sf::Vector2f& target) override {
        if (enlargeCooldown > 0) {
            enlargeCooldown--;
        }
        else if (rand() % 100 == 0) {
            shape.setSize(sf::Vector2f(60, 60));
            enlargeCooldown = 600;
            isEnlarged = true;
            enlargeTimer = 0;
        }

        if (isEnlarged) {
            enlargeTimer++;
            if (enlargeTimer >= 180) {
                shape.setSize(originalSize);
                isEnlarged = false;
                enlargeTimer = 0;
            }
        }

        MeleeMonster::moveTowards(target);
    }

private:
    int enlargeCooldown;
    bool isEnlarged;
    int enlargeTimer;
    sf::Vector2f originalSize;
};

// 黄色近战怪物
class YellowMeleeMonster : public MeleeMonster {
public:
    YellowMeleeMonster() : MeleeMonster() {
        shape.setFillColor(sf::Color::Yellow);
    }

    void moveTowards(const sf::Vector2f& target) override {
        MeleeMonster::moveTowards(target);
    }
};

// 子弹类
class Bullet {
public:
    Bullet(const sf::Vector2f& startPos, const sf::Vector2f& target, bool isPlayerBullet = false) {
        shape.setSize(sf::Vector2f(5, 5));
        shape.setFillColor(isPlayerBullet ? sf::Color::Cyan : sf::Color::Yellow);
        shape.setPosition(startPos);

        sf::Vector2f direction = target - startPos;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0) {
            direction /= length;
            velocity = direction * 5.0f;
        }
        this->isPlayerBullet = isPlayerBullet;
    }

    void move() {
        shape.move(velocity);
    }

    sf::RectangleShape getShape() const {
        return shape;
    }

    bool isFromPlayer() const {
        return isPlayerBullet;
    }

private:
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    bool isPlayerBullet;
};

// 远程怪物类
class RangedMonster {
public:
    RangedMonster() {
        shape.setSize(sf::Vector2f(30, 30));
        shape.setFillColor(sf::Color::Magenta);
        shape.setPosition(rand() % (MAP_WIDTH - 30), rand() % (MAP_HEIGHT - 30));
        shootTimer = 0;
    }

    void moveTowards(const sf::Vector2f& target) {
        sf::Vector2f direction = target - shape.getPosition();
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0) {
            direction /= length;
            sf::Vector2f newPos = shape.getPosition() + direction * 0.8f;
            if (newPos.x >= 0 && newPos.x + shape.getSize().x <= static_cast<float>(MAP_WIDTH)) {
                shape.move(direction.x * 0.8f, 0);
            }
            if (newPos.y >= 0 && newPos.y + shape.getSize().y <= static_cast<float>(MAP_HEIGHT)) {
                shape.move(0, direction.y * 0.8f);
            }
        }
    }

    void shoot(const sf::Vector2f& target, std::vector<Bullet>& bullets) {
        shootTimer++;
        if (shootTimer >= 60) {
            bullets.emplace_back(shape.getPosition(), target);
            shootTimer = 0;
        }
    }

    sf::RectangleShape getShape() const {
        return shape;
    }

    bool isHit(const sf::RectangleShape& bullet) const {
        return isCollision(shape, bullet);
    }

private:
    sf::RectangleShape shape;
    int shootTimer;
};

// 重新开始游戏
void restartGame(Player& player, std::vector<BlueMeleeMonster>& blueMonsters,
    std::vector<RedMeleeMonster>& redMonsters, std::vector<YellowMeleeMonster>& yellowMonsters,
    std::vector<RangedMonster>& rangedMonsters, int& score, int& currentLevel) {
    player.reset();
    blueMonsters.clear();
    redMonsters.clear();
    yellowMonsters.clear();
    rangedMonsters.clear();
    score = 0;
    currentLevel = 1;

    // 重新生成初始怪物
    for (int i = 0; i < 1; ++i) {
        blueMonsters.emplace_back();
    }
    for (int i = 0; i < 1; ++i) {
        redMonsters.emplace_back();
    }
    for (int i = 0; i < 1; ++i) {
        yellowMonsters.emplace_back();
    }
    for (int i = 0; i < 2; ++i) {
        rangedMonsters.emplace_back();
    }
}

// 进入下一关
void nextLevel(Player& player, std::vector<BlueMeleeMonster>& blueMonsters,
    std::vector<RedMeleeMonster>& redMonsters, std::vector<YellowMeleeMonster>& yellowMonsters,
    std::vector<RangedMonster>& rangedMonsters, int& score, int& currentLevel) {
    player.reset();
    blueMonsters.clear();
    redMonsters.clear();
    yellowMonsters.clear();
    rangedMonsters.clear();
    score = 0;

    // 检查是否是最后一关
    if (currentLevel < MAX_LEVEL) {
        currentLevel++;
    }

    // 根据当前关卡增加怪物数量
    int baseCount = 1 + currentLevel;
    for (int i = 0; i < baseCount; ++i) {
        blueMonsters.emplace_back();
    }
    for (int i = 0; i < baseCount; ++i) {
        redMonsters.emplace_back();
    }
    for (int i = 0; i < baseCount; ++i) {
        yellowMonsters.emplace_back();
    }
    for (int i = 0; i < baseCount + 1; ++i) {
        rangedMonsters.emplace_back();
    }
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    sf::RenderWindow window(sf::VideoMode(MAP_WIDTH, MAP_HEIGHT), "2D Game");
    window.setFramerateLimit(60);

    // 初始化地图
    std::vector<Map> maps;
    maps.emplace_back("desert-map_1.png");  // 第一关：沙漠
    maps.emplace_back("forest-map_1.png");  // 第二关：森林
    maps.emplace_back("hell-map_1.png");    // 第三关：地狱

    // 游戏对象
    Player player;
    std::vector<BlueMeleeMonster> blueMonsters;
    std::vector<RedMeleeMonster> redMonsters;
    std::vector<YellowMeleeMonster> yellowMonsters;
    std::vector<RangedMonster> rangedMonsters;
    std::vector<Bullet> bullets;
    int score = 0;
    int currentLevel = 1;

    // 初始化怪物
    for (int i = 0; i < 1; ++i) {
        blueMonsters.emplace_back();
    }
    for (int i = 0; i < 1; ++i) {
        redMonsters.emplace_back();
    }
    for (int i = 0; i < 1; ++i) {
        yellowMonsters.emplace_back();
    }
    for (int i = 0; i < 2; ++i) {
        rangedMonsters.emplace_back();
    }

    // 加载字体
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Could not load font!" << std::endl;
        return 1;
    }

    // UI元素
    sf::Text healthText;
    healthText.setFont(font);
    healthText.setCharacterSize(24);
    healthText.setFillColor(sf::Color::White);
    healthText.setPosition(10, 10);

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10, 40);

    sf::Text levelText;
    levelText.setFont(font);
    levelText.setCharacterSize(24);
    levelText.setFillColor(sf::Color::White);
    levelText.setPosition(10, 70);

    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(48);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setString("Game Over!");
    gameOverText.setPosition(MAP_WIDTH / 2 - gameOverText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 - 50);

    sf::RectangleShape restartButton(sf::Vector2f(200, 50));
    restartButton.setFillColor(sf::Color::Green);
    restartButton.setPosition(MAP_WIDTH / 2 - 100, MAP_HEIGHT / 2 + 20);

    sf::Text restartText;
    restartText.setFont(font);
    restartText.setCharacterSize(24);
    restartText.setFillColor(sf::Color::White);
    restartText.setString("Restart");
    restartText.setPosition(MAP_WIDTH / 2 - restartText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 35);

    sf::Text nextLevelText;
    nextLevelText.setFont(font);
    nextLevelText.setCharacterSize(48);
    nextLevelText.setFillColor(sf::Color::Green);
    nextLevelText.setString("Level Complete!");
    nextLevelText.setPosition(MAP_WIDTH / 2 - nextLevelText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 - 50);

    sf::RectangleShape nextLevelButton(sf::Vector2f(200, 50));
    nextLevelButton.setFillColor(sf::Color::Green);
    nextLevelButton.setPosition(MAP_WIDTH / 2 - 100, MAP_HEIGHT / 2 + 20);

    sf::Text nextLevelConfirmText;
    nextLevelConfirmText.setFont(font);
    nextLevelConfirmText.setCharacterSize(24);
    nextLevelConfirmText.setFillColor(sf::Color::White);
    nextLevelConfirmText.setString("Continue");
    nextLevelConfirmText.setPosition(MAP_WIDTH / 2 - nextLevelConfirmText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 35);

    sf::Text victoryText;
    victoryText.setFont(font);
    victoryText.setCharacterSize(48);
    victoryText.setFillColor(sf::Color::Green);
    victoryText.setString("Victory!");
    victoryText.setPosition(MAP_WIDTH / 2 - victoryText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 - 50);

    sf::RectangleShape victoryRestartButton(sf::Vector2f(200, 50));
    victoryRestartButton.setFillColor(sf::Color::Green);
    victoryRestartButton.setPosition(MAP_WIDTH / 2 - 100, MAP_HEIGHT / 2 + 20);

    sf::Text victoryRestartText;
    victoryRestartText.setFont(font);
    victoryRestartText.setCharacterSize(24);
    victoryRestartText.setFillColor(sf::Color::White);
    victoryRestartText.setString("Restart");
    victoryRestartText.setPosition(MAP_WIDTH / 2 - victoryRestartText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 35);

    // 游戏状态
    bool gameOver = false;
    bool nextLevelAvailable = false;
    bool gameWon = false;
    int survivalTimer = 0;

    // 游戏主循环
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (!gameOver && !nextLevelAvailable && !gameWon) {
                        // 玩家射击
                        if (player.canShoot()) {
                            sf::Vector2f playerCenter = player.getShape().getPosition() + sf::Vector2f(25, 25);
                            bullets.emplace_back(playerCenter, mousePos, true);
                            player.setShootCooldown();
                        }
                    }
                    else if (gameOver && restartButton.getGlobalBounds().contains(mousePos)) {
                        restartGame(player, blueMonsters, redMonsters, yellowMonsters, rangedMonsters, score, currentLevel);
                        bullets.clear();
                        gameOver = false;
                        gameWon = false;
                        survivalTimer = 0;
                    }
                    else if (nextLevelAvailable && nextLevelButton.getGlobalBounds().contains(mousePos)) {
                        nextLevel(player, blueMonsters, redMonsters, yellowMonsters, rangedMonsters, score, currentLevel);
                        bullets.clear();
                        nextLevelAvailable = false;
                        survivalTimer = 0;
                    }
                    else if (gameWon && victoryRestartButton.getGlobalBounds().contains(mousePos)) {
                        restartGame(player, blueMonsters, redMonsters, yellowMonsters, rangedMonsters, score, currentLevel);
                        bullets.clear();
                        gameWon = false;
                        survivalTimer = 0;
                    }
                }
            }
        }

        if (!gameOver && !nextLevelAvailable && !gameWon) {
            // 玩家移动
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                player.move(-5, 0);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                player.move(5, 0);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                player.move(0, -5);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                player.move(0, 5);

            player.updateInvincibility();
            player.updateShootCooldown();

            // 怪物移动
            for (auto& blueMonster : blueMonsters) {
                blueMonster.moveTowards(player.getShape().getPosition());
            }

            for (auto& redMonster : redMonsters) {
                redMonster.moveTowards(player.getShape().getPosition());
            }

            for (auto& yellowMonster : yellowMonsters) {
                yellowMonster.moveTowards(player.getShape().getPosition());
            }

            for (auto& rangedMonster : rangedMonsters) {
                rangedMonster.moveTowards(player.getShape().getPosition());
                rangedMonster.shoot(player.getShape().getPosition(), bullets);
            }

            // 子弹移动和碰撞检测
            for (auto it = bullets.begin(); it != bullets.end();) {
                it->move();
                bool bulletHit = false;

                // 玩家子弹击中怪物
                if (it->isFromPlayer()) {
                    // 检查蓝色怪物
                    for (auto blueIt = blueMonsters.begin(); blueIt != blueMonsters.end();) {
                        if (blueIt->isHit(it->getShape())) {
                            blueIt = blueMonsters.erase(blueIt);
                            bulletHit = true;
                            score++;
                            break;
                        }
                        else {
                            ++blueIt;
                        }
                    }

                    // 检查红色怪物
                    for (auto redIt = redMonsters.begin(); redIt != redMonsters.end();) {
                        if (redIt->isHit(it->getShape())) {
                            redIt = redMonsters.erase(redIt);
                            bulletHit = true;
                            score++;
                            break;
                        }
                        else {
                            ++redIt;
                        }
                    }

                    // 检查黄色怪物
                    for (auto yellowIt = yellowMonsters.begin(); yellowIt != yellowMonsters.end();) {
                        if (yellowIt->isHit(it->getShape())) {
                            yellowIt = yellowMonsters.erase(yellowIt);
                            bulletHit = true;
                            score++;
                            break;
                        }
                        else {
                            ++yellowIt;
                        }
                    }

                    // 检查远程怪物
                    for (auto rangedIt = rangedMonsters.begin(); rangedIt != rangedMonsters.end();) {
                        if (rangedIt->isHit(it->getShape())) {
                            rangedIt = rangedMonsters.erase(rangedIt);
                            bulletHit = true;
                            score++;
                            break;
                        }
                        else {
                            ++rangedIt;
                        }
                    }
                }
                // 怪物子弹击中玩家
                else if (isCollision(player.getShape(), it->getShape())) {
                    player.reduceHealth();
                    bulletHit = true;
                }

                // 移除出界或击中目标的子弹
                if (bulletHit ||
                    it->getShape().getPosition().x < 0 ||
                    it->getShape().getPosition().x > static_cast<float>(MAP_WIDTH) ||
                    it->getShape().getPosition().y < 0 ||
                    it->getShape().getPosition().y > static_cast<float>(MAP_HEIGHT)) {
                    it = bullets.erase(it);
                }
                else {
                    ++it;
                }
            }

            // 检查玩家与怪物的碰撞
            for (const auto& blueMonster : blueMonsters) {
                if (isCollision(player.getShape(), blueMonster.getShape())) {
                    player.reduceHealth();
                }
            }

            for (const auto& redMonster : redMonsters) {
                if (isCollision(player.getShape(), redMonster.getShape())) {
                    player.reduceHealth();
                }
            }

            for (const auto& yellowMonster : yellowMonsters) {
                if (isCollision(player.getShape(), yellowMonster.getShape())) {
                    player.reduceHealth();
                }
            }

            for (const auto& rangedMonster : rangedMonsters) {
                if (isCollision(player.getShape(), rangedMonster.getShape())) {
                    player.reduceHealth();
                }
            }

            // 更新UI文本
            healthText.setString("Health: " + std::to_string(player.getHealth()));
            scoreText.setString("Score: " + std::to_string(score));
            levelText.setString("Level: " + std::to_string(currentLevel) + "/" + std::to_string(MAX_LEVEL));

            // 检查游戏状态
            if (player.getHealth() <= 0) {
                gameOver = true;
            }
            else if (score >= 3) { // 每关需要击败3个怪物
                if (currentLevel >= MAX_LEVEL) {
                    gameWon = true; // 最后一关完成，游戏胜利
                }
                else {
                    nextLevelAvailable = true; // 还有下一关
                }
            }
        }

        // 渲染
        window.clear();

        if (!gameOver && !nextLevelAvailable && !gameWon) {
            // 绘制当前关卡的地图
            if (currentLevel > 0 && currentLevel <= maps.size()) {
                maps[currentLevel - 1].draw(window);
            }

            // 绘制玩家
            window.draw(player.getShape());

            // 绘制怪物
            for (const auto& blueMonster : blueMonsters) {
                window.draw(blueMonster.getShape());
            }

            for (const auto& redMonster : redMonsters) {
                window.draw(redMonster.getShape());
            }

            for (const auto& yellowMonster : yellowMonsters) {
                window.draw(yellowMonster.getShape());
            }

            for (const auto& rangedMonster : rangedMonsters) {
                window.draw(rangedMonster.getShape());
            }

            // 绘制子弹
            for (const auto& bullet : bullets) {
                window.draw(bullet.getShape());
            }

            // 绘制UI
            window.draw(healthText);
            window.draw(scoreText);
            window.draw(levelText);
        }
        else if (nextLevelAvailable) {
            // 绘制关卡完成界面
            window.draw(nextLevelText);
            window.draw(nextLevelButton);
            window.draw(nextLevelConfirmText);
        }
        else if (gameWon) {
            // 绘制胜利界面
            window.draw(victoryText);
            window.draw(victoryRestartButton);
            window.draw(victoryRestartText);
        }
        else {
            // 绘制游戏结束界面
            window.draw(gameOverText);
            window.draw(restartButton);
            window.draw(restartText);
        }

        window.display();
    }

    return 0;
}