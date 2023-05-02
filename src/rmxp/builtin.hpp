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
#include "word.hpp"

namespace rgm::rmxp {
/// @brief 管理从 Drawable 的 ID 对应到 Drawable 的 z 值的映射
/// id2z 的实例用于缓存 ID 到 z 的映射关系，辅助管理 drawables。
/// 在 GC 触发时，finalizer 只能获取 Drawable 的 ID 不能获取作为属性的 z，
/// 查找此 Drawable 就必须使用 id2z 获取 z 值，然后去 drawables 里查找。
/// 此外，id2z 中是否包含某个 ID 等价于 drawables 是否包含该 ID 的 drawable，
/// 可用于快速判断。
struct id2z {
  /// @brief 用 unordered_map 存储数据
  std::unordered_map<uint64_t, int> m_data;

  /// @brief 添加一个数据，或者重设其 z 值
  /// @param id drawable 的 id
  /// @param z  drawable 的 z 值
  void insert(uint64_t id, int z) { m_data.insert_or_assign(id, z); }

  /// @brief 移除特定 id 对应的数据
  /// @param id drawable 的 id
  void erase(uint64_t id) { m_data.erase(id); }

  /// @brief 查找是否储存了某个 id，返回 z 值
  /// @param id 待查找的 drawable 的 id
  /// @return 存在则返回 z 值，不存在则返回 nullopt
  std::optional<int> find_z(uint64_t id) {
    auto it = m_data.find(id);
    /* 多数情况下这个 id 是存在的 */
    if (it != m_data.end()) [[likely]] {
      return it->second;
    } else [[unlikely]] {
      return std::nullopt;
    }
  }
};

/// @brief 所有 Drawable 在 drawables 中的有序索引
/// 每个 Drawable 的 z_index 是独一无二的，因为其 id 一定不同。
/// z_index 的大小由其 z 值决定，若 z 值相同，则 ID 越大 z_index 越大。
/// 注意 ID 越大的 Drawable 创建越晚。此外，Viewport 也有 z_index。
/// 属于同一个 Viewport 中的 Drawables 会在 Viewpoint 中排序，然后才考虑
/// Viewport 与不属于任何 Viewport 的 Drawables 的顺序。
struct z_index {
  /// @brief z 值，代表图层高度
  int z;

  /// @brief Drawable 的唯一 ID
  uint64_t id;

  /// @brief 用 ruby 对象的各个属性更新自身的成员变量的值
  /// @param object 目标 ruby 对象，通常是任意的 Drawable 类型
  /// @return z_index& 返回对自身的引用
  z_index& operator<<(const VALUE object) {
    z = detail::get<word::z, int>(object);
    id = detail::get<word::id, uint64_t>(object);

    return *this;
  }

  /// @brief 声明比较运算符为 strong_ordering 类型
  /// @ref https://en.cppreference.com/w/cpp/utility/compare/strong_ordering
  /// 默认的比较运算符会按顺序比较成员变量，即先比较 z，若 z 相同则再比较 id。
  /// z_index 拥有强顺序，且不存在 z_index 重复的情况。
  std::strong_ordering operator<=>(const z_index&) const = default;
};

/// @brief 对应于 RGSS 中 Color 的类
struct color {
  /// @brief 红色
  uint8_t red;

  /// @brief 绿色
  uint8_t green;

  /// @brief 蓝色
  uint8_t blue;

  /// @brief 透明度
  uint8_t alpha;

  /// @brief 用 ruby 对象的各个属性更新自身的成员变量的值
  /// @param object 目标 ruby 对象，通常是任意的 Drawable 类型
  /// @return color& 返回对自身的引用
  color& operator<<(const VALUE object) {
    red = detail::get<word::red, uint8_t>(object);
    green = detail::get<word::green, uint8_t>(object);
    blue = detail::get<word::blue, uint8_t>(object);
    alpha = detail::get<word::alpha, uint8_t>(object);

    return *this;
  }
};

/// @brief 对应于 RGSS 中 Tone 的类
struct tone {
  /// @brief 红色，范围是 -255 ~ 255
  int16_t red;

  /// @brief 绿色，范围是 -255 ~ 255
  int16_t green;

  /// @brief 蓝色，范围是 -255 ~ 255
  int16_t blue;

  /// @brief 灰度，范围是 0 ~ 255
  int16_t gray;

  /// @brief 用 ruby 对象的各个属性更新自身的成员变量的值
  /// @param object 目标 ruby 对象，通常是任意的 Drawable 类型
  /// @return tone& 返回对自身的引用
  tone& operator<<(const VALUE object) {
    red = detail::get<word::red, int16_t>(object);
    green = detail::get<word::green, int16_t>(object);
    blue = detail::get<word::blue, int16_t>(object);
    gray = detail::get<word::gray, int16_t>(object);

    return *this;
  }

