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
#include <set>

#include "base/base.hpp"
#include "drawable.hpp"
#include "table.hpp"

namespace rgm::rmxp {
struct tilemap_info {
  std::vector<uint16_t> x_cache;
  std::vector<uint16_t> y_cache;
  const tilemap* p_tilemap;
  uint64_t id;
  int z;
  int current_index;
  int max_index;

  void setup(z_index zi, const tilemap& t, const table& map_data,
             const table& priorities) {
    id = zi.id;
    z = zi.z;
    p_tilemap = &t;

    x_cache.resize(map_data.x_size, 0);
    y_cache.resize(map_data.y_size, 0);

    current_index = 1;

    max_index = map_data.y_size;
    for (int x_index = 0; x_index < map_data.x_size; ++x_index) {
      for (int y_index = 0; y_index < map_data.y_size; ++y_index) {
        for (int z_index = 0; z_index < map_data.z_size; ++z_index) {
          int16_t tileid = map_data.get(x_index, y_index, z_index);
          int16_t priority = 0;
          if (tileid >= 0 && static_cast<size_t>(tileid) < priorities.data.size()) {
            priorities.get(tileid);
          }
          if (max_index < (y_index + priority)) {
            max_index = y_index + priority;
          }
          if (priority <= 0) continue;
          uint16_t flag = 1;
          flag <<= ((priority >= 16) ? 15 : (priority - 1));
          x_cache[x_index] = x_cache[x_index] | flag;
          y_cache[y_index] = y_cache[y_index] | flag;
        }
      }
    }
    max_index += 1;
  }

  // 返回 0 表示不再产生新的层，否则返回新层的 index
  int next_index(z_index zi) {
    int layer_z = current_z();
    if (layer_z > zi.z) return 0;
    if ((layer_z == zi.z && id < zi.id)) return 0;
    ++current_index;
    return current_index - 1;
  }

  int current_z() const { return current_index * 32 + 32 + z - p_tilemap->oy; }

  // is_valid 返回 false 时不可能再产生新的层
  bool is_valid() const { return current_index < max_index; }

  // 这两个函数供 render 时调用，快速跳过特定的行和列。
  // diff = index - y_index
  // 需要先判断 is_valid_y，后判断 is_valid_x。
  // 显然输入的 x_index 和 y_index 不能小于 0，在外部保证。
  bool is_valid_x(int x_index, int diff) const {
    uint16_t flag = x_cache[x_index];

    if (diff >= 16) return flag & 0x8000;
    return flag & (1 << (diff - 1));
  }

  bool is_valid_y(int y_index, int diff) const {
    if (diff <= 0) return false;
    uint16_t flag = y_cache[y_index];

    if (diff >= 16) return flag & 0x8000;
    return flag & (1 << (diff - 1));
  }
};

struct tilemap_manager {
  tables* p_tables;
  // infos 在逻辑线程中写，渲染线程中读
  // infos 内的元素寿命和 drawables 一样
  // 添加和删除元素都在 drawable_create 和 drawable_dispose 中
  std::map<uint64_t, tilemap_info> infos;
  std::array<std::set<z_index>, 2> layers;
  using result_t = std::pair<tilemap_info*, int>;

  void setup(z_index zi, const tilemap& t, size_t depth = 0) {
    if (t.skip()) return;

    tilemap_info& ti = infos[zi.id];
    ti.setup(zi, t, p_tables->at(t.map_data), p_tables->at(t.priorities));

    layers[depth].insert(zi);
  }

  // 当返回的 result_t.index 为 0 时，generator 中止
  auto next_layer(z_index zi, size_t depth = 0)
      -> std::pair<tilemap_info*, int> {
    std::set<z_index>& s = layers[depth];
    if (s.empty()) return {nullptr, 0};

    auto node = s.extract(s.begin());
    tilemap_info& front = infos[node.value().id];
    if (!front.is_valid()) return {nullptr, 0};
    // get_index changes the state of `front` !
    int index = front.next_index(zi);
    node.value().z = front.current_z();
    s.insert(std::move(node));

    return {&front, index};
  }
};

struct init_tilemap_manager {
  using data = std::tuple<tilemap_manager>;
  static void before(auto& worker) {
    tilemap_manager& tm = RGMDATA(tilemap_manager);
    tm.p_tables = &(RGMDATA(tables));
  }
};
}  // namespace rgm::rmxp