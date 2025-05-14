#include "prop.h"

Prop::Prop(const sf::Vector2f& position, PropType type) : type(type), lifeTime(300), alive(true) {
    shape.setRadius(10);
    shape.setOrigin(10, 10);
    shape.setPosition(position);

    // ���ݵ�������������ɫ
    switch (type) {
    case PropType::BASIC:
        shape.setFillColor(sf::Color(135, 206, 250)); // ǳ��ɫ
        break;
    case PropType::RARE:
        shape.setFillColor(sf::Color(147, 112, 219)); // ��ɫ
        break;
    case PropType::LEGENDARY:
        shape.setFillColor(sf::Color(255, 215, 0));   // ��ɫ
        break;
    }
}

void Prop::update() {
    // ��˸Ч��
    if (lifeTime < 100) {
        if (static_cast<int>(lifeTime) % 10 < 5) {
            shape.setFillColor(sf::Color(shape.getFillColor().r, shape.getFillColor().g, shape.getFillColor().b, 128));
        }
        else {
            shape.setFillColor(sf::Color(shape.getFillColor().r, shape.getFillColor().g, shape.getFillColor().b, 255));
        }
    }

    lifeTime--;
    if (lifeTime <= 0)
        alive = false;
}

void Prop::draw(sf::RenderWindow& window) const {
    window.draw(shape);
}