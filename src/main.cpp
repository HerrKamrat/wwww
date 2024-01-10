#include "wasm4.h"

#include <string.h>
#include <array>

#include "app.hpp"
#include "assets.hpp"
#include "math.hpp"
#include "entity.hpp"

// #include "assets/monochrome_tilemap_packed.hpp"

constexpr uint16_t spriteIndex(uint16_t i, uint16_t j)
{
    return j * 20 + i;
}

enum class SpriteFrame : uint16_t
{
    PlayerDefault = 12 * 20 + 0,
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
    return {{{x * 16.f, y * 16.f}, {16, 8}}, sprite};
}

const int32_t width = 160;
const int32_t height = 160;
const Rect bounds = {{0, 0}, {width, height}};

void setPalette(const uint32_t palette[4])
{
    PALETTE[0] = palette[0];
    PALETTE[1] = palette[1];
    PALETTE[2] = palette[2];
    PALETTE[3] = palette[3];
}

void clear(uint8_t color)
{
    uint8_t i = color - 1;
    memset(FRAMEBUFFER, i | (i << 2) | (i << 4) | (i << 6), width * height / 4);
}

void useColor(uint16_t i)
{
    *DRAW_COLORS = i;
}

void blitSpriteFrame(int index, int x, int y, uint32_t flags = BLIT_1BPP)
{
    const uint32_t srcW = 16;
    const uint32_t srcH = 16;
    const uint32_t stride = assets::tilemapWidth;
    const uint32_t srcX = ((uint32_t)index % 20) * srcW;
    const uint32_t srcY = ((uint32_t)index / 20) * srcH;

    blitSub(assets::tilemap, x, y, srcW, srcH, srcX, srcY, stride, flags);
}

void draw(const Vec2 &v)
{
    rect((int)v.x, (int)v.y, 1, 1);
}

void draw(const Rect &r)
{
    rect((int)r.origin.x, (int)r.origin.y, (uint32_t)r.size.width, (uint32_t)r.size.height);
}

Entity player{{{80, 0}, {16, 16}}, {}, 0, {}, 0, nullptr};
Tile tiles[] = {
    t(1, 3, 85),
    t(2, 3, 85),
    t(3, 3, 85),
    t(4, 3, 85),

    t(4, 6, 85),
    t(5, 6, 85),
    t(6, 6, 85),

    t(1, 9, 85),
    t(2, 9, 85),
    t(3, 9, 85),
    t(4, 9, 85),
};

Projectile projectiles[100] = {};

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
/*
bool outsideBounds(const Vec2 &p)
{
    return p.x < 0 || p.x > width || p.y < 0 || p.y > height;
}

bool inside(const Vec2 &p, const Rect &r)
{
    return p.x >= r.origin.x && p.x <= r.origin.x + r.size.width && p.y >= r.origin.y && p.y <= r.origin.y + r.size.height;
}
*/
void update()
{
    frame += 1;

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

    *DRAW_COLORS = 1;

    clear(4);

    setPalette(assets::palettes::default_gb);

    for (const auto &r : tiles)
    {
        blitSpriteFrame(r.sprite, (int)r.bounds.origin.x, (int)r.bounds.origin.y);
    }

    for (const Projectile &p : projectiles)
    {
        if (p.active)
            draw(p.position);
    }

    useColor(1);
    uint32_t flag = BLIT_1BPP;
    if (player.directionX < 0)
    {
        flag |= BLIT_FLIP_X;
    }
    blitSpriteFrame(player.sprite, (int)player.bounds.origin.x, (int)player.bounds.origin.y, flag);
    // blit(smiley, x + 1 + player.directionX, y + 1, 8, 8, BLIT_1BPP);
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
            text("Collision", 10, 10);
        }
        else
        {
            text("No Collision", 10, 10);
        }
        text(!player.collisions.up ? "[ ]" : "[x]", 25, 20);
        text(!player.collisions.left ? "[ ]" : "[x]", 10, 30);
        text(!player.collisions.right ? "[ ]" : "[x]", 40, 30);
        text(!player.collisions.down ? "[ ]" : "[x]", 25, 40);

        blitSpriteFrame(2 + 2 * 20, 10, 50);
    }
    /*
        useColor(1);
        rect(20, 16 * 5 + 10, 16, 16);
        useColor(2);

        blitSpriteFrame(4 * 20 + 5, 20, 16 * 5 + 10);
    */
    // blit(tilemap, 0, 0, tilemapWidth, tilemapHeight, BLIT_1BPP);
    /*{
        int frames[] = {0, 1};
        int x = 16 + 16 * (frame / 10 % 2 + 1);
        int y = 16 * 13;
        int w = 16;
        int h = 16;
        int stride = tilemapWidth;

        useColor(1);
        blitSub(tilemap, 10, 50, w, h, x, y, stride, BLIT_1BPP);
    }*/
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
