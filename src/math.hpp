#pragma once

#include <cstdlib>
#include <stdint.h>

struct Vec2 {
    using value_type = float;

    value_type x;
    value_type y;

    Vec2& operator+=(Vec2 const& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2& operator-=(Vec2 const& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vec2 operator+(const Vec2& other) const {
        Vec2 out = *this;
        out += other;
        return out;
    };

    Vec2 operator-(const Vec2& other) const {
        Vec2 out = *this;
        out -= other;
        return out;
    };

    Vec2 operator*(const value_type& other) const {
        Vec2 out = *this;
        out.x *= other;
        out.y *= other;
        return out;
    };
    Vec2 operator/(const value_type& other) const {
        Vec2 out = *this;
        out.x /= other;
        out.y /= other;
        return out;
    };
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
    inline value_type width() const {
        return size.width;
    };
    inline value_type height() const {
        return size.height;
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

namespace math {

float random();

float random(float min, float max);
} // namespace math