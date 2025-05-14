#include "bullet.h"

Bullet::Bullet(const sf::Vector2f& position, const sf::Vector2f& direction, float speed, float damage, const sf::Color& color, bool isEnemyBullet, int splitCount)
    : damage(damage), isEnemyBullet(isEnemyBullet), splitCount(splitCount) {
    shape.setRadius(5);               // 子弹的半径
    shape.setOrigin(5, 5);            // 设置原点为子弹的中心
    shape.setPosition(position);      // 设置初始位置
    shape.setFillColor(color);        // 设置颜色
    velocity = direction * speed;     // 计算速度
}

void Bullet::update() {
    shape.move(velocity);  // 根据速度更新子弹的位置
}

void Bullet::draw(sf::RenderWindow& window) const {
    window.draw(shape);  // 在窗口上绘制子弹
}

bool Bullet::isOutOfBounds() const {
    sf::Vector2f pos = shape.getPosition();
    return pos.x < -10 || pos.x > SCREEN_WIDTH + 10 ||
        pos.y < -10 || pos.y > SCREEN_HEIGHT + 10;  // 检测子弹是否超出屏幕边界
}
