#include "wasm4.h"

#include <string.h>
#include <vector>

#include "math.hpp"

#include "assets/monochrome_tilemap_packed.hpp"

const int player_default = 12 * 20 + 0;
const int player_walk_1 = 12 * 20 + 1;
const int player_walk_2 = 12 * 20 + 2;

const int player_idle_animation[] = {1, player_default};
const int player_walk_animation[] = {2, player_walk_1, player_walk_2};

int frame = 0;

struct Entity
{
    Rect bounds;
    Vec2 velocity;
    int directionX;
    struct Collisions
    {
        bool up;
        bool down;
        bool left;
        bool right;
    } collisions;

    int sprite;
    const int *animation;
};

struct Projectile
{
    Vec2 position;
    Vec2 velocity;
    bool active;
};

const uint8_t smiley[] = {
    0b11000011,
    0b10000001,
    0b00100100,
    0b00100100,
    0b00000000,
    0b00100100,
    0b10011001,
    0b11000011,
};

const uint32_t default_palette[4] = {
    0xe0f8cf,
    0x86c06c,
    0x306850,
    0x071821};

const uint32_t ice_cream_gb_palette[4] = {
    0xfff6d3,
    0xf9a875,
    0xeb6b6f,
    0x7c3f58};

const uint32_t lava_gb_palette[4] = {
    0xff8e80,
    0xc53a9d,
    0x4a2480,
    0x051f39};

const int32_t width = 160;
const int32_t height = 160;

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
    const uint32_t stride = tilemapWidth;
    const uint32_t srcX = ((uint32_t)index % 20) * srcW;
    const uint32_t srcY = ((uint32_t)index / 20) * srcH;

    blitSub(tilemap, x, y, srcW, srcH, srcX, srcY, stride, flags);

    /*{
        int frames[] = {0, 1};
        int x = 16 + 16 * (frame / 10 % 2 + 1);
        int y = 16 * 13;
        int w = 16;
        int h = 16;
        int stride = tilemapWidth;

        useColor(1);
        blitSub(tilemap, 10, 50, w, h, x, y, stride, BLIT_1BPP);
    }
    */
}

void draw(const Projectile &p)
{
    rect((int)p.position.x, (int)p.position.y, 1, 1);
}

void draw(const Rect &r)
{
    rect((int)r.origin.x, (int)r.origin.y, (uint32_t)r.size.width, (uint32_t)r.size.height);
}

