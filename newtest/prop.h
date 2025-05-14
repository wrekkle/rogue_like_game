#ifndef PROP_H
#define PROP_H

#include "game.h"

class Prop {
public:
    Prop(const sf::Vector2f& position, PropType type);

    void update();
    void draw(sf::RenderWindow& window) const;
    bool isAlive() const { return alive; }
    PropType getType() const { return type; }
    sf::CircleShape getShape() const { return shape; }

private:
    sf::CircleShape shape;
    PropType type;
    float lifeTime;
    bool alive;
};

#endif // PROP_H