#include "bullet.h"

Bullet::Bullet(const sf::Vector2f& position, const sf::Vector2f& direction, float speed, float damage, const sf::Color& color, bool isEnemyBullet, int splitCount)
    : damage(damage), isEnemyBullet(isEnemyBullet), splitCount(splitCount) {
    shape.setRadius(5);               // �ӵ��İ뾶
    shape.setOrigin(5, 5);            // ����ԭ��Ϊ�ӵ�������
    shape.setPosition(position);      // ���ó�ʼλ��
    shape.setFillColor(color);        // ������ɫ
    velocity = direction * speed;     // �����ٶ�
}

void Bullet::update() {
    shape.move(velocity);  // �����ٶȸ����ӵ���λ��
}

void Bullet::draw(sf::RenderWindow& window) const {
    window.draw(shape);  // �ڴ����ϻ����ӵ�
}

bool Bullet::isOutOfBounds() const {
    sf::Vector2f pos = shape.getPosition();
    return pos.x < -10 || pos.x > SCREEN_WIDTH + 10 ||
        pos.y < -10 || pos.y > SCREEN_HEIGHT + 10;  // ����ӵ��Ƿ񳬳���Ļ�߽�
}
