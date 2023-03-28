// zlib License

// copyright (C) 2023 Xiaomi Guo and Krimiston

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
template <>
struct render<window> {
  const window* w;
  const viewport* v;

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::textures& textures = RGMDATA(base::textures);
    base::renderstack& stack = RGMDATA(base::renderstack);

    cen::texture& windowskin = textures.at(w->windowskin);

    const int width = w->width;
    const int height = w->height;

    stack.push_empty_layer(w->width, w->height);
    // 1. 绘制背景图（拉伸或者平铺）
    if (w->back_opacity > 0) {
      windowskin.set_blend_mode(cen::blend_mode::none);
      renderer.set_clip(cen::irect(1, 1, width - 2, height - 2));
      windowskin.set_alpha_mod(w->back_opacity);
      const cen::irect src_rect(0, 0, 128, 128);
      if (w->stretch) {
        renderer.render(windowskin, src_rect, cen::irect(0, 0, width, height));
      } else {
        cen::irect dst_rect(0, 0, 128, 128);
        for (int x = 0; x < w->width; x += 128) {
          for (int y = 0; y < w->height; y += 128) {
            dst_rect.set_position({x, y});
            renderer.render(windowskin, src_rect, dst_rect);
          }
        }
      }
      windowskin.set_alpha_mod(255);
    }
    windowskin.set_blend_mode(cen::blend_mode::blend);
    // 2. 绘制背景边框
    // 四边
    cen::irect src_rect;
    cen::irect dst_rect;
    // 上边
    renderer.set_clip(cen::irect(2, 0, width - 4, height));
    src_rect = cen::irect(128 + 16, 0, 32, 16);
    dst_rect = cen::irect(0, 0, 32, 16);
    for (int x = 0; x < width; x += 32) {
      dst_rect.set_x(x);
      renderer.render(windowskin, src_rect, dst_rect);
    }
    // 下边
    src_rect = cen::irect(128 + 16, 64 - 16, 32, 16);
    dst_rect = cen::irect(0, height - 16, 32, 16);
    for (int x = 0; x < width; x += 32) {
      dst_rect.set_x(x);
      renderer.render(windowskin, src_rect, dst_rect);
    }
    // 左边
    renderer.set_clip(cen::irect(0, 2, width, height - 4));
    src_rect = cen::irect(128, 16, 16, 32);
    dst_rect = cen::irect(0, 0, 16, 32);
    for (int y = 0; y < height; y += 32) {
      dst_rect.set_y(y);
      renderer.render(windowskin, src_rect, dst_rect);
    }
    // 右边
    src_rect = cen::irect(128 + 64 - 16, 16, 16, 32);
    dst_rect = cen::irect(width - 16, 0, 16, 32);
    for (int y = 0; y < height; y += 32) {
      dst_rect.set_y(y);
      renderer.render(windowskin, src_rect, dst_rect);
    }
    // 四角
    renderer.reset_clip();
    renderer.render(windowskin, cen::irect(128, 0, 16, 16),
                    cen::irect(0, 0, 16, 16));
    renderer.render(windowskin, cen::irect(128 + 64 - 16, 0, 16, 16),
                    cen::irect(width - 16, 0, 16, 16));
    renderer.render(windowskin, cen::irect(128, 64 - 16, 16, 16),
                    cen::irect(0, height - 16, 16, 16));
    renderer.render(windowskin, cen::irect(128 + 64 - 16, 64 - 16, 16, 16),
                    cen::irect(width - 16, height - 16, 16, 16));

    // 将 window 框架绘制到下一层
    // 注意，滚动标记、暂停标记、cursor_rect 和 contents 在另一层
    auto process = [&, this](auto& up, auto& down) {
      renderer.set_target(down);
      const int v_ox = v ? v->ox : 0;
      const int v_oy = v ? v->oy : 0;
      if (v) {
        renderer.set_clip(cen::irect(0, 0, v->rect.width, v->rect.height));
      } else {
        renderer.reset_clip();
      }
      up.set_alpha_mod(w->opacity);
      up.set_blend_mode(cen::blend_mode::blend);
      renderer.render(up, cen::irect(0, 0, width, height),
                      cen::irect(w->x - v_ox, w->y - v_oy, width, height));
      up.set_alpha_mod(255);
    };

    stack.merge(process);
  }
};

template <>
struct render<overlayer<window>> {
  const overlayer<window>* o;
  const viewport* v;

  void run(auto& worker) { render_contents(worker); }

