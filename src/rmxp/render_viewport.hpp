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
#include "render_base.hpp"

namespace rgm::rmxp {
/**
 * @brief 任务：渲染 Viewport 中内容之前的处理
 * @note 向 renderstack 添加新的层
 */
struct before_render_viewport {
  const viewport* v;

  void run(auto& worker) {
    base::renderstack& stack = RGMDATA(base::renderstack);

    stack.push_capture_layer(v->rect.x, v->rect.y, v->rect.width,
                             v->rect.height);
  }
};

/**
 * @brief 任务：渲染 Viewport 中内容之后的处理
 * @note 将 viewport 对应的层绘制到 screen 对应层上
 */
struct after_render_viewport {
  const viewport* v;

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    auto process = [&renderer, this](cen::texture& up, cen::texture& down) {
      const rect& r = v->rect;
      const color& c =
          (v->color.alpha > v->flash_color.alpha) ? v->color : v->flash_color;
      const tone& t = v->tone;
      const bool use_color =
          (c.red != 0) | (c.green != 0) | (c.blue != 0) | (c.alpha != 0);

      const cen::irect src_rect(0, 0, r.width, r.height);
      const cen::irect dst_rect(r.x, r.y, r.width, r.height);

      renderer.set_target(down);
      auto render = [&] { renderer.render(up, src_rect, dst_rect); };
      up.set_blend_mode(cen::blend_mode::none);

      render_tone_helper helper(t, &dst_rect);
      helper.process(renderer, render);

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