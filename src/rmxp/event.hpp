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
#include "base/base.hpp"
#include "input.hpp"
#include "textinput.hpp"

namespace rgm::rmxp {
/**
 * @brief 绑定 event_dispatcher_t 对不同输入事件的响应
 * @note 事件分为以下 4 类：
 * 1. 退出事件，广播 interrupt_signal
 * 2. 窗口事件，处理后台运行时使用，未完成
 * 3. 键盘事件，广播 key_release 和 key_press
 * 4. 鼠标事件，未完成
 */
struct init_event {
  static void before(auto& worker) {
    base::cen_library::event_dispatcher_t& d =
        RGMDATA(base::cen_library).event_dispatcher;

    d.bind<cen::quit_event>().to(
        [&worker]([[maybe_unused]] const cen::quit_event& e) {
          cen::log_info(cen::log_category::system, "[Input] quit");
          worker >> base::interrupt_signal{};
        });

    d.bind<cen::window_event>().to([&worker](const cen::window_event& e) {
      cen::log_info(cen::log_category::input, "[Input] window %s",
                    cen::to_string(e.event_id()).data());
    });

    d.bind<cen::keyboard_event>().to([&worker](const cen::keyboard_event& e) {
      if (e.released()) {
        cen::log_debug(cen::log_category::input, "[Input] key '%s' is pressed",
                       e.key().name().data());

        const int32_t key = static_cast<int32_t>(e.key().get());
        worker >> key_release{key};
      } else if (e.pressed()) {
        cen::log_debug(cen::log_category::input, "[Input] key '%s' is released",
                       e.key().name().data());

        const int32_t key = static_cast<int32_t>(e.key().get());
        worker >> key_press{key};
      }
    });

    d.bind<cen::mouse_button_event>().to(
        [&worker](const cen::mouse_button_event& e) {
          if (e.released()) {
            cen::log_debug(cen::log_category::input,
                           "[Input] mouse button '%s' is released",
                           cen::to_string(e.button()).data());
          } else if (e.pressed()) {
            cen::log_debug(cen::log_category::input,
                           "[Input] mouse button '%s' is pressed",
                           cen::to_string(e.button()).data());
          }
        });

    d.bind<cen::text_editing_event>().to(
        [&worker](const cen::text_editing_event& e) {
          cen::log_debug(cen::log_category::input, "[Input] text edit\n");

          worker >> text_edit{std::string{e.text()}, e.start()};
        });

    d.bind<cen::text_input_event>().to(
        [&worker](const cen::text_input_event& e) {
          cen::log_info(cen::log_category::input, "[Input] text input '%s'\n",
                        e.text_utf8().data());

          worker >> text_input{std::string{e.text_utf8()}};
        });
  }
};
}  // namespace rgm::rmxp