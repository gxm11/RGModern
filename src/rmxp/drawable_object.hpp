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
#include "base/base.hpp"
#include "detail.hpp"

// drawable base class
namespace rgm::rmxp {
/** 宏：更新当前 item 的 key 属性，key 必须对应一个对象类别的属性 */
#define DO_REFRESH_OBJECT(key)                  \
  if constexpr (requires { item.key; }) {       \
    item.key << detail::get<word::key>(object); \
  }

/** 宏：更新当前 item 的 key 属性，key 必须对应一个值类别的属性 */
#define DO_REFRESH_VALUE(key)                                      \
  if constexpr (requires { item.key; }) {                          \
    item.key = detail::get<word::key, decltype(item.key)>(object); \
  }

/** 宏：case 分支，根据当前的 key 更新对应的属性，key 必须对应一个对象别的属性
 */
#define DO_CASE_BRANCH(key)                                          \
  case word::key:                                                    \
    if constexpr (requires { item.key; }) {                          \
      item.key = detail::get<word::key, decltype(item.key)>(object); \
    }                                                                \
    return

/**
 * @brief 所有 Drawable 的基类，派生类以 CRTP 的形式继承。
 *
 * @tparam T_Drawable 派生类的类型，必须对应 ruby 里的某个 Drawable
 *
 * @note 这是所有 drawable_object 的基类，相应的派生类以 CRTP
 * 的形式实现，且必须与 ruby 中某个 Drawable
 * 关联。由于使用了静态多态的技巧，派生类只需要定义特定的成员变 量，以及重载
 * visible 方法，其余的操作都在基类里实现。 注：
 * 1. ruby 中某个 Drawable 是指创建对象实例后会渲染到画面上的类，包括
 * Sprite，Plane， Window，Tilemap 和 Viewport，也包括 Animation 和 Weather。
 * 2. ruby 中的 Drawable 的 @z 和 @id 属性代表了绘制的顺序，而在 T_Drawable
 * 中不存 在相应的变量。所有的 T_Drawable 都保存在以 z_index 为 key 的 map
 * 中，并且在每次绘 制画面时都要按顺序遍历整个 map 执行渲染操作。
 * 3. ruby 中的 Drawable 的 @disposed 属性也不在 T_Drawable 中。@disposed
 * 是一个纯 ruby 的变量。Drawable 在创建时，相应的 T_Drawable 加入 map，Drawable
 * 被垃圾回收时， 相应的 T_Drawable 离开 map。Drawable
 * 被释放时，只是做了一个标记。这一设计是考虑到 RGSS 中每当场景切换时 Drawable
 * 才会被释放，并且很快就进入垃圾回收。
 * 4. ruby 中的 Drawable 的 @visible 属性也不在 T_Drawable 中。@visible
 * 属性每次绘制 都需要判断一次，所以无需花费空间存下来。基类中的 skip 函数和
 * visible 函数是为了尽可 能跳过无效绘制而使用。T_Drawable 需要定义自己的
 * visible 函数，用来告知基类除了
 * @visible = false 以外是否还有其他可以跳过绘制的情况。
 * 5. ruby 中的 Drawable 剩下的属性被分为 2
 * 种类型：值类型，对象类型。值类型中又有一个 特别的类，为 ID
 * 类型。对于值类型的属性，ruby 中会定义对应的 setter 方法，每次调用该
 * 方法会同步修改对应的 T_Drawable 中的数据。而对象类型的属性没有定义特别的
 * setter 方法， 直接设置为 accessor。但是在每次绘制之前，都会同步修改
 * T_Drawable 中相应的变量。
 * 6. ID 类型也是值类型，但是其在 ruby 层中并不是一个数值，而是 Bitmap 或 Table
 * 等类型的 对象。这些对象的真实数据存储在 C++ 层的容器中，使用数字 ID
 * 为索引，而其 ruby 层只是简单的封装。如果该类型属性的值为 nil，则其 ID = 0。
 * 7. 得益于 RGSS 的特殊设计，各个 Drawable
 * 之间的属性，如果其变量名相同，则其类型也相同。 从而我们无需逐个 T_Drawable
 * 定义 refresh_value 和 refresh_object 函数。
 */
template <typename T_Drawable>
struct drawable_object {
  /** @brief 对应 ruby 中对象的 VALUE */
  VALUE object;

  /**
   * @brief 用 ruby 对象的各个属性更新自身的成员变量的值
   *
   * @param object 目标 ruby 对象，通常是任意的 Drawable 类型
   * @return T_Drawable& 返回对自身的引用
   */
  T_Drawable& operator<<(const VALUE object) {
    T_Drawable& item = *static_cast<T_Drawable*>(this);

    item.object = object;
    item.refresh_value();
    item.refresh_object();
    return item;
  }

  /**
   * @brief 表示该对象是否可见，派生类需要重载此方法。
   *
   * @return true 默认值，表示总是可见
   */
  bool visible() const { return true; }

  /**
   * @brief 表示对该对象的绘制是否应该跳过。
   * @note 先判断 @visible 属性，再调用了派生类的 visible 方法。
   * @return true 该对象不可见，跳过其绘制。
   * @return false 不能跳过绘制。
   */
  bool skip() const {
    if (!detail::get<word::visible, bool>(object)) return true;
    return !static_cast<const T_Drawable*>(this)->visible();
  }

  /**
   * @brief 更新所有对象类型的属性所对应的成员变量。
   * @note
   * 注意宏里其实遍历了所有可能的对象类型的属性名，但实际上只会处理类中存在的成员变量。
   */
  void refresh_object() {
    T_Drawable& item = *static_cast<T_Drawable*>(this);
    ITERATE_OBJECTS(DO_REFRESH_OBJECT);
  }

  /**
   * @brief 更新所有值类型的属性所对应的成员变量。
   * @note
   * 注意宏里其实遍历了所有可能的值类型的属性名，但实际上只会处理类中存在的成员变量。
   */
  void refresh_value() {
    T_Drawable& item = *static_cast<T_Drawable*>(this);
    ITERATE_VALUES(DO_REFRESH_VALUE);
  }

  /**
   * @brief 更新特定名称的值类型的属性所对应的成员变量。
   * @note
   * 注意宏里其实遍历了所有可能的值类型的属性名，但实际上只会处理类中存在的成员变量。
   * @param type 枚举类 word 的成员，表示对应的属性名。
   */
  void refresh_value(word type) {
    T_Drawable& item = *static_cast<T_Drawable*>(this);
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
}  // namespace rgm::rmxp