#include "player.h"
#include "bullet.h"
#include "prop.h" // 引入道具头文件

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
        fireRate = 0.15f; // 更快的射击间隔
        damage = 0.8f;    // 较低的基础伤害
        break;
    case TYPE2:
        shape.setFillColor(sf::Color::Yellow);
        health = maxHealth = 3;
        speed = 4.0f;     // 较慢的移动速度
        fireRate = 0.5f;  // 固定射击间隔
        damage = 1.0f;
        break;
    case TYPE3:
        shape.setFillColor(sf::Color::Blue);
        health = maxHealth = 2;
        speed = 5.0f;
        fireRate = 0.7f;  // 长射击间隔
        damage = 2.5f;    // 高伤害
        break;
    }
}

void Player::update(const sf::Vector2f& mousePos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    // 更新无敌状态
    if (isInvincible) {
        invincibilityTimer--;
        if (invincibilityTimer <= 0)
            isInvincible = false;
    }

    // 更新无限能量状态
    if (hasUnlimitedEnergy) {
        unlimitedEnergyTimer--;
        if (unlimitedEnergyTimer <= 0) {
            hasUnlimitedEnergy = false;
            if (robotType == TYPE3)
                fireRate = 0.7f; // 恢复正常射击间隔
        }
    }
}

void Player::draw(sf::RenderWindow& window) const {
    // 无敌状态闪烁效果
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

    // 边界检查
    if (newPos.x >= 15 && newPos.x <= SCREEN_WIDTH - 15)
        shape.setPosition(newPos.x, shape.getPosition().y);

    if (newPos.y >= 15 && newPos.y <= SCREEN_HEIGHT - 15)
        shape.setPosition(shape.getPosition().x, newPos.y);
}

void Player::shoot(const sf::Vector2f& target, std::vector<std::unique_ptr<Bullet>>& bullets) {
    // 检查射击间隔
    if (lastShotTime > 0 && !hasUnlimitedEnergy)
        return;

    // 计算射击方向
    sf::Vector2f direction = normalize(target - shape.getPosition());

    // 创建子弹
    float actualDamage = damage * damageMultiplier;

    switch (robotType) {
    case TYPE1:
        // 1型机器人自动索敌射击
        bullets.emplace_back(std::make_unique<Bullet>(shape.getPosition(), direction, 6.0f, actualDamage, shape.getFillColor(), false, splitBulletsCount));
        break;
    case TYPE2:
        // 2型机器人反弹子弹逻辑（在碰撞检测中处理）
        break;
    case TYPE3:
        // 3型机器人高伤害射击
        bullets.emplace_back(std::make_unique<Bullet>(shape.getPosition(), direction, 7.0f, actualDamage, shape.getFillColor(), false, splitBulletsCount));
        break;
    }

    // 更新射击时间
    lastShotTime = fireRate * 60; // 转换为帧数
}

void Player::takeDamage(int damage) {
    if (!isInvincible) {
        health -= damage;
        if (health < 0) health = 0;

        // 触发无敌帧
        if (health > 0) {
            isInvincible = true;
            invincibilityTimer = 60; // 1秒无敌
        }
    }
}

void Player::collectProp(const PropType& propType) {
    switch (propType) {
    case PropType::BASIC:
        // 基础道具随机效果
        if (rand() % 3 == 0) {
            // 分裂子弹
            splitBulletsCount++;
        }
        else if (rand() % 2 == 0) {
            // 增强子弹
            damageMultiplier += 0.3f;
        }
        else {
            // 回血
            health = std::min(health + 1, maxHealth);
        }
        break;
    case PropType::RARE:
        // 稀有道具随机效果
        if (rand() % 3 == 0) {
            // 无限分裂弹道
            splitBulletsCount = 10; // 高分裂次数表示无限分裂
        }
        else if (rand() % 2 == 0) {
            // 增加血量上限
            maxHealth += 3;
            health = maxHealth;
        }
        else {
            // 增加基础伤害
            damage *= 2.0f;
        }
        break;
    case PropType::LEGENDARY:
        // 传奇道具（按机器人类型）
        if (robotType == TYPE1) {
            // 无敌状态
            isInvincible = true;
            invincibilityTimer = 180; // 3秒无敌
        }
        else if (robotType == TYPE2) {
            // 反弹弹幕数目翻倍（在碰撞检测中处理）
            splitBulletsCount += 5;
        }
        else if (robotType == TYPE3) {
            // 无尽能源
            hasUnlimitedEnergy = true;
            unlimitedEnergyTimer = 600; // 10秒
            fireRate = 0.05f; // 几乎无间隔
        }
        break;
    }
}
