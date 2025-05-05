#pragma once
#include<cstdlib>
#include "SFML\Graphics.hpp"
#include "SFML\window.hpp"
#include "SFML\System.hpp"
#include"Bullet.hpp"


class Player :public sf::Sprite {
	friend class Game;
private:
	sf::RenderWindow& window;
	static std::vector<Sprite> bullets;
	long int last_create_bullet_time;
public:
	Player(sf::RenderWindow& window):window(window) {
		sf::Texture* player_img = new sf::Texture();
		if (!player_img->loadFromFile("images/lewis.jpg")) { 
			throw "Could not load lewis!";
		}
		else {
			this->setTexture(*player_img);
			this->setScale(0.1f, 0.1f);
		}
	}


	~Player() {
		const sf::Texture* texture = this->getTexture();
		if (texture) delete texture;
	}

	void key_handle() {
		// 如果按下ESC那么就退出
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
			this->window.close();
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			// 键盘 上
			this->move(0.0f, -4.0f);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			// 键盘 下
			this->move(0.0f, 4.0f);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			// 键盘 左
			this->move(-4.0f, 0.f);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			// 键盘 右
			this->move(4.0f, 0.0f);
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			// 鼠标左键
			long int current_time = time(NULL);
			if (current_time > this->last_create_bullet_time) {
				this->fire();
				this->last_create_bullet_time = current_time;
			}
		}
	}


	void fire() {
		this->bullets.push_back(Bullet(this->getPosition().x, this->getPosition().y));
		for (size_t i = 0; i < this->bullets.size(); i++) {
			this->window.draw(this->bullets[i]);
		}
	}


	void display() {
		window.draw(*this);

		for (size_t i = 0; i < this->bullets.size(); i++) {
			// 向右移动子弹
			this->bullets[i].move(7.f, 0.f);
			// 如果已经移动到右侧看不到的位置，那么就删除
			if (this->bullets[i].getPosition().x > this->window.getSize().x) {
				this->bullets.erase(this->bullets.begin() + i);
			}
		}

		// 显示所有的子弹
		for (size_t i = 0; i < this->bullets.size(); i++) {
			this->window.draw(this->bullets[i]);
		}
	}
};