#include "wasm4.h"

#include <array>
#include <utility>

#include "app.hpp"
#include "assets.hpp"
#include "math.hpp"
#include "entity.hpp"
#include "renderer.hpp"

template <typename T, size_t size = 100>
struct ObjectPool
{
    using Handle = uint32_t;
    const static Handle invalid_handle = 0;

    struct Slot
    {
        uint16_t check;
        T object;
    };

    struct Iterator
    {
        Iterator(Slot *ptr, Slot *begin, Slot *end) : ptr(ptr), begin(begin), end(end)
        {
        }

        Slot *ptr, *begin, *end;
        T &operator*() const
        {
            return ptr->object;
        }
        bool operator!=(const Iterator &b) const
        {
            return ptr != b.ptr;
        }

        Iterator &operator++()
        {
            do
            {
                ptr++;
            } while (ptr < end && ((ptr->check & 0x1) == 0));
            return *this;
        }
    };

    Slot objects[size];

    Iterator begin()
    {
        auto begin = std::begin(objects);
        auto end = std::end(objects);
        Iterator it{begin - 1, begin, end};
        return ++it;
    }
    Iterator end()
    {
        auto begin = std::begin(objects);
        auto end = std::end(objects);
        return {end, begin, end};
    }

    Handle alloc()
    {
        auto begin = std::begin(objects);
        auto end = std::end(objects);
        auto slot = std::find_if(std::begin(objects), std::end(objects), [](const Slot &slot)
                                 { return (slot.check & 0x1) == 0; });
        if (slot == std::end(objects))
        {
            trace("no available object in pool");
            return invalid_handle;
        }

        uint16_t check = slot->check = slot->check + 1;
        uint16_t index = (uint16_t)std::distance(begin, slot);

        return (Handle)(check << 16 | index);
    };

    template <typename... Args>
    Handle create(Args &&...args)
    {
        auto handle = alloc();
        if (handle)
        {
            auto ptr = get(handle);
            T t{std::forward<Args>(args)...};
            *ptr = T{std::forward<Args>(args)...};
        }
        return handle;
    };

    Slot *getSlot(Handle handle)
    {
        uint16_t index = (uint16_t)handle;
        uint16_t check = (uint16_t)(handle >> 16);

        if ((check & 0x1) == 0)
        {
            return nullptr;
        }

        Slot &slot = objects[index];
        if (slot.check != check)
        {
            return nullptr;
        }

        return &slot;
    }

    void free(T *obj)
    {
        const char *ptr = reinterpret_cast<const char *>(obj);
        const char *first = reinterpret_cast<const char *>(&std::begin(objects)->object);

        if (ptr < first)
        {
            return;
        }

        auto s = sizeof(Slot);
        auto d = static_cast<decltype(s)>(std::distance(first, ptr));
        auto i = d / s;

        if (i >= 0 && i < size && (objects[i].check & 0x1) != 0)
        {
            objects[i].check += 1;
        }
    };

    void free(Handle handle)
    {
        auto slot = getSlot(handle);
        if (slot)
        {
            slot->check += 1;
        }
    };

    T *get(Handle handle)
    {
        auto slot = getSlot(handle);
        if (slot)
        {
            return &(slot->object);
        }
        tracef("get: %d, no slot", handle);
        return nullptr;
    };
};

Renderer renderer;

constexpr uint16_t spriteIndex(uint16_t i, uint16_t j)
{
    return j * 20 + i;
}

enum class SpriteFrame : uint16_t
{
    PlayerDefault = spriteIndex(0, 12),
    PlayerWalk1 = spriteIndex(1, 12), // 12 * 20 + 1,
    PlayerWalk2 = spriteIndex(2, 12), // 12 * 20 + 2,

    EnemyDefault = spriteIndex(0, 16),
    EnemyWalk1 = spriteIndex(1, 16),
    EnemyWalk2 = spriteIndex(2, 16),

};

inline const std::array<SpriteFrame, 1> arr = {SpriteFrame::PlayerDefault};

inline const int player_idle_animation[] = {1, (int)SpriteFrame::PlayerDefault};
inline const int player_walk_animation[] = {2, (int)SpriteFrame::PlayerWalk1, (int)SpriteFrame::PlayerWalk2};

inline const int enemy_idle_animation[] = {1, (int)SpriteFrame::EnemyDefault};
inline const int enemy_walk_animation[] = {2, (int)SpriteFrame::EnemyWalk1, (int)SpriteFrame::EnemyWalk2};

int frame = 0;
bool debug = false;

constexpr Tile t(int x, int y, int sprite)
{
    return {{{x * 16.f, y * 16.f}, {16, 16}}, sprite};
}

const Rect bounds = {{0, 0}, {SCREEN_SIZE, SCREEN_SIZE}};

Entity player{{{80, 0}, {16, 16}}, {}, 0, {}, {}, 0, nullptr, {player_idle_animation, player_walk_animation}};
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

ObjectPool<Projectile, 100> projectilesPool;
ObjectPool<Tile, 124> tilePool;
ObjectPool<Entity, 100> enemies;

