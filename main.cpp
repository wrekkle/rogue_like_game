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

// 障碍物类
class Obstacle {
public:
    Obstacle(float x, float y, float width, float height) {
        shape.setPosition(x, y);
        shape.setSize(sf::Vector2f(width, height));
        shape.setFillColor(sf::Color::White);
    }

    const sf::RectangleShape& getShape() const {
        return shape;
    }

    bool intersects(const sf::RectangleShape& other) const {
        return shape.getGlobalBounds().intersects(other.getGlobalBounds());
    }

private:
    sf::RectangleShape shape;
};

// 生成随机障碍物的函数
std::vector<Obstacle> generateObstacles(int level) {
    std::vector<Obstacle> obstacles;
    int numObstacles = 5 + level * 2; // 每关增加障碍物数量

    for (int i = 0; i < numObstacles; ++i) {
        float width = 30.0f + (rand() % 70); // 30-100的随机宽度
        float height = 30.0f + (rand() % 70); // 30-100的随机高度
        float x = rand() % (MAP_WIDTH - static_cast<int>(width));
        float y = rand() % (MAP_HEIGHT - static_cast<int>(height));

        // 创建临时障碍物用于检查
        Obstacle obstacle(x, y, width, height);

        // 检查是否与玩家出生点重叠（中心区域）
        sf::RectangleShape spawnArea;
        spawnArea.setSize(sf::Vector2f(100, 100));
        spawnArea.setPosition(350, 350);

        // 如果与出生点重叠，重新生成位置
        if (obstacle.intersects(spawnArea)) {
            --i;
            continue;
        }

        // 检查是否与其他障碍物重叠
        bool overlaps = false;
        for (const auto& existing : obstacles) {
            if (obstacle.intersects(existing.getShape())) {
                overlaps = true;
                break;
            }
        }

        if (!overlaps) {
            obstacles.push_back(obstacle);
        }
        else {
            --i; // 重试
        }
    }

    return obstacles;
}

// 检查是否与任何障碍物碰撞
bool checkObstacleCollision(const sf::RectangleShape& object, const std::vector<Obstacle>& obstacles) {
    for (const auto& obstacle : obstacles) {
        if (obstacle.intersects(object)) {
            return true;
        }
    }
    return false;
}


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

    virtual ~Player() = default;

    virtual void move(float dx, float dy, const std::vector<Obstacle>& obstacles) {
        sf::Vector2f newPos = shape.getPosition() + sf::Vector2f(dx, dy);
        sf::RectangleShape newShape = shape;
        newShape.setPosition(newPos);

        // 检查新位置是否与障碍物碰撞
        if (!checkObstacleCollision(newShape, obstacles)) {
            if (newPos.x >= 0 && newPos.x + shape.getSize().x <= static_cast<float>(MAP_WIDTH)) {
                shape.move(dx, 0);
            }
            if (newPos.y >= 0 && newPos.y + shape.getSize().y <= static_cast<float>(MAP_HEIGHT)) {
                shape.move(0, dy);
            }
            updateSprite();
        }
    }

    sf::RectangleShape getShape() const {
        return shape;
    }

    sf::Sprite getSprite() const {
        return sprite;
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

    virtual void reset() {
        shape.setPosition(375, 375);
        updateSprite();
        health = 5;
        invincibilityFrames = 0;
    }

    virtual bool canShoot() const {
        return shootCooldown == 0;
    }

    virtual void setShootCooldown() {
        shootCooldown = 15;
    }

    virtual void updateShootCooldown() {
        if (shootCooldown > 0) {
            shootCooldown--;
        }
    }

protected:
    sf::RectangleShape shape;
    sf::Sprite sprite;
    sf::Texture texture;
    int health;
    int invincibilityFrames;
    int shootCooldown;

    void updateSprite() {
        sprite.setPosition(shape.getPosition());
    }
};

// 近战玩家类
class MeleePlayer : public Player {
public:
    MeleePlayer() : Player() {
        if (!texture.loadFromFile("resources/player/l11.png")) {
            std::cerr << "Error: Failed to load melee player texture!" << std::endl;
        }
        sprite.setTexture(texture, true);  // true表示保持原始格式
        sprite.setPosition(shape.getPosition());

        attackRange = 100.0f;  // 增加攻击范围
        sweepAnimating = false;
        sweepAngle = 0.f;
        sweepParticles.clear();
    }

    float getAttackRange() const { return attackRange; }
    void setShootCooldown() override {
        shootCooldown = 20;
    }

    void startSweep() {
        sweepAnimating = true;
        sweepAngle = 0.f;
        sweepParticles.clear();
    }

