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
#include "base/base.hpp"
#include "drawable.hpp"
#include "table.hpp"

namespace rgm::rmxp {
/// @brief 管理 tilemap 多层数据
/// RMXP 的 tilemap 是由很多层数据绘制而成的。每个图块的高度与它的优先级
/// 有关，z = y + 优先级 * 32 + 32，或者在特定的情况下 z = 0。由于 z 与
/// y 有关，那么大多数时候不需要完整地遍历整个 table，可以缓存一部分结果。
/// tilemap_info 的作用就是辅助 render<overlayer<tilemap>>> 以加速绘制。
struct tilemap_info {
  /// @brief 储存地图图块每列的优先级数据概况。
  /// x_cache 通常大小是 map 的 x_size，第 a 个值记录的是 x = a 的那一列
  /// 所有图块的优先级数据概况。若此列存在优先级为 b 的图块，则：x_cache[a]
  /// 从低往高数第 b 位为 1。从而，x_cache[a] 中的第 b 位代表第 a 列是否存
  /// 在优先级为 b + 1 的图块。特别地，当 b = 15 时，代表的是此列是否存在图块的
  /// 优先级 >= 16。一般情况下优先级不会大于 5，使用 uint16_t 足够。
  std::vector<uint16_t> x_cache;

  /// @brief 储存地图图块每行的优先级数据概况。
  /// 同 x_cache 类似，y_cache[a] 中的第 b 位代表第 a 行是否存在优先级为
  /// b + 1 的图块。
  std::vector<uint16_t> y_cache;

  /// @brief 关联的 tilemap 数据
  const tilemap* p_tilemap;

  /// @brief 关联的 tilemap 的 id
  uint64_t tilemap_id;

  /// @brief 关联的 tilemap 的 id
  int tilemap_z;

  /// @brief 当前 overlayer 的 index
  int current_index;

  /// @brief 该 tilemap 的 overlayer 的最大 index
  /// 取决于所有图块的 z 的最大值。
  int max_index;

  /// @brief 储存所有自动元件的 texture 的容器
  std::vector<cen::texture*> autotile_textures;

  /// @brief 设置自身的各个属性
  /// @param zi tilemap 的 z_index
  /// @param t 关联的 tilemap 对象
  /// @param map_data 储存了地图的图块数据的 table
  /// @param priorities 储存了图块优先级数据的 table
  void setup(z_index zi, const tilemap& t, const table& map_data,
             const table& priorities) {
    tilemap_id = zi.id;
    tilemap_z = zi.z;
    p_tilemap = &t;

    current_index = 1;
    max_index = map_data.y_size;

    x_cache.resize(map_data.x_size, 0);
    y_cache.resize(map_data.y_size, 0);

    /* 遍历 map_data，设置 x_cache，y_cache 和 max_index 的值 */
    for (int x_index = 0; x_index < map_data.x_size; ++x_index) {
      for (int y_index = 0; y_index < map_data.y_size; ++y_index) {
        for (int z_index = 0; z_index < map_data.z_size; ++z_index) {
          int16_t priority = 0;
          /* 获取 tileid */
          int16_t tileid = map_data.get(x_index, y_index, z_index);

          /* 在不越界的情况下读取 priority */
          if (tileid > 0 && static_cast<size_t>(tileid) < priorities.size()) {
            priority = priorities.get(tileid);
          }

          /* 设置 max_index */
          if (max_index < y_index + priority) {
            max_index = y_index + priority;
          }

          /* priority = 0 属于平凡情况，不需要操作 */
          if (priority <= 0) continue;

          /* 操作 x_cache 和 y_cache 的单个 bit */
          uint16_t flag = 1;
          flag <<= ((priority >= 16) ? 15 : (priority - 1));

          x_cache[x_index] = x_cache[x_index] | flag;
          y_cache[y_index] = y_cache[y_index] | flag;
        }
      }
    }

    /* 遵循前闭后开原则，current_index 的值要始终小于 max_index */
    ++max_index;
  }

  /// @brief 在 z_index 前是否存在可以绘制的 overlayer
  /// @param zi 下一个要绘制的 drawable 的 z_index
  /// @return 返回新层的 index，若不能插入绘制新的层，则返回 0
  /// 查看是否要插空绘制一层 tilemap 的 overlayer。zi 有另外一种形式，
  /// 即 {INTMAX, 0}，此时任意的 overlayer 都满足绘制条件 z < INTMAX。
  [[nodiscard]] int next_index(z_index zi) {
    z_index current_zi{current_z(), tilemap_id};

    /* 若下一个需要绘制的 overlayer 的 z_index 更大，则返回 0*/
    if (current_zi > zi) return 0;

    /* 等价于 return current_index++; 但是这样写更好懂 */
    ++current_index;
    return current_index - 1;
  }

  /// @brief current_index 对应的 z 值
  /// @return current_index 对应的 z 值
  int current_z() const {
    return current_index * 32 + 32 + tilemap_z - p_tilemap->oy;
  }

  /// @brief 如果还能产生新的层，则判断为非空，空的对象需要删除。
  /// @return 如果还能产生新的层则返回 true，否则返回 false。
  bool empty() const { return current_index >= max_index; }

