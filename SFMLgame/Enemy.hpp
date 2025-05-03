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
		if (!enemy_img->loadFromFile("images/sf25.jpg")) {     //����Ҫ�������main��������λ�õ�ͬ���ļ�Ŀ¼·��
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
		// ÿ��һ��ʱ�䣬����1���µĵл�����
		if (this->enemy_create_time < 50) {
			this->enemy_create_time++;
		}
		if (this->enemy_create_time >= 50) {
			// �޸ĵ�1�ܵл���λ��
			this->setPosition(this->window.getSize().x, rand() % int(window.getSize().y - this->getGlobalBounds().height));
			// ���������1�ܵл�����
			enemies.push_back(Sprite(*this));
			this->enemy_create_time = 0;
		}
	}


	void move_enemy() {

		// �ƶ����еĵл�
		for (size_t i = 0; i < this->enemies.size(); i++) {

			// �����ƶ��л�
			this->enemies[i].move(-7.f, 0.0f);
			// ����Ѿ��ƶ�����࿴������λ�ã���ô��ɾ��
			if (this->enemies[i].getPosition().x < 0 - this->enemies[i].getGlobalBounds().width) {
				this->enemies.erase(this->enemies.begin() + i);
			}
		}
	}

	void display() {
		// �������л���ʾ��������
		for (size_t i = 0; i < this->enemies.size(); i++) {
			this->window.draw(this->enemies[i]);
		}
	}
};