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
/// @brief 任务：渲染 Viewport 中内容之前的处理
/// @name task
struct before_render_viewport {
  /// @brief viewport 数据的地址
  const viewport* v;

  void run(auto& worker) {
    base::renderstack& stack = RGMDATA(base::renderstack);

    /* 向 renderstack 添加新的空层 */
    stack.push_capture_layer(v->rect.x, v->rect.y, v->rect.width,
                             v->rect.height);
  }
};

/// @brief 渲染 Viewport 中内容之后的处理
/// @name task
/// 先处理 viewport 的特效，然后将 viewport 绘制到 screen 层上。
struct after_render_viewport {
  /// @brief viewport 数据的地址
  const viewport* v;

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    auto process = [&renderer, this](cen::texture& up, cen::texture& down) {
      /* 设置绘制目标为下层 */
      renderer.set_target(down);

      /* 读取 viewport 的各个属性 */
      const rect& r = v->rect;
      const color& c =
          (v->color.alpha > v->flash_color.alpha) ? v->color : v->flash_color;
      const tone& t = v->tone;

      const bool use_color =
          (c.red != 0) | (c.green != 0) | (c.blue != 0) | (c.alpha != 0);

      const cen::irect src_rect(0, 0, r.width, r.height);
      const cen::irect dst_rect(r.x, r.y, r.width, r.height);

      auto render = [&] { renderer.render(up, src_rect, dst_rect); };

      up.set_blend_mode(cen::blend_mode::none);

      /* 应用 tone 的效果 */
      render_tone_helper helper(t, &dst_rect);
      helper.process(renderer, render);

      /* 应用 color 的效果 */
      if (use_color) {
        renderer.set_blend_mode(blend_type::color);
        renderer.set_color(cen::color(c.red, c.green, c.blue, c.alpha));
        renderer.fill_rect(dst_rect);
      }
    };

    stack.merge(process);
  }
};
}  // namespace rgm::rmxp