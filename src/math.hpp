#pragma once

#include <stdint.h>

struct Vec2 {
    using value_type = float;

    value_type x;
    value_type y;
};

struct Size {
    using value_type = float;

    value_type width;
    value_type height;
};

struct Rect {
    using value_type = Vec2::value_type;

    Vec2 origin;
    Size size;

    inline value_type left() const {
        return origin.x;
    };
    inline value_type right() const {
        return origin.x + size.width;
    };
    inline value_type top() const {
        return origin.y;
    };
    inline value_type bottom() const {
        return origin.y + size.height;
    };

    bool contains(const Vec2& p) const;
    bool collision(const Rect& other) const;
};

struct Color {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
};
