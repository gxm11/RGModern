// zlib License
//
// copyright (C) 2023 Guoxiaomi and Krimiston
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once
#include "core/core.hpp"

namespace rgm::base {
/// @brief 最多支持的 controller 的数量
constexpr size_t controller_maxsize = 8;

/// @brief 存储 controller 的 axis state 的数组
/// @name data
/// controller 的 axis 一般是摇杆或者扳机键，值一般在 -32768 ~ 32767 之间。
using controller_axisstate =
    std::array<int, static_cast<size_t>(cen::controller_axis::max) *
                        controller_maxsize>;

/// @brief controller 插入时的回调事件
/// @name task
struct controller_connect {
  using data = std::tuple<controller_axisstate>;

  int joy_index;

  void run(auto& worker) {
    cen::log_warn("[Input] Controller %d is connected.", joy_index);

    /* 重置所有 controller 的 axis state */
    RGMDATA(controller_axisstate).fill(0);
  }
};

/// @brief controller 拔出时的回调事件
/// @name task
struct controller_disconnect {
  using data = std::tuple<controller_axisstate>;

  int joy_index;

  void run(auto& worker) {
    cen::log_warn("[Input] Controller %d is disconnected.", joy_index);

    /* 重置所有 controller 的 axis state */
    RGMDATA(controller_axisstate).fill(0);
  }
};
}  // namespace rgm::base