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
#include "builtin.hpp"
#include "drawable_object.hpp"
#include "overlayer.hpp"

namespace rgm::rmxp {
/// @brief 对应于 RGSS 中的 Sprite 类
/// The sprite class. Sprites are the basic concept used to display
/// characters, etc. on the game screen.
struct sprite : drawable_object<sprite> {
  static constexpr std::string_view name = "sprite";

  /* 以下对应于 Sprite 中对象类型的属性的成员变量 */
  color flash_color;
  color color;
  tone tone;
  rect src_rect;

  /* 以下对应于 Sprite 中 ID 类型的属性的成员变量 */
  uint64_t bitmap;

  /* 以下对应于 Sprite 中值类型的属性的成员变量 */
  double zoom_x;
  double zoom_y;
  double angle;
  int x;
  int y;
  int ox;
  int oy;
  int bush_depth;
  int opacity;
  uint8_t blend_type;
  uint8_t scale_mode;
  bool mirror;
  bool flash_hidden;

  /// @brief 重载父类的同名方法
  /// 在以下几种情况下跳过绘制：
  /// 1. 没有设置 bitmap
  /// 2. 透明度为 0
  /// 3. x 或者 y 方向的 zoom 小于或等于 0
  /// 4. 由于闪烁效果导致的短暂消失
  bool visible() const {
    if (bitmap == 0) return false;
    if (opacity == 0) return false;
    if (zoom_x <= 0.0) return false;
    if (zoom_y <= 0.0) return false;
    if (flash_hidden) return false;
    return true;
  }
};

/// @brief 对应于 RGSS 中的 Plane 类
/// The Plane class. Planes are special sprites that tile bitmap patterns
/// across the entire screen, and are used to display panoramas and fog.
struct plane : drawable_object<plane> {
  static constexpr std::string_view name = "plane";

  /* 以下对应于 Plane 中对象类型的属性的成员变量 */
  color color;
  tone tone;

  /* 以下对应于 Plane 中 ID 类型的属性的成员变量 */
  uint64_t bitmap;

  /* 以下对应于 Plane 中值类型的属性的成员变量 */
  double zoom_x;
  double zoom_y;
  int ox;
  int oy;
  int opacity;
  uint8_t blend_type;
  uint8_t scale_mode;

  /// @brief 重载父类的同名方法
  /// 在以下几种情况下跳过绘制：
  /// 1. 没有设置 bitmap
  /// 2. 透明度为 0
  /// 3. x 或者 y 方向的 zoom 小于或等于 0
  bool visible() const {
    if (bitmap == 0) return false;
    if (opacity == 0) return false;
    if (zoom_x <= 0.0) return false;
    if (zoom_y <= 0.0) return false;
    return true;
  }
};

/// @brief 对应于 RGSS 中的 Window 类
/// The game window class. Created internally from multiple sprites.
/// 实际的实现并没有依赖多个 sprites，而是分成 2 层绘制。
struct window : drawable_object<window> {
  static constexpr std::string_view name = "window";
  static constexpr uint16_t fixed_overlayer_zs[] = {2};

  /* 以下对应于 Window 中对象类型的属性的成员变量 */
  rect cursor_rect;

  /* 以下对应于 Window 中 ID 类型的属性的成员变量 */
  uint64_t windowskin;
  uint64_t contents;

  /* 以下对应于 Window 中值类型的属性的成员变量 */
  int x;
  int y;
  int width;
  int height;
  int ox;
  int oy;
  int opacity;
  int back_opacity;
  int contents_opacity;
  uint8_t update_count;
  uint8_t cursor_count;
  bool stretch;
  bool active;
  bool pause;

  /// @brief 重载父类的同名方法
  /// 在以下几种情况下跳过绘制：
  /// 1. 窗口没有设置 windowskin
  /// 2. 窗口的长或宽为 0
  bool visible() const {
    if (windowskin == 0) return false;
    if (width == 0 || height == 0) return false;
    return true;
  }

  /// @brief 可以获取 Viewport 的 rect 时的重载
  /// 在以下几种情况下跳过绘制：
  /// 1. 窗口在 Viewport 外
  bool visible(const rect& r) const {
    if (x + width < r.x) return false;
    if (x > r.x + r.width) return false;
    if (y + height < r.y) return false;
    if (y > r.y + r.height) return false;
    return true;
  }
};

/// @brief window 的上一层，比 window 的 z 值高 2。
/// overlayer 不继承自 drawable_object，须实现 skip() 接口。
template <>
struct overlayer<window> {
  /// @brief window 的数据
  /// overlayer 层没有数据的所有权，用常指针访问 window 类的成员。
  const window* p_drawable;

  /// @brief overlayer 的层数
  const size_t m_index;

  /// @brief 实现 skip() 接口
  /// @return 返回 true 时将跳过绘制，否则将绘制此对象。
  bool skip() const {
    bool visible = detail::get<word::visible, bool>(p_drawable->ruby_object);
    return !visible;
  }
};

/// @brief 对应于 RGSS 中的 Tilemap 类
/// The class governing tilemaps. Tilemaps are a specialized concept used
/// in 2D game map displays, created internally from multiple sprites.
/// 实际的实现并没有依赖多个 sprites，而是分成很多层绘制。
/// RGSS 的 tilemap 没有用 overlayer 层实现。
/// @see ./src/rmxp/tilemap_manager.hpp
struct tilemap : drawable_object<tilemap> {
  static constexpr std::string_view name = "tilemap";

