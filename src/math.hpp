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

    const uint32_t& asInt() const {
        return *reinterpret_cast<const uint32_t*>(&this->b);
    }

    Color& operator+=(Color const& other) {
        int i = other.b;
        b = (uint8_t)(b + i);
        i = other.g;
        g = (uint8_t)(g + i);
        i = other.r;
        r = (uint8_t)(r + i);
        i = other.a;
        a = (uint8_t)(a + i);
        return *this;
    }

    Color& operator-=(Color const& other) {
        int i = other.b;
        b = (uint8_t)(b - i);
        i = other.g;
        g = (uint8_t)(g - i);
        i = other.r;
        r = (uint8_t)(r - i);
        i = other.a;
        a = (uint8_t)(a - i);
        return *this;
    }

    Color operator+(const Color& other) const {
        Color out = *this;
        out += other;
        return out;
    };

    Color operator-(const Color& other) const {
        Color out = *this;
        out -= other;
        return out;
    };

    Color operator*(const float& f) const {
        Color out = *this;

        float i = b;
        out.b = (uint8_t)(i * f);
        i = g;
        out.g = (uint8_t)(i * f);
        i = r;
        out.r = (uint8_t)(i * f);
        i = a;
        out.a = (uint8_t)(i * f);

        return out;
    };
    Color operator/(const float& f) const {
        Color out = *this;
        float i = b;
        out.b = (uint8_t)(i / f);
        i = g;
        out.g = (uint8_t)(i / f);
        i = r;
        out.r = (uint8_t)(i / f);
        i = a;
        out.a = (uint8_t)(i / f);
        return out;
    };
};

namespace math {

float random();

float random(float min, float max);
} // namespace math