#pragma once
#include "SFML\Graphics.hpp"
#include "SFML\window.hpp"
#include "SFML\System.hpp"
#include"Player.hpp"
#include"Enemy.hpp"
#include"Bullet.hpp"


class Game {
private:
public:
    static bool collision_bullet_kill_enemy(Player& player_plane) {
        for (size_t i = 0; i < Enemy::enemies.size(); i++) { // �������еĵл�
            bool del_flag = false;
            // �ж��ӵ��Ƿ���ײ���л�
            for (size_t j = 0; j < Player::bullets.size(); j++) { // �������е��ӵ�
                if (Player::bullets[j].getGlobalBounds().intersects(Enemy::enemies[i].getGlobalBounds())) {
                    Enemy::enemies.erase(Enemy::enemies.begin() + i);
                    Player::bullets.erase(Player::bullets.begin() + j);
                    del_flag = true;
                    break;
                }
            }

            // �ж��Ƿ���ײ����ҷɻ�
            if (del_flag == false && player_plane.getGlobalBounds().intersects(Enemy::enemies[i].getGlobalBounds())) {
                Enemy::enemies.erase(Enemy::enemies.begin() + i);
                return true;
            }
        }
        return false;
    }
};