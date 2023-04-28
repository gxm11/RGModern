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
#include "render_base.hpp"
#include "table.hpp"
#include "tilemap_manager.hpp"

namespace rgm::rmxp {
/// @brief 辅助绘制 tilemap 的类，提供了对图块的迭代等函数
/// @tparam is_overlayer 是否在绘制 overlayer
/// tilemap 本体和 overlayer 的绘制的内容区别很小，主要是根据各图块的优先级
/// 来决定图块绘制在哪一层。此外，本体那层还需要绘制图块的闪烁。
template <bool is_overlayer>
struct render_tilemap_helper {
  /// @brief tilemap 数据的地址
  const tilemap* p_tilemap;

  /// @brief 储存了地图的图块数据的 table 的地址
  const table* p_map;

  /// @brief 储存了图块优先级数据的 table 的地址
  const table* p_priorities;

  /// @brief 管理 tilemap 多层数据的 info 对象的地址
  const tilemap_info* p_info;

  /// @brief 当前绘制的层级，从 0 开始计数
  int overlayer_index;

  /// @brief 构造函数
  explicit render_tilemap_helper(const tilemap* t, const tables* p_tables,
                                 const tilemap_info* info = nullptr,
                                 int index = 0)
      : p_tilemap(t),
        p_map(&p_tables->at(t->map_data)),
        p_priorities(&p_tables->at(t->priorities)),
        p_info(info),
        overlayer_index(index) {}

  /// @brief 设置自动元件对应的 texture 并返回保存它们的数组
  /// @param 管理所有 textures 的容器
  /// @return 返回保存各 texture 指针的数组
  auto make_autotiles(base::textures& textures)
      -> std::array<cen::texture*, autotiles::max_size> {
    /* 作为返回值的数组 */
    std::array<cen::texture*, autotiles::max_size> autotile_textures;

    for (size_t i = 0; i < autotiles::max_size; ++i) {
      uint64_t id = p_tilemap->autotiles.m_data[i];
      if (id) {
        autotile_textures[i] = &(textures.at(id + 1));
        if constexpr (!is_overlayer) {
          autotile_textures[i]->set_blend_mode(cen::blend_mode::blend);
        }
      } else {
        autotile_textures[i] = nullptr;
      }
    }

    return autotile_textures;
  }

  void iterate_tiles(std::function<void(int, int, int, int)> proc) {
    const viewport* p_viewport =
        p_tilemap->p_viewport ? p_tilemap->p_viewport : &default_viewport;

    int width = p_viewport->rect.width;
    int height = p_viewport->rect.height;

    // 左上角绘制的位置，和在table中的索引
    int start_x = (-p_viewport->ox - p_tilemap->ox) % 32;
    if (start_x > 0) start_x -= 32;
    int start_x_index = (start_x - (-p_viewport->ox - p_tilemap->ox)) / 32;

    int start_y = (-p_viewport->oy - p_tilemap->oy) % 32;
    if (start_y > 0) start_y -= 32;
    int start_y_index = (start_y - (-p_viewport->oy - p_tilemap->oy)) / 32;

    int y_index = start_y_index - 1;
    for (int y = start_y; y < height; y += 32) {
      ++y_index;

      if (!p_tilemap->repeat_y && (y_index < 0 || y_index >= p_map->y_size))
        continue;

      y_index = y_index % p_map->y_size;
      if (y_index < 0) y_index += p_map->y_size;

      if constexpr (is_overlayer) {
        if (p_info->skip_row(y_index, overlayer_index - y_index)) continue;
      }

      int x_index = start_x_index - 1;
      for (int x = start_x; x < width; x += 32) {
        ++x_index;

        if (!p_tilemap->repeat_x && (x_index < 0 || x_index >= p_map->x_size))
          continue;

        x_index = x_index % p_map->x_size;
        if (x_index < 0) x_index += p_map->x_size;

        if constexpr (is_overlayer) {
          if (p_info->skip_column(x_index, overlayer_index - y_index)) continue;
        }
        proc(x, y, x_index, y_index);
      }
    }
  }

