#pragma once

#include "math.hpp"
#include "wasm4.h"

class Renderer {
  public:
    enum class BitsPerPixel : uint32_t {
        One = BLIT_1BPP,
        Two = BLIT_2BPP
    };

    /// @brief set the viewport to {x, y}, {x + SCREEN_SIZE, y + SCREEN_SIZE}
    /// @param x
    /// @param y
    void setViewport(int x, int y);

    void setPalette(const uint32_t palette[4]);

    void clear(uint8_t color);
    void useColor(const uint16_t i);
    void draw(const Vec2& v);
    void draw(const Rect& r);
    void drawSpriteFrame(int index, int x, int y, bool flipX = false, bool flipY = false,
                         BitsPerPixel bbp = BitsPerPixel::Two);

    void drawText(const char* text, int x, int y);

  private:
    Vec2 cameraPosition = {0, 0};
};