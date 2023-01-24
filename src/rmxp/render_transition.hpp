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
template <size_t>
struct render_transition;

template <>
struct render_transition<1> {
  uint64_t freeze_id;
  uint64_t current_id;
  double rate;

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::textures& textures = RGMDATA(base::textures);
    base::renderstack& stack = RGMDATA(base::renderstack);
    // 准备绘制
    cen::texture& freeze = textures.at(freeze_id);
    cen::texture& current = textures.at(current_id);
    // 修改 freeze 的alpha
    uint8_t opacity = static_cast<uint8_t>(rate * 255.0);
    freeze.set_alpha_mod(255 - opacity);
    // 绘制到画面上
    renderer.set_target(stack.current());
    current.set_blend_mode(cen::blend_mode::none);
    renderer.render(current, cen::ipoint(0, 0));
    freeze.set_blend_mode(cen::blend_mode::blend);
    renderer.render(freeze, cen::ipoint(0, 0));
    freeze.set_alpha_mod(255);
  }
};

template <>
struct render_transition<2> {
  uint64_t freeze_id;
  uint64_t current_id;
  double rate;
  uint64_t transition_id;
  int vague;

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::textures& textures = RGMDATA(base::textures);
    base::renderstack& stack = RGMDATA(base::renderstack);
    // 准备绘制
    cen::texture& freeze = textures.at(freeze_id);
    cen::texture& current = textures.at(current_id);
    cen::texture& transition = textures.at(transition_id);
    // 修改 freeze 的alpha
    renderer.set_target(freeze);
    transition.set_blend_mode(blend_type::alpha);
    transition.set_scale_mode(cen::scale_mode::linear);
    {
      shader_transition shader(rate, vague);
      gl_texture t(transition);
      renderer.render(transition,
                      cen::irect(0, 0, transition.width(), transition.height()),
                      cen::irect(0, 0, freeze.width(), freeze.height()));
    }
    // 绘制到画面上
    renderer.set_target(stack.current());
    current.set_blend_mode(cen::blend_mode::none);
    renderer.render(current, cen::ipoint(0, 0));
    freeze.set_blend_mode(cen::blend_mode::blend);
    renderer.render(freeze, cen::ipoint(0, 0));
  }
};
}  // namespace rgm::rmxp