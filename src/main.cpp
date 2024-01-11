#include "wasm4.h"

#include <string.h>
#include <array>

#include "app.hpp"
#include "assets.hpp"
#include "math.hpp"
#include "entity.hpp"
#include "renderer.hpp"

Renderer renderer;

constexpr uint16_t spriteIndex(uint16_t i, uint16_t j)
{
    return j * 20 + i;
}

enum class SpriteFrame : uint16_t
{
    PlayerDefault = spriteIndex(0, 12),
    PlayerWalk1 = 12 * 20 + 1,
    PlayerWalk2 = 12 * 20 + 2
};

inline const std::array<SpriteFrame, 1> arr = {SpriteFrame::PlayerDefault};

inline const int player_idle_animation[] = {1, (int)SpriteFrame::PlayerDefault};
inline const int player_walk_animation[] = {2, (int)SpriteFrame::PlayerWalk1, (int)SpriteFrame::PlayerWalk2};

int frame = 0;
bool debug = false;

constexpr Tile t(int x, int y, int sprite)
{
    return {{{x * 16.f, y * 16.f}, {16, 16}}, sprite};
}

const Rect bounds = {{0, 0}, {SCREEN_SIZE, SCREEN_SIZE}};

Entity player{{{80, 0}, {16, 16}}, {}, 0, {}, 0, nullptr};
Tile tiles[] = {
    t(0, 0, 19 + 20 * 14),
    t(0, 1, 17 + 20 * 14),
    t(0, 2, 17 + 20 * 14),
    t(0, 3, 17 + 20 * 14),
    t(0, 4, 17 + 20 * 14),
    t(0, 5, 17 + 20 * 14),
    t(0, 6, 17 + 20 * 14),
    t(0, 7, 17 + 20 * 14),
    t(0, 8, 17 + 20 * 14),
    t(0, 9, 19 + 20 * 13),

    t(9, 0, 19 + 20 * 11),
    t(9, 1, 15 + 20 * 14),
    t(9, 2, 15 + 20 * 14),
    t(9, 3, 15 + 20 * 14),
    t(9, 4, 15 + 20 * 14),
    t(9, 5, 15 + 20 * 14),
    t(9, 6, 15 + 20 * 14),
    t(9, 7, 15 + 20 * 14),
    t(9, 8, 15 + 20 * 14),
    t(9, 9, 19 + 20 * 12),

    t(1, 0, 16 + 20 * 15),
    t(2, 0, 16 + 20 * 15),
    t(3, 0, 17 + 20 * 15),

    t(6, 0, 15 + 20 * 15),
    t(7, 0, 16 + 20 * 15),
    t(8, 0, 16 + 20 * 15),

    t(1, 9, 16 + 20 * 13),
    t(2, 9, 16 + 20 * 13),
    t(3, 9, 17 + 20 * 13),

    t(6, 9, 15 + 20 * 13),
    t(7, 9, 16 + 20 * 13),
    t(8, 9, 16 + 20 * 13),

    t(3, 6, 84),
    t(4, 6, 85),
    t(5, 6, 85),
    t(6, 6, 86),

    t(1, 3, 85),
    t(2, 3, 85),
    t(3, 3, 85),
    t(4, 3, 86)};

Projectile projectiles[100] = {};
Entity entities[100] = {};

void start()
{
    player.directionX = 1;

    for (auto &p : projectiles)
    {
        p.active = false;
    }
}

const float padding = 1.5f;
void updateForCollisionX(Entity &entity, const Rect &rect)
{
    Rect e = entity.bounds;
    e.origin.y += padding;
    e.size.height -= padding * 2;
    e.origin.x += entity.velocity.x;

    if (e.collision(rect))
    {
        if (entity.velocity.x > 0)
        {
            entity.velocity.x = 0;
            entity.bounds.origin.x = rect.origin.x - e.size.width;
            entity.collisions.right = true;
        }
        else if (entity.velocity.x < 0)
        {
            entity.velocity.x = 0;
            entity.bounds.origin.x = rect.origin.x + rect.size.width;
            entity.collisions.left = true;
        }
    }
}

void updateForCollisionY(Entity &entity, const Rect &rect)
{
    Rect e = entity.bounds;
    e.origin.x += padding;
    e.size.width -= padding * 2;
    e.origin.y += entity.velocity.y;
    if (e.collision(rect))
    {
        if (entity.velocity.y > 0)
        {
            entity.velocity.y = 0;
            entity.bounds.origin.y = rect.origin.y - e.size.height;
            entity.collisions.down = true;
        }
        else if (entity.velocity.y < 0)
        {
            entity.velocity.y = 0;
            entity.bounds.origin.y = rect.origin.y + rect.size.height;
            entity.collisions.up = true;
        }
    }
}

