#ifndef ENEMY_H
#define ENEMY_H

#include "game.h"
#include <vector>
#include <memory>

class Bullet;

class Enemy {
public:
    Enemy(EnemyType type, int level);
    virtual ~Enemy() = default;

    virtual void update(const sf::Vector2f& playerPos, std::vector<std::unique_ptr<Bullet>>& bullets);
    virtual void draw(sf::RenderWindow& window) const;
    virtual void attack(const sf::Vector2f& playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) = 0;

    void takeDamage(int damage);
    bool isAlive() const { return health > 0; }
    sf::Shape* getShape() { return shape.get(); }
    int getPoints() const { return points; }
    int getLevel() const { return level; }

protected:
    // ��״�����Լ���������
    std::unique_ptr<sf::Shape> shape;
    EnemyType enemyType;
    int health;
    float speed;
    float attackCooldown;
    float lastAttackTime;
    int points;
    int level;

    // ͨ�÷�����������ɫ
    void setShapeColor(int level);
    std::unique_ptr<Bullet> createBullet(const sf::Vector2f& position, const sf::Vector2f& direction, float speed, float damage, const sf::Color& color);
};

class TriangleEnemy : public Enemy {
public:
    TriangleEnemy(int level);
    void attack(const sf::Vector2f& playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) override;
    void update(const sf::Vector2f& playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) override;

private:
    enum class State { IDLE, CHARGING, DASHING };
    State state;
    float chargeTime;
    float dashSpeed;
    float dashDuration;
    sf::Vector2f dashDirection;

    // ����״̬�����߼�
    void idleState(const sf::Vector2f& playerPos, std::vector<std::unique_ptr<Bullet>>& bullets);
    void chargingState();
    void dashingState();
};

class HexagonEnemy : public Enemy {
public:
    HexagonEnemy(int level);
    void attack(const sf::Vector2f& playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) override;

private:
    float rotationSpeed;
};

class PentagonEnemy : public Enemy {
public:
    PentagonEnemy(int level);
    void attack(const sf::Vector2f& playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) override;

private:
    float rotationSpeed;
};

#endif // ENEMY_H
