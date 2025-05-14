#include "gamestate.h"

GameState::GameState() : window(sf::VideoMode(800, 600), "Robot Battle"), player(RobotType::TYPE1) {
    window.setFramerateLimit(60);
    player.setPosition({ 400, 300 });
}

void GameState::handleInput() {
    sf::Vector2f direction(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) direction.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) direction.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) direction.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) direction.x += 1.f;

    if (direction.x != 0.f || direction.y != 0.f)
        player.move(normalize(direction));

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        player.shoot(mousePos, bullets);
    }
}

void GameState::update() {
    player.update(window.mapPixelToCoords(sf::Mouse::getPosition(window)), bullets);

    for (auto& bullet : bullets)
        bullet->update();

    // ÒÆ³ý³ö½ç×Óµ¯
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](const std::unique_ptr<Bullet>& b) { return b->isOffScreen(); }), bullets.end());
}

void GameState::render() {
    window.clear();

    player.draw(window);

    for (const auto& bullet : bullets)
        bullet->draw(window);

    window.display();
}

void GameState::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
    }
}
