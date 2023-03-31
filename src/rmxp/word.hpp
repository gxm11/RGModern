// zlib License

// copyright (C) 2023 Guoxiaomi and Krimiston

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
#include "base/base.hpp"

namespace rgm::rmxp {
// 如果要增加新的符号，这个枚举类的内容和下面三个宏的定义也需要跟随改动：
// ITERATE_VALUES：在 drawable 中出现的值类型的变量所对应的符号；
// ITERATE_OBJECTS：在 drawable 中出现的对象类型的变量所对应的符号；
// ITERATE_BUILTINS：默认类型包含的符号，见下面的注释。
// 上述的 drawable 是指 viewport/sprite/plane/window/tilemap 等。
// 约定 ITERATE_VALUES 和 ITERATE_OBJECTS 的交集为空，RGSS 默认满足条件。

/**
 * @brief 枚举类 word 列举出 ruby 层的所有实例变量的符号以简化代码，在 detail
 * 中用到。
 * @note 强烈建议 word 中只使用小写字母，并且为了方便查找，枚举类 word
 * 在批量使用时都按照字符顺序排列。
 */
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

  total
};
}  // namespace rgm::rmxp

#ifndef ITERATE_BUILTINS
// builtin 第 1 行：z, id, viewport, visible 是全部 drawable 的共有属性
// builtin 第 2 行：red, green, blue, alpha, gray 是 color 和 tone 的属性
// builtin 第 3 行：x, y, width, height 是 rect 的属性
// builtin 第 4 行：size, bold, italic, underlined, strikethrough 是 font 的属性

#define ITERATE_BUILTINS(T) \
  T(z);                     \
  T(id);                    \
  T(viewport);              \
  T(visible);               \
  T(red);                   \
  T(green);                 \
  T(blue);                  \
  T(alpha);                 \
  T(gray);                  \
  T(x);                     \
  T(y);                     \
  T(width);                 \
  T(height);                \
  T(size);                  \
  T(bold);                  \
  T(italic);                \
  T(underlined);            \
  T(strikethrough);         \
  T(solid);
#endif

#ifndef ITERATE_VALUES
// value 是 drawable 的属性中，属于值类型的那些，参见 drawable.hpp。
// 值类型的属性在 ruby 层调用对应的 attr= 方法后，会调用
// RGM::Base.drawable_set_attr 方法同步数据到 C++ 层。
// details_t::setup_constants 中定义的常量是该方法的最后一个参数。
// 具体的实现参见 drawable_object.hpp 和 drawable_base.rb。

#define ITERATE_VALUES(T) \
  T(active);              \
  T(angle);               \
  T(back_opacity);        \
  T(bitmap);              \
  T(blend_type);          \
  T(bush_depth);          \
  T(contents);            \
  T(contents_opacity);    \
  T(cursor_count);        \
  T(flash_data);          \
  T(flash_hidden);        \
  T(height);              \
  T(map_data);            \
  T(mirror);              \
  T(opacity);             \
  T(ox);                  \
  T(oy);                  \
  T(pause);               \
  T(priorities);          \
  T(repeat_x);            \
  T(repeat_y);            \
  T(scale_mode);          \
  T(stretch);             \
  T(tileset);             \
  T(update_count);        \
  T(windowskin);          \
  T(width);               \
  T(x);                   \
  T(y);                   \
  T(zoom_x);              \
  T(zoom_y)
#endif

#ifndef ITERATE_OBJECTS
// object 是 drawable 的属性中，属于对象类型的那些，参见 drawable.hpp。
// 对象类型的属性，会在每次绘制前同步。

#define ITERATE_OBJECTS(T) \
  T(autotiles);            \
  T(color);                \
  T(cursor_rect);          \
  T(flash_color);          \
  T(rect);                 \
  T(src_rect);             \
  T(tone)
#endif
