#ifndef BULLET_H
#define BULLET_H

#include <SFML/Graphics.hpp>

const int SCREEN_WIDTH = 800; // 屏幕宽度
const int SCREEN_HEIGHT = 600; // 屏幕高度

class Bullet {
public:
    Bullet(const sf::Vector2f& position, const sf::Vector2f& direction, float speed, float damage, const sf::Color& color, bool isEnemyBullet, int splitCount = 0);

    void update();
    void draw(sf::RenderWindow& window) const;
    bool isOutOfBounds() const;

    // 获取子弹的形状（用于绘制等）
    sf::CircleShape getShape() const { return shape; }

private:
    sf::CircleShape shape;      // 子弹的形状
    sf::Vector2f velocity;      // 子弹的速度
    float damage;               // 子弹伤害
    bool isEnemyBullet;         // 是否是敌方子弹
    int splitCount;             // 子弹的分裂次数

};

#endif // BULLET_H
