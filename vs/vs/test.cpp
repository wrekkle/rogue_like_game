#include <SFML/Graphics.hpp>
#include <vector>

// 玩家类
class Player {
public:
    Player() {
        shape.setSize(sf::Vector2f(32, 32));
        shape.setFillColor(sf::Color::Green);
        shape.setPosition(400, 300);
    }

    void move(float dx, float dy) {
        shape.move(dx, dy);
    }

    sf::RectangleShape getShape() const {
        return shape;
    }

private:
    sf::RectangleShape shape;
};

// 敌人类
class Enemy {
public:
    Enemy(float x, float y) {
        shape.setSize(sf::Vector2f(32, 32));
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(x, y);
    }

    void moveTowardsPlayer(const Player& player) {
        sf::Vector2f playerPos = player.getShape().getPosition();
        sf::Vector2f enemyPos = shape.getPosition();
        sf::Vector2f direction = playerPos - enemyPos;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0) {
            direction /= length;
            shape.move(direction * 2.0f);
        }
    }

    sf::RectangleShape getShape() const {
        return shape;
    }

private:
    sf::RectangleShape shape;
};

// 游戏类
class Game {
public:
    Game() : window(sf::VideoMode(800, 600), "2D Pixel Roguelike") {
        // 初始化玩家
        player = Player();

        // 初始化敌人
        enemies.push_back(Enemy(100, 100));
        enemies.push_back(Enemy(700, 100));
    }

    void run() {
        while (window.isOpen()) {
            processEvents();
            update();
            render();
        }
    }

private:
    sf::RenderWindow window;
    Player player;
    std::vector<Enemy> enemies;

    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::W) {
                    player.move(0, -5);
                }
                if (event.key.code == sf::Keyboard::S) {
                    player.move(0, 5);
                }
                if (event.key.code == sf::Keyboard::A) {
                    player.move(-5, 0);
                }
                if (event.key.code == sf::Keyboard::D) {
                    player.move(5, 0);
                }
            }
        }
    }

    void update() {
        for (auto& enemy : enemies) {
            enemy.moveTowardsPlayer(player);
        }
    }

    void render() {
        window.clear();
        window.draw(player.getShape());
        for (const auto& enemy : enemies) {
            window.draw(enemy.getShape());
        }
        window.display();
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}