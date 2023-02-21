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
#include "shader_base.hpp"
#ifdef __WIN32
#include "shader_direct3d11.hpp"
#include "shader_direct3d9.hpp"
#endif
#include "shader_opengl.hpp"

// TODO(guoxiaomi): shader应该有独立的namespace rgm::shader
// driver_types作为普通枚举类型，进入 namespace rgm::shader 里
namespace rgm::shader {
template <template <driver_types> class T>
struct shader_instance {
  using shader_t =
      std::variant<std::monostate, T<opengl>, T<direct3d9>, T<direct3d11>>;

  shader_t t;

  template <typename... Args>
  shader_instance(Args... args) {
    switch (driver) {
      default:
        break;
      case opengl:
        t.template emplace<T<opengl>>(args...);
        break;

      case direct3d9:
        t.template emplace<T<direct3d9>>(args...);
        break;

      case direct3d11:
        t.template emplace<T<direct3d11>>(args...);
        break;
    }
  }
};

struct init_shader {
  static void before(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    switch (driver) {
      default:
        return;
      case opengl:
        shader_base<opengl>::setup(renderer);
        // shader_static<opengl, shader_gray>::setup(renderer);
        shader_dynamic<opengl, shader_gray>::setup(renderer);
        shader_dynamic<opengl, shader_hue>::setup(renderer);
        shader_dynamic<opengl, shader_tone>::setup(renderer);
        shader_dynamic<opengl, shader_transition>::setup(renderer);
        return;
      case direct3d9:
        shader_base<direct3d9>::setup(renderer);
        shader_static<direct3d9, shader_gray>::setup(renderer);
        shader_dynamic<direct3d9, shader_hue>::setup(renderer);
        shader_dynamic<direct3d9, shader_tone>::setup(renderer);
        shader_dynamic<direct3d9, shader_transition>::setup(renderer);
        return;
      case direct3d11:
        shader_base<direct3d11>::setup(renderer);
        shader_static<direct3d11, shader_gray>::setup(renderer);
        shader_dynamic<direct3d11, shader_hue>::setup(renderer);
        shader_dynamic<direct3d11, shader_tone>::setup(renderer);
        shader_dynamic<direct3d11, shader_transition>::setup(renderer);
        return;
    }
  }
};
}  // namespace rgm::shader

namespace rgm {
using shader_gray = shader::shader_instance<shader::shader_gray>;
using shader_hue = shader::shader_instance<shader::shader_hue>;
using shader_tone = shader::shader_instance<shader::shader_tone>;
using shader_transition = shader::shader_instance<shader::shader_transition>;
}