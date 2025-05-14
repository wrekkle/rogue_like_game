#include "game.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "prop.h"
#include "gamestate.h"
#include <iostream>

float distance(const sf::Vector2f& a, const sf::Vector2f& b) {
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

float angle(const sf::Vector2f& from, const sf::Vector2f& to) {
    return std::atan2(to.y - from.y, to.x - from.x);
}

sf::Vector2f normalize(const sf::Vector2f& vector) {
    float length = std::sqrt(vector.x * vector.x + vector.y * vector.y);
    return (length != 0) ? sf::Vector2f(vector.x / length, vector.y / length) : sf::Vector2f(0, 0);
}

bool isCollision(const sf::Shape& shape1, const sf::Shape& shape2) {
    return shape1.getGlobalBounds().intersects(shape2.getGlobalBounds());
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "2D Ballistic Shooting Game");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Could not load font!" << std::endl;
        return 1;
    }

    GameStateManager stateManager(font);
    stateManager.changeState(std::make_unique<MainMenuState>(&stateManager, font));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            stateManager.handleEvent(event);
        }

        stateManager.update();

        window.clear();
        stateManager.draw(window);
        window.display();
    }

    return 0;
}
