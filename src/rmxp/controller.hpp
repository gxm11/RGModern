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
struct controller_axis_move {
  cen::controller_axis axis;
  int joy_index;
  int value;

  void run(auto& worker) {
    base::controller_axisstate& ca = RGMDATA(base::controller_axisstate);

    size_t index = static_cast<int>(axis) +
                   joy_index * static_cast<int>(cen::controller_axis::max);

    if (joy_index < 0 ||
        static_cast<size_t>(joy_index) >= base::controller_maxsize)
      return;

    const int old_value = ca[index];
    ca[index] = value;

    const bool x_axis = ((config::controller_left_arrow &&
                          axis == cen::controller_axis::left_x) ||
                         (config::controller_right_arrow &&
                          axis == cen::controller_axis::right_x));
    const bool y_axis = ((config::controller_left_arrow &&
                          axis == cen::controller_axis::left_y) ||
                         (config::controller_right_arrow &&
                          axis == cen::controller_axis::right_y));

    enum class state {
      no_change,
      negative_greater,
      negative_less,
      postive_greater,
      postive_less
    };

    state s = [](int before, int after) -> state {
      constexpr int t = config::controller_axis_threshold;

      if (before < -t && -t <= after) return state::negative_greater;
      if (after <= -t && -t < before) return state::negative_less;
      if (before < t && t <= after) return state::postive_greater;
      if (after <= t && t < before) return state::postive_less;

      return state::no_change;
    }(old_value, value);

    if (x_axis) {
      switch (s) {
        default:
          break;
        case state::negative_greater:
          worker >> key_release{SDLK_LEFT};
          break;
        case state::negative_less:
          worker >> key_press{SDLK_LEFT};
          break;
        case state::postive_greater:
          worker >> key_press{SDLK_RIGHT};
          break;
        case state::postive_less:
          worker >> key_release{SDLK_RIGHT};
          break;
      }
    }

    if (y_axis) {
      switch (s) {
        default:
          break;
        case state::negative_greater:
          worker >> key_release{SDLK_UP};
          break;
        case state::negative_less:
          worker >> key_press{SDLK_UP};
          break;
        case state::postive_greater:
          worker >> key_press{SDLK_DOWN};
          break;
        case state::postive_less:
          worker >> key_release{SDLK_DOWN};
          break;
      }
    }
  }
};

struct controller_buttonmap : std::set<std::pair<int, int>> {
  /**
   * @brief 遍历所有的 { Button, RMXP Input Key } 组合，执行特定函数
   * @param  组合的第一项必须等于该值
   * @param callback 只接受一个参数，即 RMXP Input Key
   */
  void iterate(int button, std::function<void(int)> callback) {
    auto it = lower_bound({button, 0});
    while (it != end()) {
      if (it->first != button) break;
      callback(it->second);
      ++it;
    }
  }
};

struct controller_button_press {
  int joy_index;
  int button;

  void run(auto& worker) {
    controller_buttonmap& map = RGMDATA(controller_buttonmap);
    keystate& state = RGMDATA(keystate);

    int button2 =
        button + joy_index * static_cast<int>(cen::controller_button::max);
    state.last_press = button2;
    map.iterate(button2, [&state](int key) { state.press(key); });
  }
};

struct controller_button_release {
  int joy_index;
  int button;

  void run(auto& worker) {
    controller_buttonmap& map = RGMDATA(controller_buttonmap);
    keystate& state = RGMDATA(keystate);

    int button2 =
        button + joy_index * static_cast<int>(cen::controller_button::max);
    state.last_release = button2;
    map.iterate(button2, [&state](int key) { state.release(key); });
  }
};

struct controller_rumble {
  int joy_index;
  int low;
  int high;
  int duration;

  void run(auto& worker) {
    std::map<int, cen::controller>& cs = RGMDATA(base::cen_library).controllers;
    if (cen::controller::supported(joy_index)) {
      cen::controller& c = cs[joy_index];
      c.rumble(low, high, cen::u32ms{duration});
    }
  }
};

struct controller_rumble_triggers {
  int joy_index;
  int left;
  int right;
  int duration;

  void run(auto& worker) {
    std::map<int, cen::controller>& cs = RGMDATA(base::cen_library).controllers;
    if (cen::controller::supported(joy_index)) {
      cen::controller& c = cs[joy_index];
      c.rumble_triggers(left, right, cen::u32ms{duration});
    }
  }
};

struct init_controller {
  using data = std::tuple<controller_buttonmap>;

  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;

    struct wrapper {
      static VALUE bind(VALUE, VALUE button_, VALUE input_key_,
                        VALUE joy_index_) {
        controller_buttonmap& map = RGMDATA(controller_buttonmap);
        RGMLOAD(joy_index, int);
        RGMLOAD(button, int);

        int button2 =
            button + joy_index * static_cast<int>(cen::controller_button::max);

        if (input_key_ == Qnil) {
          auto begin = map.lower_bound({button2, 0});
          auto end = map.lower_bound({button2 + 1, 0})++;
          map.erase(begin, end);
        } else {
          RGMLOAD(input_key, int);
          map.insert({button2, input_key});
        }
        return Qnil;
      }

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