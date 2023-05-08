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
/// @brief 绘制 sprite
/// sprite 是最常见的 drawable 元素
/// 根据是否有 color 等特效的处理，决定是单层绘制还是双层绘制。
template <>
struct render<sprite> {
  /// @brief sprite 数据的地址
  const sprite* s;

  /// @brief 辅助绘制 sprite 的函数，实现缩放、混合模式等特效
  /// @param renderer 渲染器
  /// @param up 上层图，源图
  /// @param down 下层图，目标图
  /// @param src_rect 源矩形，从源图选取要绘制的内容
  /// @param dst_rect 目标矩形，目标图需要绘制的区域
  /// up 层通常是 bitmap 本身，但也可能是一个新层，用于处理若干绘制效果。
  void blend(cen::renderer& renderer, cen::texture& up, cen::texture& down,
             const cen::irect& src_rect, const cen::frect& dst_rect) const {
    /* 设置透明度 */
    up.set_alpha_mod(s->opacity);

    /* 设置混合模式 */
    switch (s->blend_type) {
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
    const bool opengl_sub = (s->blend_type == 2) && config::opengl;

    if (opengl_sub) {
      /* OpenGL 需要使用加法和反色实现减法 */
      up.set_blend_mode(blend_type::add);
      renderer.set_blend_mode(blend_type::reverse);
    }

    /* 设置缩放模式 */
    switch (s->scale_mode) {
      case 0:
      default:
        up.set_scale_mode(cen::scale_mode::nearest);
        break;
      case 1:
        up.set_scale_mode(cen::scale_mode::linear);
        break;
      case 2:
        up.set_scale_mode(cen::scale_mode::best);
        break;
    }

    /* 获取 viewport，如果不存在则使用 default_viewport */
    const viewport* v = s->p_viewport ? s->p_viewport : &default_viewport;

    /* 如果 down 不是目标，设置为渲染目标 */
    if (down.get() != renderer.get_target().get()) {
      renderer.set_target(down);
    }

    /* 设置绘制区域 */
    renderer.set_clip(cen::irect(0, 0, v->rect.width, v->rect.height));

    if (opengl_sub) {
      renderer.fill_with(cen::colors::white);
    }

    /*
     * 绘制 sprite 到特定位置。
     * 在这里应用 旋转 / 缩放 / 翻转 的效果。
     */
    renderer.render(
        up, src_rect, dst_rect, -s->angle, cen::fpoint(s->ox, s->oy),
        s->mirror ? cen::renderer_flip::horizontal : cen::renderer_flip::none);

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

    cen::texture& bitmap = textures.at(s->bitmap);

    const rect& r = s->src_rect;

    /* src_rect 的 width 和 height 设置为 0 时，使用 bitmap 的尺寸 */
    const int width = r.width ? r.width : bitmap.width();
    const int height = r.height ? r.height : bitmap.height();

    /* 获取 viewport，如果不存在则使用 default_viewport */
    const viewport* v = s->p_viewport ? s->p_viewport : &default_viewport;

    /* 设置源矩形和目标矩形 */
    cen::irect src_rect(r.x, r.y, width, height);
    const cen::frect dst_rect(s->x - s->ox * s->zoom_x - v->ox,
                              s->y - s->oy * s->zoom_y - v->oy,
                              width * s->zoom_x, height * s->zoom_y);

    /*
     * 根据目标矩形判断是否需要跳过绘制：
     * 1. angle = 0 时，判断 dst_rect 是否在 viewport 之外。
     * 2. angle != 0 时，判断图片中与 ox, oy 最远的那个点，设距离为 R，
     *    以 (ox, oy) 为原点，R 为半径画圆，判断圆是否在 viewport 之外。
     */
    int target_width = v->rect.width;
    int target_height = v->rect.height;

    /* 判断不再绘制的阈值 */
    constexpr int d = 8;

    if (s->angle == 0.0) {
      if (dst_rect.x() + dst_rect.width() < -d) return;
      if (dst_rect.y() + dst_rect.height() < -d) return;
      if (dst_rect.x() > target_width + d) return;
      if (dst_rect.y() > target_height + d) return;
    } else {
      int dx = std::max(std::abs(s->ox), std::abs(width - s->ox)) * s->zoom_x;
      int dy = std::max(std::abs(s->oy), std::abs(height - s->oy)) * s->zoom_y;
      float radius = std::sqrt(dx * dx + dy * dy);

      if (dst_rect.x() + radius < -d) return;
      if (dst_rect.y() + radius < -d) return;
      if (dst_rect.x() - radius > target_width + d) return;
      if (dst_rect.y() - radius > target_height + d) return;
    }

    /* 读取 sprite 的各个属性 */
    const color& c =
        (s->color.alpha > s->flash_color.alpha) ? s->color : s->flash_color;
    const tone& t = s->tone;

    const bool use_color =
        (c.red != 0) | (c.green != 0) | (c.blue != 0) | (c.alpha != 0);
    const bool use_bush = (s->bush_depth > 0);
    const bool use_tone =
        (t.red != 0) | (t.green != 0) | (t.blue != 0) | (t.gray != 0);

    auto process = [&, this](auto& up, auto& down) {
      this->blend(renderer, up, down, src_rect, dst_rect);
    };

    if (use_color | use_bush | use_tone) {
      /* 在有 color / bush / tone 的场合，分双层绘制 */
      auto render = [=, &renderer, &bitmap, this] {
        renderer.render(bitmap, cen::irect(r.x, r.y, width, height),
                        cen::irect(0, 0, width, height));
      };

      /* 添加一个中间层 */
      stack.push_empty_layer(width, height);
      bitmap.set_blend_mode(cen::blend_mode::none);

      /* 应用 tone 的效果 */
      render_tone_helper helper(t);
      helper.process(renderer, render);

      /* 应用 bush 的效果 */
      if (use_bush) {
        int bush_depth = s->bush_depth > height ? height : s->bush_depth;
        cen::irect bush_rect{0, height - bush_depth, width, bush_depth};
        renderer.set_blend_mode(blend_type::alpha);
        renderer.set_color(cen::color(0, 0, 0, 127));
        renderer.fill_rect(bush_rect);
      }

      /* 应用 color 的效果 */
      if (use_color) {
        renderer.set_blend_mode(blend_type::color);
        renderer.fill_with(cen::color(c.red, c.green, c.blue, c.alpha));
      }

      /* 还原 src_rect */
      src_rect.set_position(0, 0);

      /* 将中间层出栈，内容绘制到新的栈顶 */
      stack.merge(process);
    } else {
      /* 无复杂特效的场合，直接绘制到栈顶 */
      stack.merge(process, bitmap);
    }
  }
};
}  // namespace rgm::rmxp