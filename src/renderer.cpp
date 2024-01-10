#include "renderer.hpp"
#include "assets.hpp"

#include "wasm4.h"

#include <string.h>

void Renderer::setPalette(const uint32_t palette[4])
{
  PALETTE[0] = palette[0];
  PALETTE[1] = palette[1];
  PALETTE[2] = palette[2];
  PALETTE[3] = palette[3];
}

void Renderer::clear(uint8_t color)
{
  uint8_t i = color - 1;
  memset(FRAMEBUFFER, i | (i << 2) | (i << 4) | (i << 6), SCREEN_SIZE * SCREEN_SIZE / 4);
}

void Renderer::useColor(const uint16_t i)
{
  *DRAW_COLORS = i;
}

void Renderer::draw(const Vec2 &v)
{
  rect((int)v.x, (int)v.y, 1, 1);
}

void Renderer::draw(const Rect &r)
{
  rect((int)r.origin.x, (int)r.origin.y, (uint32_t)r.size.width, (uint32_t)r.size.height);
}

void Renderer::drawSpriteFrame(int index, int x, int y, uint32_t flags)
{
  const uint32_t srcW = 16;
  const uint32_t srcH = 16;
  const uint32_t stride = assets::tilemapWidth;
  const uint32_t srcX = ((uint32_t)index % 20) * srcW;
  const uint32_t srcY = ((uint32_t)index / 20) * srcH;

  blitSub(assets::tilemap, x, y, srcW, srcH, srcX, srcY, stride, flags);
}

void Renderer::drawText(const char *text, int x, int y)
{
  ::text(text, x, y);
}
