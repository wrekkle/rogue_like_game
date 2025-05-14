#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

// ✅ 包含 Player 和 Bullet 的头文件
#include "player.h"
#include "bullet.h"

class GameState {
public:
    GameState();

    void handleInput();
    void update();
    void render();

    bool isRunning() const { return window.isOpen(); }

private:
    sf::RenderWindow window;
    Player player; // ✅ 使用 Player 类没问题了
    std::vector<std::unique_ptr<Bullet>> bullets;

    sf::Clock clock;

    void processEvents();
};
