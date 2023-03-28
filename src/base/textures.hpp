// zlib License

// copyright (C) 2023 Xiaomi Guo and Krimiston

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
#include <cstdint>
#include <unordered_map>

#include "cen_library.hpp"
#include "core/core.hpp"

namespace rgm::base {
/**
 * @brief 管理所有 texture 的哈希表，继承自 unordered_map 以区分不同的类型。
 */
using textures = std::unordered_map<uint64_t, cen::texture>;

/** @brief 将 textures 类型的变量添加到 worker 的 datalist 中 */
struct init_textures {
  using data = std::tuple<textures>;

  static void after(auto& worker) { RGMDATA(textures).clear(); }
};
}  // namespace rgm::base