Entity player{{{80, 0}, {16, 16}}, {}, 0, {}, 0, nullptr};
Rect tile = {{10, 10}, {10, 10}};
Rect tiles[] = {
    {{0, 0}, {1, 160}},
    {{159, 0}, {10, 160}},
    {{0, 159}, {160, 1}},
    {{0, 159}, {160, 1}},

    {{30, 50}, {100, 5}},
    {{0, 90}, {60, 5}},
    {{100, 90}, {60, 5}},
    {{30, 130}, {100, 5}},

    /*
    {{60, 130}, {10, 10}},
    {{20, 40}, {10, 100}},
    //    {{70, 139}, {5, 1}},
    {{100, 110}, {10, 10}},
    {{110, 120}, {10, 10}},
    {{10, 140}, {140, 10}}
    */
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

bool overlap(float aMin, float aMax, float bMin, float bMax)
{
    return !(aMax < bMin || aMin > bMax);
}

bool overlapX(const Rect &a, const Rect &b)
{
    return overlap(a.origin.x, a.origin.x + a.size.width, b.origin.x, b.origin.x + b.size.width);
}

bool overlapY(const Rect &a, const Rect &b)
{
    return overlap(a.origin.y, a.origin.y + a.size.height, b.origin.y, b.origin.y + b.size.height);
}

bool collision(const Rect &a, const Rect &b)
{
    return overlapY(a, b) && overlapX(a, b);
}
const float padding = 1.5f;
void updateForCollisionX(Entity &entity, const Rect &rect)
{
    Rect e = entity.bounds;
    e.origin.y += padding;
    e.size.height -= padding * 2;
    e.origin.x += entity.velocity.x;
    // e.origin.y += entity.velocity.y;
    if (collision(e, rect))
    {
        if (entity.velocity.x > 0)
        {
            entity.velocity.x = 0;
            entity.bounds.origin.x = rect.origin.x - e.size.width;
            entity.collisions.right = true;

            // entity.grounded = true;
        }
        else if (entity.velocity.x < 0)
        {
            entity.velocity.x = 0;
            entity.bounds.origin.x = rect.origin.x + rect.size.width;
            // entity.grounded = true;
            entity.collisions.left = true;
        }
    }
}

void updateForCollisionY(Entity &entity, const Rect &rect)
{
    Rect e = entity.bounds;
    // e.origin.x += entity.velocity.x;
    e.origin.x += padding;
    e.size.width -= padding * 2;
    e.origin.y += entity.velocity.y;
    if (collision(e, rect))
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
    const float jumpImpulse = 3.5f;
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
        updateForCollisionY(player, tile);
    }
    for (const auto &tile : tiles)
    {
        updateForCollisionX(player, tile);
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

bool outsideBounds(const Vec2 &p)
{
    return p.x < 0 || p.x > width || p.y < 0 || p.y > height;
}

bool inside(const Vec2 &p, const Rect &r)
{
    return p.x >= r.origin.x && p.x <= r.origin.x + r.size.width && p.y >= r.origin.y && p.y <= r.origin.y + r.size.height;
}

void update()
{
    frame += 1;

    updatePlayer();

    for (Projectile &p : projectiles)
    {
        if (p.active)
        {
            if (outsideBounds(p.position))
            {
                p.active = false;
                continue;
            }

            for (const auto &tile : tiles)
            {
                if (inside(p.position, tile))
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

    setPalette(lava_gb_palette);
    auto &x = player.bounds.origin.x;
    auto &y = player.bounds.origin.y;
    /*uint8_t gamepad = *GAMEPAD1;
    if (gamepad & BUTTON_1)
    {
        setPalette(ice_cream_gb_palette);
    }
    if (gamepad & BUTTON_2)
    {
        setPalette(lava_gb_palette);
    }

    if (gamepad & BUTTON_LEFT)
    {
        x -= 1;
    }
    if (gamepad & BUTTON_RIGHT)
    {
        x += 1;
    }
    if (gamepad & BUTTON_UP)
    {
        y -= 1;
    }
    if (gamepad & BUTTON_DOWN)
    {
        y += 1;
    }*/
#if 0
    rect(0, 0, 40, 160);
    *DRAW_COLORS = 2;
    rect(40, 0, 40, 160);
    *DRAW_COLORS = 3;
    rect(80, 0, 40, 160);
    *DRAW_COLORS = 4;
    rect(120, 0, 40, 160);

    *DRAW_COLORS = 2;
    text("Hello from C!", 10, 10);

    blit(smiley, 76, 76, 8, 8, BLIT_1BPP);

    text("Press X to blink", 16, 90);
#endif

    for (const Rect &r : tiles)
    {
        draw(r);
    }

    for (const Projectile &p : projectiles)
    {
        // if (p.active)
        draw(p);
    }
    // draw(tile);

    useColor(1);
    draw(player.bounds);
    useColor(2);
    uint32_t flag = BLIT_1BPP;
    if (player.directionX < 0)
    {
        flag |= BLIT_FLIP_X;
    }
    blitSpriteFrame(player.sprite, (int)player.bounds.origin.x, (int)player.bounds.origin.y, flag);
    // blit(smiley, x + 1 + player.directionX, y + 1, 8, 8, BLIT_1BPP);

    bool c = false;
    for (auto &tile : tiles)
    {
        c = c || collision(player.bounds, tile);
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
    blitSpriteFrame(13 * 20 + 5, 10, 50 + 16);

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
