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
#include "core/core.hpp"
#include "renderstack.hpp"

namespace rgm::base {
core::stopwatch render_timer("render");

struct resize_screen {
  int width;
  int height;

  void run(auto& worker) {
    base::renderstack& stack = RGMDATA(base::renderstack);
    if constexpr (config::check_renderstack) {
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

struct resize_window {
  int width;
  int height;

  void run(auto& worker) {
    cen::window& window = RGMDATA(base::cen_library).window;
    window.set_x(window.x() + (window.width() - width) / 2);
    window.set_y(window.y() + (window.height() - height) / 2);
    window.set_size(cen::iarea{ width, height });
  }
};

/**
 * @brief 任务：渲染之前的处理，清空 renderstack 栈底 texture 的内容
 */
struct clear_screen {
  void run(auto& worker) {
    render_timer.start();
    render_timer.step(1);
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);
    if constexpr (config::check_renderstack) {
      if (stack.stack.size() != 1) {
        cen::log_error("In <clear_screen>, the stack size is not equal to 1!");
        throw std::length_error{"renderstack in clear screen"};
      }
    }
    renderer.set_target(stack.current());
    renderer.set_blend_mode(cen::blend_mode::none);
    renderer.clear_with(cen::colors::transparent);
    render_timer.step(2);
  }
};

/**
 * @brief 任务：渲染结束的处理，解锁 ruby 线程并等待垂直同步信息更新画面。
 */
struct present_window {
  int scale_mode;

  void run(auto& worker) {
    render_timer.step(3);

    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    cen::window& window = RGMDATA(base::cen_library).window;
    base::renderstack& stack = RGMDATA(base::renderstack);
    if constexpr (config::check_renderstack) {
      if (stack.stack.size() != 1) {
        cen::log_error(
            "In <present_window>, the stack size is not equal to 1!");
        throw std::length_error{"renderstack in present window"};
      }
    }
    // rendering the window
    renderer.reset_target();
    renderer.reset_clip();
    renderer.set_blend_mode(cen::blend_mode::none);
    renderer.fill_with(cen::colors::transparent);

    cen::irect src_rect(0, 0, stack.current().width(),
                        stack.current().height());
    cen::irect dst_rect(0, 0, window.width(), window.height());
    switch (scale_mode) {
      case 0:
      default:
        stack.current().set_scale_mode(cen::scale_mode::nearest);
        break;
      case 1:
        stack.current().set_scale_mode(cen::scale_mode::linear);
        break;
      case 2:
        stack.current().set_scale_mode(cen::scale_mode::best);
        break;
    }
    renderer.render(stack.current(), src_rect, dst_rect);
    // call present() will block this thread, waiting for v-sync signal
    render_timer.step(4);
    renderer.present();
    render_timer.step(5);
  }
};
}  // namespace rgm::base