  /// @brief 返回 tone 中蕴含的加法叠加的颜色
  /// @return 返回用于加法叠加的 cen::color，或者 nullopt
  std::optional<cen::color> color_add() const {
    if (red <= 0 && green <= 0 && blue <= 0) return std::nullopt;

    return cen::color(std::max<int16_t>(0, red), std::max<int16_t>(0, green),
                      std::max<int16_t>(0, blue), 255);
  }

  /// @brief 返回 tone 中蕴含的减法叠加的颜色
  /// @return 返回用于减法叠加的 cen::color，或者 nullopt
  std::optional<cen::color> color_sub() const {
    if (red >= 0 && green >= 0 && blue >= 0) return std::nullopt;

    return cen::color(std::max<int16_t>(0, -red), std::max<int16_t>(0, -green),
                      std::max<int16_t>(0, -blue), 255);
  }
};

/// @brief 对应于 RGSS 中 Rect 的类
struct rect {
  /// @brief X 坐标
  int x;

  /// @brief Y 坐标
  int y;

  /// @brief 宽度
  int width;

  /// @brief 高度
  int height;

  /// @brief 用 ruby 对象的各个属性更新自身的成员变量的值
  /// @param object 目标 ruby 对象，通常是任意的 Drawable 类型
  /// @return rect& 返回对自身的引用
  rect& operator<<(const VALUE object) {
    /* 这 4 个属性值在 ruby 中对应的是 Fixnum，所以取值范围只有 int 的一半 */
    x = detail::get<word::x, int>(object);
    y = detail::get<word::y, int>(object);
    width = detail::get<word::width, int>(object);
    height = detail::get<word::height, int>(object);

    return *this;
  }
};

/// @brief 对应于 RGSS 中 RPG::Map 的数组 @autotiles 的类
struct autotiles {
  /// @brief 数组 @autotiles 长度至少是 7
  static constexpr size_t max_size = 8;

  /// @brief 存储每个自动元件对应的原始 Bitmap 的 ID
  /// 自动元件实际对应的 Bitmap ID 要 +1
  std::vector<uint64_t> m_data;

  /// @brief 构造函数
  autotiles() : m_data(max_size, 0) {}

  /// @brief 用 ruby 对象的各个属性更新自身的成员变量的值
  /// @param object 目标 ruby 对象，通常是任意的 Drawable 类型
  /// @return autotiles& 返回对自身的引用
  autotiles& operator<<(const VALUE object) {
    /* 检查 object 必须是 Array 类型 */
    Check_Type(object, T_ARRAY);

    const size_t len = RARRAY_LEN(object);
    for (size_t i = 0; i < max_size; ++i) {
      uint64_t& id = m_data[i];
      /* 如果 object 对应的 Array 不够长，超出部分设为 0 */
      if (i >= len) {
        id = 0;
        continue;
      }
      /* 获取第 i 个元素的 object_id，此元素必须是 Bitmap */
      const VALUE bitmap_ = RARRAY_AREF(object, i);
      const uint64_t object_id = detail::get<const uint64_t>(bitmap_);
      /*
       * Bitmap 的 object_id，一定是偶数（否则在 ruby 中表示一个 Fixnum）
       * 如果 id != object_id，说明 Bitmap 已经改变了。
       * 将 id 改为 object_id + 1，等后续处理。
       */
      if (id != object_id) {
        id = object_id + 1;
      }
    }

    return *this;
  }

  /// @brief 在 << object 之后调用，对有变化的 ID 进行处理
  /// @param callback 处理方式
  /// @return autotiles& 返回对自身的引用
  /// 调用方法 m_autotiles << VALUE << callback;
  /// @see ./src/rmxp/graphics.hpp
  autotiles& operator<<(std::function<void(uint64_t)> callback) {
    /*
     * 在 graphics.hpp 的 update 函数中，实际做了以下操作：
     * 1. 执行 autotiles << VALUE
     * 2. 检查是否有值为奇数，这暗示 Bitmap 改变了
     * 3. 对改变的 Bitmap，重新绘制展开后的自动元件图，存储在 id + 1 的位置
     * 故此处的 callback，触发条件是 id 为奇数，callback 的内容是发送
     * bitmap_make_autotile{id} 任务，这里的 id 是 Bitmap 的 ID。
     */
    for (size_t i = 0; i < max_size; ++i) {
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