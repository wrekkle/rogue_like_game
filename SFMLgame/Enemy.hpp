#pragma once
#include<vector>
#include<cstdlib>
#include "SFML\Graphics.hpp"
#include "SFML\window.hpp"
#include "SFML\System.hpp"

class Enemy :sf::Sprite {
	friend class Game;
private:
	static std::vector<Sprite> enemies;
	int enemy_create_time;
	sf::RenderWindow& window;
public:
	Enemy(sf::RenderWindow& window) :window(window) {
		sf::Texture* enemy_img = new sf::Texture();
		if (!enemy_img->loadFromFile("images/sf25.jpg")) {     //这里要换成你的main函数所在位置的同级文件目录路径
			throw "Could not load sf25!";
		}
		else {
			this->setTexture(*enemy_img);
			this->setScale(sf::Vector2f(0.02f, 0.02f));
		}
		this->setPosition(window.getSize().x, rand() % int(window.getSize().y));
		this->enemies.push_back(Sprite(*this));
		this->enemy_create_time = 20;
	}


	~Enemy() {
		const sf::Texture* enemy = this->getTexture();
		if (enemy) delete enemy;
	}


	void create_enemy() {
		// 每隔一段时间，创建1个新的敌机对象
		if (this->enemy_create_time < 50) {
			this->enemy_create_time++;
		}
		if (this->enemy_create_time >= 50) {
			// 修改第1架敌机的位置
			this->setPosition(this->window.getSize().x, rand() % int(window.getSize().y - this->getGlobalBounds().height));
			// 拷贝构造第1架敌机对象
			enemies.push_back(Sprite(*this));
			this->enemy_create_time = 0;
		}
	}


	void move_enemy() {

		// 移动所有的敌机
		for (size_t i = 0; i < this->enemies.size(); i++) {

			// 向左移动敌机
			this->enemies[i].move(-7.f, 0.0f);
			// 如果已经移动到左侧看不到的位置，那么就删除
			if (this->enemies[i].getPosition().x < 0 - this->enemies[i].getGlobalBounds().width) {
				this->enemies.erase(this->enemies.begin() + i);
			}
		}
	}

	void display() {
		// 将多驱敌机显示到窗口中
		for (size_t i = 0; i < this->enemies.size(); i++) {
			this->window.draw(this->enemies[i]);
		}
	}
};