    void updateSweep() {
        if (sweepAnimating) {
            sweepAngle += 24.f;  // 保持原有的旋转速度

            // 添加新的粒子
            if (rand() % 2 == 0) {  // 50%的概率生成新粒子
                float currentAngle = (-90.f + sweepAngle) * 3.14159f / 180.f;
                sf::Vector2f center = shape.getPosition() + shape.getSize() / 2.f;
                float randRadius = attackRange * (0.6f + (rand() % 40) / 100.0f);  // 在60%-100%范围内随机

                SweepParticle particle;
                particle.position = center + sf::Vector2f(
                    std::cos(currentAngle) * randRadius,
                    std::sin(currentAngle) * randRadius
                );
                particle.color = sf::Color(100, 200, 255, 255);  // 浅蓝色
                particle.lifetime = 10;  // 粒子持续10帧
                sweepParticles.push_back(particle);
            }

            // 更新现有粒子
            for (auto& particle : sweepParticles) {
                particle.lifetime--;
                particle.color.a = static_cast<sf::Uint8>((particle.lifetime / 10.0f) * 255);
            }

            // 移除消失的粒子
            sweepParticles.erase(
                std::remove_if(sweepParticles.begin(), sweepParticles.end(),
                    [](const SweepParticle& p) { return p.lifetime <= 0; }),
                sweepParticles.end()
            );

            if (sweepAngle >= 360.f) {
                sweepAnimating = false;
                sweepAngle = 0.f;
                sweepParticles.clear();
            }
        }
    }

    void drawSweepEffect(sf::RenderWindow& window) const {
        if (sweepAnimating) {
            // 绘制主要的扇形特效
            float playerSize = shape.getSize().x;
            float outerRadius = attackRange;
            float innerRadius = playerSize * 0.8f;
            sf::Vector2f center = shape.getPosition() + shape.getSize() / 2.f;

            // 创建渐变效果的多层扇形
            for (float r = innerRadius; r <= outerRadius; r += (outerRadius - innerRadius) / 5.0f) {
                int segments = 60;
                float angleStep = sweepAngle / segments;
                sf::VertexArray ring(sf::TriangleStrip, (segments + 1) * 2);

                for (int i = 0; i <= segments; ++i) {
                    float angle = -90.f + i * angleStep;
                    float rad = angle * 3.14159f / 180.f;
                    float alpha = 1.0f - ((r - innerRadius) / (outerRadius - innerRadius));
                    sf::Uint8 alphaValue = static_cast<sf::Uint8>(alpha * 128);  // 最大透明度降低到128

                    // 外圈顶点
                    ring[i * 2].position = center + sf::Vector2f(std::cos(rad) * (r + 5), std::sin(rad) * (r + 5));
                    ring[i * 2].color = sf::Color(100, 200, 255, alphaValue);  // 浅蓝色

                    // 内圈顶点
                    ring[i * 2 + 1].position = center + sf::Vector2f(std::cos(rad) * r, std::sin(rad) * r);
                    ring[i * 2 + 1].color = sf::Color(100, 200, 255, alphaValue);
                }
                window.draw(ring);
            }

            // 绘制粒子
            for (const auto& particle : sweepParticles) {
                sf::CircleShape particleShape(3);  // 粒子大小为3像素
                particleShape.setFillColor(particle.color);
                particleShape.setPosition(particle.position);
                particleShape.setOrigin(1.5f, 1.5f);  // 设置原点为中心
                window.draw(particleShape);
            }
        }
    }

    bool isSweeping() const { return sweepAnimating; }
    float getSweepAngle() const { return sweepAngle; }

private:
    float attackRange;
    bool sweepAnimating;
    float sweepAngle;

    // 粒子结构
    struct SweepParticle {
        sf::Vector2f position;
        sf::Color color;
        int lifetime;
    };
    std::vector<SweepParticle> sweepParticles;
};

// 远程玩家类
class RangedPlayer : public Player {
public:
    RangedPlayer() : Player() {
        if (!texture.loadFromFile("resources/player/tales1.png")) {
            std::cerr << "Error: Failed to load ranged player texture!" << std::endl;
        }
        sprite.setTexture(texture, true);  // true表示保持原始格式
        sprite.setPosition(shape.getPosition());

        bulletSpeed = 7.0f;
    }

    float getBulletSpeed() const {
        return bulletSpeed;
    }

    void setShootCooldown() override {
        shootCooldown = 20;
    }

private:
    float bulletSpeed;
};

// 近战怪物基类
class MeleeMonster {
public:
    MeleeMonster() {
        shape.setSize(sf::Vector2f(30, 30));
        shape.setPosition(rand() % (MAP_WIDTH - 30), rand() % (MAP_HEIGHT - 30));
    }

    // 检查某个方向是否可以移动
    bool canMove(const sf::Vector2f& direction, const std::vector<Obstacle>& obstacles) {
        sf::Vector2f newPos = shape.getPosition() + direction;
        sf::RectangleShape newShape = shape;
        newShape.setPosition(newPos);

        return !checkObstacleCollision(newShape, obstacles) &&
            newPos.x >= 0 && newPos.x + shape.getSize().x <= static_cast<float>(MAP_WIDTH) &&
            newPos.y >= 0 && newPos.y + shape.getSize().y <= static_cast<float>(MAP_HEIGHT);
    }

