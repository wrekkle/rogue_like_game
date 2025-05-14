#include "enemy.h"
#include "bullet.h"
#include <cmath>
#include <memory>

const float PI = 3.14159265359f;

// ����Enemyʵ��
Enemy::Enemy(EnemyType type, int level)
    : enemyType(type), health(1), speed(2.0f), attackCooldown(60), lastAttackTime(0), points(10), level(level) {
    // ���ݹؿ���������
    health += level;
    speed += level * 0.2f;
    points += level * 5;
}

void Enemy::update(const sf::Vector2f& playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    // ��������ᱻ����������������
}

void Enemy::draw(sf::RenderWindow& window) const {
    window.draw(*shape);
}

void Enemy::takeDamage(int damage) {
    health -= damage;
}

void Enemy::setShapeColor(int level) {
    // ���ݹؿ�������ɫ
    if (level <= 5) shape->setFillColor(sf::Color::Red);
    else if (level <= 10) shape->setFillColor(sf::Color::Magenta);
    else shape->setFillColor(sf::Color::White);
}

std::unique_ptr<Bullet> Enemy::createBullet(const sf::Vector2f& position, const sf::Vector2f& direction, float speed, float damage, const sf::Color& color) {
    return std::make_unique<Bullet>(position, direction, speed, damage, color, true);
}

// TriangleEnemyʵ��
TriangleEnemy::TriangleEnemy(int level) : Enemy(EnemyType::TRIANGLE, level), state(State::IDLE), chargeTime(0), dashSpeed(8.0f), dashDuration(0) {
    shape = std::make_unique<sf::CircleShape>(15, 3); // ������
    health = 3 + level * 2;
    speed = 1.5f + level * 0.1f;
    attackCooldown = 120 - level * 5; // �������
    points = 15 + level * 5;

    setShapeColor(level);  // ������ɫ
    shape->setOrigin(15, 15);
}

void TriangleEnemy::update(const sf::Vector2f& playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    switch (state) {
    case State::IDLE:
        idleState(playerPos, bullets);
        break;
    case State::CHARGING:
        chargingState();
        break;
    case State::DASHING:
        dashingState();
        break;
    }
}

void TriangleEnemy::idleState(const sf::Vector2f& playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    // �����ƶ�
    Enemy::update(playerPos, bullets);

    // ���������״̬
    if (rand() % 200 == 0) {
        state = State::CHARGING;
        chargeTime = 0;
    }
}

void TriangleEnemy::chargingState() {
    // �����׶�
    chargeTime++;
    shape->setScale(1.0f + chargeTime * 0.01f, 1.0f + chargeTime * 0.01f);

    if (chargeTime >= 30) {
        state = State::DASHING;
        dashDirection = normalize(shape->getPosition() - shape->getPosition()); // �ٶ��ǳ���ҷ�����
        dashDuration = 40;
    }
}

void TriangleEnemy::dashingState() {
    // ��̽׶�
    shape->move(dashDirection * dashSpeed);
    dashDuration--;

    if (dashDuration <= 0) {
        state = State::IDLE;
        shape->setScale(1.0f, 1.0f);
    }
}

void TriangleEnemy::attack(const sf::Vector2f& playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    // ��̹����������ӵ���ֱ����ײ�˺�
}

// HexagonEnemyʵ��
HexagonEnemy::HexagonEnemy(int level) : Enemy(EnemyType::HEXAGON, level), rotationSpeed(1.0f) {
    shape = std::make_unique<sf::CircleShape>(18, 6); // ������
    health = 5 + level * 2;
    speed = 1.2f + level * 0.1f;
    attackCooldown = 100 - level * 5;
    points = 20 + level * 5;

    setShapeColor(level);  // ������ɫ
    shape->setOrigin(18, 18);
}

void HexagonEnemy::attack(const sf::Vector2f& playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    // ����ҵı߷���һ����Ļ
    sf::Vector2f direction = normalize(playerPos - shape->getPosition());
    float angleToPlayer = std::atan2(direction.y, direction.x);

    // ��������Ļ
    bullets.emplace_back(createBullet(shape->getPosition(), direction, 4.5f, 1.0f + level * 0.2f, shape->getFillColor()));

    // ��Χ�����߸�����һ����Ļ
    for (int i = 0; i < 5; i++) {
        float angle = angleToPlayer + (i + 1) * PI / 3;
        sf::Vector2f dir(std::cos(angle), std::sin(angle));
        bullets.emplace_back(createBullet(shape->getPosition(), dir, 4.0f, 0.8f + level * 0.1f, shape->getFillColor()));
    }
}

// PentagonEnemyʵ��
PentagonEnemy::PentagonEnemy(int level) : Enemy(EnemyType::PENTAGON, level), rotationSpeed(2.0f) {
    shape = std::make_unique<sf::CircleShape>(20, 5); // �����
    health = 4 + level * 2;
    speed = 1.3f + level * 0.1f;
    attackCooldown = 80 - level * 4;
    points = 18 + level * 5;

    setShapeColor(level);  // ������ɫ
    shape->setOrigin(20, 20);
}

void PentagonEnemy::attack(const sf::Vector2f& playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    // ��ת���������ε�Ļ
    float baseAngle = std::atan2(playerPos.y - shape->getPosition().y, playerPos.x - shape->getPosition().x);

    for (int i = -2; i <= 2; i++) {
        float angle = baseAngle + i * PI / 12;
        sf::Vector2f dir(std::cos(angle), std::sin(angle));
        bullets.emplace_back(createBullet(shape->getPosition(), dir, 4.2f, 0.9f + level * 0.15f, shape->getFillColor()));
    }
}
