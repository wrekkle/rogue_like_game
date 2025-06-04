#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <fstream>
#include <sstream>

// ��ͼ�ߴ� (��Ȼ��������߽����)
const int MAP_WIDTH = 800;
const int MAP_HEIGHT = 800;
const int MAX_LEVEL = 3;

// �浵�ṹ��
struct GameSave {
    int playerType;  // 0: ��ս, 1: Զ��
    int health;
    int currentLevel;
    int score;
    bool exists;     // ��Ǹô浵��λ�Ƿ���ڴ浵

    GameSave() : playerType(0), health(5), currentLevel(1), score(0), exists(false) {}
};

// ��ȡ�浵
std::vector<GameSave> loadSaves() {
    std::vector<GameSave> saves(3);  // 3���浵λ
    for (int i = 0; i < 3; i++) {
        std::stringstream ss;
        ss << "save" << i + 1 << ".dat";
        std::ifstream file(ss.str(), std::ios::binary);
        if (file.is_open()) {
            file.read(reinterpret_cast<char*>(&saves[i]), sizeof(GameSave));
            saves[i].exists = true;
            file.close();
        }
    }
    return saves;
}

// ����浵
void saveGame(const GameSave& save, int slot) {
    std::stringstream ss;
    ss << "save" << slot + 1 << ".dat";
    std::ofstream file(ss.str(), std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(&save), sizeof(GameSave));
        file.close();
    }
}

// ɾ���浵
void deleteSave(int slot) {
    std::stringstream ss;
    ss << "save" << slot + 1 << ".dat";
    std::remove(ss.str().c_str());
}

// ��ײ��⺯��
bool isCollision(const sf::RectangleShape& rect1, const sf::RectangleShape& rect2) {
    return rect1.getGlobalBounds().intersects(rect2.getGlobalBounds());
}

// �ϰ�����
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

// ��������ϰ���ĺ���
std::vector<Obstacle> generateObstacles(int level) {
    std::vector<Obstacle> obstacles;
    int numObstacles = 5 + level * 2; // ÿ�������ϰ�������

    for (int i = 0; i < numObstacles; ++i) {
        float width = 30.0f + (rand() % 70); // 30-100��������
        float height = 30.0f + (rand() % 70); // 30-100������߶�
        float x = rand() % (MAP_WIDTH - static_cast<int>(width));
        float y = rand() % (MAP_HEIGHT - static_cast<int>(height));

        // ������ʱ�ϰ������ڼ��
        Obstacle obstacle(x, y, width, height);

        // ����Ƿ�����ҳ������ص�����������
        sf::RectangleShape spawnArea;
        spawnArea.setSize(sf::Vector2f(100, 100));
        spawnArea.setPosition(350, 350);

        // �����������ص�����������λ��
        if (obstacle.intersects(spawnArea)) {
            --i;
            continue;
        }

        // ����Ƿ��������ϰ����ص�
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
            --i; // ����
        }
    }

    return obstacles;
}

// ����Ƿ����κ��ϰ�����ײ
bool checkObstacleCollision(const sf::RectangleShape& object, const std::vector<Obstacle>& obstacles) {
    for (const auto& obstacle : obstacles) {
        if (obstacle.intersects(object)) {
            return true;
        }
    }
    return false;
}

// ���ɹ�������㣬��֤�����ϰ����ص�
sf::Vector2f generateMonsterSpawn(const sf::Vector2f& size, const std::vector<Obstacle>& obstacles) {
    while (true) {
        float x = rand() % (MAP_WIDTH - static_cast<int>(size.x));
        float y = rand() % (MAP_HEIGHT - static_cast<int>(size.y));
        sf::RectangleShape temp;
        temp.setSize(size);
        temp.setPosition(x, y);
        bool overlap = false;
        for (const auto& obs : obstacles) {
            if (obs.intersects(temp)) {
                overlap = true;
                break;
            }
        }
        if (!overlap) return sf::Vector2f(x, y);
    }
}

// ��һ���
class Player {
public:
    Player() {
        shape.setSize(sf::Vector2f(50, 50));
        shape.setPosition(375, 375);
        shape.setFillColor(sf::Color::Green);  // ������ҷ���Ϊ��ɫ
        health = 5;
        invincibilityFrames = 0;
        shootCooldown = 0;
    }

    virtual ~Player() = default;

