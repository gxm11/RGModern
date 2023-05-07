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
#include "shader_base.hpp"
#ifdef __WIN32
#include "shader_direct3d11.hpp"
#include "shader_direct3d9.hpp"
#endif
#include "shader_opengl.hpp"

namespace rgm::shader {
/// @brief shader 的实例类，用于简化 shader 使用时的写法
template <template <config::driver_type> class T_shader>
struct shader_instance {
  using T_variant = std::variant<std::monostate, T_shader<opengl>,
                                 T_shader<direct3d9>, T_shader<direct3d11>>;
  /// @brief 存储某个 shader 对象的 variant
  T_variant var;

  /// @brief 构造函数，在 variant 中原位构造特定的 shader
  /// @tparam ...Args shader 的构造函数所需参数的类型
  /// @param ...args shader 的构造函数所需参数
  template <typename... Args>
  [[nodiscard]] shader_instance(Args... args) : var{} {
    switch (config::driver) {
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

  /* 显式移除 shader_instance 其他的构造函数 */
  shader_instance(const shader_instance&) = delete;
  shader_instance(shader_instance&&) = delete;
  shader_instance& operator=(const shader_instance&) = delete;
  shader_instance& operator=(shader_instance&&) = delete;
};

/// @brief shader 相关的初始化类
struct init_shader {
  static void before(auto& worker) noexcept {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;

    /* 根据不同的 driver 初始化对应的 shader */
    switch (config::driver) {
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
/* 定义以下类型，简化 shader 调用时的写法 */
using shader_gray = shader::shader_instance<shader::shader_gray>;
using shader_hue = shader::shader_instance<shader::shader_hue>;
using shader_tone = shader::shader_instance<shader::shader_tone>;
using shader_transition = shader::shader_instance<shader::shader_transition>;
}  // namespace rgm