void start()
{
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

void updatePlayer(Entity &entity)
{
    const float maxSpeed = 1.0f;
    const float jumpImpulse = 4.f;
    const float gravity = 0.15f;
    const float acc = 0.15f;

    Vec2 &o = entity.bounds.origin;
    Vec2 &v = entity.velocity;

    int inputX = (entity.input.right ? 1 : 0) - (entity.input.left ? 1 : 0);
    if (inputX != 0)
    {
        entity.directionX = inputX;
        entity.animation = entity.animations.walk;
    }
    else
    {
        entity.animation = entity.animations.idle;
    }
    bool jump = (entity.input.up);

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
        if (entity.collisions.down)
        {
            v.y = -jumpImpulse;
        }
        else if (entity.collisions.left)
        {
            v.y = -jumpImpulse;
            v.x = jumpImpulse;
        }
        else if (entity.collisions.right)
        {
            v.y = -jumpImpulse / 1.4f;
            v.x = -jumpImpulse / 1.4f;
        }
    }

    entity.collisions = {false, false, false, false};
    for (const auto &tile : tiles)
    {
        updateForCollisionY(entity, tile.bounds);
    }
    for (const auto &tile : tiles)
    {
        updateForCollisionX(entity, tile.bounds);
    }

    o.x += v.x;
    o.y += v.y;

    if (o.y >= SCREEN_SIZE)
    {
        o.y -= SCREEN_SIZE;
    }
    else if (o.y + bounds.size.height <= 0)
    {
        o.y += SCREEN_SIZE;
    }

    if (entity.input.primaryAction)
    {
        auto handle = projectilesPool.alloc();
        if (handle != projectilesPool.invalid_handle)
        {
            auto &p = *projectilesPool.get(handle);
            p.position = entity.bounds.origin;
            // p.position.x += entity.bounds.size.width / 2.0f;
            if (entity.directionX > 0)
            {
                p.position.x += entity.bounds.size.width;
            }
            p.position.y += entity.bounds.size.height / 2.0f;
            p.velocity.x = (float)entity.directionX * 5.0f;
            p.active = true;

            entity.velocity.x -= (float)entity.directionX * 0.1f;
        }
    }

    if (entity.animation && entity.animation[0] > 0)
    {
        const int index = (frame / 10 % entity.animation[0]) + 1;
        entity.sprite = entity.animation[index];
    }
};

void update()
{
    frame += 1;

    if (frame % (60 * 2) == 0)
    {
        Entity enemy{{{80, 0}, {16, 16}}, {}, 0, {false, false, true, false, false, false}, {}, 0, nullptr, {enemy_idle_animation, enemy_walk_animation}};
        enemies.create(enemy);
        // enemies.create();
    }

    // renderer.setPalette(assets::palettes::default_gb);
    {
        const uint8_t gamepad = *GAMEPAD1;
        Entity::Input &input = player.input;
        input.up = gamepad & BUTTON_UP;
        input.left = gamepad & BUTTON_LEFT;
        input.right = gamepad & BUTTON_RIGHT;
        input.primaryAction = gamepad & BUTTON_1;
        input.secondaryAction = gamepad & BUTTON_2;
        updatePlayer(player);
    }

    for (auto &enemy : enemies)
    {
        updatePlayer(enemy);
        if (enemy.collisions.left)
        {
            enemy.input.left = false;
            enemy.input.right = true;
        }
        else if (enemy.collisions.right)
        {
            enemy.input.left = true;
            enemy.input.right = false;
        }
    }

    for (auto &p : projectilesPool)
    {
        if (!bounds.contains(p.position))
        {
            projectilesPool.free(&p);
            continue;
        }

        bool free = false;
        for (auto &e : enemies)
        {
            if (e.bounds.contains(p.position))
            {
                projectilesPool.free(&p);
                enemies.free(&e);
                free = true;
                break;
            }
        }

        for (const auto &tile : tiles)
        {
            if (tile.bounds.contains(p.position))
            {
                projectilesPool.free(&p);
                free = true;
                break;
            }
        }

        if (free)
        {
            continue;
        }

        p.position.x += p.velocity.x;
        p.position.y += p.velocity.y;
    }

    renderer.useColor(0x0321);
    renderer.clear(4);

    for (const auto &t : tiles)
    {
        t.render(renderer);
    }

    for (const Projectile &p : projectilesPool)
    {
        if (p.active)
            renderer.draw(p.position);
    }

    player.render(renderer);
    for (auto &enemy : enemies)
    {
        enemy.render(renderer);
    }

    renderer.setViewport(0, 0);

    /*renderer.useColor(1);
    renderer.draw({{0, 0}, {16, 16}});
    renderer.useColor(2);
    renderer.draw({{16, 0}, {16, 16}});
    renderer.useColor(3);
    renderer.draw({{32, 0}, {16, 16}});
    renderer.useColor(4);
    renderer.draw({{48, 0}, {16, 16}});
*/

    renderer.drawSpriteFrame(0, 0, 0);
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