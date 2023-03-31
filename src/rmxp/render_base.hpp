// zlib License

// copyright (C) 2023 Guoxiaomi and Krimiston

// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once
#include "base/base.hpp"
#include "drawable.hpp"

namespace rgm::rmxp {
/**
 * @brief 任务：对应于不同的 Drawable 的渲染方式，通常需要特化处理。
 * @note 如果特化类不改写 run 函数，则不执行任何操作。
 *
 * @tparam T Drawable 的类型
 */
template <typename T>
struct render {
  const T* _t;
  const viewport* _v;

  void run(auto&) {}
};

struct render_tone_helper {
  const tone t;
  const cen::irect* r;

  explicit render_tone_helper(const tone& t, const cen::irect* r = nullptr)
      : t(t), r(r) {}

  void process(cen::renderer& renderer, std::function<void()> proc) {
    if (t.gray) {
      shader_tone shader(t);
      proc();
    } else if ((t.red != 0) | (t.green != 0) | (t.blue != 0)) {
      proc();
      if (auto c_add = t.color_add(); c_add.has_value()) {
        renderer.set_blend_mode(blend_type::add);
        if (r) {
          renderer.set_color(c_add.value());
          renderer.fill_rect(*r);
        } else {
          renderer.fill_with(c_add.value());
        }
      }
      if (auto c_sub = t.color_sub(); c_sub.has_value()) {
        if (shader::driver == shader::opengl) {
          renderer.set_blend_mode(blend_type::reverse);
          renderer.fill_with(cen::colors::white);

          renderer.set_blend_mode(blend_type::add);
          if (r) {
            renderer.set_color(c_sub.value());
            renderer.fill_rect(*r);
          } else {
            renderer.fill_with(c_sub.value());
          }

          renderer.set_blend_mode(blend_type::reverse);
          renderer.fill_with(cen::colors::white);
        } else {
          renderer.set_blend_mode(blend_type::sub);
          if (r) {
            renderer.set_color(c_sub.value());
            renderer.fill_rect(*r);
          } else {
            renderer.fill_with(c_sub.value());
          }
        }
      } else {
        proc();
      }
    }
  };
}  // namespace rgm::rmxp
