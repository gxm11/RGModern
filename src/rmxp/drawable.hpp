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

// RGM 里强制约定，所有 C++ 的映射类型中，类型为 uint64_t 的成员变量，
// 都对应于一个 ruby 中对象类型的属性，其值为该属性的 object_id。
// 在 detail 中有特殊的处理，使用 get 时会自动转换类型参数为 const uint64_t。
// 此时会尝试获取 ruby 对象的 object_id，而不是调用 NUM2ULL。
// detail 的 get 会在 drawable_object 中的宏里被调用，请参阅。
namespace rgm::rmxp {
/**
 * @brief 对应于 RGSS 中的 Sprite 类
 * @note The sprite class. Sprites are the basic concept used to display
 * characters, etc. on the game screen.
 */
struct sprite : drawable_object<sprite> {
  static constexpr const char* name = "sprite";

  // 以下对应于 Sprite 中对象类型的属性的成员变量
  color flash_color;
  color color;
  tone tone;
  rect src_rect;

  // 以下对应于 Sprite 中值类型的属性的成员变量
  uint64_t bitmap;
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

  /**
   * @brief 重载父类的同名方法
   * @note 在以下几种情况下跳过绘制：
   * 1. 没有设置 bitmap
   * 2. 透明度为 0
   * 3. x 或者 y 方向的 zoom 小于或等于 0
   * 4. 由于闪烁效果导致的短暂消失
   */
  bool visible() const {
    if (bitmap == 0) return false;
    if (opacity == 0) return false;
    if (zoom_x <= 0.0) return false;
    if (zoom_y <= 0.0) return false;
    if (flash_hidden) return false;
    return true;
  }
};

/**
 * @brief 对应于 RGSS 中的 Plane 类
 * @note The Plane class. Planes are special sprites that tile bitmap patterns
 * across the entire screen, and are used to display panoramas and fog.
 */
struct plane : drawable_object<plane> {
  static constexpr const char* name = "plane";

  // 以下对应于 Plane 中对象类型的属性的成员变量
  color color;
  tone tone;

  // 以下对应于 Plane 中值类型的属性的成员变量
  uint64_t bitmap;
  double zoom_x;
  double zoom_y;
  int ox;
  int oy;
  int opacity;
  uint8_t blend_type;
  uint8_t scale_mode;

  /**
   * @brief 重载父类的同名方法
   * @note 在以下几种情况下跳过绘制：
   * 1. 没有设置 bitmap
   * 2. 透明度为 0
   * 3. x 或者 y 方向的 zoom 小于或等于 0
   */
  bool visible() const {
    if (bitmap == 0) return false;
    if (opacity == 0) return false;
    if (zoom_x <= 0.0) return false;
    if (zoom_y <= 0.0) return false;
    return true;
  }
};

/**
 * @brief 对应于 RGSS 中的 Window 类
 * @note The game window class. Created internally from multiple sprites.
 */
struct window : drawable_object<window> {
  static constexpr const char* name = "window";
  static constexpr uint16_t fixed_overlayer_zs[] = {2};

  // 以下对应于 Window 中对象类型的属性的成员变量
  rect cursor_rect;

  // 以下对应于 Window 中值类型的属性的成员变量
  uint64_t windowskin;
  uint64_t contents;
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

  /**
   * @brief 重载父类的同名方法
   * @note 在以下几种情况下跳过绘制：
   * 1. 窗口没有设置 windowskin
   * 2. 窗口的长或宽为 0
   */
  bool visible() const {
    if (windowskin == 0) return false;
    if (width == 0 || height == 0) return false;
    return true;
  }

  bool visible(const rect& r) const {
    if (x + width < r.x) return false;
    if (x > r.x + r.width) return false;
    if (y + height < r.y) return false;
    if (y > r.y + r.height) return false;
    return true;
  }
};

template <>
struct overlayer<window> {
  const window* p_drawable;
  const size_t m_index;

