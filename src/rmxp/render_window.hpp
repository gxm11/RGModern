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
/// @brief 绘制 window 的窗口背景层
/// window 有 2 层，分别是窗口背景和窗口内容，窗口内容的 z 值多 2，
/// 在 overlayer<window> 中绘制。以下内容也在 overlayer 层中绘制：
/// 滚动标记、暂停标记和 cursor_rect。
template <>
struct render<window> {
  /// @brief window 数据的地址
  const window* w;

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::textures& textures = RGMDATA(base::textures);
    base::renderstack& stack = RGMDATA(base::renderstack);

    /* 获取窗口皮肤对应的 Bitmap */
    cen::texture& skin = textures.at(w->windowskin);

    const int width = w->width;
    const int height = w->height;

    /* 添加一个中间层 */
    stack.push_empty_layer(width, height);

    /* 1. 绘制背景图（拉伸或者平铺）*/
    if (w->back_opacity > 0) {
      /* 最外部的一圈像素点不受影响 */
      renderer.set_clip(cen::irect(1, 1, width - 2, height - 2));

      skin.set_blend_mode(cen::blend_mode::none);
      skin.set_alpha_mod(w->back_opacity);

      const cen::irect src_rect(0, 0, 128, 128);

      if (w->stretch) {
        /* 拉伸 */
        renderer.render(skin, src_rect, cen::irect(0, 0, width, height));
      } else {
        /* 平铺 */
        cen::irect dst_rect(0, 0, 128, 128);
        for (int x = 0; x < width; x += 128) {
          for (int y = 0; y < height; y += 128) {
            dst_rect.set_position({x, y});
            renderer.render(skin, src_rect, dst_rect);
          }
        }
      }
      skin.set_alpha_mod(255);
    }

    /* 2. 绘制背景边框 */
    skin.set_blend_mode(cen::blend_mode::blend);

    /* 边框四边 */
    cen::irect src_rect;
    cen::irect dst_rect;

    /* 上边 */
    renderer.set_clip(cen::irect(2, 0, width - 4, height));
    src_rect = cen::irect(128 + 16, 0, 32, 16);
    dst_rect = cen::irect(0, 0, 32, 16);
    for (int x = 0; x < width; x += 32) {
      dst_rect.set_x(x);
      renderer.render(skin, src_rect, dst_rect);
    }

    /* 下边 */
    src_rect = cen::irect(128 + 16, 64 - 16, 32, 16);
    dst_rect = cen::irect(0, height - 16, 32, 16);
    for (int x = 0; x < width; x += 32) {
      dst_rect.set_x(x);
      renderer.render(skin, src_rect, dst_rect);
    }

    /* 左边 */
    renderer.set_clip(cen::irect(0, 2, width, height - 4));
    src_rect = cen::irect(128, 16, 16, 32);
    dst_rect = cen::irect(0, 0, 16, 32);
    for (int y = 0; y < height; y += 32) {
      dst_rect.set_y(y);
      renderer.render(skin, src_rect, dst_rect);
    }

    /* 右边 */
    src_rect = cen::irect(128 + 64 - 16, 16, 16, 32);
    dst_rect = cen::irect(width - 16, 0, 16, 32);
    for (int y = 0; y < height; y += 32) {
      dst_rect.set_y(y);
      renderer.render(skin, src_rect, dst_rect);
    }

    /* 边框四角 */
    renderer.reset_clip();
    /* 左上角 */
    renderer.render(skin, cen::irect(128, 0, 16, 16), cen::irect(0, 0, 16, 16));
    /* 右上角 */
    renderer.render(skin, cen::irect(128 + 64 - 16, 0, 16, 16),
                    cen::irect(width - 16, 0, 16, 16));
    /* 左下角 */
    renderer.render(skin, cen::irect(128, 64 - 16, 16, 16),
                    cen::irect(0, height - 16, 16, 16));
    /* 右下角 */
    renderer.render(skin, cen::irect(128 + 64 - 16, 64 - 16, 16, 16),
                    cen::irect(width - 16, height - 16, 16, 16));

