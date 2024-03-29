#pragma once

#include "math.hpp"
#include "renderer.hpp"

struct Entity {
    // private:
    int team;
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

        void updateForGamepad(uint8_t gamepad);
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

    bool visible = true;
    int invulnerable = 0;

    void update();
    void render(Renderer& renderer) const;
};

struct Projectile {
    int team;
    Vec2 position;
    Vec2 velocity;
    bool active;

    void update();
    void render(Renderer& renderer) const;

  private:
    Vec2 previousPosition;
};

struct Tile {
    Rect bounds;
    int sprite;

    void update();
    void render(Renderer& renderer) const;
};
