#include "entity.hpp"
#include "wasm4.h"

void Entity::Input::updateForGamepad(uint8_t gamepad) {
    up = gamepad & BUTTON_UP;
    left = gamepad & BUTTON_LEFT;
    right = gamepad & BUTTON_RIGHT;
    primaryAction = gamepad & BUTTON_1;
    secondaryAction = gamepad & BUTTON_2;
}

void Entity::update() {
}

void Entity::render(Renderer& renderer) const {
    if (!visible) {
        return;
    }
    renderer.drawSpriteFrame(sprite, (int)bounds.origin.x, (int)bounds.origin.y, directionX < 0);
}

void Projectile::update() {
    previousPosition = position;
    position += velocity;
}

void Projectile::render(Renderer& renderer) const {
    renderer.draw(previousPosition, position);
}

void Tile::update() {
}

void Tile::render(Renderer& renderer) const {
    renderer.drawSpriteFrame(sprite, (int)bounds.origin.x, (int)bounds.origin.y);
}
