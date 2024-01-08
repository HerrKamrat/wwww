#pragma once

#include <stdint.h>

struct Vec2
{
  using value_type = float;

  value_type x;
  value_type y;
};

struct Size
{
  using value_type = float;

  value_type width;
  value_type height;
};

struct Rect
{
  Vec2 origin;
  Size size;
};