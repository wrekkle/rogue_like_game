#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>

// 常量定义
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const float PI = 3.14159265f;

// 前向声明
class Player;
class Enemy;
class Bullet;
class Prop;
class GameState;
class GameStateManager;

// 枚举类型
enum class RobotType { TYPE1, TYPE2, TYPE3 };       // 机器人类型
enum class EnemyType { TRIANGLE, HEXAGON, PENTAGON }; // 敌人形状类型
enum class PropType { BASIC, RARE, LEGENDARY };      // 道具类型
enum class GameStateType { MAIN_MENU, CHARACTER_SELECTION, IN_GAME, PAUSED, GAME_OVER, LEVEL_COMPLETED }; // 游戏状态类型

// 辅助函数声明
float distance(const sf::Vector2f& a, const sf::Vector2f& b);  // 计算两点间距离
float angle(const sf::Vector2f& from, const sf::Vector2f& to);  // 计算两点间角度
sf::Vector2f normalize(const sf::Vector2f& vector);             // 归一化向量

// 碰撞检测函数声明
bool isCollision(const sf::Shape& shape1, const sf::Shape& shape2); // 判断形状是否碰撞

#endif // GAME_H
