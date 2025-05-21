#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

// 地图尺寸 (仍然用于物体边界检测等)
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
    // 构造函数，接收地图图片文件的相对路径
    Map(const std::string& imagePath) {
        std::string fullPath = "resources/maps/" + imagePath; // 假设图片在 "resources/maps/" 文件夹下
        std::cout << "Loading map: " << fullPath << std::endl;

        if (!texture.loadFromFile(fullPath)) {
            std::cerr << "ERROR: Failed to load map texture from: " << fullPath << std::endl;
            // 如果加载失败，创建一个纯黑的默认背景
            texture.create(MAP_WIDTH, MAP_HEIGHT);
            sf::Image blankImage;
            blankImage.create(MAP_WIDTH, MAP_HEIGHT, sf::Color::Black); // 纯黑背景作为备用
            texture.update(blankImage);
            std::cerr << "Created a default black background as fallback." << std::endl;
        }
        sprite.setTexture(texture);
    }

    // 绘制地图到窗口
    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

private:
    sf::Texture texture;
    sf::Sprite sprite;
};

// 玩家基类
class Player {
public:
    Player() {
        shape.setSize(sf::Vector2f(50, 50));
        shape.setPosition(375, 375);
        health = 5;
        invincibilityFrames = 0;
        shootCooldown = 0;
    }

    virtual ~Player() = default; // 虚析构函数

    virtual void move(float dx, float dy) {
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
            invincibilityFrames = 30; // 30帧无敌
        }
    }

    void updateInvincibility() {
        if (invincibilityFrames > 0) {
            invincibilityFrames--;
        }
    }

    virtual void reset() {
        shape.setPosition(375, 375);
        health = 5;
        invincibilityFrames = 0;
    }

    virtual bool canShoot() const {
        return shootCooldown == 0;
    }

    virtual void setShootCooldown() {
        shootCooldown = 15; // 15帧冷却
    }

    virtual void updateShootCooldown() {
        if (shootCooldown > 0) {
            shootCooldown--;
        }
    }

protected:
    sf::RectangleShape shape;
    int health;
    int invincibilityFrames;
    int shootCooldown;
};

// 近战玩家类
class MeleePlayer : public Player {
public:
    MeleePlayer() : Player() {
        shape.setFillColor(sf::Color::Blue); // 近战玩家为蓝色
        // 近战玩家特有属性
        attackRange = 60.0f;
    }

    // 近战玩家特有方法
    float getAttackRange() const {
        return attackRange;
    }

    void setShootCooldown() override {
        shootCooldown = 10; // 近战玩家攻击冷却更短
    }

private:
    float attackRange; // 近战攻击范围
};

// 远程玩家类
class RangedPlayer : public Player {
public:
    RangedPlayer() : Player() {
        shape.setFillColor(sf::Color::Green); // 远程玩家为绿色
        // 远程玩家特有属性
        bulletSpeed = 7.0f;
    }

    // 远程玩家特有方法
    float getBulletSpeed() const {
        return bulletSpeed;
    }

    void setShootCooldown() override {
        shootCooldown = 20; // 远程玩家攻击冷却更长
    }

private:
    float bulletSpeed; // 子弹速度
};

// 近战怪物基类 (无改动)
class MeleeMonster {
public:
    MeleeMonster() {
        shape.setSize(sf::Vector2f(30, 30));
        shape.setFillColor(sf::Color::Red); // 默认颜色，会被子类覆盖
        shape.setPosition(rand() % (MAP_WIDTH - 30), rand() % (MAP_HEIGHT - 30));
    }

    virtual void moveTowards(const sf::Vector2f& target) {
        sf::Vector2f direction = target - shape.getPosition();
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0) {
            direction /= length;
            sf::Vector2f newPos = shape.getPosition() + direction * 1.0f;
            // 确保怪物在地图边界内移动
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

// 蓝色近战怪物（具有传送和变大能力） (无改动)
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
            if (teleportTimer < 90) { // 传送动画效果：缩小
                float scaleFactor = 1 - (static_cast<float>(teleportTimer) / 90);
                shape.setSize(sf::Vector2f(originalSize.x * scaleFactor, originalSize.y * scaleFactor));
            }
            else { // 传送完成
                sf::Vector2f newPos = target; // 传送到目标附近 (可以调整逻辑使其更精确)
                newPos.x -= shape.getSize().x / 2;
                newPos.y -= shape.getSize().y / 2;
                shape.setPosition(newPos);
                shape.setSize(originalSize); // 恢复大小
                isTeleporting = false;
                teleportTimer = 0;
                teleportCooldown = 600; // 10秒冷却
            }
            return; // 传送期间不进行普通移动
        }

