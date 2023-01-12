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
cen::blend_mode blend_type::color;
cen::blend_mode blend_type::alpha;
cen::blend_mode blend_type::blend2;

struct init_blend_type {
  static void before(auto&) { blend_type::setup(); }
};
}  // namespace rgm::rmxp