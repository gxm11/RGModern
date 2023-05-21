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

namespace rgm::ext {
struct mousestate {
  static constexpr size_t max = 8;

  std::array<uint64_t, max> m_data;
  int x;
  int y;

  /// @brief 更新按键当前帧状态，默认延续上一帧的状态。
  /// S0 -> S00，S1 -> S11
  /// 在此之后才会执行 key_press 和 key_release 的事件继续更新。
  void update() {
    for (size_t i = 0; i < max; ++i) {
      const uint64_t value = m_data[i];

      m_data[i] = (value & 1) | (value << 1);
    }
  }

  /// @brief 将按键当前帧的状态改为按下
  /// @param key 目标按键
  /// S? -> S1
  void press(uint8_t button) { m_data[button % max] |= 0b01; }

  /// @brief 将按键当前帧的状态改为松开
  /// @param key 目标按键
  /// S? -> S0
  void release(uint8_t button) {
    m_data[button % max] ^= (m_data[button % max] & 1);
  }

  [[nodiscard]] bool is_trigger(uint8_t button) const {
    return (m_data[button % max] & 0b11) == 0b01;
  }

  [[nodiscard]] bool is_press(uint8_t button) const {
    return (m_data[button % max] & 0b01) == 0b01;
  }

  /// @brief 双击判定，需要传入间隔的最大帧数
  /// @param key
  /// @param interval
  [[nodiscard]] bool is_double_click(uint8_t button, int interval) {
    interval = std::clamp(interval, 1, 60);

    uint64_t value = m_data[button % max];

    if ((value & 0b11) != 0b01) return false;

    uint64_t pattern = (1ull << interval) - 1;
    if (((value - 1) & pattern) == 0) return false;

    m_data[button % max] = 0;
    return true;
  }

  void reset() {
    m_data.fill(0);
    x = 0;
    y = 0;
  }
};

struct mouse_motion {
  int x;
  int y;

  void run(auto& worker) {
    mousestate& ms = RGMDATA(mousestate);

    ms.x = x;
    ms.y = y;
  }
};

struct mouse_press {
  uint8_t button;

  void run(auto& worker) {
    mousestate& ms = RGMDATA(mousestate);
    ms.press(button);
  }
};

struct mouse_release {
  uint8_t button;

  void run(auto& worker) {
    mousestate& ms = RGMDATA(mousestate);
    ms.release(button);
  }
};

/// @brief 鼠标相关的初始化类
/// @todo 完善鼠标的功能
struct init_mouse_event {
  static void before(auto& worker) {
    base::cen_library::event_dispatcher_t& d =
        RGMDATA(base::cen_library).event_dispatcher;

    /* 绑定鼠标按键事件 */
    d.bind<cen::mouse_button_event>().to(
        [&worker](const cen::mouse_button_event& e) {
          if (e.released()) {
            cen::log_debug("[Input] mouse button '%s' is released",
                           cen::to_string(e.button()).data());

            uint8_t button = static_cast<uint8_t>(e.button());
            worker >> mouse_release{button};
            worker >> mouse_motion{e.x(), e.y()};
          } else if (e.pressed()) {
            cen::log_debug("[Input] mouse button '%s' is pressed",
                           cen::to_string(e.button()).data());

            uint8_t button = static_cast<uint8_t>(e.button());
            worker >> mouse_press{button};
            worker >> mouse_motion{e.x(), e.y()};
          }
        });

    d.bind<cen::mouse_motion_event>().to(
        [&worker](const cen::mouse_motion_event& e) {
          worker >> mouse_motion{e.x(), e.y()};
        });
  }
};

struct init_mouse {
  using data = std::tuple<mousestate>;

  static void before(auto& this_worker) {
    /* 需要使用 base::detail 完成 ruby 到 C++ 类型的转换 */
    using detail = base::detail;

    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;
    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      static VALUE press(VALUE, VALUE button_) {
        RGMLOAD(button, uint8_t);

        return RGMDATA(mousestate).is_press(button) ? Qtrue : Qfalse;
      }

      static VALUE trigger(VALUE, VALUE button_) {
        RGMLOAD(button, uint8_t);

        return RGMDATA(mousestate).is_trigger(button) ? Qtrue : Qfalse;
      }

      static VALUE double_click(VALUE, VALUE button_, VALUE interval_) {
        RGMLOAD(button, uint8_t);
        RGMLOAD(interval, int);

        return RGMDATA(mousestate).is_double_click(button, interval) ? Qtrue
                                                                     : Qfalse;
      }

      static VALUE update(VALUE) {
        RGMDATA(mousestate).update();
        return Qnil;
      }

      static VALUE reset(VALUE) {
        RGMDATA(mousestate).reset();
        return Qnil;
      }

      static VALUE position_x(VALUE) { return INT2FIX(RGMDATA(mousestate).x); }
      static VALUE position_y(VALUE) { return INT2FIX(RGMDATA(mousestate).y); }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Ext = rb_define_module_under(rb_mRGM, "Ext");

    rb_define_module_function(rb_mRGM_Ext, "mouse_press", wrapper::press, 1);
    rb_define_module_function(rb_mRGM_Ext, "mouse_trigger", wrapper::trigger,
                              1);
    rb_define_module_function(rb_mRGM_Ext, "mouse_double_click",
                              wrapper::double_click, 2);
    rb_define_module_function(rb_mRGM_Ext, "mouse_update", wrapper::update, 0);
    rb_define_module_function(rb_mRGM_Ext, "mouse_reset", wrapper::reset, 0);
    rb_define_module_function(rb_mRGM_Ext, "mouse_x", wrapper::position_x, 0);
    rb_define_module_function(rb_mRGM_Ext, "mouse_y", wrapper::position_y, 0);
  }
};
}  // namespace rgm::ext