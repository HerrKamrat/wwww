#pragma once

#include "math.hpp"

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
};

struct Projectile
{
  Vec2 position;
  Vec2 velocity;
  bool active;
};

struct Tile
{
  Rect bounds;
  int sprite;
};