    auto process = [&, this](auto& up, auto& down) {
      /* 获取 viewport，如果不存在则使用 default_viewport */
      const viewport* v = w->p_viewport ? w->p_viewport : &default_viewport;

      /* 设置绘制目标和区域 */
      renderer.set_target(down);
      renderer.set_clip(cen::irect(0, 0, v->rect.width, v->rect.height));

      /* 设置透明度 */
      up.set_alpha_mod(w->opacity);

      /* 绘制 window 到特定位置 */
      up.set_blend_mode(cen::blend_mode::blend);
      renderer.render(up, cen::irect(0, 0, width, height),
                      cen::irect(w->x - v->ox, w->y - v->oy, width, height));

      /* 还原透明度 */
      up.set_alpha_mod(255);
    };

    /* 将中间层出栈，内容绘制到新的栈顶 */
    stack.merge(process);
  }
};

/// @brief 绘制 window 的窗口内容层
/// 在 overlayer 层中绘制：
/// 滚动标记、暂停标记、cursor_rect 和 content。
template <>
struct render<overlayer<window>> {
  /// @brief overlayer<window> 数据的地址
  const overlayer<window>* o;

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::textures& textures = RGMDATA(base::textures);

    /* 获取对应的 window 数据 */
    const window* w = o->p_drawable;

    /* 获取 viewport，如果不存在则使用 default_viewport */
    const viewport* v = w->p_viewport ? w->p_viewport : &default_viewport;

    /* 注意，这里没有添加新的空白层，而是直接绘制到栈顶 */

    /* 1. 绘制cursor_rect */
    if (w->windowskin && w->contents_opacity > 0) {
      cen::texture& skin = textures.at(w->windowskin);
      const rect& r = w->cursor_rect;

      /* 避免绘制到窗口之外 */
      renderer.set_clip(
          cen::irect(w->x - v->ox, w->y - v->oy, w->width, w->height));

      /* 如果窗口太小就不绘制了 */
      if (r.width > 2 && r.height > 2) {
        /* 实现 cursor_rect 的闪烁效果 */
        int opacity = (std::abs(31 - w->cursor_count * 2) * 4 + 128);

        /* 透明度要叠加 content 的透明度 */
        opacity = opacity * w->contents_opacity / 255;

        /* 设置 windowskin 透明度 */
        skin.set_alpha_mod(opacity);

        /* 设置在栈顶 texture 的目标位置 */
        int dst_x = w->x + r.x + 16 - v->ox;
        int dst_y = w->y + r.y + 16 - v->oy;

        /* 中心 */
        renderer.render(
            skin, cen::irect(128 + 1, 64 + 1, 32 - 2, 32 - 2),
            cen::irect(dst_x + 1, dst_y + 1, r.width - 2, r.height - 2));

        /* 四边 */
        renderer.render(skin, cen::irect(128 + 1, 64, 30, 1),
                        cen::irect(dst_x + 1, dst_y, r.width - 2, 1));
        renderer.render(skin, cen::irect(128, 64 + 1, 1, 30),
                        cen::irect(dst_x, dst_y + 1, 1, r.height - 2));
        renderer.render(
            skin, cen::irect(128 + 1, 64 + 31, 30, 1),
            cen::irect(dst_x + 1, dst_y + r.height - 1, r.width - 2, 1));
        renderer.render(
            skin, cen::irect(128 + 31, 64 + 1, 1, 30),
            cen::irect(dst_x + r.width - 1, dst_y + 1, 1, r.height - 2));

        /* 四角 */
        renderer.render(skin, cen::irect(128, 64, 1, 1),
                        cen::irect(dst_x, dst_y, 1, 1));
        renderer.render(skin, cen::irect(128 + 31, 64, 1, 1),
                        cen::irect(dst_x + r.width - 1, dst_y, 1, 1));
        renderer.render(skin, cen::irect(128, 64 + 31, 1, 1),
                        cen::irect(dst_x, dst_y + r.height - 1, 1, 1));
        renderer.render(
            skin, cen::irect(128 + 31, 64 + 31, 1, 1),
            cen::irect(dst_x + r.width - 1, dst_y + r.height - 1, 1, 1));

        /* 还原 skin 的透明度 */
        skin.set_alpha_mod(255);
      }
      renderer.reset_clip();
    }

