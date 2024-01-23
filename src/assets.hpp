#pragma once

#include "wasm4.h"
#include <array>
#include <stdint.h>

namespace assets {
// #include "assets/monochrome_tilemap_packed.hpp"
#include "assets/tilemap.hpp"

constexpr uint16_t spriteIndex(uint16_t i, uint16_t j) {
    return j * 20 + i;
}

enum class SpriteFrame : uint16_t {
    PlayerDefault = spriteIndex(0, 12),
    PlayerWalk1 = spriteIndex(1, 12), // 12 * 20 + 1,
    PlayerWalk2 = spriteIndex(2, 12), // 12 * 20 + 2,

    EnemyDefault = spriteIndex(0, 16),
    EnemyWalk1 = spriteIndex(1, 16),
    EnemyWalk2 = spriteIndex(2, 16),

    Hearth = spriteIndex(1, 2),
};

inline const std::array<SpriteFrame, 1> arr = {SpriteFrame::PlayerDefault};

inline const int player_idle_animation[] = {1, (int)SpriteFrame::PlayerDefault};
inline const int player_walk_animation[] = {2, (int)SpriteFrame::PlayerWalk1, (int)SpriteFrame::PlayerWalk2};

inline const int enemy_idle_animation[] = {1, (int)SpriteFrame::EnemyDefault};
inline const int enemy_walk_animation[] = {2, (int)SpriteFrame::EnemyWalk1, (int)SpriteFrame::EnemyWalk2};

namespace palettes {

const inline uint32_t default_gb[4] = {0xe0f8cf, 0x86c06c, 0x306850, 0x071821};

const inline uint32_t ice_cream_gb[4] = {0xfff6d3, 0xf9a875, 0xeb6b6f, 0x7c3f58};

const inline uint32_t lava_gb[4] = {0xff8e80, 0xc53a9d, 0x4a2480, 0x051f39};

} // namespace palettes
} // namespace assets