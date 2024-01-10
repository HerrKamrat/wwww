#pragma once

#include <stdint.h>

namespace assets
{
#include "assets/monochrome_tilemap_packed.hpp"

  namespace palettes
  {
    const inline uint32_t default_gb[4] = {
        0xe0f8cf,
        0x86c06c,
        0x306850,
        0x071821};

    const inline uint32_t ice_cream_gb[4] = {
        0xfff6d3,
        0xf9a875,
        0xeb6b6f,
        0x7c3f58};

    const inline uint32_t lava_gb[4] = {
        0xff8e80,
        0xc53a9d,
        0x4a2480,
        0x051f39};

  }
}