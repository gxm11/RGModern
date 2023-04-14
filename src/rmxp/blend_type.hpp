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
// 加法：s.rgb + d.rgb = [1, +, 1] [0, +, 1]
// screen：s.rgb + (1-s.rgb)*d.rgb = [1, +, 1-S] [0, +, 1]，保证透明度跟底层一样
// 反号乘法：(1-s.rgb)*d.rgb = [0, +, 1-S] [0, +, 1]，保证透明度跟底层一样
// 减法：
// 颜色、灰度：s.a*s.rgb + (1-s.a)*d.rgb = [S.a, +, 1-S.a] [0, +,
// 1]，保证透明度跟底层一样 渐变：s.a*s.rgb + (1-s.a)*d.rgb = [S.a, +, 1-S.a]
// [1, +, 1-S.a] = blend（alpha叠加） Hue：s.rgb，但是使用d.a
// 为了避免不透明度平方的问题，在viewport绘制到最终窗口上时，少乘一次opacity
// Alpha叠加 Blend：s.rgb + (1-s.a)*d.rgb = [1, +, 1-S] [1, +, 1-S.a] = blend2

/**
 * @brief 混合模式，封装了若干 SDL 默认的混合模式以及常用的自定义混合模式
 */
struct blend_type {
  static cen::blend_mode add;
  static cen::blend_mode sub;
  static cen::blend_mode reverse;
  static cen::blend_mode alpha;
  static cen::blend_mode color;
  static cen::blend_mode blend2;

  static void setup() {
    // 加法叠加
    // 公式：rgb = s.alpha * s.rgb + 1 * d.rgb, a = 0 * s.a + 1 * d.a
    add = cen::compose_blend_mode(
        cen::blend_task{cen::blend_factor::src_alpha, cen::blend_factor::one,
                        cen::blend_op::add},
        cen::blend_task{cen::blend_factor::zero, cen::blend_factor::one,
                        cen::blend_op::add});

    // 减法叠加
    // 公式：rgb = s.a * s.rgb - 1 * d.rgb, a = 0 * s.a + 1 * d.a
    sub = cen::compose_blend_mode(
        cen::blend_task{cen::blend_factor::src_alpha, cen::blend_factor::one,
                        cen::blend_op::reverse_sub},
        cen::blend_task{cen::blend_factor::zero, cen::blend_factor::one,
                        cen::blend_op::add});

    // 反色：(1 - d.rgb) * s.rgb = [1 - d, +, 0] [0, +, 1]
    // 这里用来实现减法，取 s.rgb 恒为 1，从而操作一次后会变成 1 - d，透明度 d.a
    // 比如已知 u, v，要获得 u - v
    // 首先获得 1 - u                     | 透明度 u.a
    // 然后获得 v + (1 - u)               | 透明度 u.a
    // 最后获得 1 - (v + (1 - u)) = u - v | 透明度 u.a
    reverse = cen::compose_blend_mode(
        cen::blend_task{cen::blend_factor::one_minus_dst_color,
                        cen::blend_factor::zero, cen::blend_op::add},
        cen::blend_task{cen::blend_factor::zero, cen::blend_factor::one,
                        cen::blend_op::add});

    // 修改透明度，用于实现 Sprite 的 bush_depth 效果
    // 公式：rgb = 0 * s.rgb + 1 * d.rgb, a = 0 * s.a + s.a * d.a
    alpha = cen::compose_blend_mode(
        cen::blend_task{cen::blend_factor::zero, cen::blend_factor::one,
                        cen::blend_op::add},
        cen::blend_task{cen::blend_factor::zero, cen::blend_factor::src_alpha,
                        cen::blend_op::add});
    // Alpha 叠加（仅颜色），用于实现 Sprite 等的 color 效果
    // 公式：rgb = s.a * s.rgb + (1 - s.a) * d.rgb, a = 0 * s.a + 1 * d.a
    color = cen::compose_blend_mode(
        cen::blend_task{cen::blend_factor::src_alpha,
                        cen::blend_factor::one_minus_src_alpha,
                        cen::blend_op::add},
        cen::blend_task{cen::blend_factor::zero, cen::blend_factor::one,
                        cen::blend_op::add});
    // 与 Blend 不同，少乘一次opacity。在绘制window的contents使用。
    // Alpha叠加 Blend：s.rgb + (1-s.a)*d.rgb = [1, +, 1-S] [1, +, 1-S.a] =
    // blend2
    blend2 = cen::compose_blend_mode(
        cen::blend_task{cen::blend_factor::one,
                        cen::blend_factor::one_minus_src_alpha,
                        cen::blend_op::add},
        cen::blend_task{cen::blend_factor::one,
                        cen::blend_factor::one_minus_src_alpha,
                        cen::blend_op::add});
  }
};
cen::blend_mode blend_type::add;
cen::blend_mode blend_type::sub;
cen::blend_mode blend_type::reverse;
cen::blend_mode blend_type::color;
cen::blend_mode blend_type::alpha;
cen::blend_mode blend_type::blend2;

struct init_blend_type {
  static void before(auto&) { blend_type::setup(); }
};
}  // namespace rgm::rmxp