    /* 2. 绘制窗口内容 */
    /* 如果没有 contents 或者 contents 是透明的则跳过绘制 */
    if (w->contents && w->contents_opacity > 0) {
      /* 获取 contents 对应的 Bitmap */
      cen::texture& contents = textures.at(w->contents);

      /* 设置 contents 的透明度 */
      contents.set_alpha_mod(w->contents_opacity);

      /* 这里需要使用 blend2 模式，少计算一次透明度 */
      contents.set_blend_mode(blend_type::blend2);

      renderer.set_clip(cen::irect(w->x - v->ox + 16, w->y - v->oy + 16,
                                   w->width - 32, w->height - 32));

      renderer.render(contents, cen::ipoint(w->x - w->ox - v->ox + 16,
                                            w->y - w->oy - v->oy + 16));

      renderer.reset_clip();

      /* 还原 contents 的透明度 */
      contents.set_alpha_mod(255);
    }

    /* 3. 绘制滚动标记、暂停标记 */
    if (w->windowskin) {
      cen::texture& skin = textures.at(w->windowskin);
      skin.set_blend_mode(cen::blend_mode::blend);

      /* 滚动标记 */
      /* 滚动标记表示窗口内容在此处越界，没有窗口内容，就不需要绘制 */
      if (w->contents) {
        /* 需要拿到窗口的宽和高 */
        cen::texture& contents = textures.at(w->contents);

        /* 左边 */
        if (0 - w->ox < 0) {
          cen::irect src_rect(128 + 16, 24, 8, 16);
          cen::irect dst_rect(w->x + 4 - v->ox,
                              (w->y + w->height) / 2 - 8 - v->oy, 8, 16);
          renderer.render(skin, src_rect, dst_rect);
        }
        /* 右边 */
        if (contents.width() - w->ox > w->width - 32) {
          cen::irect src_rect(128 + 40, 24, 8, 16);
          cen::irect dst_rect(w->x + w->width - 12 - v->ox,
                              w->y + w->height / 2 - 8 - v->oy, 8, 16);
          renderer.render(skin, src_rect, dst_rect);
        }
        /* 上边 */
        if (0 - w->oy < 0) {
          cen::irect src_rect(128 + 24, 16, 16, 8);
          cen::irect dst_rect(w->x + w->width / 2 - 8 - v->ox, w->y + 4 - v->oy,
                              16, 8);
          renderer.render(skin, src_rect, dst_rect);
        }
        /* 下边 */
        if (contents.height() - w->oy > w->height - 32) {
          cen::irect src_rect(128 + 24, 40, 16, 8);
          cen::irect dst_rect(w->x + w->width / 2 - 8 - v->ox,
                              w->y + w->height - 12 - v->oy, 16, 8);
          renderer.render(skin, src_rect, dst_rect);
        }
      }

      /* 暂停标记 */
      if (w->pause) {
        /* 处理暂停标记的动画，选择 windowskin 上不同的区域 */
        int src_x = (w->update_count & 0b01000) ? 128 + 32 + 16 : 128 + 32;
        int src_y = (w->update_count & 0b10000) ? 64 + 16 : 64;

        cen::irect src_rect(src_x, src_y, 16, 16);
        cen::irect dst_rect(w->x + w->width / 2 - 8 - v->ox,
                            w->y + w->height - 16 - v->oy, 16, 16);

        renderer.render(skin, src_rect, dst_rect);
      }
    }
  }
};
}  // namespace rgm::rmxp