    virtual void move(float dx, float dy, const std::vector<Obstacle>& obstacles) {
        sf::Vector2f newPos = shape.getPosition() + sf::Vector2f(dx, dy);
        sf::RectangleShape newShape = shape;
        newShape.setPosition(newPos);

        // �����λ���Ƿ����ϰ�����ײ
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

    void setHealth(int newHealth) {
        health = newHealth;
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

// ��ս�����
class MeleePlayer : public Player {
public:
    MeleePlayer() : Player() {
        if (!texture.loadFromFile("resources/player/l11.png")) {
            std::cerr << "Error: Failed to load melee player texture!" << std::endl;
        }
        sprite.setTexture(texture, true);  // true��ʾ����ԭʼ��ʽ
        sprite.setPosition(shape.getPosition());

        attackRange = 100.0f;  // ���ӹ�����Χ
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
            sweepAngle += 24.f;  // ����ԭ�е���ת�ٶ�

            // ����µ�����
            if (rand() % 2 == 0) {  // 50%�ĸ�������������
                float currentAngle = (-90.f + sweepAngle) * 3.14159f / 180.f;
                sf::Vector2f center = shape.getPosition() + shape.getSize() / 2.f;
                float randRadius = attackRange * (0.6f + (rand() % 40) / 100.0f);  // ��60%-100%��Χ�����

                SweepParticle particle;
                particle.position = center + sf::Vector2f(
                    std::cos(currentAngle) * randRadius,
                    std::sin(currentAngle) * randRadius
                );
                particle.color = sf::Color(100, 200, 255, 255);  // ǳ��ɫ
                particle.lifetime = 10;  // ���ӳ���10֡
                sweepParticles.push_back(particle);
            }

            // ������������
            for (auto& particle : sweepParticles) {
                particle.lifetime--;
                particle.color.a = static_cast<sf::Uint8>((particle.lifetime / 10.0f) * 255);
            }

            // �Ƴ���ʧ������
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
            // ������Ҫ��������Ч
            float playerSize = shape.getSize().x;
            float outerRadius = attackRange;
            float innerRadius = playerSize * 0.8f;
            sf::Vector2f center = shape.getPosition() + shape.getSize() / 2.f;

            // ��������Ч���Ķ������
            for (float r = innerRadius; r <= outerRadius; r += (outerRadius - innerRadius) / 5.0f) {
                int segments = 60;
                float angleStep = sweepAngle / segments;
                sf::VertexArray ring(sf::TriangleStrip, (segments + 1) * 2);

                for (int i = 0; i <= segments; ++i) {
                    float angle = -90.f + i * angleStep;
                    float rad = angle * 3.14159f / 180.f;
                    float alpha = 1.0f - ((r - innerRadius) / (outerRadius - innerRadius));
                    sf::Uint8 alphaValue = static_cast<sf::Uint8>(alpha * 128);  // ���͸���Ƚ��͵�128

                    // ��Ȧ����
                    ring[i * 2].position = center + sf::Vector2f(std::cos(rad) * (r + 5), std::sin(rad) * (r + 5));
                    ring[i * 2].color = sf::Color(100, 200, 255, alphaValue);  // ǳ��ɫ

                    // ��Ȧ����
                    ring[i * 2 + 1].position = center + sf::Vector2f(std::cos(rad) * r, std::sin(rad) * r);
                    ring[i * 2 + 1].color = sf::Color(100, 200, 255, alphaValue);
                }
                window.draw(ring);
            }

            // ��������
            for (const auto& particle : sweepParticles) {
                sf::CircleShape particleShape(3);  // ���Ӵ�СΪ3����
                particleShape.setFillColor(particle.color);
                particleShape.setPosition(particle.position);
                particleShape.setOrigin(1.5f, 1.5f);  // ����ԭ��Ϊ����
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

    // ���ӽṹ
    struct SweepParticle {
        sf::Vector2f position;
        sf::Color color;
        int lifetime;
    };
    std::vector<SweepParticle> sweepParticles;
};

// Զ�������
class RangedPlayer : public Player {
public:
    RangedPlayer() : Player() {
        if (!texture.loadFromFile("resources/player/tales1.png")) {
            std::cerr << "Error: Failed to load ranged player texture!" << std::endl;
        }
        sprite.setTexture(texture, true);  // true��ʾ����ԭʼ��ʽ
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

// ��ս�������
class MeleeMonster {
public:
    MeleeMonster(const std::vector<Obstacle>& obstacles) {
        shape.setSize(sf::Vector2f(30, 30));
        shape.setPosition(generateMonsterSpawn(shape.getSize(), obstacles));
    }

    // ���ĳ�������Ƿ�����ƶ�
    bool canMove(const sf::Vector2f& direction, const std::vector<Obstacle>& obstacles) {
        sf::Vector2f newPos = shape.getPosition() + direction;
        sf::RectangleShape newShape = shape;
        newShape.setPosition(newPos);

        return !checkObstacleCollision(newShape, obstacles) &&
            newPos.x >= 0 && newPos.x + shape.getSize().x <= static_cast<float>(MAP_WIDTH) &&
            newPos.y >= 0 && newPos.y + shape.getSize().y <= static_cast<float>(MAP_HEIGHT);
    }

    // Ѱ������ƶ�����
    sf::Vector2f findAlternativeDirection(const sf::Vector2f& originalDir, const std::vector<Obstacle>& obstacles) {
        // ����8����ͬ�ķ���
        const float angles[8] = { 45, -45, 90, -90, 135, -135, 180, 0 };  // �Ƕ�

        for (float angle : angles) {
            // ���Ƕ�ת��Ϊ����
            float radian = angle * 3.14159f / 180.0f;

            // �����µķ���
            sf::Vector2f newDir;
            newDir.x = std::cos(radian) * originalDir.x - std::sin(radian) * originalDir.y;
            newDir.y = std::sin(radian) * originalDir.x + std::cos(radian) * originalDir.y;

            // ��׼����������
            float length = std::sqrt(newDir.x * newDir.x + newDir.y * newDir.y);
            if (length > 0) {
                newDir /= length;
            }

            // ����·����Ƿ����
            if (canMove(newDir * 5.0f, obstacles)) {
                return newDir;
            }
        }

        return sf::Vector2f(0, 0); // ���û���ҵ����з��򣬷���������
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
                // ���û�������ϰ�������ƶ�
                if (newPos.x >= 0 && newPos.x + shape.getSize().x <= static_cast<float>(MAP_WIDTH)) {
                    shape.move(direction.x * 1.0f, 0);
                }
                if (newPos.y >= 0 && newPos.y + shape.getSize().y <= static_cast<float>(MAP_HEIGHT)) {
                    shape.move(0, direction.y * 1.0f);
                }
            }
            else {
                // ��������ϰ��Ѱ�����·��
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

// ������Ч��
class TeleportEffect {
public:
    TeleportEffect(const sf::Vector2f& position) {
        this->position = position;
        timer = 0;
        maxTimer = 20; // 20֡����ʱ��

        // �����������
        for (int i = 0; i < 12; ++i) {
            sf::CircleShape particle(2);
            particle.setFillColor(sf::Color::Cyan);
            particle.setPosition(position);
            particles.push_back(particle);

            // �������ӵĳ�ʼ�ٶȣ�������ɢ��Բ��pattern��
            float angle = (i * 30.0f) * 3.14159f / 180.0f; // ÿ30��һ������
            sf::Vector2f velocity;
            velocity.x = std::cos(angle) * 3.0f;
            velocity.y = std::sin(angle) * 3.0f;
            velocities.push_back(velocity);
        }
    }

    bool update() {
        timer++;

        // ��������λ�ú�͸����
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

// ��ɫ��ս����
class BlueMeleeMonster : public MeleeMonster {
public:
    BlueMeleeMonster(const std::vector<Obstacle>& obstacles) : MeleeMonster(obstacles) {
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

                // ��鴫��Ŀ��λ���Ƿ����ϰ�����ײ
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

// ��ɫ��ս����
class RedMeleeMonster : public MeleeMonster {
public:
    RedMeleeMonster(const std::vector<Obstacle>& obstacles) : MeleeMonster(obstacles) {
        shape.setFillColor(sf::Color::Red);
    }
};

// ��ɫ��ս����
class YellowMeleeMonster : public MeleeMonster {
public:
    YellowMeleeMonster(const std::vector<Obstacle>& obstacles) : MeleeMonster(obstacles) {
        shape.setFillColor(sf::Color::Yellow);
    }
};

// ������Ч��
class DeathEffect {
public:
    DeathEffect(const sf::Vector2f& position, const sf::Color& color) {
        this->position = position;
        this->color = color;
        timer = 0;
        maxTimer = 30; // 30֡����ʱ��

        // �����������
        for (int i = 0; i < 8; ++i) {
            sf::CircleShape particle(3);
            particle.setFillColor(color);
            particle.setPosition(position);
            particles.push_back(particle);

            // ����ٶȷ���
            float angle = (i * 45.0f) * 3.14159f / 180.0f; // ÿ45��һ������
            sf::Vector2f velocity;
            velocity.x = std::cos(angle) * (2.0f + rand() % 3);
            velocity.y = std::sin(angle) * (2.0f + rand() % 3);
            velocities.push_back(velocity);
        }
    }

    bool update() {
        timer++;

        // ��������λ�ú�͸����
        float alpha = 1.0f - (static_cast<float>(timer) / maxTimer);
        sf::Uint8 alphaValue = static_cast<sf::Uint8>(alpha * 255);

        for (size_t i = 0; i < particles.size(); ++i) {
            particles[i].move(velocities[i]);
            sf::Color particleColor = color;
            particleColor.a = alphaValue;
            particles[i].setFillColor(particleColor);

            // ����Ч��
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

// �ӵ��� (�޸Ķ�)
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
            velocity = direction * 5.0f; // �ӵ��ٶ�
        }
        this->isPlayerBullet = isPlayerBullet;
    }

    bool move(const std::vector<Obstacle>& obstacles) {
        sf::Vector2f newPos = shape.getPosition() + velocity;
        sf::RectangleShape newShape = shape;
        newShape.setPosition(newPos);

        // ����Ƿ����ϰ�����ײ
        if (checkObstacleCollision(newShape, obstacles)) {
            return false; // �ӵ������ϰ������false��ʾ��Ҫɾ��
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

// Զ�̹�����
class RangedMonster {
public:
    RangedMonster(const std::vector<Obstacle>& obstacles) {
        shape.setSize(sf::Vector2f(30, 30));
        shape.setPosition(generateMonsterSpawn(shape.getSize(), obstacles));
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

// ���¿�ʼ��Ϸ
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
    // ���������ϰ���
    obstacles = generateObstacles(currentLevel);
}

// ������һ��
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

    // �����µ��ϰ���
    obstacles = generateObstacles(currentLevel);

    int baseCount = currentLevel + 1;
    for (int i = 0; i < baseCount; ++i) {
        blueMonsters.emplace_back(obstacles);
    }
    for (int i = 0; i < baseCount; ++i) {
        redMonsters.emplace_back(obstacles);
    }
    for (int i = 0; i < baseCount; ++i) {
        yellowMonsters.emplace_back(obstacles);
    }
    for (int i = 0; i < baseCount; ++i) {
        rangedMonsters.emplace_back(obstacles);
    }
}

// �������ϵͳ��
class ParticleSystem {
public:
    struct Particle {
        sf::Vector2f position;
        sf::Vector2f velocity;
        sf::Color color;
        float lifetime;
        float maxLifetime;
        float size;
    };

    ParticleSystem(int maxParticles) : maxParticles(maxParticles) {
        particles.reserve(maxParticles);
    }

    void addParticle(const sf::Vector2f& position) {
        if (particles.size() >= maxParticles) return;

        Particle p;
        p.position = position;

        // ����ٶ�
        float angle = static_cast<float>(rand() % 360) * 3.14159f / 180.f;
        float speed = 0.5f + static_cast<float>(rand() % 100) / 100.f * 2.0f;
        p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);

        // �����ɫ - ʹ����ɫ����ɫ�Ľ���
        int r = 50 + rand() % 100;
        int g = 50 + rand() % 150;
        int b = 200 + rand() % 55;
        p.color = sf::Color(r, g, b, 200);

        // �����������
        p.maxLifetime = 3.0f + static_cast<float>(rand() % 200) / 100.f;
        p.lifetime = p.maxLifetime;

        // �����С
        p.size = 1.0f + static_cast<float>(rand() % 30) / 10.f;

        particles.push_back(p);
    }

    void update(float deltaTime) {
        for (auto it = particles.begin(); it != particles.end();) {
            it->lifetime -= deltaTime;
            if (it->lifetime <= 0) {
                it = particles.erase(it);
            }
            else {
                it->position += it->velocity;

                // ��������뿪��Ļ����������һ�߳���
                if (it->position.x < 0) it->position.x = MAP_WIDTH;
                if (it->position.x > MAP_WIDTH) it->position.x = 0;
                if (it->position.y < 0) it->position.y = MAP_HEIGHT;
                if (it->position.y > MAP_HEIGHT) it->position.y = 0;

                // �����������ڼ��٣�����͸����
                float alpha = it->lifetime / it->maxLifetime;
                it->color.a = static_cast<sf::Uint8>(alpha * 200);

                ++it;
            }
        }

        // �������������
        if (rand() % 5 == 0 && particles.size() < maxParticles) {
            float x = static_cast<float>(rand() % MAP_WIDTH);
            float y = static_cast<float>(rand() % MAP_HEIGHT);
            addParticle(sf::Vector2f(x, y));
        }
    }

    void draw(sf::RenderWindow& window) const {
        for (const auto& p : particles) {
            sf::CircleShape shape(p.size);
            shape.setFillColor(p.color);
            shape.setPosition(p.position);
            shape.setOrigin(p.size, p.size);
            window.draw(shape);
        }
    }

private:
    std::vector<Particle> particles;
    int maxParticles;
};

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    // ��������
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "ERROR: Failed to load font!" << std::endl;
    }

    sf::RenderWindow window(sf::VideoMode(MAP_WIDTH, MAP_HEIGHT), "2D Game - Save Selection");
    window.setFramerateLimit(60);

    // ��������ϵͳ
    ParticleSystem particleSystem(300);
    sf::Clock particleClock;

    // ��ʼ��һЩ����
    for (int i = 0; i < 100; ++i) {
        float x = static_cast<float>(rand() % MAP_WIDTH);
        float y = static_cast<float>(rand() % MAP_HEIGHT);
        particleSystem.addParticle(sf::Vector2f(x, y));
    }

    // UIԪ��
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
    restartButton.setPosition(MAP_WIDTH / 2 - 250, MAP_HEIGHT / 2 + 100);

    sf::Text restartText;
    restartText.setFont(font);
    restartText.setCharacterSize(24);
    restartText.setFillColor(sf::Color::White);
    restartText.setString("Restart");
    restartText.setPosition(MAP_WIDTH / 2 - 250 + 100 - restartText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 115);

    // �˳���ť
    sf::RectangleShape quitButton(sf::Vector2f(200, 50));
    quitButton.setFillColor(sf::Color::Red);
    quitButton.setPosition(MAP_WIDTH / 2 + 50, MAP_HEIGHT / 2 + 100);

    sf::Text quitText;
    quitText.setFont(font);
    quitText.setCharacterSize(24);
    quitText.setFillColor(sf::Color::White);
    quitText.setString("Quit");
    quitText.setPosition(MAP_WIDTH / 2 + 50 + 100 - quitText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 115);

    // ��һ�����UI
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

    // ʤ�����UI
    sf::Text victoryText;
    victoryText.setFont(font);
    victoryText.setCharacterSize(48);
    victoryText.setFillColor(sf::Color::Red);
    victoryText.setString("Victory!");
    victoryText.setPosition(MAP_WIDTH / 2 - victoryText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 - 50);

    sf::RectangleShape victoryRestartButton(sf::Vector2f(200, 50));
    victoryRestartButton.setFillColor(sf::Color::Green);
    victoryRestartButton.setPosition(MAP_WIDTH / 2 - 250, MAP_HEIGHT / 2 + 100);

    sf::Text victoryRestartText;
    victoryRestartText.setFont(font);
    victoryRestartText.setCharacterSize(24);
    victoryRestartText.setFillColor(sf::Color::White);
    victoryRestartText.setString("Restart");
    victoryRestartText.setPosition(MAP_WIDTH / 2 - 250 + 100 - victoryRestartText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 115);

    // ��ͣ�˵�UI
    sf::RectangleShape pauseButton(sf::Vector2f(40, 40));
    pauseButton.setFillColor(sf::Color(100, 100, 100, 200));
    pauseButton.setPosition(MAP_WIDTH - 50, 10);

    sf::RectangleShape pauseMenuBg(sf::Vector2f(300, 200));
    pauseMenuBg.setFillColor(sf::Color(50, 50, 50, 230));
    pauseMenuBg.setPosition(MAP_WIDTH / 2 - 150, MAP_HEIGHT / 2 - 100);

    sf::Text pauseTitle;
    pauseTitle.setFont(font);
    pauseTitle.setCharacterSize(32);
    pauseTitle.setFillColor(sf::Color::White);
    pauseTitle.setString("Paused");
    pauseTitle.setPosition(MAP_WIDTH / 2 - pauseTitle.getGlobalBounds().width / 2, MAP_HEIGHT / 2 - 80);

    sf::RectangleShape continueButton(sf::Vector2f(200, 50));
    continueButton.setFillColor(sf::Color::Green);
    continueButton.setPosition(MAP_WIDTH / 2 - 100, MAP_HEIGHT / 2 - 30);

    sf::Text continueText;
    continueText.setFont(font);
    continueText.setCharacterSize(24);
    continueText.setFillColor(sf::Color::White);
    continueText.setString("Continue");
    continueText.setPosition(MAP_WIDTH / 2 - continueText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 - 20);

    sf::RectangleShape exitButton(sf::Vector2f(200, 50));
    exitButton.setFillColor(sf::Color::Red);
    exitButton.setPosition(MAP_WIDTH / 2 - 100, MAP_HEIGHT / 2 + 30);

    sf::Text exitText;
    exitText.setFont(font);
    exitText.setCharacterSize(24);
    exitText.setFillColor(sf::Color::White);
    exitText.setString("Exit");
    exitText.setPosition(MAP_WIDTH / 2 - exitText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 40);

    // ��ͣ��ťͼ��
    sf::RectangleShape pauseIcon1(sf::Vector2f(5, 20));
    pauseIcon1.setFillColor(sf::Color::White);

    sf::RectangleShape pauseIcon2(sf::Vector2f(5, 20));
    pauseIcon2.setFillColor(sf::Color::White);

    // �浵ѡ�����Ԫ��
    sf::Text saveSelectTitle;
    saveSelectTitle.setFont(font);
    saveSelectTitle.setCharacterSize(48);
    saveSelectTitle.setFillColor(sf::Color::White);
    saveSelectTitle.setString("Select Save");
    saveSelectTitle.setPosition(MAP_WIDTH / 2 - saveSelectTitle.getGlobalBounds().width / 2, 100);

    std::vector<sf::RectangleShape> saveSlots(3);
    std::vector<sf::Text> saveTexts(3);
    std::vector<sf::RectangleShape> deleteButtons(3);  // ɾ����ť
    std::vector<sf::Text> deleteTexts(3);  // ɾ����ť����
    for (int i = 0; i < 3; i++) {
        saveSlots[i].setSize(sf::Vector2f(300, 80));
        saveSlots[i].setFillColor(sf::Color(100, 100, 100));
        saveSlots[i].setPosition(MAP_WIDTH / 2 - 150, 200 + i * 120);

        saveTexts[i].setFont(font);
        saveTexts[i].setCharacterSize(24);
        saveTexts[i].setFillColor(sf::Color::White);
        saveTexts[i].setPosition(MAP_WIDTH / 2 - 140, 220 + i * 120);

        // ��ʼ��ɾ����ť
        deleteButtons[i].setSize(sf::Vector2f(60, 30));
        deleteButtons[i].setFillColor(sf::Color::Red);
        deleteButtons[i].setPosition(MAP_WIDTH / 2 + 160, 225 + i * 120);  // ���ڴ浵��λ�Ҳ�

        deleteTexts[i].setFont(font);
        deleteTexts[i].setCharacterSize(16);
        deleteTexts[i].setFillColor(sf::Color::White);
        deleteTexts[i].setString("Delete");
        deleteTexts[i].setPosition(MAP_WIDTH / 2 + 165, 230 + i * 120);
    }

    // �浵��ť
    sf::RectangleShape saveButton(sf::Vector2f(200, 50));
    saveButton.setFillColor(sf::Color::White);
    saveButton.setPosition(MAP_WIDTH / 2 - 100, MAP_HEIGHT / 2 + 90);  // ����λ�õ�Continue��Exit��ť�·�

    sf::Text saveButtonText;
    saveButtonText.setFont(font);
    saveButtonText.setCharacterSize(24);
    saveButtonText.setFillColor(sf::Color::Black);
    saveButtonText.setString("Save");
    saveButtonText.setPosition(MAP_WIDTH / 2 - saveButtonText.getGlobalBounds().width / 2, MAP_HEIGHT / 2 + 100);

    // ��ɫѡ�����Ԫ��
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

    // ��ս��ɫѡ��
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

    // Զ�̽�ɫѡ��
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

    // ���ʤ�����汳��ͼƬ
    sf::Texture victoryBgTexture;
    sf::Sprite victoryBgSprite;
    if (!victoryBgTexture.loadFromFile("resources/player/victory.png")) {
        std::cerr << "Error: Failed to load victory background texture!" << std::endl;
    }
    else {
        victoryBgSprite.setTexture(victoryBgTexture);
        // ��������ͼƬ��С����Ӧ����
        float scaleX = static_cast<float>(MAP_WIDTH) / victoryBgTexture.getSize().x;
        float scaleY = static_cast<float>(MAP_HEIGHT) / victoryBgTexture.getSize().y;
        victoryBgSprite.setScale(scaleX, scaleY);
    }

    // ���ʧ�ܽ��汳��ͼƬ
    sf::Texture loseBgTexture;
    sf::Sprite loseBgSprite;
    if (!loseBgTexture.loadFromFile("resources/player/lose.png")) {
        std::cerr << "Error: Failed to load lose background texture!" << std::endl;
    }
    else {
        loseBgSprite.setTexture(loseBgTexture);
        // ��������ͼƬ��С����Ӧ����
        float scaleX = static_cast<float>(MAP_WIDTH) / loseBgTexture.getSize().x;
        float scaleY = static_cast<float>(MAP_HEIGHT) / loseBgTexture.getSize().y;
        loseBgSprite.setScale(scaleX, scaleY);
    }

    // ��ɫѡ��������
    sf::Text characterSelectTitle;
    characterSelectTitle.setFont(font);
    characterSelectTitle.setCharacterSize(48);
    characterSelectTitle.setFillColor(sf::Color::White);
    characterSelectTitle.setString("Select your character!");
    characterSelectTitle.setPosition(MAP_WIDTH / 2 - characterSelectTitle.getGlobalBounds().width / 2, 100);

    // ��Ϸ״̬
    bool inSaveSelection = true;  // �Ƿ��ڴ浵ѡ�����
    std::vector<GameSave> saves = loadSaves();
    int currentSaveSlot = -1;    // ��ǰʹ�õĴ浵��λ

    // ��Ϸ����
    Player* player = nullptr;
    std::vector<BlueMeleeMonster> blueMonsters;
    std::vector<RedMeleeMonster> redMonsters;
    std::vector<YellowMeleeMonster> yellowMonsters;
    std::vector<RangedMonster> rangedMonsters;
    std::vector<Bullet> bullets;
    std::vector<DeathEffect> deathEffects;
    std::vector<TeleportEffect> teleportEffects;  // ��Ӵ�����Ч����
    int score = 0;
    int currentLevel = 1;
    std::vector<Obstacle> obstacles;

    // ��Ϸ״̬
    bool gameOver = false;
    bool nextLevelAvailable = false;
    bool gameWon = false;
    bool needCharacterSelection = true;  // ��ʼ״̬��Ҫѡ���ɫ
    bool gamePaused = false;  // �����ͣ״̬����

    // ����Ϸ��ʼʱ���ɵ�һ�ص��ϰ���
    obstacles = generateObstacles(currentLevel);

    // ��Ϸ��ѭ��
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                if (inSaveSelection) {
                    // ����浵ѡ�����ĵ��
                    for (int i = 0; i < 3; i++) {
                        if (saves[i].exists && deleteButtons[i].getGlobalBounds().contains(mousePos)) {
                            // ɾ���浵
                            deleteSave(i);
                            // ���¼������д浵����
                            saves = loadSaves();
                            continue;  // �����浵��λ�ĵ�����
                        }
                        if (saveSlots[i].getGlobalBounds().contains(mousePos)) {
                            currentSaveSlot = i;
                            if (saves[i].exists) {
                                // �������д浵
                                currentLevel = saves[i].currentLevel;
                                score = saves[i].score;
                                if (saves[i].playerType == 0) {
                                    player = new MeleePlayer();
                                }
                                else {
                                    player = new RangedPlayer();
                                }
                                player->setHealth(saves[i].health);
                                inSaveSelection = false;
                                needCharacterSelection = false;
                                obstacles = generateObstacles(currentLevel);

                                // ��ʼ������
                                int baseCount = currentLevel + 1;
                                for (int i = 0; i < baseCount; ++i) {
                                    blueMonsters.emplace_back(obstacles);
                                }
                                for (int i = 0; i < baseCount; ++i) {
                                    redMonsters.emplace_back(obstacles);
                                }
                                for (int i = 0; i < baseCount; ++i) {
                                    yellowMonsters.emplace_back(obstacles);
                                }
                                for (int i = 0; i < baseCount; ++i) {
                                    rangedMonsters.emplace_back(obstacles);
                                }
                            }
                            else {
                                // �մ浵�������ɫѡ�����
                                inSaveSelection = false;
                                needCharacterSelection = true;
                                currentLevel = 1;
                                score = 0;
                                obstacles = generateObstacles(currentLevel);
                                // �ڴ����´浵���������¼��ش浵����
                                saves = loadSaves();
                            }
                        }
                    }
                }
                else if (needCharacterSelection) {
                    // �����ɫѡ�����ĵ��
                    if (meleeSprite.getGlobalBounds().contains(mousePos)) {
                        player = new MeleePlayer();
                        needCharacterSelection = false;
                        // ��ʼ����һ�صĹ���
                        for (int i = 0; i < 2; ++i) {
                            blueMonsters.emplace_back(obstacles);
                        }
                        for (int i = 0; i < 2; ++i) {
                            redMonsters.emplace_back(obstacles);
                        }
                        for (int i = 0; i < 2; ++i) {
                            yellowMonsters.emplace_back(obstacles);
                        }
                        for (int i = 0; i < 2; ++i) {
                            rangedMonsters.emplace_back(obstacles);
                        }
                        // �����´浵
                        GameSave save;
                        save.playerType = 0;  // ��ս
                        save.health = player->getHealth();
                        save.currentLevel = currentLevel;
                        save.score = score;
                        save.exists = true;
                        saveGame(save, currentSaveSlot);
                    }
                    else if (rangedSprite.getGlobalBounds().contains(mousePos)) {
                        player = new RangedPlayer();
                        needCharacterSelection = false;
                        // ��ʼ����һ�صĹ���
                        for (int i = 0; i < 2; ++i) {
                            blueMonsters.emplace_back(obstacles);
                        }
                        for (int i = 0; i < 2; ++i) {
                            redMonsters.emplace_back(obstacles);
                        }
                        for (int i = 0; i < 2; ++i) {
                            yellowMonsters.emplace_back(obstacles);
                        }
                        for (int i = 0; i < 2; ++i) {
                            rangedMonsters.emplace_back(obstacles);
                        }
                        // �����´浵
                        GameSave save;
                        save.playerType = 1;  // Զ��
                        save.health = player->getHealth();
                        save.currentLevel = currentLevel;
                        save.score = score;
                        save.exists = true;
                        saveGame(save, currentSaveSlot);
                    }
                }
                else if (!gameOver && !nextLevelAvailable && !gameWon && !gamePaused) {
                    // ��Ϸ�����еĵ������
                    if (pauseButton.getGlobalBounds().contains(mousePos)) {
                        gamePaused = true;
                    }
                    else if (dynamic_cast<MeleePlayer*>(player) && player->canShoot()) {
                        static_cast<MeleePlayer*>(player)->startSweep();
                        player->setShootCooldown();
                    }
                    else if (player->canShoot()) {
                        sf::Vector2f playerCenter = player->getShape().getPosition() + sf::Vector2f(player->getShape().getSize().x / 2, player->getShape().getSize().y / 2);
                        bullets.emplace_back(playerCenter, mousePos, true);
                        player->setShootCooldown();
                    }
                }
                else if (gamePaused) {
                    if (continueButton.getGlobalBounds().contains(mousePos)) {
                        gamePaused = false;
                    }
                    else if (saveButton.getGlobalBounds().contains(mousePos)) {
                        if (currentSaveSlot >= 0) {
                            // ������Ϸ״̬
                            GameSave save;
                            save.playerType = dynamic_cast<MeleePlayer*>(player) ? 0 : 1;
                            save.health = player->getHealth();
                            save.currentLevel = currentLevel;
                            save.score = score;
                            save.exists = true;
                            saveGame(save, currentSaveSlot);

                            // ���¼������д浵����
                            saves = loadSaves();

                            // ���ش浵ѡ�����
                            inSaveSelection = true;
                            gamePaused = false;
                        }
                    }
                    else if (exitButton.getGlobalBounds().contains(mousePos)) {
                        // ���ش浵ѡ�����
                        delete player;
                        player = nullptr;
                        blueMonsters.clear();
                        redMonsters.clear();
                        yellowMonsters.clear();
                        rangedMonsters.clear();
                        bullets.clear();
                        deathEffects.clear();
                        teleportEffects.clear();
                        score = 0;
                        currentLevel = 1;
                        obstacles = generateObstacles(currentLevel);
                        gameOver = false;
                        nextLevelAvailable = false;
                        gameWon = false;
                        needCharacterSelection = true;
                        gamePaused = false;
                        inSaveSelection = true;
                    }
                }
                else if (nextLevelAvailable && nextLevelButton.getGlobalBounds().contains(mousePos)) {
                    nextLevel(player, blueMonsters, redMonsters, yellowMonsters, rangedMonsters, score, currentLevel, obstacles);
                    bullets.clear();
                    deathEffects.clear();
                    teleportEffects.clear();
                    nextLevelAvailable = false;
                }
                else if (gameWon || gameOver) {
                    if ((gameOver && restartButton.getGlobalBounds().contains(mousePos)) ||
                        (gameWon && victoryRestartButton.getGlobalBounds().contains(mousePos))) {
                        restartGame(player, blueMonsters, redMonsters, yellowMonsters, rangedMonsters, score, currentLevel, obstacles);
                        bullets.clear();
                        deathEffects.clear();
                        teleportEffects.clear();
                        gameOver = false;
                        gameWon = false;
                        needCharacterSelection = true;
                        inSaveSelection = true;
                    }
                    else if (quitButton.getGlobalBounds().contains(mousePos)) {
                        window.close();
                    }
                }
            }
        }

        window.clear(sf::Color::Black);

        if (inSaveSelection) {
            // ��������ϵͳ
            particleSystem.update(particleClock.restart().asSeconds());

            // ��Ⱦ���ӱ���
            window.clear(sf::Color(10, 10, 40)); // ����ɫ����
            particleSystem.draw(window);

            // ��Ⱦ�浵ѡ�����
            window.draw(saveSelectTitle);
            for (int i = 0; i < 3; i++) {
                window.draw(saveSlots[i]);
                if (saves[i].exists) {
                    std::stringstream ss;
                    ss << "Save " << i + 1;
                    saveTexts[i].setString(ss.str());
                    // ֻΪ���д浵��ʾɾ����ť
                    window.draw(deleteButtons[i]);
                    window.draw(deleteTexts[i]);
                }
                else {
                    saveTexts[i].setString("Empty Save Slot " + std::to_string(i + 1));
                }
                window.draw(saveTexts[i]);
            }
        }
        else if (needCharacterSelection) {
            // ��Ⱦ��ɫѡ�����
            if (bgClock.getElapsedTime().asSeconds() > 1.5f) {
                bgIndex = (bgIndex + 1) % 4;
                bgClock.restart();
            }
            if (bgLoaded[bgIndex]) window.draw(bgSprites[bgIndex]);

            // ��ӱ���
            window.draw(characterSelectTitle);

            window.draw(meleeSprite);
            window.draw(meleeText);
            window.draw(meleeDesc);
            window.draw(rangedSprite);
            window.draw(rangedText);
            window.draw(rangedDesc);
        }
        else if (!gameOver && !nextLevelAvailable && !gameWon) {
            // ��Ⱦ��Ϸ����
            for (const auto& obstacle : obstacles) {
                window.draw(obstacle.getShape());
            }

            window.draw(player->getSprite());

            if (MeleePlayer* meleePlayer = dynamic_cast<MeleePlayer*>(player)) {
                meleePlayer->drawSweepEffect(window);
            }

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

            for (const auto& bullet : bullets) {
                window.draw(bullet.getShape());
            }

            for (const auto& effect : deathEffects) {
                effect.draw(window);
            }

            for (const auto& effect : teleportEffects) {
                effect.draw(window);
            }

            window.draw(healthText);
            window.draw(scoreText);
            window.draw(levelText);

            window.draw(pauseButton);
            pauseIcon1.setPosition(pauseButton.getPosition().x + 12, pauseButton.getPosition().y + 10);
            pauseIcon2.setPosition(pauseButton.getPosition().x + 23, pauseButton.getPosition().y + 10);
            window.draw(pauseIcon1);
            window.draw(pauseIcon2);

            if (gamePaused) {
                window.draw(pauseMenuBg);
                window.draw(pauseTitle);
                window.draw(continueButton);
                window.draw(continueText);
                window.draw(exitButton);
                window.draw(exitText);
                window.draw(saveButton);
                window.draw(saveButtonText);
            }
        }
        else if (nextLevelAvailable) {
            window.draw(nextLevelText);
            window.draw(nextLevelButton);
            window.draw(nextLevelConfirmText);
        }
        else if (gameWon || gameOver) {
            if (gameWon) {
                // �Ȼ���ʤ������
                window.draw(victoryBgSprite);

                window.draw(victoryText);
                window.draw(victoryRestartButton);
                window.draw(victoryRestartText);
            }
            else {
                // �Ȼ���ʧ�ܱ���
                window.draw(loseBgSprite);

                window.draw(restartButton);
                window.draw(restartText);
            }
            window.draw(quitButton);
            window.draw(quitText);
        }

        window.display();

        // ��Ϸ�߼�����
        if (!gameOver && !nextLevelAvailable && !gameWon && !gamePaused && !needCharacterSelection && !inSaveSelection) {
            // ����ƶ�
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

            // ��ɨ�����뷶Χ�˺�
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

            // ����������Ч
            for (auto effectIt = deathEffects.begin(); effectIt != deathEffects.end();) {
                if (!effectIt->update()) {
                    effectIt = deathEffects.erase(effectIt);
                }
                else {
                    ++effectIt;
                }
            }

            // ���´�����Ч
            for (auto effectIt = teleportEffects.begin(); effectIt != teleportEffects.end();) {
                if (!effectIt->update()) {
                    effectIt = teleportEffects.erase(effectIt);
                }
                else {
                    ++effectIt;
                }
            }

            // �����ƶ�
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

            // �ӵ��ƶ�����ײ���
            for (auto it = bullets.begin(); it != bullets.end();) {
                bool bulletSurvived = it->move(obstacles);
                bool bulletHit = false;

                if (!bulletSurvived) {
                    it = bullets.erase(it);
                    continue;
                }

                // ����ӵ����й���
                if (it->isFromPlayer()) {
                    // �����ɫ����
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
                    if (bulletHit) { it = bullets.erase(it); continue; }

                    // ����ɫ����
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

                    // ����ɫ����
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

                    // ���Զ�̹���
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
                // �����ӵ��������
                else if (isCollision(player->getShape(), it->getShape())) {
                    player->reduceHealth();
                    bulletHit = true;
                }

                // �Ƴ���������Ŀ����ӵ�
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

            // ��������������ײ
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

            // ����UI�ı�
            healthText.setString("Health: " + std::to_string(player->getHealth()));
            scoreText.setString("Score: " + std::to_string(score));
            levelText.setString("Level: " + std::to_string(currentLevel) + "/" + std::to_string(MAX_LEVEL));

            // �����Ϸ״̬
            if (player->getHealth() <= 0) {
                gameOver = true;
            }
            else if (score >= 3) {  // ÿ����Ҫ��ɱ3ֻ����
                score = 0;  // ���õ�ǰ�ؿ��ķ���
                if (currentLevel >= MAX_LEVEL) {
                    gameWon = true; // ��������ɣ���Ϸʤ��
                }
                else {
                    nextLevelAvailable = true; // ������һ��
                }
            }
        }
    }

    delete player;
    return 0;
}