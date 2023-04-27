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
#include "render_base.hpp"

namespace rgm::rmxp {
/// @brief 执行渐变绘制的任务，有多种不同的特化方式。
/// @tparam size_t 绘制方式
/// @name task
template <size_t>
struct render_transition;

/// @brief 使用线性透明度变化的渐变模式
/// @name task
template <>
struct render_transition<1> {
  /// @brief 渐变前的画面的 id
  uint64_t freeze_id;

  /// @brief 渐变后的画面的 id
  uint64_t current_id;

  /// @brief 渐变完成的程度，值在 0.0 ~ 1.0
  /// 0 是未开始，1 表示渐变结束。
  double rate;

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::textures& textures = RGMDATA(base::textures);
    base::renderstack& stack = RGMDATA(base::renderstack);

    /* 获取渐变前后对应的 Bitmap */
    cen::texture& freeze = textures.at(freeze_id);
    cen::texture& current = textures.at(current_id);

    /* 将 current 绘制到画面上 */
    renderer.set_target(stack.current());
    current.set_blend_mode(cen::blend_mode::none);
    renderer.render(current, cen::ipoint(0, 0));

    /* 根据渐变完成的程度修改 freeze 的 alpha */
    uint8_t opacity = std::clamp<uint8_t>(rate * 255, 0, 255);
    freeze.set_alpha_mod(255 - opacity);

    /* 将 freeze 绘制到画面上 */
    freeze.set_blend_mode(cen::blend_mode::blend);
    renderer.render(freeze, cen::ipoint(0, 0));
    freeze.set_alpha_mod(255);
  }
};

/// @brief 使用渐变图的渐变模式
/// @name task
template <>
struct render_transition<2> {
  /// @brief 渐变前的画面的 id
  uint64_t freeze_id;

  /// @brief 渐变后的画面的 id
  uint64_t current_id;

  /// @brief 渐变完成的程度，值在 0.0 ~ 1.0
  /// 0 是未开始，1 表示渐变结束。
  double rate;

  /// @brief 渐变图的 id
  uint64_t transition_id;

  /// @brief 渐变过渡的模糊程度，设为 0 则渐变有锋利的边缘
  int vague;

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::textures& textures = RGMDATA(base::textures);
    base::renderstack& stack = RGMDATA(base::renderstack);

    /* 获取渐变前后对应的 Bitmap */
    cen::texture& freeze = textures.at(freeze_id);
    cen::texture& current = textures.at(current_id);

    /* 获取渐变图对应的 Bitmap */
    cen::texture& transition = textures.at(transition_id);

    /* 使用 shader 修改 freeze 每个像素的透明度 */
    if (rate > 0) {
      transition.set_blend_mode(blend_type::alpha);
      transition.set_scale_mode(cen::scale_mode::linear);

      renderer.set_target(freeze);

      /* 使用 shader_transition，变量 shader 在调用 render 函数后析构 */
      shader_transition shader(rate, vague);
      renderer.render(transition,
                      cen::irect(0, 0, transition.width(), transition.height()),
                      cen::irect(0, 0, freeze.width(), freeze.height()));
    }

    /* 将 current 绘制到画面上 */
    renderer.set_target(stack.current());
    current.set_blend_mode(cen::blend_mode::none);
    renderer.render(current, cen::ipoint(0, 0));

    /* 将 freeze 绘制到画面上 */
    freeze.set_blend_mode(cen::blend_mode::blend);
    renderer.render(freeze, cen::ipoint(0, 0));
  }
};
}  // namespace rgm::rmxp