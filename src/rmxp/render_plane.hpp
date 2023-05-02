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
/// @brief 绘制 plane
/// plane 是无限平铺的类型，为了提升绘制效率，会作双层绘制。
template <>
struct render<plane> {
  /// @brief plane 数据的地址
  const plane* p;

  /// @brief 辅助绘制 plane 的函数，实现缩放、混合模式等特效
  /// @param renderer 渲染器
  /// @param up 上层图，源图
  /// @param down 下层图，目标图
  /// @param src_rect 源矩形，从源图选取要绘制的内容
  /// up 层的内容是用 plane 的原始素材进行了少量的平铺拼合成 src_rect 的大小。
  void blend(cen::renderer& renderer, cen::texture& up, cen::texture& down,
             const cen::irect& src_rect) {
    /* 获取 viewport，如果不存在则使用 default_viewport */
    const viewport* v = p->p_viewport ? p->p_viewport : &default_viewport;

    /*
     * 从左上角 {start_x, start_y} 开始绘制，
     * 每次绘制 {step_x, step_y} 大小，
     * 直到铺满整个 {0, 0, total_x, total_y} 的矩形区域。
     */
    int start_x = 0;
    int start_y = 0;
    int step_x = src_rect.width();
    int step_y = src_rect.height();
    int total_x = v->rect.width;
    int total_y = v->rect.height;

    start_x = (-v->ox - p->ox) % step_x;
    if (start_x > 0) start_x -= step_x;

    start_y = (-v->oy - p->oy) % step_y;
    if (start_y > 0) start_y -= step_y;

    /* 设置绘制目标和区域 */
    renderer.set_target(down);
    renderer.set_clip(cen::irect(0, 0, total_x, total_y));

    /* 设置透明度 */
    up.set_alpha_mod(p->opacity);

    /* 设置混合模式 */
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

    /* 判断是否为 opengl 渲染，且混合模式是减法 */
    const bool opengl_sub = (p->blend_type == 2) && config::opengl;

    if (opengl_sub) {
      /* OpenGL 需要使用加法和反色实现减法 */
      up.set_blend_mode(blend_type::add);
      renderer.set_blend_mode(blend_type::reverse);
    }

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

    if (opengl_sub) {
      renderer.fill_with(cen::colors::white);
    }

    /* 还原透明度 */
    up.set_alpha_mod(255);
  }

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::textures& textures = RGMDATA(base::textures);
    base::renderstack& stack = RGMDATA(base::renderstack);

    cen::texture& bitmap = textures.at(p->bitmap);

    /* 获取 viewport，如果不存在则使用 default_viewport */
    const viewport* v = p->p_viewport ? p->p_viewport : &default_viewport;

    /* 设置缩放模式 */
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

    /* 读取 plane 的各个属性 */
    const int width = std::max(1, static_cast<int>(bitmap.width() * p->zoom_x));
    const int height =
        std::max(1, static_cast<int>(bitmap.height() * p->zoom_y));
    const color& c = p->color;
    const tone& t = p->tone;

    const bool use_color =
        (c.red != 0) | (c.green != 0) | (c.blue != 0) | (c.alpha != 0);

    /*
     * 这里采用了一种更优的绘制方式，即绘制 2 次。
     * 举例，把 1x1 的 Bitmap 铺满 640x480 的屏幕，通常需要绘制
     * 640x480 = 307,200 次；但是如果第一次绘制 26x18 大小，第二次
     * 把 26x18 大小再绘制 25x18 次，那么总共绘制了 26x18+25x18 =
     * 918 次，大大减少了绘制的次数。时间复杂度从 o(n^2) -> o(n)。
     */
    const int repeat_x = 1 + static_cast<int>(sqrt(v->rect.width / width));
    const int repeat_y = 1 + static_cast<int>(sqrt(v->rect.height / height));

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

    /* 添加一个中间层 */
    stack.push_empty_layer(width * repeat_x, height * repeat_y);

    bitmap.set_blend_mode(cen::blend_mode::none);

    render_tone_helper helper(t);

    /* 应用 tone 的效果 */
    helper.process(renderer, render);

    /* 应用 color 的效果 */
    if (use_color) {
      renderer.set_blend_mode(blend_type::color);
      renderer.fill_with(cen::color(c.red, c.green, c.blue, c.alpha));
    }

    auto process = [&, this](auto& up, auto& down) {
      this->blend(renderer, up, down,
                  cen::irect(0, 0, width * repeat_x, height * repeat_y));
    };

    /* 将中间层出栈，内容绘制到新的栈顶 */
    stack.merge(process);
  }
};
}  // namespace rgm::rmxp