#include "entity.hpp"
#include "wasm4.h"

void Entity::update()
{
}

void Entity::render(Renderer &renderer) const
{
  uint32_t flag = BLIT_1BPP;
  if (directionX < 0)
  {
    flag |= BLIT_FLIP_X;
  }
  renderer.drawSpriteFrame(sprite, (int)bounds.origin.x, (int)bounds.origin.y, flag);
}

void Projectile::update()
{
}

void Projectile::render(Renderer &renderer) const
{
  renderer.draw(position);
}

void Tile::update()
{
}

void Tile::render(Renderer &renderer) const
{
  renderer.drawSpriteFrame(sprite, (int)bounds.origin.x, (int)bounds.origin.y);
}
