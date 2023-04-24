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

namespace rgm::rmxp {
/*
 * 关键字枚举类 rmxp::word 用于操作 ruby 对象的实例变量，如 @active。
 * 在 base::detail_ext 中作为类模板参数被用到。有如下功能设计：
 *
 * 1. 用于获取实例变量的值
 * 从 ruby 中获取实例变量需要调用函数 rb_ivar_get(VALUE, SYMBOL)，其中 VALUE
 * 代表一个 ruby 对象，而 SYMBOL 代表一个 ruby symbol。ruby 中的 symbol 其实
 * 就是一个整数，并且其值存储在一个 table 中永远不会变化。为了避免每次都要调用
 * rb_intern(NAME) 用字符串查找 SYMBOL，直接在程序开始时，一次性创建所有常用的
 * SYMBOL，并存储其整数值可以提升效率。
 *
 * base::detail_ext<word> 类中的容器 id_table 存储这些 SYMBOL 值，其静态函数
 * get<word>(VALUE) 等重载了基类的 get 函数，使用枚举 word 的元素查找 symbol，
 * 以快速读取实例变量的值。
 *
 * init_word 的 before 方法中，遍历 word 的元素，创建对应的 symbol。
 *
 * 在 builtin.hpp 中广泛用到，如
 *   `z = detail::get<word::z, int>(object);`
 * 意思是获取 object 的实例变量 @z 的值，并且将其转化为 int 类型。
 *
 * 2. 用于设置 T_drawable 的成员变量
 * T_drawable 的成员变量对应于 RGSS 中各 Drawable 类的实例变量，存在 3 种类型
 * 值类型（value type），此实例变量是一个即时值，比如一个 Fixnum 或 Bool；
 * 对象类型（object type），此实例变量是一个对象，但此对象在 C++ 层中不做统一
 * 管理；
 * ID 类型（id type），此实例变量是一个对象，但此对象在 C++ 层中被某个容器统一
 * 管理，只需保存 C++ 层中相应对象的 ID 即可访问到此对象。此 ID 通常是实例变量
 * 的 object_id。
 *
 * 以 Sprite 为例：实例变量 @x, @y 是整数，属于第一类；实例变量 @color 是对象
 * 类型，但在 C++ 层不做统一管理，属于第二类；实例变量 @bitmap 是对象类型，但在
 * C++ 层中所有 Bitmap 类所对应的对象（cen::texture）都在容器 base::textures
 * 中统一管理，故储存其 ID 即可。
 *
 * 由于 Drawable 类型的数据在 ruby 和 C++ 层中各存储了一份，所以数据必须定期
 * 同步。这里使用了以下策略来同步：
 * 对于值类型（value type）和 ID 类型（id type），每次 ruby 中进行相应实例变
 * 量的修改，都会调用 refresh_value 方法，将 C++ 层对应的成员变量赋值为实例变
 * 量的新值。
 * 对于对象类型（object type），在 Graphics.update 里，每次绘制之前对此成员
 * 变量重新赋值。
 *
 * 利用 word 枚举类简化赋值的写法。详见 ./src/rmxp/drawable_object.hpp。
 * 对于值类型：
 *   `item.x = detail::get<word::x, decltype(item.x)>(object);`
 * 意思是将 object 的实例变量 @x 提取出来，转换成 item.x 的类型并赋值给它。
 *
 * 对于对象类型：
 *   `item.color << detail::get<word::color>(object);`
 * 意思是将 object 的实例变量 @color 各属性提取出来，赋值给 item.color。
 * 只需在 color 等类中重载 operator<< 运算符，就可以使用统一的接口。
 *
 * 对于 ID 类型：
 *   `item.bitmap = detail::get<word::bitmap, const uint64_t>(object);`
 * 这里和值类型相同，但是 detail 对 const uint64_t 做了特化处理，会提取
 * @bitmap 的 object_id，然后赋值给 item.bitmap。
 *
 * 综上，枚举类 rmxp::word 用于操作 ruby 对象的实例变量，可以提升效率，
 * 并简化编码。
 */

/// @brief 关键字枚举类，用于操作 ruby 对象的实例变量
/// 这里的枚举必须与 ruby 中的实例变量名一一对应。
/// 为了方便查找，枚举类的名称都是全小写字母，且按照字符顺序排列。
enum class word {
  active,
  alpha,
  angle,
  autotiles,
  back_opacity,
  bitmap,
  blend_type,
  blue,
  bold,
  bush_depth,
  color,
  contents,
  contents_opacity,
  cursor_count,
  cursor_rect,
  data_ptr,
  flash_color,
  flash_data,
  flash_hidden,
  gray,
  green,
  height,
  id,
  italic,
  map_data,
  mirror,
  opacity,
  ox,
  oy,
  pause,
  priorities,
  rect,
  red,
  repeat_x,
  repeat_y,
  scale_mode,
  size,
  solid,
  src_rect,
  stretch,
  strikethrough,
  tileset,
  tone,
  viewport,
  underlined,
  update_count,
  visible,
  width,
  windowskin,
  x,
  y,
  z,
  zoom_x,
  zoom_y,

