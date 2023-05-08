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
#include "rmxp/builtin.hpp"

namespace rgm::shader {
/* 引入 driver_type 方便在模板参数中使用 enum 的值 */
using enum config::driver_type;

/// @brief 基本的 shader 类
/// @tparam driver 渲染器的类型
/// 此类仅用于 setup 用，不能实例化。
template <config::driver_type driver>
struct shader_base {
  /// @brief 空的 setup 函数
  static void setup(cen::renderer&) {}
};

/// @brief 静态 shader 类，即不包含任何运行时参数的 shader 类
/// @tparam driver 渲染器的类型
/// @tparam T_shader shader 的类型
template <config::driver_type driver,
          template <config::driver_type> class T_shader>
struct shader_static : shader_base<driver> {
  /// @brief 空的 setup 函数
  static void setup(cen::renderer&) {}
};

/// @brief 动态 shader 类，运行时要传入参数的 shader 类
/// @tparam driver 渲染器的类型
/// @tparam T_shader shader 的类型
template <config::driver_type driver,
          template <config::driver_type> class T_shader>
struct shader_dynamic : shader_base<driver> {
  /// @brief 空的 setup 函数
  static void setup(cen::renderer&) {}
};

/// @brief 用于实现灰度的 shader 类
/// @tparam driver 渲染器的类型，不同渲染器实现方式也不同
template <config::driver_type driver>
struct shader_gray : shader_static<driver, shader_gray> {
  explicit shader_gray() {}
};

/// @brief 用于实现色相的 shader 类
/// @tparam driver 渲染器的类型，不同渲染器实现方式也不同
template <config::driver_type driver>
struct shader_hue : shader_dynamic<driver, shader_hue> {
  /* 构造函数，必须传入色相旋转的值 */
  explicit shader_hue(int) {}
};

/// @brief 用于实现色调的 shader 类
/// @tparam driver 渲染器的类型，不同渲染器实现方式也不同
template <config::driver_type driver>
struct shader_tone : shader_dynamic<driver, shader_tone> {
  /* 构造函数，必须传入 rmxp::tone 对象 */
  explicit shader_tone(rmxp::tone) {}
};

/// @brief 用于实现渐变的 shader 类
/// @tparam driver 渲染器的类型，不同渲染器实现方式也不同
template <config::driver_type driver>
struct shader_transition : shader_dynamic<driver, shader_transition> {
  /* 构造函数，必须传入渐变的进度和模糊程度 */
  explicit shader_transition(double, int) {}
};
}  // namespace rgm::shader