    // 寻找替代移动方向
    sf::Vector2f findAlternativeDirection(const sf::Vector2f& originalDir, const std::vector<Obstacle>& obstacles) {
        // 尝试8个不同的方向
        const float angles[8] = { 45, -45, 90, -90, 135, -135, 180, 0 };  // 角度

        for (float angle : angles) {
            // 将角度转换为弧度
            float radian = angle * 3.14159f / 180.0f;

            // 计算新的方向
            sf::Vector2f newDir;
            newDir.x = std::cos(radian) * originalDir.x - std::sin(radian) * originalDir.y;
            newDir.y = std::sin(radian) * originalDir.x + std::cos(radian) * originalDir.y;

            // 标准化方向向量
            float length = std::sqrt(newDir.x * newDir.x + newDir.y * newDir.y);
            if (length > 0) {
                newDir /= length;
            }

            // 检查新方向是否可行
            if (canMove(newDir * 5.0f, obstacles)) {
                return newDir;
            }
        }

        return sf::Vector2f(0, 0); // 如果没有找到可行方向，返回零向量
    }

    virtual void moveTowards(const sf::Vector2f& target, const std::vector<Obstacle>& obstacles) {
        sf::Vector2f direction = target - shape.getPosition();
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (length > 0) {
            direction /= length;
            sf::Vector2f newPos = shape.getPosition() + direction * 1.0f;
            sf::RectangleShape newShape = shape;
            newShape.setPosition(newPos);

            if (!checkObstacleCollision(newShape, obstacles)) {
                // 如果没有碰到障碍物，正常移动
                if (newPos.x >= 0 && newPos.x + shape.getSize().x <= static_cast<float>(MAP_WIDTH)) {
                    shape.move(direction.x * 1.0f, 0);
                }
                if (newPos.y >= 0 && newPos.y + shape.getSize().y <= static_cast<float>(MAP_HEIGHT)) {
                    shape.move(0, direction.y * 1.0f);
                }
            }
            else {
                // 如果碰到障碍物，寻找替代路径
                sf::Vector2f alternativeDir = findAlternativeDirection(direction, obstacles);
                if (alternativeDir.x != 0 || alternativeDir.y != 0) {
                    if (shape.getPosition().x + alternativeDir.x >= 0 &&
                        shape.getPosition().x + alternativeDir.x + shape.getSize().x <= static_cast<float>(MAP_WIDTH)) {
                        shape.move(alternativeDir.x * 1.0f, 0);
                    }
                    if (shape.getPosition().y + alternativeDir.y >= 0 &&
                        shape.getPosition().y + alternativeDir.y + shape.getSize().y <= static_cast<float>(MAP_HEIGHT)) {
                        shape.move(0, alternativeDir.y * 1.0f);
                    }
                }
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

// 传送特效类
class TeleportEffect {
public:
    TeleportEffect(const sf::Vector2f& position) {
        this->position = position;
        timer = 0;
        maxTimer = 20; // 20帧动画时长

        // 创建多个粒子
        for (int i = 0; i < 12; ++i) {
            sf::CircleShape particle(2);
            particle.setFillColor(sf::Color::Cyan);
            particle.setPosition(position);
            particles.push_back(particle);

            // 设置粒子的初始速度（向外扩散的圆形pattern）
            float angle = (i * 30.0f) * 3.14159f / 180.0f; // 每30度一个粒子
            sf::Vector2f velocity;
            velocity.x = std::cos(angle) * 3.0f;
            velocity.y = std::sin(angle) * 3.0f;
            velocities.push_back(velocity);
        }
    }

    bool update() {
        timer++;

        // 更新粒子位置和透明度
        float alpha = 1.0f - (static_cast<float>(timer) / maxTimer);
        sf::Uint8 alphaValue = static_cast<sf::Uint8>(alpha * 255);

        for (size_t i = 0; i < particles.size(); ++i) {
            particles[i].move(velocities[i]);
            sf::Color particleColor = sf::Color::Cyan;
            particleColor.a = alphaValue;
            particles[i].setFillColor(particleColor);
        }

        return timer < maxTimer;
    }

    void draw(sf::RenderWindow& window) const {
        for (const auto& particle : particles) {
            window.draw(particle);
        }
    }

private:
    sf::Vector2f position;
    std::vector<sf::CircleShape> particles;
    std::vector<sf::Vector2f> velocities;
    int timer;
    int maxTimer;
};

// 蓝色近战怪物
class BlueMeleeMonster : public MeleeMonster {
public:
    BlueMeleeMonster() : MeleeMonster() {
        shape.setFillColor(sf::Color::Blue);
        teleportCooldown = 0;
        isTeleporting = false;
        teleportTimer = 0;
        hasStartEffect = false;
    }

    void moveTowards(const sf::Vector2f& target, std::vector<TeleportEffect>& effects, const std::vector<Obstacle>& obstacles) {
        if (isTeleporting) {
            teleportTimer++;

            if (!hasStartEffect) {
                sf::Vector2f effectPos = shape.getPosition() + shape.getSize() / 2.f;
                effects.emplace_back(effectPos);
                hasStartEffect = true;
            }

            if (teleportTimer >= 90) {
                sf::Vector2f newPos = target;
                newPos.x -= shape.getSize().x / 2;
                newPos.y -= shape.getSize().y / 2;

                // 检查传送目标位置是否与障碍物碰撞
                sf::RectangleShape newShape = shape;
                newShape.setPosition(newPos);
                if (!checkObstacleCollision(newShape, obstacles)) {
                    shape.setPosition(newPos);
                    sf::Vector2f effectPos = newPos + shape.getSize() / 2.f;
                    effects.emplace_back(effectPos);
                }

                isTeleporting = false;
                teleportTimer = 0;
                teleportCooldown = 600;
                hasStartEffect = false;
            }
            return;
        }

        if (teleportCooldown > 0) {
            teleportCooldown--;
        }
        else if (rand() % 100 == 0) {
            isTeleporting = true;
            teleportTimer = 0;
            hasStartEffect = false;
        }

        if (!isTeleporting) {
            MeleeMonster::moveTowards(target, obstacles);
        }
    }

private:
    int teleportCooldown;
    bool isTeleporting;
    int teleportTimer;
    bool hasStartEffect;
};

// 红色近战怪物
class RedMeleeMonster : public MeleeMonster {
public:
    RedMeleeMonster() : MeleeMonster() {
        shape.setFillColor(sf::Color::Red);
    }
};

// 黄色近战怪物
class YellowMeleeMonster : public MeleeMonster {
public:
    YellowMeleeMonster() : MeleeMonster() {
        shape.setFillColor(sf::Color::Yellow);
    }
};

// 死亡特效类
class DeathEffect {
public:
    DeathEffect(const sf::Vector2f& position, const sf::Color& color) {
        this->position = position;
        this->color = color;
        timer = 0;
        maxTimer = 30; // 30帧动画时长

        // 创建多个粒子
        for (int i = 0; i < 8; ++i) {
            sf::CircleShape particle(3);
            particle.setFillColor(color);
            particle.setPosition(position);
            particles.push_back(particle);

            // 随机速度方向
            float angle = (i * 45.0f) * 3.14159f / 180.0f; // 每45度一个粒子
            sf::Vector2f velocity;
            velocity.x = std::cos(angle) * (2.0f + rand() % 3);
            velocity.y = std::sin(angle) * (2.0f + rand() % 3);
            velocities.push_back(velocity);
        }
    }

    bool update() {
        timer++;

        // 更新粒子位置和透明度
        float alpha = 1.0f - (static_cast<float>(timer) / maxTimer);
        sf::Uint8 alphaValue = static_cast<sf::Uint8>(alpha * 255);

        for (size_t i = 0; i < particles.size(); ++i) {
            particles[i].move(velocities[i]);
            sf::Color particleColor = color;
            particleColor.a = alphaValue;
            particles[i].setFillColor(particleColor);

            // 减速效果
            velocities[i] *= 0.95f;
        }

        return timer < maxTimer;
    }

    void draw(sf::RenderWindow& window) const {
        for (const auto& particle : particles) {
            window.draw(particle);
        }
    }

private:
    sf::Vector2f position;
    sf::Color color;
    std::vector<sf::CircleShape> particles;
    std::vector<sf::Vector2f> velocities;
    int timer;
    int maxTimer;
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

    bool move(const std::vector<Obstacle>& obstacles) {
        sf::Vector2f newPos = shape.getPosition() + velocity;
        sf::RectangleShape newShape = shape;
        newShape.setPosition(newPos);

        // 检查是否与障碍物碰撞
        if (checkObstacleCollision(newShape, obstacles)) {
            return false; // 子弹碰到障碍物，返回false表示需要删除
        }

        shape.move(velocity);
        return true;
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
        shape.setPosition(rand() % (MAP_WIDTH - 30), rand() % (MAP_HEIGHT - 30));
        shape.setFillColor(sf::Color::Magenta);
        shootTimer = 0;
    }

    bool canMove(const sf::Vector2f& direction, const std::vector<Obstacle>& obstacles) {
        sf::Vector2f newPos = shape.getPosition() + direction;
        sf::RectangleShape newShape = shape;
        newShape.setPosition(newPos);

        return !checkObstacleCollision(newShape, obstacles) &&
            newPos.x >= 0 && newPos.x + shape.getSize().x <= static_cast<float>(MAP_WIDTH) &&
            newPos.y >= 0 && newPos.y + shape.getSize().y <= static_cast<float>(MAP_HEIGHT);
    }

    sf::Vector2f findAlternativeDirection(const sf::Vector2f& originalDir, const std::vector<Obstacle>& obstacles) {
        const float angles[8] = { 45, -45, 90, -90, 135, -135, 180, 0 };

        for (float angle : angles) {
            float radian = angle * 3.14159f / 180.0f;
            sf::Vector2f newDir;
            newDir.x = std::cos(radian) * originalDir.x - std::sin(radian) * originalDir.y;
            newDir.y = std::sin(radian) * originalDir.x + std::cos(radian) * originalDir.y;

            float length = std::sqrt(newDir.x * newDir.x + newDir.y * newDir.y);
            if (length > 0) {
                newDir /= length;
            }

            if (canMove(newDir * 5.0f, obstacles)) {
                return newDir;
            }
        }

        return sf::Vector2f(0, 0);
    }

    void moveTowards(const sf::Vector2f& target, const std::vector<Obstacle>& obstacles) {
        sf::Vector2f direction = target - shape.getPosition();
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (length > 0) {
            direction /= length;
            sf::Vector2f newPos = shape.getPosition() + direction * 0.8f;
            sf::RectangleShape newShape = shape;
            newShape.setPosition(newPos);

            if (!checkObstacleCollision(newShape, obstacles)) {
                if (newPos.x >= 0 && newPos.x + shape.getSize().x <= static_cast<float>(MAP_WIDTH)) {
                    shape.move(direction.x * 0.8f, 0);
                }
                if (newPos.y >= 0 && newPos.y + shape.getSize().y <= static_cast<float>(MAP_HEIGHT)) {
                    shape.move(0, direction.y * 0.8f);
                }
            }
            else {
                sf::Vector2f alternativeDir = findAlternativeDirection(direction, obstacles);
                if (alternativeDir.x != 0 || alternativeDir.y != 0) {
                    if (shape.getPosition().x + alternativeDir.x >= 0 &&
                        shape.getPosition().x + alternativeDir.x + shape.getSize().x <= static_cast<float>(MAP_WIDTH)) {
                        shape.move(alternativeDir.x * 0.8f, 0);
                    }
                    if (shape.getPosition().y + alternativeDir.y >= 0 &&
                        shape.getPosition().y + alternativeDir.y + shape.getSize().y <= static_cast<float>(MAP_HEIGHT)) {
                        shape.move(0, alternativeDir.y * 0.8f);
                    }
                }
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
void restartGame(Player*& player, std::vector<BlueMeleeMonster>& blueMonsters,
    std::vector<RedMeleeMonster>& redMonsters, std::vector<YellowMeleeMonster>& yellowMonsters,
    std::vector<RangedMonster>& rangedMonsters, int& score, int& currentLevel, std::vector<Obstacle>& obstacles) {
    delete player;
    player = nullptr;
    blueMonsters.clear();
    redMonsters.clear();
    yellowMonsters.clear();
    rangedMonsters.clear();
    obstacles.clear();
    score = 0;
    currentLevel = 1;
}

// 进入下一关
void nextLevel(Player* player, std::vector<BlueMeleeMonster>& blueMonsters,
    std::vector<RedMeleeMonster>& redMonsters, std::vector<YellowMeleeMonster>& yellowMonsters,
    std::vector<RangedMonster>& rangedMonsters, int& score, int& currentLevel, std::vector<Obstacle>& obstacles) {
    player->reset();
    blueMonsters.clear();
    redMonsters.clear();
    yellowMonsters.clear();
    rangedMonsters.clear();
    score = 0;

    if (currentLevel < MAX_LEVEL) {
        currentLevel++;
    }

    // 生成新的障碍物
    obstacles = generateObstacles(currentLevel);

    int baseCount = currentLevel + 1;
    for (int i = 0; i < baseCount; ++i) {
        blueMonsters.emplace_back();
    }
    for (int i = 0; i < baseCount; ++i) {
        redMonsters.emplace_back();
    }
    for (int i = 0; i < baseCount; ++i) {
        yellowMonsters.emplace_back();
    }
    for (int i = 0; i < baseCount; ++i) {
        rangedMonsters.emplace_back();
    }
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    // 加载字体
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "ERROR: Failed to load font!" << std::endl;
    }

    sf::RenderWindow window(sf::VideoMode(MAP_WIDTH, MAP_HEIGHT), "2D Game - Character Selection");
    window.setFramerateLimit(60);

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
    restartButton.setPosition(MAP_WIDTH / 2 - 250, MAP_HEIGHT / 2 + 20); // 向左移动更多

    sf::Text restartText;
    restartText.setFont(font);
    restartText.setCharacterSize(24);
    restartText.setFillColor(sf::Color::White);
    restartText.setString("Restart");
    restartText.setPosition(MAP_WIDTH / 2 - 250 + 100 - restartText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 35);

    // 退出按钮
    sf::RectangleShape quitButton(sf::Vector2f(200, 50));
    quitButton.setFillColor(sf::Color::Red);
    quitButton.setPosition(MAP_WIDTH / 2 + 50, MAP_HEIGHT / 2 + 20); // 向右移动更多

    sf::Text quitText;
    quitText.setFont(font);
    quitText.setCharacterSize(24);
    quitText.setFillColor(sf::Color::White);
    quitText.setString("Quit");
    quitText.setPosition(MAP_WIDTH / 2 + 50 + 100 - quitText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 35);

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
    victoryRestartButton.setPosition(MAP_WIDTH / 2 - 250, MAP_HEIGHT / 2 + 20); // Changed X from MAP_WIDTH / 2 - 100

    sf::Text victoryRestartText;
    victoryRestartText.setFont(font);
    victoryRestartText.setCharacterSize(24);
    victoryRestartText.setFillColor(sf::Color::White);
    victoryRestartText.setString("Restart");
    // Center text on the button
    victoryRestartText.setPosition(MAP_WIDTH / 2 - 250 + (200 / 2) - victoryRestartText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 35);

    // 角色选择界面元素
    std::vector<sf::Texture> bgTextures(4);
    std::vector<sf::Sprite> bgSprites(4);
    bool bgLoaded[4] = { false, false, false, false };
    const char* bgFiles[4] = { "1.png", "2.png", "3.png", "4.png" };
    for (int i = 0; i < 4; ++i) {
        if (bgTextures[i].loadFromFile(bgFiles[i])) {
            bgSprites[i].setTexture(bgTextures[i]);
            bgLoaded[i] = true;
        }
    }
    int bgIndex = 0;
    sf::Clock bgClock;
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color::White);
    titleText.setString("Select Your Character!");
    titleText.setPosition(MAP_WIDTH / 2 - titleText.getGlobalBounds().width / 2, 100);

    // 近战角色选项
    sf::Texture meleeTexture;
    sf::Sprite meleeSprite;
    if (!meleeTexture.loadFromFile("resources/player/l1.png")) {
        std::cerr << "Error: Failed to load melee character texture!" << std::endl;
    }
    meleeSprite.setTexture(meleeTexture, true);
    meleeSprite.setPosition(MAP_WIDTH / 4 - meleeSprite.getGlobalBounds().width / 2, MAP_HEIGHT / 2 - meleeSprite.getGlobalBounds().height / 2);

    sf::Text meleeText;
    meleeText.setFont(font);
    meleeText.setCharacterSize(24);
    meleeText.setFillColor(sf::Color::White);
    meleeText.setString("Melee Player");
    meleeText.setPosition(MAP_WIDTH / 4 - meleeText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 120);

    sf::Text meleeDesc;
    meleeDesc.setFont(font);
    meleeDesc.setCharacterSize(18);
    meleeDesc.setFillColor(sf::Color::White);
    meleeDesc.setString("Fast Speed");
    meleeDesc.setPosition(MAP_WIDTH / 4 - meleeDesc.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 160);

    // 远程角色选项
    sf::Texture rangedTexture;
    sf::Sprite rangedSprite;
    if (!rangedTexture.loadFromFile("resources/player/tales.png")) {
        std::cerr << "Error: Failed to load ranged character texture!" << std::endl;
    }
    rangedSprite.setTexture(rangedTexture, true);
    rangedSprite.setPosition(3 * MAP_WIDTH / 4 - rangedSprite.getGlobalBounds().width / 2, MAP_HEIGHT / 2 - rangedSprite.getGlobalBounds().height / 2);

    sf::Text rangedText;
    rangedText.setFont(font);
    rangedText.setCharacterSize(24);
    rangedText.setFillColor(sf::Color::White);
    rangedText.setString("Ranged Player");
    rangedText.setPosition(3 * MAP_WIDTH / 4 - rangedText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 120);

    sf::Text rangedDesc;
    rangedDesc.setFont(font);
    rangedDesc.setCharacterSize(18);
    rangedDesc.setFillColor(sf::Color::White);
    rangedDesc.setString("Long Range");
    rangedDesc.setPosition(3 * MAP_WIDTH / 4 - rangedDesc.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 160);

    // 游戏对象
    Player* player = nullptr;
    std::vector<BlueMeleeMonster> blueMonsters;
    std::vector<RedMeleeMonster> redMonsters;
    std::vector<YellowMeleeMonster> yellowMonsters;
    std::vector<RangedMonster> rangedMonsters;
    std::vector<Bullet> bullets;
    std::vector<DeathEffect> deathEffects;
    std::vector<TeleportEffect> teleportEffects;  // 添加传送特效容器
    int score = 0;
    int currentLevel = 1;
    std::vector<Obstacle> obstacles;

    // 游戏状态
    bool gameOver = false;
    bool nextLevelAvailable = false;
    bool gameWon = false;
    bool needCharacterSelection = true;  // 初始状态需要选择角色

    // 在游戏开始时生成第一关的障碍物
    obstacles = generateObstacles(currentLevel);

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
                        // 游戏进行中的点击处理
                        if (dynamic_cast<MeleePlayer*>(player) && player->canShoot()) {
                            static_cast<MeleePlayer*>(player)->startSweep();
                            player->setShootCooldown();
                        }
                        else if (player->canShoot()) {
                            sf::Vector2f playerCenter = player->getShape().getPosition() + sf::Vector2f(player->getShape().getSize().x / 2, player->getShape().getSize().y / 2);
                            bullets.emplace_back(playerCenter, mousePos, true);
                            player->setShootCooldown();
                        }
                    }
                    else if (gameOver) {
                        if (restartButton.getGlobalBounds().contains(mousePos)) {
                            restartGame(player, blueMonsters, redMonsters, yellowMonsters, rangedMonsters, score, currentLevel, obstacles);
                            bullets.clear();
                            deathEffects.clear();
                            teleportEffects.clear();
                            gameOver = false;
                            gameWon = false;
                            needCharacterSelection = true;  // 标记需要重新选择角色
                        }
                        else if (quitButton.getGlobalBounds().contains(mousePos)) {
                            window.close();
                        }
                    }
                    else if (nextLevelAvailable && nextLevelButton.getGlobalBounds().contains(mousePos)) {
                        nextLevel(player, blueMonsters, redMonsters, yellowMonsters, rangedMonsters, score, currentLevel, obstacles);
                        bullets.clear();
                        deathEffects.clear();
                        teleportEffects.clear();
                        nextLevelAvailable = false;
                    }
                    else if (gameWon) {
                        if (victoryRestartButton.getGlobalBounds().contains(mousePos)) {
                            restartGame(player, blueMonsters, redMonsters, yellowMonsters, rangedMonsters, score, currentLevel, obstacles);
                            bullets.clear();
                            deathEffects.clear();
                            teleportEffects.clear();
                            gameWon = false;
                            gameOver = false;
                            needCharacterSelection = true;  // 标记需要重新选择角色
                        }
                        else if (quitButton.getGlobalBounds().contains(mousePos)) {
                            window.close();
                        }
                    }
                }
            }
        }

        // 如果需要重新选择角色，进入角色选择循环
        if (needCharacterSelection) {
            bool characterSelected = false;
            window.setTitle("2D Game - Character Selection");

            // 角色选择循环
            while (window.isOpen() && !characterSelected) {
                sf::Event selectEvent;
                while (window.pollEvent(selectEvent)) {
                    if (selectEvent.type == sf::Event::Closed)
                        window.close();

                    if (selectEvent.type == sf::Event::MouseButtonPressed) {
                        if (selectEvent.mouseButton.button == sf::Mouse::Left) {
                            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                            // 检查是否点击近战角色
                            if (meleeSprite.getGlobalBounds().contains(mousePos)) {
                                player = new MeleePlayer();
                                characterSelected = true;
                            }
                            // 检查是否点击了远程角色
                            else if (rangedSprite.getGlobalBounds().contains(mousePos)) {
                                player = new RangedPlayer();
                                characterSelected = true;
                            }
                        }
                    }
                }

                // 渲染角色选择界面
                window.clear(sf::Color::Black);
                if (bgClock.getElapsedTime().asSeconds() > 1.5f) {
                    bgIndex = (bgIndex + 1) % 4;
                    bgClock.restart();
                }
                if (bgLoaded[bgIndex]) window.draw(bgSprites[bgIndex]);
                window.draw(titleText);
                window.draw(meleeSprite);
                window.draw(meleeText);
                window.draw(meleeDesc);
                window.draw(rangedSprite);
                window.draw(rangedText);
                window.draw(rangedDesc);
                window.display();
            }

            // 如果窗口已关闭，直接返回
            if (!window.isOpen()) {
                delete player;
                return 0;
            }

            // 更改窗口标题
            window.setTitle("2D Game - Playing");
            needCharacterSelection = false;  // 重置标记

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
        }

        if (!gameOver && !nextLevelAvailable && !gameWon) {
            // 玩家移动
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                player->move(-5, 0, obstacles);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                player->move(5, 0, obstacles);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                player->move(0, -5, obstacles);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                player->move(0, 5, obstacles);

            player->updateInvincibility();
            player->updateShootCooldown();
            // 横扫动画与范围伤害
            if (dynamic_cast<MeleePlayer*>(player)) {
                MeleePlayer* melee = static_cast<MeleePlayer*>(player);
                melee->updateSweep();
                static bool damageApplied = false;
                if (melee->isSweeping()) {
                    float sweepRadius = melee->getAttackRange();
                    float sweepAngle = melee->getSweepAngle();
                    sf::Vector2f center = melee->getShape().getPosition() + melee->getShape().getSize() / 2.f;
                    if (!damageApplied && sweepAngle > 180.f) {
                        auto inRange = [&](const sf::RectangleShape& m) {
                            sf::Vector2f mCenter = m.getPosition() + m.getSize() / 2.f;
                            float dist = std::hypot(center.x - mCenter.x, center.y - mCenter.y);
                            return dist <= sweepRadius;
                            };
                        for (auto it = blueMonsters.begin(); it != blueMonsters.end();) {
                            if (inRange(it->getShape())) {
                                deathEffects.emplace_back(it->getShape().getPosition() + it->getShape().getSize() / 2.f, sf::Color::Blue);
                                it = blueMonsters.erase(it);
                                score++;
                            }
                            else { ++it; }
                        }
                        for (auto it = redMonsters.begin(); it != redMonsters.end();) {
                            if (inRange(it->getShape())) {
                                deathEffects.emplace_back(it->getShape().getPosition() + it->getShape().getSize() / 2.f, sf::Color::Red);
                                it = redMonsters.erase(it);
                                score++;
                            }
                            else { ++it; }
                        }
                        for (auto it = yellowMonsters.begin(); it != yellowMonsters.end();) {
                            if (inRange(it->getShape())) {
                                deathEffects.emplace_back(it->getShape().getPosition() + it->getShape().getSize() / 2.f, sf::Color::Yellow);
                                it = yellowMonsters.erase(it);
                                score++;
                            }
                            else { ++it; }
                        }
                        for (auto it = rangedMonsters.begin(); it != rangedMonsters.end();) {
                            if (inRange(it->getShape())) {
                                deathEffects.emplace_back(it->getShape().getPosition() + it->getShape().getSize() / 2.f, sf::Color::Magenta);
                                it = rangedMonsters.erase(it);
                                score++;
                            }
                            else { ++it; }
                        }
                        damageApplied = true;
                    }
                }
                else {
                    damageApplied = false;
                }
            }
            // 更新死亡特效
            for (auto effectIt = deathEffects.begin(); effectIt != deathEffects.end();) {
                if (!effectIt->update()) {
                    effectIt = deathEffects.erase(effectIt);
                }
                else {
                    ++effectIt;
                }
            }

            // 怪物移动
            sf::Vector2f playerPos = player->getShape().getPosition();
            for (auto& blueMonster : blueMonsters) {
                blueMonster.moveTowards(playerPos, teleportEffects, obstacles);
            }
            for (auto& redMonster : redMonsters) {
                redMonster.moveTowards(playerPos, obstacles);
            }
            for (auto& yellowMonster : yellowMonsters) {
                yellowMonster.moveTowards(playerPos, obstacles);
            }
            for (auto& rangedMonster : rangedMonsters) {
                rangedMonster.moveTowards(playerPos, obstacles);
                rangedMonster.shoot(playerPos, bullets);
            }

            // 子弹移动和碰撞检测
            for (auto it = bullets.begin(); it != bullets.end();) {
                bool bulletSurvived = it->move(obstacles); // 移动子弹并检查障碍物碰撞
                bool bulletHit = false;

                if (!bulletSurvived) {
                    it = bullets.erase(it);
                    continue;
                }

                // 玩家子弹击中怪物
                if (it->isFromPlayer()) {
                    // 检查蓝色怪物
                    for (auto blueIt = blueMonsters.begin(); blueIt != blueMonsters.end();) {
                        if (blueIt->isHit(it->getShape())) {
                            deathEffects.emplace_back(blueIt->getShape().getPosition() + blueIt->getShape().getSize() / 2.f, sf::Color::Blue);
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
                            deathEffects.emplace_back(redIt->getShape().getPosition() + redIt->getShape().getSize() / 2.f, sf::Color::Red);
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
                            deathEffects.emplace_back(yellowIt->getShape().getPosition() + yellowIt->getShape().getSize() / 2.f, sf::Color::Yellow);
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
                            deathEffects.emplace_back(rangedIt->getShape().getPosition() + rangedIt->getShape().getSize() / 2.f, sf::Color::Magenta);
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
            }
            // 修改过关条件：当击杀3只怪物时
            else if (score >= 3) {  // 每关需要击杀3只怪物
                score = 0;  // 重置当前关卡的分数
                if (currentLevel >= MAX_LEVEL) {
                    gameWon = true; // 第三关完成，游戏胜利
                }
                else {
                    nextLevelAvailable = true; // 还有下一关
                }
            }
        }

        // 渲染
        window.clear(); // 清除为黑色背景

        if (!gameOver && !nextLevelAvailable && !gameWon) {
            // 首先绘制当前关卡的地图
            /*if (currentLevel > 0 && static_cast<size_t>(currentLevel) <= maps.size()) {
                maps[currentLevel - 1].draw(window); // currentLevel是1基准, vector是0基准
            }*/

            // 绘制障碍物
            for (const auto& obstacle : obstacles) {
                window.draw(obstacle.getShape());
            }

            // 绘制玩家
            window.draw(player->getSprite());

            // 绘制近战攻击特效
            if (MeleePlayer* meleePlayer = dynamic_cast<MeleePlayer*>(player)) {
                meleePlayer->drawSweepEffect(window);
            }

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

            // 绘制死亡特效
            for (const auto& effect : deathEffects) {
                effect.draw(window);
            }

            // 绘制传送特效
            for (const auto& effect : teleportEffects) {
                effect.draw(window);
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
        else if (gameWon || gameOver) { // 如果游戏结束或胜利
            if (gameWon) {
                window.draw(victoryText);
                window.draw(victoryRestartButton);
                window.draw(victoryRestartText);
                window.draw(quitButton);
                window.draw(quitText);
            }
            else { // gameOver == true
                window.draw(gameOverText);
                window.draw(restartButton);
                window.draw(restartText);
                window.draw(quitButton);
                window.draw(quitText);
            }
        }
        window.display();
    }
    // 释放玩家对象内存
    delete player;
    return 0;
}