  /* 表示此枚举类的总数 */
  max
};

/*
 * 使用 base::detail_ext 完成 ruby 到 C++ 类型的转换
 * 这里使用了 detail 的名称，从而可以调用 RGMLOAD 等宏。
 * 在 RGMLOAD 宏中，只使用了 detail，未限定命名空间，从而在 base 命名空间
 * 中使用的是 base::detail，在 rmxp 命名空间中使用的是 rmxp::detail，也
 * 就是 base::detail_ext<rmxp::word>。
 */
using detail = base::detail_ext<word>;

/*
 * 承上所述，枚举类型里的各个元素，将对应于 RGSS 中的值类型、对象类型和 ID
 * 类型。由于 RGSS 的特殊设计，相同名称的实例变量，总是相同的类型，即不存在
 * 某个实例变量，在不同的场合可能是值类型或者对象类型。
 * 这里使用宏定义，区分各个不同的类型，以简化后续的处理。
 * 定义 TERATE_VALUES 宏和 ITERATE_OBJECTS 宏，前者用于迭代值类型和 ID
 * 类型的实例变量对应的枚举项，后者用于迭代对象类型的实例变量对应的枚举项。
 */

/// @brief 迭代值类型和 ID 类型的元素，接受一个宏函数作为参数
#define ITERATE_VALUES(T) \
  T(active);              \
  T(alpha);               \
  T(angle);               \
  T(back_opacity);        \
  T(bitmap);              \
  T(blend_type);          \
  T(blue);                \
  T(bold);                \
  T(bush_depth);          \
  T(contents);            \
  T(contents_opacity);    \
  T(cursor_count);        \
  T(data_ptr);                \
  T(flash_data);          \
  T(flash_hidden);        \
  T(gray);                \
  T(green);               \
  T(height);              \
  T(id);                  \
  T(italic);              \
  T(map_data);            \
  T(mirror);              \
  T(opacity);             \
  T(ox);                  \
  T(oy);                  \
  T(pause);               \
  T(priorities);          \
  T(red);                 \
  T(repeat_x);            \
  T(repeat_y);            \
  T(scale_mode);          \
  T(size);                \
  T(solid);               \
  T(stretch);             \
  T(strikethrough);       \
  T(tileset);             \
  T(underlined);          \
  T(update_count);        \
  T(viewport);            \
  T(visible);             \
  T(windowskin);          \
  T(width);               \
  T(x);                   \
  T(y);                   \
  T(z);                   \
  T(zoom_x);              \
  T(zoom_y)

/// @brief 迭代对象类型的元素，接受一个宏函数作为参数
#define ITERATE_OBJECTS(T) \
  T(autotiles);            \
  T(color);                \
  T(cursor_rect);          \
  T(flash_color);          \
  T(rect);                 \
  T(src_rect);             \
  T(tone)

/// @brief 统计迭代的次数
#define DO_COUNT(key) ++count

/// @brief 将 key 对应的实例变量的 symbol 缓存到 id_table 中
#define DO_DEFINE_IV(key) \
  detail::id_table[static_cast<size_t>(word::key)] = rb_intern("@" #key)

/// @brief 将 key 对应的常量添加到 RGM::Word 模块中
#define DO_DEFINE_CONST(key)                               \
  rb_const_set(rb_mRGM_Word, rb_intern("Attribute_" #key), \
               INT2FIX(static_cast<int>(word::key)))

/// @brief 关键词枚举类 word 类相关的初始化类，定义了若干 symbol 和常量
struct init_word {
  static void before(auto&) {
    /* 统计 ITERATE_VALUES 和 ITERATE_OBJECTS 的总数 */
    constexpr size_t total_size = []() -> size_t {
      size_t count = 0;
      ITERATE_VALUES(DO_COUNT);
      ITERATE_OBJECTS(DO_COUNT);
      return count;
    }();

    /* 静态检查是否有遗漏的关键字 */
    static_assert(total_size == static_cast<size_t>(word::max),
                  "The enum class rmxp::word has unused or repeated items. "
                  "Check the macro ITERATE_VALUES and ITERATE_OBJECTS.");

    /* 向 id_table 中添加实例变量的名称对应的 symbol */
    detail::id_table.resize(total_size + 1, 0);

    ITERATE_VALUES(DO_DEFINE_IV);
    ITERATE_OBJECTS(DO_DEFINE_IV);

    /* 定义实例变量的名称对应的常量 */
    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Word = rb_define_module_under(rb_mRGM, "Word");

    ITERATE_VALUES(DO_DEFINE_CONST);
  }
#undef DO_DEFINE_IV
#undef DO_DEFINE_CONST
};
}  // namespace rgm::rmxp