#include "math.hpp"

namespace {
bool overlap(float aMin, float aMax, float bMin, float bMax) {
    return !(aMax < bMin || aMin > bMax);
}

bool overlapX(const Rect& a, const Rect& b) {
    return overlap(a.left(), a.right(), b.left(), b.right());
}

bool overlapY(const Rect& a, const Rect& b) {
    return overlap(a.top(), a.bottom(), b.top(), b.bottom());
}

bool collision(const Rect& a, const Rect& b) {
    return overlapY(a, b) && overlapX(a, b);
}
} // namespace

bool Rect::contains(const Vec2& p) const {
    return p.x >= left() && p.x <= right() && p.y >= top() && p.y <= bottom();
}

bool Rect::collision(const Rect& other) const {
    return ::collision(*this, other);
}
