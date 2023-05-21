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
  /// @brief 最多支持的鼠标按键数量，设置为 8 已足够
  static constexpr size_t max = 8;

  /// @brief 存储按键状态的数组，索引代表相应的按键，值代表按键的状态。
  /// 值的每一位代表每一帧该按键是否按下，0 表示松开，1 表示按下。最后一位
  /// （最低位）代表当前帧的情况，倒数第 N 位代表向前推算 N - 1 帧的情况。
  /// 比如 0b00000000_00000000_00000000_00000001，代表此帧是刚刚按下对应按键。
  std::array<uint64_t, max> m_data;

  /// @brief 记录最后一次汇报鼠标事件时的 X 坐标值
  int x;

  /// @brief 记录最后一次汇报鼠标事件时的 Y 坐标值
  int y;

  /// @brief 鼠标滚轮的 X 坐标值
  int wheel_x;

  /// @brief 鼠标滚轮的 Y 坐标值
  int wheel_y;

  /// @brief 更新按键当前帧状态，默认延续上一帧的状态。
  /// S0 -> S00，S1 -> S11
  /// 在此之后才会执行 key_press 和 key_release 的事件继续更新。
  void update() {
    wheel_x = 0;
    wheel_y = 0;
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

    /*
     * 在抬起时，判断是否刚刚经历了双击事件：
     * 只看 60 位
     * 1. 数据是否为 1.....10
     * 2. 数据是否为 01....10
     * 3. 数据是否为 0....01010
     * 4. 数据是否为 0...0101...10
     * 满足以上条件将设置 state 为 0
     */
    constexpr uint64_t pattern_0 = (1ull << 60) - 1;
    uint64_t value = m_data[button % max] & pattern_0;

    constexpr uint64_t pattern_1 = pattern_0 - 1;
    constexpr uint64_t pattern_2 = pattern_1 - (1ull << 59);
    constexpr uint64_t pattern_3 = 0b1010;

    if (value == pattern_1 || value == pattern_2 || value == pattern_3) {
      m_data[button % max] = 0;
    } else {
      int n = 59;
      while (n > 3) {
        if ((value & (1ull << n)) != 0) {
          uint64_t pattern_4 = (1ull << n) + (1ull << (n - 1)) - 2;
          if (value == pattern_4) {
            m_data[button % max] = 0;
          }
          break;
        }
        --n;
      }
    }
  }

  /// @brief 判断按键是否刚刚按下，只需检测最低 2 位是否为 01
  /// @param key 目标按键
  /// @return 返回 true 表示此按键刚刚按下
  [[nodiscard]] bool is_trigger(uint8_t button) const {
    return (m_data[button % max] & 0b11) == 0b01;
  }

  /// @brief 判断按键是否正在按下，只需检测最低 1 位是否为 1
  /// @param key 目标按键
  /// @return 返回 true 表示此按键正在按下
  [[nodiscard]] bool is_press(uint8_t button) const {
    return (m_data[button % max] & 0b01) == 0b01;
  }

  /// @brief 双击判定，需要传入间隔的最大帧数
  /// @param key 目标按键
  /// @param interval 判定双击的间隔帧数上限
  /// 为了尽可能精确的判断双击，按键抬起时有额外的判断。
  [[nodiscard]] bool is_double_click(uint8_t button, int interval) {
    interval = std::clamp(interval, 1, 60);

    uint64_t value = m_data[button % max];

    if ((value & 0b11) != 0b01) return false;

    uint64_t pattern = (1ull << interval) - 1;
    if (((value >> 2) & pattern) == 0) return false;

    /* 双击触发后，按键的状态被修改为 0b101 */
    m_data[button % max] = 0b101;
    return true;
  }

  /// @brief 重置所有的按键状态为抬起，清空记录的坐标值
  void reset() {
    m_data.fill(0);
    x = 0;
    y = 0;
  }
};

/// @brief 鼠标移动的事件
struct mouse_motion {
  /// @brief 鼠标在窗口中的 X 坐标
  int x;

