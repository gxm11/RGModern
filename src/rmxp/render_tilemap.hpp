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
/// tilemap 本体和 overlayer 的绘制的内容区别很小，主要是根据各图块的优先级
/// 来决定图块绘制在哪一层。此外，本体那层还需要绘制图块的闪烁。
struct render_tilemap_helper {
  /// @brief tilemap 数据的地址
  const tilemap* p_tilemap;

  /// @brief 储存了地图的图块数据的 table 的地址
  const table* p_map;

  /// @brief 储存了图块优先级数据的 table 的地址
  const table* p_priorities;

  /// @brief 管理 tilemap 多层数据的 info 对象的地址
  const tilemap_info* p_info;

  /// @brief 当前绘制的层级
  int layer_index;

  /// @brief 构造函数
  explicit render_tilemap_helper(const tilemap* t, const tables* p_tables,
                                 const tilemap_info* info, int index)
      : p_tilemap(t),
        p_map(&p_tables->at(t->map_data)),
        p_priorities(&p_tables->at(t->priorities)),
        p_info(info),
        layer_index(index) {}

  /// @brief 对属于当前 layer 的图块进行迭代
  /// @param proc 接受 4 个整型参数的 proc 处理图片的回调
  /// proc 的 4 个参数分别是 x, y, x_index, y_index，代表图块的数据
  /// 其中 x 和 y 是在 viewport 上此图块的左上角坐标，
  /// x_index 和 y_index 是此图块在 tilemap 中的横、纵格子位置。
  void iterate_tiles(std::function<void(int, int, int, int)> proc) {
    /* 获取 viewport，如果不存在则使用 default_viewport */
    const viewport* p_viewport =
        p_tilemap->p_viewport ? p_tilemap->p_viewport : &default_viewport;

    /* 获取 viewport 的长宽，tilemap 会平铺此 viewport */
    int width = p_viewport->rect.width;
    int height = p_viewport->rect.height;

    /*
     * 首先找到左上角第一个可能需要绘制的图块，
     * 这个图块的左上角坐标为(start_x, start_y)，
     * 这个图块在 tilemap 中的位置为 (start_x_index, start_y_index)。
     */
    int start_x = (-p_viewport->ox - p_tilemap->ox) % 32;
    if (start_x > 0) start_x -= 32;
    int start_x_index = (start_x - (-p_viewport->ox - p_tilemap->ox)) / 32;

    int start_y = (-p_viewport->oy - p_tilemap->oy) % 32;
    if (start_y > 0) start_y -= 32;
    int start_y_index = (start_y - (-p_viewport->oy - p_tilemap->oy)) / 32;

    /* 对所有可能需要绘制的图块进行迭代，直到超出 viewport 的范围 */
    int y_index = start_y_index - 1;
    for (int y = start_y; y < height; y += 32) {
      ++y_index;

      /* 如果 tilemap 在 y 方向上不是重复的，则跳过超出 tilemap 的范围 */
      if (!p_tilemap->repeat_y && (y_index < 0 || y_index >= p_map->y_size))
        continue;

      /* 求出 y_index 对 y_size 的余数，范围在 0 ~ y_size - 1 之间 */
      y_index = y_index % p_map->y_size;
      if (y_index < 0) y_index += p_map->y_size;

      /* 利用优先级数据概况跳过本行，第 0 层始终不跳过 */
      if (layer_index > 0 && p_info->skip_row(y_index, layer_index - y_index))
        continue;

      int x_index = start_x_index - 1;
      for (int x = start_x; x < width; x += 32) {
        ++x_index;

        /* 如果 tilemap 在 x 方向上不是重复的，则跳过超出 tilemap 的范围 */
        if (!p_tilemap->repeat_x && (x_index < 0 || x_index >= p_map->x_size))
          continue;

        /* 求出 x_index 对 x_size 的余数，范围在 0 ~ x_size - 1 之间 */
        x_index = x_index % p_map->x_size;
        if (x_index < 0) x_index += p_map->x_size;

        /* 利用优先级数据概况跳过本列，第 0 层始终不跳过 */
        if (layer_index > 0 &&
            p_info->skip_column(x_index, layer_index - y_index))
          continue;

        /* 处理输入的回调函数 proc */
        proc(x, y, x_index, y_index);
      }
    }
  }

