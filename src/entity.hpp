#pragma once

#include "math.hpp"
#include "renderer.hpp"

struct Entity
{
  Rect bounds;
  Vec2 velocity;
  int directionX;
  struct Collisions
  {
    bool up;
    bool down;
    bool left;
    bool right;
  } collisions;

  int sprite;
  const int *animation;

  void update();
  void render(Renderer &renderer) const;
};

struct Projectile
{
  Vec2 position;
  Vec2 velocity;
  bool active;

  void update();
  void render(Renderer &renderer) const;
};

struct Tile
{
  Rect bounds;
  int sprite;

  void update();
  void render(Renderer &renderer) const;
};