  bool skip() const {
    bool visible = detail::get<word::visible, bool>(p_drawable->ruby_object);
    return !visible;
  }
};

/**
 * @brief 对应于 RGSS 中的 Tilemap 类
 * @note The class governing tilemaps. Tilemaps are a specialized concept used
 * in 2D game map displays, created internally from multiple sprites.
 */
struct tilemap : drawable_object<tilemap> {
  static constexpr const char* name = "tilemap";

  // 以下对应于 Tilemap 中对象类型的属性的成员变量

  /**
   * @brief 存储自动原件的数组，元素为对应 Bitmap 的 ID
   * @note Refers to the bitmap (Bitmap) used as an autotile with an index
   * number from 0 to 6.
   */
  autotiles autotiles;

  // 以下对应于 Window 中值类型的属性的成员变量
  uint64_t tileset;
  uint64_t map_data;
  uint64_t flash_data;
  uint64_t priorities;
  int ox;
  int oy;
  int update_count;
  bool repeat_x;
  bool repeat_y;

  /**
   * @brief 重载父类的同名方法
   * @note 在以下几种情况下跳过绘制：
   * 1. 没有设置 tileset
   * 2. 没有设置图块数据
   */
  bool visible() const {
    if (tileset == 0) return false;
    if (map_data == 0) return false;
    return true;
  }
};

/**
 * @brief 对应于 RGSS 中的 Animation 类
 * @note 尚未完成
 */
struct animation : drawable_object<animation> {
  static constexpr const char* name = "animation";
};

/**
 * @brief 对应于 RGSS 中的 Weather 类
 * @note 尚未完成
 */
struct weather : drawable_object<weather> {
  static constexpr const char* name = "weather";
};

// 声明，具体实现见下
struct viewport;

/**
 * @brief 所有 Drawable 类型组成的 std::variant
 */
using drawable = std::variant<viewport, sprite, plane, window, tilemap,
                              overlayer<window>, animation, weather>;

/**
 * @brief 存储所有 Drawable 的 map
 * @note 索引是 z_index
 */
struct drawables : std::map<z_index, drawable> {
  /**
   * @brief 默认构造函数必须不包含任何参数
   */
  explicit drawables() : std::map<z_index, drawable>() {}

  /**
   * @brief 设置 Drawable 的 z 值
   */
  void set_z(const z_index&, const int);

  size_t total_size();
};

/**
 * @brief 对应于 RGSS 中的 Viewport 类
 * @note The viewport class. Used when displaying sprites in one portion of the
 * screen, with no overflow into other regions.
 */
struct viewport : drawable_object<viewport> {
  static constexpr const char* name = "viewport";

  /**
   * @brief 存储了所有绑定到该 Viewport 的 Drawable
   * @note 禁止修改 Drawable 绑定的 Viewport
   */
  drawables m_data;

  // 以下对应于 Window 中对象类型的属性的成员变量
  color flash_color;
  rect rect;
  color color;
  tone tone;

  // 以下对应于 Window 中值类型的属性的成员变量
  int ox;
  int oy;
  bool flash_hidden;

  /**
   * @brief 重载父类的同名方法
   * @note 在以下几种情况下跳过绘制：
   * 1. rect 的长或宽为 0
   * 2. 由于闪烁效果导致的短暂消失
   */
  bool visible() const {
    if (rect.width <= 0 || rect.height <= 0) return false;
    if (flash_hidden) return false;
    return true;
  }

  bool visible(const rmxp::rect& r) const {
    if (rect.x + rect.width < r.x) return false;
    if (rect.x > r.x + r.width) return false;
    if (rect.y + rect.height < r.y) return false;
    if (rect.y > r.y + r.height) return false;
    return true;
  }
};
// constexpr auto x0 = sizeof(z_index); // 16
// constexpr auto x1 = sizeof(sprite); // 104
// constexpr auto x2 = sizeof(plane); // 64
// constexpr auto x3 = sizeof(window); // 80
// constexpr auto x4 = sizeof(tilemap); // 80
// constexpr auto x5 = sizeof(viewport); // 104
// constexpr auto x6 = sizeof(drawable); // 112
}  // namespace rgm::rmxp