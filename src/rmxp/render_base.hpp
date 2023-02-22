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
    if (t.gray || (shader::driver == shader::opengl)) {
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
