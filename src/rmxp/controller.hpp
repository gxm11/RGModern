// zlib License

// copyright (C) 2023 Guoxiaomi and Krimiston

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
#include "base/base.hpp"
#include "input.hpp"

namespace rgm::rmxp {
using controller_axisstate =
    std::array<int, static_cast<size_t>(cen::controller_axis::max) * 8>;

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

struct controller_axis_move {
  int joy_index;
  int axis;
  int value;

  void run(auto& worker) {
    controller_axisstate& ca = RGMDATA(controller_axisstate);

    if (joy_index >= 0 && joy_index < static_cast<int>(ca.size())) {
      size_t index =
          axis + joy_index * static_cast<int>(cen::controller_axis::max);
      ca.at(index) = value;
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

struct init_controller {
  using data = std::tuple<controller_buttonmap, controller_axisstate>;

  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;

    struct wrapper {
      static VALUE bind(VALUE, VALUE joy_index_, VALUE button_,
                        VALUE input_key_) {
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

      static VALUE axis_value(VALUE, VALUE joy_index_, VALUE axis_) {
        controller_axisstate& state = RGMDATA(controller_axisstate);

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
  }
};
}  // namespace rgm::rmxp