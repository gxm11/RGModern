// zlib License

// copyright (C) 2023 Guoxiaomi and Krimiston

// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once
#include <array>

#include "cen_library.hpp"
#include "core/core.hpp"

namespace rgm::base {
using controller_axisstate =
    std::array<int, static_cast<size_t>(cen::controller_axis::max) * 8>;

struct controller_axis_reset {
  using data = std::tuple<controller_axisstate>;

  void run(auto& worker) { RGMDATA(controller_axisstate).fill(0); }
};

struct controller_axis_move {
  int joy_index;
  int axis;
  int value;

  void run(auto& worker) {
    controller_axisstate& ca = RGMDATA(controller_axisstate);

    size_t index =
        axis + joy_index * static_cast<int>(cen::controller_axis::max);
    ca.at(index) = value;
  }
};
}  // namespace rgm::base