  auto make_render_proc(cen::renderer& renderer, const cen::texture& tileset,
                        auto& autotile_textures)
      -> std::function<void(int, int, int, int)> {
    auto render = [&](int x, int y, int x_index, int y_index) {
      cen::irect dst_rect(x, y, 32, 32);
      cen::irect src_rect(0, 0, 32, 32);

      for (int z_index = 0; z_index < p_map->z_size; ++z_index) {
        const int16_t tileid = p_map->get(x_index, y_index, z_index);
        if (tileid <= 0) continue;
        if (static_cast<size_t>(tileid) >= p_priorities->size()) continue;
        const int16_t priority = p_priorities->get(tileid);

        if constexpr (is_overlayer) {
          if (priority != overlayer_index - y_index) continue;
        } else {
          if (priority != 0) continue;
        }

        if (tileid >= 384) {
          int x = tileid % 8 * 32;
          int y = (tileid - 384) / 8 * 32;
          x = x + 256 * (y / config::tileset_texture_height);
          y = y % config::tileset_texture_height;
          src_rect.set_position(x, y);
          renderer.render(tileset, src_rect, dst_rect);
        } else {
          size_t autotile_index = tileid / 48 - 1;
          cen::texture* p_autotile = autotile_textures.at(autotile_index);
          if (!p_autotile) continue;

          int x = (p_tilemap->update_count / 16 * 32) % p_autotile->width();
          int y = (p_autotile->height() == 32) ? 0 : tileid % 48 * 32;
          src_rect.set_position(x, y);
          renderer.render(*p_autotile, src_rect, dst_rect);
        }
      }
    };
    return render;
  }
};

template <>
struct render<tilemap> {
  const tilemap* t;
  // const viewport* v;
  const tables* p_tables;
  // tilemap 的第 0 层，也就是 z = 0 的那一层
  // 绘制所有优先级为 0 的图块，然后绘制闪烁
  // 不需要创建新的 viewport 而是直接画满整个画面或viewport
  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::textures& textures = RGMDATA(base::textures);

    cen::texture& tileset = textures.at(t->tileset);
    tileset.set_blend_mode(cen::blend_mode::blend);

    render_tilemap_helper<false> helper(t, p_tables);

    auto autotile_textures = helper.make_autotiles(textures);

    auto render = helper.make_render_proc(renderer, tileset, autotile_textures);

    helper.iterate_tiles(render);

    if (t->flash_data) {
      renderer.set_blend_mode(blend_type::add);

      const table& map = p_tables->at(t->map_data);
      const table& flash = p_tables->at(t->flash_data);

      auto render = [&](int x, int y, int x_index, int y_index) {
        if (x_index < 0) return;
        if (x_index >= map.x_size) return;
        if (y_index < 0) return;
        if (y_index >= map.y_size) return;

        cen::irect dst_rect(x, y, 32, 32);

        const int16_t color = flash.get(x_index, y_index, 0);
        if (color) {
          uint8_t red = (color & 0xf00) >> 4;
          uint8_t green = (color & 0x0f0);
          uint8_t blue = (color & 0x00f) << 4;
          uint8_t alpha = abs(16 - (t->update_count % 32)) * 8 + 32;
          renderer.set_color({red, green, blue, alpha});
          renderer.fill_rect(dst_rect);
        }
      };

      helper.iterate_tiles(render);
    }
  }
};

template <>
struct render<overlayer<tilemap>> {
  const tilemap_info* info;
  // const viewport* v;
  const tables* p_tables;
  int overlayer_index;

  void run(auto& worker) {
    const tilemap* t = info->p_tilemap;

    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::textures& textures = RGMDATA(base::textures);

    cen::texture& tileset = textures.at(t->tileset);
    tileset.set_blend_mode(cen::blend_mode::blend);

    render_tilemap_helper<true> helper(t, p_tables, info, overlayer_index);

    auto autotile_textures = helper.make_autotiles(textures);
    auto render = helper.make_render_proc(renderer, tileset, autotile_textures);

    helper.iterate_tiles(render);
  }
};
}  // namespace rgm::rmxp