  void render_contents(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::textures& textures = RGMDATA(base::textures);

    const window* w = o->p_drawable;
    const int v_ox = v ? v->ox : 0;
    const int v_oy = v ? v->oy : 0;
    // 注意这里没有添加新的空白层，而是直接绘制到栈顶
    // 1. 绘制cursor_rect
    if (w->windowskin && w->contents_opacity > 0) {
      cen::texture& windowskin = textures.at(w->windowskin);
      const rect& r = w->cursor_rect;
      renderer.set_clip(
          cen::irect(w->x - v_ox, w->y - v_oy, w->width, w->height));
      if (r.width > 2 && r.height > 2) {
        int opacity = (std::abs(31 - w->cursor_count * 2) * 4 + 128);
        opacity = opacity * w->contents_opacity / 255;
        // 设置 windowskin 透明度
        windowskin.set_alpha_mod(opacity);
        int dst_x = w->x + r.x + 16 - v_ox;
        int dst_y = w->y + r.y + 16 - v_oy;
        // 中心
        renderer.render(
            windowskin, cen::irect(128 + 1, 64 + 1, 32 - 2, 32 - 2),
            cen::irect(dst_x + 1, dst_y + 1, r.width - 2, r.height - 2));
        // 四边
        renderer.render(windowskin, cen::irect(128 + 1, 64, 30, 1),
                        cen::irect(dst_x + 1, dst_y, r.width - 2, 1));
        renderer.render(windowskin, cen::irect(128, 64 + 1, 1, 30),
                        cen::irect(dst_x, dst_y + 1, 1, r.height - 2));
        renderer.render(
            windowskin, cen::irect(128 + 1, 64 + 31, 30, 1),
            cen::irect(dst_x + 1, dst_y + r.height - 1, r.width - 2, 1));
        renderer.render(
            windowskin, cen::irect(128 + 31, 64 + 1, 1, 30),
            cen::irect(dst_x + r.width - 1, dst_y + 1, 1, r.height - 2));
        // 四角
        renderer.render(windowskin, cen::irect(128, 64, 1, 1),
                        cen::irect(dst_x, dst_y, 1, 1));
        renderer.render(windowskin, cen::irect(128 + 31, 64, 1, 1),
                        cen::irect(dst_x + r.width - 1, dst_y, 1, 1));
        renderer.render(windowskin, cen::irect(128, 64 + 31, 1, 1),
                        cen::irect(dst_x, dst_y + r.height - 1, 1, 1));
        renderer.render(
            windowskin, cen::irect(128 + 31, 64 + 31, 1, 1),
            cen::irect(dst_x + r.width - 1, dst_y + r.height - 1, 1, 1));
        // 还原透明度
        windowskin.set_alpha_mod(255);
      }
      renderer.reset_clip();
    }
    // 2. 绘制窗口内部的内容
    if (w->contents && w->contents_opacity > 0) {
      cen::texture& contents = textures.at(w->contents);
      contents.set_blend_mode(blend_type::blend2);
      contents.set_alpha_mod(w->contents_opacity);
      renderer.set_clip(cen::irect(w->x - v_ox + 16, w->y - v_oy + 16,
                                   w->width - 32, w->height - 32));
      renderer.render(contents, cen::ipoint(w->x - w->ox - v_ox + 16,
                                            w->y - w->oy - v_oy + 16));
      renderer.reset_clip();
      contents.set_alpha_mod(255);
    }
    // 3. 绘制滚动标记、暂停标记
    if (w->windowskin) {
      cen::texture& windowskin = textures.at(w->windowskin);
      windowskin.set_blend_mode(cen::blend_mode::blend);
      // 绘制滚动标记
      if (w->contents) {
        cen::texture& contents = textures.at(w->contents);
        // 左
        if (0 - w->ox < 0) {
          cen::irect src_rect(128 + 16, 24, 8, 16);
          cen::irect dst_rect(w->x + 4 - v_ox,
                              (w->y + w->height) / 2 - 8 - v_oy, 8, 16);
          renderer.render(windowskin, src_rect, dst_rect);
        }
        // 右
        if (contents.width() - w->ox > w->width - 32) {
          cen::irect src_rect(128 + 40, 24, 8, 16);
          cen::irect dst_rect(w->x + w->width - 12 - v_ox,
                              w->y + w->height / 2 - 8 - v_oy, 8, 16);
          renderer.render(windowskin, src_rect, dst_rect);
        }
        // 上
        if (0 - w->oy < 0) {
          cen::irect src_rect(128 + 24, 16, 16, 8);
          cen::irect dst_rect(w->x + w->width / 2 - 8 - v_ox, w->y + 4 - v_oy,
                              16, 8);
          renderer.render(windowskin, src_rect, dst_rect);
        }
        // 下
        if (contents.height() - w->oy > w->height - 32) {
          cen::irect src_rect(128 + 24, 40, 16, 8);
          cen::irect dst_rect(w->x + w->width / 2 - 8 - v_ox,
                              w->y + w->height - 12 - v_oy, 16, 8);
          renderer.render(windowskin, src_rect, dst_rect);
        }
      }
      // 绘制暂停标记
      if (w->pause) {
        int src_x = (w->update_count & 0b01000) ? 128 + 32 + 16 : 128 + 32;
        int src_y = (w->update_count & 0b10000) ? 64 + 16 : 64;
        cen::irect src_rect(src_x, src_y, 16, 16);
        cen::irect dst_rect(w->x + w->width / 2 - 8 - v_ox,
                            w->y + w->height - 16 - v_oy, 16, 16);
        renderer.render(windowskin, src_rect, dst_rect);
      }
    }
  }
};
}  // namespace rgm::rmxp