#include "wasm4.h"

#include <array>
#include <stdio.h>
#include <utility>

#include "app.hpp"
#include "assets.hpp"
#include "entity.hpp"
#include "math.hpp"
#include "object_pool.hpp"
#include "renderer.hpp"
#include "utils.hpp"

struct GameState {
    int health = 3;
    int score = 100;
};

GameState state;

class Gui {
  public:
    void update(){};
    void render(Renderer& renderer) {
        renderer.setViewport(0, 0);

        renderer.useColor(0x0024);
        renderer.draw({{8, 0}, {16 * 3, 14}});
        renderer.draw({{104, 0}, {16 * 3, 14}});

        renderer.useColor(2);
        for (int i = 0; i < state.health; i++) {
            renderer.drawSpriteFrame(assets::SpriteFrame::Hearth, 8 + 16 * i, -1);
        }

        char buffer[8] = {'\0'};
        auto str = to_string(state.score, buffer);
        renderer.drawText({str.begin(), str.end()}, 103 + 8 + (6 - str.size()) * 8, 4);
    };
};

#if !defined(X)

Renderer renderer;
Gui gui;

int frame = 0;
bool debug = false;

constexpr Tile t(int x, int y, int sprite) {
    return {{{x * 16.f, y * 16.f}, {16, 16}}, sprite};
}

const Rect bounds = {{0, 0}, {SCREEN_SIZE, SCREEN_SIZE}};

Entity player{
    {{80, 0}, {16, 16}}, {}, 0, {}, {}, 0, nullptr, {assets::player_idle_animation, assets::player_walk_animation}};
Tile tiles[] = {t(0, 0, 19 + 20 * 14), t(0, 1, 17 + 20 * 14), t(0, 2, 17 + 20 * 14), t(0, 3, 17 + 20 * 14),
                t(0, 4, 17 + 20 * 14), t(0, 5, 17 + 20 * 14), t(0, 6, 17 + 20 * 14), t(0, 7, 17 + 20 * 14),
                t(0, 8, 17 + 20 * 14), t(0, 9, 19 + 20 * 13),

                t(9, 0, 19 + 20 * 11), t(9, 1, 15 + 20 * 14), t(9, 2, 15 + 20 * 14), t(9, 3, 15 + 20 * 14),
                t(9, 4, 15 + 20 * 14), t(9, 5, 15 + 20 * 14), t(9, 6, 15 + 20 * 14), t(9, 7, 15 + 20 * 14),
                t(9, 8, 15 + 20 * 14), t(9, 9, 19 + 20 * 12),

                t(1, 0, 16 + 20 * 15), t(2, 0, 16 + 20 * 15), t(3, 0, 17 + 20 * 15),

                t(6, 0, 15 + 20 * 15), t(7, 0, 16 + 20 * 15), t(8, 0, 16 + 20 * 15),

                t(1, 9, 16 + 20 * 13), t(2, 9, 16 + 20 * 13), t(3, 9, 17 + 20 * 13),

                t(6, 9, 15 + 20 * 13), t(7, 9, 16 + 20 * 13), t(8, 9, 16 + 20 * 13),

                t(3, 6, 84),           t(4, 6, 85),           t(5, 6, 85),           t(6, 6, 86),

                t(1, 3, 85),           t(2, 3, 85),           t(3, 3, 85),           t(4, 3, 86)};

ObjectPool<Projectile, 100> projectilesPool;
ObjectPool<Tile, 124> tilePool;
ObjectPool<Entity, 2> enemies;

void start() {
    // clang-format off
    int map[] = {
        1,1,1,1,1,0,1,1,1,1,1,
        1,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,1,1,1,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,1,
        1,1,1,1,1,0,1,1,1,1,1,
    };
    // clang-format on

    player.directionX = 1;
    renderer.setPalette(assets::palettes::lava_gb);
    renderer.useColor(0x4321);
}

const float padding = 1.5f;
void updateForCollisionX(Entity& entity, const Rect& rect) {
    Rect e = entity.bounds;
    e.origin.y += padding;
    e.size.height -= padding * 2;
    e.origin.x += entity.velocity.x;

    if (e.collision(rect)) {
        if (entity.velocity.x > 0) {
            entity.velocity.x = 0;
            entity.bounds.origin.x = rect.origin.x - e.size.width;
            entity.collisions.right = true;
        } else if (entity.velocity.x < 0) {
            entity.velocity.x = 0;
            entity.bounds.origin.x = rect.origin.x + rect.size.width;
            entity.collisions.left = true;
        }
    }
}

void updateForCollisionY(Entity& entity, const Rect& rect) {
    Rect e = entity.bounds;
    e.origin.x += padding;
    e.size.width -= padding * 2;
    e.origin.y += entity.velocity.y;
    if (e.collision(rect)) {
        if (entity.velocity.y > 0) {
            entity.velocity.y = 0;
            entity.bounds.origin.y = rect.origin.y - e.size.height;
            entity.collisions.down = true;
        } else if (entity.velocity.y < 0) {
            entity.velocity.y = 0;
            entity.bounds.origin.y = rect.origin.y + rect.size.height;
            entity.collisions.up = true;
        }
    }
}

