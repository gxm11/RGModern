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
#include "input.hpp"

namespace rgm::rmxp {
/// @brief 控制器的摇杆和扳机的触发事件
/// config::controller_left_arrow 和 config::controller_right_arrow 分别决定
/// 是否将左、右摇杆映射成方向键的输入。这里也可以类似添加其他操作。
struct controller_axis_move {
  /*
   * 摇杆的状态变化。考虑到触发阈值 t 和数值上限 a，那么存在以下 3 个区段：
   * [-a, -t), [-t, t), [t, a)。
   * 故可能导致方向键触发的有以下四种情况：
   * 值从 [-a, -t) -> [-t, a) 触发方向键的 release，对应 negative_greater；
   * 值从 [-t, a) -> [-a, -t) 触发方向键的 press，对应 negative_less；
   * 值从 [-a, t) -> [t, a) 触发方向键的 press，对应 positive_greater；
   * 值从 [t, a) -> [-a, t) 触发方向键的 release，对应 positive_less；
   * 这 4 种情况逻辑上只有 1 种会触发，如果都没有触发则对应 no_change。
   */
  enum class state {
    no_change,
    negative_greater,
    negative_less,
    postive_greater,
    postive_less
  };

  /// @brief 控制器的摇杆和扳机的类型
  cen::controller_axis axis;

  /// @brief 控制器的索引
  int joy_index;

  /// @brief 当前摇杆和扳机的值
  int value;

  void keyevent_helper(auto& worker, state s, int32_t key1, int32_t key2) {
    /* 直接发送按键事件，在下一次 Input.update 时处理 */
    switch (s) {
      default:
        break;
      case state::negative_greater:
        worker >> key_release{key1};
        break;
      case state::negative_less:
        worker >> key_press{key1};
        break;
      case state::postive_greater:
        worker >> key_press{key2};
        break;
      case state::postive_less:
        worker >> key_release{key2};
        break;
    }
  }

  [[nodiscard]] static state get_state(int before, int after) {
    constexpr int t = config::controller_axis_threshold;

    if (before < -t && -t <= after) return state::negative_greater;
    if (after <= -t && -t < before) return state::negative_less;
    if (before < t && t <= after) return state::postive_greater;
    if (after <= t && t < before) return state::postive_less;

    return state::no_change;
  }

  void run(auto& worker) {
    base::controller_axisstate& ca = RGMDATA(base::controller_axisstate);

    size_t index = static_cast<int>(axis) +
                   joy_index * static_cast<int>(cen::controller_axis::max);

    /* 检测到控制器的索引非法则返回 */
    if (joy_index < 0 ||
        static_cast<size_t>(joy_index) >= base::controller_maxsize)
      return;

    /* 储存当前 controller_axisstate 的对应值后，再设置 controller_axisstate */
    const int old_value = ca[index];
    ca[index] = value;

    const state s = get_state(old_value, value);

    /* 是否触发 x 和 y 方向上的方向键事件 */
    const bool x_axis = ((config::controller_left_arrow &&
                          axis == cen::controller_axis::left_x) ||
                         (config::controller_right_arrow &&
                          axis == cen::controller_axis::right_x));
    const bool y_axis = ((config::controller_left_arrow &&
                          axis == cen::controller_axis::left_y) ||
                         (config::controller_right_arrow &&
                          axis == cen::controller_axis::right_y));

    /* x 轴的变化触发左右键 */
    if (x_axis) keyevent_helper(worker, s, SDLK_LEFT, SDLK_RIGHT);

    /* y 轴的变化触发上下键 */
    if (y_axis) keyevent_helper(worker, s, SDLK_UP, SDLK_DOWN);
  }
};

/// @brief 记录控制器按键与 RGSS 中虚拟按键的映射关系
/// 同一个控制器按键可以对应多个虚拟按键，当此控制器按键按下或抬起时，
/// 会触发此按键对应的全部虚拟按键的按下或抬起。
/// RGSS 中可以调用 Input.trigger? 等方法检测相应的虚拟按键。
/// @see ./src/rxmp/input.hpp
struct controller_buttonmap {
  /// @brief 储存映射关系的 set，有序排放
  /// pair 中第一个元素是 Button，
  /// 第二个元素是 RGSS 虚拟按键，即 RMXP Input Key。
  std::set<std::pair<int, int>> m_data;

  /// @brief 遍历所有的 { Button, RMXP Input Key } 组合，执行特定函数
  /// @param 组合的第一项必须等于该值
  /// @param callback 只接受一个 int 类型的参数，即 RMXP Input Key
  /// callback 通常是修改 RMXP Input Key 按下或抬起的状态。
  /// @see ./src/rxmp/input.hpp
  void iterate(int button, std::function<void(int)> callback) const {
    /*
     * 查询不小于 {button, 0} 的那个元素
     * 此元素显然是 {button, i}，i 对应绑定的值最小的那个虚拟按键值
     * 若 button 未有任何绑定，则返回 end()
     */
    auto it = m_data.lower_bound({button, 0});

    while (it != m_data.end()) {
      /* 当控制器按键不再是 button 时，结束循环 */
      if (it->first != button) break;

      /* 使用绑定的虚拟按键触发回调 */
      callback(it->second);

      ++it;
    }
  }

