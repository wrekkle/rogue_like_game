#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "bullet.h"
#include "prop.h" // 引入道具类型头文件

// RobotType 枚举类型的声明
enum RobotType { TYPE1, TYPE2, TYPE3 };

// Player 类
class Player {
public:
    Player(RobotType type); // 构造函数
    void update(const sf::Vector2f& mousePos, std::vector<std::unique_ptr<Bullet>>& bullets);
    void draw(sf::RenderWindow& window) const;
    void move(const sf::Vector2f& direction);
    void shoot(const sf::Vector2f& target, std::vector<std::unique_ptr<Bullet>>& bullets);
    void takeDamage(int damage);
    void collectProp(const PropType& propType);

    // 获取玩家的属性
    sf::RectangleShape getShape() const { return shape; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    RobotType getRobotType() const { return robotType; }

private:
    void initStats(); // 初始化玩家的属性

    RobotType robotType; // 玩家机器人的类型
    int health, maxHealth;
    float speed;
    float fireRate;
    float damage;
    int lastShotTime; // 用于控制射击间隔
    int splitBulletsCount; // 分裂子弹的计数
    float damageMultiplier;
    bool hasUnlimitedEnergy; // 是否有无限能量
    int unlimitedEnergyTimer; // 无限能量计时器
    bool isInvincible; // 是否无敌
    int invincibilityTimer; // 无敌计时器

    sf::RectangleShape shape; // 玩家形状
};

#endif // PLAYER_H
