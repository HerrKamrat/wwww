#include "wasm4.h"

#include "app.hpp"
#include "assets.hpp"
#include "entity.hpp"
#include "math.hpp"
#include "object_pool.hpp"
#include "renderer.hpp"
#include "utils.hpp"
#include <array>
#include <utility>

float light = 0.0f;

struct GameState {
    int health = 3;
    int score = 0;
    struct Camera {
        Vec2 position{0, 0};
        Vec2 velocity{0, 0};
    } camera;
};

struct World {
    const Rect bounds = {{0, 0}, {SCREEN_SIZE, SCREEN_SIZE}};
    ObjectPool<Tile, 124> tiles;
    ObjectPool<Entity, 100> entities;
    ObjectPool<Projectile, 100> projectiles;
};

GameState state;
World world;

struct UpdateContext {
    int frame = 0;
};

UpdateContext updateContext;

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
        renderer.drawText({str.begin(), str.end()}, 103 + 8 + (6 - (int)str.size()) * 8, 4);
    };
};

class Physics {
  public:
    void update(){};
    void render(){};
};

#if !defined(X)

Renderer renderer;
Gui gui;

decltype(World::entities)::Handle player;

void start() {
    {
        int width = 10;
        int height = 11;
        int map[] = {
            // clang-format off
            219,316,316,317,  0,  0,315,316,316,239,
            297,  0,  0,  0,  0,  0,  0,  0,  0,295,
            297,  0,  0,  0,  0,  0,  0,  0,  0,295,
            297,  0,  0, 84, 85, 85, 86,  0,  0,295,
            297,  0,  0,  0,  0,  0,  0,  0,  0,295,
            297,  0,  0,  0,  0,  0, 84, 85, 85,295,
            297, 85, 85, 86,  0,  0,  0,  0,  0,295,
            297,  0,  0,  0,  0,  0,  0,  0,  0,295,
            297,  0,  0, 84, 85, 85, 86,  0,  0,295,
            297,  0,  0,  0,  0,  0,  0,  0,  0,295,
            199,276,276,277,  0,  0,275,276,276,259,
            // clang-format on

        };
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                const int type = map[y * width + x];
                if (type == 0) {
                    continue;
                }
                float h = 16;
                if (type < 100) {
                    h = 8;
                }
                // TODO: fix camera offset
                world.tiles.create(Rect{{x * 16.f, y * 16.f - 8.0f}, {16, h}}, type);
            }
        }
    }

    player = world.entities.create(Entity{1,
                                          {{16 * 4.5f, 16 * 5.0f}, {16, 16}},
                                          {},
                                          1,
                                          {},
                                          {},
                                          0,
                                          nullptr,
                                          {assets::player_idle_animation, assets::player_walk_animation}});

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

int lastPrimaryActionFrame = 0;
int primaryActionInterval = 30;
void updateEntity(Entity& entity, World& world) {
    const float maxSpeed = 1.0f;
    const float jumpImpulse = 4.f;
    const float gravity = 0.15f;
    const float acc = 0.15f;

    if (entity.invulnerable > 0) {
        entity.invulnerable -= 1;
    }

    entity.visible = (entity.invulnerable % 8) < 4;

    Rect& b = entity.bounds;
    Vec2& o = entity.bounds.origin;
    Vec2& v = entity.velocity;

    int inputX = (entity.input.right ? 1 : 0) - (entity.input.left ? 1 : 0);
    if (inputX != 0) {
        entity.directionX = inputX;
        entity.animation = entity.animations.walk;
    } else {
        entity.animation = entity.animations.idle;
    }
    bool jump = (entity.input.up || entity.input.secondaryAction);

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

    o += v;

    if (b.top() >= world.bounds.bottom()) {
        o.y -= world.bounds.height();
    } else if (b.bottom() <= world.bounds.top()) {
        o.y += world.bounds.height();
    }

    if (entity.input.primaryAction && (lastPrimaryActionFrame + primaryActionInterval < updateContext.frame)) {
        lastPrimaryActionFrame = updateContext.frame;
        light += 0.2f;
        for (int i = 0; i < 5; i++) {
            auto handle = world.projectiles.alloc();
            if (handle != world.projectiles.invalid_handle) {
                auto& p = *world.projectiles.get(handle);
                p.team = entity.team;
                p.position = entity.bounds.origin;
                // p.position.x += entity.bounds.size.width / 2.0f;
                if (entity.directionX > 0) {
                    p.position.x += entity.bounds.size.width;
                }
                p.position.y += entity.bounds.size.height / 2.0f;
                p.velocity.x = (float)entity.directionX * 5.0f;
                p.velocity.y = math::random(-1.0f, 1.0f);
                state.camera.velocity += p.velocity * 0.1f;
                p.active = true;

                entity.velocity.x -= (float)entity.directionX * 0.1f;
            }
        }
    }

    if (entity.animation && entity.animation[0] > 0) {
        const int index = (updateContext.frame / 10 % entity.animation[0]) + 1;
        entity.sprite = entity.animation[index];
    }

    entity.collisions = {false, false, false, false};
    for (const auto& tile : world.tiles) {
        updateForCollisionY(entity, tile.bounds);
    }
    for (const auto& tile : world.tiles) {
        updateForCollisionX(entity, tile.bounds);
    }
};

