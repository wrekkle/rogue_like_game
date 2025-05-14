#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>

// ��������
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const float PI = 3.14159265f;

// ǰ������
class Player;
class Enemy;
class Bullet;
class Prop;
class GameState;
class GameStateManager;

// ö������
enum class RobotType { TYPE1, TYPE2, TYPE3 };       // ����������
enum class EnemyType { TRIANGLE, HEXAGON, PENTAGON }; // ������״����
enum class PropType { BASIC, RARE, LEGENDARY };      // ��������
enum class GameStateType { MAIN_MENU, CHARACTER_SELECTION, IN_GAME, PAUSED, GAME_OVER, LEVEL_COMPLETED }; // ��Ϸ״̬����

// ������������
float distance(const sf::Vector2f& a, const sf::Vector2f& b);  // ������������
float angle(const sf::Vector2f& from, const sf::Vector2f& to);  // ���������Ƕ�
sf::Vector2f normalize(const sf::Vector2f& vector);             // ��һ������

// ��ײ��⺯������
bool isCollision(const sf::Shape& shape1, const sf::Shape& shape2); // �ж���״�Ƿ���ײ

#endif // GAME_H