  /// @brief 创建 tilemap 单个图片的 render 函数
  /// @param renderer 渲染器
  /// @param tileset tilemap 使用的图片素材
  /// @return render 函数，在 iterate_tiles 中调用
  auto make_render_proc(cen::renderer& renderer, const cen::texture& tileset)
      -> std::function<void(int, int, int, int)> {
    /* render 的 4 个参数的含义详见 iterate_tiles */
    auto render = [&](int x, int y, int x_index, int y_index) {
      /* 目标矩形，代表图块将绘制到 viewport 上的位置 */
      cen::irect dst_rect(x, y, 32, 32);

      /* 源矩形，代表图块在 tileset 上或者 autotiles 上的位置 */
      cen::irect src_rect(0, 0, 32, 32);

      /* 对 tilemap 的全部层依次处理 */
      for (int z_index = 0; z_index < p_map->z_size; ++z_index) {
        /* 获取图块的 tileid */
        const int16_t tileid = p_map->get(x_index, y_index, z_index);

        /* 图块的 tileid = 0 是透明图块，跳过绘制 */
        if (tileid == 0) continue;

        /*
         * 图块的 tileid < 0 也跳过绘制。这可能是数据出现了错误。
         */
        if (tileid < 0) {
          cen::log_info(
              "The tile at <%d, %d> has invalid tileid %d, which should "
              "greater than or equal to 0.",
              x_index, y_index);
          continue;
        }

        /*
         * 图块的 tileid 没有对应的优先级数据也跳过绘制
         * 这可能是在绘制完地图后，更换了更短的 tileset 导致的。
         */
        if (static_cast<size_t>(tileid) >= p_priorities->size()) {
          cen::log_info(
              "The tile at <%d, %d> has invalid tileid %d, which should less "
              "than %lld.",
              x_index, y_index, p_priorities->size());
          continue;
        }

        /* 获取此图块的优先级数据 */
        const int16_t priority = p_priorities->get(tileid);

        /* priority 需要和 layer_index 等匹配，否则此图块应在其他层绘制 */
        if (layer_index == 0 && priority != 0) continue;

        if (layer_index > 0 && priority != layer_index - y_index) continue;

        if (tileid >= 384) {
          /* 普通图块的场合，从 tileset 上提取 */
          int x = tileid % 8 * 32;
          int y = (tileid - 384) / 8 * 32;

          /*
           * texture 的长和宽是有限的，一般是 16384 或者 32768。为了支持更大的
           * tileset，实际操作中以 config::tileset_texture_height = 8192 为
           * 单位进行分割。然后横向拼接起来。分割的这部分工作使用了 Palette 类，
           * 在 ruby 中执行，参考 ./src/scripts/rpgcache.rb。
           *
           * 比如说一个高 30000，宽 256 的 tileset，对应的 texture 大小应该是高
           * 8192，宽 1024。由于 tileid 是 int16_t，最大不超过 32767，那么最终
           * 拼合而成的 texture 宽度也不会超过 4096，就不会超出图块大小限制。
           */
          x = x + 256 * (y / config::tileset_texture_height);
          y = y % config::tileset_texture_height;
          src_rect.set_position(x, y);

          renderer.render(tileset, src_rect, dst_rect);
        } else {
          /* 自动元件的场合，从 autotiles 上提取 */

          /* 查找此图块使用的 autotile */
          size_t autotile_index = tileid / 48 - 1;
          const cen::texture* p_autotile =
              p_info->autotile_textures.at(autotile_index);

          /* autotile 不存在的情况下，跳过绘制 */
          if (!p_autotile) continue;

          /* 实现 autotile 的动画效果 */
          int x = (p_tilemap->update_count / 16 * 32) % p_autotile->width();

          /*
           * 对于高度为 32 的单行 autotile，只存在一种模式。
           * 否则，根据不同的 tileid 绘制不同的模式。
           */
          int y = (p_autotile->height() == 32) ? 0 : tileid % 48 * 32;

          src_rect.set_position(x, y);

          renderer.render(*p_autotile, src_rect, dst_rect);
        }
      }
    };
    return render;
  }
};

/// @brief 绘制 tilemap
/// @name task
/// tilemap 的不同层都在这里绘制，只是第 0 层需要绘制闪烁效果。
template <>
struct render<overlayer<tilemap>> {
  /// @brief 管理 tilemap 多层数据的 info 对象的地址
  const tilemap_info* info;

  /// @brief 管理所有 table 的容器的指针。
  const tables* p_tables;

  /// @brief 当前绘制的层级
  int layer_index;

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::textures& textures = RGMDATA(base::textures);

    /* 获取 tilemap 的数据 */
    const tilemap* t = info->p_tilemap;

    /* 创建 render_tilemap_helper 对象辅助绘制 */
    render_tilemap_helper helper(t, p_tables, info, layer_index);

    cen::texture& tileset = textures.at(t->tileset);
    auto render = helper.make_render_proc(renderer, tileset);

    /* 逐个绘制图块 */
    helper.iterate_tiles(render);

    /* 处理闪烁的效果 */
    if (layer_index == 0 && t->flash_data) {
      renderer.set_blend_mode(blend_type::add);

      const table& flash = p_tables->at(t->flash_data);

      auto render = [&](int x, int y, int x_index, int y_index) {
        /* 读取闪烁的颜色 */
        const int16_t color = flash.get(x_index, y_index, 0);

        if (color) {
          /* 设置绘制的目标区域 */
          cen::irect dst_rect(x, y, 32, 32);

          /* 将 int16_t 的数据转换成 rgb */
          uint8_t red = (color & 0xf00) >> 4;
          uint8_t green = (color & 0x0f0);
          uint8_t blue = (color & 0x00f) << 4;

          /* 设置闪烁的效果，会随着时间改变透明度 */
          uint8_t alpha = std::abs(16 - (t->update_count % 32)) * 8 + 32;

          /* 直接将颜色绘制到画面上 */
          renderer.set_color({red, green, blue, alpha});
          renderer.fill_rect(dst_rect);
        }
      };

      helper.iterate_tiles(render);
    }
  }
};
}  // namespace rgm::rmxp