  /// @brief 鼠标在窗口中的 Y 坐标
  int y;

  void run(auto& worker) {
    mousestate& ms = RGMDATA(mousestate);

    ms.x = x;
    ms.y = y;
  }
};

/// @brief 鼠标按键按下的事件
struct mouse_press {
  uint8_t button;

  void run(auto& worker) {
    mousestate& ms = RGMDATA(mousestate);
    ms.press(button);
  }
};

/// @brief 鼠标按键抬起的事件
struct mouse_release {
  uint8_t button;

  void run(auto& worker) {
    mousestate& ms = RGMDATA(mousestate);
    ms.release(button);
  }
};

/// @brief 鼠标滚轮的事件
struct mouse_wheel {
  /// @brief 鼠标滚轮在 X 方向上滚动的距离
  int x;

  /// @brief 鼠标滚轮在 X 方向上滚动的距离
  int y;

  void run(auto& worker) {
    mousestate& ms = RGMDATA(mousestate);

    ms.wheel_x = x;
    ms.wheel_y = y;
  }
};

/// @brief 鼠标事件相关的初始化类
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

    /* 绑定鼠标移动事件 */
    d.bind<cen::mouse_motion_event>().to(
        [&worker](const cen::mouse_motion_event& e) {
          worker >> mouse_motion{e.x(), e.y()};
        });

    /* 绑定鼠标滚轮事件 */
    d.bind<cen::mouse_wheel_event>().to(
        [&worker](const cen::mouse_wheel_event& e) {
          int x = static_cast<int>(e.x());
          int y = static_cast<int>(e.y());
          cen::log_debug("[Input] mouse wheel [%d, %d]", x, y);

          worker >> mouse_wheel{x, y};
        });
  }
};

/// @brief 鼠标相关操作的初始化类
struct init_mouse {
  using data = std::tuple<mousestate>;

  static void before(auto& this_worker) {
    /* 需要使用 base::detail 完成 ruby 到 C++ 类型的转换 */
    using detail = base::detail;

    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;
    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Ext#mouse_press -> mousestate::is_press */
      static VALUE press(VALUE, VALUE button_) {
        RGMLOAD(button, uint8_t);

        return RGMDATA(mousestate).is_press(button) ? Qtrue : Qfalse;
      }

      /* ruby method: Ext#mouse_trigger -> mousestate::is_trigger */
      static VALUE trigger(VALUE, VALUE button_) {
        RGMLOAD(button, uint8_t);

        return RGMDATA(mousestate).is_trigger(button) ? Qtrue : Qfalse;
      }

      /* ruby method: Ext#mouse_double_click -> mousestate::is_double_click */
      static VALUE double_click(VALUE, VALUE button_, VALUE interval_) {
        RGMLOAD(button, uint8_t);
        RGMLOAD(interval, int);

        mousestate& ms = RGMDATA(mousestate);
        return ms.is_double_click(button, interval) ? Qtrue : Qfalse;
      }

      /* ruby method: Ext#mouse_update -> mousestate::update */
      static VALUE update(VALUE) {
        RGMDATA(mousestate).update();
        return Qnil;
      }

      /* ruby method: Ext#mouse_reset -> mousestate::reset */
      static VALUE reset(VALUE) {
        RGMDATA(mousestate).reset();
        return Qnil;
      }

      /* ruby method: Ext#mouse_x -> mousestate::x */
      static VALUE position_x(VALUE) { return INT2FIX(RGMDATA(mousestate).x); }

      /* ruby method: Ext#mouse_y -> mousestate::y */
      static VALUE position_y(VALUE) { return INT2FIX(RGMDATA(mousestate).y); }

      /* ruby method: Ext#mouse_wheel -> mousestate::wheel_y */
      static VALUE wheel(VALUE) { return INT2FIX(RGMDATA(mousestate).wheel_y); }
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
    rb_define_module_function(rb_mRGM_Ext, "mouse_wheel", wrapper::wheel, 0);
  }
};
}  // namespace rgm::ext