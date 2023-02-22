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
template <>
struct render<plane> {
  const plane* p;
  const viewport* v;

  void blend(cen::renderer& renderer, cen::texture& up, cen::texture& down,
             const cen::irect& src_rect) {
    int start_x = 0;
    int start_y = 0;
    int step_x = src_rect.width();
    int step_y = src_rect.height();
    int total_x = 0;
    int total_y = 0;

    if (v) {
      total_x = v->rect.width;
      start_x = (-v->ox - p->ox) % total_x;
      if (start_x > 0) start_x -= total_x;

      total_y = v->rect.height;
      start_y = (-v->oy - p->oy) % total_y;
      if (start_y > 0) start_y -= total_y;
      renderer.set_clip(cen::irect(0, 0, total_x, total_y));
    } else {
      total_x = down.width();
      start_x = (-p->ox) % total_x;
      if (start_x > 0) start_x -= total_x;

      total_y = down.height();
      start_y = (-p->oy) % total_y;
      if (start_y > 0) start_y -= total_y;
      renderer.reset_clip();
    }

    renderer.set_target(down);

    up.set_alpha_mod(p->opacity);
    switch (p->blend_type) {
      case 0:
      default:
        up.set_blend_mode(cen::blend_mode::blend);
        break;
      case 1:
        up.set_blend_mode(blend_type::add);
        break;
      case 2:
        up.set_blend_mode(blend_type::sub);
        break;
    }
    // opengl的情况下，使用reverse代替sub（第1步）
    const bool opengl_sub =
        (p->blend_type == 2) && (shader::driver == shader::opengl);
    if (opengl_sub) {
      up.set_blend_mode(blend_type::add);
      renderer.set_blend_mode(blend_type::reverse);
    }
    // opengl的情况下，使用reverse代替sub（第2步）
    if (opengl_sub) {
      renderer.fill_with(cen::colors::white);
    }
    cen::irect dst_rect(0, 0, step_x, step_y);
    for (int i = start_x; i < total_x; i += step_x) {
      for (int j = start_y; j < total_y; j += step_y) {
        dst_rect.set_position(i, j);
        renderer.render(up, src_rect, dst_rect);
      }
    }
    // opengl的情况下，使用reverse代替sub（第3步）
    if (opengl_sub) {
      renderer.fill_with(cen::colors::white);
    }
    up.set_alpha_mod(255);
  }

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::textures& textures = RGMDATA(base::textures);
    base::renderstack& stack = RGMDATA(base::renderstack);

    cen::texture& bitmap = textures.at(p->bitmap);
    // 设置缩放模式
    switch (p->scale_mode) {
      case 0:
      default:
        bitmap.set_scale_mode(cen::scale_mode::nearest);
        break;
      case 1:
        bitmap.set_scale_mode(cen::scale_mode::linear);
        break;
      case 2:
        bitmap.set_scale_mode(cen::scale_mode::best);
        break;
    }

    const int width = std::max(1, static_cast<int>(bitmap.width() * p->zoom_x));
    const int height =
        std::max(1, static_cast<int>(bitmap.height() * p->zoom_y));
    const color& c = p->color;
    const tone& t = p->tone;
    const bool use_color =
        (c.red != 0) | (c.green != 0) | (c.blue != 0) | (c.alpha != 0);

    const int repeat_x =
        1 + static_cast<int>(sqrt((v ? v->rect.width : 512) / width));
    const int repeat_y =
        1 + static_cast<int>(sqrt((v ? v->rect.height : 512) / height));

    auto render = [=, &renderer, &bitmap] {
      const cen::irect src_rect(0, 0, bitmap.width(), bitmap.height());
      cen::irect dst_rect(0, 0, width, height);
      for (int i = 0; i < repeat_x; ++i) {
        for (int j = 0; j < repeat_y; ++j) {
          dst_rect.set_position(width * i, height * j);
          renderer.render(bitmap, src_rect, dst_rect);
        }
      }
    };

    stack.push_empty_layer(width * repeat_x, height * repeat_y);

    bitmap.set_blend_mode(cen::blend_mode::none);

    render_tone_helper helper(t);
    helper.process(renderer, render);

    if (use_color) {
      renderer.set_blend_mode(blend_type::color);
      renderer.fill_with(cen::color(c.red, c.green, c.blue, c.alpha));
    }

    auto process = [&, this](auto& up, auto& down) {
      this->blend(renderer, up, down,
                  cen::irect(0, 0, width * repeat_x, height * repeat_y));
    };

    stack.merge(process);
  }
};
}  // namespace rgm::rmxp