  /*
   * 这两个函数供 render 时调用，快速跳过特定的行和列。
   * 需要先判断 skip_row，后判断 skip_column。
   * diff = index - y_index。
   * 显然输入的 x_index 和 y_index 不能小于 0，在外部保证。
   */
  bool skip_column(int x_index, int diff) const {
    // return false;

    if (diff <= 0) return true;
    uint16_t flag = x_cache[x_index];

    if (diff >= 16) return (flag & 0x8000) == 0;
    return (flag & (1u << (diff - 1))) == 0;
  }

  bool skip_row(int y_index, int diff) const {
    // return false;

    if (diff <= 0) return true;
    uint16_t flag = y_cache[y_index];

    if (diff >= 16) return (flag & 0x8000) == 0;
    return (flag & (1u << (diff - 1))) == 0;
  }
};

/// @brief 设置 tilemap_info
/// @name task
struct tilemap_set_info {
  /// @brief tilemap 数据的地址
  const tilemap* p_tilemap;

  /// @brief tilemap_info 数据的地址
  tilemap_info* p_info;

  void run(auto& worker) {
    base::textures& textures = RGMDATA(base::textures);

    /* 设置 autotile 对应的 textures */
    auto& autotile_textures = p_info->autotile_textures;
    autotile_textures.clear();

    for (size_t i = 0; i < p_tilemap->autotiles.m_data.size(); ++i) {
      if (i == autotiles::max_size) break;

      uint64_t id = p_tilemap->autotiles.m_data[i];

      if (id == 0) {
        autotile_textures.push_back(nullptr);
      } else {
        cen::texture& t = textures.at(id + 1);
        t.set_blend_mode(cen::blend_mode::blend);

        autotile_textures.push_back(&t);
      }
    }

    /* 设置 tileset 的混合模式 */
    cen::texture& tileset = textures.at(p_info->p_tilemap->tileset);
    tileset.set_blend_mode(cen::blend_mode::blend);
  }
};

/// @brief 管理所有的 tilemap 的 overlayer 的绘制的类
/// @name data
/// 支持多个 tilemap 互相嵌套。
struct tilemap_manager {
  /// @brief 管理所有 table 的容器的指针。
  tables* p_tables;

  /// @brief 存储所有的 tilemap_info 的容器
  /// infos 在逻辑线程中写，渲染线程中读。
  /// infos 内的元素寿命和 tilemap 一样，添加和删除 infos 分别在
  /// drawable_create 和 drawable_dispose 中。
  std::map<uint64_t, tilemap_info> infos;

  /// @brief 分层管理所有的 tilemap 的 z_index
  /// layers[0] 储存无 viewport 的 tilemap，
  /// layers[1] 储存有 viewport 的 tilemap。
  std::array<std::set<z_index>, 2> layers;

  /// @brief 添加一个 tilemap
  /// @param t 需要添加的 tilemap
  tilemap_info& insert(const tilemap& t) {
    /* 读取 zi*/
    z_index zi;

    /*下一行代码只能在 ruby 的 worker 中安全读取数据 */
    zi << t.ruby_object;

    /* 设置 tilemap_info，考虑到 tilemap 除了 id 之外都可能变化了 */
    tilemap_info& ti = infos[zi.id];
    ti.setup(zi, t, p_tables->at(t.map_data), p_tables->at(t.priorities));

    /* 添加到 layers 中管理，根据是否在 viewport 中分不同的 layer 管理 */
    size_t depth = t.p_viewport ? 1 : 0;
    layers[depth].insert(zi);

    return ti;
  }

  /// @brief 返回下一个可绘制的 overlayer 层
  /// @return 返回下一层的 tilemap_info 和 index，不存在则返回 std::nullopt
  auto next_layer(z_index zi, size_t depth = 0)
      -> std::optional<std::pair<tilemap_info*, int>> {
    /* layer 中什么也没有则返回空 */
    std::set<z_index>& s = layers[depth];
    if (s.empty()) return std::nullopt;

    /* 读取 layer 中的第一个元素，获取最小的 z_index */
    z_index front_zi = *s.begin();

    /* 读取对应的 tilemap_info */
    tilemap_info& front_info = infos.at(front_zi.id);

    /* 如果 tilemap_info 不能产生新的层，移除 front_zi 并返回空 */
    if (front_info.empty()) {
      s.erase(s.begin());
      return std::nullopt;
    }

    /* 查看是否有下一层 */
    int index = front_info.next_index(zi);

    /* index = 0 说明在当前范围不能产生新的层，需要等下一个更大的 z 值 */
    if (index == 0) return std::nullopt;

    /* 将 front_zi 的 z 值改为 tilemap_info 当前的值 */
    s.erase(s.begin());
    s.insert({front_info.current_z(), front_zi.id});

    /* 返回 tilemap_info 的指针和 index */
    return std::pair{&front_info, index};
  }
};

/// @brief tilemap_manager 相关的初始化类
/// @name task
struct init_tilemap_manager {
  /* 引入数据类型 tilemap_manager */
  using data = std::tuple<tilemap_manager>;

  static void before(auto& worker) {
    tilemap_manager& tm = RGMDATA(tilemap_manager);

    /* 将 tables 的指针保存为 tilemap_manager 的成员变量 */
    tm.p_tables = &(RGMDATA(tables));
  }
};
}  // namespace rgm::rmxp