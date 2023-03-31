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
#include "render_base.hpp"

namespace rgm::rmxp {
// todo: 一定不会显示的图片，以及减少 set_target
// visible
// 还可以更多的优化，比如由于坐标太偏，一定不能显示在viewport上或者screen上时
// 注意 viewport 的 ox 和 oy？可能要在render处做

/**
 * @brief 任务：对应于 Sprite 的渲染方式
 * @tparam sprite 特化版本
 * @note 根据是否有 bush_depth 和 color 的处理，没有的话就不向 stack
 * 中添加新的层。 按照以下顺序绘制：
 * 1. src_rect
 * 2. tone / bush_depth
 * 3. color / flash_color
 * 4. opacity
 * 5. blend_type
 * 6. x, y, ox, oy, angle, mirror, zoom_x, zoom_y
 */
template <>
struct render<sprite> {
  const sprite* s;
  const viewport* v;

  /**
   * @brief Sprite 绘制到 viewport 或 screen 上的方式
   *
   * @param renderer SDL 渲染器
   * @param up 上层图，源图
   * @param down 下层图，目标图
   * @param src_rect 源图中待绘制的截取区域
   * @param dst_rect 目标图中的目标区域
   */
  void blend(cen::renderer& renderer, cen::texture& up, cen::texture& down,
             const cen::irect& src_rect, const cen::frect& dst_rect) {
    // 5. opacity
    up.set_alpha_mod(s->opacity);
    // 设置渲染模式
    switch (s->blend_type) {
      case 0:
      default:
        up.set_blend_mode(cen::blend_mode::blend);
        break;
      case 1:
        up.set_blend_mode(blend_type::add);
        break;
      case 2:
        // opengl的情况下，使用reverse代替sub（第1步）

        up.set_blend_mode(blend_type::sub);

        break;
    }
    // opengl的情况下，使用reverse代替sub（第1步）
    const bool opengl_sub =
        (s->blend_type == 2) && (shader::driver == shader::opengl);
    if (opengl_sub) {
      up.set_blend_mode(blend_type::add);
      renderer.set_blend_mode(blend_type::reverse);
    }
    // 设置缩放模式
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
    // 如果 down 不是目标，设置渲染目标
    if (down.get() != renderer.get_target().get()) {
      renderer.set_target(down);
    }
    if (v) {
      renderer.set_clip(cen::irect(0, 0, v->rect.width, v->rect.height));
    } else {
      renderer.reset_clip();
    }
    // opengl的情况下，使用reverse代替sub（第2步）
    if (opengl_sub) {
      renderer.fill_with(cen::colors::white);
    }
    // 绘制画面
    renderer.render(
        up, src_rect, dst_rect, -s->angle, cen::fpoint(s->ox, s->oy),
        s->mirror ? cen::renderer_flip::horizontal : cen::renderer_flip::none);
    // opengl的情况下，使用reverse代替sub（第3步）
    if (opengl_sub) {
      renderer.fill_with(cen::colors::white);
    }
    // reset renderer and source bitmap (up)
    up.set_alpha_mod(255);
  }

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::textures& textures = RGMDATA(base::textures);
    base::renderstack& stack = RGMDATA(base::renderstack);

    cen::texture& bitmap = textures.at(s->bitmap);

    const rect& r = s->src_rect;
    const color& c =
        (s->color.alpha > s->flash_color.alpha) ? s->color : s->flash_color;
    const tone& t = s->tone;

    // src_rect 的 width 和 height 设置为 0 时，使用 bitmap 的尺寸
    const int width = r.width ? r.width : bitmap.width();
    const int height = r.height ? r.height : bitmap.height();
    const bool use_color =
        (c.red != 0) | (c.green != 0) | (c.blue != 0) | (c.alpha != 0);
    const bool use_bush = (s->bush_depth > 0);
    const bool use_tone =
        (t.red != 0) | (t.green != 0) | (t.blue != 0) | (t.gray != 0);

    // 设置图形的缩放和位置
    // viewport ox, oy
    const int v_ox = v ? v->ox : 0;
    const int v_oy = v ? v->oy : 0;

    auto render = [=, &renderer, &bitmap, this] {
      renderer.render(bitmap, cen::irect(r.x, r.y, width, height),
                      cen::irect(0, 0, width, height));
    };

    // 被引用捕获的 src_rect，当起新的一层绘制时，x 和 y 要设置为 0
    cen::irect src_rect(r.x, r.y, width, height);
    const cen::frect dst_rect(s->x - s->ox * s->zoom_x - v_ox,
                              s->y - s->oy * s->zoom_y - v_oy,
                              width * s->zoom_x, height * s->zoom_y);
    // 绘制流程，捕获renderer，this 和上面的常量
    auto process = [&, this](auto& up, auto& down) {
      this->blend(renderer, up, down, src_rect, dst_rect);
    };

    // 根据不同的情况绘制
    // 设置了 color 和 bush_depth 时，需要一个额外的层缓存并修改 bitmap
    // 未设置时，可以将 bitmap 直接绘制
    // shader_tone 有 raii 机制，使得接下来的 render 附带色调改变的效果
    if (use_color | use_bush | use_tone) {
      stack.push_empty_layer(width, height);
      bitmap.set_blend_mode(cen::blend_mode::none);

      render_tone_helper helper(t);
      helper.process(renderer, render);

      if (use_bush) {
        int bush_depth = s->bush_depth > height ? height : s->bush_depth;
        cen::irect bush_rect{0, height - bush_depth, width, bush_depth};
        renderer.set_blend_mode(blend_type::alpha);
        renderer.set_color(cen::color(0, 0, 0, 127));
        renderer.fill_rect(bush_rect);
      }
      if (use_color) {
        renderer.set_blend_mode(blend_type::color);
        renderer.fill_with(cen::color(c.red, c.green, c.blue, c.alpha));
      }
      src_rect.set_position(0, 0);
      stack.merge(process);
    } else {
      stack.merge(process, bitmap);
    }
  }
};
}  // namespace rgm::rmxp