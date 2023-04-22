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
/// @brief 刷新 C++ 层变量的值，作为 ITERATE_VALUES 的参数。
/// 对于 uint64_t 类型的变量，按照 const uint64_t 类型处理，读取其 object_id。
#define DO_REFRESH_VALUE(key)                                               \
  if constexpr (requires { item.key; }) {                                   \
    using T = decltype(item.key);                                           \
    using U =                                                               \
        std::conditional_t<std::is_same_v<T, uint64_t>, const uint64_t, T>; \
    item.key = detail::get<word::key, U>(ruby_object);                      \
  }

/// @brief 刷新 C++ 层变量的值，作为 ITERATE_OBJECTS 的参数。
#define DO_REFRESH_OBJECT(key)                       \
  if constexpr (requires { item.key; }) {            \
    item.key << detail::get<word::key>(ruby_object); \
  }

/// @brief case 分支，根据当前的 key 更新对应的成员变量
/// 同样是用于刷新 C++ 层变量的值，作为 ITERATE_OBJECTS 的参数使用。
/// 对于 uint64_t 类型的变量，按照 const uint64_t 类型处理，读取其 object_id。
/// 此处 `if constexpr (requires { item.key; })` 的含义是：
/// 如果 item 没有名称为 key 的成员变量，此 case 分支直接返回。
#define DO_CASE_BRANCH(key)                                                   \
  case word::key:                                                             \
    if constexpr (requires { item.key; }) {                                   \
      using T = decltype(item.key);                                           \
      using U =                                                               \
          std::conditional_t<std::is_same_v<T, uint64_t>, const uint64_t, T>; \
      item.key = detail::get<word::key, U>(ruby_object);                      \
    }                                                                         \
    return

/// @brief 所有 Drawable 的基类，派生类以 CRTP 的形式继承。
/// @tparam T_Drawable 派生类的类型
/// 所谓 Drawable，对应于会在画面上显示的对象。此对象在 ruby 和 C++ 层各存有一份
/// 数据。需要通过 refresh_value 和 refresh_object 函数将数据同步到 C++ 层。
/// C++ 层的数据比 ruby 层的数据少了以下 4 种属性：@z，@id，@visible 和
/// @disposed。原因如下：
/// 1. @z 和 @id 作为 z_index 类型的索引使用，不需要保存在 Drawable 中；
/// 2. @visible 在绘制的每帧都需要查询，没有保存的必要；
/// 3. @disposed 对应 Drawable 是否存在，当 ruby 中对应的对象 dispose 时，C++
///    层的对象会随之销毁。所以只要对象存在，该值始终是 true，没有保存的必要。
template <typename T_Drawable>
struct drawable_object {
  /// @brief 对应 ruby 中对象的 VALUE
  VALUE ruby_object;

  /// @brief 读取 ruby 对象中各个实例变量，更新自身的成员变量
  /// @param object 目标 ruby 对象，通常是任意的 Drawable 类型
  /// @return T_Drawable& 返回对自身的引用
  T_Drawable& operator<<(const VALUE object) {
    ruby_object = object;

    /* 读取 ruby object 的实例变量，设置自身的成员变量 */
    refresh_value();
    refresh_object();

    /* 返回派生类型的引用 */
    return *static_cast<T_Drawable*>(this);
  }

  /// @brief 表示该对象是否可见，派生类需要重载此方法，否则永远可见。
  bool visible() const { return true; }

  /// @brief 表示对该对象的绘制是否应该跳过。
  /// @return true 该对象不可见，跳过其绘制，否则不能跳过绘制。
  /// 此函数才是用来判断是否绘制的接口，与 visible 不同名为了防止递归调用。
  bool skip() const {
    /* 判断 @visible 属性 */
    bool visible = detail::get<word::visible, bool>(ruby_object);
    if (!visible) return true;

    /* 调用派生类的 visible 方法 */
    const T_Drawable& item = *static_cast<const T_Drawable*>(this);

    return !item.visible();
  }

  /// @brief 更新所有值类型和 ID 类型的实例变量所对应的成员变量。
  void refresh_value() {
    T_Drawable& item = *static_cast<T_Drawable*>(this);

    ITERATE_VALUES(DO_REFRESH_VALUE);
  }

  /// @brief 更新所有对象类型的实例变量所对应的成员变量。
  void refresh_object() {
    T_Drawable& item = *static_cast<T_Drawable*>(this);

    ITERATE_OBJECTS(DO_REFRESH_OBJECT);
  }

  /// @brief 更新特定名称的值类型的属性所对应的成员变量。
  /// @param type 枚举类 word 的元素，表示对应的属性名。
  void refresh_value(word type) {
    T_Drawable& item = *static_cast<T_Drawable*>(this);

    /*
     * 比起为每一个 Drawable 类型的每一个成员变量定义 setter 方法，
     * 使用 switch 可能有一点点性能损失，但大大简化了编码。
     */
    switch (type) {
      ITERATE_VALUES(DO_CASE_BRANCH);
      default:
        return;
    }
  }
};
#undef DO_REFRESH_OBJECT
#undef DO_REFRESH_VALUE
#undef DO_CASE_BRANCH
#undef ITERATE_VALUES
#undef ITERATE_OBJECTS
}  // namespace rgm::rmxp