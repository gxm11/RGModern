// zlib License

// Copyright (C) [2023] [Xiaomi Guo]

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
#include <map>

#include "base/base.hpp"
#include "detail.hpp"

namespace rgm::rmxp {
/**
 * @brief 管理从 Drawable 的 ID 对应到 Drawable 的 z 值的映射，继承自 multimap
 * @note id2z 的实例用于辅助管理 drawables，因为有的时候只能获得 drawable 的
 * ID。 此外，id2z 中是否包含某个 ID 等价于 drawables 是否包含该 ID 的
 * drawable。 由于 window 之类的多层 drawable
 */
struct id2z : std::unordered_map<uint64_t, int> {
  explicit id2z() : std::unordered_map<uint64_t, int>() {}
};

/**
 * @brief 所有 Drawable 的索引，每个 Drawable 的 z_index 是独一无二的。
 * @note 所有 Drawable 的顺序由其 z 值决定，相同 z 值，ID 越大的处于上方。
 * 注意 ID 越大的 Drawable 创建越晚。属于同一个 Viewport 中的 Drawables 会
 * 在 Viewpoint 中排序，然后才考虑 Viewport 与不属于任何 Viewport 的 Drawables
 * 的顺序。
 */
struct z_index {
  int z;
  uint64_t id;

  /**
   * @brief 用 ruby 对象的各个属性更新自身的成员变量的值
   *
   * @param object 目标 ruby 对象，通常是任意的 Drawable 类型
   * @return z_index& 返回对自身的引用
   */
  z_index& operator<<(const VALUE object) {
    z = detail::get<word::z, int>(object);

    VALUE id_ = detail::get<word::id>(object);
    id = NUM2ULL(id_);

    return *this;
  }

  /** @brief z_index 拥有强顺序，且不存在 z_index 相同的情况 */
  std::strong_ordering operator<=>(const z_index&) const = default;
};

/**
 * @brief 对应于 RGSS 中 Color 的类
 */
struct color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t alpha;

  /**
   * @brief 用 ruby 对象的各个属性更新自身的成员变量的值
   *
   * @param object 目标 ruby 对象，通常是 Color 类型
   * @return z_index& 返回对自身的引用
   */
  color& operator<<(const VALUE object) {
    red = detail::get<word::red, uint8_t>(object);
    green = detail::get<word::green, uint8_t>(object);
    blue = detail::get<word::blue, uint8_t>(object);
    alpha = detail::get<word::alpha, uint8_t>(object);

    return *this;
  }
};

/**
 * @brief 对应于 RGSS 中 Tone 的类
 */
struct tone {
  int16_t red;
  int16_t green;
  int16_t blue;
  int16_t gray;

  /**
   * @brief 用 ruby 对象的各个属性更新自身的成员变量的值
   *
   * @param object 目标 ruby 对象，通常是 Tone 类型
   * @return z_index& 返回对自身的引用
   */
  tone& operator<<(const VALUE object) {
    red = detail::get<word::red, int16_t>(object);
    green = detail::get<word::green, int16_t>(object);
    blue = detail::get<word::blue, int16_t>(object);
    gray = detail::get<word::gray, int16_t>(object);

    return *this;
  }

  std::optional<cen::color> color_add() const {
    if (red <= 0 && green <= 0 && blue <= 0) {
      return std::nullopt;
    }
    return cen::color(std::max<int16_t>(0, red), std::max<int16_t>(0, green),
                      std::max<int16_t>(0, blue), 255);
  }

  std::optional<cen::color> color_sub() const {
    if (red >= 0 && green >= 0 && blue >= 0) {
      return std::nullopt;
    }
    return cen::color(std::max<int16_t>(0, -red), std::max<int16_t>(0, -green),
                      std::max<int16_t>(0, -blue), 255);
  }
};

/**
 * @brief 对应于 RGSS 中 Rect 的类
 */
struct rect {
  int x;
  int y;
  int width;
  int height;

  /**
   * @brief 用 ruby 对象的各个属性更新自身的成员变量的值
   *
   * @param object 目标 ruby 对象，通常是 Rect 类型
   * @return z_index& 返回对自身的引用
   */
  rect& operator<<(const VALUE object) {
    x = detail::get<word::x, int>(object);
    y = detail::get<word::y, int>(object);
    width = detail::get<word::width, int>(object);
    height = detail::get<word::height, int>(object);

    return *this;
  }
};

/**
 * @brief 对应于 RGSS 中RPG::Map的数组 Autotiles 的类
 * @note 尚未完成，等 render<tilemap> 写好后再更新。
 */
struct autotiles {
  static constexpr size_t max_size = 7;
  std::vector<uint64_t> m_data;

  autotiles() : m_data(max_size, 0) {}

  /**
   * @brief 用 ruby 对象的各个属性更新自身的成员变量的值
   *
   * @param object 目标 ruby 对象，通常是 Array 类型
   * @return z_index& 返回对自身的引用
   */
  autotiles& operator<<(const VALUE object) {
    const size_t len = RARRAY_LEN(object);
    for (size_t i = 0; i < max_size; ++i) {
      uint64_t& id = m_data[i];
      if (i >= len) {
        id = 0;
        continue;
      }
      const VALUE bitmap_ = RARRAY_AREF(object, i);
      const uint64_t new_id = detail::get<const uint64_t>(bitmap_);
      // new_id 是 Bitmap 的 object_id，一定是偶数（否则在 ruby 中表示一个
      // FIXNUM） 如果 id != new_id，说明 Bitmap 已经改变了，将 id + 1
      // 存储，等后续处理
      if (id != new_id) {
        id = new_id + 1;
      }
    }
    return *this;
  }

  autotiles& operator<<(std::function<void(uint64_t)> callback) {
    for (size_t i = 0; i < max_size; ++i) {
      // 调用 m_autotiles << VALUE << callback;
      // 其中 callback 会发送一个指令到绘制线程，并包含一个 Bitmap 的 id。
      // 这个指令会将 Bitmap 转换成 Autotile 的形式，并且存储在 id + 1 的位置。
      // 在调用 autotiles 时，读取 bitmap 需要将 id + 1 得到其 Autotile 形式。
      // Bitmap 析构时，会顺便析构 id + 1 的 Bitmap，stl 会忽略无效的 erase。

      // 如果 id 是奇数，则说明在 << VALUE 操作时 id 发生了变化，需要触发
      // callback
      uint64_t& id = m_data[i];
      if (id & 1) {
        --id;
        callback(id);
      }
    }
    return *this;
  }
};
}  // namespace rgm::rmxp