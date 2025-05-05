#include <iostream>
#include<vector>

#include "SFML\Graphics.hpp"
#include "SFML\window.hpp"
#include "SFML\System.hpp"
#include"Enemy.hpp"
#include"Player.hpp"
#include"Bullet.hpp"
#include"Game.hpp"

using namespace std;
using namespace sf;



vector<Sprite> Enemy::enemies;
vector<Sprite> Player::bullets;


int main() {
    // 创建窗口对象
    RenderWindow window(VideoMode(1200,800), "fighters", Style::Default);

    // 显示刷新速度
    window.setFramerateLimit(60);

    Player player(window);

    Enemy enemy(window);



    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
        }

        player.key_handle();

        enemy.create_enemy();

        enemy.move_enemy();

        if (Game::collision_bullet_kill_enemy(player)) {
            window.close();
        }

        window.clear(Color::Black);

        player.display();

        enemy.display(); 

        window.display();
    }
    return 0;
}
