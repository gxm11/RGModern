// Copyright (c) 2022 Xiaomi Guo
// Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan
// PSL v2. You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
// Mulan PSL v2 for more details.

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