  /// @brief 重置特定控制器按键的全部映射，使其不再对应任何虚拟按键
  /// 即从 set 中移除 {button, i}，i 是任意的 int
  void erase(int button) {
    /*
     * 查询不小于 {button, 0} 的那个元素
     * 此元素显然是 {button, i}，i 对应绑定的值最小的那个虚拟按键值
     * 若 button 未有任何绑定，则返回 end()
     */
    auto begin = m_data.lower_bound({button, 0});

    if (begin == m_data.end()) return;

    /*
     * 查询不小于 {button + 1, 0} 的那个元素
     * 此元素显然是 {button_next, ?}，其中 button_next > button。
     */
    auto end = m_data.lower_bound({button + 1, 0});

    /* STL 中算法遵循前闭后开原则，begin 会被移除而 end 不会被移除 */
    m_data.erase(begin, end);
  }

  /// @brief 添加一组映射关系
  /// @param button 控制器按键
  /// @param key RGSS 虚拟按键
  void insert(int button, int key) { m_data.insert({button, key}); }
};

/// @brief 控制器按键按下的事件
struct controller_button_press {
  /// @brief 控制器的索引
  int joy_index;

  /// @brief 控制器的按键
  int button;

  void run(auto& worker) {
    controller_buttonmap& map = RGMDATA(controller_buttonmap);
    keystate& state = RGMDATA(keystate);

    /* 不同的索引对应的按键也不同 */
    int button2 =
        button + joy_index * static_cast<int>(cen::controller_button::max);

    /* 记录为 last_press 值 */
    state.last_press = button2;

    /* 修改 key_state 中对应的值 */
    map.iterate(button2, [&state](int key) { state.press(key); });
  }
};

/// @brief 控制器按键抬起的事件
struct controller_button_release {
  /// @brief 控制器的索引
  int joy_index;

  /// @brief 控制器的按键
  int button;

  void run(auto& worker) {
    controller_buttonmap& map = RGMDATA(controller_buttonmap);
    keystate& state = RGMDATA(keystate);

    /* 不同的索引对应的按键也不同 */
    int button2 =
        button + joy_index * static_cast<int>(cen::controller_button::max);

    /* 记录为 last_release 值 */
    state.last_release = button2;

    /* 修改 key_state 中对应的值 */
    map.iterate(button2, [&state](int key) { state.release(key); });
  }
};

/// @brief 控制器震动事件
/// @ref https://wiki.libsdl.org/SDL2/SDL_GameControllerRumble
struct controller_rumble {
  /// @brief 控制器的索引
  int joy_index;

  /// @brief （左边）低频震动马达的强度，范围是 0 ~ 65535
  int low;

  /// @brief （右边）高频震动马达的强度，范围是 0 ~ 65535
  int high;

  /// @brief 震动的时间，单位是毫秒（ms）
  int duration;

  void run(auto& worker) {
    std::map<int, cen::controller>& cs = RGMDATA(base::cen_library).controllers;

    /* 判断控制器是否处于连接状态 */
    if (cen::controller::supported(joy_index)) {
      cen::controller& c = cs[joy_index];
      c.rumble(low, high, cen::u32ms{duration});
    }
  }
};

/// @brief 控制器的扳机震动事件
/// @ref https://wiki.libsdl.org/SDL2/SDL_GameControllerRumbleTriggers
/// 根据上面的资料，扳机震动只支持 Xbox One 控制器。
struct controller_rumble_triggers {
  /// @brief 控制器的索引
  int joy_index;

  /// @brief 左边震动马达的强度，范围是 0 ~ 65535
  int left;

  /// @brief 右边震动马达的强度，范围是 0 ~ 65535
  int right;

  /// @brief 震动的时间，单位是毫秒（ms）
  int duration;

  void run(auto& worker) {
    std::map<int, cen::controller>& cs = RGMDATA(base::cen_library).controllers;

    /* 判断控制器是否处于连接状态 */
    if (cen::controller::supported(joy_index)) {
      cen::controller& c = cs[joy_index];
      c.rumble_triggers(left, right, cen::u32ms{duration});
    }
  }
};

/// @brief 控制器相关操作的初始化类
struct init_controller {
  using data = std::tuple<controller_buttonmap>;

  static void before(auto& this_worker) noexcept {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Base#controller_bind -> controller_buttonmap::insert */
      static VALUE bind(VALUE, VALUE button_, VALUE input_key_,
                        VALUE joy_index_) {
        controller_buttonmap& map = RGMDATA(controller_buttonmap);
        RGMLOAD(joy_index, int);
        RGMLOAD(button, int);

        int button2 =
            button + joy_index * static_cast<int>(cen::controller_button::max);

        if (input_key_ == Qnil) {
          map.erase(button2);
        } else {
          RGMLOAD(input_key, int);

          map.insert(button2, input_key);
        }
        return Qnil;
      }

      /* ruby method: Base#controller_axis_value -> controller_axisstate */
      static VALUE axis_value(VALUE, VALUE axis_, VALUE joy_index_) {
        base::controller_axisstate& state = RGMDATA(base::controller_axisstate);

        RGMLOAD(joy_index, int);
        RGMLOAD(axis, int);

        size_t index =
            axis + joy_index * static_cast<int>(cen::controller_axis::max);
        return INT2FIX(state.at(index));
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "controller_bind", wrapper::bind,
                              3);
    rb_define_module_function(rb_mRGM_Base, "controller_axis_value",
                              wrapper::axis_value, 2);

    RGMBIND(rb_mRGM_Base, "controller_rumble", controller_rumble, 4);
    RGMBIND(rb_mRGM_Base, "controller_rumble_triggers",
            controller_rumble_triggers, 4);
  }
};
}  // namespace rgm::rmxp