#pragma once
#include<vector>
#include "SFML\Graphics.hpp"
#include "SFML\window.hpp"
#include "SFML\System.hpp"


class Bullet : public sf::Sprite {
    private:
    public:
        Bullet(float x, float y) {
            sf::Texture* bullet_img = new sf::Texture();
            if (!bullet_img->loadFromFile("images/bullet2.png")) {
                throw "Could not load png!";
            }
            else {
                this->setTexture(*bullet_img);
                this->setScale(sf::Vector2f(0.5, 0.5));
                this->setPosition(sf::Vector2f(x, y));
            }
        }

        ~Bullet() {
            const sf::Texture* img = this->getTexture();
            if (img) delete img;
        }
};