        if (teleportCooldown > 0) {
            teleportCooldown--;
        }
        else if (rand() % 100 == 0) { // 随机触发传送
            isTeleporting = true;
            teleportTimer = 0;
        }

        if (enlargeCooldown > 0) {
            enlargeCooldown--;
        }
        else if (rand() % 100 == 0) { // 随机触发变大
            shape.setSize(sf::Vector2f(60, 60));
            enlargeCooldown = 600; // 10秒冷却
            isEnlarged = true;
            enlargeTimer = 0;
        }

        if (isEnlarged) {
            enlargeTimer++;
            if (enlargeTimer >= 180) { // 变大持续3秒
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

// 红色近战怪物（具有变大能力） (无改动)
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
        else if (rand() % 100 == 0) { // 随机触发变大
            shape.setSize(sf::Vector2f(60, 60));
            enlargeCooldown = 600; // 10秒冷却
            isEnlarged = true;
            enlargeTimer = 0;
        }

        if (isEnlarged) {
            enlargeTimer++;
            if (enlargeTimer >= 180) { // 变大持续3秒
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

// 黄色近战怪物 (无改动)
class YellowMeleeMonster : public MeleeMonster {
public:
    YellowMeleeMonster() : MeleeMonster() {
        shape.setFillColor(sf::Color::Yellow);
    }

    void moveTowards(const sf::Vector2f& target) override {
        MeleeMonster::moveTowards(target);
    }
};

// 子弹类 (无改动)
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
            velocity = direction * 5.0f; // 子弹速度
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

// 远程怪物类 (无改动)
class RangedMonster {
public:
    RangedMonster() {
        shape.setSize(sf::Vector2f(30, 30));
        shape.setFillColor(sf::Color::Magenta);
        shape.setPosition(rand() % (MAP_WIDTH - 30), rand() % (MAP_HEIGHT - 30));
        shootTimer = 0; // 射击计时器
    }

    void moveTowards(const sf::Vector2f& target) {
        sf::Vector2f direction = target - shape.getPosition();
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0) {
            direction /= length;
            sf::Vector2f newPos = shape.getPosition() + direction * 0.8f; // 远程怪移动慢一点
            // 确保怪物在地图边界内移动
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
        if (shootTimer >= 60) { // 每60帧 (1秒) 射击一次
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
void restartGame(Player* player, std::vector<BlueMeleeMonster>& blueMonsters,
    std::vector<RedMeleeMonster>& redMonsters, std::vector<YellowMeleeMonster>& yellowMonsters,
    std::vector<RangedMonster>& rangedMonsters, int& score, int& currentLevel) {
    player->reset();
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
    for (int i = 0; i < 1; ++i) { // 远程怪数量减少
        rangedMonsters.emplace_back();
    }
}

// 进入下一关
void nextLevel(Player* player, std::vector<BlueMeleeMonster>& blueMonsters,
    std::vector<RedMeleeMonster>& redMonsters, std::vector<YellowMeleeMonster>& yellowMonsters,
    std::vector<RangedMonster>& rangedMonsters, int& score, int& currentLevel) {
    player->reset(); // 玩家状态重置
    blueMonsters.clear();
    redMonsters.clear();
    yellowMonsters.clear();
    rangedMonsters.clear();
    score = 0; // 分数重置以计算当前关卡分数

    // 检查是否是最后一关
    if (currentLevel < MAX_LEVEL) {
        currentLevel++;
    }
    // (如果已经是最后一关且通过，游戏胜利逻辑在main循环中处理)

    // 根据当前关卡增加怪物数量
    int baseCount = currentLevel; // 减少基础怪物数量
    for (int i = 0; i < baseCount; ++i) {
        blueMonsters.emplace_back();
    }
    for (int i = 0; i < baseCount; ++i) {
        redMonsters.emplace_back();
    }
    for (int i = 0; i < baseCount; ++i) {
        yellowMonsters.emplace_back();
    }
    for (int i = 0; i < baseCount; ++i) { // 远程怪数量减少
        rangedMonsters.emplace_back();
    }
}


int main() {
    srand(static_cast<unsigned int>(time(nullptr)));


    // 加载字体
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "ERROR: Failed to load font!" << std::endl;
        // 如果无法加载字体，程序仍然继续运行，但文本将不可见
    }

    sf::RenderWindow window(sf::VideoMode(MAP_WIDTH, MAP_HEIGHT), "2D Game - Character Selection");
    window.setFramerateLimit(60);

    // 字体已在角色选择阶段加载

    // 游戏对象
    Player* player = nullptr; // 使用指针，稍后根据选择创建具体类型
    std::vector<BlueMeleeMonster> blueMonsters;
    std::vector<RedMeleeMonster> redMonsters;
    std::vector<YellowMeleeMonster> yellowMonsters;
    std::vector<RangedMonster> rangedMonsters;
    std::vector<Bullet> bullets;
    std::vector<Map> maps;
    maps.emplace_back("1.png");  // 第一关的地图图片
    maps.emplace_back("2.png");  // 第二关的地图图片
    maps.emplace_back("3.png");  // 第三关的地图图片
    int score = 0;
    int currentLevel = 1;

    // 角色选择界面元素
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color::White);
    titleText.setString("选择你的角色");
    titleText.setPosition(MAP_WIDTH / 2 - titleText.getGlobalBounds().width / 2, 100);

    // 近战角色选项
    sf::RectangleShape meleeOption(sf::Vector2f(200, 200));
    meleeOption.setFillColor(sf::Color::Blue);
    meleeOption.setPosition(MAP_WIDTH / 4 - 100, MAP_HEIGHT / 2 - 100);

    sf::Text meleeText;
    meleeText.setFont(font);
    meleeText.setCharacterSize(24);
    meleeText.setFillColor(sf::Color::White);
    meleeText.setString("Melee Player");
    meleeText.setPosition(MAP_WIDTH / 4 - meleeText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 120);

    // 远程角色选项
    sf::RectangleShape rangedOption(sf::Vector2f(200, 200));
    rangedOption.setFillColor(sf::Color::Green);
    rangedOption.setPosition(3 * MAP_WIDTH / 4 - 100, MAP_HEIGHT / 2 - 100);

    sf::Text rangedText;
    rangedText.setFont(font);
    rangedText.setCharacterSize(24);
    rangedText.setFillColor(sf::Color::White);
    rangedText.setString("Ranged Player");
    rangedText.setPosition(3 * MAP_WIDTH / 4 - rangedText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 120);

    // 角色描述
    sf::Text meleeDesc;
    meleeDesc.setFont(font);
    meleeDesc.setCharacterSize(18);
    meleeDesc.setFillColor(sf::Color::White);
    meleeDesc.setString("Fast Speed");
    meleeDesc.setPosition(MAP_WIDTH / 4 - meleeDesc.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 160);

    sf::Text rangedDesc;
    rangedDesc.setFont(font);
    rangedDesc.setCharacterSize(18);
    rangedDesc.setFillColor(sf::Color::White);
    rangedDesc.setString("Long Range");
    rangedDesc.setPosition(3 * MAP_WIDTH / 4 - rangedDesc.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 160);

    // 角色选择状态
    bool characterSelected = false;
    
    // 角色选择循环
    while (window.isOpen() && !characterSelected) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    
                    // 检查是否点击近战角色
                    if (meleeOption.getGlobalBounds().contains(mousePos)) {
                        player = new MeleePlayer();
                        characterSelected = true;
                    }
                    // 检查是否点击了远程角色
                    else if (rangedOption.getGlobalBounds().contains(mousePos)) {
                        player = new RangedPlayer();
                        characterSelected = true;
                    }
                }
            }
        }

        // 渲染角色选择界面
        window.clear(sf::Color::Black);
        window.draw(titleText);
        window.draw(meleeOption);
        window.draw(meleeText);
        window.draw(meleeDesc);
        window.draw(rangedOption);
        window.draw(rangedText);
        window.draw(rangedDesc);
        window.display();
    }

    // 如果窗口已关闭，直接返回
    if (!window.isOpen()) {
        delete player; // 清理资源
        return 0;
    }

    // 更改窗口标题
    window.setTitle("2D Game - Playing");

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

    // 字体已在角色选择阶段加载

    // UI元素 (无改动)
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

    sf::RectangleShape victoryRestartButton(sf::Vector2f(200, 50)); // 可以复用restartButton，但为了清晰分开
    victoryRestartButton.setFillColor(sf::Color::Green);
    victoryRestartButton.setPosition(MAP_WIDTH / 2 - 100, MAP_HEIGHT / 2 + 20);

    sf::Text victoryRestartText;
    victoryRestartText.setFont(font);
    victoryRestartText.setCharacterSize(24);
    victoryRestartText.setFillColor(sf::Color::White);
    victoryRestartText.setString("Restart");
    victoryRestartText.setPosition(MAP_WIDTH / 2 - victoryRestartText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 35);


    // 游戏状态 (无改动)
    bool gameOver = false;
    bool nextLevelAvailable = false;
    bool gameWon = false;
    // int survivalTimer = 0; // 这个变量在原代码中定义了但未使用，可以移除
    sf::Clock endGameClock; // 用于计时游戏结束后的等待时间
    bool shouldClose = false; // 标记是否需要关闭窗口

    // 游戏主循环
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && !shouldClose) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (!gameOver && !nextLevelAvailable && !gameWon) {
                        // 玩家射击
                        if (player->canShoot()) {
                            sf::Vector2f playerCenter = player->getShape().getPosition() + sf::Vector2f(player->getShape().getSize().x / 2, player->getShape().getSize().y / 2);
                            bullets.emplace_back(playerCenter, mousePos, true);
                            player->setShootCooldown();
                        }
                    }
                    else if (gameOver && restartButton.getGlobalBounds().contains(mousePos)) {
                        restartGame(player, blueMonsters, redMonsters, yellowMonsters, rangedMonsters, score, currentLevel);
                        bullets.clear();
                        gameOver = false;
                        gameWon = false; //确保重置
                        // survivalTimer = 0;
                    }
                    else if (nextLevelAvailable && nextLevelButton.getGlobalBounds().contains(mousePos)) {
                        nextLevel(player, blueMonsters, redMonsters, yellowMonsters, rangedMonsters, score, currentLevel);
                        bullets.clear();
                        nextLevelAvailable = false;
                        // survivalTimer = 0;
                    }
                    else if (gameWon && victoryRestartButton.getGlobalBounds().contains(mousePos)) {
                        restartGame(player, blueMonsters, redMonsters, yellowMonsters, rangedMonsters, score, currentLevel);
                        bullets.clear();
                        gameWon = false;
                        gameOver = false; //确保重置
                        // survivalTimer = 0;
                    }
                }
            }
        }

        // 检查是否需要关闭窗口 (无改动)
        if (shouldClose && endGameClock.getElapsedTime().asSeconds() >= 5.0f) {
            window.close();
            continue; // 跳过本轮剩余逻辑和渲染
        }


        if (!gameOver && !nextLevelAvailable && !gameWon && !shouldClose) {
            // 玩家移动
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                player->move(-5, 0);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                player->move(5, 0);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                player->move(0, -5);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                player->move(0, 5);

            player->updateInvincibility();
            player->updateShootCooldown();

            // 怪物移动
            sf::Vector2f playerPos = player->getShape().getPosition(); // 获取一次玩家位置供所有怪物使用
            for (auto& blueMonster : blueMonsters) {
                blueMonster.moveTowards(playerPos);
            }
            for (auto& redMonster : redMonsters) {
                redMonster.moveTowards(playerPos);
            }
            for (auto& yellowMonster : yellowMonsters) {
                yellowMonster.moveTowards(playerPos);
            }
            for (auto& rangedMonster : rangedMonsters) {
                rangedMonster.moveTowards(playerPos);
                rangedMonster.shoot(playerPos, bullets);
            }

            // 子弹移动和碰撞检测 (无改动)
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
                        else { ++blueIt; }
                    }
                    if (bulletHit) { it = bullets.erase(it); continue; } // 子弹已处理

                    // 检查红色怪物
                    for (auto redIt = redMonsters.begin(); redIt != redMonsters.end();) {
                        if (redIt->isHit(it->getShape())) {
                            redIt = redMonsters.erase(redIt);
                            bulletHit = true;
                            score++;
                            break;
                        }
                        else { ++redIt; }
                    }
                    if (bulletHit) { it = bullets.erase(it); continue; }

                    // 检查黄色怪物
                    for (auto yellowIt = yellowMonsters.begin(); yellowIt != yellowMonsters.end();) {
                        if (yellowIt->isHit(it->getShape())) {
                            yellowIt = yellowMonsters.erase(yellowIt);
                            bulletHit = true;
                            score++;
                            break;
                        }
                        else { ++yellowIt; }
                    }
                    if (bulletHit) { it = bullets.erase(it); continue; }

                    // 检查远程怪物
                    for (auto rangedIt = rangedMonsters.begin(); rangedIt != rangedMonsters.end();) {
                        if (rangedIt->isHit(it->getShape())) {
                            rangedIt = rangedMonsters.erase(rangedIt);
                            bulletHit = true;
                            score++;
                            break;
                        }
                        else { ++rangedIt; }
                    }
                    if (bulletHit) { it = bullets.erase(it); continue; }
                }
                // 怪物子弹击中玩家
                else if (isCollision(player->getShape(), it->getShape())) {
                    player->reduceHealth();
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
                if (isCollision(player->getShape(), blueMonster.getShape())) {
                    player->reduceHealth();
                }
            }
            for (const auto& redMonster : redMonsters) {
                if (isCollision(player->getShape(), redMonster.getShape())) {
                    player->reduceHealth();
                }
            }
            for (const auto& yellowMonster : yellowMonsters) {
                if (isCollision(player->getShape(), yellowMonster.getShape())) {
                    player->reduceHealth();
                }
            }
            for (const auto& rangedMonster : rangedMonsters) {
                if (isCollision(player->getShape(), rangedMonster.getShape())) {
                    player->reduceHealth();
                }
            }

            // 更新UI文本
            healthText.setString("Health: " + std::to_string(player->getHealth()));
            scoreText.setString("Score: " + std::to_string(score));
            levelText.setString("Level: " + std::to_string(currentLevel) + "/" + std::to_string(MAX_LEVEL));

            // 检查游戏状态
            if (player->getHealth() <= 0) {
                gameOver = true;
                endGameClock.restart(); // 开始计时
            }
            // 修改过关条件：当所有怪物被消灭时
            else if (score>=3) {
                if (currentLevel >= MAX_LEVEL) {
                    gameWon = true; // 最后一关完成，游戏胜利
                    endGameClock.restart(); // 开始计时
                }
                else {
                    nextLevelAvailable = true; // 还有下一关
                }
            }
        }

        // 渲染
        window.clear(); // 清除为黑色背景

        if (!gameOver && !nextLevelAvailable && !gameWon && !shouldClose) {
            // 首先绘制当前关卡的地图
            /*if (currentLevel > 0 && static_cast<size_t>(currentLevel) <= maps.size()) {
                maps[currentLevel - 1].draw(window); // currentLevel是1基准, vector是0基准
            }*/
 

            // 绘制玩家
            window.draw(player->getShape());

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
        else if (nextLevelAvailable && !shouldClose) {
            // 绘制关卡完成界面
            window.draw(nextLevelText);
            window.draw(nextLevelButton);
            window.draw(nextLevelConfirmText);
        }
        else if (gameWon || gameOver) { // 如果游戏结束或胜利
            if (gameWon) {
                window.draw(victoryText);
                window.draw(victoryRestartButton); // 在胜利界面也显示重启按钮
                window.draw(victoryRestartText);
            }
            else { // gameOver == true
                window.draw(gameOverText);
                window.draw(restartButton);
                window.draw(restartText);
            }

            // 如果已经过了5秒，标记需要关闭窗口
            if (endGameClock.getElapsedTime().asSeconds() >= 5.0f && (gameOver || gameWon)) {
                // 这里的逻辑是，如果玩家不点击重启，5秒后自动关闭
                // 如果需要点击按钮才关闭，则移除这个自动关闭逻辑或调整
                shouldClose = true;
                endGameClock.restart(); // 重启时钟，避免立即再次触发关闭
            }
        }
        window.display();
    }
    // 释放玩家对象内存
    delete player;
    return 0;
}