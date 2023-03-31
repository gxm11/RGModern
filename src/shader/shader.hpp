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
#include "shader_base.hpp"
#ifdef __WIN32
#include "shader_direct3d11.hpp"
#include "shader_direct3d9.hpp"
#endif
#include "shader_opengl.hpp"

namespace rgm::shader {
template <template <size_t> class T_shader>
struct shader_instance {
  using T_variant = std::variant<std::monostate, T_shader<opengl>,
                                 T_shader<direct3d9>, T_shader<direct3d11>>;

  T_variant var;

  template <typename... Args>
  shader_instance(Args... args) : var{} {
    switch (driver) {
      default:
        break;
      case opengl:
        var.template emplace<T_shader<opengl>>(args...);
        break;

      case direct3d9:
        var.template emplace<T_shader<direct3d9>>(args...);
        break;

      case direct3d11:
        var.template emplace<T_shader<direct3d11>>(args...);
        break;
    }
  }

  shader_instance(const shader_instance&) = delete;
  shader_instance(shader_instance&&) = delete;
  shader_instance& operator=(const shader_instance&) = delete;
  shader_instance& operator=(shader_instance&&) = delete;
};

struct init_shader {
  static void before(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    switch (driver) {
      default:
        return;
      case opengl:
        shader_base<opengl>::setup(renderer);
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
using init_shader = shader::init_shader;
}  // namespace rgm