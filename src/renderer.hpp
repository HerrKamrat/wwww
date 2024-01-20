#pragma once

#include "math.hpp"

class Renderer
{
public:
  /// @brief set the viewport to {x, y}, {x + SCREEN_SIZE, y + SCREEN_SIZE}
  /// @param x
  /// @param y
  void setViewport(int x, int y);

  void setPalette(const uint32_t palette[4]);

  void clear(uint8_t color);
  void useColor(const uint16_t i);
  void draw(const Vec2 &v);
  void draw(const Rect &r);
  void drawSpriteFrame(int index, int x, int y, uint32_t flags = 0);

  void drawText(const char *text, int x, int y);

private:
  Vec2 cameraPosition = {0, 0};
};