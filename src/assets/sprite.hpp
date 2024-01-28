#pragma once

#include <span>

struct Sprite {
    int width;
    int height;
    int flags;
    std::span<uint8_t> data;
};