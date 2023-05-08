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
/// @brief 自定义混合模式
/// 封装了若干自定义混合模式，在实现各种绘制效果时用到
struct blend_type {
  /// @brief 加法叠加
  /// 公式：rgb = s.a * s.rgb + d.rgb, a = d.a
  inline static cen::blend_mode add = cen::blend_mode::add;

  /// @brief 减法叠加
  /// 公式：rgb = s.a * s.rgb - d.rgb, a = d.a
  inline static cen::blend_mode sub = cen::blend_mode::mul;

  /// @brief 反色
  /// 公式：rgb = (1 - d.rgb) * s.rgb, a = d.a
  /// 用于实现减法，s.rgb 始终为 1
  inline static cen::blend_mode reverse = cen::blend_mode::mod;

  /// @brief 透明度修饰
  /// 公式：rgb = d.rgb, a = s.a * d.a
  /// 用于实现 Sprite 的 bush_depth 效果
  inline static cen::blend_mode alpha = cen::blend_mode::blend;

  /// @brief 颜色修饰，alpha 叠加，但是不计算透明度
  /// 公式：rgb = s.a * s.rgb + (1 - s.a) * d.rgb, a = d.a
  /// 用于实现 Sprite 等的 color 效果
  inline static cen::blend_mode color = cen::blend_mode::mod;

  /// @brief alpha 叠加 2
  /// 公式：rgb = s.rgb + (1 - s.a) * d.rgb，a = s.a + (1 - s.a) * d.a
  /// alpha 叠加公式：rgb = s.a * s.rgb + (1 - s.a) * d.rgb
  /// 与标准的 alpha 叠加相比少计算一次透明度，s.rgb 没有乘以自身的透明度。
  /// 用于绘制 window 的 contents。
  inline static cen::blend_mode blend2 = cen::blend_mode::blend;

  static void setup() {
    /* 加法叠加 */
    add = cen::compose_blend_mode(
        cen::blend_task{cen::blend_factor::src_alpha, cen::blend_factor::one,
                        cen::blend_op::add},
        cen::blend_task{cen::blend_factor::zero, cen::blend_factor::one,
                        cen::blend_op::add});

    /* 减法叠加 */
    sub = cen::compose_blend_mode(
        cen::blend_task{cen::blend_factor::src_alpha, cen::blend_factor::one,
                        cen::blend_op::reverse_sub},
        cen::blend_task{cen::blend_factor::zero, cen::blend_factor::one,
                        cen::blend_op::add});

    /*
     * 反色：
     * 这里用来实现减法，取 s.rgb 恒为 1，从而操作一次后会变成 1 - d，透明度 d.a
     * 比如已知 u, v，要获得 u - v
     * 首先获得 1 - u                     | 透明度 u.a
     * 然后获得 v + (1 - u)               | 透明度 u.a
     * 最后获得 1 - (v + (1 - u)) = u - v | 透明度 u.a
     */
    reverse = cen::compose_blend_mode(
        cen::blend_task{cen::blend_factor::one_minus_dst_color,
                        cen::blend_factor::zero, cen::blend_op::add},
        cen::blend_task{cen::blend_factor::zero, cen::blend_factor::one,
                        cen::blend_op::add});

    /* 透明度修饰 */
    alpha = cen::compose_blend_mode(
        cen::blend_task{cen::blend_factor::zero, cen::blend_factor::one,
                        cen::blend_op::add},
        cen::blend_task{cen::blend_factor::zero, cen::blend_factor::src_alpha,
                        cen::blend_op::add});
    /* 颜色修饰 */
    color = cen::compose_blend_mode(
        cen::blend_task{cen::blend_factor::src_alpha,
                        cen::blend_factor::one_minus_src_alpha,
                        cen::blend_op::add},
        cen::blend_task{cen::blend_factor::zero, cen::blend_factor::one,
                        cen::blend_op::add});

    /*
     * alpha 叠加 2，但是少计算一次透明度
     * 与 Blend 不同之处在于，少乘一次opacity
     */
    blend2 = cen::compose_blend_mode(
        cen::blend_task{cen::blend_factor::one,
                        cen::blend_factor::one_minus_src_alpha,
                        cen::blend_op::add},
        cen::blend_task{cen::blend_factor::one,
                        cen::blend_factor::one_minus_src_alpha,
                        cen::blend_op::add});
  }
};

/// @brief 定义各种自定义的混合模式
struct init_blend_type {
  static void before(auto&) {
    if (config::driver != config::driver_type::software) {
      blend_type::setup();
    }
  }
};
}  // namespace rgm::rmxp