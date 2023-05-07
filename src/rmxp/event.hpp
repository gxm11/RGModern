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
#include "controller.hpp"
#include "input.hpp"

namespace rgm::rmxp {
/// @brief 绑定 event_dispatcher_t 对不同输入事件的响应
/// 此处绑定了以下 5 种事件：
/// 1. 退出事件，执行 worker.stop()；
/// 2. 窗口事件，目前没有效果，只是打印日志；
/// 3. 键盘事件，发送 key_release 和 key_press；
/// 4. 控制器摇杆和扳机事件，发送 controller_axis_move；
/// 5. 控制器按键事件，发送 controller_button_press 和
/// controller_button_release。
struct init_event {
  static void before(auto& worker) noexcept {
    base::cen_library::event_dispatcher_t& d =
        RGMDATA(base::cen_library).event_dispatcher;

    /* 退出事件 */
    d.bind<cen::quit_event>().to(
        [&worker]([[maybe_unused]] const cen::quit_event& e) {
          cen::log_warn("[Input] quit");
          worker.stop();
        });

    /* 窗口事件 */
    d.bind<cen::window_event>().to([&worker](const cen::window_event& e) {
      cen::log_info("[Input] window %s", cen::to_string(e.event_id()).data());
    });

    /* 键盘事件 */
    d.bind<cen::keyboard_event>().to([&worker](const cen::keyboard_event& e) {
      if (e.released()) {
        cen::log_debug("[Input] key '%s' is released", e.key().name().data());

        const int32_t key = static_cast<int32_t>(e.key().get());
        worker >> key_release{key};
      } else if (e.pressed()) {
        cen::log_debug("[Input] key '%s' is pressed", e.key().name().data());

        const int32_t key = static_cast<int32_t>(e.key().get());
        worker >> key_press{key};
      }
    });

    /* 控制器摇杆和扳机事件 */
    d.bind<cen::controller_axis_event>().to(
        [&worker](const cen::controller_axis_event& e) {
          cen::log_debug("[Input] controller axis '%s', value %d\n",
                         cen::to_string(e.axis()).data(), e.value());

          worker >> controller_axis_move{e.axis(), e.which(), e.value()};
        });

    /* 控制器按键事件 */
    d.bind<cen::controller_button_event>().to(
        [&worker](const cen::controller_button_event& e) {
          if (e.is_released()) {
            cen::log_debug("[Input] controller %d button '%s' is released",
                           e.which(), cen::to_string(e.button()).data());

            const int key = static_cast<int>(e.button());
            worker >> controller_button_release{e.which(), key};
          } else if (e.is_pressed()) {
            cen::log_debug("[Input] controller %d button '%s' is pressed",
                           e.which(), cen::to_string(e.button()).data());

            const int key = static_cast<int>(e.button());
            worker >> controller_button_press{e.which(), key};
          }
        });
  }
};
}  // namespace rgm::rmxp