Color c[4] = {{255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}};
void doUpdate() {
    updateContext.frame += 1;
    auto& camera = state.camera;

    if (light > 0) {
        light -= 1.0f / 30;
        if (light < 0) {
            light = 0;
        }
    }
    Color white = {255, 255, 255, 255};
    auto p = assets::palettes::lava_gb;
    for (int i = 0; i < 4; i++) {
        c[i] = p[i] * (1.0f - light) + white * (light);
    }
    renderer.setPalette(c);

    camera.position += camera.velocity;
    camera.velocity = camera.velocity * 0.9f - camera.position * 0.1f;

    if (updateContext.frame % (60 * 2) == 0) {
        bool left = math::random() < 0.5;
        Entity enemy{2,
                     {{16 * 4.5f, 0}, {16, 16}},
                     {},
                     0,
                     {false, false, left, !left, false, false},
                     {},
                     0,
                     nullptr,
                     {assets::enemy_idle_animation, assets::enemy_walk_animation}};
        world.entities.create(enemy);
    }

    auto playerEntity = world.entities.get(player);

    if (playerEntity) {
        const uint8_t gamepad = *GAMEPAD1;
        playerEntity->input.updateForGamepad(gamepad);

        if (playerEntity->invulnerable <= 0) {
            for (auto& entity : world.entities) {
                if (&entity == playerEntity) {
                    continue;
                }

                if (entity.bounds.collision(playerEntity->bounds)) {
                    state.health -= 1;

                    playerEntity->invulnerable = 45;
                    bool left = entity.bounds.left() < playerEntity->bounds.left();

                    playerEntity->velocity.x += left ? 5 : -5;

                    if (state.health <= 0) {
                        world.entities.free(player);
                    }
                    break;
                }
            }
        }
    }

    for (auto& entity : world.entities) {
        updateEntity(entity, world);
        if (entity.collisions.left) {
            entity.input.left = false;
            entity.input.right = true;
        } else if (entity.collisions.right) {
            entity.input.left = true;
            entity.input.right = false;
        }
    }

    for (auto& p : world.projectiles) {

        p.update();
        if (!world.bounds.contains(p.position)) {
            world.projectiles.free(&p);
            continue;
        }

        bool free = false;
        for (auto& e : world.entities) {
            if (e.team == p.team) {
                continue;
            }
            if (e.bounds.contains(p.position)) {
                world.projectiles.free(&p);
                world.entities.free(&e);
                free = true;
                state.score += 25;
                break;
            }
        }

        for (const auto& tile : world.tiles) {
            if (tile.bounds.contains(p.position)) {
                world.projectiles.free(&p);
                free = true;
                break;
            }
        }

        if (free) {
            continue;
        }
    }

    gui.update();
}

void doRender() {
    renderer.useColor(0x0321);
    renderer.clear(4);
    renderer.setViewport(state.camera.position.x, state.camera.position.y);

    for (const auto& t : world.tiles) {
        t.render(renderer);
    }

    for (const auto& p : world.projectiles) {
        p.render(renderer);
    }

    for (auto& entity : world.entities) {
        entity.render(renderer);
    }

    gui.render(renderer);

    renderer.useColor(1);
    renderer.draw({{0, 159}, {1, 1}});
    renderer.useColor(2);
    renderer.draw({{1, 159}, {1, 1}});
    renderer.useColor(3);
    renderer.draw({{2, 159}, {1, 1}});
    renderer.useColor(4);
    renderer.draw({{3, 159}, {1, 1}});
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