void updatePlayer(Entity& entity) {
    const float maxSpeed = 1.0f;
    const float jumpImpulse = 4.f;
    const float gravity = 0.15f;
    const float acc = 0.15f;

    Vec2& o = entity.bounds.origin;
    Vec2& v = entity.velocity;

    int inputX = (entity.input.right ? 1 : 0) - (entity.input.left ? 1 : 0);
    if (inputX != 0) {
        entity.directionX = inputX;
        entity.animation = entity.animations.walk;
    } else {
        entity.animation = entity.animations.idle;
    }
    bool jump = (entity.input.up);

    v.x = (v.x * (1 - acc)) + ((float)inputX * maxSpeed * acc);
    if (v.x < -maxSpeed) {
        v.x = -maxSpeed;
    } else if (v.x > maxSpeed) {
        v.x = maxSpeed;
    }

    v.y += gravity;

    if (jump) {
        if (entity.collisions.down) {
            v.y = -jumpImpulse;
        } else if (entity.collisions.left) {
            v.y = -jumpImpulse;
            v.x = jumpImpulse;
        } else if (entity.collisions.right) {
            v.y = -jumpImpulse / 1.4f;
            v.x = -jumpImpulse / 1.4f;
        }
    }

    entity.collisions = {false, false, false, false};
    for (const auto& tile : tiles) {
        updateForCollisionY(entity, tile.bounds);
    }
    for (const auto& tile : tiles) {
        updateForCollisionX(entity, tile.bounds);
    }

    o.x += v.x;
    o.y += v.y;

    if (o.y >= SCREEN_SIZE) {
        o.y -= SCREEN_SIZE;
    } else if (o.y + bounds.size.height <= 0) {
        o.y += SCREEN_SIZE;
    }

    if (entity.input.primaryAction) {
        auto handle = projectilesPool.alloc();
        if (handle != projectilesPool.invalid_handle) {
            auto& p = *projectilesPool.get(handle);
            p.position = entity.bounds.origin;
            // p.position.x += entity.bounds.size.width / 2.0f;
            if (entity.directionX > 0) {
                p.position.x += entity.bounds.size.width;
            }
            p.position.y += entity.bounds.size.height / 2.0f;
            p.velocity.x = (float)entity.directionX * 5.0f;
            p.active = true;

            entity.velocity.x -= (float)entity.directionX * 0.1f;
        }
    }

    if (entity.animation && entity.animation[0] > 0) {
        const int index = (frame / 10 % entity.animation[0]) + 1;
        entity.sprite = entity.animation[index];
    }
};

void doUpdate() {
    frame += 1;

    if (frame % (60 * 2) == 0) {
        Entity enemy{{{80, 0}, {16, 16}},
                     {},
                     0,
                     {false, false, true, false, false, false},
                     {},
                     0,
                     nullptr,
                     {assets::enemy_idle_animation, assets::enemy_walk_animation}};
        enemies.create(enemy);
        // enemies.create();
    }

    {
        const uint8_t gamepad = *GAMEPAD1;
        Entity::Input& input = player.input;
        input.up = gamepad & BUTTON_UP;
        input.left = gamepad & BUTTON_LEFT;
        input.right = gamepad & BUTTON_RIGHT;
        input.primaryAction = gamepad & BUTTON_1;
        input.secondaryAction = gamepad & BUTTON_2;
        updatePlayer(player);
    }

    for (auto& enemy : enemies) {
        updatePlayer(enemy);
        if (enemy.collisions.left) {
            enemy.input.left = false;
            enemy.input.right = true;
        } else if (enemy.collisions.right) {
            enemy.input.left = true;
            enemy.input.right = false;
        }
    }

    for (auto& p : projectilesPool) {
        if (!bounds.contains(p.position)) {
            projectilesPool.free(&p);
            continue;
        }

        bool free = false;
        for (auto& e : enemies) {
            if (e.bounds.contains(p.position)) {
                projectilesPool.free(&p);
                enemies.free(&e);
                free = true;
                state.score += 25;
                break;
            }
        }

        for (const auto& tile : tiles) {
            if (tile.bounds.contains(p.position)) {
                projectilesPool.free(&p);
                free = true;
                break;
            }
        }

        if (free) {
            continue;
        }

        p.position.x += p.velocity.x;
        p.position.y += p.velocity.y;
    }

    gui.update();
}

void doRender() {
    renderer.useColor(0x0321);
    renderer.clear(4);

    for (const auto& t : tiles) {
        t.render(renderer);
    }

    for (const auto& p : projectilesPool) {
        renderer.draw(p.position);
    }

    player.render(renderer);
    for (auto& enemy : enemies) {
        enemy.render(renderer);
    }

    gui.render(renderer);
}

void update() {
    doUpdate();
    doRender();
}
#else

App app;

void start() {
    app.start();
}

void update() {
    app.update();
    app.render();
}

#endif