  /* 以下对应于 Tilemap 中对象类型的属性的成员变量 */

  /// @brief 存储自动原件的数组，元素为对应 Bitmap 的 ID
  /// @see ./src/rmxp/builtin.hpp
  autotiles autotiles;

  /* 以下对应于 Tilemap 中 ID 类型的属性的成员变量 */
  uint64_t tileset;
  uint64_t map_data;
  uint64_t flash_data;
  uint64_t priorities;

  /* 以下对应于 Tilemap 中值类型的属性的成员变量 */
  int ox;
  int oy;
  int update_count;
  bool repeat_x;
  bool repeat_y;

  /// @brief 重载父类的同名方法
  /// 在以下几种情况下跳过绘制：
  /// 1. 没有设置 tileset
  /// 2. 没有设置图块数据
  bool visible() const {
    if (tileset == 0) return false;
    if (map_data == 0) return false;
    return true;
  }
};

/// @brief 对应于 RGSS 中的 Animation 类
/// @todo 实现 Animation 类更高效的绘制。
struct animation : drawable_object<animation> {
  static constexpr std::string_view name = "animation";
};

/// @brief 对应于 RGSS 中的 Weather 类
/// @todo 实现 Weather 类更高效的绘制。
/// Class for weather effects (rain, storm, snow) displayed via
/// RPGXP's Event command.
struct weather : drawable_object<weather> {
  static constexpr std::string_view name = "weather";
};

/// @brief 可存储所有 Drawable 类型的 std::variant
using drawable = std::variant<viewport, sprite, plane, window, tilemap,
                              overlayer<window>, animation, weather>;

/// @brief 对应于 RGSS 中的 Viewport 类
/// The viewport class. Used when displaying sprites in one portion of the
/// screen, with no overflow into other regions.
struct viewport : drawable_object<viewport> {
  static constexpr std::string_view name = "viewport";

  /// @brief 指向 drawables 对象，存储了所有绑定到该 Viewport 的 Drawable
  std::unique_ptr<drawables> p_drawables;

  // 以下对应于 Viewport 中对象类型的属性的成员变量
  color flash_color;
  rect rect;
  color color;
  tone tone;

  // 以下对应于 Viewport 中值类型的属性的成员变量
  int ox;
  int oy;
  bool flash_hidden;

  /// @brief 重载父类的同名方法
  /// 在以下几种情况下跳过绘制：
  /// 1. rect 的长或宽为 0
  /// 2. 由于闪烁效果导致的短暂消失
  bool visible() const {
    if (rect.width <= 0 || rect.height <= 0) return false;
    if (flash_hidden) return false;
    return true;
  }

  /// @brief 可以获取 Screen 的 rect 时的重载
  /// 在以下几种情况下跳过绘制：
  /// 1. Viewport 在 Screen 外
  bool visible(const rmxp::rect& r) const {
    if (rect.x + rect.width < r.x) return false;
    if (rect.x > r.x + r.width) return false;
    if (rect.y + rect.height < r.y) return false;
    if (rect.y > r.y + r.height) return false;
    return true;
  }
};

/// @brief 存储所有 Drawable 的 map
/// @name data
/// Drawables 是有序的，索引是 z_index
struct drawables {
  /// @brief 线程不安全的资源池，所有的 drawables 共用这个资源池
  static std::pmr::unsynchronized_pool_resource pool;

  /// @brief 存储 drawable 的 map
  /// 所有的 drawable 按照 z_index 有序排列。
  std::pmr::map<z_index, drawable> m_data;

  /// @brief 构造函数
  explicit drawables() : m_data(&pool) {}

  /// @brief 设置某个 drawable 的新 z 值。
  /// 需要从 map 中取出再放回。
  /// 这个函数不会操作 drawable 的堆上数据，故指针不会变化。
  void set_z(const z_index& key, const int new_z) {
    /* 将 key 对应的节点从 map 中取出 */
    auto node = m_data.extract(key);

    /* 若 key 不存在直接返回 */
    if (node.empty()) return;

    /* 对于 overlayer，相应的 z 值也要跟随改变 */
    auto set_z_visitor = [=, &key, this]([[maybe_unused]] auto&& item) {
      if constexpr (requires { item.fixed_overlayer_zs; }) {
        for (uint16_t delta_z : item.fixed_overlayer_zs) {
          set_z(z_index{key.z + delta_z, key.id}, new_z + delta_z);
        }
      }
    };
    /* 修改 overlayer 的 z 值 */
    std::visit(set_z_visitor, node.mapped());

    /* 修改节点对应的 key，重新放回 map */    
    node.key() = z_index{new_z, key.id};
    m_data.insert(std::move(node));    
  }
};
/// @brief 初始化资源池
/// std::pmr::pool_options 中的 1024 代表每次从上游取出的 block 数量，
/// 128 代表资源池允许申请资源大小，超过此值就会从上游分配。
/// @ref https://en.cppreference.com/w/cpp/memory/pool_options
std::pmr::unsynchronized_pool_resource drawables::pool(std::pmr::pool_options{
    1024, 128});

// constexpr auto x0 = sizeof(z_index);   // 16
// constexpr auto x1 = sizeof(sprite);    // 112
// constexpr auto x2 = sizeof(plane);     // 72
// constexpr auto x3 = sizeof(window);    // 96
// constexpr auto x4 = sizeof(tilemap);   // 88
// constexpr auto x5 = sizeof(viewport);  // 80
// constexpr auto x6 = sizeof(drawable);  // 120
}  // namespace rgm::rmxp