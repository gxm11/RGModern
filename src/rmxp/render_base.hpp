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
#include "base/base.hpp"
#include "blend_type.hpp"
#include "drawable.hpp"
#include "shader/shader.hpp"
#include "viewport.hpp"

namespace rgm::rmxp {
/// @brief 绘制特定的 Drawable
/// @tparam Drawable 的类型
/// render<T> 系列的任务执行时，ruby worker 会进入等待，可以安全地访问数据。
template <typename T>
struct render {
  /// @brief Drawable 数据的地址
  const T* _t;

  void run(auto&) {}
};

/// @brief 辅助实现色调处理的类
/// 当 tone 不包含灰度的处理时，直接调用 blend_mode 实现加减法，
/// 否则调用 shader_tone 来实现。opengl 的减法需要特殊的实现方式。
struct render_tone_helper {
  /// @brief 色调变化的效果
  const tone t;

  /// @brief 应用色调变化的区域
  const cen::irect* r;

  /// @brief 构造函数
  /// @param t 色调变化的效果
  /// @param r 色调变化的区域
  [[nodiscard]] explicit render_tone_helper(const tone& t,
                                            const cen::irect* r = nullptr)
      : t(t), r(r) {}

  /// @brief 在处理特定的绘制后，应用色调的效果
  /// @param renderer 渲染器
  /// @param proc 待调制的绘制内容，在绘制之后应用色调效果。
  void process(cen::renderer& renderer, std::function<void()> proc) {
    /* 需要处理灰度时，使用 shader_tone */
    if (t.gray) {
      shader_tone shader(t);
      proc();
      return;
    }

    proc();

    /* 没有任何需要处理的颜色时，直接返回 */
    if ((t.red == 0) && (t.green == 0) && (t.blue == 0)) return;

    /* 加法的处理 */
    if (auto c_add = t.color_add(); c_add) {
      renderer.set_blend_mode(blend_type::add);
      if (r) {
        renderer.set_color(*c_add);
        renderer.fill_rect(*r);
      } else {
        renderer.fill_with(*c_add);
      }
    }

    /* 减法的处理 */
    if (auto c_sub = t.color_sub(); c_sub) {
      /* OpenGL 需要使用加法和反色实现减法 */
      if (config::opengl) {
        /* 第 1 步：反色 */
        renderer.set_blend_mode(blend_type::reverse);
        renderer.fill_with(cen::colors::white);

        /* 第 2 步：加法 */
        renderer.set_blend_mode(blend_type::add);
        if (r) {
          renderer.set_color(*c_sub);
          renderer.fill_rect(*r);
        } else {
          renderer.fill_with(*c_sub);
        }

        /* 第 3 步：反色 */
        renderer.set_blend_mode(blend_type::reverse);
        renderer.fill_with(cen::colors::white);
      } else {
        renderer.set_blend_mode(blend_type::sub);
        if (r) {
          renderer.set_color(*c_sub);
          renderer.fill_rect(*r);
        } else {
          renderer.fill_with(*c_sub);
        }
      }
    }
  }
};
}  // namespace rgm::rmxp