void updatePlayer()
{
    const float maxSpeed = 2.0f;
    const float jumpImpulse = 4.f;
    const float gravity = 0.15f;
    const float acc = 0.15f;

    Vec2 &o = player.bounds.origin;
    Vec2 &v = player.velocity;

    const uint8_t gamepad = *GAMEPAD1;
    int inputX = (gamepad & BUTTON_RIGHT ? 1 : 0) - (gamepad & BUTTON_LEFT ? 1 : 0);
    if (inputX != 0)
    {
        player.directionX = inputX;
        player.animation = player_walk_animation;
    }
    else
    {
        player.animation = player_idle_animation;
    }
    bool jump = (gamepad & BUTTON_UP);

    v.x = (v.x * (1 - acc)) + ((float)inputX * maxSpeed * acc);
    if (v.x < -maxSpeed)
    {
        v.x = -maxSpeed;
    }
    else if (v.x > maxSpeed)
    {
        v.x = maxSpeed;
    }

    v.y += gravity;

    if (jump)
    {
        if (player.collisions.down)
        {
            v.y = -jumpImpulse;
        }
        else if (player.collisions.left)
        {
            v.y = -jumpImpulse;
            v.x = jumpImpulse;
        }
        else if (player.collisions.right)
        {
            v.y = -jumpImpulse / 1.4f;
            v.x = -jumpImpulse / 1.4f;
        }
    }

    player.collisions = {false, false, false, false};
    for (const auto &tile : tiles)
    {
        updateForCollisionY(player, tile.bounds);
    }
    for (const auto &tile : tiles)
    {
        updateForCollisionX(player, tile.bounds);
    }

    o.x += v.x;
    o.y += v.y;

    if (gamepad & BUTTON_1)
    {
        for (Projectile &p : projectiles)
        {
            if (!p.active)
            {
                p.position = player.bounds.origin;
                // p.position.x += player.bounds.size.width / 2.0f;
                if (player.directionX > 0)
                {
                    p.position.x += player.bounds.size.width;
                }
                p.position.y += player.bounds.size.height / 2.0f;
                p.velocity.x = (float)player.directionX * 5.0f;
                p.active = true;

                player.velocity.x -= (float)player.directionX * 0.1f;
                break;
            }
        }
    }

    if (player.animation && player.animation[0] > 0)
    {
        const int index = (frame / 10 % player.animation[0]) + 1;
        player.sprite = player.animation[index];
    }
};

void update()
{
    frame += 1;

    renderer.setPalette(assets::palettes::default_gb);

    updatePlayer();

    for (Projectile &p : projectiles)
    {
        if (p.active)
        {
            if (!bounds.contains(p.position))
            {
                p.active = false;
                continue;
            }

            for (const auto &tile : tiles)
            {
                if (tile.bounds.contains(p.position))
                {
                    p.active = false;
                    break;
                }
            }

            if (!p.active)
            {
                continue;
            }
            p.position.x += p.velocity.x;
            p.position.y += p.velocity.y;
        }
    }

    renderer.clear(4);

    for (const auto &t : tiles)
    {
        t.render(renderer);
    }

    for (const Projectile &p : projectiles)
    {
        if (p.active)
            renderer.draw(p.position);
    }

    player.render(renderer);
#if 0
    if (*GAMEPAD1 & BUTTON_2)
    {
        debug = !debug;
    }

    if (debug)
    {
        bool c = false;
        for (auto &tile : tiles)
        {
            c = c || player.bounds.collision(tile.bounds);
        }
        if (c)
        {
            renderer.drawText("Collision", 10, 10);
        }
        else
        {
            renderer.drawText("No Collision", 10, 10);
        }
        renderer.drawText(!player.collisions.up ? "[ ]" : "[x]", 25, 20);
        renderer.drawText(!player.collisions.left ? "[ ]" : "[x]", 10, 30);
        renderer.drawText(!player.collisions.right ? "[ ]" : "[x]", 40, 30);
        renderer.drawText(!player.collisions.down ? "[ ]" : "[x]", 25, 40);

        renderer.drawSpriteFrame(2 + 2 * 20, 10, 50);
    }
#endif
}

#if 0

App app;

void start()
{
    app.start();
}

void update()
{
    app.update();
}

#endif
