#ifndef BULLET_H
#define BULLET_H

#include <SFML/Graphics.hpp>

const int SCREEN_WIDTH = 800; // ��Ļ���
const int SCREEN_HEIGHT = 600; // ��Ļ�߶�

class Bullet {
public:
    Bullet(const sf::Vector2f& position, const sf::Vector2f& direction, float speed, float damage, const sf::Color& color, bool isEnemyBullet, int splitCount = 0);

    void update();
    void draw(sf::RenderWindow& window) const;
    bool isOutOfBounds() const;

    // ��ȡ�ӵ�����״�����ڻ��Ƶȣ�
    sf::CircleShape getShape() const { return shape; }

private:
    sf::CircleShape shape;      // �ӵ�����״
    sf::Vector2f velocity;      // �ӵ����ٶ�
    float damage;               // �ӵ��˺�
    bool isEnemyBullet;         // �Ƿ��ǵз��ӵ�
    int splitCount;             // �ӵ��ķ��Ѵ���

};

#endif // BULLET_H
