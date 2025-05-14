#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "bullet.h"
#include "prop.h" // �����������ͷ�ļ�

// RobotType ö�����͵�����
enum RobotType { TYPE1, TYPE2, TYPE3 };

// Player ��
class Player {
public:
    Player(RobotType type); // ���캯��
    void update(const sf::Vector2f& mousePos, std::vector<std::unique_ptr<Bullet>>& bullets);
    void draw(sf::RenderWindow& window) const;
    void move(const sf::Vector2f& direction);
    void shoot(const sf::Vector2f& target, std::vector<std::unique_ptr<Bullet>>& bullets);
    void takeDamage(int damage);
    void collectProp(const PropType& propType);

    // ��ȡ��ҵ�����
    sf::RectangleShape getShape() const { return shape; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    RobotType getRobotType() const { return robotType; }

private:
    void initStats(); // ��ʼ����ҵ�����

    RobotType robotType; // ��һ����˵�����
    int health, maxHealth;
    float speed;
    float fireRate;
    float damage;
    int lastShotTime; // ���ڿ���������
    int splitBulletsCount; // �����ӵ��ļ���
    float damageMultiplier;
    bool hasUnlimitedEnergy; // �Ƿ�����������
    int unlimitedEnergyTimer; // ����������ʱ��
    bool isInvincible; // �Ƿ��޵�
    int invincibilityTimer; // �޵м�ʱ��

    sf::RectangleShape shape; // �����״
};

#endif // PLAYER_H
