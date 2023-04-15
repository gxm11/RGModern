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
/// @brief 存储所有 cen::texture，即位图（Bitmap）对象的类
/// @name data
using textures = std::unordered_map<uint64_t, cen::texture>;

/// @brief 数据类 textures 相关的初始化类
/// @name task
struct init_textures {
  using data = std::tuple<textures>;

  static void after(auto& worker) { RGMDATA(textures).clear(); }
};
}  // namespace rgm::base