#pragma once

#include "math.hpp"
#include "renderer.hpp"

struct Entity {
    Rect bounds;
    Vec2 velocity;
    int directionX;
    struct Input {
        bool up;
        bool down;
        bool left;
        bool right;
        bool primaryAction;
        bool secondaryAction;
    } input;

    struct Collisions {
        bool up;
        bool down;
        bool left;
        bool right;
    } collisions;

    int sprite;
    const int* animation;

    struct Animations {
        const int* idle;
        const int* walk;
    } animations;

    void update();
    void render(Renderer& renderer) const;
};

struct Projectile {
    Vec2 position;
    Vec2 velocity;
    bool active;

    void update();
    void render(Renderer& renderer) const;
};

struct Tile {
    Rect bounds;
    int sprite;

    void update();
    void render(Renderer& renderer) const;
};
