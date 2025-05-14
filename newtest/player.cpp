#include "player.h"
#include "bullet.h"
#include "prop.h" // �������ͷ�ļ�

Player::Player(RobotType type) : robotType(type), health(5), maxHealth(5), speed(5.0f), fireRate(0.2f), damage(1.0f), lastShotTime(0),
splitBulletsCount(0), damageMultiplier(1.0f), hasUnlimitedEnergy(false), unlimitedEnergyTimer(0), isInvincible(false), invincibilityTimer(0) {
    shape.setSize(sf::Vector2f(30, 30));
    shape.setOrigin(15, 15);
    initStats();
}

void Player::initStats() {
    switch (robotType) {
    case TYPE1:
        shape.setFillColor(sf::Color::Green);
        health = maxHealth = 5;
        speed = 6.0f;
        fireRate = 0.15f; // �����������
        damage = 0.8f;    // �ϵ͵Ļ����˺�
        break;
    case TYPE2:
        shape.setFillColor(sf::Color::Yellow);
        health = maxHealth = 3;
        speed = 4.0f;     // �������ƶ��ٶ�
        fireRate = 0.5f;  // �̶�������
        damage = 1.0f;
        break;
    case TYPE3:
        shape.setFillColor(sf::Color::Blue);
        health = maxHealth = 2;
        speed = 5.0f;
        fireRate = 0.7f;  // ��������
        damage = 2.5f;    // ���˺�
        break;
    }
}

void Player::update(const sf::Vector2f& mousePos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    // �����޵�״̬
    if (isInvincible) {
        invincibilityTimer--;
        if (invincibilityTimer <= 0)
            isInvincible = false;
    }

    // ������������״̬
    if (hasUnlimitedEnergy) {
        unlimitedEnergyTimer--;
        if (unlimitedEnergyTimer <= 0) {
            hasUnlimitedEnergy = false;
            if (robotType == TYPE3)
                fireRate = 0.7f; // �ָ�����������
        }
    }
}

void Player::draw(sf::RenderWindow& window) const {
    // �޵�״̬��˸Ч��
    if (isInvincible && static_cast<int>(invincibilityTimer) % 10 < 5) {
        sf::RectangleShape tempShape = shape;
        tempShape.setFillColor(sf::Color(shape.getFillColor().r, shape.getFillColor().g, shape.getFillColor().b, 128));
        window.draw(tempShape);
    }
    else {
        window.draw(shape);
    }
}

void Player::move(const sf::Vector2f& direction) {
    sf::Vector2f newPos = shape.getPosition() + direction * speed;

    // �߽���
    if (newPos.x >= 15 && newPos.x <= SCREEN_WIDTH - 15)
        shape.setPosition(newPos.x, shape.getPosition().y);

    if (newPos.y >= 15 && newPos.y <= SCREEN_HEIGHT - 15)
        shape.setPosition(shape.getPosition().x, newPos.y);
}

void Player::shoot(const sf::Vector2f& target, std::vector<std::unique_ptr<Bullet>>& bullets) {
    // ���������
    if (lastShotTime > 0 && !hasUnlimitedEnergy)
        return;

    // �����������
    sf::Vector2f direction = normalize(target - shape.getPosition());

    // �����ӵ�
    float actualDamage = damage * damageMultiplier;

    switch (robotType) {
    case TYPE1:
        // 1�ͻ������Զ��������
        bullets.emplace_back(std::make_unique<Bullet>(shape.getPosition(), direction, 6.0f, actualDamage, shape.getFillColor(), false, splitBulletsCount));
        break;
    case TYPE2:
        // 2�ͻ����˷����ӵ��߼�������ײ����д���
        break;
    case TYPE3:
        // 3�ͻ����˸��˺����
        bullets.emplace_back(std::make_unique<Bullet>(shape.getPosition(), direction, 7.0f, actualDamage, shape.getFillColor(), false, splitBulletsCount));
        break;
    }

    // �������ʱ��
    lastShotTime = fireRate * 60; // ת��Ϊ֡��
}

void Player::takeDamage(int damage) {
    if (!isInvincible) {
        health -= damage;
        if (health < 0) health = 0;

        // �����޵�֡
        if (health > 0) {
            isInvincible = true;
            invincibilityTimer = 60; // 1���޵�
        }
    }
}

void Player::collectProp(const PropType& propType) {
    switch (propType) {
    case PropType::BASIC:
        // �����������Ч��
        if (rand() % 3 == 0) {
            // �����ӵ�
            splitBulletsCount++;
        }
        else if (rand() % 2 == 0) {
            // ��ǿ�ӵ�
            damageMultiplier += 0.3f;
        }
        else {
            // ��Ѫ
            health = std::min(health + 1, maxHealth);
        }
        break;
    case PropType::RARE:
        // ϡ�е������Ч��
        if (rand() % 3 == 0) {
            // ���޷��ѵ���
            splitBulletsCount = 10; // �߷��Ѵ�����ʾ���޷���
        }
        else if (rand() % 2 == 0) {
            // ����Ѫ������
            maxHealth += 3;
            health = maxHealth;
        }
        else {
            // ���ӻ����˺�
            damage *= 2.0f;
        }
        break;
    case PropType::LEGENDARY:
        // ������ߣ������������ͣ�
        if (robotType == TYPE1) {
            // �޵�״̬
            isInvincible = true;
            invincibilityTimer = 180; // 3���޵�
        }
        else if (robotType == TYPE2) {
            // ������Ļ��Ŀ����������ײ����д���
            splitBulletsCount += 5;
        }
        else if (robotType == TYPE3) {
            // �޾���Դ
            hasUnlimitedEnergy = true;
            unlimitedEnergyTimer = 600; // 10��
            fireRate = 0.05f; // �����޼��
        }
        break;
    }
}
