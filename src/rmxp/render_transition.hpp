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