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
#include "core/core.hpp"
#include "renderstack.hpp"

namespace rgm::base {
/// @brief 秒表 render，统计每帧渲染花费的时间
/// 起点在 clear_screen 中，终点在 present_window 中
core::stopwatch render_timer("render");

/// @brief 重新设置绘制屏幕的大小
/// @name task
/// 屏幕上的内容会被缩放到窗口上，然后再经过一次全屏的缩放展示给玩家
struct resize_screen {
  /// @brief 屏幕的宽
  int width;

  /// @brief 屏幕的高度
  int height;

  void run(auto& worker) {
    renderstack& stack = RGMDATA(renderstack);

    /* 开发模式检查是否有 renderstack 的出入栈错误 */
    if constexpr (config::develop) {
      if (stack.stack.size() != 1) {
        cen::log_error(
            "resize screen failed, the stack depth is not equal to 1!");
        throw std::length_error{"renderstack in resize screen"};
      }
    }
    cen::texture screen = stack.make_empty_texture(width, height);
    screen.set_scale_mode(cen::scale_mode::nearest);

    stack.stack.pop_back();
    stack.stack.push_back(std::move(screen));
  }
};

/// @brief 重新设置窗口的大小
/// @name task
struct resize_window {
  /// @brief 窗口的宽
  int width;

  /// @brief 窗口的高度
  int height;

  /// @brief 屏幕绘制到窗口上的缩放模式
  int scale_mode;

  void run(auto& worker) {
    cen::window& window = RGMDATA(cen_library).window;
    renderstack& stack = RGMDATA(renderstack);

    RGMDATA(cen_library).scale_mode = scale_mode;

    if (window.width() == width && window.height() == height) return;

    /* 开发模式检查是否有 renderstack 的出入栈错误 */
    if constexpr (config::develop) {
      if (stack.stack.size() != 1) {
        cen::log_error(
            "In <resize screen>, the stack depth is not equal to 1!");
        throw std::length_error{"renderstack in resize screen"};
      }
    }

    /* 保持窗口的中心位置不变 */
    window.set_x(window.x() + (window.width() - width) / 2);
    window.set_y(window.y() + (window.height() - height) / 2);
    window.set_size(cen::iarea{width, height});
  }
};

/// @brief 渲染之前的处理，清空 renderstack 栈底 texture 的内容
/// @name task
struct clear_screen {
  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(cen_library).renderer;
    renderstack& stack = RGMDATA(renderstack);

    /* render_timer 的起点 */
    render_timer.start();
    render_timer.step(1);

    /* 开发模式检查是否有 renderstack 的出入栈错误 */
    if constexpr (config::develop) {
      if (stack.stack.size() != 1) {
        cen::log_error("In <clear_screen>, the stack size is not equal to 1!");
        throw std::length_error{"renderstack in clear screen"};
      }
    }

    renderer.set_target(stack.current());
    renderer.set_blend_mode(cen::blend_mode::none);
    renderer.clear_with(config::screen_background_color);
    render_timer.step(2);
  }
};

/// @brief 渲染结束的处理，将渲染栈的栈底绘制到窗口上
/// @name task
/// 垂直同步默认开启，此任务会等待垂直同步信号
struct present_window {
  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(cen_library).renderer;
    cen::window& window = RGMDATA(cen_library).window;
    renderstack& stack = RGMDATA(renderstack);
    int scale_mode = RGMDATA(cen_library).scale_mode;

    render_timer.step(3);

    /* 开发模式检查是否有 renderstack 的出入栈错误 */
    if constexpr (config::develop) {
      if (stack.stack.size() != 1) {
        cen::log_error(
            "In <present_window>, the stack size is not equal to 1!");
        throw std::length_error{"renderstack in present window"};
      }
    }

    /* 渲染 window 的内容 */
    renderer.reset_target();
    renderer.reset_clip();
    renderer.set_blend_mode(cen::blend_mode::none);
    renderer.fill_with(cen::colors::transparent);

    cen::irect src_rect(0, 0, stack.current().width(),
                        stack.current().height());
    cen::irect dst_rect(0, 0, window.width(), window.height());

    switch (scale_mode) {
      case 0:
        stack.current().set_scale_mode(cen::scale_mode::nearest);
        break;
      case 1:
        stack.current().set_scale_mode(cen::scale_mode::linear);
        break;
      case 2:
        stack.current().set_scale_mode(cen::scale_mode::best);
        break;
      default:
        /* 其它缩放模式的场合，屏幕不缩放而是居中显示 */
        stack.current().set_scale_mode(cen::scale_mode::nearest);
        dst_rect.set_size(src_rect.width(), src_rect.height());
        dst_rect.set_position((window.width() - src_rect.width()) / 2,
                              (window.height() - src_rect.height()) / 2);
        break;
    }
    renderer.render(stack.current(), src_rect, dst_rect);
    render_timer.step(4);

    /* 调用 present() 会阻塞，直到收到垂直同步信号 */
    renderer.present();

    /* render_timer 的终点 */
    render_timer.step(5);
  }
};

/// @brief 窗口、画面相关的初始化类
/// @name task
struct init_render {
  static void before(auto& worker) {
    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");

    RGMBIND(rb_mRGM_Base, "present_window", base::present_window, 0);
    RGMBIND(rb_mRGM_Base, "resize_screen", base::resize_screen, 2);
    RGMBIND(rb_mRGM_Base, "resize_window", base::resize_window, 3);
  }
};
}  // namespace rgm::base
